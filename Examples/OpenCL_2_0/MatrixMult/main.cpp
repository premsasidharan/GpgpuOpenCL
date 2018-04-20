#include <vector>
#include <iostream>

#include <CL/cl2.hpp>
#include <opencv2/core.hpp>

template<class T>
using svm_vector = std::vector<T, cl::SVMAllocator<T, cl::SVMTraitFine<>>>;

const std::string sSource = R"(

kernel void multiply(global float* pC, global const float* pA, global const float* pB, int M, int N, int P)
{
    local float shA[16][16];
    local float shB[16][16];

    int m = get_global_id(0);
    int p = get_global_id(1);
    int pc = (get_group_id(1)<<4)+get_local_id(0);

    float result = 0.0;
    for (int n = get_local_id(1); n < N; n += 16)
    {
        shA[get_local_id(0)][get_local_id(1)] = pA[(N*m)+n];
        shB[get_local_id(0)][get_local_id(1)] = pB[(P*n)+pc];
        barrier(CLK_LOCAL_MEM_FENCE);
        
        for (int i = 0; i < 16; i++)
        {
            result += (shA[get_local_id(0)][i]*shB[get_local_id(1)][i]);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    
    pC[(P*m)+p] = result;
}

)";

void fill_random_data(svm_vector<cl_float>& data)
{
    for (auto i = 0; i < data.size(); i++)
    {
        data[i] = (cl_float)(rand()%10);
    }
}

int matrix_mult()
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.size() == 0)
    {
        std::cout << "Platform size 0\n";
        return -1;
    }

    cl::Context context(CL_DEVICE_TYPE_GPU);
    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    cl::CommandQueue queue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

    int err = 0;
    cl_queue_properties qprop[] = { CL_QUEUE_PROPERTIES, (cl_command_queue_properties)(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT | CL_QUEUE_PROFILING_ENABLE), 0 };
    cl_command_queue dev_q = clCreateCommandQueueWithProperties(context(), devices[0](), qprop, &err);

    std::string name, version;
    devices[0].getInfo<std::string>(CL_DEVICE_NAME, &name);
    devices[0].getInfo<std::string>(CL_DEVICE_OPENCL_C_VERSION, &version);
    std::cout << name << ", " << version << std::endl;
    if (0 != version.find("OpenCL C 2.0"))
    {
        std::cout << std::endl << "OpenCL 2.0 required. Exiting..." << std::endl;
        return -1;
    }

    cl::SVMAllocator<cl_float, cl::SVMTraitFine<>> svmAlloc(context);

    const int M = 512;
    const int N = 256;
    const int P = 512;
    svm_vector<cl_float> d_mat1(svmAlloc);
    svm_vector<cl_float> d_mat2(svmAlloc);
    svm_vector<cl_float> d_mat3(svmAlloc);
    svm_vector<cl_float> d_mat4(svmAlloc);

    d_mat1.resize(M*N);
    d_mat2.resize(N*P);
    d_mat3.resize(M*P);
    d_mat4.resize(M*P);

    cv::Mat mat1(M, N, CV_32FC1, d_mat1.data());
    cv::Mat mat2(N, P, CV_32FC1, d_mat2.data());
    cv::Mat mat3(M, P, CV_32FC1, d_mat3.data());
    cv::Mat mat4(M, P, CV_32FC1, d_mat4.data());

    fill_random_data(d_mat1);
    fill_random_data(d_mat2);

    mat3 = mat1*mat2;

    std::ostringstream options("-cl-std=CL2.0");
    cl::Program program(context, sSource);
    program.build(options.str().c_str());

    cl::Kernel multiply(program, "multiply");

    multiply.setArg(0, d_mat4.data());
    multiply.setArg(1, d_mat1.data());
    multiply.setArg(2, d_mat2.data());
    multiply.setArg(3, M);
    multiply.setArg(4, N);
    multiply.setArg(5, P);
    cl::Event event;
    queue.enqueueNDRangeKernel(multiply, cl::NullRange, cl::NDRange(M, P), cl::NDRange(16, 16), NULL, &event);
    event.wait();

    auto success = true;
    for (auto i = 0; i < d_mat3.size() && success; i++)
    {
        float diff = d_mat3[i]-d_mat4[i];
        if (fabs(diff) > 0.01)
        {
            success = false;
            std::cout << "Failed " << d_mat3[i] << ", " << d_mat4[i] << std::endl;
        }
    }

    std::cout << "OpenCL Matrix Multiplication status: " << (success ? "Success" : "Failed");

    return 0;
}

int main(int argc, char** argv)
{
    try
    {
        return matrix_mult();
    }

    catch (const cl::Error& error)
    {
        std::cerr << "Error: " << error.what() << "(" << error.err() << ")" << std::endl;
    }

    return 0;
}

#include <vector>
#include <iostream>

#include <CL/cl.hpp>
#include <opencv2/core.hpp>

#define OCL_PROGRAM_SOURCE(s) #s

const std::string sSource = OCL_PROGRAM_SOURCE(

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

);

void fill_random_data(float* p_data, size_t size)
{
    for (auto i = 0; i < size; i++)
    {
        p_data[i] = (float)(rand()%10);
    }
}

void copy_data(const cl::CommandQueue& queue, const cv::Mat& matrix, cl::Buffer& buffer)
{
    const float* pSrc = (const float *)matrix.data;
    float* pData = (float *)queue.enqueueMapBuffer(buffer, CL_TRUE, CL_MAP_WRITE, 0, matrix.rows*matrix.cols*sizeof(float));
    for (auto i = 0; i < (matrix.rows*matrix.cols); i++)
    {
        pData[i] = pSrc[i];
    }
    queue.enqueueUnmapMemObject(buffer, pData);
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

    std::string name, version;
    devices[0].getInfo<std::string>(CL_DEVICE_NAME, &name);
    devices[0].getInfo<std::string>(CL_DEVICE_OPENCL_C_VERSION, &version);
    std::cout << name << ", " << version << std::endl;

    const int M = 512;
    const int N = 256;
    const int P = 512;

    cv::Mat mat1(M, N, CV_32FC1);
    cv::Mat mat2(N, P, CV_32FC1);
    cv::Mat mat3(M, P, CV_32FC1);

    fill_random_data((float*)mat1.data, M*N);
    fill_random_data((float*)mat2.data, N*P);

    cl::Buffer d_mat1(context, CL_MEM_READ_ONLY, M*N*sizeof(cl_float));
    cl::Buffer d_mat2(context, CL_MEM_READ_ONLY, N*P*sizeof(cl_float));
    cl::Buffer d_mat3(context, CL_MEM_READ_WRITE, M*P*sizeof(cl_float));
    
    copy_data(queue, mat1, d_mat1);
    copy_data(queue, mat2, d_mat2);

    mat3 = mat1*mat2;

    cl::Program program(context, sSource);
    program.build();

    cl::Kernel multiply(program, "multiply");

    multiply.setArg(0, d_mat3);
    multiply.setArg(1, d_mat1);
    multiply.setArg(2, d_mat2);
    multiply.setArg(3, M);
    multiply.setArg(4, N);
    multiply.setArg(5, P);
    cl::Event event;
    queue.enqueueNDRangeKernel(multiply, cl::NullRange, cl::NDRange(M, P), cl::NDRange(16, 16), NULL, &event);
    event.wait();

    auto success = true;
    float* pData1 = (float*)mat3.data;
    float* pData2 = (float *)queue.enqueueMapBuffer(d_mat3, CL_TRUE, CL_MAP_READ, 0, M*P*sizeof(float));
    for (auto i = 0; i < (M*P) && success; i++)
    {
        float diff = pData1[i]-pData2[i];
        if (fabs(diff) > 0.01)
        {
            success = false;
            std::cout << "Failed " << pData1[i] << ", " << pData2[i] << std::endl;
        }
    }
    queue.enqueueUnmapMemObject(d_mat3, pData2);

    std::cout << "OpenCL Matrix Multiplication status: " << (success ? "Success" : "Failed");

    return 0;
}

int main(int argc, char** argv)
{
    try
    {
        return matrix_mult();
    }

    catch (cl::Error error)
    {
        std::cerr << "Error: " << error.what() << "(" << error.err() << ")" << std::endl;
    }

    return 0;
}

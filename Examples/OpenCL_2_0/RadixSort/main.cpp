#include <vector>
#include <iostream>

#include "RadixSort.h"

void fill_random_data(svm_vector<cl_int>& data)
{
    for (auto i = 0; i < data.size(); i++)
    {
        data[i] = (cl_int)(rand()%(1<<31));
    }
}

int radix_sort()
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

    cl::SVMAllocator<cl_int, cl::SVMTraitFine<>> svmAlloc(context);
    svm_vector<cl_int> data(svmAlloc);

    data.resize(1024*60);
    fill_random_data(data);

    RadixSort radix(context);
    radix.sort(queue, data);
    
    auto success = true;
    for (auto i = 0; i < data.size()-1; i++)
    {
        if (data[i] > data[i+1])
        {
            std::cout << "Sort Failed: " << data[i] << ", " << data[i + 1] << " : " << i << std::endl;
            success = false;
            break;
        }
    }

    if (success)
    {
        std::cout << "Sorting Done" << std::endl;
    }

    return 0;
}

int main(int argc, char** argv)
{
    try
    {
        return radix_sort();
    }

    catch (cl::Error error)
    {
        std::cerr << "Error: " << error.what() << "(" << error.err() << ")" << std::endl;
    }

    return 0;
}

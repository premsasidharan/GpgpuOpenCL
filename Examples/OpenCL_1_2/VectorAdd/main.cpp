#include <iostream>
#include <CL/cl.hpp>

const std::string sSource =
"kernel void add(global const int* pA, global const int* pB, global int* pC)"
"{"
"    const int id = get_global_id(0);"
"    pC[id] = (pA[id] + pB[id]);"
"}";

void vectorAdd()
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.size() == 0)
    {
        std::cout << "Platform size 0\n";
        return;
    }

    for (size_t i = 0; i < platforms.size(); i++)
    {
        std::string name, vendor, version, profile;
        platforms[i].getInfo(CL_PLATFORM_NAME, &name);
        platforms[i].getInfo(CL_PLATFORM_VERSION, &version);
        platforms[i].getInfo(CL_PLATFORM_PROFILE, &profile);
        platforms[i].getInfo(CL_PLATFORM_VENDOR, &vendor);
        std::cout << i << " : " << name << "-" << version << "-" << profile << "-" << vendor << std::endl;
    }

    cl::Context context(CL_DEVICE_TYPE_GPU);

    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    cl::CommandQueue queue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

    for (size_t i = 0; i < devices.size(); i++)
    {
        std::string name;
        devices[i].getInfo<std::string>(CL_DEVICE_NAME, &name);
        std::string version;
        devices[i].getInfo<std::string>(CL_DEVICE_OPENCL_C_VERSION, &version);
        cl_uint max_cu = 0;
        devices[i].getInfo<cl_uint>(CL_DEVICE_MAX_COMPUTE_UNITS, &max_cu);
        std::cout << "Device " << i << ": " << name << ", " << version << ", Max. Compute Units: " << max_cu <<std::endl;
    }

    size_t count = 1000;
    cl::Buffer bufferA(context, CL_MEM_READ_ONLY, count*sizeof(cl_int));
    cl::Buffer bufferB(context, CL_MEM_READ_ONLY, count*sizeof(cl_int));
    cl::Buffer bufferC(context, CL_MEM_READ_WRITE, count*sizeof(cl_int));

    cl_int* pDataA = (cl_int *)queue.enqueueMapBuffer(bufferA, CL_TRUE, CL_MAP_WRITE, 0, count*sizeof(cl_int));
    cl_int* pDataB = (cl_int *)queue.enqueueMapBuffer(bufferB, CL_TRUE, CL_MAP_WRITE, 0, count*sizeof(cl_int));
    cl_int* pDataC = (cl_int *)queue.enqueueMapBuffer(bufferC, CL_TRUE, CL_MAP_WRITE, 0, count*sizeof(cl_int));
    for (size_t i = 0; i < count; i++)
    {
        pDataA[i] = rand()/100;
        pDataB[i] = rand()/100;
        pDataC[i] = 0;
    }
    queue.enqueueUnmapMemObject(bufferA, pDataA);
    queue.enqueueUnmapMemObject(bufferB, pDataB);
    queue.enqueueUnmapMemObject(bufferC, pDataC);

    cl::Program program(context, sSource);
    program.build();

    std::cout << "Adding " << count << " element vectors" << std::endl;

    cl::Kernel kernel(program, "add");
    kernel.setArg(0, bufferA);
    kernel.setArg(1, bufferB);
    kernel.setArg(2, bufferC);
    cl::Event event;
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange, NULL, &event);
    event.wait();

    bool flag = true;

    pDataA = (cl_int *)queue.enqueueMapBuffer(bufferA, CL_TRUE, CL_MAP_READ, 0, count*sizeof(cl_int));
    pDataB = (cl_int *)queue.enqueueMapBuffer(bufferB, CL_TRUE, CL_MAP_READ, 0, count*sizeof(cl_int));
    pDataC = (cl_int *)queue.enqueueMapBuffer(bufferC, CL_TRUE, CL_MAP_READ, 0, count*sizeof(cl_int));
    for (size_t i = 0; i < count && flag; i++)
    {
        flag = ((pDataA[i] + pDataB[i]) == pDataC[i]);
    }
    queue.enqueueUnmapMemObject(bufferA, pDataA);
    queue.enqueueUnmapMemObject(bufferB, pDataB);
    queue.enqueueUnmapMemObject(bufferC, pDataC);

    std::cout << "Gpu Vector Addition " << (flag?"Success":"Failed") << std::endl;
}

int main(void)
{
	try
	{
        vectorAdd();
	}

	catch (cl::Error error)
	{
		std::cerr << "Error: " << error.what() << "(" << error.err() << ")" << std::endl;
	}

	return 0;
}

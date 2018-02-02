#include <iostream>
#include <sstream>
#include <CL/cl.hpp>

const std::string sSource = R"(

inline void breduce(local volatile int* shData)
{
    int bSize = get_local_size(0);
    while (bSize > 0)
    {
        int i = get_local_id(0);
        if (i < bSize) shData[i] += shData[bSize+i];
        barrier(CLK_LOCAL_MEM_FENCE);
        bSize /= 2;
    }
}

kernel void reduceSum(global const int* pInput, global int* pOutput, const int count)
{
    local int shData[SH_MEM_SIZE];
    int i = get_local_id(0);
    int id = (SH_MEM_SIZE*get_group_id(0))+i;
    shData[i] = (id < count)?pInput[id]:0;
    i += get_local_size(0);
    id += get_local_size(0);
    shData[i] = (id < count)?pInput[id]:0;
    barrier(CLK_LOCAL_MEM_FENCE);
    breduce(shData);
    if (get_local_id(0) == 0) pOutput[get_group_id(0)] = shData[0];
}

)";

void reduceSumMain()
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.size() == 0)
    {
        std::cout << "Platform size 0\n";
        return;
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
        std::cout << "Device " << i << ": " << name << ", " << version << std::endl;
    }

    size_t count = 1000000;
    size_t gSize = count/256;
    if ((count % 256)) ++gSize;

    cl::Buffer buffInp(context, CL_MEM_READ_WRITE, count*sizeof(cl_int));
    cl::Buffer buffOut(context, CL_MEM_READ_WRITE, gSize*sizeof(cl_int));

    int sum = 0;
    cl_int* pDataInp = (cl_int *)queue.enqueueMapBuffer(buffInp, CL_TRUE, CL_MAP_WRITE, 0, count*sizeof(cl_int));
    for (size_t i = 0; i < count; i++)
    {
        int rdata = rand()%5;
        pDataInp[i] = rdata;
        sum += rdata;
    }
    queue.enqueueUnmapMemObject(buffInp, pDataInp);

    cl::Program program(context, sSource);
    std::ostringstream options;
    options << "-DSH_MEM_SIZE=" << 256;
    program.build(options.str().c_str());

    cl::Kernel reduce(program, "reduceSum");

    std::vector<cl::Event> events(100);
    std::vector< std::vector<cl::Event> > wEvents(100);

    size_t evtCount = 0;
    size_t wEvtCount = 0;

    reduce.setArg(0, buffInp);
    reduce.setArg(1, buffOut);
    reduce.setArg(2, (int)count);
    queue.enqueueNDRangeKernel(reduce, cl::NullRange, cl::NDRange(256*gSize), cl::NDRange(128), NULL, &events[evtCount]);
    
    while (gSize > 1)
    {
        reduce.setArg(0, buffOut);
        reduce.setArg(1, buffOut);
        reduce.setArg(2, (int)gSize);
        wEvents[wEvtCount].push_back(events[evtCount++]);
        size_t ngSize = (gSize/256)+(((gSize%256)!=0)?1:0);
        queue.enqueueNDRangeKernel(reduce, cl::NullRange, cl::NDRange(ngSize*256), cl::NDRange(128), &wEvents[wEvtCount++], &events[evtCount]);
        gSize = ngSize;
    }
    events[evtCount].wait();

    cl_int* pDataOut = (cl_int *)queue.enqueueMapBuffer(buffOut, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_int));
    std::cout << "Reduce Sum " << ((pDataOut[0] == sum) ? "Success" : "Failed") << " (" << sum << ", " << pDataOut[0] << ")" << std::endl;
    queue.enqueueUnmapMemObject(buffOut, pDataOut);

    size_t time = 0;
    for (size_t i = 0; i < (evtCount + 1); i++)
    {
        time += (events[i].getProfilingInfo<CL_PROFILING_COMMAND_END>() - events[i].getProfilingInfo<CL_PROFILING_COMMAND_START>());
    }
    std::cout << "Time " << time << " ns" << std::endl;
}

int main(void)
{
    try
    {
        reduceSumMain();
    }

    catch (cl::Error error)
    {
        std::cerr << "Error: " << error.what() << "(" << error.err() << ")" << std::endl;
    }

    return 0;
}

#include <iostream>
#include <sstream>
#include <CL/cl.hpp>

const std::string sSource = R"(

inline void wscan(int i, local volatile int* sh_data)
{
    const int wid = i&(WARP_SIZE-1);
    if (wid >= 1) sh_data[i] += sh_data[i-1];
    if (wid >= 2) sh_data[i] += sh_data[i-2];
    if (wid >= 4) sh_data[i] += sh_data[i-4];
    if (wid >= 8) sh_data[i] += sh_data[i-8];
    if (WARP_SIZE == 32)
    {
        if (wid >= 16) sh_data[i] += sh_data[i-16];
    }
}

inline void bscan(int i, local volatile int* shData, local volatile int* shDataT)
{
    wscan(i, shData);
    barrier(CLK_LOCAL_MEM_FENCE);

    if (i < (SH_MEM_SIZE/WARP_SIZE)) shDataT[i] = shData[(WARP_SIZE*i)+(WARP_SIZE-1)];
    barrier(CLK_LOCAL_MEM_FENCE);

    if (i < WARP_SIZE) wscan(i, shDataT);
    barrier(CLK_LOCAL_MEM_FENCE);

    if (i >= WARP_SIZE) shData[i] += shDataT[(i/WARP_SIZE)-1];
    barrier(CLK_LOCAL_MEM_FENCE);
}

kernel void scan(global const int* pInput, global int* pOutput, const int count)
{
    local int shData[SH_MEM_SIZE+WARP_SIZE];
    int i = get_local_id(0);
    int id = get_global_id(0);
    shData[i] = (id < count)?pInput[id]:0;
    bscan(i, shData, &shData[SH_MEM_SIZE]);
    if (id < count) pOutput[id] = shData[i];
}

kernel void gather(global const int* pInput, global int* pOutput, const int start, const int count)
{
    local int shData[SH_MEM_SIZE+WARP_SIZE];
    int i = get_local_id(0);
    int id = start+(get_local_size(0)*get_local_id(0));
    shData[i] = (id < count)?pInput[id]:0;
    bscan(i, shData, &shData[SH_MEM_SIZE]);
    pOutput[i] = shData[i];
}

kernel void add(global const int* pInput, global int* pOutput, const int start, const int count)
{
    local int shData;
    if (get_local_id(0) == 0) shData = pInput[get_group_id(0)];
    barrier(CLK_LOCAL_MEM_FENCE);
    const int id = start+get_global_id(0);
    if (id < count) pOutput[id] += shData;
}

)";

void prefixSumMain()
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

    size_t count = 640*480;
    cl::Buffer buffInp(context, CL_MEM_READ_WRITE, count*sizeof(cl_int));
    cl::Buffer buffOut(context, CL_MEM_READ_WRITE, count*sizeof(cl_int));
    cl::Buffer buffTemp(context, CL_MEM_READ_WRITE, 256*sizeof(cl_int));

    cl_int* pDataInp = (cl_int *)queue.enqueueMapBuffer(buffInp, CL_TRUE, CL_MAP_WRITE, 0, count*sizeof(cl_int));
    cl_int* pDataOut = (cl_int *)queue.enqueueMapBuffer(buffOut, CL_TRUE, CL_MAP_WRITE, 0, count*sizeof(cl_int));
    for (size_t i = 0; i < count; i++)
    {
        pDataInp[i] = 1;
        pDataOut[i] = 0;
    }
    queue.enqueueUnmapMemObject(buffInp, pDataInp);
    queue.enqueueUnmapMemObject(buffOut, pDataOut);

    cl::Program program(context, sSource);
    std::ostringstream options;
    options << " -DWARP_SIZE=" << 32 << " -DSH_MEM_SIZE=" << 256;
    program.build(options.str().c_str());

    cl::Kernel scan(program, "scan");
    cl::Kernel gather(program, "gather");
    cl::Kernel add(program, "add");

    size_t gSize = count/256;
    if ((count%256)) ++gSize;
    std::vector<cl::Event> events(100);
    std::vector< std::vector<cl::Event> > wEvents(100);

    size_t evtCount = 0;
    size_t wEvtCount = 0;

    scan.setArg(0, buffInp);
    scan.setArg(1, buffOut);
    scan.setArg(2, (int)count);
    queue.enqueueNDRangeKernel(scan, cl::NullRange, cl::NDRange(256*gSize), cl::NDRange(256), NULL, &events[evtCount]);
    
    for (size_t i = 255; i < count; i += (256*256))
    {
        gather.setArg(0, buffOut);
        gather.setArg(1, buffTemp);
        gather.setArg(2, (int)i);
        gather.setArg(3, (int)count);
        wEvents[wEvtCount].push_back(events[evtCount++]);
        queue.enqueueNDRangeKernel(gather, cl::NullRange, cl::NDRange(256), cl::NDRange(256), &wEvents[wEvtCount++], &events[evtCount]);
        
        add.setArg(0, buffTemp);
        add.setArg(1, buffOut);
        add.setArg(2, (int)(i+1));
        add.setArg(3, (int)count);
        wEvents[wEvtCount].push_back(events[evtCount++]);
        queue.enqueueNDRangeKernel(add, cl::NullRange, cl::NDRange(256*256), cl::NDRange(256), &wEvents[wEvtCount++], &events[evtCount]);
    }
    events[evtCount].wait();

    size_t time = 0;
    for (size_t i = 0; i < (evtCount+1); i++)
    {
        time += (events[i].getProfilingInfo<CL_PROFILING_COMMAND_END>()-events[i].getProfilingInfo<CL_PROFILING_COMMAND_START>());
    }
    std::cout << "Time " << time << " ns" << std::endl;

    bool flag = true;
    pDataOut = (cl_int *)queue.enqueueMapBuffer(buffOut, CL_TRUE, CL_MAP_READ, 0, count*sizeof(cl_int));
    for (size_t i = 0; (i < count) && flag; i++)
    {
        if (pDataOut[i] != (i+1))
        {
            flag = false;
            std::cout << "Failed " << (i+1) << ", " << pDataOut[i] << std::endl;
        }
    }
    queue.enqueueUnmapMemObject(buffOut, pDataOut);
    std::cout << "PrefixSum " << (flag ? "Success" : "Failed") << std::endl;
}

int main(void)
{
    try
    {
        prefixSumMain();
    }

    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << std::endl;
    }

    return 0;
}

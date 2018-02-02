#include "RadixSort.h"

#include <sstream>

const std::string RadixSort::sSource = R"(

kernel void block_even_odd_count(global const int* p_data, const int mask, global int* p_block_even, global int* p_block_odd)
{
    const int id = get_global_id(0);
    const int data = p_data[id];
    int even = ((data&mask) == 0) ? 1 : 0;
    int count_even = work_group_reduce_add(even);
    if (get_local_id(0) == 0)
    {
        p_block_even[get_group_id(0)] = count_even;
        p_block_odd[get_group_id(0)] = get_local_size(0) - count_even;
    }
}

kernel void compact(global const int* p_data, const int mask, global const int* p_even_offs, global const int* p_odd_offs, global int* p_out_data)
{
    local int sh_even_offs;
    local int sh_odd_offs;
    
    const int id = get_global_id(0);
    const int data = p_data[id];
    
    if (get_local_id(0) == 0)
    {
        sh_even_offs = p_even_offs[get_group_id(0)];
        sh_odd_offs = p_even_offs[get_num_groups(0)] + p_odd_offs[get_group_id(0)];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    int even = ((data&mask) == 0) ? 1 : 0;
    int even_idx = work_group_scan_exclusive_add(even);
    int odd_idx = work_group_scan_exclusive_add(1-even);

    p_out_data[even?(sh_even_offs+even_idx):(sh_odd_offs+odd_idx)] = data;
}

kernel void child_scan(global int4* pdata, int count)
{
    local int shData[1024];
    int id = get_global_id(0);
    ((int4 *)shData)[get_local_id(0)] = (id < count)?pdata[id]:0;
    barrier(CLK_LOCAL_MEM_FENCE);

    int i = get_local_id(0);
    shData[i] = work_group_scan_exclusive_add(shData[i]);
    barrier(CLK_LOCAL_MEM_FENCE);
    i += 255;
    shData[i] = work_group_scan_exclusive_add(shData[i]);
    barrier(CLK_LOCAL_MEM_FENCE);
    i += 255;
    shData[i] = work_group_scan_exclusive_add(shData[i]);
    barrier(CLK_LOCAL_MEM_FENCE);
    i += 255;
    shData[i] = work_group_scan_exclusive_add(shData[i]);
    barrier(CLK_LOCAL_MEM_FENCE);
    i += 255;
    int data = work_group_scan_exclusive_add((i < 1024) ? shData[i] : 0);
    if (i < 1024) shData[i] = data;
    barrier(CLK_LOCAL_MEM_FENCE);
    if (id < count) pdata[id] = ((int4 *)shData)[get_local_id(0)];
}

kernel void gather_scan(global const int* pdata, global int* tempData, int offset, int count)
{
    local int shData[1024];
    for (int i = 0; i < 4; i++)
    {
        int index = (4 * get_local_id(0)) + i;
        int j = offset + (1024 * index) - 1;
        shData[index] = (j < count) ? pdata[j] : 0;
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    int i = get_local_id(0);
    shData[i] = work_group_scan_exclusive_add(shData[i]);
    barrier(CLK_LOCAL_MEM_FENCE);
    i += 255;
    shData[i] = work_group_scan_exclusive_add(shData[i]);
    barrier(CLK_LOCAL_MEM_FENCE);
    i += 255;
    shData[i] = work_group_scan_exclusive_add(shData[i]);
    barrier(CLK_LOCAL_MEM_FENCE);
    i += 255;
    shData[i] = work_group_scan_exclusive_add(shData[i]);
    barrier(CLK_LOCAL_MEM_FENCE);
    i += 255;
    int data = work_group_scan_exclusive_add((i < 1024) ? shData[i] : 0);
    if (i < 1024) shData[i] = data;
    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = 0; i < 4; i++)
    {
        int index = (4 * get_local_id(0)) + i;
        tempData[index] = shData[index];
    }
}

kernel void add_data(global int4* pdata, global const int* tempData, int offset, int count)
{
    local int shData;
    if (get_local_id(0) == 0)
    {
        shData = tempData[get_group_id(0)];
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    int i = (256 * get_group_id(0)) + get_local_id(0) + (offset / 4);
    int4 data = pdata[i];
    data += shData;
    pdata[i] = data;
}

kernel void scan(global int* pdata, global int* tempData, int count)
{
    clk_event_t event1;
    clk_event_t event2;
    const int BLK_SIZE = 256;
    const int BLK_SIZE2 = (BLK_SIZE*BLK_SIZE);
    int ncount = count / 4;
    int gcount = ((ncount / BLK_SIZE) + (((ncount%BLK_SIZE) == 0) ? 0 : 1))*BLK_SIZE;
    enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(gcount, BLK_SIZE), 0, 0, &event1, ^{ child_scan(pdata, ncount); });
    for (int i = (4 * BLK_SIZE); i < count; i += (16 * BLK_SIZE2))
    {
        enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(BLK_SIZE, BLK_SIZE), 1, &event1, &event2, ^{ gather_scan(pdata, tempData, i, count); });
        enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D((4 * BLK_SIZE2), BLK_SIZE), 1, &event2, &event1, ^{ add_data(pdata, tempData, i, count); });
    }
    release_event(event1);
}

kernel void radix_sort(global int* p_in_data, global int* p_temp_data, int count)
{
    clk_event_t event[4];
    const int BLK_SIZE = 256;
    int size = 4+(count/256);
    int temp_buff_size = 1024*((size/1024)+((size%1024)==0?0:1));
    global int* p_odd_blk_count = p_temp_data + temp_buff_size;
    global int* p_even_blk_count = p_odd_blk_count + size;
    global int* p_out_data = p_even_blk_count + size;

    unsigned int mask = 0x0001;
    for (int i = 0; mask != 0; i++)
    {
        global int* p_input_data = ((i%2) == 0)?p_in_data:p_out_data;
        global int* p_output_data = ((i%2) == 0)?p_out_data:p_in_data;
        enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(count, BLK_SIZE), (i==0)?0:1, (i==0)?0:&event[3], &event[0], ^{ block_even_odd_count(p_input_data, mask, p_even_blk_count, p_odd_blk_count); });
        enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, 1), 1, &event[0], &event[1], ^{ scan(p_even_blk_count, p_temp_data, size); });
        enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1, 1), 1, &event[0], &event[2], ^{ scan(p_odd_blk_count, p_temp_data, size); });
        enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(count, BLK_SIZE), 2, &event[1], &event[3], ^{ compact(p_input_data, mask, p_even_blk_count, p_odd_blk_count, p_output_data); });
        mask = mask << 1;
    }
    release_event(event[3]);
}

)";

RadixSort::RadixSort(const cl::Context& ctxt)
    :mProgram(ctxt, sSource),
     mCtxt(ctxt)
{
    std::ostringstream options("-cl-std=CL2.0");
    mProgram.build(options.str().c_str());
    mRadix = cl::Kernel(mProgram, "radix_sort");
}

RadixSort::~RadixSort()
{
}

void RadixSort::sort(cl::CommandQueue& queue, svm_vector<cl_int>& data)
{
    int even_odd_size = 4+(data.size()/256);
    int scan_temp_size = 1024*((even_odd_size/1024)+((even_odd_size%1024)==0?0:1));
    int temp_size = data.size()+(even_odd_size*2)+scan_temp_size;

    cl::SVMAllocator<cl_int, cl::SVMTraitFine<>> svmAlloc(mCtxt);
    svm_vector<cl_int> tempData(temp_size, 0, svmAlloc);
    
    mRadix.setArg(0, data.data());
    mRadix.setArg(1, tempData.data());
    mRadix.setArg(2, (cl_int)data.size());

    cl::Event event;
    queue.enqueueNDRangeKernel(mRadix, cl::NullRange, cl::NDRange(1), cl::NullRange, NULL, &event);
    event.wait();
}

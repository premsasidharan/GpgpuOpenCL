#pragma once

#include <CL/cl.hpp>

namespace Ocl
{

template <typename Type>
class DataBuffer
{
public:
    DataBuffer(const cl::Context& context, cl_mem_flags flags, size_t size, void* host_ptr = NULL)
        :mBuffer(context, flags, sizeof(Type)*size, host_ptr) {};
    DataBuffer() {};
    ~DataBuffer() {};

public:
    Type* map(const cl::CommandQueue& queue, cl_bool blocking, cl_map_flags flags, size_t offset, size_t size)
    {
        return (Type *)queue.enqueueMapBuffer(mBuffer, blocking, flags, offset * sizeof(Type), size * sizeof(Type));
    }

    void unmap(const cl::CommandQueue& queue, Type*& pData)
    {
        queue.enqueueUnmapMemObject(mBuffer, pData);
        pData = 0;
    }

    DataBuffer operator=(const cl::Buffer& buff) { mBuffer = buff; return *this; };

    const cl::Buffer& buffer() const { return mBuffer; };

    size_t count() const
    {
        size_t size = 0;
        mBuffer.getInfo<size_t>(CL_MEM_SIZE, &size);
        size /= sizeof(Type);
        return size;
    };

    cl::Buffer createSubBuffer(cl_mem_flags flags, size_t origin, size_t size)
    {
        cl_buffer_region region = { origin*sizeof(Type), size*sizeof(Type) };
        return mBuffer.createSubBuffer(flags, CL_BUFFER_CREATE_TYPE_REGION, &region);
    }

private:
    DataBuffer(const DataBuffer& dataBuff)
    {
        mBuffer = dataBuff.buffer();
    }

private:
    cl::Buffer mBuffer;
};

};

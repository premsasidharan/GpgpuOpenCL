#pragma once

#include "OclDataBuffer.h"
#include <memory>

namespace Ocl
{

class CompactPrv;

class Compact
{
public:
    Compact(const cl::Context& ctxt);
    ~Compact();

    void process(const cl::CommandQueue& queue, const cl::Image& inpImage, Ocl::DataBuffer<cl_int2>& coords,
                   float threshold, Ocl::DataBuffer<cl_int>& count, std::vector<cl::Event>& event,
                   std::vector<cl::Event>* pWaitEvent = 0);

private:
    std::unique_ptr<Ocl::CompactPrv> mPrv;
};

};

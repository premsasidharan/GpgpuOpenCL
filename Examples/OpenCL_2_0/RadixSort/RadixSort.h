#pragma once

#include <vector>
#include <string>
#include <CL/cl2.hpp>

template<class T>
using svm_vector = std::vector<T, cl::SVMAllocator<T, cl::SVMTraitFine<>>>;

class RadixSort
{
public:
    RadixSort(const cl::Context& ctxt);
    ~RadixSort();

    void sort(cl::CommandQueue& queue, svm_vector<cl_int>& data);

private:
    cl::Kernel mRadix;
    cl::Program mProgram;
    const cl::Context& mCtxt;
    static const std::string sSource;
};
#pragma once

#include "OglImage.h"
#include "OglImagePainter.h"

#include "OglBuffer.h"
#include "OglBuffer.h"
#include "OglPainter.h"
#include "OglShape.h"
#include "OclDataBuffer.h"
#include "OglHistogramPainter.h"

class OglView
{
public:
    OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue);
    ~OglView();

public:
    void draw(uint8_t* pData);
    void resize(GLsizei w, GLsizei h);

private:
    void computeHistogram(const Ogl::Image<GL_RED>& image);

private:
    cl::Context& mCtxtCL;
    cl::CommandQueue& mQueueCL;

    Ogl::Image<GL_BGR> mBgrImg;
    Ogl::Image<GL_RED> mLumaImg;
    Ocl::DataBuffer<cl_int> mHistData;

    cl::Kernel mInitKernel;
    cl::Kernel mHistKernel;
    cl::Program mProgram;

    HistogramPainter mHistPainter;
    Ogl::ImagePainter<Ogl::LumaShader, Ogl::IImage> mLumaPainter;
};

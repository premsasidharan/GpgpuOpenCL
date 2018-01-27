#pragma once

#include "OglImage.h"
#include "OglImagePainter.h"

#include "OglBuffer.h"
#include "OglBuffer.h"
#include "OglPainter.h"
#include "OglShape.h"

#include "OclCompact.h"
#include "OglColorShader.h"

class OglView
{
public:
    OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue);
    ~OglView();

public:
    void draw(uint8_t* pData);
    void resize(GLsizei w, GLsizei h);
    void thresholdUp();
    void thresholdDown();

private:
    void drawCorners();
    void harrisCorner();

    void eigen(const cl::Image& inpImg, cl::Image& outImg);
    void gradient(const cl::Image& inpImg, cl::Image& outImg);
    void suppress(const cl::Image& inpImg, cl::Image& outImg, float value);

private:
    float mRvalue;
    GLsizei mWidth;
    GLsizei mHeight;
    cl::Context& mCtxtCL;
    cl::CommandQueue& mQueueCL;

    Ogl::Buffer mPointBuff;
    Ogl::Image<GL_BGR> mBgrImg;
    Ogl::Image<GL_RED> mGrayImg;

    Ocl::DataBuffer<cl_int2> mCorners;
    Ocl::DataBuffer<cl_int> mCornerCount;

    cl::Program mPgm;

    cl::Kernel mGradKernel;
    cl::Kernel mEigenKernel;
    cl::Kernel mCornerKernel;
    cl::Kernel mCoordKernel;

    cl::Image2D mIxIyImg;
    cl::Image2D mEigenImg;
    cl::Image2D mCornerImg;

    Ocl::Compact mCompact;
    std::vector<cl::Event> mEvents;
    std::vector<cl::Event> mWaitEvent[3];

    static const std::string sSource;

    Ogl::Painter<Ogl::ColorShader> mCrossPainter;
    Ogl::ImagePainter< Ogl::RgbaShader, Ogl::Image<GL_BGR> > mBgrPainter;
};

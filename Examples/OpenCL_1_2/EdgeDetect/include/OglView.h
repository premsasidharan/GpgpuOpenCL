#pragma once

#include "OglImage.h"
#include "OglImagePainter.h"

#include "OglBuffer.h"
#include "OglBuffer.h"
#include "OglPainter.h"
#include "OglShape.h"

#include <CL/cl.hpp>

class OglView
{
public:
    OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue);
    ~OglView();

public:
    void draw(uint8_t* pData);
    void resize(GLsizei w, GLsizei h);

    void minThresholdUp();
    void minThresholdDown();
    void maxThresholdUp();
    void maxThresholdDown();

private:
    void gradient(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg);
    void gaussSmooth(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg);
    void nonMaxEdgeSuppress(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg);
    void binaryThreshold(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg, float minThresh, float maxThresh);
    void cannyEdge(const cl::CommandQueue& queue, const cl::ImageGL& inpImage, cl::ImageGL& outImage, float minThresh, float maxThresh);

private:
    GLsizei mWidth;
    GLsizei mHeight;
    float mMinThresh;
    float mMaxThresh;
    cl::Context& mCtxtCL;
    cl::CommandQueue& mQueueCL;

    Ogl::Image<GL_BGR> mBgrImg;
    Ogl::Image<GL_RED> mGrayImg;
    Ogl::Image<GL_RED> mEdgeImg;

    cl::Image2D mGradImg;
    cl::Image2D mGaussImg;
    cl::Image2D mNmesImg;

    cl::Program mPgm;

    cl::Kernel mGradKernel;
    cl::Kernel mGaussKernel;
    cl::Kernel mNmesKernel;
    cl::Kernel mBinThreshKernel;

    cl::Event mEvent[4];
    std::vector<cl::Event> mWaitEvent[3];

    static const std::string sSource;

    Ogl::ImagePainter< Ogl::RgbaShader, Ogl::Image<GL_BGR> > mRgbaPainter;
    Ogl::ImagePainter< Ogl::LumaShader, Ogl::Image<GL_RED> > mGrayPainter;
};

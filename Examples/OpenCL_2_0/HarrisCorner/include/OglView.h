#pragma once

#include "OglImage.h"
#include "OglImagePainter.h"

#include "OglBuffer.h"
#include "OglBuffer.h"
#include "OglPainter.h"
#include "OglShape.h"
#include "OglColorShader.h"

#include <CL/cl2.hpp>

typedef struct
{
    cl_float rvalue;
    cl_int cornerCount;
    cl_int maxCornerCount;
} CornerParams;

template<class T>
using svm_vector = std::vector<T, cl::SVMAllocator<T, cl::SVMTraitFine<>>>;

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
    void init();
    void drawCorners();
    void compact(const cl::Image& inpImg);
    void detectCorners(const cl::ImageGL& inpImg);
    void eigen(const cl::Image& inpImg, cl::Image& outImg);
    void gradient(const cl::Image& inpImg, cl::Image& outImg);
    void suppress(const cl::Image& inpImg, cl::Image& outImg, float value);

private:
    GLsizei mWidth;
    GLsizei mHeight;
    cl::Context& mCtxtCL;
    cl::CommandQueue& mQueueCL;

    Ogl::Image<GL_BGR> mBgrImg;
    Ogl::Image<GL_RED> mGrayImg;

    Ogl::Buffer mPointBuff;
    Ogl::Painter<Ogl::ColorShader> mPainter;

    cl::Program mProgram;

    cl::Kernel mGradKernel;
    cl::Kernel mEigenKernel;
    cl::Kernel mCrossKernel;
    cl::Kernel mCornerKernel;
    cl::Kernel mCompactKernel;

    cl::Image2D mIxIyImg;
    cl::Image2D mEigenImg;
    cl::Image2D mCornerImg;

    std::vector<cl::Event> mEvents;
    std::vector<cl::Event> mWaitEvent[3];

    svm_vector<cl_int> mTempBuffer;
    svm_vector<cl_int2> mCornerData;
    svm_vector<unsigned char> mCornerParamData;

    CornerParams& mCornerParam;

    Ogl::ImagePainter< Ogl::RgbaShader, Ogl::IImage > mRgbaPainter;

    static const std::string sSource;
};

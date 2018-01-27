#include <CL/cl2.hpp>

#include "OglView.h"
#include "OglImageFormat.h"

#include <sstream>

#define MAX_CORNER_COUNT 10000

OglView::OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue)
    :mWidth(w),
     mHeight(h),
     mCtxtCL(ctxt),
     mQueueCL(queue),
     mBgrImg(w, h, GL_RGB, GL_UNSIGNED_BYTE),
     mGrayImg(w, h, GL_R32F, GL_FLOAT),
     mPointBuff(GL_ARRAY_BUFFER, (8 * MAX_CORNER_COUNT * sizeof(GLfloat)), 0, GL_DYNAMIC_DRAW),
     mIxIyImg(mCtxtCL, CL_MEM_READ_WRITE, cl::ImageFormat(CL_RG, CL_FLOAT), w, h),
     mEigenImg(mCtxtCL, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_FLOAT), w, h),
     mCornerImg(mCtxtCL, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_FLOAT), w, h),
     mTempBuffer(sizeof(CornerParams)+256+((w/32)*(h/8)), 0, cl::SVMAllocator<cl_int, cl::SVMTraitFine<>>(mCtxtCL)),
     mCornerData(cl::SVMAllocator<cl_int2, cl::SVMTraitFine<>>(mCtxtCL)),
     mCornerParamData(sizeof(CornerParams), 0, cl::SVMAllocator<cl_int, cl::SVMTraitFine<>>(mCtxtCL)),
     mCornerParam(*((CornerParams *)mCornerParamData.data()))
{
    init();

    mCornerData.resize(MAX_CORNER_COUNT);
    mCornerParam.rvalue = 0.00125f;
    mCornerParam.cornerCount = 0;
    mCornerParam.maxCornerCount = MAX_CORNER_COUNT;

    mPainter.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    mEvents.reserve(1024);
}

OglView::~OglView()
{
}

void OglView::init()
{
    try
    {
        std::ostringstream options;
        options << "-cl-std=CL2.0 -DBLK_SIZE_X=" << 32 << " -DBLK_SIZE_Y=" << 8;

        mProgram = cl::Program(mCtxtCL, sSource);
        mProgram.build(options.str().c_str());

        mGradKernel = cl::Kernel(mProgram, "gradient");
        mEigenKernel = cl::Kernel(mProgram, "eigen");
        mCornerKernel = cl::Kernel(mProgram, "nms");
        mCrossKernel = cl::Kernel(mProgram, "extractCoords");
        mCompactKernel = cl::Kernel(mProgram, "compact");
    }

    catch (cl::Error err)
    {
        printf("\nFailed: %s", err.what());
        exit(0);
    }
}

void OglView::gradient(const cl::Image& inpImg, cl::Image& outImg)
{
    mGradKernel.setArg(0, inpImg);
    mGradKernel.setArg(1, outImg);
    mEvents.resize(1 + mEvents.size());
    mQueueCL.enqueueNDRangeKernel(mGradKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), NULL, &mEvents.back());
}

void OglView::eigen(const cl::Image& inpImg, cl::Image& outImg)
{
    mEigenKernel.setArg(0, inpImg);
    mEigenKernel.setArg(1, outImg);
    mWaitEvent[0] = { mEvents.back() };
    mEvents.resize(1 + mEvents.size());
    mQueueCL.enqueueNDRangeKernel(mEigenKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), &mWaitEvent[0], &mEvents.back());
}

void OglView::suppress(const cl::Image& inpImg, cl::Image& outImg, float value)
{
    mCornerKernel.setArg(0, inpImg);
    mCornerKernel.setArg(1, outImg);
    mCornerKernel.setArg(2, value);
    mWaitEvent[1] = { mEvents.back() };
    mEvents.resize(1 + mEvents.size());
    mQueueCL.enqueueNDRangeKernel(mCornerKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), &mWaitEvent[1], &mEvents.back());
}

void OglView::compact(const cl::Image& inpImg)
{
    mCompactKernel.setArg(0, inpImg);
    mCompactKernel.setArg(1, mCornerData);
    mCompactKernel.setArg(2, mTempBuffer);
    mCompactKernel.setArg(3, mCornerParamData.data());
    mEvents.resize(1 + mEvents.size());
    mQueueCL.enqueueNDRangeKernel(mCompactKernel, cl::NullRange, cl::NDRange(1), cl::NDRange(1), &mWaitEvent[2], &mEvents.back());
}

void OglView::detectCorners(const cl::ImageGL& inpImg)
{
    mEvents.resize(0);
    std::vector<cl::Memory> gl_objs = { inpImg };
    mQueueCL.enqueueAcquireGLObjects(&gl_objs);
    gradient(inpImg, mIxIyImg);
    mQueueCL.enqueueReleaseGLObjects(&gl_objs);
    eigen(mIxIyImg, mEigenImg);
    suppress(mEigenImg, mCornerImg, mCornerParam.rvalue);
    mWaitEvent[2] = { mEvents.back() };
    compact(mCornerImg);
    mEvents.back().wait();
}

void OglView::drawCorners()
{
    cl::BufferGL buffGL(mCtxtCL, CL_MEM_READ_WRITE, mPointBuff());

    cl::Event event;
    std::vector<cl::Memory> gl_objs = { buffGL };

    mCrossKernel.setArg(0, mCornerData);
    mCrossKernel.setArg(1, (int)mCornerParam.cornerCount);
    mCrossKernel.setArg(2, buffGL);
    mCrossKernel.setArg(3, (int)(mBgrImg.width()/2));
    mCrossKernel.setArg(4, (int)(mBgrImg.height()/2));
    size_t gSize = mCornerParam.cornerCount+(16-(mCornerParam.cornerCount%16));
    mQueueCL.enqueueAcquireGLObjects(&gl_objs);
    mQueueCL.enqueueNDRangeKernel(mCrossKernel, cl::NullRange, cl::NDRange(gSize), cl::NullRange, NULL, &event);
    event.wait();
    mQueueCL.enqueueReleaseGLObjects(&gl_objs);

    mPainter.draw(GL_LINES, 0, (GLsizei)(mCornerParam.cornerCount*4), mPointBuff);
}

void OglView::draw(uint8_t* pData)
{
    mBgrImg.load(pData);
    Ogl::ImageFormat::convert(mGrayImg, mBgrImg);

    cl::ImageGL inpImgGL(mCtxtCL, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, mGrayImg());
    detectCorners(inpImgGL);

    mRgbaPainter.draw(mBgrImg);

    if (mCornerParam.cornerCount > 0)
    {
        drawCorners();
    }
}

void OglView::resize(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
}

void OglView::thresholdUp()
{
    mCornerParam.rvalue *= 2.0;
}

void OglView::thresholdDown()
{
    mCornerParam.rvalue /= 2.0;
}

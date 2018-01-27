#include "OglView.h"
#include "OglImageFormat.h"
#include "OclUtils.h"

#include <sstream>

#define MAX_CORNER_COUNT 10000

OglView::OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue)
    :mRvalue(0.00125f),
     mWidth(w),
     mHeight(h),
     mCtxtCL(ctxt),
     mQueueCL(queue),
     mPointBuff(GL_ARRAY_BUFFER, (8*MAX_CORNER_COUNT*sizeof(GLfloat)), 0, GL_DYNAMIC_DRAW),
     mBgrImg(w, h, GL_RGB, GL_UNSIGNED_BYTE),
     mGrayImg(w, h, GL_R32F, GL_FLOAT),
     mCorners(mCtxtCL, CL_MEM_READ_WRITE, MAX_CORNER_COUNT),
     mCornerCount(mCtxtCL, CL_MEM_READ_WRITE, 1),
     mIxIyImg(mCtxtCL, CL_MEM_READ_WRITE, cl::ImageFormat(CL_RG, CL_FLOAT), mWidth, mHeight),
     mEigenImg(mCtxtCL, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_FLOAT), mWidth, mHeight),
     mCornerImg(mCtxtCL, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_FLOAT), mWidth, mHeight),
     mCompact(mCtxtCL)
{
    mPgm = cl::Program(mCtxtCL, sSource);

    std::ostringstream options;
    options << " -DBLK_SIZE_X=" << 16 << " -DBLK_SIZE_Y=" << 8;
    mPgm.build(options.str().c_str());

    mGradKernel = cl::Kernel(mPgm, "gradient");
    mEigenKernel = cl::Kernel(mPgm, "eigen");
    mCornerKernel = cl::Kernel(mPgm, "nms");
    mCoordKernel = cl::Kernel(mPgm, "extractCoords");

    mEvents.reserve(1024);
}

OglView::~OglView()
{
}

void OglView::draw(uint8_t* pData)
{
    mBgrImg.load(pData);
    Ogl::ImageFormat::convert(mGrayImg, mBgrImg);

    harrisCorner();

    mBgrPainter.draw(mBgrImg);
    drawCorners();
}

void OglView::drawCorners()
{
    cl::BufferGL buffGL(mCtxtCL, CL_MEM_READ_WRITE, mPointBuff());

    cl::Event event;
    std::vector<cl::Memory> gl_objs = { buffGL };

    cl_int count = 0;
    mQueueCL.enqueueReadBuffer(mCornerCount.buffer(), CL_TRUE, 0, sizeof(cl_int), &count);

    mCoordKernel.setArg(0, mCorners.buffer());
    mCoordKernel.setArg(1, count);
    mCoordKernel.setArg(2, buffGL);
    mCoordKernel.setArg(3, (int)(mWidth/2));
    mCoordKernel.setArg(4, (int)(mHeight/2));
    size_t gSize = count + (16 - (count % 16));
    mQueueCL.enqueueAcquireGLObjects(&gl_objs);
    mQueueCL.enqueueNDRangeKernel(mCoordKernel, cl::NullRange, cl::NDRange(gSize), cl::NullRange, NULL, &event);
    event.wait();
    mQueueCL.enqueueReleaseGLObjects(&gl_objs);

    mCrossPainter.draw(GL_LINES, 0, (GLsizei)(count*4), mPointBuff);
}

void OglView::resize(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
}

void OglView::thresholdUp()
{
    mRvalue *= 2.0;
}

void OglView::thresholdDown()
{
    mRvalue /= 2.0;
}

void OglView::gradient(const cl::Image& inpImg, cl::Image& outImg)
{
    mGradKernel.setArg(0, inpImg);
    mGradKernel.setArg(1, outImg);
    mEvents.resize(1+mEvents.size());
    mQueueCL.enqueueNDRangeKernel(mGradKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), NULL, &mEvents.back());
}

void OglView::eigen(const cl::Image& inpImg, cl::Image& outImg)
{
    mEigenKernel.setArg(0, inpImg);
    mEigenKernel.setArg(1, outImg);
    mWaitEvent[0] = { mEvents.back() };
    mEvents.resize(1+mEvents.size());
    mQueueCL.enqueueNDRangeKernel(mEigenKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), &mWaitEvent[0], &mEvents.back());
}

void OglView::suppress(const cl::Image& inpImg, cl::Image& outImg, float value)
{
    mCornerKernel.setArg(0, inpImg);
    mCornerKernel.setArg(1, outImg);
    mCornerKernel.setArg(2, value);
    mWaitEvent[1] = { mEvents.back() };
    mEvents.resize(1+mEvents.size());
    mQueueCL.enqueueNDRangeKernel(mCornerKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), &mWaitEvent[1], &mEvents.back());
}

void OglView::harrisCorner()
{
    cl::ImageGL imgGL(mCtxtCL, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, mGrayImg());
    mEvents.resize(0); 
    std::vector<cl::Memory> gl_objs = { imgGL };
    mQueueCL.enqueueAcquireGLObjects(&gl_objs);
    gradient(imgGL, mIxIyImg);
    mQueueCL.enqueueReleaseGLObjects(&gl_objs);
    eigen(mIxIyImg, mEigenImg);
    suppress(mEigenImg, mCornerImg, mRvalue);
    mWaitEvent[2] = { mEvents.back() };
    mCompact.process(mQueueCL, mCornerImg, mCorners, 1.0f, mCornerCount, mEvents, &mWaitEvent[2]);
    mEvents.back().wait();
    size_t time = Ocl::kernelExecTime(mQueueCL, mEvents.data(), mEvents.size());
}

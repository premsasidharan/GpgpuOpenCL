#include "OglView.h"
#include "OglImageFormat.h"
#include "OclUtils.h"

#define THRESHOLD_CHANGE 0.001f

OglView::OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue)
    :mWidth(w),
     mHeight(h),
     mMinThresh((float)(15.0/256.0)),
     mMaxThresh((float)(70.0/256.0)),
     mCtxtCL(ctxt),
     mQueueCL(queue),
     mBgrImg(w, h, GL_RGB, GL_UNSIGNED_BYTE),
     mGrayImg(w, h, GL_R32F, GL_FLOAT),
     mEdgeImg(w, h, GL_R32F, GL_FLOAT),
     mGradImg(mCtxtCL, CL_MEM_READ_WRITE, cl::ImageFormat(CL_RG, CL_FLOAT), w, h),
     mGaussImg(mCtxtCL, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_FLOAT), w, h),
     mNmesImg(mCtxtCL, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_FLOAT), w, h)
{
    mPgm = cl::Program(mCtxtCL, sSource);
    mPgm.build();

    mGradKernel = cl::Kernel(mPgm, "gradient");
    mGaussKernel = cl::Kernel(mPgm, "gauss");
    mNmesKernel = cl::Kernel(mPgm, "nmes");
    mBinThreshKernel = cl::Kernel(mPgm, "binary_threshold");

    for (size_t i = 0; i < 3; i++)
    {
        mWaitEvent[i].resize(1);
    }
}

OglView::~OglView()
{
}

void OglView::draw(uint8_t* pData)
{
    mBgrImg.load(pData);
    Ogl::ImageFormat::convert(mGrayImg, mBgrImg);

    cl::ImageGL inpImgGL(mCtxtCL, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, mGrayImg());
    cl::ImageGL outImgGL(mCtxtCL, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, mEdgeImg());
    cannyEdge(mQueueCL, inpImgGL, outImgGL, mMinThresh, mMaxThresh);

    mGrayPainter.draw(mEdgeImg);
    //Ogl::IGeometry::Rect vp = { mBgrImg.width()/2, 0, mBgrImg.width()/2, mBgrImg.height()/2 };
    //mRgbaPainter.draw(vp, mBgrImg);
}

void OglView::resize(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
}

void OglView::minThresholdUp()
{
    float thresh = mMinThresh+THRESHOLD_CHANGE;
    if (thresh < mMaxThresh)
    {
        mMinThresh = thresh;
    }
}

void OglView::minThresholdDown()
{
    float thresh = mMinThresh-THRESHOLD_CHANGE;
    if (thresh > 0.0)
    {
        mMinThresh = thresh;
    }
}

void OglView::maxThresholdUp()
{
    float thresh = mMaxThresh+THRESHOLD_CHANGE;
    if (thresh < 1.0)
    {
        mMaxThresh = thresh;
    }
}

void OglView::maxThresholdDown()
{
    float thresh = mMaxThresh-THRESHOLD_CHANGE;
    if (thresh > mMinThresh)
    {
        mMaxThresh = thresh;
    }
}

void OglView::gradient(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg)
{
    mGradKernel.setArg(0, inpImg);
    mGradKernel.setArg(1, outImg);
    mWaitEvent[0][0] = mEvent[0];
    queue.enqueueNDRangeKernel(mGradKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), &mWaitEvent[0], &mEvent[1]);
}

void OglView::gaussSmooth(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg)
{
    mGaussKernel.setArg(0, inpImg);
    mGaussKernel.setArg(1, outImg);
    queue.enqueueNDRangeKernel(mGaussKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), 0, &mEvent[0]);
}

void OglView::nonMaxEdgeSuppress(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg)
{
    mNmesKernel.setArg(0, inpImg);
    mNmesKernel.setArg(1, outImg);
    mWaitEvent[1][0] = mEvent[1];
    queue.enqueueNDRangeKernel(mNmesKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), &mWaitEvent[1], &mEvent[2]);
}

void OglView::binaryThreshold(const cl::CommandQueue& queue, const cl::Image& inpImg, cl::Image& outImg, float minThresh, float maxThresh)
{
    mBinThreshKernel.setArg(0, inpImg);
    mBinThreshKernel.setArg(1, outImg);
    mBinThreshKernel.setArg(2, minThresh);
    mBinThreshKernel.setArg(3, maxThresh);
    mWaitEvent[2][0] = mEvent[2];
    queue.enqueueNDRangeKernel(mBinThreshKernel, cl::NullRange, cl::NDRange(mWidth, mHeight), cl::NDRange(16, 8), &mWaitEvent[2], &mEvent[3]);
}

void OglView::cannyEdge(const cl::CommandQueue& queue, const cl::ImageGL& inpImg, cl::ImageGL& outImg, float minThresh, float maxThresh)
{
    std::vector<cl::Memory> gl_objs = { inpImg, outImg };
    queue.enqueueAcquireGLObjects(&gl_objs);
    gaussSmooth(queue, inpImg, mGaussImg);
    gradient(queue, mGaussImg, mGradImg);
    nonMaxEdgeSuppress(queue, mGradImg, mNmesImg);
    binaryThreshold(queue, mNmesImg, outImg, minThresh, maxThresh);
    queue.enqueueReleaseGLObjects(&gl_objs);
    mEvent[3].wait();
    size_t time = Ocl::kernelExecTime(queue, mEvent, 4);
}

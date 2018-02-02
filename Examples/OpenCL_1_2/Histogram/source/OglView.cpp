#include "OglView.h"
#include "OglImageFormat.h"

#include "OclUtils.h"

//Histogram kernel source
//atomic operation on global memory is very slow. Optimize Histogram operation using shared memory
const std::string sSource = R"(

kernel void init(global int* pHistData)
{
    pHistData[get_global_id(0)] = 0;
}

/*kernel void histogram(read_only image2d_t inpImg, global int* pHistData)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    int idata = (int)ceil(255.0f*read_imagef(inpImg, (int2)(x, y)).x);
    atomic_inc(&pHistData[idata]);
}*/

kernel void histogram(read_only image2d_t inpImg, global int* pHistData)
{
    local int shHistData[256];
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    int index = (get_local_size(0)*get_local_id(1)) + get_local_id(0);
    if (index < 256) shHistData[index] = 0;
    barrier(CLK_LOCAL_MEM_FENCE);

    int idata = (int)ceil(255.0f*read_imagef(inpImg, (int2)(x, y)).x);
    atomic_inc(&shHistData[idata]);
    barrier(CLK_LOCAL_MEM_FENCE);

    if (index < 256) atomic_add(&pHistData[index], shHistData[index]);
}

)";

OglView::OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue)
    :mCtxtCL(ctxt),
     mQueueCL(queue),
     mBgrImg(w, h, GL_RGBA32F, GL_UNSIGNED_BYTE),
     mLumaImg(w, h, GL_R32F, GL_FLOAT),
     mHistData(mCtxtCL, CL_MEM_READ_WRITE, 256),
     mHistPainter(mCtxtCL)
{
    mProgram = cl::Program(mCtxtCL, sSource);
    mProgram.build();
    mInitKernel = cl::Kernel(mProgram, "init");
    mHistKernel = cl::Kernel(mProgram, "histogram");

    mHistPainter.setColor(1.0f, 0.0f, 0.0f);
}

OglView::~OglView()
{
}

void OglView::computeHistogram(const Ogl::Image<GL_RED>& image)
{
    cl::ImageGL inpImgGL(mCtxtCL, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, image());

    cl::Event event[2];
    mInitKernel.setArg(0, mHistData.buffer());
    mQueueCL.enqueueNDRangeKernel(mInitKernel, cl::NullRange, cl::NDRange(256), cl::NDRange(16), NULL, &event[0]);

    mHistKernel.setArg(0, inpImgGL);
    mHistKernel.setArg(1, mHistData.buffer());
    std::vector<cl::Memory> gl_objs = { inpImgGL };
    mQueueCL.enqueueAcquireGLObjects(&gl_objs);
    std::vector<cl::Event> wList = { event[0] };
    mQueueCL.enqueueNDRangeKernel(mHistKernel, cl::NullRange, cl::NDRange(image.width(), image.height()), cl::NDRange(32, 8), &wList, &event[1]);
    event[1].wait();
    mQueueCL.enqueueReleaseGLObjects(&gl_objs);

    auto time = Ocl::kernelExecTime(mQueueCL, event, 2);
}

void OglView::draw(uint8_t* pData)
{
    mBgrImg.load(pData);
    Ogl::ImageFormat::convert(mLumaImg, mBgrImg);

    computeHistogram(mLumaImg);
    mLumaPainter.draw(mLumaImg);

    auto maxValue = mLumaImg.width()*mLumaImg.height()>>4;
    mHistPainter.draw(mQueueCL, mHistData, maxValue);
}

void OglView::resize(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
}

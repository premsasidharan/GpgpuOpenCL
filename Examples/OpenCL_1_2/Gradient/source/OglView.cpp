#include "OglView.h"
#include "OglImageFormat.h"

#include "OclUtils.h"

#define OCL_PROGRAM_SOURCE(s) #s

const std::string sSource = OCL_PROGRAM_SOURCE(
kernel void gradient(read_only image2d_t inpImg, write_only image2d_t outImg, global const float* pIx, global const float* pIy)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP|CLK_FILTER_LINEAR;

    float ix = 0.0f;
    float iy = 0.0f;
    
    for (int i = -1; i <= +1; i++)
    {
        for (int j = -1; j <= +1; j++)
        {
            int index = (3*(i+1))+(j+1);
            ix += (pIx[index]*read_imagef(inpImg, sampler, (int2)(x+j, y+i)).x);
            iy += (pIy[index]*read_imagef(inpImg, sampler, (int2)(x+j, y+i)).x);
        }
    }

    float odata = sqrt((ix*ix)+(iy*iy));
    write_imagef(outImg, (int2)(x, y), odata);
}
);

const GLfloat maskIx[] =
    {
        -1.0f, +0.0f, +1.0f,
        -2.0f, +0.0f, +2.0f,
        -1.0f, +0.0f, +1.0f
    };

const GLfloat maskIy[] = 
    {
        +1.0f, +2.0f, +1.0f,
        +0.0f, +0.0f, +0.0f,
        -1.0f, -2.0f, -1.0f
    };

OglView::OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue)
    :mCtxtCL(ctxt),
     mQueueCL(queue),
     mBgrImg(w, h, GL_RGBA32F, GL_UNSIGNED_BYTE),
     mLumaImg(w, h, GL_R32F, GL_FLOAT),
     mGradImg(w, h, GL_R32F, GL_FLOAT),
     mCoeffIx(mCtxtCL, CL_MEM_READ_ONLY, 9),
     mCoeffIy(mCtxtCL, CL_MEM_READ_ONLY, 9)
{
    mProgram = cl::Program(mCtxtCL, sSource);
    mProgram.build();
    mKernel = cl::Kernel(mProgram, "gradient");

    load(maskIx, mCoeffIx);
    load(maskIy, mCoeffIy);
}

OglView::~OglView()
{
}

void OglView::load(const GLfloat coeffs[], Ocl::DataBuffer<float>& coeffBuff)
{
    float* pData = coeffBuff.map(mQueueCL, CL_TRUE, CL_MAP_WRITE, 0, 9);
    memcpy(pData, coeffs, 9 * sizeof(float));
    coeffBuff.unmap(mQueueCL, pData);
}

void OglView::draw(uint8_t* pData)
{
    mBgrImg.load(pData);
    Ogl::ImageFormat::convert(mLumaImg, mBgrImg);

    cl::ImageGL inpImgGL(mCtxtCL, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, mLumaImg());
    cl::ImageGL outImgGL(mCtxtCL, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, mGradImg());

    std::vector<cl::Memory> gl_objs = { inpImgGL, outImgGL };

    cl::Event event;
    mKernel.setArg(0, inpImgGL);
    mKernel.setArg(1, outImgGL);
    mKernel.setArg(2, mCoeffIx.buffer());
    mKernel.setArg(3, mCoeffIy.buffer());
    mQueueCL.enqueueAcquireGLObjects(&gl_objs);
    mQueueCL.enqueueNDRangeKernel(mKernel, cl::NullRange, cl::NDRange(mBgrImg.width(), mBgrImg.height()), cl::NDRange(16, 8), NULL, &event);
    event.wait();
    mQueueCL.enqueueReleaseGLObjects(&gl_objs);

    size_t time = Ocl::kernelExecTime(mQueueCL, event);

    mLumaPainter.draw(mGradImg);
    //Ogl::IGeometry::Rect vp = { mBgrImg.width()/2, 0, mBgrImg.width()/2, mBgrImg.height()/2 };
    //mRgbaPainter.draw(vp, mBgrImg);
}

void OglView::resize(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
}

#include "OglView.h"
#include "OglImageFormat.h"


const std::string sSource = R"(

kernel void invert_color(read_only image2d_t inpImg, write_only image2d_t outImg)
{
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    float3 idata = read_imagef(inpImg, coord).xyz;
    float4 odata = (float4)(1.0f-idata.x, 1.0f-idata.y, 1.0f-idata.z, 1.0f);
    write_imagef(outImg, coord, odata);
}

)";

OglView::OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue)
    :mCtxtCL(ctxt),
     mQueueCL(queue),
     mBgrImg(w, h, GL_RGBA32F, GL_UNSIGNED_BYTE),
     mInvImg(w, h, GL_RGBA32F, GL_FLOAT)
{
    mProgram = cl::Program(mCtxtCL, sSource);
    mProgram.build();
    mKernel = cl::Kernel(mProgram, "invert_color");
}

OglView::~OglView()
{
}

void OglView::draw(uint8_t* pData)
{
    mBgrImg.load(pData);

    cl::ImageGL inpImgGL(mCtxtCL, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, mBgrImg());
    cl::ImageGL outImgGL(mCtxtCL, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, mInvImg());

    std::vector<cl::Memory> gl_objs = { inpImgGL, outImgGL };

    cl::Event event;
    mKernel.setArg(0, inpImgGL);
    mKernel.setArg(1, outImgGL);
    mQueueCL.enqueueAcquireGLObjects(&gl_objs);
    mQueueCL.enqueueNDRangeKernel(mKernel, cl::NullRange, cl::NDRange(mBgrImg.width(), mBgrImg.height()), cl::NDRange(8, 8), NULL, &event);
    event.wait();
    mQueueCL.enqueueReleaseGLObjects(&gl_objs);

    mRgbaPainter.draw(mBgrImg);
    Ogl::IGeometry::Rect vp = { mBgrImg.width()/2, 0, mBgrImg.width()/2, mBgrImg.height()/2 };
    mRgbaPainter.draw(vp, mInvImg);
}

void OglView::resize(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
}

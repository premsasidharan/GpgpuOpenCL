#pragma once

#include "OglImage.h"
#include "OglImagePainter.h"

#include "OglBuffer.h"
#include "OglBuffer.h"
#include "OglPainter.h"
#include "OglShape.h"
#include "OclDataBuffer.h"

class OglView
{
public:
    OglView(GLsizei w, GLsizei h, cl::Context& ctxt, cl::CommandQueue& queue);
    ~OglView();

public:
    void draw(uint8_t* pData);
    void resize(GLsizei w, GLsizei h);

private:
    void load(const GLfloat coeffs[], Ocl::DataBuffer<float>& coeffBuff);

private:
    cl::Context& mCtxtCL;
    cl::CommandQueue& mQueueCL;

    Ogl::Image<GL_BGR> mBgrImg;
    Ogl::Image<GL_RED> mLumaImg;
    Ogl::Image<GL_RED> mGradImg;

    Ocl::DataBuffer<float> mCoeffIx;
    Ocl::DataBuffer<float> mCoeffIy;

    cl::Kernel mKernel;
    cl::Program mProgram;

    Ogl::ImagePainter<Ogl::RgbaShader, Ogl::IImage> mRgbaPainter;
    Ogl::ImagePainter<Ogl::LumaShader, Ogl::IImage> mLumaPainter;
};

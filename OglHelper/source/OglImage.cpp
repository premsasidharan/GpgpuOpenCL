#include "OglImage.h"

using namespace Ogl;

Yuv420Image::Yuv420Image(GLsizei w, GLsizei h, const void* pData, bool bPyramid)
    :mY(w, h, GL_R32F, GL_UNSIGNED_BYTE, pData, bPyramid),
     mU(w/2, h/2, GL_R32F, GL_UNSIGNED_BYTE, (pData==0)?0:((uint8_t*)pData+(w*h)), bPyramid),
     mV(w/2, h/2, GL_R32F, GL_UNSIGNED_BYTE, (pData==0)?0:((uint8_t*)pData+(w*h)+((w*h)>>2)), bPyramid)
{
}

Yuv420Image::~Yuv420Image()
{
}

void Yuv420Image::bind(GLenum tex) const
{
    mY.bind(tex);
    mU.bind((GLenum)(tex+1));
    mV.bind((GLenum)(tex+2));
}

void Yuv420Image::unbind(GLenum tex) const
{
    mY.unbind(tex);
    mU.unbind((GLenum)(tex+1));
    mV.unbind((GLenum)(tex+2));
}

void Yuv420Image::load(const void* pData)
{
    size_t size = mY.width()*mY.height();
    uint8_t* pUdata = (uint8_t*)pData+size;
    uint8_t* pVdata = pUdata + (size >> 2);
    mY.load(pData);
    mU.load(pUdata);
    mV.load(pVdata);
}

void Yuv420Image::load(const Ogl::Buffer2D& buffer)
{
    size_t size = mY.width()*mY.height();
    mY.load(buffer, 0);
    mU.load(buffer, size);
    mV.load(buffer, (size+(size>>2)));
}

Yuy2Image::Yuy2Image(GLsizei w, GLsizei h, const void* pData, bool bPyramid)
    :mY(w, h, GL_RG32F, GL_UNSIGNED_BYTE, pData, bPyramid),
     mUV(w/2, h, GL_RGBA32F, GL_UNSIGNED_BYTE, pData, bPyramid)
{
}

Yuy2Image::~Yuy2Image()
{
}

void Yuy2Image::bind(GLenum tex) const
{
    mY.bind(tex);
    mUV.bind((GLenum)(tex+1));
}

void Yuy2Image::unbind(GLenum tex) const
{
    mY.unbind(tex);
    mUV.unbind((GLenum)(tex+1));
}

void Yuy2Image::load(const void* pData)
{
    mY.load(pData);
    mUV.load(pData);
}

void Yuy2Image::load(const Ogl::Buffer2D& buffer)
{
    mY.load(buffer, 0);
    mUV.load(buffer, 0);
}

Nv12Image::Nv12Image(GLsizei w, GLsizei h, const void* pData, bool bPyramid)
    :mY(w, h, GL_RED, GL_UNSIGNED_BYTE, pData, bPyramid),
     mUV(w, h/2, GL_RG, GL_UNSIGNED_BYTE, (pData==0)?0:((uint8_t*)pData+(w*h)), bPyramid)
{
}

Nv12Image::~Nv12Image()
{
}

void Nv12Image::bind(GLenum tex) const
{
    mY.bind(tex);
    mUV.bind((GLenum)(tex+1));
}

void Nv12Image::unbind(GLenum tex) const
{
    mY.unbind(tex);
    mUV.unbind((GLenum)(tex+1));
}

void Nv12Image::load(const void* pData)
{
    mY.load(pData);
    mUV.load((uint8_t*)pData+(mY.width()*mY.height()));
}

void Nv12Image::load(const Ogl::Buffer2D& buffer)
{
    mY.load(buffer, 0);
    mUV.load(buffer, (mY.width()*mY.height()));
}

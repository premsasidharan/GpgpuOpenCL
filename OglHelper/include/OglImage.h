#pragma once

#include "IOglImage.h"

namespace Ogl
{

template <GLenum Format>
class Image:public Ogl::IImage
{
public:
    explicit Image(GLsizei w, GLsizei h, GLint ifmt, GLenum dataType, const void* pData = 0, bool bPyramid = false):mTexture(ifmt, Format, dataType, w, h, pData, bPyramid) {};
    ~Image() {};

    GLsizei width() const { return mTexture.width(); };
    GLsizei height() const { return mTexture.height(); };

    bool isPyramid() const { return mTexture.isPyramid(); };

    GLuint operator()() const { return mTexture(); };

    void load(const void* pData) { return mTexture.load(pData); };
    void load(const Ogl::Buffer2D& buffer) { return mTexture.load(buffer); };
    void load(const Ogl::Buffer2D& buffer, size_t offset) { return mTexture.load(buffer, offset); };
    void bind(GLenum tex) const { mTexture.bind(tex); };
    void unbind(GLenum tex) const { Ogl::Texture2D::unbind(tex); };

    Image() = delete;
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

private:
    Ogl::Texture2D mTexture;
};

class Yuv420Image:public Ogl::IImage
{
public:
    explicit Yuv420Image(GLsizei w, GLsizei h, const void* pData = 0, bool bPyramid = false);
    ~Yuv420Image();

    GLsizei width() const { return mY.width(); };
    GLsizei height() const { return mY.height(); };

    const Ogl::Image<GL_RED>& yImage() { return mY; };
    const Ogl::Image<GL_RED>& uImage() { return mU; };
    const Ogl::Image<GL_RED>& vImage() { return mV; };

    void load(const void* pData);
    void load(const Ogl::Buffer2D& buffer);

    void bind(GLenum tex) const;
    void unbind(GLenum tex) const;

    Yuv420Image() = delete;
    Yuv420Image(const Yuv420Image&) = delete;
    Yuv420Image& operator=(const Yuv420Image&) = delete;

protected:
    Ogl::Image<GL_RED> mY;
    Ogl::Image<GL_RED> mU;
    Ogl::Image<GL_RED> mV;
};

class Yuy2Image:public Ogl::IImage
{
public:
    explicit Yuy2Image(GLsizei w, GLsizei h, const void* pData = 0, bool bPyramid = false);
    ~Yuy2Image();

    GLsizei width() const { return mY.width(); };
    GLsizei height() const { return mY.height(); };

    const Ogl::Image<GL_RG>& yImage() { return mY; };
    const Ogl::Image<GL_RGBA>& uvImage() { return mUV; };

    void load(const void* pData);
    void load(const Ogl::Buffer2D& buffer);

    void bind(GLenum tex) const;
    void unbind(GLenum tex) const;

    Yuy2Image() = delete;
    Yuy2Image(const Yuy2Image&) = delete;
    Yuy2Image& operator=(const Yuy2Image&) = delete;

protected:
    Ogl::Image<GL_RG> mY;
    Ogl::Image<GL_RGBA> mUV;
};

class Nv12Image:public Ogl::IImage
{
public:
    Nv12Image(GLsizei w, GLsizei h, const void* pData = 0, bool bPyramid = false);
    ~Nv12Image();

    GLsizei width() const { return mY.width(); };
    GLsizei height() const { return mY.height(); };

    const Ogl::Image<GL_RED>& yImage() { return mY; };
    const Ogl::Image<GL_RG>& uImage() { return mUV; };

    void load(const void* pData);
    void load(const Ogl::Buffer2D& buffer);

    void bind(GLenum tex) const;
    void unbind(GLenum tex) const;

    Nv12Image() = delete;
    Nv12Image(const Nv12Image&) = delete;
    Nv12Image& operator=(const Nv12Image&) = delete;

protected:
    Ogl::Image<GL_RED> mY;
    Ogl::Image<GL_RG> mUV;
};

};



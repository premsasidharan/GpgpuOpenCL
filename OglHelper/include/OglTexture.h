#pragma once

#include "glew.h"
#include <GL/GL.h>
#include "OglBuffer2D.h"

#include <cstdint>

namespace Ogl
{

class Texture2D
{
public:
    explicit Texture2D(GLint ifmt, GLenum fmt, GLenum type, GLsizei w, GLsizei h, const void* pData = 0, bool pyramid = false);
    ~Texture2D();

    Texture2D() = delete;
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    void load(const void* pData);
    void load(const Ogl::Buffer2D& buffer);
    void load(const Ogl::Buffer2D& buffer, size_t offset);

    void bind(GLenum tex) const;
    static void unbind(GLenum tex);

    GLsizei width() const { return mWidth; };
    GLsizei height() const { return mHeight; };

    GLenum type() const { return mType; };
    GLenum format() const { return mFormat; };
    GLint internalFormat() const { return mIntFormat; };
    bool isPyramid() const { return mPyramid; };

    GLuint operator()() const { return mTexture; };

private:
    void init(const void* pData);

private:
    GLenum mType;
    bool mPyramid;
    GLenum mFormat;
    GLsizei mWidth;
    GLsizei mHeight;
    GLuint mTexture;
    GLint mIntFormat;
};

};

#pragma once

#include "glew.h"
#include <GL/GL.h>

#include <cstdint>

namespace Ogl
{

class Buffer2D
{
public:
    Buffer2D(GLenum target, int width, int height, size_t size, const GLvoid* pData, GLenum usage);
    ~Buffer2D();

    GLuint operator()() const { return mBuffId; };

    int width() const { return mWidth; };
    int height() const { return mHeight; };

    void bind() const;
    void unbind() const;

    void* map(GLenum access);
    GLboolean unmap();

protected:
    int mWidth;
    int mHeight;
    GLuint mBuffId;
    GLenum mTarget;
};

};
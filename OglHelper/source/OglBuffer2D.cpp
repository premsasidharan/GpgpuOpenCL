#include "OglBuffer2D.h"

using namespace Ogl;

Buffer2D::Buffer2D(GLenum target, int width, int height, size_t size, const GLvoid* pData, GLenum usage)
    :mWidth(width),
     mHeight(height),
     mBuffId(0),
     mTarget(target)
{
    glGenBuffers(1, &mBuffId);
    glBindBuffer(target, mBuffId);
    glBufferData(target, size, pData, usage);
    glBindBuffer(target, 0);
}

Buffer2D::~Buffer2D()
{
    glDeleteBuffers(1, &mBuffId);
}

void Buffer2D::bind() const
{
    glBindBuffer(mTarget, mBuffId);
}

void Buffer2D::unbind() const
{
    glBindBuffer(mTarget, 0);
}

void* Buffer2D::map(GLenum access)
{
    return glMapBuffer(mTarget, access);
}

GLboolean Buffer2D::unmap()
{
    return glUnmapBuffer(mTarget);
}

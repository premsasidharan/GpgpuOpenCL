#pragma once

#include "OglTexture.h"

namespace Ogl
{

class FrameBuffer
{
public:
    FrameBuffer();
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    void bind(GLenum target);
    static void release(GLenum target);

private:
    GLuint mFb;
};

};

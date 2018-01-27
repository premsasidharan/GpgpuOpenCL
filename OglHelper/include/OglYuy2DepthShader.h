#pragma once

#include "IOglShader.h"

namespace Ogl
{

class Yuy2DepthShader:public Ogl::IShader
{
public:
    Yuy2DepthShader();
    ~Yuy2DepthShader();

    GLint GetAttribLocation(const GLchar* name);

protected:
    void ApplyParameters(GLenum tex);

private:
    static const std::string vsCode;
    static const std::string fsCode;
};

};

#pragma once

#include "IOglShader.h"

namespace Ogl
{

class Yuy2Shader:public Ogl::IShader
{
public:
    Yuy2Shader();
    ~Yuy2Shader();

    GLint GetAttribLocation(const GLchar* name);

protected:
    void ApplyParameters(GLenum tex);

private:
    static const std::string vsCode;
    static const std::string fsCode;
};

};

#pragma once

#include "IOglShader.h"

namespace Ogl
{

class RgbaGrayShader :public Ogl::IShader
{
public:
    RgbaGrayShader();
    ~RgbaGrayShader();

    GLint GetAttribLocation(const GLchar* name);

protected:
    void ApplyParameters(GLenum tex);

private:
    static const std::string vsCode;
    static const std::string fsCode;
};

};

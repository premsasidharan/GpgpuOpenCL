#pragma once

#include "IOglShader.h"

namespace Ogl
{

class RgbaShader:public Ogl::IShader
{
public:
	RgbaShader();
	~RgbaShader();

	GLint GetAttribLocation(const GLchar* name);

protected:
	void ApplyParameters(GLenum tex);

private:
	static const std::string vsCode;
	static const std::string fsCode;
};

};

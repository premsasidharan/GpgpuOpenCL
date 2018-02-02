#include "OglRgbaShader.h"

using namespace Ogl;

const std::string RgbaShader::vsCode = R"(
#version 150
in vec4 inVc;
in vec2 inTc;
out vec2 tc;
void main(void)
{
	gl_Position = inVc;
	tc = inTc;
}
)";

const std::string RgbaShader::fsCode = R"(
#version 150
in vec2 tc;
out vec4 fragColor;
uniform sampler2D tex;
void main(void)
{
	fragColor = texture(tex, tc).rgba;
}
)";

RgbaShader::RgbaShader()
{
	mPgm.reset(new Ogl::Program(vsCode, fsCode));
}

RgbaShader::~RgbaShader()
{
}

void RgbaShader::ApplyParameters(GLenum tex)
{
	GLint id = (tex-GL_TEXTURE0);
	mPgm->setUniform1i("tex", id);
}

GLint RgbaShader::GetAttribLocation(const GLchar* name)
{
	return mPgm->GetAttribLocation(name);
}

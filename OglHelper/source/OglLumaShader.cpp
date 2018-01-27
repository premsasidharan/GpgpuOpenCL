#include "OglLumaShader.h"

using namespace Ogl;

const std::string LumaShader::vsCode =
	"#version 150\n"
	"in vec4 inVc;\n"
	"in vec2 inTc;\n"
	"out vec2 tc;\n"
	"void main(void)\n"
	"{\n"
	"    gl_Position = inVc;\n"
	"    tc = inTc;\n"
	"}\n";

const std::string LumaShader::fsCode =
	"#version 150\n"
	"in vec2 tc;\n"
	"out vec4 fragColor;\n"
	"uniform sampler2D tex;\n"
	"\n"
	"void main(void)\n"
	"{\n"
	"   float color = texture(tex, tc).r;\n"
	"	fragColor = vec4(color, color, color, 1.0);\n"
	"}\n";

LumaShader::LumaShader()
{
	mPgm.reset(new Ogl::Program(vsCode, fsCode));
}

LumaShader::~LumaShader()
{
}

void LumaShader::ApplyParameters(GLenum tex)
{
	mPgm->setUniform1i("tex", (tex-GL_TEXTURE0));
}

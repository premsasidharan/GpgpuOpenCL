#include "OglLumaShader.h"

using namespace Ogl;

const std::string LumaShader::vsCode = R"(
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

const std::string LumaShader::fsCode = R"(
	#version 150
	in vec2 tc;
	out vec4 fragColor;
	uniform sampler2D tex;
	void main(void)
	{
	   float color = texture(tex, tc).r;
		fragColor = vec4(color, color, color, 1.0);
	}
)";

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

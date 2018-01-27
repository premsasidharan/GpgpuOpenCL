#include "OglYuy2Shader.h"

using namespace Ogl;

const std::string Yuy2Shader::vsCode = SHADER_SOURCE_CODE(
#version 150\n
in vec4 inVc;
in vec2 inTc;
out vec2 tc;
void main(void)
{
	gl_Position = inVc;
	tc = inTc;
}
);

const std::string Yuy2Shader::fsCode = SHADER_SOURCE_CODE(
#version 150\n
in vec2 tc;
out vec4 fragColor;
uniform sampler2D tex_y;
uniform sampler2D tex_uv;
const mat3 bt709 = mat3(1.164,  1.164, 1.164,
						0.0,   -0.213, 2.115,
						1.789, -0.534, 0.0);
	
void main(void)
{
    vec3 yuv = vec3(texture(tex_y, tc).r, texture(tex_uv, tc).g, texture(tex_uv, tc).a);
	yuv -= vec3(1.0/16.0, 0.5, 0.5);
	vec3 color = bt709*yuv;
	fragColor = vec4(color.rgb, 1.0);
}
);

Yuy2Shader::Yuy2Shader()
{
	mPgm.reset(new Ogl::Program(vsCode, fsCode));
}

Yuy2Shader::~Yuy2Shader()
{
}

void Yuy2Shader::ApplyParameters(GLenum tex)
{
	GLint id = (tex - GL_TEXTURE0);
	mPgm->setUniform1i("tex_y", id);
	mPgm->setUniform1i("tex_uv", id+1);
}

GLint Yuy2Shader::GetAttribLocation(const GLchar* name)
{
	return mPgm->GetAttribLocation(name);
}

#include "OglYuy2DepthShader.h"

using namespace Ogl;

const std::string Yuy2DepthShader::vsCode = R"(
#version 150
in vec4 inVc;
in vec2 inTc;
out vec2 tc;
void main(void)
{
	gl_Position = inVc;
	tc = inTc;
}
);

const std::string Yuy2DepthShader::fsCode = fsCode"(
#version 150\n
in vec2 tc;
out vec4 fragColor;
uniform sampler2D tex_y;
uniform sampler2D tex_uv;
uniform sampler2D depth;
const mat3 bt709 = mat3(1.164,  1.164, 1.164,
						0.0,   -0.213, 2.115,
						1.789, -0.534, 0.0);
	
void main(void)
{
    vec3 color;
    if (texture(depth, tc).r < 0.000001f)
    {
        color = vec3(0.0, 0.0, 0.0);
    }
    else
    {
        vec3 yuv = vec3(texture(tex_y, tc).r, texture(tex_uv, tc).g, texture(tex_uv, tc).a);
        yuv -= vec3(1.0 / 16.0, 0.5, 0.5);
        color = bt709*yuv;
    }
	fragColor = vec4(color, 1.0);
}
)";

Yuy2DepthShader::Yuy2DepthShader()
{
	mPgm.reset(new Ogl::Program(vsCode, fsCode));
}

Yuy2DepthShader::~Yuy2DepthShader()
{
}

void Yuy2DepthShader::ApplyParameters(GLenum tex)
{
	GLint id = (tex - GL_TEXTURE0);
	mPgm->setUniform1i("tex_y", id);
	mPgm->setUniform1i("tex_uv", id+1);
    mPgm->setUniform1i("depth", id+2);
}

GLint Yuy2DepthShader::GetAttribLocation(const GLchar* name)
{
	return mPgm->GetAttribLocation(name);
}

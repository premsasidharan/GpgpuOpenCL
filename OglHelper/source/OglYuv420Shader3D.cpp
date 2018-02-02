#include "OglYuv420Shader3D.h"

using namespace Ogl;

#define _USE_MATH_DEFINES
#include <math.h>

const std::string Yuv420Shader3D::vsCode = R"(
#version 150
in vec4 inVc;
in vec2 inTc;
//out vec2 tc;
out vec4 vc;
uniform mat4 mvMatrix;
uniform mat4 projMatrix;
void main(void)
{
	gl_Position = projMatrix*(mvMatrix*inVc);
    //gl_Position = (mvMatrix*inVc);
    //tc = inTc;
    vc = inVc;
}
)";

const std::string Yuv420Shader3D::fsCode = R"(
#version 150
//in vec2 tc;
in vec4 vc;
out vec4 fragColor;
uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform sampler2D tex_v;
void main(void)
{
    float la = degrees(atan(abs(vc.x), abs(vc.z)));
    if (vc.z < 0.0 && vc.x >= 0.0)
        la = 180.0 - la;
    else if (vc.z < 0.0 && vc.x < 0.0)
        la = 180.0 + la;
    else if (vc.z >= 0.0 && vc.x < 0.0)
        la = 360.0 - la;
    float lo = degrees(asin(vc.y));
    vec2 pos = vec2((360-la)/360.0, (90.0-lo)/180.0);
    float y = 1.1643*(texture(tex_y, pos).r - 0.0625);
    float u = texture(tex_u, pos).r - 0.5;
    float v = texture(tex_v, pos).r - 0.5;
    fragColor = vec4(y+1.5958*v, y-0.39173*u-0.81290*v, y+2.017*u, 1.0);

    /*float y = 1.1643*(texture(tex_y, tc).r-0.0625);
    float u = texture(tex_u, tc).r-0.5;
    float v = texture(tex_v, tc).r-0.5;
    fragColor = vec4(y+1.5958*v, y-0.39173*u-0.81290*v, y+2.017*u, 1.0);*/
}
)";

Yuv420Shader3D::Yuv420Shader3D()
{
    mScale[0] = mScale[1] = mScale[2] = 1.0;
    mTrans[0] = mTrans[1] = mTrans[2] = 0.0;
    mAngle[0] = mAngle[1] = mAngle[2] = 0.0;
	mPgm.reset(new Ogl::Program(vsCode, fsCode));
}

Yuv420Shader3D::~Yuv420Shader3D()
{
}

void Yuv420Shader3D::ApplyParameters(GLenum tex)
{
	GLint id = (tex-GL_TEXTURE0);
	mPgm->setUniform1i("tex_y", id);
    mPgm->setUniform1i("tex_u", id+1);
    mPgm->setUniform1i("tex_v", id+2);
    updateMvMatrix();
    mPgm->setUniformMatrix4fv("mvMatrix", 1, true, mvMatrix);
    mPgm->setUniformMatrix4fv("projMatrix", 1, true, projMatrix);
}

GLint Yuv420Shader3D::GetAttribLocation(const GLchar* name)
{
	return mPgm->GetAttribLocation(name);
}

void Yuv420Shader3D::scale(GLfloat sx, GLfloat sy, GLfloat sz)
{
    mScale[0] = sx;
    mScale[1] = sy;
    mScale[2] = sz;
}

void Yuv420Shader3D::rotate(GLfloat alpha, GLfloat beta, GLfloat gamma)
{
    mAngle[0] = (GLfloat)((M_PI/180.0)*alpha);
    mAngle[1] = (GLfloat)((M_PI/180.0)*beta);
    mAngle[2] = (GLfloat)((M_PI/180.0)*gamma);
}

void Yuv420Shader3D::translate(GLfloat tx, GLfloat ty, GLfloat tz)
{
    mTrans[0] = tx;
    mTrans[1] = ty;
    mTrans[2] = tz;
}

void Yuv420Shader3D::updateMvMatrix()
{
    mvMatrix[0] = mScale[0]*cos(mAngle[0])*cos(mAngle[1]);
    mvMatrix[1] = mScale[1]*(cos(mAngle[0])*sin(mAngle[1])*sin(mAngle[2])-(sin(mAngle[0])*cos(mAngle[2])));
    mvMatrix[2] = mScale[2]*(cos(mAngle[0])*sin(mAngle[1])*cos(mAngle[2])+(sin(mAngle[0])*sin(mAngle[2])));
    mvMatrix[3] = mTrans[0];

    mvMatrix[4] = mScale[0]*sin(mAngle[0])*cos(mAngle[1]);
    mvMatrix[5] = mScale[1]*(sin(mAngle[0])*sin(mAngle[1])*sin(mAngle[2])+(cos(mAngle[0])*cos(mAngle[2])));
    mvMatrix[6] = mScale[2]*(sin(mAngle[0])*sin(mAngle[1])*cos(mAngle[2])-(cos(mAngle[0])*sin(mAngle[2])));
    mvMatrix[7] = mTrans[1];

    mvMatrix[8] = -mScale[0]*sin(mAngle[1]);
    mvMatrix[9] = mScale[1]*cos(mAngle[1])*sin(mAngle[2]);
    mvMatrix[10] = mScale[2]*cos(mAngle[1])*cos(mAngle[2]);
    mvMatrix[11] = mTrans[2];

    mvMatrix[12] = 0.0;
    mvMatrix[13] = 0.0;
    mvMatrix[14] = 0.0;
    mvMatrix[15] = 1.0;
}

void Yuv420Shader3D::perspective(double fov, double asRatio, double near, double far)
{
    double top = near*tan(fov/2.0);
    double bottom = -top;
    double right = asRatio*top;
    double left = -right;
    updateProjMatrix(top, bottom, left, right, near, far);
}

void Yuv420Shader3D::updateProjMatrix(double top, double bottom, double left, double right, double near, double far)
{
    projMatrix[0] = (GLfloat)((2.0*near)/(right-left));
    projMatrix[1] = 0.0;
    projMatrix[2] = (GLfloat)((right+left)/(right-left));
    projMatrix[3] = 0.0;

    projMatrix[4] = 0.0;
    projMatrix[5] = (GLfloat)((2.0*near)/(top-bottom));
    projMatrix[6] = (GLfloat)((top+bottom)/(top-bottom));
    projMatrix[7] = 0.0;

    projMatrix[8] = 0.0;
    projMatrix[9] = 0.0;
    projMatrix[10] = (GLfloat)(-1.0*(far+near)/(far-near));
    projMatrix[11] = (GLfloat)((-2.0*far*near)/(far-near));

    projMatrix[12] = 0.0;
    projMatrix[13] = 0.0;
    projMatrix[14] = -1.0;
    projMatrix[15] = 0.0;
}

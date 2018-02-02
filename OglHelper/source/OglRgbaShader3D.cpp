#include "OglRgbaShader3D.h"

using namespace Ogl;

#define _USE_MATH_DEFINES
#include <math.h>

const std::string RgbaShader3D::vsCode = R"(
#version 150
in vec4 inVc;
in vec2 inTc;
out vec2 tc;
uniform mat4 mvMatrix;
uniform mat4 transMatrix;
void main(void)
{
    vec4 pos = inVc.xyzw;
	gl_Position = (mvMatrix*pos);
	tc = inTc;
}
)";

const std::string RgbaShader3D::fsCode = R"(
#version 150
in vec2 tc;
out vec4 fragColor;
uniform sampler2D tex;
void main(void)
{
	fragColor = texture(tex, tc).rgba;
}
)";

RgbaShader3D::RgbaShader3D()
    :mSx(1.0),
     mSy(1.0),
     mSz(1.0),
     mTx(0.0),
     mTy(0.0),
     mTz(0.0),
     mBeta(0.0),
     mAlpha(0.0),
     mGamma(0.0)
{
	mPgm.reset(new Ogl::Program(vsCode, fsCode));
}

RgbaShader3D::~RgbaShader3D()
{
}

void RgbaShader3D::ApplyParameters(GLenum tex)
{
	GLint id = (tex - GL_TEXTURE0);
	mPgm->setUniform1i("tex", id);
    updateMvMatrix();
    mPgm->setUniformMatrix4fv("mvMatrix", 1, true, mvMatrix);
}

GLint RgbaShader3D::GetAttribLocation(const GLchar* name)
{
	return mPgm->GetAttribLocation(name);
}

void RgbaShader3D::scale(GLfloat sx, GLfloat sy, GLfloat sz)
{
    mSx = sx;
    mSy = sy;
    mSz = sz;
}

void RgbaShader3D::rotate(GLfloat alpha, GLfloat beta, GLfloat gamma)
{
    mAlpha = (GLfloat)((M_PI/180.0)*alpha);
    mBeta = (GLfloat)((M_PI/180.0)*beta);
    mGamma = (GLfloat)((M_PI/180.0)*gamma);
}

void RgbaShader3D::translate(GLfloat tx, GLfloat ty, GLfloat tz)
{
    mTx = tx;
    mTy = ty;
    mTz = tz;
}

void RgbaShader3D::updateMvMatrix()
{
    mvMatrix[0] = mSx*cos(mAlpha)*cos(mBeta);
    mvMatrix[1] = mSy*(cos(mAlpha)*sin(mBeta)*sin(mGamma) - (sin(mAlpha)*cos(mGamma)));
    mvMatrix[2] = mSz*(cos(mAlpha)*sin(mBeta)*cos(mGamma) + (sin(mAlpha)*sin(mGamma)));
    mvMatrix[3] = mTx;

    mvMatrix[4] = mSx*sin(mAlpha)*cos(mBeta);
    mvMatrix[5] = mSy*(sin(mAlpha)*sin(mBeta)*sin(mGamma) + (cos(mAlpha)*cos(mGamma)));
    mvMatrix[6] = mSz*(sin(mAlpha)*sin(mBeta)*cos(mGamma) - (cos(mAlpha)*sin(mGamma)));
    mvMatrix[7] = mTy;

    mvMatrix[8] = -mSx*sin(mBeta);
    mvMatrix[9] = mSy*cos(mBeta)*sin(mGamma);
    mvMatrix[10] = mSz*cos(mBeta)*cos(mGamma);
    mvMatrix[11] = mTz;

    mvMatrix[12] = 0.0;
    mvMatrix[13] = 0.0;
    mvMatrix[14] = 0.0;
    mvMatrix[15] = 1.0;
}

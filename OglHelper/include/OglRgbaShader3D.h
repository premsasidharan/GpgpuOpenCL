#pragma once

#include "IOglShader.h"

namespace Ogl
{

class RgbaShader3D:public Ogl::IShader
{
public:
	RgbaShader3D();
	~RgbaShader3D();

	GLint GetAttribLocation(const GLchar* name);

    void scale(GLfloat sx, GLfloat sy, GLfloat sz);
    void translate(GLfloat tx, GLfloat ty, GLfloat tz);
    void rotate(GLfloat alpha, GLfloat beta, GLfloat gamma);

protected:
    void updateMvMatrix();
	void ApplyParameters(GLenum tex);

private:
    GLfloat mSx;
    GLfloat mSy;
    GLfloat mSz;
    GLfloat mTx;
    GLfloat mTy;
    GLfloat mTz;
    GLfloat mBeta;
    GLfloat mAlpha;
    GLfloat mGamma;
    GLfloat mvMatrix[16];

	static const std::string vsCode;
	static const std::string fsCode;
};

};

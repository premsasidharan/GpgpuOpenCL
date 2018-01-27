#pragma once

#include "IOglShader.h"

namespace Ogl
{

class Yuv420Shader3D:public Ogl::IShader
{
public:
    Yuv420Shader3D();
	~Yuv420Shader3D();

	GLint GetAttribLocation(const GLchar* name);
    void scale(GLfloat sx, GLfloat sy, GLfloat sz);
    void translate(GLfloat tx, GLfloat ty, GLfloat tz);
    void rotate(GLfloat alpha, GLfloat beta, GLfloat gamma);
    void perspective(double fov, double asRatio, double near, double far);

protected:
    void updateMvMatrix();
	void ApplyParameters(GLenum tex);
    void updateProjMatrix(double top, double bottom, double left, double right, double near, double far);

private:
    GLfloat mScale[3];
    GLfloat mTrans[3];
    GLfloat mAngle[3];
    GLfloat mvMatrix[16];
    GLfloat projMatrix[16];

	static const std::string vsCode;
	static const std::string fsCode;
};

};

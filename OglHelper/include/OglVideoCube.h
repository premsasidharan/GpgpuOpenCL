#pragma once

#include "OglShape.h"
#include "OglImage.h"

namespace Ogl
{

template <class Shader, GLenum ImgFormat>
class VideoCube
{
public:
    VideoCube();
    ~VideoCube();

public:
    void draw(const Ogl::Image<ImgFormat>& image);
    void scale(GLfloat sx, GLfloat sy, GLfloat sz);
    void translate(GLfloat tx, GLfloat ty, GLfloat tz);
    void rotate(GLfloat alpha, GLfloat beta, GLfloat gamma);

private:
    std::unique_ptr<Shader> mShader;
    std::unique_ptr<Ogl::Shape> mShape;
};

extern const std::vector<GLushort> index_cube;
extern const std::vector<Ogl::GeometryCoord> coords_cube;

template <class Shader, GLenum ImgFormat>
VideoCube<Shader, ImgFormat>::VideoCube()
{
    mShader.reset(new Shader);
    mShape.reset(new Ogl::Shape(mShader->GetAttribLocation("inVc"), 
        mShader->GetAttribLocation("inTc"), index_cube, coords_cube));
}

template <class Shader, GLenum ImgFormat>
VideoCube<Shader, ImgFormat>::~VideoCube()
{
}

template <class Shader, GLenum ImgFormat>
void VideoCube<Shader, ImgFormat>::draw(const Ogl::Image<ImgFormat>& image)
{
    mShape->draw(*mShader, image);
}

template <class Shader, GLenum ImgFormat>
void VideoCube<Shader, ImgFormat>::scale(GLfloat sx, GLfloat sy, GLfloat sz)
{
    mShader->scale(sx, sy, sz);
}

template <class Shader, GLenum ImgFormat>
void VideoCube<Shader, ImgFormat>::translate(GLfloat tx, GLfloat ty, GLfloat tz)
{
    mShader->translate(tx, ty, tz);
}

template <class Shader, GLenum ImgFormat>
void VideoCube<Shader, ImgFormat>::rotate(GLfloat alpha, GLfloat beta, GLfloat gamma)
{
    mShader->rotate(alpha, beta, gamma);
}

};

#pragma once

#include "OglShape.h"
#include "OglImage.h"

namespace Ogl
{

template <class Shader, class TImage>
class Sphere
{
public:
    Sphere();
    ~Sphere();

public:
    void draw(const TImage& image);
    void scale(GLfloat sx, GLfloat sy, GLfloat sz);
    void translate(GLfloat tx, GLfloat ty, GLfloat tz);
    void rotate(GLfloat alpha, GLfloat beta, GLfloat gamma);
    void perspective(double fov, double asRatio, double near, double far);

private:
    std::unique_ptr<Shader> mShader;
    std::unique_ptr<Ogl::Shape> mShape;
};

extern std::vector<GLushort> index_sphere;
extern std::vector<GeometryCoord> coords_sphere;

extern void computeSphereCoords();

template <class Shader, class TImage>
Sphere<Shader, TImage>::Sphere()
{
    mShader.reset(new Shader);
    if (index_sphere.size() == 0 || coords_sphere.size() == 0)
    {
        computeSphereCoords();
    }
    mShape.reset(new Ogl::Shape(mShader->GetAttribLocation("inVc"),
        mShader->GetAttribLocation("inTc"), index_sphere, coords_sphere));
}

template <class Shader, class TImage>
Sphere<Shader, TImage>::~Sphere()
{
}

template <class Shader, class TImage>
void Sphere<Shader, TImage>::draw(const TImage& image)
{
    mShape->draw(*mShader, image);
}

template <class Shader, class TImage>
void Sphere<Shader, TImage>::scale(GLfloat sx, GLfloat sy, GLfloat sz)
{
    mShader->scale(sx, sy, sz);
}

template <class Shader, class TImage>
void Sphere<Shader, TImage>::translate(GLfloat tx, GLfloat ty, GLfloat tz)
{
    mShader->translate(tx, ty, tz);
}

template <class Shader, class TImage>
void Sphere<Shader, TImage>::rotate(GLfloat alpha, GLfloat beta, GLfloat gamma)
{
    mShader->rotate(alpha, beta, gamma);
}

template <class Shader, class TImage>
void Sphere<Shader, TImage>::perspective(double fov, double asRatio, double near, double far)
{
    mShader->perspective(fov, asRatio, near, far);
}

}

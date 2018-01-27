#include "OglSphere.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace Ogl
{

std::vector<GLushort> index_sphere;
std::vector<Ogl::GeometryCoord> coords_sphere;

void computeSphereCoords()
{
    auto step = 5;
    auto maxAngle = 180;
    auto p = 1+(360/step);
    auto q = 1+(maxAngle/step);
    auto sphMaxSize = (p*q);
    auto sphIndexSize = (p)*(q-1)*6;

    coords_sphere.resize(sphMaxSize);
    index_sphere.resize(sphIndexSize);

    auto count = 0;
    for (auto i = -90; i <= 90; i += step)
    {
        GLfloat x = 1.0*sin(double(i)*M_PI/180.0);
        auto r = 1.0*cos(double(i)*M_PI/180.0);
        for (auto angle = 0; angle <= 360; angle += step)
        {
            GLfloat y = r*sin(double(angle)*M_PI/180.0);
            GLfloat z = r*cos(double(angle)*M_PI/180.0);
            
            coords_sphere[count].vc.x = (GLfloat)x;
            coords_sphere[count].vc.y = (GLfloat)y;
            coords_sphere[count].vc.z = (GLfloat)z;

            /*
            //Seeing patches
            auto la = (z == 0.0)?90.0:(atan(fabs(x/z))*180.0/M_PI);
            if (z < 0.0 && x >= 0.0)
            {
                la = 180.0-la;
            }
            else if (z < 0.0 && x < 0.0)
            {
                la = 180.0+la;
            }
            else if (z >= 0.0 && x < 0.0)
            {
                la = 360.0-la;
            }
            auto lo = asin(y)*180.0/M_PI;
            coords_sphere[count].tc.x = (GLfloat)((360.0-la)/360.0);
            coords_sphere[count].tc.y = (GLfloat)((90.0-lo)/180.0);*/
            ++count;
        }
    }

    for (auto i = 0; i < q-1; i++)
    {
        auto m = i*(p)*6;
        for (auto j = 0; j < p; j++)
        {
            index_sphere[m] = (unsigned short)((p*i)+j);
            index_sphere[m+1] = index_sphere[m]+1;
            index_sphere[m+2] = index_sphere[m]+(unsigned short)p;
            index_sphere[m+3] = index_sphere[m+1];
            index_sphere[m+4] = index_sphere[m+2]+1;
            index_sphere[m+5] = index_sphere[m+2];
            m += 6;
        }
    }
}

}

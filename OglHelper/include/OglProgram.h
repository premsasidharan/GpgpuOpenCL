#pragma once

#include "glew.h"
#include <GL/GL.h>
#include <string>

namespace Ogl
{

class Program
{
public:
    explicit Program(const std::string& vsSrc, const std::string& fsSrc);
    ~Program();

    Program() = delete;
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

public:
    void use() { glUseProgram(mProgram); };
    void release() { glUseProgram(0); };

    void setUniform1i(const GLchar* name, GLint value);
    void setUniform1f(const GLchar* name, GLfloat value);
    void setUniform4f(const GLchar* name, GLfloat value1, GLfloat value2, GLfloat value3, GLfloat value4);
    void setUniformMatrix4fv(const GLchar* name, GLsizei count, GLboolean transpose, const GLfloat *value);

    GLint GetAttribLocation(const GLchar* name) { return glGetAttribLocation(mProgram, name); };
    GLint GetFragDataLocation(const GLchar* name) { return glGetFragDataLocation(mProgram, name); };

private:
    void cleanup();
    void compileShader(GLuint shader);

private:
    GLuint mProgram;
    GLuint mVertShader;
    GLuint mFragShader;
};

};

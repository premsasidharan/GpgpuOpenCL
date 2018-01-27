#include "OglHistogramPainter.h"

#define OCL_PROGRAM_SOURCE(s) #s

const std::string HistogramPainter::sSource = OCL_PROGRAM_SOURCE(
kernel void histogram_coords(global read_only unsigned int* p_hist_data, unsigned int max_value, global float2* coord)
{
    int i = get_global_id(0);
    coord[i].x = -1.0f + (((float)i)/128.0f);
    coord[i].y = -1.0f + (((float)p_hist_data[i]*2.0f)/(float)max_value);
};
);

HistogramPainter::HistogramPainter(const cl::Context& ctxt)
    :mContext(ctxt),
     mBuffer(GL_ARRAY_BUFFER, 512*sizeof(GLfloat), 0, GL_DYNAMIC_DRAW),
     mBufferGL(mContext, CL_MEM_READ_WRITE, mBuffer())
{
    mProgram = cl::Program(mContext, sSource);
    mProgram.build();

    mKernel = cl::Kernel(mProgram, "histogram_coords");
}

HistogramPainter::~HistogramPainter()
{
}

void HistogramPainter::setColor(GLfloat r, GLfloat g, GLfloat b)
{
    mPainter.SetColor(r, g, b, 1.0);
}

void HistogramPainter::compute(const cl::CommandQueue& queue, const Ocl::DataBuffer<cl_int> &hData, int maxValue)
{
    cl::Event event;
    std::vector<cl::Memory> gl_objs = { mBufferGL };

    mKernel.setArg(0, hData.buffer());
    mKernel.setArg(1, maxValue);
    mKernel.setArg(2, mBufferGL);

    queue.enqueueAcquireGLObjects(&gl_objs);
    queue.enqueueNDRangeKernel(mKernel, cl::NullRange, cl::NDRange(256), cl::NDRange(256), NULL, &event);
    event.wait();
    queue.enqueueReleaseGLObjects(&gl_objs);
}

void HistogramPainter::draw(const cl::CommandQueue& queue, const Ocl::DataBuffer<cl_int>& hData, int maxValue)
{
    compute(queue, hData, maxValue);
    mPainter.draw(GL_LINE_STRIP, 0, (GLsizei)hData.count(), mBuffer);
}

void HistogramPainter::draw(const Ogl::IGeometry::Rect& vp, const cl::CommandQueue& queue, const Ocl::DataBuffer<cl_int>& hData, int maxValue)
{
    compute(queue, hData, maxValue);
    GLint params[4];
    glGetIntegerv(GL_VIEWPORT, params);
    glViewport(vp.x, vp.y, vp.width, vp.height);
    mPainter.draw(GL_LINE_STRIP, 0, (GLsizei)hData.count(), mBuffer);
    glViewport(params[0], params[1], params[3], params[4]);
}

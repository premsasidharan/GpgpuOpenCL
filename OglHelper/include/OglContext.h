#pragma once

#include "stdafx.h"

#include "glew.h"
#include <GL/GL.h>

namespace Ogl
{

class WinGlContext
{
    friend class UseWinGlContext;
public:
    explicit WinGlContext(CWnd* pWnd) :m_pWnd(pWnd) { mHglRc = wglCreateContext(m_pWnd->GetDC()->m_hDC); };
    ~WinGlContext() { wglDeleteContext(mHglRc); };

    void share(const WinGlContext& glCtxt)
    {
        BOOL status =wglShareLists(mHglRc, glCtxt.mHglRc);
        if (status == FALSE)
        {
            MessageBox(m_pWnd->m_hWnd, _T("GL Context Sharing Failed"), _T("Error"), MB_OK);
        }
    };

    WinGlContext() = delete;
    WinGlContext(const WinGlContext&) = delete;
    WinGlContext& operator=(const WinGlContext&) = delete;

private:
    HGLRC mHglRc;
    CWnd* m_pWnd;
};

class UseWinGlContext
{
public:
    explicit UseWinGlContext(WinGlContext& ctxt) :mCtxt(ctxt) { wglMakeCurrent(mCtxt.m_pWnd->GetDC()->m_hDC, mCtxt.mHglRc); };
    ~UseWinGlContext() { /*SwapBuffers(mCtxt.m_pWnd->GetDC()->m_hDC);*/ wglMakeCurrent(0, 0); };

    UseWinGlContext() = delete;
    UseWinGlContext(const UseWinGlContext&) = delete;
    UseWinGlContext& operator=(const UseWinGlContext&) = delete;

private:
    WinGlContext& mCtxt;
};

};

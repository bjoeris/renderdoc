/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "gl_driver.h"
#include "gl_replay.h"

void GLReplay::CreateOutputWindowBackbuffer(OutputWindow &outwin, bool depth)
{
  if(m_pDriver == NULL)
    return;

  MakeCurrentReplayContext(m_DebugCtx);

  WrappedOpenGL &gl = *m_pDriver;

  // create fake backbuffer for this output window.
  // We'll make an FBO for this backbuffer on the replay context, so we can
  // use the replay context to do the hard work of rendering to it, then just
  // blit across to the real default framebuffer on the output window context
  gl.glGenFramebuffers(1, &outwin.BlitData.windowFBO);
  gl.glBindFramebuffer(eGL_FRAMEBUFFER, outwin.BlitData.windowFBO);

  gl.glGenTextures(1, &outwin.BlitData.backbuffer);
  gl.glBindTexture(eGL_TEXTURE_2D, outwin.BlitData.backbuffer);

  gl.glTextureImage2DEXT(outwin.BlitData.backbuffer, eGL_TEXTURE_2D, 0, eGL_SRGB8_ALPHA8,
                         outwin.width, outwin.height, 0, eGL_RGBA, eGL_UNSIGNED_BYTE, NULL);
  gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_MAX_LEVEL, 0);
  gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_MIN_FILTER, eGL_NEAREST);
  gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_MAG_FILTER, eGL_NEAREST);
  gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_WRAP_S, eGL_CLAMP_TO_EDGE);
  gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_WRAP_T, eGL_CLAMP_TO_EDGE);
  gl.glFramebufferTexture(eGL_FRAMEBUFFER, eGL_COLOR_ATTACHMENT0, outwin.BlitData.backbuffer, 0);

  if(depth)
  {
    gl.glGenTextures(1, &outwin.BlitData.depthstencil);
    gl.glBindTexture(eGL_TEXTURE_2D, outwin.BlitData.depthstencil);

    gl.glTextureImage2DEXT(outwin.BlitData.depthstencil, eGL_TEXTURE_2D, 0, eGL_DEPTH_COMPONENT24,
                           outwin.width, outwin.height, 0, eGL_DEPTH_COMPONENT, eGL_UNSIGNED_INT,
                           NULL);
    gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_MAX_LEVEL, 0);
    gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_MIN_FILTER, eGL_NEAREST);
    gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_MAG_FILTER, eGL_NEAREST);
    gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_WRAP_S, eGL_CLAMP_TO_EDGE);
    gl.glTexParameteri(eGL_TEXTURE_2D, eGL_TEXTURE_WRAP_T, eGL_CLAMP_TO_EDGE);
  }
  else
  {
    outwin.BlitData.depthstencil = 0;
  }

  outwin.BlitData.replayFBO = 0;
}

void GLReplay::InitOutputWindow(OutputWindow &outwin)
{
  if(m_pDriver == NULL)
    return;

  MakeCurrentReplayContext(&outwin);

  WrappedOpenGL &gl = *m_pDriver;

  gl.glGenVertexArrays(1, &outwin.BlitData.emptyVAO);
  gl.glBindVertexArray(outwin.BlitData.emptyVAO);

  gl.glGenFramebuffers(1, &outwin.BlitData.readFBO);
  gl.glBindFramebuffer(eGL_READ_FRAMEBUFFER, outwin.BlitData.readFBO);
  gl.glReadBuffer(eGL_COLOR_ATTACHMENT0);
}

bool GLReplay::CheckResizeOutputWindow(uint64_t id)
{
  if(id == 0 || m_OutputWindows.find(id) == m_OutputWindows.end())
    return false;

  OutputWindow &outw = m_OutputWindows[id];

  if(outw.wnd == 0)
    return false;

  int32_t w, h;
  GetOutputWindowDimensions(id, w, h);

  if(w != outw.width || h != outw.height)
  {
    outw.width = w;
    outw.height = h;

    MakeCurrentReplayContext(m_DebugCtx);

    WrappedOpenGL &gl = *m_pDriver;

    bool haddepth = false;

    gl.glDeleteTextures(1, &outw.BlitData.backbuffer);
    if(outw.BlitData.depthstencil)
    {
      haddepth = true;
      gl.glDeleteTextures(1, &outw.BlitData.depthstencil);
    }
    gl.glDeleteFramebuffers(1, &outw.BlitData.windowFBO);

    CreateOutputWindowBackbuffer(outw, haddepth);

    return true;
  }

  return false;
}

void GLReplay::BindOutputWindow(uint64_t id, bool depth)
{
  if(id == 0 || m_OutputWindows.find(id) == m_OutputWindows.end())
    return;

  OutputWindow &outw = m_OutputWindows[id];

  MakeCurrentReplayContext(m_DebugCtx);

  m_pDriver->glBindFramebuffer(eGL_FRAMEBUFFER, outw.BlitData.windowFBO);
  m_pDriver->glViewport(0, 0, outw.width, outw.height);

  m_pDriver->glFramebufferTexture(
      eGL_FRAMEBUFFER, eGL_DEPTH_ATTACHMENT,
      depth && outw.BlitData.depthstencil ? outw.BlitData.depthstencil : 0, 0);

  DebugData.outWidth = float(outw.width);
  DebugData.outHeight = float(outw.height);
}

void GLReplay::ClearOutputWindowColor(uint64_t id, FloatVector col)
{
  if(id == 0 || m_OutputWindows.find(id) == m_OutputWindows.end())
    return;

  MakeCurrentReplayContext(m_DebugCtx);

  m_pDriver->glClearBufferfv(eGL_COLOR, 0, &col.x);
}

void GLReplay::ClearOutputWindowDepth(uint64_t id, float depth, uint8_t stencil)
{
  if(id == 0 || m_OutputWindows.find(id) == m_OutputWindows.end())
    return;

  MakeCurrentReplayContext(m_DebugCtx);

  m_pDriver->glClearBufferfi(eGL_DEPTH_STENCIL, 0, depth, (GLint)stencil);
}

void GLReplay::FlipOutputWindow(uint64_t id)
{
  if(id == 0 || m_OutputWindows.find(id) == m_OutputWindows.end())
    return;

  OutputWindow &outw = m_OutputWindows[id];

  MakeCurrentReplayContext(&outw);

  WrappedOpenGL &gl = *m_pDriver;

  // go directly to real function so we don't try to bind the 'fake' backbuffer FBO.
  gl.m_Real.glBindFramebuffer(eGL_FRAMEBUFFER, 0);
  gl.glViewport(0, 0, outw.width, outw.height);

  gl.glBindFramebuffer(eGL_READ_FRAMEBUFFER, outw.BlitData.readFBO);

  gl.glFramebufferTexture2D(eGL_READ_FRAMEBUFFER, eGL_COLOR_ATTACHMENT0, eGL_TEXTURE_2D,
                            outw.BlitData.backbuffer, 0);
  gl.glReadBuffer(eGL_COLOR_ATTACHMENT0);

  gl.glEnable(eGL_FRAMEBUFFER_SRGB);

  gl.glBlitFramebuffer(0, 0, outw.width, outw.height, 0, 0, outw.width, outw.height,
                       GL_COLOR_BUFFER_BIT, eGL_NEAREST);

  SwapBuffers(&outw);
}

uint64_t GLReplay::MakeOutputWindow(WindowingData window, bool depth)
{
  OutputWindow win = m_pDriver->m_Platform.MakeOutputWindow(window, depth, m_ReplayCtx);
  if(!win.wnd)
    return 0;

  m_pDriver->m_Platform.GetOutputWindowDimensions(win, win.width, win.height);

  MakeCurrentReplayContext(&win);
  InitOutputWindow(win);
  CreateOutputWindowBackbuffer(win, depth);

  uint64_t ret = m_OutputWindowID++;

  m_OutputWindows[ret] = win;

  return ret;
}

void GLReplay::DestroyOutputWindow(uint64_t id)
{
  auto it = m_OutputWindows.find(id);
  if(id == 0 || it == m_OutputWindows.end())
    return;

  OutputWindow &outw = it->second;

  MakeCurrentReplayContext(&outw);

  WrappedOpenGL &gl = *m_pDriver;
  gl.glDeleteFramebuffers(1, &outw.BlitData.readFBO);

  m_pDriver->m_Platform.DeleteReplayContext(outw);

  m_OutputWindows.erase(it);
}

void GLReplay::GetOutputWindowDimensions(uint64_t id, int32_t &w, int32_t &h)
{
  if(id == 0 || m_OutputWindows.find(id) == m_OutputWindows.end())
    return;

  OutputWindow &outw = m_OutputWindows[id];

  m_pDriver->m_Platform.GetOutputWindowDimensions(outw, w, h);
}

bool GLReplay::IsOutputWindowVisible(uint64_t id)
{
  if(id == 0 || m_OutputWindows.find(id) == m_OutputWindows.end())
    return false;

  return m_pDriver->m_Platform.IsOutputWindowVisible(m_OutputWindows[id]);
}

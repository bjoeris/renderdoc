/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2018 Google LLC
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
//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
//
// File: main_xlib.cpp
//
//-----------------------------------------------------------------------------
// Defines the entry point that initializes and runs the serialized frame
// capture on Linux
#if defined(__linux__) && !defined(__yeti__) && !defined(__ggp__)

#include <stdio.h>
#include <string.h>
#include <stdexcept>

#include <X11/Xlib.h>

#include "gen_main.h"

int frameLoops = -1;
double accumTimeWithReset = 0;
double accumTime = 0;
double avgTimeWithReset = 0;
double avgTime = 0;
double avgFPSWithReset = 0;
double avgFPS = 0;
uint64_t frames = 0;
double avgFrameMilliseconds = 0;
uint64_t performanceCounterFrequency;
bool automated = false;
bool resourceReset = false;

Display *appDisplay;
Window appWindow;

#define RDOC_WINDOW_TITLE "RenderDoc Frame Loop"

void PostStageProgress(const char *stage, uint32_t i, uint32_t N)
{
  XStoreName(appDisplay, appWindow, StageProgressString(stage, i, N).c_str());
}

Display *CreateDisplay()
{
  Display *res = XOpenDisplay(nullptr);
  if(!res)
  {
    throw std::runtime_error("Failed to open appDisplay");
  }
  return res;
}

Window CreateWindow()
{
  int screen = DefaultScreen(appDisplay);
  Window window = XCreateSimpleWindow(
      appDisplay, DefaultRootWindow(appDisplay), 0, 0, resolutionWidth, resolutionHeight, 0,
      BlackPixel(appDisplay, screen), WhitePixel(appDisplay, screen));
  XMapWindow(appDisplay, window);
  XStoreName(appDisplay, window, RDOC_WINDOW_TITLE);
  XFlush(appDisplay);

  return window;
}

void CreateResources()
{
  appDisplay = CreateDisplay();
  appWindow = CreateWindow();

  main_create();
  main_init();
}

//-----------------------------------------------------------------------------
// ReleaseResources
//-----------------------------------------------------------------------------
void ReleaseResources()
{
  main_release();
}

void Render()
{
  main_prereset();
  main_render();
  main_postreset();
}

//-----------------------------------------------------------------------------
// ParseCommandLine
//-----------------------------------------------------------------------------
static bool ParseCommandLine(int argc, char **argv)
{
  for(int i = 1; i < argc; ++i)
  {
    if(0 == strcmp(argv[i], "-repeat"))
    {
      ++i;
      if(i >= argc)
      {
        return false;
      }
      frameLoops = atoi(argv[i]);
    }
    else if(0 == strcmp(argv[i], "-reset"))
    {
      resourceReset = true;
    }
    else
    {
      // Unknown command
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
// Usage
//-----------------------------------------------------------------------------
static void Usage()
{
  const char usage[] =
      "Options:\n"
      "-repeat N    -- Number of frames to run\n"
      "-reset       -- Perform a state reset in between frames\n";

  fprintf(stderr, "%s", usage);
}

int main(int argc, char **argv)
{
  bool quit = false;

  if(!ParseCommandLine(argc, argv))
  {
    Usage();
    return EXIT_FAILURE;
  }

  try
  {
    CreateResources();

    // TODO QueryPerformanceFrequency(&performanceCounterFrequency);

    int repeatIteration = 0;
    while(frameLoops == -1 || repeatIteration < frameLoops)
    {
      // TODO
      // ProcessMessages(quit);
      if(quit || main_should_quit_now())
      {
        break;
      }

      Render();

      repeatIteration = (std::max)(0, repeatIteration + 1);
    }
  }
  catch(std::exception &e)
  {
    fprintf(stderr, "Error: %s", e.what());
  }

  ReleaseResources();
}

#endif

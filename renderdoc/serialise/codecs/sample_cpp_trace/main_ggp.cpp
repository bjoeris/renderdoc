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
// File: main_ggp.cpp
//-----------------------------------------------------------------------------
// Defines the entry point that initializes and runs the serialized frame
// capture on GGP
#if defined(__yeti__) || defined(__ggp__)

#if defined(__yeti__)
#include <yeti_c/yeti.h>
#define GgpEventQueue YetiEventQueue
#define GgpEventQueueCreate YetiEventQueueCreate
#define GgpEventQueueProcessEvent YetiEventQueueProcessEvent
#define GgpSuspended YetiSuspended
#define GgpStreamStoppedEvent YetiStreamStoppedEvent
#define GgpShutDown YetiShutDown
#define GgpRemoveStreamStoppedHandler YetiRemoveStreamStoppedHandler
#define GgpRemoveStreamStartedHandler YetiRemoveStreamStartedHandler
#define GgpEventQueueDestroy YetiEventQueueDestroy
#define GgpReadyToStream YetiReadyToStream
#define GgpHandlersRegistered YetiHandlersRegistered
#define GgpAddStreamStoppedHandler YetiAddStreamStoppedHandler
#define GgpAddStreamStartedHandler YetiAddStreamStartedHandler
#define kGgpStreamStopped_Exited kYetiStreamStopped_Exited
#define kGgpStreamStopped_Unexpected kYetiStreamStopped_Unexpected
#endif

#if defined(__ggp__)
#include <ggp_c/ggp.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <sstream>
#include <string>

#include "gen_main.h"

int frameLoops = -1;
double accumTimeWithReset = 0;
double accumTime = 0;
double avgTimeWithReset = 0;
double avgTime = 0;
double avgFPSWithReset = 0;
double avgFPS = 0;
uint64_t frames = 0;

void PostStageProgress(const char *stage, uint32_t i, uint32_t N)
{
  fprintf(stdout, "%s\n", StageProgressString(stage, i, N).c_str());
}

void CreateResources();
void ReleaseResources();
void Render();

void CreateResources()
{
  main_create();
  main_init();
}

void ReleaseResources()
{
  main_release();
}

static inline double GetTimestampMilliseconds()
{
  struct timespec now = {};
  clock_gettime(CLOCK_MONOTONIC_RAW, &now);
  double nanoseconds = (now.tv_sec * 1000000000.0) + now.tv_nsec;
  double microseconds = nanoseconds / 1000.0;
  return microseconds / 1000.0;
}

void Render()
{
  double ts_pre_reset = GetTimestampMilliseconds();
  main_prereset();
  double ts_start = GetTimestampMilliseconds();
  main_render();
  main_postreset();
  double ts_end = GetTimestampMilliseconds();
  double frame_time = ts_end - ts_start;
  double frame_time_with_reset = ts_end - ts_pre_reset;

  frames++;

  accumTimeWithReset += frame_time_with_reset;
  accumTime += frame_time;
  avgTimeWithReset = accumTimeWithReset / frames;
  avgTime = accumTime / frames;
  avgFPSWithReset = 1000.0 / avgTimeWithReset;
  avgFPS = 1000.0 / avgTime;

  if(frames % 100 == 0)
  {
    char str[256];
    sprintf(str, "%s Avg Time [%f / %f] Avg FPS [%f /%f]\n", "RenderDoc Frame Loop",
            avgTimeWithReset, avgTime, avgFPSWithReset, avgFPS);
    fprintf(stdout, "%s", str);
  }
}

//-----------------------------------------------------------------------------
// ParseCommandLine
//-----------------------------------------------------------------------------
static bool ParseCommandLine(int argc, char **argv)
{
  int i = 1;
  while(i < argc)
  {
    if(!main_parse_command_line_flags(argc, argv, &i))
      return false;
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

const uint64_t kMicrosecondsPerFrame = 16666L;

static struct
{
  // GGP
  GgpEventQueue event_queue;
  uint32_t stream_started_handler_id;
  uint32_t stream_stopped_handler_id;

  // General
  bool quit;
} app_data = {0};

// HandleStreamStarted(): Client connected handler
static void HandleStreamStarted(void *user_data)
{
  fprintf(stdout, "client connected\n");
}

// HandleStreamStopped(): Client disconnected handler
static void HandleStreamStopped(const GgpStreamStoppedEvent *event, void *user_data)
{
  fprintf(stdout, "client disconnected\n");

  // Exit the application if the client disconnects to exit.
  if(event->stream_stopped_reason == kGgpStreamStopped_Exited)
  {
    app_data.quit = true;
  }
  else if(event->stream_stopped_reason == kGgpStreamStopped_Unexpected)
  {
    GgpSuspended();
  }
}

// UnregisterCallback(): Handler unregistered
static void UnregisterCallback(void *user_data)
{
  fprintf(stdout, "unregistered callback\n");
}

// Initialize(): Initialize application
static void Initialize()
{
  // Initialize event queue
  app_data.event_queue = GgpEventQueueCreate();
  fprintf(stdout, "GGP event queue created\n");

  // Add client connection handlers
  app_data.stream_started_handler_id = GgpAddStreamStartedHandler(
      app_data.event_queue, HandleStreamStarted, NULL, UnregisterCallback);
  app_data.stream_stopped_handler_id = GgpAddStreamStoppedHandler(
      app_data.event_queue, HandleStreamStopped, NULL, UnregisterCallback);

  // Signal that the session is ready to receive events.
  GgpHandlersRegistered();

  // Initialize all of Vulkan
  CreateResources();

  GgpReadyToStream();
  fprintf(stdout, "GGP ready to stream\n");
}

// Finalize(): Clean up application resources
static void Finalize()
{
  // Destroy the event queue
  GgpEventQueueDestroy(app_data.event_queue);
  fprintf(stdout, "GGP event queue destroyed\n");

  // Remove client connection handlers.
  GgpRemoveStreamStartedHandler(app_data.stream_started_handler_id);
  GgpRemoveStreamStoppedHandler(app_data.stream_stopped_handler_id);

  // Indicate that GGP should shutdown.
  GgpShutDown();
}

int main(int argc, char **argv)
{
  if(!ParseCommandLine(argc, argv))
  {
    Usage();
    return 1;
  }

  Initialize();

  // Wait until user closes the window, then exit
  while(!app_data.quit)
  {
    while(GgpEventQueueProcessEvent(app_data.event_queue, 0))
    {
    }    // empty loop
    Render();
    if(main_should_quit_now())
      break;
  }

  Finalize();
  return 0;
}

#endif    // #if defined(__yeti__) || defined(__ggp__)

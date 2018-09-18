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
#include "vk_cpp_codec_writer.h"

namespace vk_cpp_codec
{
TemplateFileDesc CodeWriter::TemplateFiles[TEMPLATE_FILE_COUNT] = {
    /******************************************************************************/
    /* TEMPLATE_FILE_ROOT_CMAKE                                                   */
    /******************************************************************************/
    {"", "CMakeLists.txt",
     R"(CMAKE_MINIMUM_REQUIRED (VERSION 3.9)
# Disable some of the default cmake build targets, keep debug and release
SET (CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)

PROJECT(renderdoc_gen_frame)

SET (WORKING_DIRECTORY_DEBUG      "../../")
SET (WORKING_DIRECTORY_RELEASE    "../../")
SET (MSVS_USERFILE                "${PROJECT_SOURCE_DIR}/Template.user")

SET (PROJECT_PREFIX               "")
SET (MACHINE_POSTFIX              "")
SET (USERFILE_PLATFORM            "")

SET (MACHINE_IS_X64               TRUE)
SET (CMAKE_CXX_STANDARD           11)

IF ("${CMAKE_SIZEOF_VOID_P}"      EQUAL "8")
  SET (MACHINE_IS_X64  TRUE)
ELSEIF ("${CMAKE_SIZEOF_VOID_P}"  EQUAL "4")
  SET (MACHINE_IS_X64 FALSE)
ENDIF ()

IF (WIN32)
  IF (MACHINE_IS_X64)
    SET (MACHINE_POSTFIX            "_x64")
    SET (USERFILE_PLATFORM          "x64")
  ELSE ()
    SET (MACHINE_POSTFIX            "_x86")
    SET (USERFILE_PLATFORM          "Win32")
  ENDIF ()
  IF (MSVC)
    SET (PROJECT_PREFIX               "vs${MSVC_TOOLSET_VERSION}_")
  ENDIF ()
ENDIF ()

GET_FILENAME_COMPONENT(Trace ${CMAKE_CURRENT_SOURCE_DIR} NAME)
STRING(REPLACE " " "_" Trace ${Trace})
PROJECT(${PROJECT_PREFIX}${Trace}${MACHINE_POSTFIX})

OPTION (OPTION_TREAT_WARNINGS_AS_ERRORS
  "Check if you want to treat warnings as errors" FALSE)
OPTION(ENABLE_YETI "Enable Yeti support" OFF)

IF (MSVC)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  IF (OPTION_TREAT_WARNINGS_AS_ERRORS)
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
  ENDIF (OPTION_TREAT_WARNINGS_AS_ERRORS)
ELSEIF (UNIX)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")
ENDIF()

SET (EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/Build${MACHINE_POSTFIX}")
SET (LIBRARY_OUTPUT_PATH    "${CMAKE_SOURCE_DIR}/Build${MACHINE_POSTFIX}")

SET (DEBUG_POSTFIX "_DEBUG" CACHE STRING "Debug Postfitx")
SET (CMAKE_RELEASE_POSTFIX "" CACHE STRING "Release Postfitx")
SET (CMAKE_MINSIZEREL_POSTFIX "_MINSIZEREL" CACHE STRING "Minimum Size Release Postfitx")
SET (CMAKE_RELWITHDEBINFO_POSTFIX "_RELWITHDEBINFO" CACHE STRING "Release With Debug Info Postfitx")

###############################################################################
# Function that changes output names, adding the prefix and postfix
###############################################################################
FUNCTION (SETUP_PROJECT target)
  SET_TARGET_PROPERTIES (${target} PROPERTIES
                         OUTPUT_NAME_DEBUG   ${target}${MACHINE_POSTFIX}${DEBUG_POSTFIX}
                         OUTPUT_NAME_RELEASE ${target}${MACHINE_POSTFIX})
  SET (USER_FILE ${target}.vcxproj.user)
  SET (USERFILE_WORKING_DIRECTORY_DEBUG   ${WORKING_DIRECTORY_DEBUG}${target})
  SET (USERFILE_WORKING_DIRECTORY_RELEASE ${WORKING_DIRECTORY_RELEASE}${target})
  SET (OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/${USER_FILE})
  CONFIGURE_FILE (${MSVS_USERFILE} ${OUTPUT_PATH} @ONLY)
ENDFUNCTION ()

###############################################################################
# Search for Vulkan
###############################################################################
FIND_PACKAGE (Vulkan)
  MESSAGE (STATUS "Vulkan SDK                : $ENV{VULKAN_SDK}")
IF (Vulkan_FOUND)
  MESSAGE (STATUS "Vulkan Includes           : ${Vulkan_INCLUDE_DIRS}")
  MESSAGE (STATUS "Vulkan Libraries          : ${Vulkan_LIBRARIES}")
ELSE ()
  MESSAGE (FATAL_ERROR "Vulkan not found!")
ENDIF ()

ADD_LIBRARY (vulkan STATIC IMPORTED)

SET_TARGET_PROPERTIES (vulkan PROPERTIES
                       INTERFACE_INCLUDE_DIRECTORIES ${Vulkan_INCLUDE_DIRS}
                       IMPORTED_LOCATION             ${Vulkan_LIBRARIES})

SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sample_cpp_trace)

###############################################################################
# List Cmake projects
###############################################################################
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}")

ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/sample_cpp_trace"
                 "${CMAKE_BINARY_DIR}/sample_cpp_trace")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/helper"
                 "${CMAKE_BINARY_DIR}/helper")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/sample_cpp_shim"
                 "${CMAKE_BINARY_DIR}/sample_cpp_shim")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/gold_reference_shim"
                 "${CMAKE_BINARY_DIR}/gold_reference_shim")
ADD_SUBDIRECTORY("${CMAKE_SOURCE_DIR}/amd_shader_info_shim"
                 "${CMAKE_BINARY_DIR}/amd_shader_info_shim")
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_SAMPLE_MAIN_WIN                                              */
    /******************************************************************************/
    {"sample_cpp_trace", "main_win.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: main_win.cpp
//-----------------------------------------------------------------------------
// Defines the entry point that initializes and runs the serialized frame
// capture on Windows
#if _WIN32

#include <Windows.h>
#include <algorithm>

#include "gen_main.h"

int frameLoops = -1;
double accumTimeWithReset = 0;
double accumTime = 0;
double avgTimeWithReset = 0;
double avgTime = 0;
double avgFPSWithReset = 0;
double avgFPS = 0;
uint64_t frames = 0;
LARGE_INTEGER performanceCounterFrequency;
HINSTANCE appInstance;
HWND appHwnd;

#define RDOC_WINDOW_CLASS_NAME L"RenderDoc Frame Loop"
#define RDOC_WINDOW_TITLE L"RenderDoc Frame Loop"

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL RegisterWndClass(HINSTANCE hInstance, UINT style);
HWND CreateWnd(HINSTANCE hInstance, HINSTANCE hPrevInstance, uint32_t PosX, uint32_t PosY,
               uint32_t Width, uint32_t Height, DWORD Style, DWORD ExtendedStyle);
void CreateResources();
void ReleaseResources();
void Render();

void PostStageProgress(const char *stage, uint32_t i, uint32_t N) {
  SetWindowTextA(appHwnd, StageProgressString(stage, i, N).c_str());
}

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_KEYDOWN:
      if(wParam == VK_ESCAPE)
      {
        PostQuitMessage(0);
        return 0;
      }

      break;
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
      break;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL RegisterWndClass(HINSTANCE hInstance, UINT style)
{
  WNDCLASS wc;
  wc.style = style;
  wc.lpfnWndProc = (WNDPROC)MainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName = L"";
  wc.lpszClassName = RDOC_WINDOW_CLASS_NAME;
  BOOL ret = RegisterClass(&wc);
  if(!ret && GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
  {
    return TRUE;
  }
  return ret;
}

HWND CreateWnd(HINSTANCE hInstance, HINSTANCE hPrevInstance, uint32_t PosX, uint32_t PosY,
               uint32_t Width, uint32_t Height, DWORD Style, DWORD ExtendedStyle)
{
  // Need to adjust first, to see if it fits the screen.
  RECT WindowRect = {(LONG)PosX, (LONG)PosY, (LONG)(PosX + Width), (LONG)(PosY + Height)};
  AdjustWindowRectEx(&WindowRect, Style, NULL, ExtendedStyle);
  Width = WindowRect.right - WindowRect.left;
  Height = WindowRect.bottom - WindowRect.top;
  return CreateWindowEx(ExtendedStyle, RDOC_WINDOW_CLASS_NAME, RDOC_WINDOW_TITLE, Style, PosX, PosY,
                        Width, Height, (HWND)NULL, (HMENU)NULL, hInstance, (LPVOID)NULL);
}

void CreateResources()
{
  RegisterWndClass(appInstance, CS_HREDRAW | CS_VREDRAW);
  // Resolution Width and Height are declared in gen_variables
  appHwnd = CreateWnd(appInstance, NULL, 0, 0, resolutionWidth, resolutionHeight,
                      WS_BORDER | WS_DLGFRAME | WS_GROUP | WS_OVERLAPPED | WS_POPUP | WS_SIZEBOX |
                          WS_SYSMENU | WS_TILED | WS_VISIBLE,
                      0);
  SetWindowTextA(appHwnd, "RenderDoc Frame Loop: Creating Resources");
  main_create();
  SetWindowTextA(appHwnd, "RenderDoc Frame Loop: Initializing Resources");
  main_init();
}

void ReleaseResources()
{
  main_release();
}

double GetTimestampMilliseconds()
{
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return 1e3 * ((double)counter.QuadPart) / performanceCounterFrequency.QuadPart;
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

  if(frames % 1 == 0)
  {
    char str[256];
    sprintf(str, "%s Avg Time [%f / %f] Avg FPS [%f /%f]", "RenderDoc Frame Loop", avgTimeWithReset,
            avgTime, avgFPSWithReset, avgFPS);
    SetWindowTextA(appHwnd, str);
  }
}

static void ProcessMessages(bool &quit)
{
  MSG msg;
  if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    if(msg.message == WM_QUIT)
    {
      quit = true;
    }
    else
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  bool quit = false;
  appInstance = hInstance;

  CreateResources();
  QueryPerformanceFrequency(&performanceCounterFrequency);

  int repeatIteration = 0;
  while(frameLoops == -1 || repeatIteration < frameLoops)
  {
    ProcessMessages(quit);
    if(quit)
    {
      break;
    }
    Render();
    repeatIteration = (std::max)(0, repeatIteration + 1);
  }

  ReleaseResources();
  return EXIT_SUCCESS;
}

#endif    // #if _WIN32
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_SAMPLE_MAIN_YETI                                             */
    /******************************************************************************/
    {"sample_cpp_trace", "main_yeti.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: main_yeti.cpp
//-----------------------------------------------------------------------------
// Defines the entry point that initializes and runs the serialized frame
// capture on Yeti
#if defined(__yeti__)

#include <yeti_c/yeti.h>

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

void PostStageProgress(const char *stage, uint32_t i, uint32_t N) {
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

void Render()
{
  main_prereset();
  main_render();
  main_postreset();
}

const uint64_t kMicrosecondsPerFrame = 16666L;

static struct
{
  // Yeti
  YetiEventQueue event_queue;
  uint32_t stream_started_handler_id;
  uint32_t stream_stopped_handler_id;

  // General
  bool quit;
} app_data = {0};

// ClockNowMicroSeconds(): Return current time in microseconds
static inline uint64_t ClockNowMicroSeconds()
{
  struct timespec now = {};
  clock_gettime(CLOCK_MONOTONIC_RAW, &now);
  uint64_t nanoseconds = (now.tv_sec * 1000000000LL) + now.tv_nsec;
  uint64_t microseconds = nanoseconds / 1000LL;
  return microseconds;
}

// HandleStreamStarted(): Client connected handler
static void HandleStreamStarted(void *user_data)
{
  fprintf(stdout, "client connected\n");
}

// HandleStreamStopped(): Client disconnected handler
static void HandleStreamStopped(const YetiStreamStoppedEvent *event, void *user_data)
{
  fprintf(stdout, "client disconnected\n");

  // Exit the application if the client disconnects to exit.
  if(event->stream_stopped_reason == kYetiStreamStopped_Exited)
  {
    app_data.quit = true;
  }
  else if(event->stream_stopped_reason == kYetiStreamStopped_Unexpected)
  {
    YetiSuspended();
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
  app_data.event_queue = YetiEventQueueCreate();
  fprintf(stdout, "Yeti event queue created\n");

  // Add client connection handlers
  app_data.stream_started_handler_id = YetiAddStreamStartedHandler(
      app_data.event_queue, HandleStreamStarted, NULL, UnregisterCallback);
  app_data.stream_stopped_handler_id = YetiAddStreamStoppedHandler(
      app_data.event_queue, HandleStreamStopped, NULL, UnregisterCallback);

  // Signal that the session is ready to receive events.
  YetiHandlersRegistered();

  // Initialize all of Vulkan
  CreateResources();

  YetiReadyToStream();
  fprintf(stdout, "Yeti ready to stream\n");
}

// Finalize(): Clean up application resources
static void Finalize()
{
  // Destroy the event queue
  YetiEventQueueDestroy(app_data.event_queue);
  fprintf(stdout, "Yeti event queue destroyed\n");

  // Remove client connection handlers.
  YetiRemoveStreamStartedHandler(app_data.stream_started_handler_id);
  YetiRemoveStreamStoppedHandler(app_data.stream_stopped_handler_id);

  // Indicate that Yeti should shutdown.
  YetiShutDown();
}

int main(int argc, char **argv)
{
  Initialize();

  // Wait until user closes the window, then exit
  while(!app_data.quit)
  {
    fprintf(stdout, "main loop\n");

    // uint64_t whenToResume = ClockNowMicroSeconds() + kMicrosecondsPerFrame;

    while(YetiEventQueueProcessEvent(app_data.event_queue, 0))
    {
    }    // empty loop

    fprintf(stdout, "main loop pre-render\n");
    Render();
    fprintf(stdout, "main loop post-render\n");

    // Sleep for 1/60 second (one frame)
    // uint64_t timeLeft = whenToResume - ClockNowMicroSeconds();
    // if (timeLeft > 0) {
    //  struct timespec sleepTime = {};
    //  sleepTime.tv_nsec         = timeLeft * 1000LL;
    //  nanosleep(&sleepTime, NULL);
    //}
  }

  Finalize();
  return 0;
}

#endif    // #if defined(__yeti__)
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_SAMPLE_MAIN_XLIB                                             */
    /******************************************************************************/
    {"sample_cpp_trace", "main_xlib.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: main_xlib.cpp
//-----------------------------------------------------------------------------
// Defines the entry point that initializes and runs the serialized frame
// capture on Linux
#if defined(__linux__) && !defined(__yeti__)

#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>
#include <stdexcept>

#include "gen_main.h"

int frameLoops = -1;

Display *appDisplay;
Window appWindow;

#define RDOC_WINDOW_TITLE "RenderDoc Frame Loop"

void PostStageProgress(const char *stage, uint32_t i, uint32_t N) {
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

int main(int argc, char **argv)
{
  bool quit = false;
  CreateResources();
  int repeatIteration = 0;
  while(frameLoops == -1 || repeatIteration < frameLoops)
  {
    if(quit)
    {
      break;
    }
    Render();
    repeatIteration = (std::max)(0, repeatIteration + 1);
  }

  ReleaseResources();
}

#endif    // #if defined(__linux__) && !defined(__yeti__)
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_SAMPLE_COMMON                                                */
    /******************************************************************************/
    {"sample_cpp_trace", "common.h",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: common.h
//-----------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "helper/helper.h"

void PostStageProgress(const char *stage, uint32_t i, uint32_t N);
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_SAMPLE_CMAKE                                                 */
    /******************************************************************************/
    {"sample_cpp_trace", "CMakeLists.txt",
     R"(IF(WIN32)
  SET (THIS_PROJECT_NAME sample_cpp_trace)
ELSE ()
  SET (THIS_PROJECT_NAME sample_cpp_trace_elf)
ENDIF ()
PROJECT(${THIS_PROJECT_NAME})

FILE(GLOB HEADERS "*.h")
FILE(GLOB SOURCES "*.cpp")
SET(SHADERS )
SET(TEMPLATES main_win.cpp main_yeti.cpp main_xlib.cpp common.h)

SOURCE_GROUP ( "Source Files"   FILES ${SOURCES} )
SOURCE_GROUP ( "Template Files" FILES ${TEMPLATES} )
SOURCE_GROUP ( "Header Files"   FILES ${HEADERS} )
SOURCE_GROUP ( "Shader Files"   FILES ${SHADERS} )

ADD_EXECUTABLE(${THIS_PROJECT_NAME} ${TEMPLATES} ${SOURCES}
                                    ${HEADERS} ${SHADERS})

SETUP_PROJECT(${THIS_PROJECT_NAME})

TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE UNICODE _UNICODE)

IF (WIN32)
  TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                             VK_USE_PLATFORM_WIN32_KHR
                             _CRT_SECURE_NO_DEPRECATE)
  TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                        vulkan helper comctl32 rpcrt4
                        winmm advapi32 wsock32 Dbghelp sample_cpp_shim)
  SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /STACK:67108864"
                        LINK_FLAGS_DEBUG   "/SUBSYSTEM:WINDOWS /STACK:67108864")
ELSEIF (ENABLE_YETI)
  TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                             VK_USE_PLATFORM_YETI_GOOGLE
                             __yeti__)
  TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                        libyeti.so libdl.so vulkan helper sample_cpp_shim)
ELSE ()
  TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                             VK_USE_PLATFORM_XLIB_KHR
                             __linux__)
  TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                        libX11.so libdl.so helper vulkan sample_cpp_shim)
ENDIF ()
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_HELPER_H                                                     */
    /******************************************************************************/
    {"helper", "helper.h",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: helper.h
//-----------------------------------------------------------------------------
#pragma once

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#if defined(_WIN32)
#include <Windows.h>
#endif

#include <algorithm>
#include <vector>

#include "vulkan/vulkan.h"
#include "format_helper.h"

#define var_to_string(s) #s

struct AuxVkTraceResources
{
  VkInstance instance;
  VkDevice device;
  VkPhysicalDevice physDevice;
  VkPhysicalDeviceProperties physDeviceProperties;
  VkDebugReportCallbackEXT callback;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
  VkQueue queue;
  VkFence fence;
  VkSemaphore semaphore;
};

struct Region
{
  uint64_t offset = 0;
  uint64_t size = 0;
  Region(){};
  Region(uint64_t o, uint64_t s) : offset(o), size(s) {}
};

struct MemoryRemap
{
  Region capture;
  Region replay;
};

typedef std::vector<MemoryRemap> MemoryRemapVec;

VkPresentModeKHR GetCompatiblePresentMode(VkPresentModeKHR captured,
                                          std::vector<VkPresentModeKHR> present);

int32_t MemoryTypeIndex(VkMemoryPropertyFlags mask, uint32_t bits,
                        VkPhysicalDeviceMemoryProperties memory_props);

uint32_t CompatibleMemoryTypeIndex(uint32_t type, const VkPhysicalDeviceMemoryProperties &captured,
                                   const VkPhysicalDeviceMemoryProperties &present, uint32_t bits);

VkResult CheckMemoryAllocationCompatibility(uint32_t type,
                                            const VkPhysicalDeviceMemoryProperties &captured,
                                            const VkPhysicalDeviceMemoryProperties &present,
                                            const VkMemoryRequirements &requirements);

void ReadBuffer(const char *name, std::vector<uint8_t> &buf);

void InitializeDestinationBuffer(VkDevice device, VkBuffer *dst_buffer, VkDeviceMemory dst_memory,
                                 uint64_t size);
void InitializeSourceBuffer(VkDevice device, VkBuffer *buffer, VkDeviceMemory *memory, size_t size,
                            uint8_t *initial_data, VkPhysicalDeviceMemoryProperties props,
                            MemoryRemapVec &remap);
void InitializeAuxResources(AuxVkTraceResources *aux, VkInstance instance,
                            VkPhysicalDevice physDevice, VkDevice device);

void ImageLayoutTransition(VkCommandBuffer cmdBuffer, VkImage dstImage,
  VkImageSubresourceRange subresourceRange, VkImageLayout newLayout,
  uint32_t dstQueueFamily, VkImageLayout oldLayout, uint32_t srcQueueFamily);

void ImageLayoutTransition(AuxVkTraceResources aux, VkImage dst, VkImageCreateInfo dst_ci,
                           VkImageLayout final_layout,
                           VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED);
void ImageLayoutTransition(AuxVkTraceResources aux, VkImage dst,
                           VkImageSubresourceRange subresourceRange, VkImageLayout final_layout,
                           VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED);
void ImageLayoutTransition(AuxVkTraceResources aux, VkImage dstImg, uint32_t arrayLayer,
                           uint32_t mipLevel, VkImageAspectFlagBits aspect, VkImageLayout newLayout,
                           VkImageLayout oldLayout);

void CopyResetImage(AuxVkTraceResources aux, VkImage dst, VkBuffer src, VkImageCreateInfo dst_ci);
void CopyResetBuffer(AuxVkTraceResources aux, VkBuffer dst, VkBuffer src, VkDeviceSize size);

void CopyImageToBuffer(AuxVkTraceResources aux, VkImage src, VkBuffer dst, VkImageCreateInfo src_ci);
void DiffDeviceMemory(AuxVkTraceResources aux, VkDeviceMemory expected,
                      VkDeviceSize expected_offset, VkDeviceMemory actual,
                      VkDeviceSize actual_offset, VkDeviceSize size, const char *name);
void InitializeDiffBuffer(VkDevice device, VkBuffer *buffer, VkDeviceMemory *memory, size_t size,
                          VkPhysicalDeviceMemoryProperties props);

void MakePhysicalDeviceFeaturesMatch(const VkPhysicalDeviceFeatures &available,
                                     VkPhysicalDeviceFeatures *captured_request);

void RegisterDebugCallback(AuxVkTraceResources aux, VkInstance instance,
                           uint32_t flags);

void MapUpdate(AuxVkTraceResources aux, uint8_t *dst, uint8_t *src, const VkMappedMemoryRange &range,
               VkMemoryAllocateInfo &ai, MemoryRemapVec &remap, VkDevice dev);

inline uint64_t AlignedSize(uint64_t size, uint64_t alignment)
{
  return ((size / alignment) + ((size % alignment) > 0 ? 1 : 0)) * alignment;
}
inline uint64_t AlignedDown(uint64_t size, uint64_t alignment)
{
  return (uint64_t(size / alignment)) * alignment;
}

bool IsExtEnabled(const char *const *extList, uint32_t count, const char *ext);
bool IsExtSupported(VkPhysicalDevice physicalDevice, const char *ext);

std::string StageProgressString(const char *stage, uint32_t i, uint32_t N);
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_HELPER_CPP                                                   */
    /******************************************************************************/
    {"helper", "helper.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: helper.cpp
//-----------------------------------------------------------------------------
#include <string>
#include "helper.h"

VkBool32 VKAPI_PTR DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                 uint64_t object, size_t location, int32_t messageCode,
                                 const char *pLayerPrefix, const char *pMessage, void *pUserData)
{
  switch(flags)
  {
    case VK_DEBUG_REPORT_ERROR_BIT_EXT:
    case VK_DEBUG_REPORT_DEBUG_BIT_EXT: fprintf(stderr, "%s\n", pMessage);
#if defined(_WIN32)
      OutputDebugStringA(pMessage);
      OutputDebugStringA("\n");
#endif
  }

  return VK_FALSE;
}

void RegisterDebugCallback(AuxVkTraceResources aux, VkInstance instance,
                           uint32_t flags)
{
  PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
  CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugReportCallbackEXT");

  VkDebugReportCallbackCreateInfoEXT ci = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
                                           0, VkDebugReportFlagsEXT(flags), DebugCallback, NULL};
  if(CreateDebugReportCallback != NULL)
  {
    VkResult result = CreateDebugReportCallback(instance, &ci, NULL, &aux.callback);
    assert(result == VK_SUCCESS);
  }
}

VkPresentModeKHR GetCompatiblePresentMode(VkPresentModeKHR captured,
  std::vector<VkPresentModeKHR> present) {
  for(uint32_t i = 0; i < present.size(); i++)
    if(present[i] == captured)
      return captured;

  assert(present.size() > 0);
  return present[0];
}

void ImageLayoutTransition(VkCommandBuffer cmdBuffer, VkImage dstImage,
  VkImageSubresourceRange subresourceRange, VkImageLayout newLayout,
                           uint32_t dstQueueFamily, VkImageLayout oldLayout, uint32_t srcQueueFamily)
{
  uint32_t all_access =
    VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
    VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
    VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_READ_BIT |
    VK_ACCESS_HOST_WRITE_BIT;

  VkImageMemoryBarrier imgBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                     NULL,
                                     all_access,
                                     VK_ACCESS_TRANSFER_WRITE_BIT,
                                     oldLayout,
                                     newLayout,
                                     srcQueueFamily,
                                     dstQueueFamily,
                                     dstImage,
                                     subresourceRange};

  vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imgBarrier);
}

void ImageLayoutTransition(AuxVkTraceResources aux, VkImage dstImage,
                           VkImageSubresourceRange subresourceRange, VkImageLayout newLayout,
                           VkImageLayout oldLayout)
{
  ImageLayoutTransition(aux.command_buffer, dstImage, subresourceRange, newLayout,
                        VK_QUEUE_FAMILY_IGNORED, oldLayout, VK_QUEUE_FAMILY_IGNORED);
}

void ImageLayoutTransition(AuxVkTraceResources aux, VkImage dstImage, VkImageCreateInfo dstCI,
                           VkImageLayout newLayout, VkImageLayout oldLayout)
{
  VkImageSubresourceRange subresourceRange = {FullAspectFromFormat(dstCI.format), 0,
                                              VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

  ImageLayoutTransition(aux, dstImage, subresourceRange, newLayout, oldLayout);
}

void ImageLayoutTransition(AuxVkTraceResources aux, VkImage dstImg, uint32_t arrayLayer,
                           uint32_t mipLevel, VkImageAspectFlagBits aspect, VkImageLayout newLayout,
                           VkImageLayout oldLayout)
{
  VkImageSubresourceRange subresourceRange = {VkImageAspectFlags(aspect), mipLevel, 1, arrayLayer, 1};

  ImageLayoutTransition(aux, dstImg, subresourceRange, newLayout, oldLayout);
}

void CopyResetImage(AuxVkTraceResources aux, VkImage dst, VkBuffer src, VkImageCreateInfo dst_ci)
{
  ImageLayoutTransition(aux, dst, dst_ci, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  if(dst_ci.samples == VK_SAMPLE_COUNT_1_BIT)
  {
    VkImageAspectFlags aspect = FullAspectFromFormat(dst_ci.format);
    VkImageAspectFlags color_depth_stencil =
        VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    assert(((aspect & color_depth_stencil) != 0) &&
           ((aspect & (~color_depth_stencil)) ==
            0));    // only color, depth or stencil aspects are allowed

    std::vector<VkImageAspectFlagBits> aspects;
    if(aspect & VK_IMAGE_ASPECT_COLOR_BIT)
      aspects.push_back(VK_IMAGE_ASPECT_COLOR_BIT);
    if(aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
      aspects.push_back(VK_IMAGE_ASPECT_DEPTH_BIT);
    if(aspect & VK_IMAGE_ASPECT_STENCIL_BIT)
      aspects.push_back(VK_IMAGE_ASPECT_STENCIL_BIT);

    std::vector<VkBufferImageCopy> regions;
    uint32_t offset = 0;
    for(uint32_t j = 0; j < aspects.size(); j++)
    {
    for(uint32_t a = 0; a < dst_ci.arrayLayers; a++)
    {
      VkExtent3D dim = dst_ci.extent;
      uint32_t x = 0;
      FixCompressedSizes(dst_ci.format, dim, x);
        for(uint32_t m = 0; m < dst_ci.mipLevels; m++)
      {
          VkBufferImageCopy region = {offset,     dim.width,
                                      dim.height, {VkImageAspectFlags(aspects[j]), m, a, 1},
                                      {0, 0, 0},  dim};
          offset += (uint32_t)(dim.depth * dim.width * dim.height *
                               SizeOfFormat(dst_ci.format, aspects[j]));
        dim.height = std::max<int>(dim.height / 2, 1);
        dim.width = std::max<int>(dim.width / 2, 1);
        dim.depth = std::max<int>(dim.depth / 2, 1);
        FixCompressedSizes(dst_ci.format, dim, offset);
        regions.push_back(region);
        }    // mip
      }      // array
    }        // aspect
    const uint32_t kMaxUpdate = 100;
    for(uint32_t i = 0; i * kMaxUpdate < regions.size(); i++)
    {
      uint32_t count = std::min<uint32_t>(kMaxUpdate, (uint32_t)regions.size() - i * kMaxUpdate);
      uint32_t offset = i * kMaxUpdate;
      vkCmdCopyBufferToImage(aux.command_buffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             count, regions.data() + offset);
    }
  } else {
    std::string msg = std::string(__FUNCTION__) + std::string(": resets MSAA resource with ") +
      std::to_string(dst_ci.samples) + std::string(" samples. Currently this is not implemented.\n");
    printf("%s", msg.c_str());
#if defined(_WIN32) || defined(WIN32)
    OutputDebugStringA(msg.c_str());
#endif
  }
}
void CopyResetBuffer(AuxVkTraceResources aux, VkBuffer dst, VkBuffer src, VkDeviceSize size)
{
  if(size == 0)
    return;
  VkBufferCopy region = {0, 0, size};
  vkCmdCopyBuffer(aux.command_buffer, src, dst, 1, &region);
}

void CopyImageToBuffer(AuxVkTraceResources aux, VkImage src, VkBuffer dst, VkImageCreateInfo src_ci)
{
  if(src_ci.samples == VK_SAMPLE_COUNT_1_BIT)
  {
    VkImageAspectFlags aspect = FullAspectFromFormat(src_ci.format);
    VkImageAspectFlags color_depth_stencil =
      VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    assert(((aspect & color_depth_stencil) != 0) &&
      ((aspect & (~color_depth_stencil)) ==
        0));    // only color, depth or stencil aspects are allowed

    std::vector<VkImageAspectFlagBits> aspects;
    if (aspect & VK_IMAGE_ASPECT_COLOR_BIT)
      aspects.push_back(VK_IMAGE_ASPECT_COLOR_BIT);
    if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
      aspects.push_back(VK_IMAGE_ASPECT_DEPTH_BIT);
    if (aspect & VK_IMAGE_ASPECT_STENCIL_BIT)
      aspects.push_back(VK_IMAGE_ASPECT_STENCIL_BIT);

    std::vector<VkBufferImageCopy> regions;
    uint32_t offset = 0;
    for (uint32_t j = 0; j < aspects.size(); j++) {
      for (uint32_t a = 0; a < src_ci.arrayLayers; a++) {
        VkExtent3D dim = src_ci.extent;
        uint32_t x = 0;
        FixCompressedSizes(src_ci.format, dim, x);
        for (uint32_t m = 0; m < src_ci.mipLevels; m++) {
          VkBufferImageCopy region = {offset,     dim.width,
                                      dim.height, {VkImageAspectFlags(aspects[j]), m, a, 1},
                                      {0, 0, 0},  dim};
          offset += (uint32_t) (dim.depth * dim.width * dim.height * SizeOfFormat(src_ci.format, aspects[j]));
          dim.height = std::max<int>(dim.height / 2, 1);
          dim.width = std::max<int>(dim.width / 2, 1);
          dim.depth = std::max<int>(dim.depth / 2, 1);
          FixCompressedSizes(src_ci.format, dim, offset);
          regions.push_back(region);
        }
      }
    }
    const uint32_t kMaxUpdate = 100;
    for(uint32_t i = 0; i * kMaxUpdate < regions.size(); i++)
    {
      uint32_t count = std::min<uint32_t>(kMaxUpdate, (uint32_t)regions.size() - i * kMaxUpdate);
      uint32_t offset = i * kMaxUpdate;
      vkCmdCopyImageToBuffer(aux.command_buffer, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst,
                             count, regions.data() + offset);
    }
  }
  else
  {
    assert(0);
  }
}

void DiffDeviceMemory(AuxVkTraceResources aux, VkDeviceMemory expected,
                      VkDeviceSize expected_offset, VkDeviceMemory actual,
                      VkDeviceSize actual_offset, VkDeviceSize size, const char *name)
{
  uint8_t *expected_data = NULL;
  VkResult result = vkMapMemory(aux.device, actual, actual_offset, size, 0, (void **)&expected_data);
  assert(result == VK_SUCCESS);

  uint8_t *actual_data = NULL;
  result = vkMapMemory(aux.device, expected, expected_offset, size, 0, (void **)&actual_data);
  assert(result == VK_SUCCESS);

  if(memcmp(expected_data, actual_data, (size_t)size) != 0)
  {
    std::string msg = std::string(__FUNCTION__) + std::string(": Resource ")
        + std::string(name) + std::string(" has changed by the end of the frame.\n");
    printf("%s", msg.c_str());
#if defined(_WIN32) || defined(WIN32)
    OutputDebugStringA(msg.c_str());
#endif
  }

  vkUnmapMemory(aux.device, expected);
  vkUnmapMemory(aux.device, actual);
}

void InitializeDiffBuffer(VkDevice device, VkBuffer *buffer, VkDeviceMemory *memory, size_t size,
                          VkPhysicalDeviceMemoryProperties props)
{
  assert(buffer != NULL && memory != NULL);
  if(size == 0)
    return;

  VkBufferCreateInfo buffer_ci = {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      NULL,
      0,
      size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_SHARING_MODE_EXCLUSIVE,
      0,
      NULL};

  VkResult result = vkCreateBuffer(device, &buffer_ci, NULL, buffer);
  assert(result == VK_SUCCESS);

  VkMemoryRequirements buffer_requirements;
  vkGetBufferMemoryRequirements(device, *buffer, &buffer_requirements);

  VkFlags gpu_and_cpu_visible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  uint32_t memory_type =
      MemoryTypeIndex(gpu_and_cpu_visible, buffer_requirements.memoryTypeBits, props);

  VkMemoryAllocateInfo memory_ai = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL,
                                    buffer_requirements.size, memory_type};

  result = vkAllocateMemory(device, &memory_ai, NULL, memory);
  assert(result == VK_SUCCESS);

  result = vkBindBufferMemory(device, *buffer, *memory, 0);
  assert(result == VK_SUCCESS);
}

void InitializeDestinationBuffer(VkDevice device, VkBuffer *dst_buffer, VkDeviceMemory dst_memory,
                                 uint64_t size)
{
  assert(dst_buffer != NULL);
  if(size == 0)
    return;

  VkBufferCreateInfo buffer_dst_ci = {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      NULL,
      0,
      size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_SHARING_MODE_EXCLUSIVE,
      0,
      NULL};

  VkResult result = vkCreateBuffer(device, &buffer_dst_ci, NULL, dst_buffer);
  assert(result == VK_SUCCESS);

  result = vkBindBufferMemory(device, *dst_buffer, dst_memory, 0);
  assert(result == VK_SUCCESS);
}

void InitializeSourceBuffer(VkDevice device, VkBuffer *src_buffer, VkDeviceMemory *src_memory,
                            size_t size, uint8_t *initial_data,
                            VkPhysicalDeviceMemoryProperties props, MemoryRemapVec &remap)
{
  assert(src_buffer != NULL && src_memory != NULL);
  if(size == 0)
    return;

  VkBufferCreateInfo buffer_src_ci = {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      NULL,
      0,
      size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_SHARING_MODE_EXCLUSIVE,
      0,
      NULL};

  VkResult result = vkCreateBuffer(device, &buffer_src_ci, NULL, src_buffer);
  assert(result == VK_SUCCESS);

  VkMemoryRequirements buffer_requirements;
  vkGetBufferMemoryRequirements(device, *src_buffer, &buffer_requirements);

  VkFlags gpu_and_cpu_visible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  uint32_t memory_type =
      MemoryTypeIndex(gpu_and_cpu_visible, buffer_requirements.memoryTypeBits, props);

  VkMemoryAllocateInfo memory_ai = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL,
                                    buffer_requirements.size, memory_type};

  result = vkAllocateMemory(device, &memory_ai, NULL, src_memory);
  assert(result == VK_SUCCESS);

  result = vkBindBufferMemory(device, *src_buffer, *src_memory, 0);
  assert(result == VK_SUCCESS);

  uint8_t *data = NULL;
  result = vkMapMemory(device, *src_memory, 0, size, 0, (void **)&data);
  assert(result == VK_SUCCESS);

  // For each resource bound in the memory allocation, copy the correct
  // memory segment into 'src' buffer.
  if(remap.size() > 0)
  {
    for(uint32_t i = 0; i < remap.size(); i++)
    {
      MemoryRemap mr = remap[i];
      if(mr.replay.offset + mr.replay.size <= size)
      {
        memcpy(data + mr.replay.offset, initial_data + mr.capture.offset,
               std::min<uint64_t>(mr.capture.size, mr.replay.size));
      }
    }
  }
  else
  {
    memcpy(data, initial_data, size);
  }

  VkMappedMemoryRange memory_range = {VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, NULL, *src_memory, 0,
                                      size};

  result = vkFlushMappedMemoryRanges(device, 1, &memory_range);
  assert(result == VK_SUCCESS);

  vkUnmapMemory(device, *src_memory);
}

void InitializeAuxResources(AuxVkTraceResources *aux, VkInstance instance,
                            VkPhysicalDevice physDevice, VkDevice device)
{
  aux->instance = instance;
  aux->physDevice = physDevice;
  aux->device = device;
  vkGetPhysicalDeviceProperties(aux->physDevice, &aux->physDeviceProperties);

  vkGetDeviceQueue(device, 0, 0, &aux->queue);

  VkCommandPoolCreateInfo cmd_pool_ci = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, NULL,
                                         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 0};

  VkResult result = vkCreateCommandPool(device, &cmd_pool_ci, NULL, &aux->command_pool);
  assert(result == VK_SUCCESS);

  VkCommandBufferAllocateInfo cmd_buffer_ai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, NULL,
                                               aux->command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};

  result = vkAllocateCommandBuffers(device, &cmd_buffer_ai, &aux->command_buffer);
  assert(result == VK_SUCCESS);

  VkFenceCreateInfo fence_ci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, NULL, 0};

  result = vkCreateFence(device, &fence_ci, NULL, &aux->fence);
  assert(result == VK_SUCCESS);

  VkSemaphoreCreateInfo semaphore_ci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, NULL, 0};
  result = vkCreateSemaphore(device, &semaphore_ci, NULL, &aux->semaphore);
}
)" + std::string(R"(
int32_t MemoryTypeIndex(VkMemoryPropertyFlags mask, uint32_t bits,
                        VkPhysicalDeviceMemoryProperties memory_props)
{
  for(uint32_t i = 0; i < memory_props.memoryTypeCount; ++i)
  {
    if((bits & 1) == 1)
    {
      // Type is available, does it match user properties?
      if((memory_props.memoryTypes[i].propertyFlags & mask) == mask)
      {
        return i;
      }
    }
    bits = bits >> 1;
  }
  return -1;
}

uint32_t CompatibleMemoryTypeIndex(uint32_t type, const VkPhysicalDeviceMemoryProperties &captured,
                                   const VkPhysicalDeviceMemoryProperties &present, uint32_t bits)
{
  // When the application was captured this is the property flag that was
  // picked as compatible. Try to find the closest match.
  // This if fairly conservative and here is an example where this might fail:
  // Let System A, where the trace is captured, has all the memory marked as
  // DEVICE_LOCAL_BIT | HOST_VISIBLE_BIT (for example UMA devices).
  // The application requests a memory allocation with just a
  // HOST_VISIBLE_BIT but gets a memory type index that points to
  // HOST_VISIBLE_BIT | DEVICE_LOCAL_BIT. On System B, memory is split into:
  // 1. HOST_VISIBLE 2. DEVICE_LOCAL and 3. HOST_VISIBLE | DEVICE_LOCAL.
  // Since the captured memory type was HOST_VISIBLE | DEVICE_LOCAL on replay
  // the 3rd memory segment will get selected.
  VkMemoryType mem_type = captured.memoryTypes[type];
  VkMemoryPropertyFlags propertyFlag = mem_type.propertyFlags;

  // All memory types are approved with 0xFFFFFFFF bits
  return MemoryTypeIndex(propertyFlag, bits, present);
}

VkResult CheckMemoryAllocationCompatibility(uint32_t type,
                                            const VkPhysicalDeviceMemoryProperties &captured,
                                            const VkPhysicalDeviceMemoryProperties &present,
                                            const VkMemoryRequirements &requirements)
{
  VkMemoryType mem_type = captured.memoryTypes[type];
  VkMemoryPropertyFlags propertyFlag = mem_type.propertyFlags;

  uint32_t compat_type =
      CompatibleMemoryTypeIndex(type, captured, present, requirements.memoryTypeBits);

  uint32_t current = MemoryTypeIndex(propertyFlag, requirements.memoryTypeBits, present);

  return (compat_type == current ? VK_SUCCESS : VK_ERROR_VALIDATION_FAILED_EXT);
}

void ReadBuffer(const char *name, std::vector<uint8_t> &buf)
{
  FILE *f = fopen(name, "rb");
  if(f == NULL)
  {
    return;
  }

  fseek(f, 0, SEEK_END);
  uint64_t length = ftell(f);
  buf.resize(length);
  rewind(f);

  uint64_t result = fread(buf.data(), 1, length, f);
  fclose(f);
  assert(result <= length);
}

#define ReportMismatchedFeature(x, y)                                                             \
  if(x > y)                                                                                       \
  {                                                                                               \
    fprintf(stdout, "%s (%d) doesn't match %s (%d)\n", var_to_string(x), x, var_to_string(y), y); \
    x = y;                                                                                        \
  }

void MakePhysicalDeviceFeaturesMatch(const VkPhysicalDeviceFeatures &available,
                                     VkPhysicalDeviceFeatures *captured_request)
{
  ReportMismatchedFeature(captured_request->robustBufferAccess, available.robustBufferAccess);
  ReportMismatchedFeature(captured_request->fullDrawIndexUint32, available.fullDrawIndexUint32);
  ReportMismatchedFeature(captured_request->imageCubeArray, available.imageCubeArray);
  ReportMismatchedFeature(captured_request->independentBlend, available.independentBlend);
  ReportMismatchedFeature(captured_request->geometryShader, available.geometryShader);
  ReportMismatchedFeature(captured_request->tessellationShader, available.tessellationShader);
  ReportMismatchedFeature(captured_request->sampleRateShading, available.sampleRateShading);
  ReportMismatchedFeature(captured_request->dualSrcBlend, available.dualSrcBlend);
  ReportMismatchedFeature(captured_request->logicOp, available.logicOp);
  ReportMismatchedFeature(captured_request->multiDrawIndirect, available.multiDrawIndirect);
  ReportMismatchedFeature(captured_request->drawIndirectFirstInstance,
                          available.drawIndirectFirstInstance);
  ReportMismatchedFeature(captured_request->depthClamp, available.depthClamp);
  ReportMismatchedFeature(captured_request->depthBiasClamp, available.depthBiasClamp);
  ReportMismatchedFeature(captured_request->fillModeNonSolid, available.fillModeNonSolid);
  ReportMismatchedFeature(captured_request->depthBounds, available.depthBounds);
  ReportMismatchedFeature(captured_request->wideLines, available.wideLines);
  ReportMismatchedFeature(captured_request->largePoints, available.largePoints);
  ReportMismatchedFeature(captured_request->alphaToOne, available.alphaToOne);
  ReportMismatchedFeature(captured_request->multiViewport, available.multiViewport);
  ReportMismatchedFeature(captured_request->samplerAnisotropy, available.samplerAnisotropy);
  ReportMismatchedFeature(captured_request->textureCompressionETC2, available.textureCompressionETC2);
  ReportMismatchedFeature(captured_request->textureCompressionASTC_LDR,
                          available.textureCompressionASTC_LDR);
  ReportMismatchedFeature(captured_request->textureCompressionBC, available.textureCompressionBC);
  ReportMismatchedFeature(captured_request->occlusionQueryPrecise, available.occlusionQueryPrecise);
  ReportMismatchedFeature(captured_request->pipelineStatisticsQuery,
                          available.pipelineStatisticsQuery);
  ReportMismatchedFeature(captured_request->vertexPipelineStoresAndAtomics,
                          available.vertexPipelineStoresAndAtomics);
  ReportMismatchedFeature(captured_request->fragmentStoresAndAtomics,
                          available.fragmentStoresAndAtomics);
  ReportMismatchedFeature(captured_request->shaderTessellationAndGeometryPointSize,
                          available.shaderTessellationAndGeometryPointSize);
  ReportMismatchedFeature(captured_request->shaderImageGatherExtended,
                          available.shaderImageGatherExtended);
  ReportMismatchedFeature(captured_request->shaderStorageImageExtendedFormats,
                          available.shaderStorageImageExtendedFormats);
  ReportMismatchedFeature(captured_request->shaderStorageImageMultisample,
                          available.shaderStorageImageMultisample);
  ReportMismatchedFeature(captured_request->shaderStorageImageReadWithoutFormat,
                          available.shaderStorageImageReadWithoutFormat);
  ReportMismatchedFeature(captured_request->shaderStorageImageWriteWithoutFormat,
                          available.shaderStorageImageWriteWithoutFormat);
  ReportMismatchedFeature(captured_request->shaderUniformBufferArrayDynamicIndexing,
                          available.shaderUniformBufferArrayDynamicIndexing);
  ReportMismatchedFeature(captured_request->shaderSampledImageArrayDynamicIndexing,
                          available.shaderSampledImageArrayDynamicIndexing);
  ReportMismatchedFeature(captured_request->shaderStorageBufferArrayDynamicIndexing,
                          available.shaderStorageBufferArrayDynamicIndexing);
  ReportMismatchedFeature(captured_request->shaderStorageImageArrayDynamicIndexing,
                          available.shaderStorageImageArrayDynamicIndexing);
  ReportMismatchedFeature(captured_request->shaderClipDistance, available.shaderClipDistance);
  ReportMismatchedFeature(captured_request->shaderCullDistance, available.shaderCullDistance);
  ReportMismatchedFeature(captured_request->shaderFloat64, available.shaderFloat64);
  ReportMismatchedFeature(captured_request->shaderInt64, available.shaderInt64);
  ReportMismatchedFeature(captured_request->shaderInt16, available.shaderInt16);
  ReportMismatchedFeature(captured_request->shaderResourceResidency,
                          available.shaderResourceResidency);
  ReportMismatchedFeature(captured_request->shaderResourceMinLod, available.shaderResourceMinLod);
  ReportMismatchedFeature(captured_request->sparseBinding, available.sparseBinding);
  ReportMismatchedFeature(captured_request->sparseResidencyBuffer, available.sparseResidencyBuffer);
  ReportMismatchedFeature(captured_request->sparseResidencyImage2D, available.sparseResidencyImage2D);
  ReportMismatchedFeature(captured_request->sparseResidencyImage3D, available.sparseResidencyImage3D);
  ReportMismatchedFeature(captured_request->sparseResidency2Samples,
                          available.sparseResidency2Samples);
  ReportMismatchedFeature(captured_request->sparseResidency4Samples,
                          available.sparseResidency4Samples);
  ReportMismatchedFeature(captured_request->sparseResidency8Samples,
                          available.sparseResidency8Samples);
  ReportMismatchedFeature(captured_request->sparseResidency16Samples,
                          available.sparseResidency16Samples);
  ReportMismatchedFeature(captured_request->sparseResidencyAliased, available.sparseResidencyAliased);
  ReportMismatchedFeature(captured_request->variableMultisampleRate,
                          available.variableMultisampleRate);
  ReportMismatchedFeature(captured_request->inheritedQueries, available.inheritedQueries);
}

bool RegionsOverlap(const Region &r1, const Region &r2)
{
  // interval '1' and '2' start and end points:
  uint64_t i1_start = r1.offset;
  uint64_t i1_end = r1.offset + r1.size;
  uint64_t i2_start = r2.offset;
  uint64_t i2_end = r2.offset + r2.size;

  // two intervals i1 [s, e] and i2 [s, e] intersect
  // if X = max(i1.s, i2.s) < Y = min(i1.e, i2.e).
  return std::max<uint64_t>(i1_start, i2_start) < std::min<uint64_t>(i1_end, i2_end);
}

// RegionsIntersect(A, B) == RegionsIntersect(B, A)
Region RegionsIntersect(const Region &r1, const Region &r2)
{
  Region r;

  // two intervals i1 [s, e] and i2 [s, e] intersect
  // if X = max(i1.s, i2.s) < Y = min(i1.e, i2.e).
  r.offset = std::max<uint64_t>(r1.offset, r2.offset);
  r.size = std::min<uint64_t>(r1.offset + r1.size, r2.offset + r2.size) - r.offset;
  return r;
}

void MapUpdate(AuxVkTraceResources aux, uint8_t *dst, uint8_t *src, const VkMappedMemoryRange &range,
               VkMemoryAllocateInfo &ai, MemoryRemapVec &remap, VkDevice dev)
{
  if(dst != NULL)
  {
    std::vector<VkMappedMemoryRange> ranges;
    Region memory_region = {range.offset, range.size};
    assert(range.size != VK_WHOLE_SIZE);

    if(remap.size() > 0)
    {
      for(uint32_t i = 0; i < remap.size(); i++)
      {
        MemoryRemap mr = remap[i];
        Region captured_resource_region(mr.capture.offset, mr.capture.size);
        // If this memory range doesn't overlap with any captured resource continue
        if(!RegionsOverlap(memory_region, captured_resource_region))
          continue;

        // Find the inteval where these two regions overlap. It is guaranteed to be non-null.
        Region intersect = RegionsIntersect(memory_region, captured_resource_region);

        uint64_t skipped_resource_bytes = intersect.offset - mr.capture.offset;
        uint64_t skipped_memory_bytes = intersect.offset - memory_region.offset;
        intersect.size = std::min<uint64_t>(intersect.size, mr.replay.size);

        memcpy(dst + mr.replay.offset + skipped_resource_bytes, src + skipped_memory_bytes,
               intersect.size);

        VkMappedMemoryRange r = range;
        r.offset = mr.replay.offset + skipped_resource_bytes;
        r.size = AlignedSize(r.offset + intersect.size,
                             aux.physDeviceProperties.limits.nonCoherentAtomSize);
        r.offset = AlignedDown(r.offset, aux.physDeviceProperties.limits.nonCoherentAtomSize);
        r.size = r.size - r.offset;
        if(r.offset + r.size > range.offset + range.size || r.offset + r.size > ai.allocationSize)
        {
          r.size = VK_WHOLE_SIZE;
        }
        ranges.push_back(r);
      }
    }
    else
    {
      VkMappedMemoryRange r = range;
      r.size = std::min<uint64_t>(ai.allocationSize, range.size);
      memcpy(dst + r.offset, src, r.size);
      ranges.push_back(r);
    }

    VkResult result = vkFlushMappedMemoryRanges(dev, (uint32_t)ranges.size(), ranges.data());
    assert(result == VK_SUCCESS);
  }
}

bool IsExtEnabled(const char *const *extList, uint32_t count, const char *ext)
{
  for(uint32_t i = 0; i < count; i++)
  {
    if(strcmp(extList[i], ext) == 0)
      return true;
  }
  return false;
}

bool IsExtSupported(VkPhysicalDevice physicalDevice, const char *ext)
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extensionCount, NULL);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extensionCount, extensions.data());
  for(auto extension : extensions)
  {
    if(strcmp(extension.extensionName, ext) == 0)
    {
      return true;
    }
  }
  return false;
}

std::string StageProgressString(const char *stage, uint32_t i, uint32_t N)
{
  return std::string("RenderDoc Frame Loop: " + std::string(stage) + " part " + std::to_string(i) +
                     " of " + std::to_string(N));
})")},

    /******************************************************************************/
    /* TEMPLATE_FILE_HELPER_FORMAT_HELPER                                         */
    /******************************************************************************/
    {"helper", "format_helper.h",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: format_helper.h
//-----------------------------------------------------------------------------
#pragma once

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#if defined(_WIN32)
#include <Windows.h>
#endif

#include <algorithm>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "vulkan/vulkan.h"

extern std::map<VkFormat, std::pair<VkImageAspectFlags, uint32_t>> kImageAspectsAndByteSize;

double             SizeOfFormat(VkFormat fmt, VkImageAspectFlagBits aspect);
uint32_t           ChannelsInFormat(VkFormat fmt);
bool               isHDRFormat(VkFormat fmt);
bool               IsFPFormat(VkFormat fmt);
bool               IsSignedFormat(VkFormat fmt);
bool               IsDepthFormat(VkFormat fmt);
uint32_t           BitsPerChannelInFormat(VkFormat fmt, VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT);
VkImageAspectFlags FullAspectFromFormat(VkFormat fmt);
VkImageAspectFlags AspectFromFormat(VkFormat fmt);
uint32_t           FixCompressedSizes(VkFormat fmt, VkExtent3D &dim, uint32_t &offset);
std::string        FormatToString(VkFormat f);
uint32_t           MinDimensionSize(VkFormat format);
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_HELPER_FORMAT_SIZE_AND_ASPECT                                */
    /******************************************************************************/
    {"helper", "format_size_and_aspect.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: format_size_and_aspect.cpp
//-----------------------------------------------------------------------------
#include "format_helper.h"
#include "helper.h"

#define FORMAT_ASPECT_BYTE_SIZE(f, a, bs)                                                 \
  {                                                                                       \
    VK_FORMAT_##f, std::pair<VkImageAspectFlags, uint32_t>(VK_IMAGE_ASPECT_##a##_BIT, bs) \
  }

std::map<VkFormat, std::pair<VkImageAspectFlags, uint32_t>> kImageAspectsAndByteSize = {
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_USCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R5G5B5A1_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_UINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_SFLOAT, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_SNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R32_UINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_UINT, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R64_UINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32_UINT, COLOR, 12),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_SRGB, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_USCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_USCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_UNORM, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R64G64_UINT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SSCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8_UNORM, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_SNORM, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_UNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16_USCALED, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_SSCALED, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32A32_UINT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_UINT, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(R8_USCALED, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R8_SSCALED, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_USCALED, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(R4G4_UNORM_PACK8, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_UINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64_SINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_SSCALED, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SFLOAT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R64G64_SINT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_UINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_SSCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_UNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R32G32_UINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R4G4B4A4_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_SRGB, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_SNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_UNORM, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SSCALED, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R8_SINT, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R32_SFLOAT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SNORM, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_SINT, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_SNORM, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_SINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8_SRGB, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_USCALED, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SRGB, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16_UNORM, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SRGB, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_USCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_SINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_SNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_USCALED, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_UNORM, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_UINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_SSCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16_SNORM, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_SINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_SSCALED, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R8_UINT, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R64G64_SFLOAT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R16_SINT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32A32_SINT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R5G6B5_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_USCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64_SINT, COLOR, 24),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_UNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32_SINT, COLOR, 12),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_SNORM, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_UINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(B5G5R5A1_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_UINT, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_SINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16_SFLOAT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B4G4R4A4_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_UNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64_SFLOAT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(B10G11R11_UFLOAT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64A64_SFLOAT, COLOR, 32),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_USCALED, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32_SFLOAT, COLOR, 12),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_SINT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_SINT, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32A32_SFLOAT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_SSCALED, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_UNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R32_SINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64A64_SINT, COLOR, 32),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_SFLOAT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_UNORM, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_SSCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8_SNORM, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_UINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_SSCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64_SFLOAT, COLOR, 24),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_SRGB, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_SNORM, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(B5G6R5_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16_UINT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R32G32_SINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SSCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_SINT, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_UNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64_UINT, COLOR, 24),
    FORMAT_ASPECT_BYTE_SIZE(R16_SSCALED, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_SRGB_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A1R5G5B5_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_SNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_UINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_UINT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R32G32_SFLOAT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64A64_UINT, COLOR, 32),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_UNORM, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_USCALED, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_USCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(D32_SFLOAT, DEPTH, 4),
    FORMAT_ASPECT_BYTE_SIZE(D16_UNORM, DEPTH, 2),
    {VK_FORMAT_D16_UNORM_S8_UINT, std::pair<VkImageAspectFlags, uint32_t>(
                                      VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 3)},
    {VK_FORMAT_D24_UNORM_S8_UINT, std::pair<VkImageAspectFlags, uint32_t>(
                                      VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 4)},
    {VK_FORMAT_D32_SFLOAT_S8_UINT, std::pair<VkImageAspectFlags, uint32_t>(
                                       VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 5)},
};

std::string FormatToString(VkFormat f) {
#define RETURN_VK_FORMAT_STRING(x) \
  case VK_FORMAT_##x: return std::string(var_to_string(x));
)" + std::string(R"(
  switch (f) {
    RETURN_VK_FORMAT_STRING(UNDEFINED);
    RETURN_VK_FORMAT_STRING(R4G4_UNORM_PACK8);
    RETURN_VK_FORMAT_STRING(R4G4B4A4_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(B4G4R4A4_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R5G6B5_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(B5G6R5_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R5G5B5A1_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(B5G5R5A1_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(A1R5G5B5_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R8_UNORM);
    RETURN_VK_FORMAT_STRING(R8_SNORM);
    RETURN_VK_FORMAT_STRING(R8_USCALED);
    RETURN_VK_FORMAT_STRING(R8_SSCALED);
    RETURN_VK_FORMAT_STRING(R8_UINT);
    RETURN_VK_FORMAT_STRING(R8_SINT);
    RETURN_VK_FORMAT_STRING(R8_SRGB);
    RETURN_VK_FORMAT_STRING(R8G8_UNORM);
    RETURN_VK_FORMAT_STRING(R8G8_SNORM);
    RETURN_VK_FORMAT_STRING(R8G8_USCALED);
    RETURN_VK_FORMAT_STRING(R8G8_SSCALED);
    RETURN_VK_FORMAT_STRING(R8G8_UINT);
    RETURN_VK_FORMAT_STRING(R8G8_SINT);
    RETURN_VK_FORMAT_STRING(R8G8_SRGB);
    RETURN_VK_FORMAT_STRING(R8G8B8_UNORM);
    RETURN_VK_FORMAT_STRING(R8G8B8_SNORM);
    RETURN_VK_FORMAT_STRING(R8G8B8_USCALED);
    RETURN_VK_FORMAT_STRING(R8G8B8_SSCALED);
    RETURN_VK_FORMAT_STRING(R8G8B8_UINT);
    RETURN_VK_FORMAT_STRING(R8G8B8_SINT);
    RETURN_VK_FORMAT_STRING(R8G8B8_SRGB);
    RETURN_VK_FORMAT_STRING(B8G8R8_UNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8_SNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8_USCALED);
    RETURN_VK_FORMAT_STRING(B8G8R8_SSCALED);
    RETURN_VK_FORMAT_STRING(B8G8R8_UINT);
    RETURN_VK_FORMAT_STRING(B8G8R8_SINT);
    RETURN_VK_FORMAT_STRING(B8G8R8_SRGB);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_UNORM);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_SNORM);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_USCALED);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_SSCALED);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_UINT);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_SINT);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_SRGB);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_UNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_SNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_USCALED);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_SSCALED);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_UINT);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_SINT);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_SRGB);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_UNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_SNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_USCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_SSCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_UINT_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_SINT_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_SRGB_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_UNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_SNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_USCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_SSCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_UINT_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_SINT_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_UNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_SNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_USCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_SSCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_UINT_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_SINT_PACK32);
    RETURN_VK_FORMAT_STRING(R16_UNORM);
    RETURN_VK_FORMAT_STRING(R16_SNORM);
    RETURN_VK_FORMAT_STRING(R16_USCALED);
    RETURN_VK_FORMAT_STRING(R16_SSCALED);
    RETURN_VK_FORMAT_STRING(R16_UINT);
    RETURN_VK_FORMAT_STRING(R16_SINT);
    RETURN_VK_FORMAT_STRING(R16_SFLOAT);
    RETURN_VK_FORMAT_STRING(R16G16_UNORM);
    RETURN_VK_FORMAT_STRING(R16G16_SNORM);
    RETURN_VK_FORMAT_STRING(R16G16_USCALED);
    RETURN_VK_FORMAT_STRING(R16G16_SSCALED);
    RETURN_VK_FORMAT_STRING(R16G16_UINT);
    RETURN_VK_FORMAT_STRING(R16G16_SINT);
    RETURN_VK_FORMAT_STRING(R16G16_SFLOAT);
    RETURN_VK_FORMAT_STRING(R16G16B16_UNORM);
    RETURN_VK_FORMAT_STRING(R16G16B16_SNORM);
    RETURN_VK_FORMAT_STRING(R16G16B16_USCALED);
    RETURN_VK_FORMAT_STRING(R16G16B16_SSCALED);
    RETURN_VK_FORMAT_STRING(R16G16B16_UINT);
    RETURN_VK_FORMAT_STRING(R16G16B16_SINT);
    RETURN_VK_FORMAT_STRING(R16G16B16_SFLOAT);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_UNORM);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_SNORM);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_USCALED);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_SSCALED);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_UINT);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_SINT);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_SFLOAT);
    RETURN_VK_FORMAT_STRING(R32_UINT);
    RETURN_VK_FORMAT_STRING(R32_SINT);
    RETURN_VK_FORMAT_STRING(R32_SFLOAT);
    RETURN_VK_FORMAT_STRING(R32G32_UINT);
    RETURN_VK_FORMAT_STRING(R32G32_SINT);
    RETURN_VK_FORMAT_STRING(R32G32_SFLOAT);
    RETURN_VK_FORMAT_STRING(R32G32B32_UINT);
    RETURN_VK_FORMAT_STRING(R32G32B32_SINT);
    RETURN_VK_FORMAT_STRING(R32G32B32_SFLOAT);
    RETURN_VK_FORMAT_STRING(R32G32B32A32_UINT);
    RETURN_VK_FORMAT_STRING(R32G32B32A32_SINT);
    RETURN_VK_FORMAT_STRING(R32G32B32A32_SFLOAT);
    RETURN_VK_FORMAT_STRING(R64_UINT);
    RETURN_VK_FORMAT_STRING(R64_SINT);
    RETURN_VK_FORMAT_STRING(R64_SFLOAT);
    RETURN_VK_FORMAT_STRING(R64G64_UINT);
    RETURN_VK_FORMAT_STRING(R64G64_SINT);
    RETURN_VK_FORMAT_STRING(R64G64_SFLOAT);
    RETURN_VK_FORMAT_STRING(R64G64B64_UINT);
    RETURN_VK_FORMAT_STRING(R64G64B64_SINT);
    RETURN_VK_FORMAT_STRING(R64G64B64_SFLOAT);
    RETURN_VK_FORMAT_STRING(R64G64B64A64_UINT);
    RETURN_VK_FORMAT_STRING(R64G64B64A64_SINT);
    RETURN_VK_FORMAT_STRING(R64G64B64A64_SFLOAT);
    RETURN_VK_FORMAT_STRING(B10G11R11_UFLOAT_PACK32);
    RETURN_VK_FORMAT_STRING(E5B9G9R9_UFLOAT_PACK32);
    RETURN_VK_FORMAT_STRING(D16_UNORM);
    RETURN_VK_FORMAT_STRING(X8_D24_UNORM_PACK32);
    RETURN_VK_FORMAT_STRING(D32_SFLOAT);
    RETURN_VK_FORMAT_STRING(S8_UINT);
    RETURN_VK_FORMAT_STRING(D16_UNORM_S8_UINT);
    RETURN_VK_FORMAT_STRING(D24_UNORM_S8_UINT);
    RETURN_VK_FORMAT_STRING(D32_SFLOAT_S8_UINT);
    RETURN_VK_FORMAT_STRING(BC1_RGB_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC1_RGB_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(BC1_RGBA_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC1_RGBA_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(BC2_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC2_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(BC3_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC3_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(BC4_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC4_SNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC5_SNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC6H_UFLOAT_BLOCK);
    RETURN_VK_FORMAT_STRING(BC6H_SFLOAT_BLOCK);
    RETURN_VK_FORMAT_STRING(BC7_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC7_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8A1_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8A1_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8A8_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8A8_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(EAC_R11_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(EAC_R11_SNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(EAC_R11G11_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(EAC_R11G11_SNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_4x4_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_4x4_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_5x4_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_5x4_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_5x5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_5x5_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_6x5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_6x5_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_6x6_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_6x6_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x5_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x6_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x6_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x8_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x8_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x5_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x6_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x6_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x8_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x8_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x10_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x10_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_12x10_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_12x10_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_12x12_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_12x12_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(G8B8G8R8_422_UNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8G8_422_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8_R8_3PLANE_420_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8R8_2PLANE_420_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8_R8_3PLANE_422_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8R8_2PLANE_422_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8_R8_3PLANE_444_UNORM);
    RETURN_VK_FORMAT_STRING(R10X6_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R10X6G10X6_UNORM_2PACK16);
    RETURN_VK_FORMAT_STRING(R10X6G10X6B10X6A10X6_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(G10X6B10X6G10X6R10X6_422_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(B10X6G10X6R10X6G10X6_422_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(R12X4_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R12X4G12X4_UNORM_2PACK16);
    RETURN_VK_FORMAT_STRING(R12X4G12X4B12X4A12X4_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(G12X4B12X4G12X4R12X4_422_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(B12X4G12X4R12X4G12X4_422_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G16B16G16R16_422_UNORM);
    RETURN_VK_FORMAT_STRING(B16G16R16G16_422_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16_R16_3PLANE_420_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16R16_2PLANE_420_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16_R16_3PLANE_422_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16R16_2PLANE_422_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16_R16_3PLANE_444_UNORM);
    RETURN_VK_FORMAT_STRING(PVRTC1_2BPP_UNORM_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC1_4BPP_UNORM_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC2_2BPP_UNORM_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC2_4BPP_UNORM_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC1_2BPP_SRGB_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC1_4BPP_SRGB_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC2_2BPP_SRGB_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC2_4BPP_SRGB_BLOCK_IMG);

    default: assert(0);
  }

  return "ERROR";
}

uint32_t FixCompressedSizes(VkFormat fmt, VkExtent3D &dim, uint32_t &offset) {
  switch (fmt) {
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    dim.width = (uint32_t) AlignedSize(dim.width, 4);
    dim.height = (uint32_t) AlignedSize(dim.height, 4);
    dim.depth = (uint32_t) AlignedSize(dim.depth, 1);
    offset = (uint32_t) AlignedSize(offset, 4);
    return 1;
  }

  offset = (uint32_t) AlignedSize(offset, 4);
  return 0;
}

VkImageAspectFlags FullAspectFromFormat(VkFormat fmt) {
  if (fmt == VK_FORMAT_D16_UNORM || fmt == VK_FORMAT_D32_SFLOAT)
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  else if (fmt == VK_FORMAT_D16_UNORM_S8_UINT || fmt == VK_FORMAT_D24_UNORM_S8_UINT ||
    fmt == VK_FORMAT_D32_SFLOAT_S8_UINT)
    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  else    //  TODO(akharlamov): this is actually incorrect but OK for now
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageAspectFlags AspectFromFormat(VkFormat fmt) {
  if (fmt == VK_FORMAT_D16_UNORM || fmt == VK_FORMAT_D32_SFLOAT || fmt == VK_FORMAT_D16_UNORM_S8_UINT ||
    fmt == VK_FORMAT_D24_UNORM_S8_UINT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT)
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  else
    return VK_IMAGE_ASPECT_COLOR_BIT;
})") + std::string(R"(
double SizeOfFormat(VkFormat fmt, VkImageAspectFlagBits aspect) {
  if (aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
    switch (fmt) {
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_D16_UNORM_S8_UINT: return 1.0;
      default: assert(0);
    }
  } else if (aspect == VK_IMAGE_ASPECT_COLOR_BIT || aspect == VK_IMAGE_ASPECT_DEPTH_BIT) {
    switch (fmt) {
      case VK_FORMAT_R4G4_UNORM_PACK8:
      case VK_FORMAT_R8_UNORM:
      case VK_FORMAT_R8_SNORM:
      case VK_FORMAT_R8_USCALED:
      case VK_FORMAT_R8_SSCALED:
      case VK_FORMAT_R8_UINT:
      case VK_FORMAT_R8_SINT:
      case VK_FORMAT_R8_SRGB: return 1.0;

      case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
      case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
      case VK_FORMAT_R5G6B5_UNORM_PACK16:
      case VK_FORMAT_B5G6R5_UNORM_PACK16:
      case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
      case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
      case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
      case VK_FORMAT_R8G8_UNORM:
      case VK_FORMAT_R8G8_SNORM:
      case VK_FORMAT_R8G8_USCALED:
      case VK_FORMAT_R8G8_SSCALED:
      case VK_FORMAT_R8G8_UINT:
      case VK_FORMAT_R8G8_SINT:
      case VK_FORMAT_R8G8_SRGB:
      case VK_FORMAT_R16_UNORM:
      case VK_FORMAT_R16_SNORM:
      case VK_FORMAT_R16_USCALED:
      case VK_FORMAT_R16_SSCALED:
      case VK_FORMAT_R16_UINT:
      case VK_FORMAT_R16_SINT:
      case VK_FORMAT_R16_SFLOAT:
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_D16_UNORM_S8_UINT: return 2.0;

      case VK_FORMAT_R8G8B8_UNORM:
      case VK_FORMAT_R8G8B8_SNORM:
      case VK_FORMAT_R8G8B8_USCALED:
      case VK_FORMAT_R8G8B8_SSCALED:
      case VK_FORMAT_R8G8B8_UINT:
      case VK_FORMAT_R8G8B8_SINT:
      case VK_FORMAT_R8G8B8_SRGB:
      case VK_FORMAT_B8G8R8_UNORM:
      case VK_FORMAT_B8G8R8_SNORM:
      case VK_FORMAT_B8G8R8_USCALED:
      case VK_FORMAT_B8G8R8_SSCALED:
      case VK_FORMAT_B8G8R8_UINT:
      case VK_FORMAT_B8G8R8_SINT:
      case VK_FORMAT_B8G8R8_SRGB:
      case VK_FORMAT_D24_UNORM_S8_UINT: return 3.0;

      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_R8G8B8A8_SNORM:
      case VK_FORMAT_R8G8B8A8_USCALED:
      case VK_FORMAT_R8G8B8A8_SSCALED:
      case VK_FORMAT_R8G8B8A8_UINT:
      case VK_FORMAT_R8G8B8A8_SINT:
      case VK_FORMAT_R8G8B8A8_SRGB:
      case VK_FORMAT_B8G8R8A8_UNORM:
      case VK_FORMAT_B8G8R8A8_SNORM:
      case VK_FORMAT_B8G8R8A8_USCALED:
      case VK_FORMAT_B8G8R8A8_SSCALED:
      case VK_FORMAT_B8G8R8A8_UINT:
      case VK_FORMAT_B8G8R8A8_SINT:
      case VK_FORMAT_B8G8R8A8_SRGB:
      case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_UINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_UINT_PACK32:
      case VK_FORMAT_A2R10G10B10_SINT_PACK32:
      case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      case VK_FORMAT_A2B10G10R10_SINT_PACK32:
      case VK_FORMAT_R16G16_UNORM:
      case VK_FORMAT_R16G16_SNORM:
      case VK_FORMAT_R16G16_USCALED:
      case VK_FORMAT_R16G16_SSCALED:
      case VK_FORMAT_R16G16_UINT:
      case VK_FORMAT_R16G16_SINT:
      case VK_FORMAT_R16G16_SFLOAT:
      case VK_FORMAT_R32_UINT:
      case VK_FORMAT_R32_SINT:
      case VK_FORMAT_R32_SFLOAT:
      case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
      case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT: return 4.0;

      case VK_FORMAT_R16G16B16_UNORM:
      case VK_FORMAT_R16G16B16_SNORM:
      case VK_FORMAT_R16G16B16_USCALED:
      case VK_FORMAT_R16G16B16_SSCALED:
      case VK_FORMAT_R16G16B16_UINT:
      case VK_FORMAT_R16G16B16_SINT:
      case VK_FORMAT_R16G16B16_SFLOAT:
      return 6.0f;

      case VK_FORMAT_R16G16B16A16_UNORM:
      case VK_FORMAT_R16G16B16A16_SNORM:
      case VK_FORMAT_R16G16B16A16_USCALED:
      case VK_FORMAT_R16G16B16A16_SSCALED:
      case VK_FORMAT_R16G16B16A16_UINT:
      case VK_FORMAT_R16G16B16A16_SINT:
      case VK_FORMAT_R16G16B16A16_SFLOAT:
      case VK_FORMAT_R32G32_UINT:
      case VK_FORMAT_R32G32_SINT:
      case VK_FORMAT_R32G32_SFLOAT:
      case VK_FORMAT_R64_UINT:
      case VK_FORMAT_R64_SINT:
      case VK_FORMAT_R64_SFLOAT: return 8.0;

      case VK_FORMAT_R32G32B32_UINT:
      case VK_FORMAT_R32G32B32_SINT:
      case VK_FORMAT_R32G32B32_SFLOAT:

      return 12.0;

      case VK_FORMAT_R32G32B32A32_UINT:
      case VK_FORMAT_R32G32B32A32_SINT:
      case VK_FORMAT_R32G32B32A32_SFLOAT:
      case VK_FORMAT_R64G64_UINT:
      case VK_FORMAT_R64G64_SINT:
      case VK_FORMAT_R64G64_SFLOAT: return 16.0;

      case VK_FORMAT_R64G64B64_UINT:
      case VK_FORMAT_R64G64B64_SINT:
      case VK_FORMAT_R64G64B64_SFLOAT:
      return 24.0;

      case VK_FORMAT_R64G64B64A64_UINT:
      case VK_FORMAT_R64G64B64A64_SINT:
      case VK_FORMAT_R64G64B64A64_SFLOAT:
      return 32.0;

      case VK_FORMAT_BC2_SRGB_BLOCK:
      case VK_FORMAT_BC2_UNORM_BLOCK:
      case VK_FORMAT_BC3_SRGB_BLOCK:
      case VK_FORMAT_BC3_UNORM_BLOCK:
      case VK_FORMAT_BC5_SNORM_BLOCK:
      case VK_FORMAT_BC5_UNORM_BLOCK:
      case VK_FORMAT_BC6H_SFLOAT_BLOCK:
      case VK_FORMAT_BC6H_UFLOAT_BLOCK:
      case VK_FORMAT_BC7_SRGB_BLOCK:
      case VK_FORMAT_BC7_UNORM_BLOCK: return 1.0;

      case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
      case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
      case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
      case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
      case VK_FORMAT_BC4_SNORM_BLOCK:
      case VK_FORMAT_BC4_UNORM_BLOCK: return 0.5;

      default: assert(0);
    }
  }
  return 0.0;
}

uint32_t ChannelsInFormat(VkFormat fmt) {
    switch (fmt) {

      case VK_FORMAT_R8_UNORM:
      case VK_FORMAT_R8_SNORM:
      case VK_FORMAT_R8_USCALED:
      case VK_FORMAT_R8_SSCALED:
      case VK_FORMAT_R8_UINT:
      case VK_FORMAT_R8_SINT:
      case VK_FORMAT_R8_SRGB:
      case VK_FORMAT_R16_UNORM:
      case VK_FORMAT_R16_SNORM:
      case VK_FORMAT_R16_USCALED:
      case VK_FORMAT_R16_SSCALED:
      case VK_FORMAT_R16_UINT:
      case VK_FORMAT_R16_SINT:
      case VK_FORMAT_R16_SFLOAT:
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_D16_UNORM_S8_UINT:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_R32_UINT:
      case VK_FORMAT_R32_SINT:
      case VK_FORMAT_R32_SFLOAT:
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
      case VK_FORMAT_R64_UINT:
      case VK_FORMAT_R64_SINT:
      case VK_FORMAT_R64_SFLOAT:
      return 1;

      case VK_FORMAT_R4G4_UNORM_PACK8:
      case VK_FORMAT_R8G8_UNORM:
      case VK_FORMAT_R8G8_SNORM:
      case VK_FORMAT_R8G8_USCALED:
      case VK_FORMAT_R8G8_SSCALED:
      case VK_FORMAT_R8G8_UINT:
      case VK_FORMAT_R8G8_SINT:
      case VK_FORMAT_R8G8_SRGB:
      case VK_FORMAT_R16G16_UNORM:
      case VK_FORMAT_R16G16_SNORM:
      case VK_FORMAT_R16G16_USCALED:
      case VK_FORMAT_R16G16_SSCALED:
      case VK_FORMAT_R16G16_UINT:
      case VK_FORMAT_R16G16_SINT:
      case VK_FORMAT_R16G16_SFLOAT:
      case VK_FORMAT_R32G32_UINT:
      case VK_FORMAT_R32G32_SINT:
      case VK_FORMAT_R32G32_SFLOAT:
      case VK_FORMAT_R64G64_UINT:
      case VK_FORMAT_R64G64_SINT:
      case VK_FORMAT_R64G64_SFLOAT:
      return 2;

      case VK_FORMAT_R5G6B5_UNORM_PACK16:
      case VK_FORMAT_B5G6R5_UNORM_PACK16:
      case VK_FORMAT_R8G8B8_UNORM:
      case VK_FORMAT_R8G8B8_SNORM:
      case VK_FORMAT_R8G8B8_USCALED:
      case VK_FORMAT_R8G8B8_SSCALED:
      case VK_FORMAT_R8G8B8_UINT:
      case VK_FORMAT_R8G8B8_SINT:
      case VK_FORMAT_R8G8B8_SRGB:
      case VK_FORMAT_B8G8R8_UNORM:
      case VK_FORMAT_B8G8R8_SNORM:
      case VK_FORMAT_B8G8R8_USCALED:
      case VK_FORMAT_B8G8R8_SSCALED:
      case VK_FORMAT_B8G8R8_UINT:
      case VK_FORMAT_B8G8R8_SINT:
      case VK_FORMAT_B8G8R8_SRGB:
      case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
      case VK_FORMAT_R16G16B16_UNORM:
      case VK_FORMAT_R16G16B16_SNORM:
      case VK_FORMAT_R16G16B16_USCALED:
      case VK_FORMAT_R16G16B16_SSCALED:
      case VK_FORMAT_R16G16B16_UINT:
      case VK_FORMAT_R16G16B16_SINT:
      case VK_FORMAT_R16G16B16_SFLOAT:
      case VK_FORMAT_R32G32B32_UINT:
      case VK_FORMAT_R32G32B32_SINT:
      case VK_FORMAT_R32G32B32_SFLOAT:
      case VK_FORMAT_R64G64B64_UINT:
      case VK_FORMAT_R64G64B64_SINT:
      case VK_FORMAT_R64G64B64_SFLOAT:
      return 3;

      case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
      case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
      case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
      case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
      case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_R8G8B8A8_SNORM:
      case VK_FORMAT_R8G8B8A8_USCALED:
      case VK_FORMAT_R8G8B8A8_SSCALED:
      case VK_FORMAT_R8G8B8A8_UINT:
      case VK_FORMAT_R8G8B8A8_SINT:
      case VK_FORMAT_R8G8B8A8_SRGB:
      case VK_FORMAT_B8G8R8A8_UNORM:
      case VK_FORMAT_B8G8R8A8_SNORM:
      case VK_FORMAT_B8G8R8A8_USCALED:
      case VK_FORMAT_B8G8R8A8_SSCALED:
      case VK_FORMAT_B8G8R8A8_UINT:
      case VK_FORMAT_B8G8R8A8_SINT:
      case VK_FORMAT_B8G8R8A8_SRGB:
      case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_UINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_UINT_PACK32:
      case VK_FORMAT_A2R10G10B10_SINT_PACK32:
      case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      case VK_FORMAT_A2B10G10R10_SINT_PACK32:
      case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
      case VK_FORMAT_R16G16B16A16_UNORM:
      case VK_FORMAT_R16G16B16A16_SNORM:
      case VK_FORMAT_R16G16B16A16_USCALED:
      case VK_FORMAT_R16G16B16A16_SSCALED:
      case VK_FORMAT_R16G16B16A16_UINT:
      case VK_FORMAT_R16G16B16A16_SINT:
      case VK_FORMAT_R16G16B16A16_SFLOAT:
      case VK_FORMAT_R32G32B32A32_UINT:
      case VK_FORMAT_R32G32B32A32_SINT:
      case VK_FORMAT_R32G32B32A32_SFLOAT:
      case VK_FORMAT_R64G64B64A64_UINT:
      case VK_FORMAT_R64G64B64A64_SINT:
      case VK_FORMAT_R64G64B64A64_SFLOAT:
      return 4;

      default: assert(0);
    }
  return 0;
}

bool isHDRFormat(VkFormat fmt) {
  switch (fmt) {
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64B64A64_UINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    return true;

    default: return false;
  }
  return false;
}

uint32_t BitsPerChannelInFormat(VkFormat fmt, VkImageAspectFlagBits aspect) {
  return static_cast<uint32_t>(SizeOfFormat(fmt, aspect) * 8 / ChannelsInFormat(fmt));
}

bool IsFPFormat(VkFormat fmt) {
  switch (fmt) {
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    return true;
    default: return false;
  }
  return false;
}

bool IsSignedFormat(VkFormat fmt) {
  switch (fmt) {
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    return true;
    default: return false;
  }
  return false;
}

uint32_t MinDimensionSize(VkFormat format) {
  switch (format) {
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    return 4;

    default: return 1;
  }
}

bool IsDepthFormat(VkFormat fmt) {
  return
    (fmt == VK_FORMAT_D16_UNORM ||
     fmt == VK_FORMAT_D16_UNORM_S8_UINT ||
     fmt == VK_FORMAT_D24_UNORM_S8_UINT ||
     fmt == VK_FORMAT_D32_SFLOAT ||
     fmt == VK_FORMAT_D32_SFLOAT_S8_UINT);
}
)")},

    /******************************************************************************/
    /* TEMPLATE_FILE_HELPER_CMAKE                                                 */
    /******************************************************************************/
    {"helper", "CMakeLists.txt",
     R"(SET (THIS_PROJECT_NAME helper)
PROJECT(${THIS_PROJECT_NAME})

ADD_LIBRARY(${THIS_PROJECT_NAME} STATIC "helper.h" "helper.cpp"
             format_helper.h format_size_and_aspect.cpp)

TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME}
    PRIVATE UNICODE _UNICODE)
IF (NOT WIN32)
    TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME}
        PRIVATE HELPER_COMPILE_STATIC_LIB)
ENDIF ()

TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME} vulkan)

SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                      OUTPUT_NAME ${THIS_PROJECT_NAME}
                      ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      POSITION_INDEPENDENT_CODE ON)
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_SHIM_H                                                       */
    /******************************************************************************/
    {"sample_cpp_shim", "shim_vulkan.h",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: shim_vulkan.h
//-----------------------------------------------------------------------------
#pragma once
#if defined(_WIN32)
#define SHIM_VK_API_IMPORT __declspec(dllimport)
#define SHIM_VK_API_EXPORT __declspec(dllexport)
#else
#define SHIM_VK_API_IMPORT __attribute__((visibility("default")))
#define SHIM_VK_API_EXPORT __attribute__((visibility("default")))
#endif
#if defined(SHIM_VK_COMPILE_STATIC_LIB)
#define SHIM_VK_API
#else
#if defined(SHIM_VK_EXPORT)
#define SHIM_VK_API SHIM_VK_API_EXPORT
#else
#define SHIM_VK_API SHIM_VK_API_IMPORT
#endif
#endif
#include "vulkan/vulkan.h"

SHIM_VK_API VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator,
                                           VkInstance *pInstance);

SHIM_VK_API VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice,
                                         const VkDeviceCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);

SHIM_VK_API void shim_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkEnumeratePhysicalDevices(VkInstance instance,
                                                     uint32_t *pPhysicalDeviceCount,
                                                     VkPhysicalDevice *pPhysicalDevices);

SHIM_VK_API void shim_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties *pQueueFamilyProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties *pMemoryProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceFeatures *pFeatures);

SHIM_VK_API void shim_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice,
                                                          VkFormat format,
                                                          VkFormatProperties *pFormatProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkImageFormatProperties *pImageFormatProperties);

SHIM_VK_API void shim_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkEnumerateInstanceVersion(uint32_t *pApiVersion);

SHIM_VK_API VkResult shim_vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                                             VkLayerProperties *pProperties);

SHIM_VK_API VkResult shim_vkEnumerateInstanceExtensionProperties(const char *pLayerName,
                                                                 uint32_t *pPropertyCount,
                                                                 VkExtensionProperties *pProperties);

SHIM_VK_API VkResult shim_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                                                           uint32_t *pPropertyCount,
                                                           VkLayerProperties *pProperties);

SHIM_VK_API VkResult shim_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                               const char *pLayerName,
                                                               uint32_t *pPropertyCount,
                                                               VkExtensionProperties *pProperties);

SHIM_VK_API void shim_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex,
                                       uint32_t queueIndex, VkQueue *pQueue);

SHIM_VK_API VkResult shim_vkQueueSubmit(VkQueue queue, uint32_t submitCount,
                                        const VkSubmitInfo *pSubmits, VkFence fence);

SHIM_VK_API VkResult shim_vkQueueWaitIdle(VkQueue queue);

SHIM_VK_API VkResult shim_vkDeviceWaitIdle(VkDevice device);

SHIM_VK_API VkResult shim_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                           const VkAllocationCallbacks *pAllocator,
                                           VkDeviceMemory *pMemory);

SHIM_VK_API void shim_vkFreeMemory(VkDevice device, VkDeviceMemory memory,
                                   const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset,
                                      VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);

SHIM_VK_API void shim_vkUnmapMemory(VkDevice device, VkDeviceMemory memory);

SHIM_VK_API VkResult shim_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                    const VkMappedMemoryRange *pMemoryRanges);

SHIM_VK_API VkResult shim_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                         const VkMappedMemoryRange *pMemoryRanges);

SHIM_VK_API void shim_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                  VkDeviceSize *pCommittedMemoryInBytes);

SHIM_VK_API void shim_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                    VkMemoryRequirements *pMemoryRequirements);

SHIM_VK_API VkResult shim_vkBindBufferMemory(VkDevice device, VkBuffer buffer,
                                             VkDeviceMemory memory, VkDeviceSize memoryOffset);

SHIM_VK_API void shim_vkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                   VkMemoryRequirements *pMemoryRequirements);

SHIM_VK_API VkResult shim_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                            VkDeviceSize memoryOffset);

SHIM_VK_API void shim_vkGetImageSparseMemoryRequirements(
    VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements *pSparseMemoryRequirements);

SHIM_VK_API void shim_vkGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
    VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties);

SHIM_VK_API VkResult shim_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount,
                                            const VkBindSparseInfo *pBindInfo, VkFence fence);

SHIM_VK_API VkResult shim_vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkFence *pFence);

SHIM_VK_API void shim_vkDestroyFence(VkDevice device, VkFence fence,
                                     const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences);

SHIM_VK_API VkResult shim_vkGetFenceStatus(VkDevice device, VkFence fence);

SHIM_VK_API VkResult shim_vkWaitForFences(VkDevice device, uint32_t fenceCount,
                                          const VkFence *pFences, VkBool32 waitAll, uint64_t timeout);

SHIM_VK_API VkResult shim_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkSemaphore *pSemaphore);

SHIM_VK_API void shim_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkEvent *pEvent);

SHIM_VK_API void shim_vkDestroyEvent(VkDevice device, VkEvent event,
                                     const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetEventStatus(VkDevice device, VkEvent event);

SHIM_VK_API VkResult shim_vkSetEvent(VkDevice device, VkEvent event);

SHIM_VK_API VkResult shim_vkResetEvent(VkDevice device, VkEvent event);

SHIM_VK_API VkResult shim_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkQueryPool *pQueryPool);

SHIM_VK_API void shim_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool,
                                                uint32_t firstQuery, uint32_t queryCount,
                                                size_t dataSize, void *pData, VkDeviceSize stride,
                                                VkQueryResultFlags flags);

SHIM_VK_API VkResult shim_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer);

SHIM_VK_API void shim_vkDestroyBuffer(VkDevice device, VkBuffer buffer,
                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateBufferView(VkDevice device,
                                             const VkBufferViewCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkBufferView *pView);

SHIM_VK_API void shim_vkDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                          const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkImage *pImage);

SHIM_VK_API void shim_vkDestroyImage(VkDevice device, VkImage image,
                                     const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkGetImageSubresourceLayout(VkDevice device, VkImage image,
                                                  const VkImageSubresource *pSubresource,
                                                  VkSubresourceLayout *pLayout);

SHIM_VK_API VkResult shim_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkImageView *pView);

SHIM_VK_API void shim_vkDestroyImageView(VkDevice device, VkImageView imageView,
                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateShaderModule(VkDevice device,
                                               const VkShaderModuleCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkShaderModule *pShaderModule);

SHIM_VK_API void shim_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                            const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreatePipelineCache(VkDevice device,
                                                const VkPipelineCacheCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator,
                                                VkPipelineCache *pPipelineCache);

SHIM_VK_API void shim_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                             const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache,
                                                 size_t *pDataSize, void *pData);

SHIM_VK_API VkResult shim_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache,
                                                uint32_t srcCacheCount,
                                                const VkPipelineCache *pSrcCaches);

SHIM_VK_API VkResult shim_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                    uint32_t createInfoCount,
                                                    const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkPipeline *pPipelines);

SHIM_VK_API VkResult shim_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                   uint32_t createInfoCount,
                                                   const VkComputePipelineCreateInfo *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator,
                                                   VkPipeline *pPipelines);

SHIM_VK_API void shim_vkDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                        const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreatePipelineLayout(VkDevice device,
                                                 const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkPipelineLayout *pPipelineLayout);

SHIM_VK_API void shim_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                              const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkSampler *pSampler);

SHIM_VK_API void shim_vkDestroySampler(VkDevice device, VkSampler sampler,
                                       const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateDescriptorSetLayout(
    VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout);

SHIM_VK_API void shim_vkDestroyDescriptorSetLayout(VkDevice device,
                                                   VkDescriptorSetLayout descriptorSetLayout,
                                                   const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateDescriptorPool(VkDevice device,
                                                 const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDescriptorPool *pDescriptorPool);

SHIM_VK_API void shim_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                              const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                VkDescriptorPoolResetFlags flags);

SHIM_VK_API VkResult shim_vkAllocateDescriptorSets(VkDevice device,
                                                   const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                   VkDescriptorSet *pDescriptorSets);
)" + std::string(R"(
SHIM_VK_API VkResult shim_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                               uint32_t descriptorSetCount,
                                               const VkDescriptorSet *pDescriptorSets);

SHIM_VK_API void shim_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet *pDescriptorWrites,
                                             uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet *pDescriptorCopies);

SHIM_VK_API VkResult shim_vkCreateFramebuffer(VkDevice device,
                                              const VkFramebufferCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator,
                                              VkFramebuffer *pFramebuffer);

SHIM_VK_API void shim_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                           const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateRenderPass(VkDevice device,
                                             const VkRenderPassCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkRenderPass *pRenderPass);

SHIM_VK_API void shim_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                          const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass,
                                                 VkExtent2D *pGranularity);

SHIM_VK_API VkResult shim_vkCreateCommandPool(VkDevice device,
                                              const VkCommandPoolCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator,
                                              VkCommandPool *pCommandPool);

SHIM_VK_API void shim_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                           const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                             VkCommandPoolResetFlags flags);

SHIM_VK_API VkResult shim_vkAllocateCommandBuffers(VkDevice device,
                                                   const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                   VkCommandBuffer *pCommandBuffers);

SHIM_VK_API void shim_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                           uint32_t commandBufferCount,
                                           const VkCommandBuffer *pCommandBuffers);

SHIM_VK_API VkResult shim_vkBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                               const VkCommandBufferBeginInfo *pBeginInfo);

SHIM_VK_API VkResult shim_vkEndCommandBuffer(VkCommandBuffer commandBuffer);

SHIM_VK_API VkResult shim_vkResetCommandBuffer(VkCommandBuffer commandBuffer,
                                               VkCommandBufferResetFlags flags);

SHIM_VK_API void shim_vkCmdBindPipeline(VkCommandBuffer commandBuffer,
                                        VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);

SHIM_VK_API void shim_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                       uint32_t viewportCount, const VkViewport *pViewports);

SHIM_VK_API void shim_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                      uint32_t scissorCount, const VkRect2D *pScissors);

SHIM_VK_API void shim_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);

SHIM_VK_API void shim_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                                        float depthBiasClamp, float depthBiasSlopeFactor);

SHIM_VK_API void shim_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer,
                                             const float blendConstants[4]);

SHIM_VK_API void shim_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                                          float maxDepthBounds);

SHIM_VK_API void shim_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer,
                                                 VkStencilFaceFlags faceMask, uint32_t compareMask);

SHIM_VK_API void shim_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer,
                                               VkStencilFaceFlags faceMask, uint32_t writeMask);

SHIM_VK_API void shim_vkCmdSetStencilReference(VkCommandBuffer commandBuffer,
                                               VkStencilFaceFlags faceMask, uint32_t reference);

SHIM_VK_API void shim_vkCmdBindDescriptorSets(
    VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
    uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets,
    uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets);

SHIM_VK_API void shim_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                           VkDeviceSize offset, VkIndexType indexType);

SHIM_VK_API void shim_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                             uint32_t bindingCount, const VkBuffer *pBuffers,
                                             const VkDeviceSize *pOffsets);

SHIM_VK_API void shim_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount,
                                uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

SHIM_VK_API void shim_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                       uint32_t instanceCount, uint32_t firstIndex,
                                       int32_t vertexOffset, uint32_t firstInstance);

SHIM_VK_API void shim_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                        VkDeviceSize offset, uint32_t drawCount, uint32_t stride);

SHIM_VK_API void shim_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                               VkDeviceSize offset, uint32_t drawCount,
                                               uint32_t stride);

SHIM_VK_API void shim_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                    uint32_t groupCountY, uint32_t groupCountZ);

SHIM_VK_API void shim_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                            VkDeviceSize offset);

SHIM_VK_API void shim_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                      VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy *pRegions);

SHIM_VK_API void shim_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                     VkImageLayout srcImageLayout, VkImage dstImage,
                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy *pRegions);

SHIM_VK_API void shim_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                     VkImageLayout srcImageLayout, VkImage dstImage,
                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit *pRegions, VkFilter filter);

SHIM_VK_API void shim_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                             VkImage dstImage, VkImageLayout dstImageLayout,
                                             uint32_t regionCount, const VkBufferImageCopy *pRegions);

SHIM_VK_API void shim_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                             VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                             uint32_t regionCount, const VkBufferImageCopy *pRegions);

SHIM_VK_API void shim_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                        VkDeviceSize dstOffset, VkDeviceSize dataSize,
                                        const void *pData);

SHIM_VK_API void shim_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                      VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);

SHIM_VK_API void shim_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                           VkImageLayout imageLayout,
                                           const VkClearColorValue *pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange *pRanges);

SHIM_VK_API void shim_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                  VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue *pDepthStencil,
                                                  uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges);

SHIM_VK_API void shim_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment *pAttachments,
                                            uint32_t rectCount, const VkClearRect *pRects);

SHIM_VK_API void shim_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                        VkImageLayout srcImageLayout, VkImage dstImage,
                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve *pRegions);

SHIM_VK_API void shim_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                    VkPipelineStageFlags stageMask);

SHIM_VK_API void shim_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                      VkPipelineStageFlags stageMask);

SHIM_VK_API void shim_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                      const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                                      VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                      const VkMemoryBarrier *pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount,
                                      const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount,
                                      const VkImageMemoryBarrier *pImageMemoryBarriers);

SHIM_VK_API void shim_vkCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);

SHIM_VK_API void shim_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                      uint32_t query, VkQueryControlFlags flags);

SHIM_VK_API void shim_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                    uint32_t query);

SHIM_VK_API void shim_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                          uint32_t firstQuery, uint32_t queryCount);

SHIM_VK_API void shim_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer,
                                          VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t query);

SHIM_VK_API void shim_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                uint32_t firstQuery, uint32_t queryCount,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags);

SHIM_VK_API void shim_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                         VkShaderStageFlags stageFlags, uint32_t offset,
                                         uint32_t size, const void *pValues);

SHIM_VK_API void shim_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                                           const VkRenderPassBeginInfo *pRenderPassBegin,
                                           VkSubpassContents contents);

SHIM_VK_API void shim_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);

SHIM_VK_API void shim_vkCmdEndRenderPass(VkCommandBuffer commandBuffer);

SHIM_VK_API void shim_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer *pCommandBuffers);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPropertiesKHR *pProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
    VkDisplayPlanePropertiesKHR *pProperties);

SHIM_VK_API VkResult shim_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice,
                                                                uint32_t planeIndex,
                                                                uint32_t *pDisplayCount,
                                                                VkDisplayKHR *pDisplays);

SHIM_VK_API VkResult shim_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                        VkDisplayKHR display,
                                                        uint32_t *pPropertyCount,
                                                        VkDisplayModePropertiesKHR *pProperties);

SHIM_VK_API VkResult shim_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice,
                                                 VkDisplayKHR display,
                                                 const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDisplayModeKHR *pMode);

SHIM_VK_API VkResult shim_vkGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
    VkDisplayPlaneCapabilitiesKHR *pCapabilities);

SHIM_VK_API VkResult shim_vkCreateDisplayPlaneSurfaceKHR(
    VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);

SHIM_VK_API VkResult shim_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                      const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator,
                                                      VkSwapchainKHR *pSwapchains);

SHIM_VK_API void shim_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                          const VkAllocationCallbacks *pAllocator);
)") + std::string(R"(
SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                               uint32_t queueFamilyIndex,
                                                               VkSurfaceKHR surface,
                                                               VkBool32 *pSupported);

SHIM_VK_API VkResult
shim_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                               VkSurfaceCapabilitiesKHR *pSurfaceCapabilities);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                                                               VkSurfaceKHR surface,
                                                               uint32_t *pSurfaceFormatCount,
                                                               VkSurfaceFormatKHR *pSurfaceFormats);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                    VkSurfaceKHR surface,
                                                                    uint32_t *pPresentModeCount,
                                                                    VkPresentModeKHR *pPresentModes);

SHIM_VK_API VkResult shim_vkCreateSwapchainKHR(VkDevice device,
                                               const VkSwapchainCreateInfoKHR *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkSwapchainKHR *pSwapchain);

SHIM_VK_API void shim_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                            const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                  uint32_t *pSwapchainImageCount,
                                                  VkImage *pSwapchainImages);

SHIM_VK_API VkResult shim_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                uint64_t timeout, VkSemaphore semaphore,
                                                VkFence fence, uint32_t *pImageIndex);

SHIM_VK_API VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo);

SHIM_VK_API VkResult shim_vkCreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback);

SHIM_VK_API void shim_vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                                      VkDebugReportCallbackEXT callback,
                                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                              VkDebugReportObjectTypeEXT objectType,
                                              uint64_t object, size_t location, int32_t messageCode,
                                              const char *pLayerPrefix, const char *pMessage);

SHIM_VK_API VkResult
shim_vkDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo);

SHIM_VK_API VkResult shim_vkDebugMarkerSetObjectTagEXT(VkDevice device,
                                                       const VkDebugMarkerObjectTagInfoEXT *pTagInfo);

SHIM_VK_API void shim_vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                               const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);

SHIM_VK_API void shim_vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);

SHIM_VK_API void shim_vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
                                                const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties);

SHIM_VK_API void shim_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                VkDeviceSize offset, VkBuffer countBuffer,
                                                VkDeviceSize countBufferOffset,
                                                uint32_t maxDrawCount, uint32_t stride);

SHIM_VK_API void shim_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                       VkDeviceSize offset, VkBuffer countBuffer,
                                                       VkDeviceSize countBufferOffset,
                                                       uint32_t maxDrawCount, uint32_t stride);

SHIM_VK_API void shim_vkCmdProcessCommandsNVX(VkCommandBuffer commandBuffer,
                                              const VkCmdProcessCommandsInfoNVX *pProcessCommandsInfo);

SHIM_VK_API void shim_vkCmdReserveSpaceForCommandsNVX(
    VkCommandBuffer commandBuffer, const VkCmdReserveSpaceForCommandsInfoNVX *pReserveSpaceInfo);

SHIM_VK_API VkResult shim_vkCreateIndirectCommandsLayoutNVX(
    VkDevice device, const VkIndirectCommandsLayoutCreateInfoNVX *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNVX *pIndirectCommandsLayout);

SHIM_VK_API void shim_vkDestroyIndirectCommandsLayoutNVX(
    VkDevice device, VkIndirectCommandsLayoutNVX indirectCommandsLayout,
    const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateObjectTableNVX(VkDevice device,
                                                 const VkObjectTableCreateInfoNVX *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkObjectTableNVX *pObjectTable);

SHIM_VK_API void shim_vkDestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable,
                                              const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkRegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                               uint32_t objectCount,
                                               const VkObjectTableEntryNVX *const *ppObjectTableEntries,
                                               const uint32_t *pObjectIndices);

SHIM_VK_API VkResult shim_vkUnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                                 uint32_t objectCount,
                                                 const VkObjectEntryTypeNVX *pObjectEntryTypes,
                                                 const uint32_t *pObjectIndices);

SHIM_VK_API void shim_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice physicalDevice, VkDeviceGeneratedCommandsFeaturesNVX *pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX *pLimits);

SHIM_VK_API void shim_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceFeatures2 *pFeatures);

SHIM_VK_API void shim_vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                      VkPhysicalDeviceFeatures2 *pFeatures);

SHIM_VK_API void shim_vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                     VkPhysicalDeviceProperties2 *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                        VkPhysicalDeviceProperties2 *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice,
                                                           VkFormat format,
                                                           VkFormatProperties2 *pFormatProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                              VkFormat format,
                                                              VkFormatProperties2 *pFormatProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2 *pQueueFamilyProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2 *pQueueFamilyProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);

SHIM_VK_API void shim_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                                VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set,
                                                uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet *pDescriptorWrites);

SHIM_VK_API void shim_vkTrimCommandPool(VkDevice device, VkCommandPool commandPool,
                                        VkCommandPoolTrimFlags flags);

SHIM_VK_API void shim_vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool,
                                           VkCommandPoolTrimFlags flags);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties);

SHIM_VK_API VkResult shim_vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo,
                                           int *pFd);

SHIM_VK_API VkResult shim_vkGetMemoryFdPropertiesKHR(VkDevice device,
                                                     VkExternalMemoryHandleTypeFlagBits handleType,
                                                     int fd,
                                                     VkMemoryFdPropertiesKHR *pMemoryFdProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties);

SHIM_VK_API VkResult shim_vkGetSemaphoreFdKHR(VkDevice device,
                                              const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd);

SHIM_VK_API VkResult shim_vkImportSemaphoreFdKHR(
    VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties);

SHIM_VK_API VkResult shim_vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo,
                                          int *pFd);

SHIM_VK_API VkResult shim_vkImportFenceFdKHR(VkDevice device,
                                             const VkImportFenceFdInfoKHR *pImportFenceFdInfo);

SHIM_VK_API VkResult shim_vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);

SHIM_VK_API VkResult shim_vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                   const VkDisplayPowerInfoEXT *pDisplayPowerInfo);

SHIM_VK_API VkResult shim_vkRegisterDeviceEventEXT(VkDevice device,
                                                   const VkDeviceEventInfoEXT *pDeviceEventInfo,
                                                   const VkAllocationCallbacks *pAllocator,
                                                   VkFence *pFence);

SHIM_VK_API VkResult shim_vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                    const VkDisplayEventInfoEXT *pDisplayEventInfo,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkFence *pFence);

SHIM_VK_API VkResult shim_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                   VkSurfaceCounterFlagBitsEXT counter,
                                                   uint64_t *pCounterValue);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
    VkSurfaceCapabilities2EXT *pSurfaceCapabilities);

SHIM_VK_API VkResult shim_vkEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);

SHIM_VK_API VkResult shim_vkEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);

SHIM_VK_API void shim_vkGetDeviceGroupPeerMemoryFeatures(
    VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
    VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);

SHIM_VK_API void shim_vkGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
    VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);

SHIM_VK_API VkResult shim_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                              const VkBindBufferMemoryInfo *pBindInfos);

SHIM_VK_API VkResult shim_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindBufferMemoryInfo *pBindInfos);

SHIM_VK_API VkResult shim_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                             const VkBindImageMemoryInfo *pBindInfos);
)") + std::string(R"(
SHIM_VK_API VkResult shim_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                const VkBindImageMemoryInfo *pBindInfos);

SHIM_VK_API void shim_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);

SHIM_VK_API void shim_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);

SHIM_VK_API VkResult shim_vkGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities);

SHIM_VK_API VkResult shim_vkGetDeviceGroupSurfacePresentModesKHR(
    VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR *pModes);

SHIM_VK_API VkResult shim_vkAcquireNextImage2KHR(VkDevice device,
                                                 const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                 uint32_t *pImageIndex);

SHIM_VK_API void shim_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                        uint32_t baseGroupY, uint32_t baseGroupZ,
                                        uint32_t groupCountX, uint32_t groupCountY,
                                        uint32_t groupCountZ);

SHIM_VK_API void shim_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                           uint32_t baseGroupY, uint32_t baseGroupZ,
                                           uint32_t groupCountX, uint32_t groupCountY,
                                           uint32_t groupCountZ);

SHIM_VK_API VkResult shim_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice,
                                                                  VkSurfaceKHR surface,
                                                                  uint32_t *pRectCount,
                                                                  VkRect2D *pRects);

SHIM_VK_API VkResult shim_vkCreateDescriptorUpdateTemplate(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);

SHIM_VK_API VkResult shim_vkCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);

SHIM_VK_API void shim_vkDestroyDescriptorUpdateTemplate(
    VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkDestroyDescriptorUpdateTemplateKHR(
    VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkUpdateDescriptorSetWithTemplate(
    VkDevice device, VkDescriptorSet descriptorSet,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);

SHIM_VK_API void shim_vkUpdateDescriptorSetWithTemplateKHR(
    VkDevice device, VkDescriptorSet descriptorSet,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);

SHIM_VK_API void shim_vkCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    VkPipelineLayout layout, uint32_t set, const void *pData);

SHIM_VK_API void shim_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount,
                                          const VkSwapchainKHR *pSwapchains,
                                          const VkHdrMetadataEXT *pMetadata);

SHIM_VK_API VkResult shim_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);

SHIM_VK_API VkResult
shim_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                     VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties);

SHIM_VK_API VkResult shim_vkGetPastPresentationTimingGOOGLE(
    VkDevice device, VkSwapchainKHR swapchain, uint32_t *pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE *pPresentationTimings);

SHIM_VK_API void shim_vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer,
                                                 uint32_t firstViewport, uint32_t viewportCount,
                                                 const VkViewportWScalingNV *pViewportWScalings);

SHIM_VK_API void shim_vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer,
                                                  uint32_t firstDiscardRectangle,
                                                  uint32_t discardRectangleCount,
                                                  const VkRect2D *pDiscardRectangles);

SHIM_VK_API void shim_vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                 const VkSampleLocationsInfoEXT *pSampleLocationsInfo);

SHIM_VK_API void shim_vkGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
    VkMultisamplePropertiesEXT *pMultisampleProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    uint32_t *pSurfaceFormatCount, VkSurfaceFormat2KHR *pSurfaceFormats);

SHIM_VK_API void shim_vkGetBufferMemoryRequirements2(VkDevice device,
                                                     const VkBufferMemoryRequirementsInfo2 *pInfo,
                                                     VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetBufferMemoryRequirements2KHR(VkDevice device,
                                                        const VkBufferMemoryRequirementsInfo2 *pInfo,
                                                        VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetImageMemoryRequirements2(VkDevice device,
                                                    const VkImageMemoryRequirementsInfo2 *pInfo,
                                                    VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetImageMemoryRequirements2KHR(VkDevice device,
                                                       const VkImageMemoryRequirementsInfo2 *pInfo,
                                                       VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetImageSparseMemoryRequirements2(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);

SHIM_VK_API void shim_vkGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);

SHIM_VK_API VkResult shim_vkCreateSamplerYcbcrConversion(
    VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);

SHIM_VK_API VkResult shim_vkCreateSamplerYcbcrConversionKHR(
    VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);

SHIM_VK_API void shim_vkDestroySamplerYcbcrConversion(VkDevice device,
                                                      VkSamplerYcbcrConversion ycbcrConversion,
                                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkDestroySamplerYcbcrConversionKHR(VkDevice device,
                                                         VkSamplerYcbcrConversion ycbcrConversion,
                                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo,
                                        VkQueue *pQueue);

SHIM_VK_API VkResult shim_vkCreateValidationCacheEXT(VkDevice device,
                                                     const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator,
                                                     VkValidationCacheEXT *pValidationCache);

SHIM_VK_API void shim_vkDestroyValidationCacheEXT(VkDevice device,
                                                  VkValidationCacheEXT validationCache,
                                                  const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetValidationCacheDataEXT(VkDevice device,
                                                      VkValidationCacheEXT validationCache,
                                                      size_t *pDataSize, void *pData);

SHIM_VK_API VkResult shim_vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache,
                                                     uint32_t srcCacheCount,
                                                     const VkValidationCacheEXT *pSrcCaches);

SHIM_VK_API void shim_vkGetDescriptorSetLayoutSupport(VkDevice device,
                                                      const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                      VkDescriptorSetLayoutSupport *pSupport);

SHIM_VK_API void shim_vkGetDescriptorSetLayoutSupportKHR(
    VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    VkDescriptorSetLayoutSupport *pSupport);

SHIM_VK_API VkResult shim_vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline,
                                             VkShaderStageFlagBits shaderStage,
                                             VkShaderInfoTypeAMD infoType, size_t *pInfoSize,
                                             void *pInfo);

SHIM_VK_API VkResult shim_vkSetDebugUtilsObjectNameEXT(VkDevice device,
                                                       const VkDebugUtilsObjectNameInfoEXT *pNameInfo);

SHIM_VK_API VkResult shim_vkSetDebugUtilsObjectTagEXT(VkDevice device,
                                                      const VkDebugUtilsObjectTagInfoEXT *pTagInfo);

SHIM_VK_API void shim_vkQueueBeginDebugUtilsLabelEXT(VkQueue queue,
                                                     const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API void shim_vkQueueEndDebugUtilsLabelEXT(VkQueue queue);

SHIM_VK_API void shim_vkQueueInsertDebugUtilsLabelEXT(VkQueue queue,
                                                      const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API void shim_vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                   const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API void shim_vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);

SHIM_VK_API void shim_vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                    const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API VkResult shim_vkCreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger);

SHIM_VK_API void shim_vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                      VkDebugUtilsMessengerEXT messenger,
                                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkSubmitDebugUtilsMessageEXT(
    VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);

SHIM_VK_API VkResult shim_vkGetMemoryHostPointerPropertiesEXT(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void *pHostPointer,
    VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties);

SHIM_VK_API void shim_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer,
                                                VkPipelineStageFlagBits pipelineStage,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                uint32_t marker);

)")},

    /******************************************************************************/
    /* TEMPLATE_FILE_SHIM_CPP                                                     */
    /******************************************************************************/
    {"sample_cpp_shim", "shim_vulkan.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: shim_vulkan.cpp
//-----------------------------------------------------------------------------
#ifndef SHIM_VK_COMPILE_STATIC_LIB
#define SHIM_VK_EXPORT
#endif
#include "helper/helper.h"
#include "shim_vulkan.h"

AuxVkTraceResources aux;

VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance)
{
  VkResult r = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
  aux.instance = *pInstance;
  return r;
}

VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
  VkResult r = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
  InitializeAuxResources(&aux, aux.instance, physicalDevice, *pDevice);
  return r;
}

void shim_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyInstance fn = vkDestroyInstance;
  fn(instance, pAllocator);
  return;
}

VkResult shim_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                         VkPhysicalDevice *pPhysicalDevices)
{
  static PFN_vkEnumeratePhysicalDevices fn = vkEnumeratePhysicalDevices;
  VkResult r = fn(instance, pPhysicalDeviceCount, pPhysicalDevices);
  return r;
}

void shim_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                        VkPhysicalDeviceProperties *pProperties)
{
  static PFN_vkGetPhysicalDeviceProperties fn = vkGetPhysicalDeviceProperties;
  fn(physicalDevice, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                   uint32_t *pQueueFamilyPropertyCount,
                                                   VkQueueFamilyProperties *pQueueFamilyProperties)
{
  static PFN_vkGetPhysicalDeviceQueueFamilyProperties fn = vkGetPhysicalDeviceQueueFamilyProperties;
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  return;
}

void shim_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                              VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
  static PFN_vkGetPhysicalDeviceMemoryProperties fn = vkGetPhysicalDeviceMemoryProperties;
  fn(physicalDevice, pMemoryProperties);
  return;
}

void shim_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                      VkPhysicalDeviceFeatures *pFeatures)
{
  static PFN_vkGetPhysicalDeviceFeatures fn = vkGetPhysicalDeviceFeatures;
  fn(physicalDevice, pFeatures);
  return;
}

void shim_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                              VkFormatProperties *pFormatProperties)
{
  static PFN_vkGetPhysicalDeviceFormatProperties fn = vkGetPhysicalDeviceFormatProperties;
  fn(physicalDevice, format, pFormatProperties);
  return;
}

VkResult shim_vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice,
                                                       VkFormat format, VkImageType type,
                                                       VkImageTiling tiling, VkImageUsageFlags usage,
                                                       VkImageCreateFlags flags,
                                                       VkImageFormatProperties *pImageFormatProperties)
{
  static PFN_vkGetPhysicalDeviceImageFormatProperties fn = vkGetPhysicalDeviceImageFormatProperties;
  VkResult r = fn(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
  return r;
}

void shim_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDevice fn = vkDestroyDevice;
  fn(device, pAllocator);
  return;
}

VkResult shim_vkEnumerateInstanceVersion(uint32_t *pApiVersion)
{
  static PFN_vkEnumerateInstanceVersion fn = vkEnumerateInstanceVersion;
  VkResult r = fn(pApiVersion);
  return r;
}

VkResult shim_vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                                 VkLayerProperties *pProperties)
{
  static PFN_vkEnumerateInstanceLayerProperties fn = vkEnumerateInstanceLayerProperties;
  VkResult r = fn(pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount,
                                                     VkExtensionProperties *pProperties)
{
  static PFN_vkEnumerateInstanceExtensionProperties fn = vkEnumerateInstanceExtensionProperties;
  VkResult r = fn(pLayerName, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                                               uint32_t *pPropertyCount,
                                               VkLayerProperties *pProperties)
{
  static PFN_vkEnumerateDeviceLayerProperties fn = vkEnumerateDeviceLayerProperties;
  VkResult r = fn(physicalDevice, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                   const char *pLayerName, uint32_t *pPropertyCount,
                                                   VkExtensionProperties *pProperties)
{
  static PFN_vkEnumerateDeviceExtensionProperties fn = vkEnumerateDeviceExtensionProperties;
  VkResult r = fn(physicalDevice, pLayerName, pPropertyCount, pProperties);
  return r;
}

void shim_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                           VkQueue *pQueue)
{
  static PFN_vkGetDeviceQueue fn = vkGetDeviceQueue;
  fn(device, queueFamilyIndex, queueIndex, pQueue);
  return;
}

VkResult shim_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                            VkFence fence)
{
  static PFN_vkQueueSubmit fn = vkQueueSubmit;
  VkResult r = fn(queue, submitCount, pSubmits, fence);
  return r;
}

VkResult shim_vkQueueWaitIdle(VkQueue queue)
{
  static PFN_vkQueueWaitIdle fn = vkQueueWaitIdle;
  VkResult r = fn(queue);
  return r;
}

VkResult shim_vkDeviceWaitIdle(VkDevice device)
{
  static PFN_vkDeviceWaitIdle fn = vkDeviceWaitIdle;
  VkResult r = fn(device);
  return r;
}

VkResult shim_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory)
{
  static PFN_vkAllocateMemory fn = vkAllocateMemory;
  VkResult r = fn(device, pAllocateInfo, pAllocator, pMemory);
  return r;
}

void shim_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkFreeMemory fn = vkFreeMemory;
  fn(device, memory, pAllocator);
  return;
}

VkResult shim_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset,
                          VkDeviceSize size, VkMemoryMapFlags flags, void **ppData)
{
  static PFN_vkMapMemory fn = vkMapMemory;
  VkResult r = fn(device, memory, offset, size, flags, ppData);
  return r;
}

void shim_vkUnmapMemory(VkDevice device, VkDeviceMemory memory)
{
  static PFN_vkUnmapMemory fn = vkUnmapMemory;
  fn(device, memory);
  return;
}

VkResult shim_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                        const VkMappedMemoryRange *pMemoryRanges)
{
  static PFN_vkFlushMappedMemoryRanges fn = vkFlushMappedMemoryRanges;
  VkResult r = fn(device, memoryRangeCount, pMemoryRanges);
  return r;
}

VkResult shim_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                             const VkMappedMemoryRange *pMemoryRanges)
{
  static PFN_vkInvalidateMappedMemoryRanges fn = vkInvalidateMappedMemoryRanges;
  VkResult r = fn(device, memoryRangeCount, pMemoryRanges);
  return r;
}

void shim_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                      VkDeviceSize *pCommittedMemoryInBytes)
{
  static PFN_vkGetDeviceMemoryCommitment fn = vkGetDeviceMemoryCommitment;
  fn(device, memory, pCommittedMemoryInBytes);
  return;
}

void shim_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                        VkMemoryRequirements *pMemoryRequirements)
{
  static PFN_vkGetBufferMemoryRequirements fn = vkGetBufferMemoryRequirements;
  fn(device, buffer, pMemoryRequirements);
  return;
}

VkResult shim_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                 VkDeviceSize memoryOffset)
{
  static PFN_vkBindBufferMemory fn = vkBindBufferMemory;
  VkResult r = fn(device, buffer, memory, memoryOffset);
  return r;
}

void shim_vkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                       VkMemoryRequirements *pMemoryRequirements)
{
  static PFN_vkGetImageMemoryRequirements fn = vkGetImageMemoryRequirements;
  fn(device, image, pMemoryRequirements);
  return;
}

VkResult shim_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                VkDeviceSize memoryOffset)
{
  static PFN_vkBindImageMemory fn = vkBindImageMemory;
  VkResult r = fn(device, image, memory, memoryOffset);
  return r;
}

void shim_vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                             uint32_t *pSparseMemoryRequirementCount,
                                             VkSparseImageMemoryRequirements *pSparseMemoryRequirements)
{
  static PFN_vkGetImageSparseMemoryRequirements fn = vkGetImageSparseMemoryRequirements;
  fn(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
  return;
}

void shim_vkGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
    VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties)
{
  static PFN_vkGetPhysicalDeviceSparseImageFormatProperties fn =
      vkGetPhysicalDeviceSparseImageFormatProperties;
  fn(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
  return;
}

VkResult shim_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount,
                                const VkBindSparseInfo *pBindInfo, VkFence fence)
{
  static PFN_vkQueueBindSparse fn = vkQueueBindSparse;
  VkResult r = fn(queue, bindInfoCount, pBindInfo, fence);
  return r;
}

VkResult shim_vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkFence *pFence)
{
  static PFN_vkCreateFence fn = vkCreateFence;
  VkResult r = fn(device, pCreateInfo, pAllocator, pFence);
  return r;
}

void shim_vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyFence fn = vkDestroyFence;
  fn(device, fence, pAllocator);
  return;
}

VkResult shim_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences)
{
  static PFN_vkResetFences fn = vkResetFences;
  VkResult r = fn(device, fenceCount, pFences);
  return r;
}

VkResult shim_vkGetFenceStatus(VkDevice device, VkFence fence)
{
  static PFN_vkGetFenceStatus fn = vkGetFenceStatus;
  VkResult r = fn(device, fence);
  return r;
}

VkResult shim_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                              VkBool32 waitAll, uint64_t timeout)
{
  static PFN_vkWaitForFences fn = vkWaitForFences;
  VkResult r = fn(device, fenceCount, pFences, waitAll, timeout);
  return r;
}

VkResult shim_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore)
{
  static PFN_vkCreateSemaphore fn = vkCreateSemaphore;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSemaphore);
  return r;
}

void shim_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySemaphore fn = vkDestroySemaphore;
  fn(device, semaphore, pAllocator);
  return;
}

VkResult shim_vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkEvent *pEvent)
{
  static PFN_vkCreateEvent fn = vkCreateEvent;
  VkResult r = fn(device, pCreateInfo, pAllocator, pEvent);
  return r;
}

void shim_vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyEvent fn = vkDestroyEvent;
  fn(device, event, pAllocator);
  return;
}

VkResult shim_vkGetEventStatus(VkDevice device, VkEvent event)
{
  static PFN_vkGetEventStatus fn = vkGetEventStatus;
  VkResult r = fn(device, event);
  return r;
}

VkResult shim_vkSetEvent(VkDevice device, VkEvent event)
{
  static PFN_vkSetEvent fn = vkSetEvent;
  VkResult r = fn(device, event);
  return r;
}

VkResult shim_vkResetEvent(VkDevice device, VkEvent event)
{
  static PFN_vkResetEvent fn = vkResetEvent;
  VkResult r = fn(device, event);
  return r;
}

VkResult shim_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool)
{
  static PFN_vkCreateQueryPool fn = vkCreateQueryPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pQueryPool);
  return r;
}

void shim_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyQueryPool fn = vkDestroyQueryPool;
  fn(device, queryPool, pAllocator);
  return;
}

VkResult shim_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                    uint32_t queryCount, size_t dataSize, void *pData,
                                    VkDeviceSize stride, VkQueryResultFlags flags)
{
  static PFN_vkGetQueryPoolResults fn = vkGetQueryPoolResults;
  VkResult r = fn(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
  return r;
}

VkResult shim_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer)
{
  static PFN_vkCreateBuffer fn = vkCreateBuffer;
  VkResult r = fn(device, pCreateInfo, pAllocator, pBuffer);
  return r;
}

void shim_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyBuffer fn = vkDestroyBuffer;
  fn(device, buffer, pAllocator);
  return;
}
)" + std::string(R"(
VkResult shim_vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkBufferView *pView)
{
  static PFN_vkCreateBufferView fn = vkCreateBufferView;
  VkResult r = fn(device, pCreateInfo, pAllocator, pView);
  return r;
}

void shim_vkDestroyBufferView(VkDevice device, VkBufferView bufferView,
                              const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyBufferView fn = vkDestroyBufferView;
  fn(device, bufferView, pAllocator);
  return;
}

VkResult shim_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkImage *pImage)
{
  static PFN_vkCreateImage fn = vkCreateImage;
  VkResult r = fn(device, pCreateInfo, pAllocator, pImage);
  return r;
}

void shim_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyImage fn = vkDestroyImage;
  fn(device, image, pAllocator);
  return;
}

void shim_vkGetImageSubresourceLayout(VkDevice device, VkImage image,
                                      const VkImageSubresource *pSubresource,
                                      VkSubresourceLayout *pLayout)
{
  static PFN_vkGetImageSubresourceLayout fn = vkGetImageSubresourceLayout;
  fn(device, image, pSubresource, pLayout);
  return;
}

VkResult shim_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkImageView *pView)
{
  static PFN_vkCreateImageView fn = vkCreateImageView;
  VkResult r = fn(device, pCreateInfo, pAllocator, pView);
  return r;
}

void shim_vkDestroyImageView(VkDevice device, VkImageView imageView,
                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyImageView fn = vkDestroyImageView;
  fn(device, imageView, pAllocator);
  return;
}

VkResult shim_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator,
                                   VkShaderModule *pShaderModule)
{
  static PFN_vkCreateShaderModule fn = vkCreateShaderModule;
  VkResult r = fn(device, pCreateInfo, pAllocator, pShaderModule);
  return r;
}

void shim_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyShaderModule fn = vkDestroyShaderModule;
  fn(device, shaderModule, pAllocator);
  return;
}

VkResult shim_vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator,
                                    VkPipelineCache *pPipelineCache)
{
  static PFN_vkCreatePipelineCache fn = vkCreatePipelineCache;
  VkResult r = fn(device, pCreateInfo, pAllocator, pPipelineCache);
  return r;
}

void shim_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                 const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyPipelineCache fn = vkDestroyPipelineCache;
  fn(device, pipelineCache, pAllocator);
  return;
}

VkResult shim_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache,
                                     size_t *pDataSize, void *pData)
{
  static PFN_vkGetPipelineCacheData fn = vkGetPipelineCacheData;
  VkResult r = fn(device, pipelineCache, pDataSize, pData);
  return r;
}

VkResult shim_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache,
                                    uint32_t srcCacheCount, const VkPipelineCache *pSrcCaches)
{
  static PFN_vkMergePipelineCaches fn = vkMergePipelineCaches;
  VkResult r = fn(device, dstCache, srcCacheCount, pSrcCaches);
  return r;
}

VkResult shim_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                        uint32_t createInfoCount,
                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                        const VkAllocationCallbacks *pAllocator,
                                        VkPipeline *pPipelines)
{
  static PFN_vkCreateGraphicsPipelines fn = vkCreateGraphicsPipelines;
  VkResult r = fn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
  return r;
}

VkResult shim_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                       uint32_t createInfoCount,
                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                       const VkAllocationCallbacks *pAllocator,
                                       VkPipeline *pPipelines)
{
  static PFN_vkCreateComputePipelines fn = vkCreateComputePipelines;
  VkResult r = fn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
  return r;
}

void shim_vkDestroyPipeline(VkDevice device, VkPipeline pipeline,
                            const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyPipeline fn = vkDestroyPipeline;
  fn(device, pipeline, pAllocator);
  return;
}

VkResult shim_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkPipelineLayout *pPipelineLayout)
{
  static PFN_vkCreatePipelineLayout fn = vkCreatePipelineLayout;
  VkResult r = fn(device, pCreateInfo, pAllocator, pPipelineLayout);
  return r;
}

void shim_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                  const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyPipelineLayout fn = vkDestroyPipelineLayout;
  fn(device, pipelineLayout, pAllocator);
  return;
}

VkResult shim_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler)
{
  static PFN_vkCreateSampler fn = vkCreateSampler;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSampler);
  return r;
}

void shim_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySampler fn = vkDestroySampler;
  fn(device, sampler, pAllocator);
  return;
}

VkResult shim_vkCreateDescriptorSetLayout(VkDevice device,
                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDescriptorSetLayout *pSetLayout)
{
  static PFN_vkCreateDescriptorSetLayout fn = vkCreateDescriptorSetLayout;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSetLayout);
  return r;
}

void shim_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                       const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDescriptorSetLayout fn = vkDestroyDescriptorSetLayout;
  fn(device, descriptorSetLayout, pAllocator);
  return;
}

VkResult shim_vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkDescriptorPool *pDescriptorPool)
{
  static PFN_vkCreateDescriptorPool fn = vkCreateDescriptorPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorPool);
  return r;
}

void shim_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                  const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDescriptorPool fn = vkDestroyDescriptorPool;
  fn(device, descriptorPool, pAllocator);
  return;
}

VkResult shim_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                    VkDescriptorPoolResetFlags flags)
{
  static PFN_vkResetDescriptorPool fn = vkResetDescriptorPool;
  VkResult r = fn(device, descriptorPool, flags);
  return r;
}

VkResult shim_vkAllocateDescriptorSets(VkDevice device,
                                       const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                       VkDescriptorSet *pDescriptorSets)
{
  static PFN_vkAllocateDescriptorSets fn = vkAllocateDescriptorSets;
  VkResult r = fn(device, pAllocateInfo, pDescriptorSets);
  return r;
}

VkResult shim_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                   uint32_t descriptorSetCount,
                                   const VkDescriptorSet *pDescriptorSets)
{
  static PFN_vkFreeDescriptorSets fn = vkFreeDescriptorSets;
  VkResult r = fn(device, descriptorPool, descriptorSetCount, pDescriptorSets);
  return r;
}

void shim_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                 const VkWriteDescriptorSet *pDescriptorWrites,
                                 uint32_t descriptorCopyCount,
                                 const VkCopyDescriptorSet *pDescriptorCopies)
{
  static PFN_vkUpdateDescriptorSets fn = vkUpdateDescriptorSets;
  fn(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
  return;
}

VkResult shim_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator,
                                  VkFramebuffer *pFramebuffer)
{
  static PFN_vkCreateFramebuffer fn = vkCreateFramebuffer;
  VkResult r = fn(device, pCreateInfo, pAllocator, pFramebuffer);
  return r;
}

void shim_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                               const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyFramebuffer fn = vkDestroyFramebuffer;
  fn(device, framebuffer, pAllocator);
  return;
}

VkResult shim_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass)
{
  static PFN_vkCreateRenderPass fn = vkCreateRenderPass;
  VkResult r = fn(device, pCreateInfo, pAllocator, pRenderPass);
  return r;
}

void shim_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                              const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyRenderPass fn = vkDestroyRenderPass;
  fn(device, renderPass, pAllocator);
  return;
}

void shim_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass,
                                     VkExtent2D *pGranularity)
{
  static PFN_vkGetRenderAreaGranularity fn = vkGetRenderAreaGranularity;
  fn(device, renderPass, pGranularity);
  return;
}

VkResult shim_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator,
                                  VkCommandPool *pCommandPool)
{
  static PFN_vkCreateCommandPool fn = vkCreateCommandPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pCommandPool);
  return r;
}

void shim_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                               const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyCommandPool fn = vkDestroyCommandPool;
  fn(device, commandPool, pAllocator);
  return;
}

VkResult shim_vkResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                 VkCommandPoolResetFlags flags)
{
  static PFN_vkResetCommandPool fn = vkResetCommandPool;
  VkResult r = fn(device, commandPool, flags);
  return r;
}

VkResult shim_vkAllocateCommandBuffers(VkDevice device,
                                       const VkCommandBufferAllocateInfo *pAllocateInfo,
                                       VkCommandBuffer *pCommandBuffers)
{
  static PFN_vkAllocateCommandBuffers fn = vkAllocateCommandBuffers;
  VkResult r = fn(device, pAllocateInfo, pCommandBuffers);
  return r;
}

void shim_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                               uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers)
{
  static PFN_vkFreeCommandBuffers fn = vkFreeCommandBuffers;
  fn(device, commandPool, commandBufferCount, pCommandBuffers);
  return;
}

VkResult shim_vkBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                   const VkCommandBufferBeginInfo *pBeginInfo)
{
  static PFN_vkBeginCommandBuffer fn = vkBeginCommandBuffer;
  VkResult r = fn(commandBuffer, pBeginInfo);
  return r;
}

VkResult shim_vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
  static PFN_vkEndCommandBuffer fn = vkEndCommandBuffer;
  VkResult r = fn(commandBuffer);
  return r;
}

VkResult shim_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
  static PFN_vkResetCommandBuffer fn = vkResetCommandBuffer;
  VkResult r = fn(commandBuffer, flags);
  return r;
}

void shim_vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                            VkPipeline pipeline)
{
  static PFN_vkCmdBindPipeline fn = vkCmdBindPipeline;
  fn(commandBuffer, pipelineBindPoint, pipeline);
  return;
}

void shim_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                           uint32_t viewportCount, const VkViewport *pViewports)
{
  static PFN_vkCmdSetViewport fn = vkCmdSetViewport;
  fn(commandBuffer, firstViewport, viewportCount, pViewports);
  return;
}

void shim_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                          uint32_t scissorCount, const VkRect2D *pScissors)
{
  static PFN_vkCmdSetScissor fn = vkCmdSetScissor;
  fn(commandBuffer, firstScissor, scissorCount, pScissors);
  return;
}

void shim_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
  static PFN_vkCmdSetLineWidth fn = vkCmdSetLineWidth;
  fn(commandBuffer, lineWidth);
  return;
}

void shim_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                            float depthBiasClamp, float depthBiasSlopeFactor)
{
  static PFN_vkCmdSetDepthBias fn = vkCmdSetDepthBias;
  fn(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
  return;
}

void shim_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
  static PFN_vkCmdSetBlendConstants fn = vkCmdSetBlendConstants;
  fn(commandBuffer, blendConstants);
  return;
}

void shim_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                              float maxDepthBounds)
{
  static PFN_vkCmdSetDepthBounds fn = vkCmdSetDepthBounds;
  fn(commandBuffer, minDepthBounds, maxDepthBounds);
  return;
}

void shim_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                     uint32_t compareMask)
{
  static PFN_vkCmdSetStencilCompareMask fn = vkCmdSetStencilCompareMask;
  fn(commandBuffer, faceMask, compareMask);
  return;
}

void shim_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                   uint32_t writeMask)
{
  static PFN_vkCmdSetStencilWriteMask fn = vkCmdSetStencilWriteMask;
  fn(commandBuffer, faceMask, writeMask);
  return;
}

void shim_vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                   uint32_t reference)
{
  static PFN_vkCmdSetStencilReference fn = vkCmdSetStencilReference;
  fn(commandBuffer, faceMask, reference);
  return;
}
)") + std::string(R"(
void shim_vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                  VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                  uint32_t firstSet, uint32_t descriptorSetCount,
                                  const VkDescriptorSet *pDescriptorSets,
                                  uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets)
{
  static PFN_vkCmdBindDescriptorSets fn = vkCmdBindDescriptorSets;
  fn(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
     dynamicOffsetCount, pDynamicOffsets);
  return;
}

void shim_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                               VkIndexType indexType)
{
  static PFN_vkCmdBindIndexBuffer fn = vkCmdBindIndexBuffer;
  fn(commandBuffer, buffer, offset, indexType);
  return;
}

void shim_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                 uint32_t bindingCount, const VkBuffer *pBuffers,
                                 const VkDeviceSize *pOffsets)
{
  static PFN_vkCmdBindVertexBuffers fn = vkCmdBindVertexBuffers;
  fn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
  return;
}

void shim_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                    uint32_t firstVertex, uint32_t firstInstance)
{
  static PFN_vkCmdDraw fn = vkCmdDraw;
  fn(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
  return;
}

void shim_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                           uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
  static PFN_vkCmdDrawIndexed fn = vkCmdDrawIndexed;
  fn(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
  return;
}

void shim_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                            uint32_t drawCount, uint32_t stride)
{
  static PFN_vkCmdDrawIndirect fn = vkCmdDrawIndirect;
  fn(commandBuffer, buffer, offset, drawCount, stride);
  return;
}

void shim_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                   VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
  static PFN_vkCmdDrawIndexedIndirect fn = vkCmdDrawIndexedIndirect;
  fn(commandBuffer, buffer, offset, drawCount, stride);
  return;
}

void shim_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                        uint32_t groupCountZ)
{
  static PFN_vkCmdDispatch fn = vkCmdDispatch;
  fn(commandBuffer, groupCountX, groupCountY, groupCountZ);
  return;
}

void shim_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
  static PFN_vkCmdDispatchIndirect fn = vkCmdDispatchIndirect;
  fn(commandBuffer, buffer, offset);
  return;
}

void shim_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                          uint32_t regionCount, const VkBufferCopy *pRegions)
{
  static PFN_vkCmdCopyBuffer fn = vkCmdCopyBuffer;
  fn(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
  return;
}

void shim_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                         VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                         uint32_t regionCount, const VkImageCopy *pRegions)
{
  static PFN_vkCmdCopyImage fn = vkCmdCopyImage;
  fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
  return;
}

void shim_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                         VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                         uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter)
{
  static PFN_vkCmdBlitImage fn = vkCmdBlitImage;
  fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
     filter);
  return;
}

void shim_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                 VkImage dstImage, VkImageLayout dstImageLayout,
                                 uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
  static PFN_vkCmdCopyBufferToImage fn = vkCmdCopyBufferToImage;
  fn(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
  return;
}

void shim_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                 VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                 uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
  static PFN_vkCmdCopyImageToBuffer fn = vkCmdCopyImageToBuffer;
  fn(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
  return;
}

void shim_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                            VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData)
{
  static PFN_vkCmdUpdateBuffer fn = vkCmdUpdateBuffer;
  fn(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
  return;
}

void shim_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                          VkDeviceSize size, uint32_t data)
{
  static PFN_vkCmdFillBuffer fn = vkCmdFillBuffer;
  fn(commandBuffer, dstBuffer, dstOffset, size, data);
  return;
}

void shim_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                               VkImageLayout imageLayout, const VkClearColorValue *pColor,
                               uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
  static PFN_vkCmdClearColorImage fn = vkCmdClearColorImage;
  fn(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
  return;
}

void shim_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                      VkImageLayout imageLayout,
                                      const VkClearDepthStencilValue *pDepthStencil,
                                      uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
  static PFN_vkCmdClearDepthStencilImage fn = vkCmdClearDepthStencilImage;
  fn(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
  return;
}

void shim_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                const VkClearAttachment *pAttachments, uint32_t rectCount,
                                const VkClearRect *pRects)
{
  static PFN_vkCmdClearAttachments fn = vkCmdClearAttachments;
  fn(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
  return;
}

void shim_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                            VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount,
                            const VkImageResolve *pRegions)
{
  static PFN_vkCmdResolveImage fn = vkCmdResolveImage;
  fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
  return;
}

void shim_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
  static PFN_vkCmdSetEvent fn = vkCmdSetEvent;
  fn(commandBuffer, event, stageMask);
  return;
}

void shim_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
  static PFN_vkCmdResetEvent fn = vkCmdResetEvent;
  fn(commandBuffer, event, stageMask);
  return;
}

void shim_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount,
                          const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                          VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                          const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                          const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                          uint32_t imageMemoryBarrierCount,
                          const VkImageMemoryBarrier *pImageMemoryBarriers)
{
  static PFN_vkCmdWaitEvents fn = vkCmdWaitEvents;
  fn(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
     pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
     pImageMemoryBarriers);
  return;
}

void shim_vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                               uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                               uint32_t bufferMemoryBarrierCount,
                               const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                               uint32_t imageMemoryBarrierCount,
                               const VkImageMemoryBarrier *pImageMemoryBarriers)
{
  static PFN_vkCmdPipelineBarrier fn = vkCmdPipelineBarrier;
  fn(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
     bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
  return;
}

void shim_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                          VkQueryControlFlags flags)
{
  static PFN_vkCmdBeginQuery fn = vkCmdBeginQuery;
  fn(commandBuffer, queryPool, query, flags);
  return;
}

void shim_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
  static PFN_vkCmdEndQuery fn = vkCmdEndQuery;
  fn(commandBuffer, queryPool, query);
  return;
}

void shim_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                              uint32_t firstQuery, uint32_t queryCount)
{
  static PFN_vkCmdResetQueryPool fn = vkCmdResetQueryPool;
  fn(commandBuffer, queryPool, firstQuery, queryCount);
  return;
}

void shim_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                              VkQueryPool queryPool, uint32_t query)
{
  static PFN_vkCmdWriteTimestamp fn = vkCmdWriteTimestamp;
  fn(commandBuffer, pipelineStage, queryPool, query);
  return;
}

void shim_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                    uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                    VkDeviceSize dstOffset, VkDeviceSize stride,
                                    VkQueryResultFlags flags)
{
  static PFN_vkCmdCopyQueryPoolResults fn = vkCmdCopyQueryPoolResults;
  fn(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
  return;
}

void shim_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                             VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                             const void *pValues)
{
  static PFN_vkCmdPushConstants fn = vkCmdPushConstants;
  fn(commandBuffer, layout, stageFlags, offset, size, pValues);
  return;
}

void shim_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                               const VkRenderPassBeginInfo *pRenderPassBegin,
                               VkSubpassContents contents)
{
  static PFN_vkCmdBeginRenderPass fn = vkCmdBeginRenderPass;
  fn(commandBuffer, pRenderPassBegin, contents);
  return;
}

void shim_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
  static PFN_vkCmdNextSubpass fn = vkCmdNextSubpass;
  fn(commandBuffer, contents);
  return;
}

void shim_vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
  static PFN_vkCmdEndRenderPass fn = vkCmdEndRenderPass;
  fn(commandBuffer);
  return;
}

void shim_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                               const VkCommandBuffer *pCommandBuffers)
{
  static PFN_vkCmdExecuteCommands fn = vkCmdExecuteCommands;
  fn(commandBuffer, commandBufferCount, pCommandBuffers);
  return;
}

VkResult shim_vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                      uint32_t *pPropertyCount,
                                                      VkDisplayPropertiesKHR *pProperties)
{
  static PFN_vkGetPhysicalDeviceDisplayPropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
  VkResult r = fn(physicalDevice, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                           uint32_t *pPropertyCount,
                                                           VkDisplayPlanePropertiesKHR *pProperties)
{
  static PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
  VkResult r = fn(physicalDevice, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice,
                                                    uint32_t planeIndex, uint32_t *pDisplayCount,
                                                    VkDisplayKHR *pDisplays)
{
  static PFN_vkGetDisplayPlaneSupportedDisplaysKHR fn =
      (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
  VkResult r = fn(physicalDevice, planeIndex, pDisplayCount, pDisplays);
  return r;
}

VkResult shim_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                            uint32_t *pPropertyCount,
                                            VkDisplayModePropertiesKHR *pProperties)
{
  static PFN_vkGetDisplayModePropertiesKHR fn = (PFN_vkGetDisplayModePropertiesKHR)vkGetInstanceProcAddr(
      aux.instance, "vkGetDisplayModePropertiesKHR");
  VkResult r = fn(physicalDevice, display, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                     const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode)
{
  static PFN_vkCreateDisplayModeKHR fn =
      (PFN_vkCreateDisplayModeKHR)vkGetInstanceProcAddr(aux.instance, "vkCreateDisplayModeKHR");
  VkResult r = fn(physicalDevice, display, pCreateInfo, pAllocator, pMode);
  return r;
}

VkResult shim_vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                               VkDisplayModeKHR mode, uint32_t planeIndex,
                                               VkDisplayPlaneCapabilitiesKHR *pCapabilities)
{
  static PFN_vkGetDisplayPlaneCapabilitiesKHR fn =
      (PFN_vkGetDisplayPlaneCapabilitiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetDisplayPlaneCapabilitiesKHR");
  VkResult r = fn(physicalDevice, mode, planeIndex, pCapabilities);
  return r;
}

VkResult shim_vkCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                             const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkSurfaceKHR *pSurface)
{
  static PFN_vkCreateDisplayPlaneSurfaceKHR fn = (PFN_vkCreateDisplayPlaneSurfaceKHR)vkGetInstanceProcAddr(
      instance, "vkCreateDisplayPlaneSurfaceKHR");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pSurface);
  return r;
}
)") + std::string(R"(
VkResult shim_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkSwapchainKHR *pSwapchains)
{
  static PFN_vkCreateSharedSwapchainsKHR fn =
      (PFN_vkCreateSharedSwapchainsKHR)vkGetDeviceProcAddr(device, "vkCreateSharedSwapchainsKHR");
  VkResult r = fn(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
  return r;
}

void shim_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                              const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySurfaceKHR fn =
      (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR");
  fn(instance, surface, pAllocator);
  return;
}

VkResult shim_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                   uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                   VkBool32 *pSupported)
{
  static PFN_vkGetPhysicalDeviceSurfaceSupportKHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
  VkResult r = fn(physicalDevice, queueFamilyIndex, surface, pSupported);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                        VkSurfaceKHR surface,
                                                        VkSurfaceCapabilitiesKHR *pSurfaceCapabilities)
{
  static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
  VkResult r = fn(physicalDevice, surface, pSurfaceCapabilities);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                                                   VkSurfaceKHR surface,
                                                   uint32_t *pSurfaceFormatCount,
                                                   VkSurfaceFormatKHR *pSurfaceFormats)
{
  static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
  VkResult r = fn(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                        VkSurfaceKHR surface,
                                                        uint32_t *pPresentModeCount,
                                                        VkPresentModeKHR *pPresentModes)
{
  static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fn =
      (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
  VkResult r = fn(physicalDevice, surface, pPresentModeCount, pPresentModes);
  return r;
}

VkResult shim_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator,
                                   VkSwapchainKHR *pSwapchain)
{
  static PFN_vkCreateSwapchainKHR fn =
      (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pSwapchain);
  return r;
}

void shim_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySwapchainKHR fn =
      (PFN_vkDestroySwapchainKHR)vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
  fn(device, swapchain, pAllocator);
  return;
}

VkResult shim_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                      uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages)
{
  static PFN_vkGetSwapchainImagesKHR fn =
      (PFN_vkGetSwapchainImagesKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
  VkResult r = fn(device, swapchain, pSwapchainImageCount, pSwapchainImages);
  return r;
}

VkResult shim_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                    VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
{
  static PFN_vkAcquireNextImageKHR fn =
      (PFN_vkAcquireNextImageKHR)vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
  VkResult r = fn(device, swapchain, timeout, semaphore, fence, pImageIndex);
  return r;
}

VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
  static PFN_vkQueuePresentKHR fn =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
  VkResult r = fn(queue, pPresentInfo);
  return r;
}

VkResult shim_vkCreateDebugReportCallbackEXT(VkInstance instance,
                                             const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkDebugReportCallbackEXT *pCallback)
{
  static PFN_vkCreateDebugReportCallbackEXT fn = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugReportCallbackEXT");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pCallback);
  return r;
}

void shim_vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                          const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDebugReportCallbackEXT fn = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugReportCallbackEXT");
  fn(instance, callback, pAllocator);
  return;
}

void shim_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                  VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                  size_t location, int32_t messageCode, const char *pLayerPrefix,
                                  const char *pMessage)
{
  static PFN_vkDebugReportMessageEXT fn =
      (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");
  fn(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
  return;
}

VkResult shim_vkDebugMarkerSetObjectNameEXT(VkDevice device,
                                            const VkDebugMarkerObjectNameInfoEXT *pNameInfo)
{
  static PFN_vkDebugMarkerSetObjectNameEXT fn = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(
      device, "vkDebugMarkerSetObjectNameEXT");
  VkResult r = fn(device, pNameInfo);
  return r;
}

VkResult shim_vkDebugMarkerSetObjectTagEXT(VkDevice device,
                                           const VkDebugMarkerObjectTagInfoEXT *pTagInfo)
{
  static PFN_vkDebugMarkerSetObjectTagEXT fn =
      (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
  VkResult r = fn(device, pTagInfo);
  return r;
}

void shim_vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                   const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
  static PFN_vkCmdDebugMarkerBeginEXT fn =
      (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(aux.device, "vkCmdDebugMarkerBeginEXT");
  fn(commandBuffer, pMarkerInfo);
  return;
}

void shim_vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer)
{
  static PFN_vkCmdDebugMarkerEndEXT fn =
      (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(aux.device, "vkCmdDebugMarkerEndEXT");
  fn(commandBuffer);
  return;
}

void shim_vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
                                    const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
  static PFN_vkCmdDebugMarkerInsertEXT fn =
      (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(aux.device, "vkCmdDebugMarkerInsertEXT");
  fn(commandBuffer, pMarkerInfo);
  return;
}

VkResult shim_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties)
{
  static PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV fn =
      (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
  VkResult r = fn(physicalDevice, format, type, tiling, usage, flags, externalHandleType,
                  pExternalImageFormatProperties);
  return r;
}

void shim_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                    VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                    uint32_t stride)
{
  static PFN_vkCmdDrawIndirectCountAMD fn =
      (PFN_vkCmdDrawIndirectCountAMD)vkGetDeviceProcAddr(aux.device, "vkCmdDrawIndirectCountAMD");
  fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
  return;
}

void shim_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                           VkDeviceSize offset, VkBuffer countBuffer,
                                           VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride)
{
  static PFN_vkCmdDrawIndexedIndirectCountAMD fn = (PFN_vkCmdDrawIndexedIndirectCountAMD)vkGetDeviceProcAddr(
      aux.device, "vkCmdDrawIndexedIndirectCountAMD");
  fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
  return;
}

void shim_vkCmdProcessCommandsNVX(VkCommandBuffer commandBuffer,
                                  const VkCmdProcessCommandsInfoNVX *pProcessCommandsInfo)
{
  static PFN_vkCmdProcessCommandsNVX fn =
      (PFN_vkCmdProcessCommandsNVX)vkGetDeviceProcAddr(aux.device, "vkCmdProcessCommandsNVX");
  fn(commandBuffer, pProcessCommandsInfo);
  return;
}

void shim_vkCmdReserveSpaceForCommandsNVX(VkCommandBuffer commandBuffer,
                                          const VkCmdReserveSpaceForCommandsInfoNVX *pReserveSpaceInfo)
{
  static PFN_vkCmdReserveSpaceForCommandsNVX fn = (PFN_vkCmdReserveSpaceForCommandsNVX)vkGetDeviceProcAddr(
      aux.device, "vkCmdReserveSpaceForCommandsNVX");
  fn(commandBuffer, pReserveSpaceInfo);
  return;
}

VkResult shim_vkCreateIndirectCommandsLayoutNVX(
    VkDevice device, const VkIndirectCommandsLayoutCreateInfoNVX *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNVX *pIndirectCommandsLayout)
{
  static PFN_vkCreateIndirectCommandsLayoutNVX fn =
      (PFN_vkCreateIndirectCommandsLayoutNVX)vkGetDeviceProcAddr(
          device, "vkCreateIndirectCommandsLayoutNVX");
  VkResult r = fn(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
  return r;
}

void shim_vkDestroyIndirectCommandsLayoutNVX(VkDevice device,
                                             VkIndirectCommandsLayoutNVX indirectCommandsLayout,
                                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyIndirectCommandsLayoutNVX fn =
      (PFN_vkDestroyIndirectCommandsLayoutNVX)vkGetDeviceProcAddr(
          device, "vkDestroyIndirectCommandsLayoutNVX");
  fn(device, indirectCommandsLayout, pAllocator);
  return;
}

VkResult shim_vkCreateObjectTableNVX(VkDevice device, const VkObjectTableCreateInfoNVX *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkObjectTableNVX *pObjectTable)
{
  static PFN_vkCreateObjectTableNVX fn =
      (PFN_vkCreateObjectTableNVX)vkGetDeviceProcAddr(device, "vkCreateObjectTableNVX");
  VkResult r = fn(device, pCreateInfo, pAllocator, pObjectTable);
  return r;
}

void shim_vkDestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable,
                                  const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyObjectTableNVX fn =
      (PFN_vkDestroyObjectTableNVX)vkGetDeviceProcAddr(device, "vkDestroyObjectTableNVX");
  fn(device, objectTable, pAllocator);
  return;
}

VkResult shim_vkRegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                   uint32_t objectCount,
                                   const VkObjectTableEntryNVX *const *ppObjectTableEntries,
                                   const uint32_t *pObjectIndices)
{
  static PFN_vkRegisterObjectsNVX fn =
      (PFN_vkRegisterObjectsNVX)vkGetDeviceProcAddr(device, "vkRegisterObjectsNVX");
  VkResult r = fn(device, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);
  return r;
}

VkResult shim_vkUnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                     uint32_t objectCount,
                                     const VkObjectEntryTypeNVX *pObjectEntryTypes,
                                     const uint32_t *pObjectIndices)
{
  static PFN_vkUnregisterObjectsNVX fn =
      (PFN_vkUnregisterObjectsNVX)vkGetDeviceProcAddr(device, "vkUnregisterObjectsNVX");
  VkResult r = fn(device, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);
  return r;
}

void shim_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice physicalDevice, VkDeviceGeneratedCommandsFeaturesNVX *pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX *pLimits)
{
  static PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX fn =
      (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX)vkGetDeviceProcAddr(
          aux.device, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
  fn(physicalDevice, pFeatures, pLimits);
  return;
}

void shim_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                       VkPhysicalDeviceFeatures2 *pFeatures)
{
  static PFN_vkGetPhysicalDeviceFeatures2 fn = (PFN_vkGetPhysicalDeviceFeatures2)vkGetInstanceProcAddr(
      aux.instance, "vkGetPhysicalDeviceFeatures2");
  fn(physicalDevice, pFeatures);
  return;
}

void shim_vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                          VkPhysicalDeviceFeatures2 *pFeatures)
{
  static PFN_vkGetPhysicalDeviceFeatures2KHR fn = (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(
      aux.instance, "vkGetPhysicalDeviceFeatures2KHR");
  fn(physicalDevice, pFeatures);
  return;
}

void shim_vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                         VkPhysicalDeviceProperties2 *pProperties)
{
  static PFN_vkGetPhysicalDeviceProperties2 fn = (PFN_vkGetPhysicalDeviceProperties2)vkGetInstanceProcAddr(
      aux.instance, "vkGetPhysicalDeviceProperties2");
  fn(physicalDevice, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                            VkPhysicalDeviceProperties2 *pProperties)
{
  static PFN_vkGetPhysicalDeviceProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceProperties2KHR");
  fn(physicalDevice, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                               VkFormatProperties2 *pFormatProperties)
{
  static PFN_vkGetPhysicalDeviceFormatProperties2 fn =
      (PFN_vkGetPhysicalDeviceFormatProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceFormatProperties2");
  fn(physicalDevice, format, pFormatProperties);
  return;
}
)") + std::string(R"(
void shim_vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                  VkFormatProperties2 *pFormatProperties)
{
  static PFN_vkGetPhysicalDeviceFormatProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceFormatProperties2KHR");
  fn(physicalDevice, format, pFormatProperties);
  return;
}

VkResult shim_vkGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties)
{
  static PFN_vkGetPhysicalDeviceImageFormatProperties2 fn =
      (PFN_vkGetPhysicalDeviceImageFormatProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceImageFormatProperties2");
  VkResult r = fn(physicalDevice, pImageFormatInfo, pImageFormatProperties);
  return r;
}

VkResult shim_vkGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties)
{
  static PFN_vkGetPhysicalDeviceImageFormatProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
  VkResult r = fn(physicalDevice, pImageFormatInfo, pImageFormatProperties);
  return r;
}

void shim_vkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                    uint32_t *pQueueFamilyPropertyCount,
                                                    VkQueueFamilyProperties2 *pQueueFamilyProperties)
{
  static PFN_vkGetPhysicalDeviceQueueFamilyProperties2 fn =
      (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  return;
}

void shim_vkGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                       uint32_t *pQueueFamilyPropertyCount,
                                                       VkQueueFamilyProperties2 *pQueueFamilyProperties)
{
  static PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  return;
}

void shim_vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                               VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
{
  static PFN_vkGetPhysicalDeviceMemoryProperties2 fn =
      (PFN_vkGetPhysicalDeviceMemoryProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceMemoryProperties2");
  fn(physicalDevice, pMemoryProperties);
  return;
}

void shim_vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
{
  static PFN_vkGetPhysicalDeviceMemoryProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
  fn(physicalDevice, pMemoryProperties);
  return;
}

void shim_vkGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties)
{
  static PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 fn =
      (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
  fn(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties)
{
  static PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
  fn(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
  return;
}

void shim_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                    VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                    uint32_t set, uint32_t descriptorWriteCount,
                                    const VkWriteDescriptorSet *pDescriptorWrites)
{
  static PFN_vkCmdPushDescriptorSetKHR fn =
      (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(aux.device, "vkCmdPushDescriptorSetKHR");
  fn(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
  return;
}

void shim_vkTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
{
  static PFN_vkTrimCommandPool fn =
      (PFN_vkTrimCommandPool)vkGetDeviceProcAddr(device, "vkTrimCommandPool");
  fn(device, commandPool, flags);
  return;
}

void shim_vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool,
                               VkCommandPoolTrimFlags flags)
{
  static PFN_vkTrimCommandPoolKHR fn =
      (PFN_vkTrimCommandPoolKHR)vkGetDeviceProcAddr(device, "vkTrimCommandPoolKHR");
  fn(device, commandPool, flags);
  return;
}

void shim_vkGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties)
{
  static PFN_vkGetPhysicalDeviceExternalBufferProperties fn =
      (PFN_vkGetPhysicalDeviceExternalBufferProperties)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalBufferProperties");
  fn(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
  return;
}

void shim_vkGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties)
{
  static PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
  fn(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
  return;
}

VkResult shim_vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd)
{
  static PFN_vkGetMemoryFdKHR fn = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdKHR");
  VkResult r = fn(device, pGetFdInfo, pFd);
  return r;
}

VkResult shim_vkGetMemoryFdPropertiesKHR(VkDevice device,
                                         VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                         VkMemoryFdPropertiesKHR *pMemoryFdProperties)
{
  static PFN_vkGetMemoryFdPropertiesKHR fn =
      (PFN_vkGetMemoryFdPropertiesKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdPropertiesKHR");
  VkResult r = fn(device, handleType, fd, pMemoryFdProperties);
  return r;
}

void shim_vkGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
{
  static PFN_vkGetPhysicalDeviceExternalSemaphoreProperties fn =
      (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
  fn(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
  return;
}

void shim_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
{
  static PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
  fn(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
  return;
}

VkResult shim_vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd)
{
  static PFN_vkGetSemaphoreFdKHR fn =
      (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreFdKHR");
  VkResult r = fn(device, pGetFdInfo, pFd);
  return r;
}

VkResult shim_vkImportSemaphoreFdKHR(VkDevice device,
                                     const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo)
{
  static PFN_vkImportSemaphoreFdKHR fn =
      (PFN_vkImportSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkImportSemaphoreFdKHR");
  VkResult r = fn(device, pImportSemaphoreFdInfo);
  return r;
}

void shim_vkGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties)
{
  static PFN_vkGetPhysicalDeviceExternalFenceProperties fn =
      (PFN_vkGetPhysicalDeviceExternalFenceProperties)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalFenceProperties");
  fn(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
  return;
}

void shim_vkGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties)
{
  static PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
  fn(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
  return;
}

VkResult shim_vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd)
{
  static PFN_vkGetFenceFdKHR fn = (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(device, "vkGetFenceFdKHR");
  VkResult r = fn(device, pGetFdInfo, pFd);
  return r;
}

VkResult shim_vkImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo)
{
  static PFN_vkImportFenceFdKHR fn =
      (PFN_vkImportFenceFdKHR)vkGetDeviceProcAddr(device, "vkImportFenceFdKHR");
  VkResult r = fn(device, pImportFenceFdInfo);
  return r;
}

VkResult shim_vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display)
{
  static PFN_vkReleaseDisplayEXT fn =
      (PFN_vkReleaseDisplayEXT)vkGetInstanceProcAddr(aux.instance, "vkReleaseDisplayEXT");
  VkResult r = fn(physicalDevice, display);
  return r;
}

VkResult shim_vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                       const VkDisplayPowerInfoEXT *pDisplayPowerInfo)
{
  static PFN_vkDisplayPowerControlEXT fn =
      (PFN_vkDisplayPowerControlEXT)vkGetDeviceProcAddr(device, "vkDisplayPowerControlEXT");
  VkResult r = fn(device, display, pDisplayPowerInfo);
  return r;
}

VkResult shim_vkRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT *pDeviceEventInfo,
                                       const VkAllocationCallbacks *pAllocator, VkFence *pFence)
{
  static PFN_vkRegisterDeviceEventEXT fn =
      (PFN_vkRegisterDeviceEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDeviceEventEXT");
  VkResult r = fn(device, pDeviceEventInfo, pAllocator, pFence);
  return r;
}

VkResult shim_vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                        const VkDisplayEventInfoEXT *pDisplayEventInfo,
                                        const VkAllocationCallbacks *pAllocator, VkFence *pFence)
{
  static PFN_vkRegisterDisplayEventEXT fn =
      (PFN_vkRegisterDisplayEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDisplayEventEXT");
  VkResult r = fn(device, display, pDisplayEventInfo, pAllocator, pFence);
  return r;
}

VkResult shim_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                       VkSurfaceCounterFlagBitsEXT counter, uint64_t *pCounterValue)
{
  static PFN_vkGetSwapchainCounterEXT fn =
      (PFN_vkGetSwapchainCounterEXT)vkGetDeviceProcAddr(device, "vkGetSwapchainCounterEXT");
  VkResult r = fn(device, swapchain, counter, pCounterValue);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                         VkSurfaceKHR surface,
                                                         VkSurfaceCapabilities2EXT *pSurfaceCapabilities)
{
  static PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT fn =
      (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
  VkResult r = fn(physicalDevice, surface, pSurfaceCapabilities);
  return r;
}

VkResult shim_vkEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties)
{
  static PFN_vkEnumeratePhysicalDeviceGroups fn = (PFN_vkEnumeratePhysicalDeviceGroups)vkGetInstanceProcAddr(
      instance, "vkEnumeratePhysicalDeviceGroups");
  VkResult r = fn(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
  return r;
}

VkResult shim_vkEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties)
{
  static PFN_vkEnumeratePhysicalDeviceGroupsKHR fn =
      (PFN_vkEnumeratePhysicalDeviceGroupsKHR)vkGetInstanceProcAddr(
          instance, "vkEnumeratePhysicalDeviceGroupsKHR");
  VkResult r = fn(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
  return r;
}

void shim_vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex,
                                             uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
                                             VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
  static PFN_vkGetDeviceGroupPeerMemoryFeatures fn =
      (PFN_vkGetDeviceGroupPeerMemoryFeatures)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupPeerMemoryFeatures");
  fn(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
  return;
}

void shim_vkGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex,
                                                uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
                                                VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
  static PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR fn =
      (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
  fn(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
  return;
}

VkResult shim_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                  const VkBindBufferMemoryInfo *pBindInfos)
{
  static PFN_vkBindBufferMemory2 fn =
      (PFN_vkBindBufferMemory2)vkGetDeviceProcAddr(device, "vkBindBufferMemory2");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

VkResult shim_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                     const VkBindBufferMemoryInfo *pBindInfos)
{
  static PFN_vkBindBufferMemory2KHR fn =
      (PFN_vkBindBufferMemory2KHR)vkGetDeviceProcAddr(device, "vkBindBufferMemory2KHR");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}
)") + std::string(R"(
VkResult shim_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                 const VkBindImageMemoryInfo *pBindInfos)
{
  static PFN_vkBindImageMemory2 fn =
      (PFN_vkBindImageMemory2)vkGetDeviceProcAddr(device, "vkBindImageMemory2");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

VkResult shim_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                    const VkBindImageMemoryInfo *pBindInfos)
{
  static PFN_vkBindImageMemory2KHR fn =
      (PFN_vkBindImageMemory2KHR)vkGetDeviceProcAddr(device, "vkBindImageMemory2KHR");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

void shim_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
  static PFN_vkCmdSetDeviceMask fn =
      (PFN_vkCmdSetDeviceMask)vkGetDeviceProcAddr(aux.device, "vkCmdSetDeviceMask");
  fn(commandBuffer, deviceMask);
  return;
}

void shim_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
  static PFN_vkCmdSetDeviceMaskKHR fn =
      (PFN_vkCmdSetDeviceMaskKHR)vkGetDeviceProcAddr(aux.device, "vkCmdSetDeviceMaskKHR");
  fn(commandBuffer, deviceMask);
  return;
}

VkResult shim_vkGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities)
{
  static PFN_vkGetDeviceGroupPresentCapabilitiesKHR fn =
      (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupPresentCapabilitiesKHR");
  VkResult r = fn(device, pDeviceGroupPresentCapabilities);
  return r;
}

VkResult shim_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                     VkDeviceGroupPresentModeFlagsKHR *pModes)
{
  static PFN_vkGetDeviceGroupSurfacePresentModesKHR fn =
      (PFN_vkGetDeviceGroupSurfacePresentModesKHR)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupSurfacePresentModesKHR");
  VkResult r = fn(device, surface, pModes);
  return r;
}

VkResult shim_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                     uint32_t *pImageIndex)
{
  static PFN_vkAcquireNextImage2KHR fn =
      (PFN_vkAcquireNextImage2KHR)vkGetDeviceProcAddr(device, "vkAcquireNextImage2KHR");
  VkResult r = fn(device, pAcquireInfo, pImageIndex);
  return r;
}

void shim_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                            uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                            uint32_t groupCountZ)
{
  static PFN_vkCmdDispatchBase fn =
      (PFN_vkCmdDispatchBase)vkGetDeviceProcAddr(aux.device, "vkCmdDispatchBase");
  fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
  return;
}

void shim_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                               uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX,
                               uint32_t groupCountY, uint32_t groupCountZ)
{
  static PFN_vkCmdDispatchBaseKHR fn =
      (PFN_vkCmdDispatchBaseKHR)vkGetDeviceProcAddr(aux.device, "vkCmdDispatchBaseKHR");
  fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
  return;
}

VkResult shim_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice,
                                                      VkSurfaceKHR surface, uint32_t *pRectCount,
                                                      VkRect2D *pRects)
{
  static PFN_vkGetPhysicalDevicePresentRectanglesKHR fn =
      (PFN_vkGetPhysicalDevicePresentRectanglesKHR)vkGetDeviceProcAddr(
          aux.device, "vkGetPhysicalDevicePresentRectanglesKHR");
  VkResult r = fn(physicalDevice, surface, pRectCount, pRects);
  return r;
}

VkResult shim_vkCreateDescriptorUpdateTemplate(VkDevice device,
                                               const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate)
{
  static PFN_vkCreateDescriptorUpdateTemplate fn = (PFN_vkCreateDescriptorUpdateTemplate)vkGetDeviceProcAddr(
      device, "vkCreateDescriptorUpdateTemplate");
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
  return r;
}

VkResult shim_vkCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate)
{
  static PFN_vkCreateDescriptorUpdateTemplateKHR fn =
      (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(
          device, "vkCreateDescriptorUpdateTemplateKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
  return r;
}

void shim_vkDestroyDescriptorUpdateTemplate(VkDevice device,
                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                            const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDescriptorUpdateTemplate fn =
      (PFN_vkDestroyDescriptorUpdateTemplate)vkGetDeviceProcAddr(
          device, "vkDestroyDescriptorUpdateTemplate");
  fn(device, descriptorUpdateTemplate, pAllocator);
  return;
}

void shim_vkDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                               const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDescriptorUpdateTemplateKHR fn =
      (PFN_vkDestroyDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(
          device, "vkDestroyDescriptorUpdateTemplateKHR");
  fn(device, descriptorUpdateTemplate, pAllocator);
  return;
}

void shim_vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                            const void *pData)
{
  static PFN_vkUpdateDescriptorSetWithTemplate fn =
      (PFN_vkUpdateDescriptorSetWithTemplate)vkGetDeviceProcAddr(
          device, "vkUpdateDescriptorSetWithTemplate");
  fn(device, descriptorSet, descriptorUpdateTemplate, pData);
  return;
}

void shim_vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                               const void *pData)
{
  static PFN_vkUpdateDescriptorSetWithTemplateKHR fn =
      (PFN_vkUpdateDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(
          device, "vkUpdateDescriptorSetWithTemplateKHR");
  fn(device, descriptorSet, descriptorUpdateTemplate, pData);
  return;
}

void shim_vkCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                VkPipelineLayout layout, uint32_t set,
                                                const void *pData)
{
  static PFN_vkCmdPushDescriptorSetWithTemplateKHR fn =
      (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(
          aux.device, "vkCmdPushDescriptorSetWithTemplateKHR");
  fn(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
  return;
}

void shim_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount,
                              const VkSwapchainKHR *pSwapchains, const VkHdrMetadataEXT *pMetadata)
{
  static PFN_vkSetHdrMetadataEXT fn =
      (PFN_vkSetHdrMetadataEXT)vkGetDeviceProcAddr(device, "vkSetHdrMetadataEXT");
  fn(device, swapchainCount, pSwapchains, pMetadata);
  return;
}

VkResult shim_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain)
{
  static PFN_vkGetSwapchainStatusKHR fn =
      (PFN_vkGetSwapchainStatusKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainStatusKHR");
  VkResult r = fn(device, swapchain);
  return r;
}

VkResult shim_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                              VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties)
{
  static PFN_vkGetRefreshCycleDurationGOOGLE fn = (PFN_vkGetRefreshCycleDurationGOOGLE)vkGetDeviceProcAddr(
      device, "vkGetRefreshCycleDurationGOOGLE");
  VkResult r = fn(device, swapchain, pDisplayTimingProperties);
  return r;
}

VkResult shim_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                uint32_t *pPresentationTimingCount,
                                                VkPastPresentationTimingGOOGLE *pPresentationTimings)
{
  static PFN_vkGetPastPresentationTimingGOOGLE fn =
      (PFN_vkGetPastPresentationTimingGOOGLE)vkGetDeviceProcAddr(
          device, "vkGetPastPresentationTimingGOOGLE");
  VkResult r = fn(device, swapchain, pPresentationTimingCount, pPresentationTimings);
  return r;
}

void shim_vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                     uint32_t viewportCount,
                                     const VkViewportWScalingNV *pViewportWScalings)
{
  static PFN_vkCmdSetViewportWScalingNV fn =
      (PFN_vkCmdSetViewportWScalingNV)vkGetDeviceProcAddr(aux.device, "vkCmdSetViewportWScalingNV");
  fn(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
  return;
}

void shim_vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                      uint32_t discardRectangleCount,
                                      const VkRect2D *pDiscardRectangles)
{
  static PFN_vkCmdSetDiscardRectangleEXT fn = (PFN_vkCmdSetDiscardRectangleEXT)vkGetDeviceProcAddr(
      aux.device, "vkCmdSetDiscardRectangleEXT");
  fn(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
  return;
}

void shim_vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                     const VkSampleLocationsInfoEXT *pSampleLocationsInfo)
{
  static PFN_vkCmdSetSampleLocationsEXT fn =
      (PFN_vkCmdSetSampleLocationsEXT)vkGetDeviceProcAddr(aux.device, "vkCmdSetSampleLocationsEXT");
  fn(commandBuffer, pSampleLocationsInfo);
  return;
}

void shim_vkGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice,
                                                      VkSampleCountFlagBits samples,
                                                      VkMultisamplePropertiesEXT *pMultisampleProperties)
{
  static PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT fn =
      (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)vkGetDeviceProcAddr(
          aux.device, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
  fn(physicalDevice, samples, pMultisampleProperties);
  return;
}

VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities)
{
  static PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
  VkResult r = fn(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                    const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                    uint32_t *pSurfaceFormatCount,
                                                    VkSurfaceFormat2KHR *pSurfaceFormats)
{
  static PFN_vkGetPhysicalDeviceSurfaceFormats2KHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
  VkResult r = fn(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
  return r;
}

void shim_vkGetBufferMemoryRequirements2(VkDevice device,
                                         const VkBufferMemoryRequirementsInfo2 *pInfo,
                                         VkMemoryRequirements2 *pMemoryRequirements)
{
  static PFN_vkGetBufferMemoryRequirements2 fn = (PFN_vkGetBufferMemoryRequirements2)vkGetDeviceProcAddr(
      device, "vkGetBufferMemoryRequirements2");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetBufferMemoryRequirements2KHR(VkDevice device,
                                            const VkBufferMemoryRequirementsInfo2 *pInfo,
                                            VkMemoryRequirements2 *pMemoryRequirements)
{
  static PFN_vkGetBufferMemoryRequirements2KHR fn =
      (PFN_vkGetBufferMemoryRequirements2KHR)vkGetDeviceProcAddr(
          device, "vkGetBufferMemoryRequirements2KHR");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                        VkMemoryRequirements2 *pMemoryRequirements)
{
  static PFN_vkGetImageMemoryRequirements2 fn = (PFN_vkGetImageMemoryRequirements2)vkGetDeviceProcAddr(
      device, "vkGetImageMemoryRequirements2");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetImageMemoryRequirements2KHR(VkDevice device,
                                           const VkImageMemoryRequirementsInfo2 *pInfo,
                                           VkMemoryRequirements2 *pMemoryRequirements)
{
  static PFN_vkGetImageMemoryRequirements2KHR fn = (PFN_vkGetImageMemoryRequirements2KHR)vkGetDeviceProcAddr(
      device, "vkGetImageMemoryRequirements2KHR");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetImageSparseMemoryRequirements2(VkDevice device,
                                              const VkImageSparseMemoryRequirementsInfo2 *pInfo,
                                              uint32_t *pSparseMemoryRequirementCount,
                                              VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
  static PFN_vkGetImageSparseMemoryRequirements2 fn =
      (PFN_vkGetImageSparseMemoryRequirements2)vkGetDeviceProcAddr(
          device, "vkGetImageSparseMemoryRequirements2");
  fn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
  return;
}

void shim_vkGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
  static PFN_vkGetImageSparseMemoryRequirements2KHR fn =
      (PFN_vkGetImageSparseMemoryRequirements2KHR)vkGetDeviceProcAddr(
          device, "vkGetImageSparseMemoryRequirements2KHR");
  fn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
  return;
}

VkResult shim_vkCreateSamplerYcbcrConversion(VkDevice device,
                                             const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkSamplerYcbcrConversion *pYcbcrConversion)
{
  static PFN_vkCreateSamplerYcbcrConversion fn = (PFN_vkCreateSamplerYcbcrConversion)vkGetDeviceProcAddr(
      device, "vkCreateSamplerYcbcrConversion");
  VkResult r = fn(device, pCreateInfo, pAllocator, pYcbcrConversion);
  return r;
}
)") + std::string(R"(
VkResult shim_vkCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator,
                                                VkSamplerYcbcrConversion *pYcbcrConversion)
{
  static PFN_vkCreateSamplerYcbcrConversionKHR fn =
      (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(
          device, "vkCreateSamplerYcbcrConversionKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pYcbcrConversion);
  return r;
}

void shim_vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                          const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySamplerYcbcrConversion fn = (PFN_vkDestroySamplerYcbcrConversion)vkGetDeviceProcAddr(
      device, "vkDestroySamplerYcbcrConversion");
  fn(device, ycbcrConversion, pAllocator);
  return;
}

void shim_vkDestroySamplerYcbcrConversionKHR(VkDevice device,
                                             VkSamplerYcbcrConversion ycbcrConversion,
                                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySamplerYcbcrConversionKHR fn =
      (PFN_vkDestroySamplerYcbcrConversionKHR)vkGetDeviceProcAddr(
          device, "vkDestroySamplerYcbcrConversionKHR");
  fn(device, ycbcrConversion, pAllocator);
  return;
}

void shim_vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue)
{
  static PFN_vkGetDeviceQueue2 fn = vkGetDeviceQueue2;
  fn(device, pQueueInfo, pQueue);
  return;
}

VkResult shim_vkCreateValidationCacheEXT(VkDevice device,
                                         const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator,
                                         VkValidationCacheEXT *pValidationCache)
{
  static PFN_vkCreateValidationCacheEXT fn =
      (PFN_vkCreateValidationCacheEXT)vkGetDeviceProcAddr(device, "vkCreateValidationCacheEXT");
  VkResult r = fn(device, pCreateInfo, pAllocator, pValidationCache);
  return r;
}

void shim_vkDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                      const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyValidationCacheEXT fn =
      (PFN_vkDestroyValidationCacheEXT)vkGetDeviceProcAddr(device, "vkDestroyValidationCacheEXT");
  fn(device, validationCache, pAllocator);
  return;
}

VkResult shim_vkGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                          size_t *pDataSize, void *pData)
{
  static PFN_vkGetValidationCacheDataEXT fn =
      (PFN_vkGetValidationCacheDataEXT)vkGetDeviceProcAddr(device, "vkGetValidationCacheDataEXT");
  VkResult r = fn(device, validationCache, pDataSize, pData);
  return r;
}

VkResult shim_vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache,
                                         uint32_t srcCacheCount,
                                         const VkValidationCacheEXT *pSrcCaches)
{
  static PFN_vkMergeValidationCachesEXT fn =
      (PFN_vkMergeValidationCachesEXT)vkGetDeviceProcAddr(device, "vkMergeValidationCachesEXT");
  VkResult r = fn(device, dstCache, srcCacheCount, pSrcCaches);
  return r;
}

void shim_vkGetDescriptorSetLayoutSupport(VkDevice device,
                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                          VkDescriptorSetLayoutSupport *pSupport)
{
  static PFN_vkGetDescriptorSetLayoutSupport fn = (PFN_vkGetDescriptorSetLayoutSupport)vkGetDeviceProcAddr(
      device, "vkGetDescriptorSetLayoutSupport");
  fn(device, pCreateInfo, pSupport);
  return;
}

void shim_vkGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                             const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                             VkDescriptorSetLayoutSupport *pSupport)
{
  static PFN_vkGetDescriptorSetLayoutSupportKHR fn =
      (PFN_vkGetDescriptorSetLayoutSupportKHR)vkGetDeviceProcAddr(
          device, "vkGetDescriptorSetLayoutSupportKHR");
  fn(device, pCreateInfo, pSupport);
  return;
}

VkResult shim_vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline,
                                 VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType,
                                 size_t *pInfoSize, void *pInfo)
{
  static PFN_vkGetShaderInfoAMD fn =
      (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(device, "vkGetShaderInfoAMD");
  VkResult r = fn(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
  return r;
}

VkResult shim_vkSetDebugUtilsObjectNameEXT(VkDevice device,
                                           const VkDebugUtilsObjectNameInfoEXT *pNameInfo)
{
  static PFN_vkSetDebugUtilsObjectNameEXT fn = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
      aux.instance, "vkSetDebugUtilsObjectNameEXT");
  VkResult r = fn(device, pNameInfo);
  return r;
}

VkResult shim_vkSetDebugUtilsObjectTagEXT(VkDevice device,
                                          const VkDebugUtilsObjectTagInfoEXT *pTagInfo)
{
  static PFN_vkSetDebugUtilsObjectTagEXT fn = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(
      aux.instance, "vkSetDebugUtilsObjectTagEXT");
  VkResult r = fn(device, pTagInfo);
  return r;
}

void shim_vkQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo)
{
  static PFN_vkQueueBeginDebugUtilsLabelEXT fn = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkQueueBeginDebugUtilsLabelEXT");
  fn(queue, pLabelInfo);
  return;
}

void shim_vkQueueEndDebugUtilsLabelEXT(VkQueue queue)
{
  static PFN_vkQueueEndDebugUtilsLabelEXT fn = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkQueueEndDebugUtilsLabelEXT");
  fn(queue);
  return;
}

void shim_vkQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo)
{
  static PFN_vkQueueInsertDebugUtilsLabelEXT fn = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkQueueInsertDebugUtilsLabelEXT");
  fn(queue, pLabelInfo);
  return;
}

void shim_vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                       const VkDebugUtilsLabelEXT *pLabelInfo)
{
  static PFN_vkCmdBeginDebugUtilsLabelEXT fn = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkCmdBeginDebugUtilsLabelEXT");
  fn(commandBuffer, pLabelInfo);
  return;
}

void shim_vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer)
{
  static PFN_vkCmdEndDebugUtilsLabelEXT fn = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkCmdEndDebugUtilsLabelEXT");
  fn(commandBuffer);
  return;
}

void shim_vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                        const VkDebugUtilsLabelEXT *pLabelInfo)
{
  static PFN_vkCmdInsertDebugUtilsLabelEXT fn = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkCmdInsertDebugUtilsLabelEXT");
  fn(commandBuffer, pLabelInfo);
  return;
}

VkResult shim_vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkDebugUtilsMessengerEXT *pMessenger)
{
  static PFN_vkCreateDebugUtilsMessengerEXT fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pMessenger);
  return r;
}

void shim_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                          const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDebugUtilsMessengerEXT fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  fn(instance, messenger, pAllocator);
  return;
}

void shim_vkSubmitDebugUtilsMessageEXT(VkInstance instance,
                                       VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData)
{
  static PFN_vkSubmitDebugUtilsMessageEXT fn = (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(
      instance, "vkSubmitDebugUtilsMessageEXT");
  fn(instance, messageSeverity, messageTypes, pCallbackData);
  return;
}

VkResult shim_vkGetMemoryHostPointerPropertiesEXT(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void *pHostPointer,
    VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties)
{
  static PFN_vkGetMemoryHostPointerPropertiesEXT fn =
      (PFN_vkGetMemoryHostPointerPropertiesEXT)vkGetDeviceProcAddr(
          device, "vkGetMemoryHostPointerPropertiesEXT");
  VkResult r = fn(device, handleType, pHostPointer, pMemoryHostPointerProperties);
  return r;
}

void shim_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer,
                                    VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer,
                                    VkDeviceSize dstOffset, uint32_t marker)
{
  static PFN_vkCmdWriteBufferMarkerAMD fn =
      (PFN_vkCmdWriteBufferMarkerAMD)vkGetDeviceProcAddr(aux.device, "vkCmdWriteBufferMarkerAMD");
  fn(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
  return;
}
)")},

    /******************************************************************************/
    /* TEMPLATE_FILE_SHIM_CMAKE                                                   */
    /******************************************************************************/
    {"sample_cpp_shim", "CMakeLists.txt",
     R"(SET (THIS_PROJECT_NAME sample_cpp_shim)
PROJECT(${THIS_PROJECT_NAME})

ADD_LIBRARY(${THIS_PROJECT_NAME} SHARED "shim_vulkan.h" "shim_vulkan.cpp")

TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                           UNICODE _UNICODE)
IF (NOT WIN32)
  SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                        CXX_VISIBILITY_PRESET hidden)
ENDIF ()

TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                      vulkan
                      helper)

SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                      OUTPUT_NAME shim_vulkan
                      ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}")

ADD_CUSTOM_COMMAND(TARGET ${THIS_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${THIS_PROJECT_NAME}> ${WORKING_DIRECTORY_DEBUG}sample_cpp_trace)
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_SHADER_INFO_SHIM_H                                           */
    /******************************************************************************/
    {"amd_shader_info_shim", "shim_vulkan.h",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: shim_vulkan.h
//-----------------------------------------------------------------------------
#pragma once
#if defined(_WIN32)
#define SHIM_VK_API_IMPORT __declspec(dllimport)
#define SHIM_VK_API_EXPORT __declspec(dllexport)
#else
#define SHIM_VK_API_IMPORT __attribute__((visibility("default")))
#define SHIM_VK_API_EXPORT __attribute__((visibility("default")))
#endif
#if defined(SHIM_VK_COMPILE_STATIC_LIB)
#define SHIM_VK_API
#else
#if defined(SHIM_VK_EXPORT)
#define SHIM_VK_API SHIM_VK_API_EXPORT
#else
#define SHIM_VK_API SHIM_VK_API_IMPORT
#endif
#endif
#include "vulkan/vulkan.h"

SHIM_VK_API VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator,
                                           VkInstance *pInstance);

SHIM_VK_API VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice,
                                         const VkDeviceCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);

SHIM_VK_API void shim_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkEnumeratePhysicalDevices(VkInstance instance,
                                                     uint32_t *pPhysicalDeviceCount,
                                                     VkPhysicalDevice *pPhysicalDevices);

SHIM_VK_API void shim_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties *pQueueFamilyProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties *pMemoryProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceFeatures *pFeatures);

SHIM_VK_API void shim_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice,
                                                          VkFormat format,
                                                          VkFormatProperties *pFormatProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkImageFormatProperties *pImageFormatProperties);

SHIM_VK_API void shim_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkEnumerateInstanceVersion(uint32_t *pApiVersion);

SHIM_VK_API VkResult shim_vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                                             VkLayerProperties *pProperties);

SHIM_VK_API VkResult shim_vkEnumerateInstanceExtensionProperties(const char *pLayerName,
                                                                 uint32_t *pPropertyCount,
                                                                 VkExtensionProperties *pProperties);

SHIM_VK_API VkResult shim_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                                                           uint32_t *pPropertyCount,
                                                           VkLayerProperties *pProperties);

SHIM_VK_API VkResult shim_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                               const char *pLayerName,
                                                               uint32_t *pPropertyCount,
                                                               VkExtensionProperties *pProperties);

SHIM_VK_API void shim_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex,
                                       uint32_t queueIndex, VkQueue *pQueue);

SHIM_VK_API VkResult shim_vkQueueSubmit(VkQueue queue, uint32_t submitCount,
                                        const VkSubmitInfo *pSubmits, VkFence fence);

SHIM_VK_API VkResult shim_vkQueueWaitIdle(VkQueue queue);

SHIM_VK_API VkResult shim_vkDeviceWaitIdle(VkDevice device);

SHIM_VK_API VkResult shim_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                           const VkAllocationCallbacks *pAllocator,
                                           VkDeviceMemory *pMemory);

SHIM_VK_API void shim_vkFreeMemory(VkDevice device, VkDeviceMemory memory,
                                   const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset,
                                      VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);

SHIM_VK_API void shim_vkUnmapMemory(VkDevice device, VkDeviceMemory memory);

SHIM_VK_API VkResult shim_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                    const VkMappedMemoryRange *pMemoryRanges);

SHIM_VK_API VkResult shim_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                         const VkMappedMemoryRange *pMemoryRanges);

SHIM_VK_API void shim_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                  VkDeviceSize *pCommittedMemoryInBytes);

SHIM_VK_API void shim_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                    VkMemoryRequirements *pMemoryRequirements);

SHIM_VK_API VkResult shim_vkBindBufferMemory(VkDevice device, VkBuffer buffer,
                                             VkDeviceMemory memory, VkDeviceSize memoryOffset);

SHIM_VK_API void shim_vkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                   VkMemoryRequirements *pMemoryRequirements);

SHIM_VK_API VkResult shim_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                            VkDeviceSize memoryOffset);

SHIM_VK_API void shim_vkGetImageSparseMemoryRequirements(
    VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements *pSparseMemoryRequirements);

SHIM_VK_API void shim_vkGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
    VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties);

SHIM_VK_API VkResult shim_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount,
                                            const VkBindSparseInfo *pBindInfo, VkFence fence);

SHIM_VK_API VkResult shim_vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkFence *pFence);

SHIM_VK_API void shim_vkDestroyFence(VkDevice device, VkFence fence,
                                     const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences);

SHIM_VK_API VkResult shim_vkGetFenceStatus(VkDevice device, VkFence fence);

SHIM_VK_API VkResult shim_vkWaitForFences(VkDevice device, uint32_t fenceCount,
                                          const VkFence *pFences, VkBool32 waitAll, uint64_t timeout);

SHIM_VK_API VkResult shim_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkSemaphore *pSemaphore);

SHIM_VK_API void shim_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkEvent *pEvent);

SHIM_VK_API void shim_vkDestroyEvent(VkDevice device, VkEvent event,
                                     const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetEventStatus(VkDevice device, VkEvent event);

SHIM_VK_API VkResult shim_vkSetEvent(VkDevice device, VkEvent event);

SHIM_VK_API VkResult shim_vkResetEvent(VkDevice device, VkEvent event);

SHIM_VK_API VkResult shim_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkQueryPool *pQueryPool);

SHIM_VK_API void shim_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool,
                                                uint32_t firstQuery, uint32_t queryCount,
                                                size_t dataSize, void *pData, VkDeviceSize stride,
                                                VkQueryResultFlags flags);

SHIM_VK_API VkResult shim_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer);

SHIM_VK_API void shim_vkDestroyBuffer(VkDevice device, VkBuffer buffer,
                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateBufferView(VkDevice device,
                                             const VkBufferViewCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkBufferView *pView);

SHIM_VK_API void shim_vkDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                          const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkImage *pImage);

SHIM_VK_API void shim_vkDestroyImage(VkDevice device, VkImage image,
                                     const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkGetImageSubresourceLayout(VkDevice device, VkImage image,
                                                  const VkImageSubresource *pSubresource,
                                                  VkSubresourceLayout *pLayout);

SHIM_VK_API VkResult shim_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkImageView *pView);

SHIM_VK_API void shim_vkDestroyImageView(VkDevice device, VkImageView imageView,
                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateShaderModule(VkDevice device,
                                               const VkShaderModuleCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkShaderModule *pShaderModule);

SHIM_VK_API void shim_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                            const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreatePipelineCache(VkDevice device,
                                                const VkPipelineCacheCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator,
                                                VkPipelineCache *pPipelineCache);

SHIM_VK_API void shim_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                             const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache,
                                                 size_t *pDataSize, void *pData);

SHIM_VK_API VkResult shim_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache,
                                                uint32_t srcCacheCount,
                                                const VkPipelineCache *pSrcCaches);

SHIM_VK_API VkResult shim_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                    uint32_t createInfoCount,
                                                    const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkPipeline *pPipelines);

SHIM_VK_API VkResult shim_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                   uint32_t createInfoCount,
                                                   const VkComputePipelineCreateInfo *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator,
                                                   VkPipeline *pPipelines);

SHIM_VK_API void shim_vkDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                        const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreatePipelineLayout(VkDevice device,
                                                 const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkPipelineLayout *pPipelineLayout);

SHIM_VK_API void shim_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                              const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkSampler *pSampler);

SHIM_VK_API void shim_vkDestroySampler(VkDevice device, VkSampler sampler,
                                       const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateDescriptorSetLayout(
    VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout);

SHIM_VK_API void shim_vkDestroyDescriptorSetLayout(VkDevice device,
                                                   VkDescriptorSetLayout descriptorSetLayout,
                                                   const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateDescriptorPool(VkDevice device,
                                                 const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDescriptorPool *pDescriptorPool);

SHIM_VK_API void shim_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                              const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                VkDescriptorPoolResetFlags flags);

SHIM_VK_API VkResult shim_vkAllocateDescriptorSets(VkDevice device,
                                                   const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                   VkDescriptorSet *pDescriptorSets);
)" + std::string(R"(
SHIM_VK_API VkResult shim_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                               uint32_t descriptorSetCount,
                                               const VkDescriptorSet *pDescriptorSets);

SHIM_VK_API void shim_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet *pDescriptorWrites,
                                             uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet *pDescriptorCopies);

SHIM_VK_API VkResult shim_vkCreateFramebuffer(VkDevice device,
                                              const VkFramebufferCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator,
                                              VkFramebuffer *pFramebuffer);

SHIM_VK_API void shim_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                           const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateRenderPass(VkDevice device,
                                             const VkRenderPassCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkRenderPass *pRenderPass);

SHIM_VK_API void shim_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                          const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass,
                                                 VkExtent2D *pGranularity);

SHIM_VK_API VkResult shim_vkCreateCommandPool(VkDevice device,
                                              const VkCommandPoolCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator,
                                              VkCommandPool *pCommandPool);

SHIM_VK_API void shim_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                           const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                             VkCommandPoolResetFlags flags);

SHIM_VK_API VkResult shim_vkAllocateCommandBuffers(VkDevice device,
                                                   const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                   VkCommandBuffer *pCommandBuffers);

SHIM_VK_API void shim_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                           uint32_t commandBufferCount,
                                           const VkCommandBuffer *pCommandBuffers);

SHIM_VK_API VkResult shim_vkBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                               const VkCommandBufferBeginInfo *pBeginInfo);

SHIM_VK_API VkResult shim_vkEndCommandBuffer(VkCommandBuffer commandBuffer);

SHIM_VK_API VkResult shim_vkResetCommandBuffer(VkCommandBuffer commandBuffer,
                                               VkCommandBufferResetFlags flags);

SHIM_VK_API void shim_vkCmdBindPipeline(VkCommandBuffer commandBuffer,
                                        VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);

SHIM_VK_API void shim_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                       uint32_t viewportCount, const VkViewport *pViewports);

SHIM_VK_API void shim_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                      uint32_t scissorCount, const VkRect2D *pScissors);

SHIM_VK_API void shim_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);

SHIM_VK_API void shim_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                                        float depthBiasClamp, float depthBiasSlopeFactor);

SHIM_VK_API void shim_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer,
                                             const float blendConstants[4]);

SHIM_VK_API void shim_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                                          float maxDepthBounds);

SHIM_VK_API void shim_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer,
                                                 VkStencilFaceFlags faceMask, uint32_t compareMask);

SHIM_VK_API void shim_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer,
                                               VkStencilFaceFlags faceMask, uint32_t writeMask);

SHIM_VK_API void shim_vkCmdSetStencilReference(VkCommandBuffer commandBuffer,
                                               VkStencilFaceFlags faceMask, uint32_t reference);

SHIM_VK_API void shim_vkCmdBindDescriptorSets(
    VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
    uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets,
    uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets);

SHIM_VK_API void shim_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                           VkDeviceSize offset, VkIndexType indexType);

SHIM_VK_API void shim_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                             uint32_t bindingCount, const VkBuffer *pBuffers,
                                             const VkDeviceSize *pOffsets);

SHIM_VK_API void shim_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount,
                                uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

SHIM_VK_API void shim_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                       uint32_t instanceCount, uint32_t firstIndex,
                                       int32_t vertexOffset, uint32_t firstInstance);

SHIM_VK_API void shim_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                        VkDeviceSize offset, uint32_t drawCount, uint32_t stride);

SHIM_VK_API void shim_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                               VkDeviceSize offset, uint32_t drawCount,
                                               uint32_t stride);

SHIM_VK_API void shim_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                    uint32_t groupCountY, uint32_t groupCountZ);

SHIM_VK_API void shim_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                            VkDeviceSize offset);

SHIM_VK_API void shim_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                      VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy *pRegions);

SHIM_VK_API void shim_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                     VkImageLayout srcImageLayout, VkImage dstImage,
                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy *pRegions);

SHIM_VK_API void shim_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                     VkImageLayout srcImageLayout, VkImage dstImage,
                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit *pRegions, VkFilter filter);

SHIM_VK_API void shim_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                             VkImage dstImage, VkImageLayout dstImageLayout,
                                             uint32_t regionCount, const VkBufferImageCopy *pRegions);

SHIM_VK_API void shim_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                             VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                             uint32_t regionCount, const VkBufferImageCopy *pRegions);

SHIM_VK_API void shim_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                        VkDeviceSize dstOffset, VkDeviceSize dataSize,
                                        const void *pData);

SHIM_VK_API void shim_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                      VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);

SHIM_VK_API void shim_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                           VkImageLayout imageLayout,
                                           const VkClearColorValue *pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange *pRanges);

SHIM_VK_API void shim_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                  VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue *pDepthStencil,
                                                  uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges);

SHIM_VK_API void shim_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment *pAttachments,
                                            uint32_t rectCount, const VkClearRect *pRects);

SHIM_VK_API void shim_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                        VkImageLayout srcImageLayout, VkImage dstImage,
                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve *pRegions);

SHIM_VK_API void shim_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                    VkPipelineStageFlags stageMask);

SHIM_VK_API void shim_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                      VkPipelineStageFlags stageMask);

SHIM_VK_API void shim_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                      const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                                      VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                      const VkMemoryBarrier *pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount,
                                      const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount,
                                      const VkImageMemoryBarrier *pImageMemoryBarriers);

SHIM_VK_API void shim_vkCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);

SHIM_VK_API void shim_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                      uint32_t query, VkQueryControlFlags flags);

SHIM_VK_API void shim_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                    uint32_t query);

SHIM_VK_API void shim_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                          uint32_t firstQuery, uint32_t queryCount);

SHIM_VK_API void shim_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer,
                                          VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t query);

SHIM_VK_API void shim_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                uint32_t firstQuery, uint32_t queryCount,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags);

SHIM_VK_API void shim_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                         VkShaderStageFlags stageFlags, uint32_t offset,
                                         uint32_t size, const void *pValues);

SHIM_VK_API void shim_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                                           const VkRenderPassBeginInfo *pRenderPassBegin,
                                           VkSubpassContents contents);

SHIM_VK_API void shim_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);

SHIM_VK_API void shim_vkCmdEndRenderPass(VkCommandBuffer commandBuffer);

SHIM_VK_API void shim_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer *pCommandBuffers);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPropertiesKHR *pProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
    VkDisplayPlanePropertiesKHR *pProperties);

SHIM_VK_API VkResult shim_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice,
                                                                uint32_t planeIndex,
                                                                uint32_t *pDisplayCount,
                                                                VkDisplayKHR *pDisplays);

SHIM_VK_API VkResult shim_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                        VkDisplayKHR display,
                                                        uint32_t *pPropertyCount,
                                                        VkDisplayModePropertiesKHR *pProperties);

SHIM_VK_API VkResult shim_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice,
                                                 VkDisplayKHR display,
                                                 const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDisplayModeKHR *pMode);

SHIM_VK_API VkResult shim_vkGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
    VkDisplayPlaneCapabilitiesKHR *pCapabilities);

SHIM_VK_API VkResult shim_vkCreateDisplayPlaneSurfaceKHR(
    VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);

SHIM_VK_API VkResult shim_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                      const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator,
                                                      VkSwapchainKHR *pSwapchains);

SHIM_VK_API void shim_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                          const VkAllocationCallbacks *pAllocator);
)") + std::string(R"(
SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                               uint32_t queueFamilyIndex,
                                                               VkSurfaceKHR surface,
                                                               VkBool32 *pSupported);

SHIM_VK_API VkResult
shim_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                               VkSurfaceCapabilitiesKHR *pSurfaceCapabilities);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                                                               VkSurfaceKHR surface,
                                                               uint32_t *pSurfaceFormatCount,
                                                               VkSurfaceFormatKHR *pSurfaceFormats);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                    VkSurfaceKHR surface,
                                                                    uint32_t *pPresentModeCount,
                                                                    VkPresentModeKHR *pPresentModes);

SHIM_VK_API VkResult shim_vkCreateSwapchainKHR(VkDevice device,
                                               const VkSwapchainCreateInfoKHR *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkSwapchainKHR *pSwapchain);

SHIM_VK_API void shim_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                            const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                  uint32_t *pSwapchainImageCount,
                                                  VkImage *pSwapchainImages);

SHIM_VK_API VkResult shim_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                uint64_t timeout, VkSemaphore semaphore,
                                                VkFence fence, uint32_t *pImageIndex);

SHIM_VK_API VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo);

SHIM_VK_API VkResult shim_vkCreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback);

SHIM_VK_API void shim_vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                                      VkDebugReportCallbackEXT callback,
                                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                              VkDebugReportObjectTypeEXT objectType,
                                              uint64_t object, size_t location, int32_t messageCode,
                                              const char *pLayerPrefix, const char *pMessage);

SHIM_VK_API VkResult
shim_vkDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo);

SHIM_VK_API VkResult shim_vkDebugMarkerSetObjectTagEXT(VkDevice device,
                                                       const VkDebugMarkerObjectTagInfoEXT *pTagInfo);

SHIM_VK_API void shim_vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                               const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);

SHIM_VK_API void shim_vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);

SHIM_VK_API void shim_vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
                                                const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties);

SHIM_VK_API void shim_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                VkDeviceSize offset, VkBuffer countBuffer,
                                                VkDeviceSize countBufferOffset,
                                                uint32_t maxDrawCount, uint32_t stride);

SHIM_VK_API void shim_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                       VkDeviceSize offset, VkBuffer countBuffer,
                                                       VkDeviceSize countBufferOffset,
                                                       uint32_t maxDrawCount, uint32_t stride);

SHIM_VK_API void shim_vkCmdProcessCommandsNVX(VkCommandBuffer commandBuffer,
                                              const VkCmdProcessCommandsInfoNVX *pProcessCommandsInfo);

SHIM_VK_API void shim_vkCmdReserveSpaceForCommandsNVX(
    VkCommandBuffer commandBuffer, const VkCmdReserveSpaceForCommandsInfoNVX *pReserveSpaceInfo);

SHIM_VK_API VkResult shim_vkCreateIndirectCommandsLayoutNVX(
    VkDevice device, const VkIndirectCommandsLayoutCreateInfoNVX *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNVX *pIndirectCommandsLayout);

SHIM_VK_API void shim_vkDestroyIndirectCommandsLayoutNVX(
    VkDevice device, VkIndirectCommandsLayoutNVX indirectCommandsLayout,
    const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateObjectTableNVX(VkDevice device,
                                                 const VkObjectTableCreateInfoNVX *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkObjectTableNVX *pObjectTable);

SHIM_VK_API void shim_vkDestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable,
                                              const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkRegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                               uint32_t objectCount,
                                               const VkObjectTableEntryNVX *const *ppObjectTableEntries,
                                               const uint32_t *pObjectIndices);

SHIM_VK_API VkResult shim_vkUnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                                 uint32_t objectCount,
                                                 const VkObjectEntryTypeNVX *pObjectEntryTypes,
                                                 const uint32_t *pObjectIndices);

SHIM_VK_API void shim_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice physicalDevice, VkDeviceGeneratedCommandsFeaturesNVX *pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX *pLimits);

SHIM_VK_API void shim_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceFeatures2 *pFeatures);

SHIM_VK_API void shim_vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                      VkPhysicalDeviceFeatures2 *pFeatures);

SHIM_VK_API void shim_vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                     VkPhysicalDeviceProperties2 *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                        VkPhysicalDeviceProperties2 *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice,
                                                           VkFormat format,
                                                           VkFormatProperties2 *pFormatProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                              VkFormat format,
                                                              VkFormatProperties2 *pFormatProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2 *pQueueFamilyProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2 *pQueueFamilyProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);

SHIM_VK_API void shim_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                                VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set,
                                                uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet *pDescriptorWrites);

SHIM_VK_API void shim_vkTrimCommandPool(VkDevice device, VkCommandPool commandPool,
                                        VkCommandPoolTrimFlags flags);

SHIM_VK_API void shim_vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool,
                                           VkCommandPoolTrimFlags flags);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties);

SHIM_VK_API VkResult shim_vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo,
                                           int *pFd);

SHIM_VK_API VkResult shim_vkGetMemoryFdPropertiesKHR(VkDevice device,
                                                     VkExternalMemoryHandleTypeFlagBits handleType,
                                                     int fd,
                                                     VkMemoryFdPropertiesKHR *pMemoryFdProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties);

SHIM_VK_API VkResult shim_vkGetSemaphoreFdKHR(VkDevice device,
                                              const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd);

SHIM_VK_API VkResult shim_vkImportSemaphoreFdKHR(
    VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties);

SHIM_VK_API VkResult shim_vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo,
                                          int *pFd);

SHIM_VK_API VkResult shim_vkImportFenceFdKHR(VkDevice device,
                                             const VkImportFenceFdInfoKHR *pImportFenceFdInfo);

SHIM_VK_API VkResult shim_vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);

SHIM_VK_API VkResult shim_vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                   const VkDisplayPowerInfoEXT *pDisplayPowerInfo);

SHIM_VK_API VkResult shim_vkRegisterDeviceEventEXT(VkDevice device,
                                                   const VkDeviceEventInfoEXT *pDeviceEventInfo,
                                                   const VkAllocationCallbacks *pAllocator,
                                                   VkFence *pFence);

SHIM_VK_API VkResult shim_vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                    const VkDisplayEventInfoEXT *pDisplayEventInfo,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkFence *pFence);

SHIM_VK_API VkResult shim_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                   VkSurfaceCounterFlagBitsEXT counter,
                                                   uint64_t *pCounterValue);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
    VkSurfaceCapabilities2EXT *pSurfaceCapabilities);

SHIM_VK_API VkResult shim_vkEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);

SHIM_VK_API VkResult shim_vkEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);

SHIM_VK_API void shim_vkGetDeviceGroupPeerMemoryFeatures(
    VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
    VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);

SHIM_VK_API void shim_vkGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
    VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);

SHIM_VK_API VkResult shim_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                              const VkBindBufferMemoryInfo *pBindInfos);

SHIM_VK_API VkResult shim_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindBufferMemoryInfo *pBindInfos);

SHIM_VK_API VkResult shim_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                             const VkBindImageMemoryInfo *pBindInfos);
)") + std::string(R"(
SHIM_VK_API VkResult shim_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                const VkBindImageMemoryInfo *pBindInfos);

SHIM_VK_API void shim_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);

SHIM_VK_API void shim_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);

SHIM_VK_API VkResult shim_vkGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities);

SHIM_VK_API VkResult shim_vkGetDeviceGroupSurfacePresentModesKHR(
    VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR *pModes);

SHIM_VK_API VkResult shim_vkAcquireNextImage2KHR(VkDevice device,
                                                 const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                 uint32_t *pImageIndex);

SHIM_VK_API void shim_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                        uint32_t baseGroupY, uint32_t baseGroupZ,
                                        uint32_t groupCountX, uint32_t groupCountY,
                                        uint32_t groupCountZ);

SHIM_VK_API void shim_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                           uint32_t baseGroupY, uint32_t baseGroupZ,
                                           uint32_t groupCountX, uint32_t groupCountY,
                                           uint32_t groupCountZ);

SHIM_VK_API VkResult shim_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice,
                                                                  VkSurfaceKHR surface,
                                                                  uint32_t *pRectCount,
                                                                  VkRect2D *pRects);

SHIM_VK_API VkResult shim_vkCreateDescriptorUpdateTemplate(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);

SHIM_VK_API VkResult shim_vkCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);

SHIM_VK_API void shim_vkDestroyDescriptorUpdateTemplate(
    VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkDestroyDescriptorUpdateTemplateKHR(
    VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkUpdateDescriptorSetWithTemplate(
    VkDevice device, VkDescriptorSet descriptorSet,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);

SHIM_VK_API void shim_vkUpdateDescriptorSetWithTemplateKHR(
    VkDevice device, VkDescriptorSet descriptorSet,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);

SHIM_VK_API void shim_vkCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    VkPipelineLayout layout, uint32_t set, const void *pData);

SHIM_VK_API void shim_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount,
                                          const VkSwapchainKHR *pSwapchains,
                                          const VkHdrMetadataEXT *pMetadata);

SHIM_VK_API VkResult shim_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);

SHIM_VK_API VkResult
shim_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                     VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties);

SHIM_VK_API VkResult shim_vkGetPastPresentationTimingGOOGLE(
    VkDevice device, VkSwapchainKHR swapchain, uint32_t *pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE *pPresentationTimings);

SHIM_VK_API void shim_vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer,
                                                 uint32_t firstViewport, uint32_t viewportCount,
                                                 const VkViewportWScalingNV *pViewportWScalings);

SHIM_VK_API void shim_vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer,
                                                  uint32_t firstDiscardRectangle,
                                                  uint32_t discardRectangleCount,
                                                  const VkRect2D *pDiscardRectangles);

SHIM_VK_API void shim_vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                 const VkSampleLocationsInfoEXT *pSampleLocationsInfo);

SHIM_VK_API void shim_vkGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
    VkMultisamplePropertiesEXT *pMultisampleProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    uint32_t *pSurfaceFormatCount, VkSurfaceFormat2KHR *pSurfaceFormats);

SHIM_VK_API void shim_vkGetBufferMemoryRequirements2(VkDevice device,
                                                     const VkBufferMemoryRequirementsInfo2 *pInfo,
                                                     VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetBufferMemoryRequirements2KHR(VkDevice device,
                                                        const VkBufferMemoryRequirementsInfo2 *pInfo,
                                                        VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetImageMemoryRequirements2(VkDevice device,
                                                    const VkImageMemoryRequirementsInfo2 *pInfo,
                                                    VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetImageMemoryRequirements2KHR(VkDevice device,
                                                       const VkImageMemoryRequirementsInfo2 *pInfo,
                                                       VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetImageSparseMemoryRequirements2(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);

SHIM_VK_API void shim_vkGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);

SHIM_VK_API VkResult shim_vkCreateSamplerYcbcrConversion(
    VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);

SHIM_VK_API VkResult shim_vkCreateSamplerYcbcrConversionKHR(
    VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);

SHIM_VK_API void shim_vkDestroySamplerYcbcrConversion(VkDevice device,
                                                      VkSamplerYcbcrConversion ycbcrConversion,
                                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkDestroySamplerYcbcrConversionKHR(VkDevice device,
                                                         VkSamplerYcbcrConversion ycbcrConversion,
                                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo,
                                        VkQueue *pQueue);

SHIM_VK_API VkResult shim_vkCreateValidationCacheEXT(VkDevice device,
                                                     const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator,
                                                     VkValidationCacheEXT *pValidationCache);

SHIM_VK_API void shim_vkDestroyValidationCacheEXT(VkDevice device,
                                                  VkValidationCacheEXT validationCache,
                                                  const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetValidationCacheDataEXT(VkDevice device,
                                                      VkValidationCacheEXT validationCache,
                                                      size_t *pDataSize, void *pData);

SHIM_VK_API VkResult shim_vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache,
                                                     uint32_t srcCacheCount,
                                                     const VkValidationCacheEXT *pSrcCaches);

SHIM_VK_API void shim_vkGetDescriptorSetLayoutSupport(VkDevice device,
                                                      const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                      VkDescriptorSetLayoutSupport *pSupport);

SHIM_VK_API void shim_vkGetDescriptorSetLayoutSupportKHR(
    VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    VkDescriptorSetLayoutSupport *pSupport);

SHIM_VK_API VkResult shim_vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline,
                                             VkShaderStageFlagBits shaderStage,
                                             VkShaderInfoTypeAMD infoType, size_t *pInfoSize,
                                             void *pInfo);

SHIM_VK_API VkResult shim_vkSetDebugUtilsObjectNameEXT(VkDevice device,
                                                       const VkDebugUtilsObjectNameInfoEXT *pNameInfo);

SHIM_VK_API VkResult shim_vkSetDebugUtilsObjectTagEXT(VkDevice device,
                                                      const VkDebugUtilsObjectTagInfoEXT *pTagInfo);

SHIM_VK_API void shim_vkQueueBeginDebugUtilsLabelEXT(VkQueue queue,
                                                     const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API void shim_vkQueueEndDebugUtilsLabelEXT(VkQueue queue);

SHIM_VK_API void shim_vkQueueInsertDebugUtilsLabelEXT(VkQueue queue,
                                                      const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API void shim_vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                   const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API void shim_vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);

SHIM_VK_API void shim_vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                    const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API VkResult shim_vkCreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger);

SHIM_VK_API void shim_vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                      VkDebugUtilsMessengerEXT messenger,
                                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkSubmitDebugUtilsMessageEXT(
    VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);

SHIM_VK_API VkResult shim_vkGetMemoryHostPointerPropertiesEXT(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void *pHostPointer,
    VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties);

SHIM_VK_API void shim_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer,
                                                VkPipelineStageFlagBits pipelineStage,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                uint32_t marker);
)")},

    /******************************************************************************/
    /* TEMPLATE_FILE_SHADER_INFO_SHIM_CPP                                         */
    /******************************************************************************/
    {"amd_shader_info_shim", "shim_vulkan.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: amd_shader_info_shim/shim_vulkan.cpp
//-----------------------------------------------------------------------------
#ifndef SHIM_VK_COMPILE_STATIC_LIB
#define SHIM_VK_EXPORT
#endif
#include "helper/helper.h"
#include "shim_vulkan.h"
#include "utils.h"

AuxVkTraceResources aux;
int presentIndex = 0;
bool extAvailable = false;

/************************* shimmed functions *******************************/
VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
  // Enable VK_AMD_shader_info extension if supported.
  uint32_t extCount;
  VkResult r = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extCount, NULL);
  assert(r == VK_SUCCESS);
  std::vector<VkExtensionProperties> extensions(extCount);
  r = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extCount, extensions.data());
  assert(r == VK_SUCCESS);
  for(const auto &v : extensions)
  {
    if(strcmp(v.extensionName, VK_AMD_SHADER_INFO_EXTENSION_NAME) == 0)
    {
      extAvailable = true;
      break;
    }
  }
  if(extAvailable)
  {
    std::vector<const char *> enabledExtensionNames(pCreateInfo->enabledExtensionCount + 1);
    for(uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
    {
      enabledExtensionNames[i] = pCreateInfo->ppEnabledExtensionNames[i];
    }
    enabledExtensionNames[pCreateInfo->enabledExtensionCount] = VK_AMD_SHADER_INFO_EXTENSION_NAME;
    VkDeviceCreateInfo newCI = {
        /* sType = */ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        /* pNext */ pCreateInfo->pNext,
        /* flags */ pCreateInfo->flags,
        /* queueCreateInfoCount = */ pCreateInfo->queueCreateInfoCount,
        /* pQueueCreateInfos = */ pCreateInfo->pQueueCreateInfos,
        /* enabledLayerCount = */ pCreateInfo->enabledLayerCount,
        /* ppEnabledLayerNames = */ pCreateInfo->ppEnabledLayerNames,
        /* enabledExtensionCount = */ static_cast<uint32_t>(enabledExtensionNames.size()),
        /* ppEnabledExtensionNames = */ enabledExtensionNames.data(),
        /* pEnabledFeatures = */ pCreateInfo->pEnabledFeatures,
    };
    r = vkCreateDevice(physicalDevice, &newCI, pAllocator, pDevice);
  }
  else
  {
    r = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
  }
  InitializeAuxResources(&aux, aux.instance, physicalDevice, *pDevice);
  return r;
}

VkResult shim_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
  uint32_t createInfoCount,
  const VkComputePipelineCreateInfo *pCreateInfos,
  const VkAllocationCallbacks *pAllocator,
  VkPipeline *pPipelines) {
  static PFN_vkCreateComputePipelines fn = vkCreateComputePipelines;
  VkResult r = fn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

  // Get shader info.
  PFN_vkGetShaderInfoAMD pfnGetShaderInfoAMD =
    (PFN_vkGetShaderInfoAMD) vkGetDeviceProcAddr(device, "vkGetShaderInfoAMD");

  if (pfnGetShaderInfoAMD == NULL)
    return r;

  VkShaderStatisticsInfoAMD statistics = {};
  size_t dataSize;

  for (uint32_t i = 0; i < createInfoCount; i++) {
    VkShaderStageFlagBits shaderStage = pCreateInfos[i].stage.stage;
    VkPipeline p = pPipelines[i];

    dataSize = sizeof(statistics);
    // Print statistics data.
    r = pfnGetShaderInfoAMD(device, p, shaderStage, VK_SHADER_INFO_TYPE_STATISTICS_AMD, &dataSize,
      &statistics);
    assert(r == VK_SUCCESS);
    printShaderInfo(p, shaderStage, statistics);

    // Print disassembly data.
    r = pfnGetShaderInfoAMD(device, p, shaderStage, VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD, &dataSize,
      NULL);
    assert(r == VK_SUCCESS);
    std::vector<uint8_t> disassembly(dataSize);
    r = pfnGetShaderInfoAMD(device, p, shaderStage, VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD, &dataSize,
      disassembly.data());
    assert(r == VK_SUCCESS);
    printShaderInfo(p, shaderStage, (char *) disassembly.data(), disassembly.size());
  }

  return r;
}

VkResult shim_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                        uint32_t createInfoCount,
                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                        const VkAllocationCallbacks *pAllocator,
                                        VkPipeline *pPipelines)
{
  static PFN_vkCreateGraphicsPipelines fn = vkCreateGraphicsPipelines;
  VkResult r = fn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
  if(presentIndex > 0 || r != VK_SUCCESS || !extAvailable)
    return r;

  // Get shader info.
  PFN_vkGetShaderInfoAMD pfnGetShaderInfoAMD =
      (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(device, "vkGetShaderInfoAMD");

  if (pfnGetShaderInfoAMD == NULL)
    return r;

  VkShaderStatisticsInfoAMD statistics = {};
  size_t dataSize;

  for(uint32_t i = 0; i < createInfoCount; i++)
  {
    std::vector<VkShaderStageFlagBits> shaderStages;
    for (uint32_t s = 0; s < pCreateInfos[i].stageCount; s++)
      shaderStages.push_back(pCreateInfos[i].pStages[s].stage);
    VkPipeline p = pPipelines[i];
    for (uint32_t s = 0; s < shaderStages.size(); s++) {
      dataSize = sizeof(statistics);
      // Print statistics data.
      r = pfnGetShaderInfoAMD(device, p, shaderStages[s], VK_SHADER_INFO_TYPE_STATISTICS_AMD, &dataSize,
        &statistics);
      assert(r == VK_SUCCESS);
      printShaderInfo(p, shaderStages[s], statistics);

      // Print disassembly data.
      r = pfnGetShaderInfoAMD(device, p, shaderStages[s], VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD, &dataSize,
        NULL);
      assert(r == VK_SUCCESS);
      std::vector<uint8_t> disassembly(dataSize);
      r = pfnGetShaderInfoAMD(device, p, shaderStages[s], VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD, &dataSize,
        disassembly.data());
      assert(r == VK_SUCCESS);
      printShaderInfo(p, shaderStages[s], (char *) disassembly.data(), disassembly.size());
    }
  }

  return r;
}

VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
  static PFN_vkQueuePresentKHR fn =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
  VkResult r = fn(queue, pPresentInfo);
  presentIndex++;
  return r;
}
/************************* boilerplates *******************************/
VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance)
{
  VkResult r = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
  aux.instance = *pInstance;
  return r;
}

void shim_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyInstance fn = vkDestroyInstance;
  fn(instance, pAllocator);
  return;
}

VkResult shim_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                         VkPhysicalDevice *pPhysicalDevices)
{
  static PFN_vkEnumeratePhysicalDevices fn = vkEnumeratePhysicalDevices;
  VkResult r = fn(instance, pPhysicalDeviceCount, pPhysicalDevices);
  return r;
}

void shim_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                        VkPhysicalDeviceProperties *pProperties)
{
  static PFN_vkGetPhysicalDeviceProperties fn = vkGetPhysicalDeviceProperties;
  fn(physicalDevice, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                   uint32_t *pQueueFamilyPropertyCount,
                                                   VkQueueFamilyProperties *pQueueFamilyProperties)
{
  static PFN_vkGetPhysicalDeviceQueueFamilyProperties fn = vkGetPhysicalDeviceQueueFamilyProperties;
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  return;
}

void shim_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                              VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
  static PFN_vkGetPhysicalDeviceMemoryProperties fn = vkGetPhysicalDeviceMemoryProperties;
  fn(physicalDevice, pMemoryProperties);
  return;
}

void shim_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                      VkPhysicalDeviceFeatures *pFeatures)
{
  static PFN_vkGetPhysicalDeviceFeatures fn = vkGetPhysicalDeviceFeatures;
  fn(physicalDevice, pFeatures);
  return;
}

void shim_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                              VkFormatProperties *pFormatProperties)
{
  static PFN_vkGetPhysicalDeviceFormatProperties fn = vkGetPhysicalDeviceFormatProperties;
  fn(physicalDevice, format, pFormatProperties);
  return;
}

VkResult shim_vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice,
                                                       VkFormat format, VkImageType type,
                                                       VkImageTiling tiling, VkImageUsageFlags usage,
                                                       VkImageCreateFlags flags,
                                                       VkImageFormatProperties *pImageFormatProperties)
{
  static PFN_vkGetPhysicalDeviceImageFormatProperties fn = vkGetPhysicalDeviceImageFormatProperties;
  VkResult r = fn(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
  return r;
}

void shim_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDevice fn = vkDestroyDevice;
  fn(device, pAllocator);
  return;
}

VkResult shim_vkEnumerateInstanceVersion(uint32_t *pApiVersion)
{
  static PFN_vkEnumerateInstanceVersion fn = vkEnumerateInstanceVersion;
  VkResult r = fn(pApiVersion);
  return r;
}

VkResult shim_vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                                 VkLayerProperties *pProperties)
{
  static PFN_vkEnumerateInstanceLayerProperties fn = vkEnumerateInstanceLayerProperties;
  VkResult r = fn(pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount,
                                                     VkExtensionProperties *pProperties)
{
  static PFN_vkEnumerateInstanceExtensionProperties fn = vkEnumerateInstanceExtensionProperties;
  VkResult r = fn(pLayerName, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                                               uint32_t *pPropertyCount,
                                               VkLayerProperties *pProperties)
{
  static PFN_vkEnumerateDeviceLayerProperties fn = vkEnumerateDeviceLayerProperties;
  VkResult r = fn(physicalDevice, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                   const char *pLayerName, uint32_t *pPropertyCount,
                                                   VkExtensionProperties *pProperties)
{
  static PFN_vkEnumerateDeviceExtensionProperties fn = vkEnumerateDeviceExtensionProperties;
  VkResult r = fn(physicalDevice, pLayerName, pPropertyCount, pProperties);
  return r;
}

void shim_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                           VkQueue *pQueue)
{
  static PFN_vkGetDeviceQueue fn = vkGetDeviceQueue;
  fn(device, queueFamilyIndex, queueIndex, pQueue);
  return;
}

VkResult shim_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                            VkFence fence)
{
  static PFN_vkQueueSubmit fn = vkQueueSubmit;
  VkResult r = fn(queue, submitCount, pSubmits, fence);
  return r;
}

VkResult shim_vkQueueWaitIdle(VkQueue queue)
{
  static PFN_vkQueueWaitIdle fn = vkQueueWaitIdle;
  VkResult r = fn(queue);
  return r;
}

VkResult shim_vkDeviceWaitIdle(VkDevice device)
{
  static PFN_vkDeviceWaitIdle fn = vkDeviceWaitIdle;
  VkResult r = fn(device);
  return r;
}

VkResult shim_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory)
{
  static PFN_vkAllocateMemory fn = vkAllocateMemory;
  VkResult r = fn(device, pAllocateInfo, pAllocator, pMemory);
  return r;
}

void shim_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkFreeMemory fn = vkFreeMemory;
  fn(device, memory, pAllocator);
  return;
}

VkResult shim_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset,
                          VkDeviceSize size, VkMemoryMapFlags flags, void **ppData)
{
  static PFN_vkMapMemory fn = vkMapMemory;
  VkResult r = fn(device, memory, offset, size, flags, ppData);
  return r;
}

void shim_vkUnmapMemory(VkDevice device, VkDeviceMemory memory)
{
  static PFN_vkUnmapMemory fn = vkUnmapMemory;
  fn(device, memory);
  return;
}

VkResult shim_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                        const VkMappedMemoryRange *pMemoryRanges)
{
  static PFN_vkFlushMappedMemoryRanges fn = vkFlushMappedMemoryRanges;
  VkResult r = fn(device, memoryRangeCount, pMemoryRanges);
  return r;
}

VkResult shim_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                             const VkMappedMemoryRange *pMemoryRanges)
{
  static PFN_vkInvalidateMappedMemoryRanges fn = vkInvalidateMappedMemoryRanges;
  VkResult r = fn(device, memoryRangeCount, pMemoryRanges);
  return r;
}

void shim_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                      VkDeviceSize *pCommittedMemoryInBytes)
{
  static PFN_vkGetDeviceMemoryCommitment fn = vkGetDeviceMemoryCommitment;
  fn(device, memory, pCommittedMemoryInBytes);
  return;
}

void shim_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                        VkMemoryRequirements *pMemoryRequirements)
{
  static PFN_vkGetBufferMemoryRequirements fn = vkGetBufferMemoryRequirements;
  fn(device, buffer, pMemoryRequirements);
  return;
}

VkResult shim_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                 VkDeviceSize memoryOffset)
{
  static PFN_vkBindBufferMemory fn = vkBindBufferMemory;
  VkResult r = fn(device, buffer, memory, memoryOffset);
  return r;
}

void shim_vkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                       VkMemoryRequirements *pMemoryRequirements)
{
  static PFN_vkGetImageMemoryRequirements fn = vkGetImageMemoryRequirements;
  fn(device, image, pMemoryRequirements);
  return;
}

VkResult shim_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                VkDeviceSize memoryOffset)
{
  static PFN_vkBindImageMemory fn = vkBindImageMemory;
  VkResult r = fn(device, image, memory, memoryOffset);
  return r;
}

void shim_vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                             uint32_t *pSparseMemoryRequirementCount,
                                             VkSparseImageMemoryRequirements *pSparseMemoryRequirements)
{
  static PFN_vkGetImageSparseMemoryRequirements fn = vkGetImageSparseMemoryRequirements;
  fn(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
  return;
}

void shim_vkGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
    VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties)
{
  static PFN_vkGetPhysicalDeviceSparseImageFormatProperties fn =
      vkGetPhysicalDeviceSparseImageFormatProperties;
  fn(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
  return;
}
)" + std::string(R"(
VkResult shim_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount,
                                const VkBindSparseInfo *pBindInfo, VkFence fence)
{
  static PFN_vkQueueBindSparse fn = vkQueueBindSparse;
  VkResult r = fn(queue, bindInfoCount, pBindInfo, fence);
  return r;
}

VkResult shim_vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkFence *pFence)
{
  static PFN_vkCreateFence fn = vkCreateFence;
  VkResult r = fn(device, pCreateInfo, pAllocator, pFence);
  return r;
}

void shim_vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyFence fn = vkDestroyFence;
  fn(device, fence, pAllocator);
  return;
}

VkResult shim_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences)
{
  static PFN_vkResetFences fn = vkResetFences;
  VkResult r = fn(device, fenceCount, pFences);
  return r;
}

VkResult shim_vkGetFenceStatus(VkDevice device, VkFence fence)
{
  static PFN_vkGetFenceStatus fn = vkGetFenceStatus;
  VkResult r = fn(device, fence);
  return r;
}

VkResult shim_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                              VkBool32 waitAll, uint64_t timeout)
{
  static PFN_vkWaitForFences fn = vkWaitForFences;
  VkResult r = fn(device, fenceCount, pFences, waitAll, timeout);
  return r;
}

VkResult shim_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore)
{
  static PFN_vkCreateSemaphore fn = vkCreateSemaphore;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSemaphore);
  return r;
}

void shim_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySemaphore fn = vkDestroySemaphore;
  fn(device, semaphore, pAllocator);
  return;
}

VkResult shim_vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkEvent *pEvent)
{
  static PFN_vkCreateEvent fn = vkCreateEvent;
  VkResult r = fn(device, pCreateInfo, pAllocator, pEvent);
  return r;
}

void shim_vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyEvent fn = vkDestroyEvent;
  fn(device, event, pAllocator);
  return;
}

VkResult shim_vkGetEventStatus(VkDevice device, VkEvent event)
{
  static PFN_vkGetEventStatus fn = vkGetEventStatus;
  VkResult r = fn(device, event);
  return r;
}

VkResult shim_vkSetEvent(VkDevice device, VkEvent event)
{
  static PFN_vkSetEvent fn = vkSetEvent;
  VkResult r = fn(device, event);
  return r;
}

VkResult shim_vkResetEvent(VkDevice device, VkEvent event)
{
  static PFN_vkResetEvent fn = vkResetEvent;
  VkResult r = fn(device, event);
  return r;
}

VkResult shim_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool)
{
  static PFN_vkCreateQueryPool fn = vkCreateQueryPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pQueryPool);
  return r;
}

void shim_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyQueryPool fn = vkDestroyQueryPool;
  fn(device, queryPool, pAllocator);
  return;
}

VkResult shim_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                    uint32_t queryCount, size_t dataSize, void *pData,
                                    VkDeviceSize stride, VkQueryResultFlags flags)
{
  static PFN_vkGetQueryPoolResults fn = vkGetQueryPoolResults;
  VkResult r = fn(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
  return r;
}

VkResult shim_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer)
{
  static PFN_vkCreateBuffer fn = vkCreateBuffer;
  VkResult r = fn(device, pCreateInfo, pAllocator, pBuffer);
  return r;
}

void shim_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyBuffer fn = vkDestroyBuffer;
  fn(device, buffer, pAllocator);
  return;
}

VkResult shim_vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkBufferView *pView)
{
  static PFN_vkCreateBufferView fn = vkCreateBufferView;
  VkResult r = fn(device, pCreateInfo, pAllocator, pView);
  return r;
}

void shim_vkDestroyBufferView(VkDevice device, VkBufferView bufferView,
                              const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyBufferView fn = vkDestroyBufferView;
  fn(device, bufferView, pAllocator);
  return;
}

VkResult shim_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkImage *pImage)
{
  static PFN_vkCreateImage fn = vkCreateImage;
  VkResult r = fn(device, pCreateInfo, pAllocator, pImage);
  return r;
}

void shim_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyImage fn = vkDestroyImage;
  fn(device, image, pAllocator);
  return;
}

void shim_vkGetImageSubresourceLayout(VkDevice device, VkImage image,
                                      const VkImageSubresource *pSubresource,
                                      VkSubresourceLayout *pLayout)
{
  static PFN_vkGetImageSubresourceLayout fn = vkGetImageSubresourceLayout;
  fn(device, image, pSubresource, pLayout);
  return;
}

VkResult shim_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkImageView *pView)
{
  static PFN_vkCreateImageView fn = vkCreateImageView;
  VkResult r = fn(device, pCreateInfo, pAllocator, pView);
  return r;
}

void shim_vkDestroyImageView(VkDevice device, VkImageView imageView,
                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyImageView fn = vkDestroyImageView;
  fn(device, imageView, pAllocator);
  return;
}

VkResult shim_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator,
                                   VkShaderModule *pShaderModule)
{
  static PFN_vkCreateShaderModule fn = vkCreateShaderModule;
  VkResult r = fn(device, pCreateInfo, pAllocator, pShaderModule);
  return r;
}

void shim_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyShaderModule fn = vkDestroyShaderModule;
  fn(device, shaderModule, pAllocator);
  return;
}

VkResult shim_vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator,
                                    VkPipelineCache *pPipelineCache)
{
  static PFN_vkCreatePipelineCache fn = vkCreatePipelineCache;
  VkResult r = fn(device, pCreateInfo, pAllocator, pPipelineCache);
  return r;
}

void shim_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                 const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyPipelineCache fn = vkDestroyPipelineCache;
  fn(device, pipelineCache, pAllocator);
  return;
}

VkResult shim_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache,
                                     size_t *pDataSize, void *pData)
{
  static PFN_vkGetPipelineCacheData fn = vkGetPipelineCacheData;
  VkResult r = fn(device, pipelineCache, pDataSize, pData);
  return r;
}

VkResult shim_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache,
                                    uint32_t srcCacheCount, const VkPipelineCache *pSrcCaches)
{
  static PFN_vkMergePipelineCaches fn = vkMergePipelineCaches;
  VkResult r = fn(device, dstCache, srcCacheCount, pSrcCaches);
  return r;
}

void shim_vkDestroyPipeline(VkDevice device, VkPipeline pipeline,
                            const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyPipeline fn = vkDestroyPipeline;
  fn(device, pipeline, pAllocator);
  return;
}

VkResult shim_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkPipelineLayout *pPipelineLayout)
{
  static PFN_vkCreatePipelineLayout fn = vkCreatePipelineLayout;
  VkResult r = fn(device, pCreateInfo, pAllocator, pPipelineLayout);
  return r;
}

void shim_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                  const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyPipelineLayout fn = vkDestroyPipelineLayout;
  fn(device, pipelineLayout, pAllocator);
  return;
}

VkResult shim_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler)
{
  static PFN_vkCreateSampler fn = vkCreateSampler;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSampler);
  return r;
}

void shim_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySampler fn = vkDestroySampler;
  fn(device, sampler, pAllocator);
  return;
}

VkResult shim_vkCreateDescriptorSetLayout(VkDevice device,
                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDescriptorSetLayout *pSetLayout)
{
  static PFN_vkCreateDescriptorSetLayout fn = vkCreateDescriptorSetLayout;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSetLayout);
  return r;
}

void shim_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                       const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDescriptorSetLayout fn = vkDestroyDescriptorSetLayout;
  fn(device, descriptorSetLayout, pAllocator);
  return;
}

VkResult shim_vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkDescriptorPool *pDescriptorPool)
{
  static PFN_vkCreateDescriptorPool fn = vkCreateDescriptorPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorPool);
  return r;
}

void shim_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                  const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDescriptorPool fn = vkDestroyDescriptorPool;
  fn(device, descriptorPool, pAllocator);
  return;
}

VkResult shim_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                    VkDescriptorPoolResetFlags flags)
{
  static PFN_vkResetDescriptorPool fn = vkResetDescriptorPool;
  VkResult r = fn(device, descriptorPool, flags);
  return r;
}

VkResult shim_vkAllocateDescriptorSets(VkDevice device,
                                       const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                       VkDescriptorSet *pDescriptorSets)
{
  static PFN_vkAllocateDescriptorSets fn = vkAllocateDescriptorSets;
  VkResult r = fn(device, pAllocateInfo, pDescriptorSets);
  return r;
}

VkResult shim_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                   uint32_t descriptorSetCount,
                                   const VkDescriptorSet *pDescriptorSets)
{
  static PFN_vkFreeDescriptorSets fn = vkFreeDescriptorSets;
  VkResult r = fn(device, descriptorPool, descriptorSetCount, pDescriptorSets);
  return r;
}

void shim_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                 const VkWriteDescriptorSet *pDescriptorWrites,
                                 uint32_t descriptorCopyCount,
                                 const VkCopyDescriptorSet *pDescriptorCopies)
{
  static PFN_vkUpdateDescriptorSets fn = vkUpdateDescriptorSets;
  fn(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
  return;
}

VkResult shim_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator,
                                  VkFramebuffer *pFramebuffer)
{
  static PFN_vkCreateFramebuffer fn = vkCreateFramebuffer;
  VkResult r = fn(device, pCreateInfo, pAllocator, pFramebuffer);
  return r;
}

void shim_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                               const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyFramebuffer fn = vkDestroyFramebuffer;
  fn(device, framebuffer, pAllocator);
  return;
}

VkResult shim_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass)
{
  static PFN_vkCreateRenderPass fn = vkCreateRenderPass;
  VkResult r = fn(device, pCreateInfo, pAllocator, pRenderPass);
  return r;
}

void shim_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                              const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyRenderPass fn = vkDestroyRenderPass;
  fn(device, renderPass, pAllocator);
  return;
}

void shim_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass,
                                     VkExtent2D *pGranularity)
{
  static PFN_vkGetRenderAreaGranularity fn = vkGetRenderAreaGranularity;
  fn(device, renderPass, pGranularity);
  return;
}

VkResult shim_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator,
                                  VkCommandPool *pCommandPool)
{
  static PFN_vkCreateCommandPool fn = vkCreateCommandPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pCommandPool);
  return r;
}

void shim_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                               const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyCommandPool fn = vkDestroyCommandPool;
  fn(device, commandPool, pAllocator);
  return;
}

VkResult shim_vkResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                 VkCommandPoolResetFlags flags)
{
  static PFN_vkResetCommandPool fn = vkResetCommandPool;
  VkResult r = fn(device, commandPool, flags);
  return r;
}

VkResult shim_vkAllocateCommandBuffers(VkDevice device,
                                       const VkCommandBufferAllocateInfo *pAllocateInfo,
                                       VkCommandBuffer *pCommandBuffers)
{
  static PFN_vkAllocateCommandBuffers fn = vkAllocateCommandBuffers;
  VkResult r = fn(device, pAllocateInfo, pCommandBuffers);
  return r;
}

void shim_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                               uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers)
{
  static PFN_vkFreeCommandBuffers fn = vkFreeCommandBuffers;
  fn(device, commandPool, commandBufferCount, pCommandBuffers);
  return;
}

VkResult shim_vkBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                   const VkCommandBufferBeginInfo *pBeginInfo)
{
  static PFN_vkBeginCommandBuffer fn = vkBeginCommandBuffer;
  VkResult r = fn(commandBuffer, pBeginInfo);
  return r;
}

VkResult shim_vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
  static PFN_vkEndCommandBuffer fn = vkEndCommandBuffer;
  VkResult r = fn(commandBuffer);
  return r;
}
)") + std::string(R"(
VkResult shim_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
  static PFN_vkResetCommandBuffer fn = vkResetCommandBuffer;
  VkResult r = fn(commandBuffer, flags);
  return r;
}

void shim_vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                            VkPipeline pipeline)
{
  static PFN_vkCmdBindPipeline fn = vkCmdBindPipeline;
  fn(commandBuffer, pipelineBindPoint, pipeline);
  return;
}

void shim_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                           uint32_t viewportCount, const VkViewport *pViewports)
{
  static PFN_vkCmdSetViewport fn = vkCmdSetViewport;
  fn(commandBuffer, firstViewport, viewportCount, pViewports);
  return;
}

void shim_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                          uint32_t scissorCount, const VkRect2D *pScissors)
{
  static PFN_vkCmdSetScissor fn = vkCmdSetScissor;
  fn(commandBuffer, firstScissor, scissorCount, pScissors);
  return;
}

void shim_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
  static PFN_vkCmdSetLineWidth fn = vkCmdSetLineWidth;
  fn(commandBuffer, lineWidth);
  return;
}

void shim_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                            float depthBiasClamp, float depthBiasSlopeFactor)
{
  static PFN_vkCmdSetDepthBias fn = vkCmdSetDepthBias;
  fn(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
  return;
}

void shim_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
  static PFN_vkCmdSetBlendConstants fn = vkCmdSetBlendConstants;
  fn(commandBuffer, blendConstants);
  return;
}

void shim_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                              float maxDepthBounds)
{
  static PFN_vkCmdSetDepthBounds fn = vkCmdSetDepthBounds;
  fn(commandBuffer, minDepthBounds, maxDepthBounds);
  return;
}

void shim_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                     uint32_t compareMask)
{
  static PFN_vkCmdSetStencilCompareMask fn = vkCmdSetStencilCompareMask;
  fn(commandBuffer, faceMask, compareMask);
  return;
}

void shim_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                   uint32_t writeMask)
{
  static PFN_vkCmdSetStencilWriteMask fn = vkCmdSetStencilWriteMask;
  fn(commandBuffer, faceMask, writeMask);
  return;
}

void shim_vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                   uint32_t reference)
{
  static PFN_vkCmdSetStencilReference fn = vkCmdSetStencilReference;
  fn(commandBuffer, faceMask, reference);
  return;
}

void shim_vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                  VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                  uint32_t firstSet, uint32_t descriptorSetCount,
                                  const VkDescriptorSet *pDescriptorSets,
                                  uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets)
{
  static PFN_vkCmdBindDescriptorSets fn = vkCmdBindDescriptorSets;
  fn(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
     dynamicOffsetCount, pDynamicOffsets);
  return;
}

void shim_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                               VkIndexType indexType)
{
  static PFN_vkCmdBindIndexBuffer fn = vkCmdBindIndexBuffer;
  fn(commandBuffer, buffer, offset, indexType);
  return;
}

void shim_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                 uint32_t bindingCount, const VkBuffer *pBuffers,
                                 const VkDeviceSize *pOffsets)
{
  static PFN_vkCmdBindVertexBuffers fn = vkCmdBindVertexBuffers;
  fn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
  return;
}

void shim_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                    uint32_t firstVertex, uint32_t firstInstance)
{
  static PFN_vkCmdDraw fn = vkCmdDraw;
  fn(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
  return;
}

void shim_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                           uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
  static PFN_vkCmdDrawIndexed fn = vkCmdDrawIndexed;
  fn(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
  return;
}

void shim_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                            uint32_t drawCount, uint32_t stride)
{
  static PFN_vkCmdDrawIndirect fn = vkCmdDrawIndirect;
  fn(commandBuffer, buffer, offset, drawCount, stride);
  return;
}

void shim_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                   VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
  static PFN_vkCmdDrawIndexedIndirect fn = vkCmdDrawIndexedIndirect;
  fn(commandBuffer, buffer, offset, drawCount, stride);
  return;
}

void shim_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                        uint32_t groupCountZ)
{
  static PFN_vkCmdDispatch fn = vkCmdDispatch;
  fn(commandBuffer, groupCountX, groupCountY, groupCountZ);
  return;
}

void shim_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
  static PFN_vkCmdDispatchIndirect fn = vkCmdDispatchIndirect;
  fn(commandBuffer, buffer, offset);
  return;
}

void shim_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                          uint32_t regionCount, const VkBufferCopy *pRegions)
{
  static PFN_vkCmdCopyBuffer fn = vkCmdCopyBuffer;
  fn(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
  return;
}

void shim_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                         VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                         uint32_t regionCount, const VkImageCopy *pRegions)
{
  static PFN_vkCmdCopyImage fn = vkCmdCopyImage;
  fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
  return;
}

void shim_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                         VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                         uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter)
{
  static PFN_vkCmdBlitImage fn = vkCmdBlitImage;
  fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
     filter);
  return;
}

void shim_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                 VkImage dstImage, VkImageLayout dstImageLayout,
                                 uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
  static PFN_vkCmdCopyBufferToImage fn = vkCmdCopyBufferToImage;
  fn(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
  return;
}

void shim_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                 VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                 uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
  static PFN_vkCmdCopyImageToBuffer fn = vkCmdCopyImageToBuffer;
  fn(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
  return;
}

void shim_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                            VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData)
{
  static PFN_vkCmdUpdateBuffer fn = vkCmdUpdateBuffer;
  fn(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
  return;
}

void shim_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                          VkDeviceSize size, uint32_t data)
{
  static PFN_vkCmdFillBuffer fn = vkCmdFillBuffer;
  fn(commandBuffer, dstBuffer, dstOffset, size, data);
  return;
}

void shim_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                               VkImageLayout imageLayout, const VkClearColorValue *pColor,
                               uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
  static PFN_vkCmdClearColorImage fn = vkCmdClearColorImage;
  fn(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
  return;
}

void shim_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                      VkImageLayout imageLayout,
                                      const VkClearDepthStencilValue *pDepthStencil,
                                      uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
  static PFN_vkCmdClearDepthStencilImage fn = vkCmdClearDepthStencilImage;
  fn(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
  return;
}

void shim_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                const VkClearAttachment *pAttachments, uint32_t rectCount,
                                const VkClearRect *pRects)
{
  static PFN_vkCmdClearAttachments fn = vkCmdClearAttachments;
  fn(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
  return;
}

void shim_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                            VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount,
                            const VkImageResolve *pRegions)
{
  static PFN_vkCmdResolveImage fn = vkCmdResolveImage;
  fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
  return;
}

void shim_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
  static PFN_vkCmdSetEvent fn = vkCmdSetEvent;
  fn(commandBuffer, event, stageMask);
  return;
}

void shim_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
  static PFN_vkCmdResetEvent fn = vkCmdResetEvent;
  fn(commandBuffer, event, stageMask);
  return;
}

void shim_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount,
                          const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                          VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                          const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                          const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                          uint32_t imageMemoryBarrierCount,
                          const VkImageMemoryBarrier *pImageMemoryBarriers)
{
  static PFN_vkCmdWaitEvents fn = vkCmdWaitEvents;
  fn(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
     pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
     pImageMemoryBarriers);
  return;
}

void shim_vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                               uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                               uint32_t bufferMemoryBarrierCount,
                               const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                               uint32_t imageMemoryBarrierCount,
                               const VkImageMemoryBarrier *pImageMemoryBarriers)
{
  static PFN_vkCmdPipelineBarrier fn = vkCmdPipelineBarrier;
  fn(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
     bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
  return;
}

void shim_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                          VkQueryControlFlags flags)
{
  static PFN_vkCmdBeginQuery fn = vkCmdBeginQuery;
  fn(commandBuffer, queryPool, query, flags);
  return;
}

void shim_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
  static PFN_vkCmdEndQuery fn = vkCmdEndQuery;
  fn(commandBuffer, queryPool, query);
  return;
}

void shim_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                              uint32_t firstQuery, uint32_t queryCount)
{
  static PFN_vkCmdResetQueryPool fn = vkCmdResetQueryPool;
  fn(commandBuffer, queryPool, firstQuery, queryCount);
  return;
}

void shim_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                              VkQueryPool queryPool, uint32_t query)
{
  static PFN_vkCmdWriteTimestamp fn = vkCmdWriteTimestamp;
  fn(commandBuffer, pipelineStage, queryPool, query);
  return;
}

void shim_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                    uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                    VkDeviceSize dstOffset, VkDeviceSize stride,
                                    VkQueryResultFlags flags)
{
  static PFN_vkCmdCopyQueryPoolResults fn = vkCmdCopyQueryPoolResults;
  fn(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
  return;
}

void shim_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                             VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                             const void *pValues)
{
  static PFN_vkCmdPushConstants fn = vkCmdPushConstants;
  fn(commandBuffer, layout, stageFlags, offset, size, pValues);
  return;
}

void shim_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                               const VkRenderPassBeginInfo *pRenderPassBegin,
                               VkSubpassContents contents)
{
  static PFN_vkCmdBeginRenderPass fn = vkCmdBeginRenderPass;
  fn(commandBuffer, pRenderPassBegin, contents);
  return;
}

void shim_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
  static PFN_vkCmdNextSubpass fn = vkCmdNextSubpass;
  fn(commandBuffer, contents);
  return;
}

void shim_vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
  static PFN_vkCmdEndRenderPass fn = vkCmdEndRenderPass;
  fn(commandBuffer);
  return;
}

void shim_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                               const VkCommandBuffer *pCommandBuffers)
{
  static PFN_vkCmdExecuteCommands fn = vkCmdExecuteCommands;
  fn(commandBuffer, commandBufferCount, pCommandBuffers);
  return;
}

VkResult shim_vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                      uint32_t *pPropertyCount,
                                                      VkDisplayPropertiesKHR *pProperties)
{
  static PFN_vkGetPhysicalDeviceDisplayPropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
  VkResult r = fn(physicalDevice, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                           uint32_t *pPropertyCount,
                                                           VkDisplayPlanePropertiesKHR *pProperties)
{
  static PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
  VkResult r = fn(physicalDevice, pPropertyCount, pProperties);
  return r;
}
)") + std::string(R"(
VkResult shim_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice,
                                                    uint32_t planeIndex, uint32_t *pDisplayCount,
                                                    VkDisplayKHR *pDisplays)
{
  static PFN_vkGetDisplayPlaneSupportedDisplaysKHR fn =
      (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
  VkResult r = fn(physicalDevice, planeIndex, pDisplayCount, pDisplays);
  return r;
}

VkResult shim_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                            uint32_t *pPropertyCount,
                                            VkDisplayModePropertiesKHR *pProperties)
{
  static PFN_vkGetDisplayModePropertiesKHR fn =
      (PFN_vkGetDisplayModePropertiesKHR)vkGetInstanceProcAddr(aux.instance,
                                                               "vkGetDisplayModePropertiesKHR");
  VkResult r = fn(physicalDevice, display, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                     const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode)
{
  static PFN_vkCreateDisplayModeKHR fn =
      (PFN_vkCreateDisplayModeKHR)vkGetInstanceProcAddr(aux.instance, "vkCreateDisplayModeKHR");
  VkResult r = fn(physicalDevice, display, pCreateInfo, pAllocator, pMode);
  return r;
}

VkResult shim_vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                               VkDisplayModeKHR mode, uint32_t planeIndex,
                                               VkDisplayPlaneCapabilitiesKHR *pCapabilities)
{
  static PFN_vkGetDisplayPlaneCapabilitiesKHR fn =
      (PFN_vkGetDisplayPlaneCapabilitiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetDisplayPlaneCapabilitiesKHR");
  VkResult r = fn(physicalDevice, mode, planeIndex, pCapabilities);
  return r;
}

VkResult shim_vkCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                             const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkSurfaceKHR *pSurface)
{
  static PFN_vkCreateDisplayPlaneSurfaceKHR fn =
      (PFN_vkCreateDisplayPlaneSurfaceKHR)vkGetInstanceProcAddr(instance,
                                                                "vkCreateDisplayPlaneSurfaceKHR");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pSurface);
  return r;
}

VkResult shim_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkSwapchainKHR *pSwapchains)
{
  static PFN_vkCreateSharedSwapchainsKHR fn =
      (PFN_vkCreateSharedSwapchainsKHR)vkGetDeviceProcAddr(device, "vkCreateSharedSwapchainsKHR");
  VkResult r = fn(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
  return r;
}

void shim_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                              const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySurfaceKHR fn =
      (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR");
  fn(instance, surface, pAllocator);
  return;
}

VkResult shim_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                   uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                   VkBool32 *pSupported)
{
  static PFN_vkGetPhysicalDeviceSurfaceSupportKHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
  VkResult r = fn(physicalDevice, queueFamilyIndex, surface, pSupported);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                        VkSurfaceKHR surface,
                                                        VkSurfaceCapabilitiesKHR *pSurfaceCapabilities)
{
  static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
  VkResult r = fn(physicalDevice, surface, pSurfaceCapabilities);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                                                   VkSurfaceKHR surface,
                                                   uint32_t *pSurfaceFormatCount,
                                                   VkSurfaceFormatKHR *pSurfaceFormats)
{
  static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
  VkResult r = fn(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                        VkSurfaceKHR surface,
                                                        uint32_t *pPresentModeCount,
                                                        VkPresentModeKHR *pPresentModes)
{
  static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fn =
      (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
  VkResult r = fn(physicalDevice, surface, pPresentModeCount, pPresentModes);
  return r;
}

VkResult shim_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator,
                                   VkSwapchainKHR *pSwapchain)
{
  static PFN_vkCreateSwapchainKHR fn =
      (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pSwapchain);
  return r;
}

void shim_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySwapchainKHR fn =
      (PFN_vkDestroySwapchainKHR)vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
  fn(device, swapchain, pAllocator);
  return;
}

VkResult shim_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                      uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages)
{
  static PFN_vkGetSwapchainImagesKHR fn =
      (PFN_vkGetSwapchainImagesKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
  VkResult r = fn(device, swapchain, pSwapchainImageCount, pSwapchainImages);
  return r;
}

VkResult shim_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                    VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
{
  static PFN_vkAcquireNextImageKHR fn =
      (PFN_vkAcquireNextImageKHR)vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
  VkResult r = fn(device, swapchain, timeout, semaphore, fence, pImageIndex);
  return r;
}

VkResult shim_vkCreateDebugReportCallbackEXT(VkInstance instance,
                                             const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkDebugReportCallbackEXT *pCallback)
{
  static PFN_vkCreateDebugReportCallbackEXT fn =
      (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,
                                                                "vkCreateDebugReportCallbackEXT");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pCallback);
  return r;
}

void shim_vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                          const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDebugReportCallbackEXT fn =
      (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,
                                                                 "vkDestroyDebugReportCallbackEXT");
  fn(instance, callback, pAllocator);
  return;
}

void shim_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                  VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                  size_t location, int32_t messageCode, const char *pLayerPrefix,
                                  const char *pMessage)
{
  static PFN_vkDebugReportMessageEXT fn =
      (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");
  fn(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
  return;
}

VkResult shim_vkDebugMarkerSetObjectNameEXT(VkDevice device,
                                            const VkDebugMarkerObjectNameInfoEXT *pNameInfo)
{
  static PFN_vkDebugMarkerSetObjectNameEXT fn =
      (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device,
                                                             "vkDebugMarkerSetObjectNameEXT");
  VkResult r = fn(device, pNameInfo);
  return r;
}

VkResult shim_vkDebugMarkerSetObjectTagEXT(VkDevice device,
                                           const VkDebugMarkerObjectTagInfoEXT *pTagInfo)
{
  static PFN_vkDebugMarkerSetObjectTagEXT fn =
      (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
  VkResult r = fn(device, pTagInfo);
  return r;
}

void shim_vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                   const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
  static PFN_vkCmdDebugMarkerBeginEXT fn =
      (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(aux.device, "vkCmdDebugMarkerBeginEXT");
  fn(commandBuffer, pMarkerInfo);
  return;
}

void shim_vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer)
{
  static PFN_vkCmdDebugMarkerEndEXT fn =
      (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(aux.device, "vkCmdDebugMarkerEndEXT");
  fn(commandBuffer);
  return;
}

void shim_vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
                                    const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
  static PFN_vkCmdDebugMarkerInsertEXT fn =
      (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(aux.device, "vkCmdDebugMarkerInsertEXT");
  fn(commandBuffer, pMarkerInfo);
  return;
}

VkResult shim_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties)
{
  static PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV fn =
      (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
  VkResult r = fn(physicalDevice, format, type, tiling, usage, flags, externalHandleType,
                  pExternalImageFormatProperties);
  return r;
}

void shim_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                    VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                    uint32_t stride)
{
  static PFN_vkCmdDrawIndirectCountAMD fn =
      (PFN_vkCmdDrawIndirectCountAMD)vkGetDeviceProcAddr(aux.device, "vkCmdDrawIndirectCountAMD");
  fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
  return;
}

void shim_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                           VkDeviceSize offset, VkBuffer countBuffer,
                                           VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride)
{
  static PFN_vkCmdDrawIndexedIndirectCountAMD fn =
      (PFN_vkCmdDrawIndexedIndirectCountAMD)vkGetDeviceProcAddr(aux.device,
                                                                "vkCmdDrawIndexedIndirectCountAMD");
  fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
  return;
}

void shim_vkCmdProcessCommandsNVX(VkCommandBuffer commandBuffer,
                                  const VkCmdProcessCommandsInfoNVX *pProcessCommandsInfo)
{
  static PFN_vkCmdProcessCommandsNVX fn =
      (PFN_vkCmdProcessCommandsNVX)vkGetDeviceProcAddr(aux.device, "vkCmdProcessCommandsNVX");
  fn(commandBuffer, pProcessCommandsInfo);
  return;
}

void shim_vkCmdReserveSpaceForCommandsNVX(VkCommandBuffer commandBuffer,
                                          const VkCmdReserveSpaceForCommandsInfoNVX *pReserveSpaceInfo)
{
  static PFN_vkCmdReserveSpaceForCommandsNVX fn =
      (PFN_vkCmdReserveSpaceForCommandsNVX)vkGetDeviceProcAddr(aux.device,
                                                               "vkCmdReserveSpaceForCommandsNVX");
  fn(commandBuffer, pReserveSpaceInfo);
  return;
}

VkResult shim_vkCreateIndirectCommandsLayoutNVX(
    VkDevice device, const VkIndirectCommandsLayoutCreateInfoNVX *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNVX *pIndirectCommandsLayout)
{
  static PFN_vkCreateIndirectCommandsLayoutNVX fn =
      (PFN_vkCreateIndirectCommandsLayoutNVX)vkGetDeviceProcAddr(
          device, "vkCreateIndirectCommandsLayoutNVX");
  VkResult r = fn(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
  return r;
}

void shim_vkDestroyIndirectCommandsLayoutNVX(VkDevice device,
                                             VkIndirectCommandsLayoutNVX indirectCommandsLayout,
                                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyIndirectCommandsLayoutNVX fn =
      (PFN_vkDestroyIndirectCommandsLayoutNVX)vkGetDeviceProcAddr(
          device, "vkDestroyIndirectCommandsLayoutNVX");
  fn(device, indirectCommandsLayout, pAllocator);
  return;
}

VkResult shim_vkCreateObjectTableNVX(VkDevice device, const VkObjectTableCreateInfoNVX *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkObjectTableNVX *pObjectTable)
{
  static PFN_vkCreateObjectTableNVX fn =
      (PFN_vkCreateObjectTableNVX)vkGetDeviceProcAddr(device, "vkCreateObjectTableNVX");
  VkResult r = fn(device, pCreateInfo, pAllocator, pObjectTable);
  return r;
}

void shim_vkDestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable,
                                  const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyObjectTableNVX fn =
      (PFN_vkDestroyObjectTableNVX)vkGetDeviceProcAddr(device, "vkDestroyObjectTableNVX");
  fn(device, objectTable, pAllocator);
  return;
}

VkResult shim_vkRegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                   uint32_t objectCount,
                                   const VkObjectTableEntryNVX *const *ppObjectTableEntries,
                                   const uint32_t *pObjectIndices)
{
  static PFN_vkRegisterObjectsNVX fn =
      (PFN_vkRegisterObjectsNVX)vkGetDeviceProcAddr(device, "vkRegisterObjectsNVX");
  VkResult r = fn(device, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);
  return r;
}
)") + std::string(R"(
VkResult shim_vkUnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                     uint32_t objectCount,
                                     const VkObjectEntryTypeNVX *pObjectEntryTypes,
                                     const uint32_t *pObjectIndices)
{
  static PFN_vkUnregisterObjectsNVX fn =
      (PFN_vkUnregisterObjectsNVX)vkGetDeviceProcAddr(device, "vkUnregisterObjectsNVX");
  VkResult r = fn(device, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);
  return r;
}

void shim_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice physicalDevice, VkDeviceGeneratedCommandsFeaturesNVX *pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX *pLimits)
{
  static PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX fn =
      (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX)vkGetDeviceProcAddr(
          aux.device, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
  fn(physicalDevice, pFeatures, pLimits);
  return;
}

void shim_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                       VkPhysicalDeviceFeatures2 *pFeatures)
{
  static PFN_vkGetPhysicalDeviceFeatures2 fn =
      (PFN_vkGetPhysicalDeviceFeatures2)vkGetInstanceProcAddr(aux.instance,
                                                              "vkGetPhysicalDeviceFeatures2");
  fn(physicalDevice, pFeatures);
  return;
}

void shim_vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                          VkPhysicalDeviceFeatures2 *pFeatures)
{
  static PFN_vkGetPhysicalDeviceFeatures2KHR fn =
      (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(aux.instance,
                                                                 "vkGetPhysicalDeviceFeatures2KHR");
  fn(physicalDevice, pFeatures);
  return;
}

void shim_vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                         VkPhysicalDeviceProperties2 *pProperties)
{
  static PFN_vkGetPhysicalDeviceProperties2 fn =
      (PFN_vkGetPhysicalDeviceProperties2)vkGetInstanceProcAddr(aux.instance,
                                                                "vkGetPhysicalDeviceProperties2");
  fn(physicalDevice, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                            VkPhysicalDeviceProperties2 *pProperties)
{
  static PFN_vkGetPhysicalDeviceProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceProperties2KHR");
  fn(physicalDevice, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                               VkFormatProperties2 *pFormatProperties)
{
  static PFN_vkGetPhysicalDeviceFormatProperties2 fn =
      (PFN_vkGetPhysicalDeviceFormatProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceFormatProperties2");
  fn(physicalDevice, format, pFormatProperties);
  return;
}

void shim_vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                  VkFormatProperties2 *pFormatProperties)
{
  static PFN_vkGetPhysicalDeviceFormatProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceFormatProperties2KHR");
  fn(physicalDevice, format, pFormatProperties);
  return;
}

VkResult shim_vkGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties)
{
  static PFN_vkGetPhysicalDeviceImageFormatProperties2 fn =
      (PFN_vkGetPhysicalDeviceImageFormatProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceImageFormatProperties2");
  VkResult r = fn(physicalDevice, pImageFormatInfo, pImageFormatProperties);
  return r;
}

VkResult shim_vkGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties)
{
  static PFN_vkGetPhysicalDeviceImageFormatProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
  VkResult r = fn(physicalDevice, pImageFormatInfo, pImageFormatProperties);
  return r;
}

void shim_vkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                    uint32_t *pQueueFamilyPropertyCount,
                                                    VkQueueFamilyProperties2 *pQueueFamilyProperties)
{
  static PFN_vkGetPhysicalDeviceQueueFamilyProperties2 fn =
      (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  return;
}

void shim_vkGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                       uint32_t *pQueueFamilyPropertyCount,
                                                       VkQueueFamilyProperties2 *pQueueFamilyProperties)
{
  static PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  return;
}

void shim_vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                               VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
{
  static PFN_vkGetPhysicalDeviceMemoryProperties2 fn =
      (PFN_vkGetPhysicalDeviceMemoryProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceMemoryProperties2");
  fn(physicalDevice, pMemoryProperties);
  return;
}

void shim_vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
{
  static PFN_vkGetPhysicalDeviceMemoryProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
  fn(physicalDevice, pMemoryProperties);
  return;
}

void shim_vkGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties)
{
  static PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 fn =
      (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
  fn(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties)
{
  static PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
  fn(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
  return;
}

void shim_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                    VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                    uint32_t set, uint32_t descriptorWriteCount,
                                    const VkWriteDescriptorSet *pDescriptorWrites)
{
  static PFN_vkCmdPushDescriptorSetKHR fn =
      (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(aux.device, "vkCmdPushDescriptorSetKHR");
  fn(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
  return;
}

void shim_vkTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
{
  static PFN_vkTrimCommandPool fn =
      (PFN_vkTrimCommandPool)vkGetDeviceProcAddr(device, "vkTrimCommandPool");
  fn(device, commandPool, flags);
  return;
}

void shim_vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool,
                               VkCommandPoolTrimFlags flags)
{
  static PFN_vkTrimCommandPoolKHR fn =
      (PFN_vkTrimCommandPoolKHR)vkGetDeviceProcAddr(device, "vkTrimCommandPoolKHR");
  fn(device, commandPool, flags);
  return;
}

void shim_vkGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties)
{
  static PFN_vkGetPhysicalDeviceExternalBufferProperties fn =
      (PFN_vkGetPhysicalDeviceExternalBufferProperties)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalBufferProperties");
  fn(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
  return;
}

void shim_vkGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties)
{
  static PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
  fn(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
  return;
}

VkResult shim_vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd)
{
  static PFN_vkGetMemoryFdKHR fn =
      (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdKHR");
  VkResult r = fn(device, pGetFdInfo, pFd);
  return r;
}

VkResult shim_vkGetMemoryFdPropertiesKHR(VkDevice device,
                                         VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                         VkMemoryFdPropertiesKHR *pMemoryFdProperties)
{
  static PFN_vkGetMemoryFdPropertiesKHR fn =
      (PFN_vkGetMemoryFdPropertiesKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdPropertiesKHR");
  VkResult r = fn(device, handleType, fd, pMemoryFdProperties);
  return r;
}

void shim_vkGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
{
  static PFN_vkGetPhysicalDeviceExternalSemaphoreProperties fn =
      (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
  fn(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
  return;
}

void shim_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
{
  static PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
  fn(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
  return;
}

VkResult shim_vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd)
{
  static PFN_vkGetSemaphoreFdKHR fn =
      (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreFdKHR");
  VkResult r = fn(device, pGetFdInfo, pFd);
  return r;
}

VkResult shim_vkImportSemaphoreFdKHR(VkDevice device,
                                     const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo)
{
  static PFN_vkImportSemaphoreFdKHR fn =
      (PFN_vkImportSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkImportSemaphoreFdKHR");
  VkResult r = fn(device, pImportSemaphoreFdInfo);
  return r;
}

void shim_vkGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties)
{
  static PFN_vkGetPhysicalDeviceExternalFenceProperties fn =
      (PFN_vkGetPhysicalDeviceExternalFenceProperties)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalFenceProperties");
  fn(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
  return;
}

void shim_vkGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties)
{
  static PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
  fn(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
  return;
}

VkResult shim_vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd)
{
  static PFN_vkGetFenceFdKHR fn =
      (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(device, "vkGetFenceFdKHR");
  VkResult r = fn(device, pGetFdInfo, pFd);
  return r;
}

VkResult shim_vkImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo)
{
  static PFN_vkImportFenceFdKHR fn =
      (PFN_vkImportFenceFdKHR)vkGetDeviceProcAddr(device, "vkImportFenceFdKHR");
  VkResult r = fn(device, pImportFenceFdInfo);
  return r;
}

VkResult shim_vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display)
{
  static PFN_vkReleaseDisplayEXT fn =
      (PFN_vkReleaseDisplayEXT)vkGetInstanceProcAddr(aux.instance, "vkReleaseDisplayEXT");
  VkResult r = fn(physicalDevice, display);
  return r;
}

VkResult shim_vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                       const VkDisplayPowerInfoEXT *pDisplayPowerInfo)
{
  static PFN_vkDisplayPowerControlEXT fn =
      (PFN_vkDisplayPowerControlEXT)vkGetDeviceProcAddr(device, "vkDisplayPowerControlEXT");
  VkResult r = fn(device, display, pDisplayPowerInfo);
  return r;
}

VkResult shim_vkRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT *pDeviceEventInfo,
                                       const VkAllocationCallbacks *pAllocator, VkFence *pFence)
{
  static PFN_vkRegisterDeviceEventEXT fn =
      (PFN_vkRegisterDeviceEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDeviceEventEXT");
  VkResult r = fn(device, pDeviceEventInfo, pAllocator, pFence);
  return r;
}

VkResult shim_vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                        const VkDisplayEventInfoEXT *pDisplayEventInfo,
                                        const VkAllocationCallbacks *pAllocator, VkFence *pFence)
{
  static PFN_vkRegisterDisplayEventEXT fn =
      (PFN_vkRegisterDisplayEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDisplayEventEXT");
  VkResult r = fn(device, display, pDisplayEventInfo, pAllocator, pFence);
  return r;
}

VkResult shim_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                       VkSurfaceCounterFlagBitsEXT counter, uint64_t *pCounterValue)
{
  static PFN_vkGetSwapchainCounterEXT fn =
      (PFN_vkGetSwapchainCounterEXT)vkGetDeviceProcAddr(device, "vkGetSwapchainCounterEXT");
  VkResult r = fn(device, swapchain, counter, pCounterValue);
  return r;
}
)") + std::string(R"(
VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                         VkSurfaceKHR surface,
                                                         VkSurfaceCapabilities2EXT *pSurfaceCapabilities)
{
  static PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT fn =
      (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
  VkResult r = fn(physicalDevice, surface, pSurfaceCapabilities);
  return r;
}

VkResult shim_vkEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties)
{
  static PFN_vkEnumeratePhysicalDeviceGroups fn =
      (PFN_vkEnumeratePhysicalDeviceGroups)vkGetInstanceProcAddr(instance,
                                                                 "vkEnumeratePhysicalDeviceGroups");
  VkResult r = fn(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
  return r;
}

VkResult shim_vkEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties)
{
  static PFN_vkEnumeratePhysicalDeviceGroupsKHR fn =
      (PFN_vkEnumeratePhysicalDeviceGroupsKHR)vkGetInstanceProcAddr(
          instance, "vkEnumeratePhysicalDeviceGroupsKHR");
  VkResult r = fn(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
  return r;
}

void shim_vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex,
                                             uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
                                             VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
  static PFN_vkGetDeviceGroupPeerMemoryFeatures fn =
      (PFN_vkGetDeviceGroupPeerMemoryFeatures)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupPeerMemoryFeatures");
  fn(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
  return;
}

void shim_vkGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex,
                                                uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
                                                VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
  static PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR fn =
      (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
  fn(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
  return;
}

VkResult shim_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                  const VkBindBufferMemoryInfo *pBindInfos)
{
  static PFN_vkBindBufferMemory2 fn =
      (PFN_vkBindBufferMemory2)vkGetDeviceProcAddr(device, "vkBindBufferMemory2");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

VkResult shim_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                     const VkBindBufferMemoryInfo *pBindInfos)
{
  static PFN_vkBindBufferMemory2KHR fn =
      (PFN_vkBindBufferMemory2KHR)vkGetDeviceProcAddr(device, "vkBindBufferMemory2KHR");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

VkResult shim_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                 const VkBindImageMemoryInfo *pBindInfos)
{
  static PFN_vkBindImageMemory2 fn =
      (PFN_vkBindImageMemory2)vkGetDeviceProcAddr(device, "vkBindImageMemory2");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

VkResult shim_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                    const VkBindImageMemoryInfo *pBindInfos)
{
  static PFN_vkBindImageMemory2KHR fn =
      (PFN_vkBindImageMemory2KHR)vkGetDeviceProcAddr(device, "vkBindImageMemory2KHR");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

void shim_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
  static PFN_vkCmdSetDeviceMask fn =
      (PFN_vkCmdSetDeviceMask)vkGetDeviceProcAddr(aux.device, "vkCmdSetDeviceMask");
  fn(commandBuffer, deviceMask);
  return;
}

void shim_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
  static PFN_vkCmdSetDeviceMaskKHR fn =
      (PFN_vkCmdSetDeviceMaskKHR)vkGetDeviceProcAddr(aux.device, "vkCmdSetDeviceMaskKHR");
  fn(commandBuffer, deviceMask);
  return;
}

VkResult shim_vkGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities)
{
  static PFN_vkGetDeviceGroupPresentCapabilitiesKHR fn =
      (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupPresentCapabilitiesKHR");
  VkResult r = fn(device, pDeviceGroupPresentCapabilities);
  return r;
}

VkResult shim_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                     VkDeviceGroupPresentModeFlagsKHR *pModes)
{
  static PFN_vkGetDeviceGroupSurfacePresentModesKHR fn =
      (PFN_vkGetDeviceGroupSurfacePresentModesKHR)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupSurfacePresentModesKHR");
  VkResult r = fn(device, surface, pModes);
  return r;
}

VkResult shim_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                     uint32_t *pImageIndex)
{
  static PFN_vkAcquireNextImage2KHR fn =
      (PFN_vkAcquireNextImage2KHR)vkGetDeviceProcAddr(device, "vkAcquireNextImage2KHR");
  VkResult r = fn(device, pAcquireInfo, pImageIndex);
  return r;
}

void shim_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                            uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                            uint32_t groupCountZ)
{
  static PFN_vkCmdDispatchBase fn =
      (PFN_vkCmdDispatchBase)vkGetDeviceProcAddr(aux.device, "vkCmdDispatchBase");
  fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
  return;
}

void shim_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                               uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX,
                               uint32_t groupCountY, uint32_t groupCountZ)
{
  static PFN_vkCmdDispatchBaseKHR fn =
      (PFN_vkCmdDispatchBaseKHR)vkGetDeviceProcAddr(aux.device, "vkCmdDispatchBaseKHR");
  fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
  return;
}

VkResult shim_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice,
                                                      VkSurfaceKHR surface, uint32_t *pRectCount,
                                                      VkRect2D *pRects)
{
  static PFN_vkGetPhysicalDevicePresentRectanglesKHR fn =
      (PFN_vkGetPhysicalDevicePresentRectanglesKHR)vkGetDeviceProcAddr(
          aux.device, "vkGetPhysicalDevicePresentRectanglesKHR");
  VkResult r = fn(physicalDevice, surface, pRectCount, pRects);
  return r;
}

VkResult shim_vkCreateDescriptorUpdateTemplate(VkDevice device,
                                               const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate)
{
  static PFN_vkCreateDescriptorUpdateTemplate fn =
      (PFN_vkCreateDescriptorUpdateTemplate)vkGetDeviceProcAddr(device,
                                                                "vkCreateDescriptorUpdateTemplate");
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
  return r;
}

VkResult shim_vkCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate)
{
  static PFN_vkCreateDescriptorUpdateTemplateKHR fn =
      (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(
          device, "vkCreateDescriptorUpdateTemplateKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
  return r;
}

void shim_vkDestroyDescriptorUpdateTemplate(VkDevice device,
                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                            const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDescriptorUpdateTemplate fn =
      (PFN_vkDestroyDescriptorUpdateTemplate)vkGetDeviceProcAddr(
          device, "vkDestroyDescriptorUpdateTemplate");
  fn(device, descriptorUpdateTemplate, pAllocator);
  return;
}

void shim_vkDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                               const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDescriptorUpdateTemplateKHR fn =
      (PFN_vkDestroyDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(
          device, "vkDestroyDescriptorUpdateTemplateKHR");
  fn(device, descriptorUpdateTemplate, pAllocator);
  return;
}

void shim_vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                            const void *pData)
{
  static PFN_vkUpdateDescriptorSetWithTemplate fn =
      (PFN_vkUpdateDescriptorSetWithTemplate)vkGetDeviceProcAddr(
          device, "vkUpdateDescriptorSetWithTemplate");
  fn(device, descriptorSet, descriptorUpdateTemplate, pData);
  return;
}

void shim_vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                               const void *pData)
{
  static PFN_vkUpdateDescriptorSetWithTemplateKHR fn =
      (PFN_vkUpdateDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(
          device, "vkUpdateDescriptorSetWithTemplateKHR");
  fn(device, descriptorSet, descriptorUpdateTemplate, pData);
  return;
}

void shim_vkCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                VkPipelineLayout layout, uint32_t set,
                                                const void *pData)
{
  static PFN_vkCmdPushDescriptorSetWithTemplateKHR fn =
      (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(
          aux.device, "vkCmdPushDescriptorSetWithTemplateKHR");
  fn(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
  return;
}

void shim_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount,
                              const VkSwapchainKHR *pSwapchains, const VkHdrMetadataEXT *pMetadata)
{
  static PFN_vkSetHdrMetadataEXT fn =
      (PFN_vkSetHdrMetadataEXT)vkGetDeviceProcAddr(device, "vkSetHdrMetadataEXT");
  fn(device, swapchainCount, pSwapchains, pMetadata);
  return;
}

VkResult shim_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain)
{
  static PFN_vkGetSwapchainStatusKHR fn =
      (PFN_vkGetSwapchainStatusKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainStatusKHR");
  VkResult r = fn(device, swapchain);
  return r;
}

VkResult shim_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                              VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties)
{
  static PFN_vkGetRefreshCycleDurationGOOGLE fn =
      (PFN_vkGetRefreshCycleDurationGOOGLE)vkGetDeviceProcAddr(device,
                                                               "vkGetRefreshCycleDurationGOOGLE");
  VkResult r = fn(device, swapchain, pDisplayTimingProperties);
  return r;
}

VkResult shim_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                uint32_t *pPresentationTimingCount,
                                                VkPastPresentationTimingGOOGLE *pPresentationTimings)
{
  static PFN_vkGetPastPresentationTimingGOOGLE fn =
      (PFN_vkGetPastPresentationTimingGOOGLE)vkGetDeviceProcAddr(
          device, "vkGetPastPresentationTimingGOOGLE");
  VkResult r = fn(device, swapchain, pPresentationTimingCount, pPresentationTimings);
  return r;
}

void shim_vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                     uint32_t viewportCount,
                                     const VkViewportWScalingNV *pViewportWScalings)
{
  static PFN_vkCmdSetViewportWScalingNV fn =
      (PFN_vkCmdSetViewportWScalingNV)vkGetDeviceProcAddr(aux.device, "vkCmdSetViewportWScalingNV");
  fn(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
  return;
}

void shim_vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                      uint32_t discardRectangleCount,
                                      const VkRect2D *pDiscardRectangles)
{
  static PFN_vkCmdSetDiscardRectangleEXT fn = (PFN_vkCmdSetDiscardRectangleEXT)vkGetDeviceProcAddr(
      aux.device, "vkCmdSetDiscardRectangleEXT");
  fn(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
  return;
}

void shim_vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                     const VkSampleLocationsInfoEXT *pSampleLocationsInfo)
{
  static PFN_vkCmdSetSampleLocationsEXT fn =
      (PFN_vkCmdSetSampleLocationsEXT)vkGetDeviceProcAddr(aux.device, "vkCmdSetSampleLocationsEXT");
  fn(commandBuffer, pSampleLocationsInfo);
  return;
}

void shim_vkGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice,
                                                      VkSampleCountFlagBits samples,
                                                      VkMultisamplePropertiesEXT *pMultisampleProperties)
{
  static PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT fn =
      (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)vkGetDeviceProcAddr(
          aux.device, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
  fn(physicalDevice, samples, pMultisampleProperties);
  return;
}

VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities)
{
  static PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
  VkResult r = fn(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                    const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                    uint32_t *pSurfaceFormatCount,
                                                    VkSurfaceFormat2KHR *pSurfaceFormats)
{
  static PFN_vkGetPhysicalDeviceSurfaceFormats2KHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
  VkResult r = fn(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
  return r;
}

void shim_vkGetBufferMemoryRequirements2(VkDevice device,
                                         const VkBufferMemoryRequirementsInfo2 *pInfo,
                                         VkMemoryRequirements2 *pMemoryRequirements)
{
  static PFN_vkGetBufferMemoryRequirements2 fn =
      (PFN_vkGetBufferMemoryRequirements2)vkGetDeviceProcAddr(device,
                                                              "vkGetBufferMemoryRequirements2");
  fn(device, pInfo, pMemoryRequirements);
  return;
}
)") + std::string(R"(
void shim_vkGetBufferMemoryRequirements2KHR(VkDevice device,
                                            const VkBufferMemoryRequirementsInfo2 *pInfo,
                                            VkMemoryRequirements2 *pMemoryRequirements)
{
  static PFN_vkGetBufferMemoryRequirements2KHR fn =
      (PFN_vkGetBufferMemoryRequirements2KHR)vkGetDeviceProcAddr(
          device, "vkGetBufferMemoryRequirements2KHR");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                        VkMemoryRequirements2 *pMemoryRequirements)
{
  static PFN_vkGetImageMemoryRequirements2 fn =
      (PFN_vkGetImageMemoryRequirements2)vkGetDeviceProcAddr(device,
                                                             "vkGetImageMemoryRequirements2");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetImageMemoryRequirements2KHR(VkDevice device,
                                           const VkImageMemoryRequirementsInfo2 *pInfo,
                                           VkMemoryRequirements2 *pMemoryRequirements)
{
  static PFN_vkGetImageMemoryRequirements2KHR fn =
      (PFN_vkGetImageMemoryRequirements2KHR)vkGetDeviceProcAddr(device,
                                                                "vkGetImageMemoryRequirements2KHR");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetImageSparseMemoryRequirements2(VkDevice device,
                                              const VkImageSparseMemoryRequirementsInfo2 *pInfo,
                                              uint32_t *pSparseMemoryRequirementCount,
                                              VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
  static PFN_vkGetImageSparseMemoryRequirements2 fn =
      (PFN_vkGetImageSparseMemoryRequirements2)vkGetDeviceProcAddr(
          device, "vkGetImageSparseMemoryRequirements2");
  fn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
  return;
}

void shim_vkGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
  static PFN_vkGetImageSparseMemoryRequirements2KHR fn =
      (PFN_vkGetImageSparseMemoryRequirements2KHR)vkGetDeviceProcAddr(
          device, "vkGetImageSparseMemoryRequirements2KHR");
  fn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
  return;
}

VkResult shim_vkCreateSamplerYcbcrConversion(VkDevice device,
                                             const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkSamplerYcbcrConversion *pYcbcrConversion)
{
  static PFN_vkCreateSamplerYcbcrConversion fn =
      (PFN_vkCreateSamplerYcbcrConversion)vkGetDeviceProcAddr(device,
                                                              "vkCreateSamplerYcbcrConversion");
  VkResult r = fn(device, pCreateInfo, pAllocator, pYcbcrConversion);
  return r;
}

VkResult shim_vkCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator,
                                                VkSamplerYcbcrConversion *pYcbcrConversion)
{
  static PFN_vkCreateSamplerYcbcrConversionKHR fn =
      (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(
          device, "vkCreateSamplerYcbcrConversionKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pYcbcrConversion);
  return r;
}

void shim_vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                          const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySamplerYcbcrConversion fn =
      (PFN_vkDestroySamplerYcbcrConversion)vkGetDeviceProcAddr(device,
                                                               "vkDestroySamplerYcbcrConversion");
  fn(device, ycbcrConversion, pAllocator);
  return;
}

void shim_vkDestroySamplerYcbcrConversionKHR(VkDevice device,
                                             VkSamplerYcbcrConversion ycbcrConversion,
                                             const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroySamplerYcbcrConversionKHR fn =
      (PFN_vkDestroySamplerYcbcrConversionKHR)vkGetDeviceProcAddr(
          device, "vkDestroySamplerYcbcrConversionKHR");
  fn(device, ycbcrConversion, pAllocator);
  return;
}

void shim_vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue)
{
  static PFN_vkGetDeviceQueue2 fn = vkGetDeviceQueue2;
  fn(device, pQueueInfo, pQueue);
  return;
}

VkResult shim_vkCreateValidationCacheEXT(VkDevice device,
                                         const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator,
                                         VkValidationCacheEXT *pValidationCache)
{
  static PFN_vkCreateValidationCacheEXT fn =
      (PFN_vkCreateValidationCacheEXT)vkGetDeviceProcAddr(device, "vkCreateValidationCacheEXT");
  VkResult r = fn(device, pCreateInfo, pAllocator, pValidationCache);
  return r;
}

void shim_vkDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                      const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyValidationCacheEXT fn =
      (PFN_vkDestroyValidationCacheEXT)vkGetDeviceProcAddr(device, "vkDestroyValidationCacheEXT");
  fn(device, validationCache, pAllocator);
  return;
}

VkResult shim_vkGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                          size_t *pDataSize, void *pData)
{
  static PFN_vkGetValidationCacheDataEXT fn =
      (PFN_vkGetValidationCacheDataEXT)vkGetDeviceProcAddr(device, "vkGetValidationCacheDataEXT");
  VkResult r = fn(device, validationCache, pDataSize, pData);
  return r;
}

VkResult shim_vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache,
                                         uint32_t srcCacheCount,
                                         const VkValidationCacheEXT *pSrcCaches)
{
  static PFN_vkMergeValidationCachesEXT fn =
      (PFN_vkMergeValidationCachesEXT)vkGetDeviceProcAddr(device, "vkMergeValidationCachesEXT");
  VkResult r = fn(device, dstCache, srcCacheCount, pSrcCaches);
  return r;
}

void shim_vkGetDescriptorSetLayoutSupport(VkDevice device,
                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                          VkDescriptorSetLayoutSupport *pSupport)
{
  static PFN_vkGetDescriptorSetLayoutSupport fn =
      (PFN_vkGetDescriptorSetLayoutSupport)vkGetDeviceProcAddr(device,
                                                               "vkGetDescriptorSetLayoutSupport");
  fn(device, pCreateInfo, pSupport);
  return;
}

void shim_vkGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                             const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                             VkDescriptorSetLayoutSupport *pSupport)
{
  static PFN_vkGetDescriptorSetLayoutSupportKHR fn =
      (PFN_vkGetDescriptorSetLayoutSupportKHR)vkGetDeviceProcAddr(
          device, "vkGetDescriptorSetLayoutSupportKHR");
  fn(device, pCreateInfo, pSupport);
  return;
}

VkResult shim_vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline,
                                 VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType,
                                 size_t *pInfoSize, void *pInfo)
{
  static PFN_vkGetShaderInfoAMD fn =
      (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(device, "vkGetShaderInfoAMD");
  VkResult r = fn(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
  return r;
}

VkResult shim_vkSetDebugUtilsObjectNameEXT(VkDevice device,
                                           const VkDebugUtilsObjectNameInfoEXT *pNameInfo)
{
  static PFN_vkSetDebugUtilsObjectNameEXT fn =
      (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(aux.instance,
                                                              "vkSetDebugUtilsObjectNameEXT");
  VkResult r = fn(device, pNameInfo);
  return r;
}

VkResult shim_vkSetDebugUtilsObjectTagEXT(VkDevice device,
                                          const VkDebugUtilsObjectTagInfoEXT *pTagInfo)
{
  static PFN_vkSetDebugUtilsObjectTagEXT fn = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(
      aux.instance, "vkSetDebugUtilsObjectTagEXT");
  VkResult r = fn(device, pTagInfo);
  return r;
}

void shim_vkQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo)
{
  static PFN_vkQueueBeginDebugUtilsLabelEXT fn =
      (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(aux.instance,
                                                                "vkQueueBeginDebugUtilsLabelEXT");
  fn(queue, pLabelInfo);
  return;
}

void shim_vkQueueEndDebugUtilsLabelEXT(VkQueue queue)
{
  static PFN_vkQueueEndDebugUtilsLabelEXT fn =
      (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(aux.instance,
                                                              "vkQueueEndDebugUtilsLabelEXT");
  fn(queue);
  return;
}

void shim_vkQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo)
{
  static PFN_vkQueueInsertDebugUtilsLabelEXT fn =
      (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(aux.instance,
                                                                 "vkQueueInsertDebugUtilsLabelEXT");
  fn(queue, pLabelInfo);
  return;
}

void shim_vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                       const VkDebugUtilsLabelEXT *pLabelInfo)
{
  static PFN_vkCmdBeginDebugUtilsLabelEXT fn =
      (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(aux.instance,
                                                              "vkCmdBeginDebugUtilsLabelEXT");
  fn(commandBuffer, pLabelInfo);
  return;
}

void shim_vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer)
{
  static PFN_vkCmdEndDebugUtilsLabelEXT fn = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkCmdEndDebugUtilsLabelEXT");
  fn(commandBuffer);
  return;
}

void shim_vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                        const VkDebugUtilsLabelEXT *pLabelInfo)
{
  static PFN_vkCmdInsertDebugUtilsLabelEXT fn =
      (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(aux.instance,
                                                               "vkCmdInsertDebugUtilsLabelEXT");
  fn(commandBuffer, pLabelInfo);
  return;
}

VkResult shim_vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkDebugUtilsMessengerEXT *pMessenger)
{
  static PFN_vkCreateDebugUtilsMessengerEXT fn =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                "vkCreateDebugUtilsMessengerEXT");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pMessenger);
  return r;
}

void shim_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                          const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyDebugUtilsMessengerEXT fn =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                 "vkDestroyDebugUtilsMessengerEXT");
  fn(instance, messenger, pAllocator);
  return;
}

void shim_vkSubmitDebugUtilsMessageEXT(VkInstance instance,
                                       VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData)
{
  static PFN_vkSubmitDebugUtilsMessageEXT fn =
      (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(instance,
                                                              "vkSubmitDebugUtilsMessageEXT");
  fn(instance, messageSeverity, messageTypes, pCallbackData);
  return;
}

VkResult shim_vkGetMemoryHostPointerPropertiesEXT(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void *pHostPointer,
    VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties)
{
  static PFN_vkGetMemoryHostPointerPropertiesEXT fn =
      (PFN_vkGetMemoryHostPointerPropertiesEXT)vkGetDeviceProcAddr(
          device, "vkGetMemoryHostPointerPropertiesEXT");
  VkResult r = fn(device, handleType, pHostPointer, pMemoryHostPointerProperties);
  return r;
}

void shim_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer,
                                    VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer,
                                    VkDeviceSize dstOffset, uint32_t marker)
{
  static PFN_vkCmdWriteBufferMarkerAMD fn =
      (PFN_vkCmdWriteBufferMarkerAMD)vkGetDeviceProcAddr(aux.device, "vkCmdWriteBufferMarkerAMD");
  fn(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
  return;
}
)")},

    /******************************************************************************/
    /* TEMPLATE_FILE_SHADER_INFO_UTILS_H                                          */
    /******************************************************************************/
    {"amd_shader_info_shim", "utils.h",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: amd_shader_info_shim/utils.h
//-----------------------------------------------------------------------------
#pragma once

#include "helper/helper.h"

extern AuxVkTraceResources aux;

std::string getStageStr(VkShaderStageFlagBits stage);
void printShaderInfo(VkPipeline p, VkShaderStageFlagBits stage, const char *disassembly, size_t size);
void printShaderInfo(VkPipeline p, VkShaderStageFlagBits stage,
                     VkShaderStatisticsInfoAMD &statistics);
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_SHADER_INFO_UTILS_CPP                                        */
    /******************************************************************************/
    {"amd_shader_info_shim", "utils.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: amd_shader_info_shim/utils.cpp
//-----------------------------------------------------------------------------
#include "utils.h"

#include <fstream>
#include <sstream>

std::string getStageStr(VkShaderStageFlagBits stage)
{
  switch(stage)
  {
    case(VK_SHADER_STAGE_VERTEX_BIT): return "VERTEX";
    case(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT): return "TESSELLATION_CONTROL";
    case(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT): return "TESSELLATION_EVALUATION";
    case(VK_SHADER_STAGE_GEOMETRY_BIT): return "GEOMETRY";
    case(VK_SHADER_STAGE_FRAGMENT_BIT): return "FRAGMENT";
    case(VK_SHADER_STAGE_COMPUTE_BIT): return "COMPUTE";
    case(VK_SHADER_STAGE_ALL_GRAPHICS): return "ALL_GRAPHICS";
    case(VK_SHADER_STAGE_ALL): return "ALL";
    default: return "UNKNOWN";
  }
}

void printShaderInfo(VkPipeline p, VkShaderStageFlagBits stage, const char *disassembly, size_t size)
{
  std::stringstream ss;
  ss << "0x" << std::hex << reinterpret_cast<uint64_t>(p) << "_VK_SHADER_STAGE_"
     << getStageStr(stage) << "_disassembly.txt";
  std::string filename;
  ss >> filename;
  std::ofstream file(filename);
  file.write(disassembly, size);
  file.close();
}

void printShaderInfo(VkPipeline p, VkShaderStageFlagBits stage, VkShaderStatisticsInfoAMD &statistics)
{
  std::stringstream ss;
  ss << "0x" << std::hex << reinterpret_cast<uint64_t>(p) << "_VK_SHADER_STAGE_"
     << getStageStr(stage) << "_statistics.txt";
  std::string filename;
  ss >> filename;
  std::ofstream file(filename);
  file << "vkGetShaderInfoAMD" << std::endl
       << "{" << std::endl
       << "  pipeline: "
       << "0x" << std::hex << reinterpret_cast<uint64_t>(p) << std::endl
       << "  shaderStage: "
       << "VK_SHADER_STAGE_" << getStageStr(stage) << std::endl
       << "  infoType: VK_SHADER_INFO_TYPE_STATISTICS_AMD" << std::endl
       << "  pInfo: " << std::endl
       << "  {" << std::endl
       << "    shaderStageMask: " << statistics.shaderStageMask << std::endl
       << "    resourceUsage: " << std::endl
       << std::dec << "    {" << std::endl
       << "      numUsedVgprs: " << statistics.resourceUsage.numUsedVgprs << std::endl
       << "      numUsedSgprs: " << statistics.resourceUsage.numUsedSgprs << std::endl
       << "      ldsSizePerLocalWorkGroup: " << statistics.resourceUsage.ldsSizePerLocalWorkGroup
       << std::endl
       << "      ldsUsageSizeInBytes: " << statistics.resourceUsage.ldsUsageSizeInBytes << std::endl
       << "      scratchMemUsageInBytes: " << statistics.resourceUsage.scratchMemUsageInBytes
       << std::endl
       << "    }" << std::endl
       << "    numPhysicalVgprs: " << statistics.numPhysicalVgprs << std::endl
       << "    numPhysicalSgprs: " << statistics.numPhysicalSgprs << std::endl
       << "    numAvailableVgprs: " << statistics.numAvailableVgprs << std::endl
       << "    numAvailableSgprs: " << statistics.numAvailableSgprs << std::endl
       << "    computeWorkGroupSize: "
       << "[" << statistics.computeWorkGroupSize[0] << "," << statistics.computeWorkGroupSize[1]
       << "," << statistics.computeWorkGroupSize[2] << "]" << std::endl
       << "  }" << std::endl
       << "}" << std::endl;
  file.close();
})"},

    /******************************************************************************/
    /* TEMPLATE_FILE_SHADER_INFO_SHIM_CMAKE                                       */
    /******************************************************************************/
    {"amd_shader_info_shim", "CMakeLists.txt",
     R"(SET (THIS_PROJECT_NAME amd_shader_info_shim)
PROJECT(${THIS_PROJECT_NAME})

ADD_LIBRARY(${THIS_PROJECT_NAME} SHARED "shim_vulkan.h" "shim_vulkan.cpp" "utils.h" "utils.cpp")

TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                           UNICODE _UNICODE)
IF (NOT WIN32)
  SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                        CXX_VISIBILITY_PRESET hidden)
ENDIF ()

TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                      vulkan
                      helper)

SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                      OUTPUT_NAME shim_vulkan
                      ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}")

ADD_CUSTOM_COMMAND(TARGET ${THIS_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${THIS_PROJECT_NAME}> ${WORKING_DIRECTORY_DEBUG}sample_cpp_trace)
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_GOLD_REFERENCE_SHIM_H                                           */
    /******************************************************************************/
    {"gold_reference_shim", "shim_vulkan.h",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: gold_reference_shim/shim_vulkan.h
//-----------------------------------------------------------------------------
#pragma once
#if defined(_WIN32)
#define SHIM_VK_API_IMPORT __declspec(dllimport)
#define SHIM_VK_API_EXPORT __declspec(dllexport)
#else
#define SHIM_VK_API_IMPORT __attribute__((visibility("default")))
#define SHIM_VK_API_EXPORT __attribute__((visibility("default")))
#endif
#if defined(SHIM_VK_COMPILE_STATIC_LIB)
#define SHIM_VK_API
#else
#if defined(SHIM_VK_EXPORT)
#define SHIM_VK_API SHIM_VK_API_EXPORT
#else
#define SHIM_VK_API SHIM_VK_API_IMPORT
#endif
#endif
#include "vulkan/vulkan.h"
SHIM_VK_API VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator,
                                           VkInstance *pInstance);

SHIM_VK_API VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice,
                                         const VkDeviceCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);

SHIM_VK_API void shim_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkEnumeratePhysicalDevices(VkInstance instance,
                                                     uint32_t *pPhysicalDeviceCount,
                                                     VkPhysicalDevice *pPhysicalDevices);

SHIM_VK_API void shim_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties *pQueueFamilyProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties *pMemoryProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceFeatures *pFeatures);

SHIM_VK_API void shim_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice,
                                                          VkFormat format,
                                                          VkFormatProperties *pFormatProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkImageFormatProperties *pImageFormatProperties);

SHIM_VK_API void shim_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkEnumerateInstanceVersion(uint32_t *pApiVersion);

SHIM_VK_API VkResult shim_vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                                             VkLayerProperties *pProperties);

SHIM_VK_API VkResult shim_vkEnumerateInstanceExtensionProperties(const char *pLayerName,
                                                                 uint32_t *pPropertyCount,
                                                                 VkExtensionProperties *pProperties);

SHIM_VK_API VkResult shim_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                                                           uint32_t *pPropertyCount,
                                                           VkLayerProperties *pProperties);

SHIM_VK_API VkResult shim_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                               const char *pLayerName,
                                                               uint32_t *pPropertyCount,
                                                               VkExtensionProperties *pProperties);

SHIM_VK_API void shim_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex,
                                       uint32_t queueIndex, VkQueue *pQueue);

SHIM_VK_API VkResult shim_vkQueueSubmit(VkQueue queue, uint32_t submitCount,
                                        const VkSubmitInfo *pSubmits, VkFence fence);

SHIM_VK_API VkResult shim_vkQueueWaitIdle(VkQueue queue);

SHIM_VK_API VkResult shim_vkDeviceWaitIdle(VkDevice device);

SHIM_VK_API VkResult shim_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                           const VkAllocationCallbacks *pAllocator,
                                           VkDeviceMemory *pMemory);

SHIM_VK_API void shim_vkFreeMemory(VkDevice device, VkDeviceMemory memory,
                                   const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset,
                                      VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);

SHIM_VK_API void shim_vkUnmapMemory(VkDevice device, VkDeviceMemory memory);

SHIM_VK_API VkResult shim_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                    const VkMappedMemoryRange *pMemoryRanges);

SHIM_VK_API VkResult shim_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                         const VkMappedMemoryRange *pMemoryRanges);

SHIM_VK_API void shim_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                  VkDeviceSize *pCommittedMemoryInBytes);

SHIM_VK_API void shim_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                    VkMemoryRequirements *pMemoryRequirements);

SHIM_VK_API VkResult shim_vkBindBufferMemory(VkDevice device, VkBuffer buffer,
                                             VkDeviceMemory memory, VkDeviceSize memoryOffset);

SHIM_VK_API void shim_vkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                   VkMemoryRequirements *pMemoryRequirements);

SHIM_VK_API VkResult shim_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                            VkDeviceSize memoryOffset);

SHIM_VK_API void shim_vkGetImageSparseMemoryRequirements(
    VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements *pSparseMemoryRequirements);

SHIM_VK_API void shim_vkGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
    VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties);

SHIM_VK_API VkResult shim_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount,
                                            const VkBindSparseInfo *pBindInfo, VkFence fence);

SHIM_VK_API VkResult shim_vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkFence *pFence);

SHIM_VK_API void shim_vkDestroyFence(VkDevice device, VkFence fence,
                                     const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences);

SHIM_VK_API VkResult shim_vkGetFenceStatus(VkDevice device, VkFence fence);

SHIM_VK_API VkResult shim_vkWaitForFences(VkDevice device, uint32_t fenceCount,
                                          const VkFence *pFences, VkBool32 waitAll, uint64_t timeout);

SHIM_VK_API VkResult shim_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkSemaphore *pSemaphore);

SHIM_VK_API void shim_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkEvent *pEvent);

SHIM_VK_API void shim_vkDestroyEvent(VkDevice device, VkEvent event,
                                     const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetEventStatus(VkDevice device, VkEvent event);

SHIM_VK_API VkResult shim_vkSetEvent(VkDevice device, VkEvent event);

SHIM_VK_API VkResult shim_vkResetEvent(VkDevice device, VkEvent event);

SHIM_VK_API VkResult shim_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkQueryPool *pQueryPool);

SHIM_VK_API void shim_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool,
                                                uint32_t firstQuery, uint32_t queryCount,
                                                size_t dataSize, void *pData, VkDeviceSize stride,
                                                VkQueryResultFlags flags);

SHIM_VK_API VkResult shim_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer);

SHIM_VK_API void shim_vkDestroyBuffer(VkDevice device, VkBuffer buffer,
                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateBufferView(VkDevice device,
                                             const VkBufferViewCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkBufferView *pView);

SHIM_VK_API void shim_vkDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                          const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkImage *pImage);

SHIM_VK_API void shim_vkDestroyImage(VkDevice device, VkImage image,
                                     const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkGetImageSubresourceLayout(VkDevice device, VkImage image,
                                                  const VkImageSubresource *pSubresource,
                                                  VkSubresourceLayout *pLayout);

SHIM_VK_API VkResult shim_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkImageView *pView);

SHIM_VK_API void shim_vkDestroyImageView(VkDevice device, VkImageView imageView,
                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateShaderModule(VkDevice device,
                                               const VkShaderModuleCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkShaderModule *pShaderModule);

SHIM_VK_API void shim_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                            const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreatePipelineCache(VkDevice device,
                                                const VkPipelineCacheCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator,
                                                VkPipelineCache *pPipelineCache);

SHIM_VK_API void shim_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                             const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache,
                                                 size_t *pDataSize, void *pData);

SHIM_VK_API VkResult shim_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache,
                                                uint32_t srcCacheCount,
                                                const VkPipelineCache *pSrcCaches);

SHIM_VK_API VkResult shim_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                    uint32_t createInfoCount,
                                                    const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkPipeline *pPipelines);

SHIM_VK_API VkResult shim_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                   uint32_t createInfoCount,
                                                   const VkComputePipelineCreateInfo *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator,
                                                   VkPipeline *pPipelines);

SHIM_VK_API void shim_vkDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                        const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreatePipelineLayout(VkDevice device,
                                                 const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkPipelineLayout *pPipelineLayout);

SHIM_VK_API void shim_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                              const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkSampler *pSampler);

SHIM_VK_API void shim_vkDestroySampler(VkDevice device, VkSampler sampler,
                                       const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateDescriptorSetLayout(
    VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout);

SHIM_VK_API void shim_vkDestroyDescriptorSetLayout(VkDevice device,
                                                   VkDescriptorSetLayout descriptorSetLayout,
                                                   const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateDescriptorPool(VkDevice device,
                                                 const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDescriptorPool *pDescriptorPool);

SHIM_VK_API void shim_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                              const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                VkDescriptorPoolResetFlags flags);

SHIM_VK_API VkResult shim_vkAllocateDescriptorSets(VkDevice device,
                                                   const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                   VkDescriptorSet *pDescriptorSets);
)" + std::string(R"(
SHIM_VK_API VkResult shim_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                               uint32_t descriptorSetCount,
                                               const VkDescriptorSet *pDescriptorSets);

SHIM_VK_API void shim_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet *pDescriptorWrites,
                                             uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet *pDescriptorCopies);

SHIM_VK_API VkResult shim_vkCreateFramebuffer(VkDevice device,
                                              const VkFramebufferCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator,
                                              VkFramebuffer *pFramebuffer);

SHIM_VK_API void shim_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                           const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateRenderPass(VkDevice device,
                                             const VkRenderPassCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkRenderPass *pRenderPass);

SHIM_VK_API void shim_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                          const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass,
                                                 VkExtent2D *pGranularity);

SHIM_VK_API VkResult shim_vkCreateCommandPool(VkDevice device,
                                              const VkCommandPoolCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator,
                                              VkCommandPool *pCommandPool);

SHIM_VK_API void shim_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                           const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                             VkCommandPoolResetFlags flags);

SHIM_VK_API VkResult shim_vkAllocateCommandBuffers(VkDevice device,
                                                   const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                   VkCommandBuffer *pCommandBuffers);

SHIM_VK_API void shim_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                           uint32_t commandBufferCount,
                                           const VkCommandBuffer *pCommandBuffers);

SHIM_VK_API VkResult shim_vkBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                               const VkCommandBufferBeginInfo *pBeginInfo);

SHIM_VK_API VkResult shim_vkEndCommandBuffer(VkCommandBuffer commandBuffer);

SHIM_VK_API VkResult shim_vkResetCommandBuffer(VkCommandBuffer commandBuffer,
                                               VkCommandBufferResetFlags flags);

SHIM_VK_API void shim_vkCmdBindPipeline(VkCommandBuffer commandBuffer,
                                        VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);

SHIM_VK_API void shim_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                       uint32_t viewportCount, const VkViewport *pViewports);

SHIM_VK_API void shim_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                      uint32_t scissorCount, const VkRect2D *pScissors);

SHIM_VK_API void shim_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);

SHIM_VK_API void shim_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                                        float depthBiasClamp, float depthBiasSlopeFactor);

SHIM_VK_API void shim_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer,
                                             const float blendConstants[4]);

SHIM_VK_API void shim_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                                          float maxDepthBounds);

SHIM_VK_API void shim_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer,
                                                 VkStencilFaceFlags faceMask, uint32_t compareMask);

SHIM_VK_API void shim_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer,
                                               VkStencilFaceFlags faceMask, uint32_t writeMask);

SHIM_VK_API void shim_vkCmdSetStencilReference(VkCommandBuffer commandBuffer,
                                               VkStencilFaceFlags faceMask, uint32_t reference);

SHIM_VK_API void shim_vkCmdBindDescriptorSets(
    VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
    uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets,
    uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets);

SHIM_VK_API void shim_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                           VkDeviceSize offset, VkIndexType indexType);

SHIM_VK_API void shim_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                             uint32_t bindingCount, const VkBuffer *pBuffers,
                                             const VkDeviceSize *pOffsets);

SHIM_VK_API void shim_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount,
                                uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

SHIM_VK_API void shim_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                       uint32_t instanceCount, uint32_t firstIndex,
                                       int32_t vertexOffset, uint32_t firstInstance);

SHIM_VK_API void shim_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                        VkDeviceSize offset, uint32_t drawCount, uint32_t stride);

SHIM_VK_API void shim_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                               VkDeviceSize offset, uint32_t drawCount,
                                               uint32_t stride);

SHIM_VK_API void shim_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                    uint32_t groupCountY, uint32_t groupCountZ);

SHIM_VK_API void shim_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                            VkDeviceSize offset);

SHIM_VK_API void shim_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                      VkBuffer dstBuffer, uint32_t regionCount,
                                      const VkBufferCopy *pRegions);

SHIM_VK_API void shim_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                     VkImageLayout srcImageLayout, VkImage dstImage,
                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy *pRegions);

SHIM_VK_API void shim_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                     VkImageLayout srcImageLayout, VkImage dstImage,
                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit *pRegions, VkFilter filter);

SHIM_VK_API void shim_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                             VkImage dstImage, VkImageLayout dstImageLayout,
                                             uint32_t regionCount, const VkBufferImageCopy *pRegions);

SHIM_VK_API void shim_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                             VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                             uint32_t regionCount, const VkBufferImageCopy *pRegions);

SHIM_VK_API void shim_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                        VkDeviceSize dstOffset, VkDeviceSize dataSize,
                                        const void *pData);

SHIM_VK_API void shim_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                      VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);

SHIM_VK_API void shim_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                           VkImageLayout imageLayout,
                                           const VkClearColorValue *pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange *pRanges);

SHIM_VK_API void shim_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                  VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue *pDepthStencil,
                                                  uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges);

SHIM_VK_API void shim_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment *pAttachments,
                                            uint32_t rectCount, const VkClearRect *pRects);

SHIM_VK_API void shim_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                        VkImageLayout srcImageLayout, VkImage dstImage,
                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve *pRegions);

SHIM_VK_API void shim_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                    VkPipelineStageFlags stageMask);

SHIM_VK_API void shim_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                      VkPipelineStageFlags stageMask);

SHIM_VK_API void shim_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                      const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                                      VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                      const VkMemoryBarrier *pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount,
                                      const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount,
                                      const VkImageMemoryBarrier *pImageMemoryBarriers);

SHIM_VK_API void shim_vkCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);

SHIM_VK_API void shim_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                      uint32_t query, VkQueryControlFlags flags);

SHIM_VK_API void shim_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                    uint32_t query);

SHIM_VK_API void shim_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                          uint32_t firstQuery, uint32_t queryCount);

SHIM_VK_API void shim_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer,
                                          VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t query);

SHIM_VK_API void shim_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                uint32_t firstQuery, uint32_t queryCount,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize stride, VkQueryResultFlags flags);

SHIM_VK_API void shim_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                         VkShaderStageFlags stageFlags, uint32_t offset,
                                         uint32_t size, const void *pValues);

SHIM_VK_API void shim_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                                           const VkRenderPassBeginInfo *pRenderPassBegin,
                                           VkSubpassContents contents);

SHIM_VK_API void shim_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);

SHIM_VK_API void shim_vkCmdEndRenderPass(VkCommandBuffer commandBuffer);

SHIM_VK_API void shim_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer *pCommandBuffers);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPropertiesKHR *pProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
    VkDisplayPlanePropertiesKHR *pProperties);

SHIM_VK_API VkResult shim_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice,
                                                                uint32_t planeIndex,
                                                                uint32_t *pDisplayCount,
                                                                VkDisplayKHR *pDisplays);

SHIM_VK_API VkResult shim_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                        VkDisplayKHR display,
                                                        uint32_t *pPropertyCount,
                                                        VkDisplayModePropertiesKHR *pProperties);

SHIM_VK_API VkResult shim_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice,
                                                 VkDisplayKHR display,
                                                 const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDisplayModeKHR *pMode);

SHIM_VK_API VkResult shim_vkGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
    VkDisplayPlaneCapabilitiesKHR *pCapabilities);

SHIM_VK_API VkResult shim_vkCreateDisplayPlaneSurfaceKHR(
    VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);

SHIM_VK_API VkResult shim_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                      const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator,
                                                      VkSwapchainKHR *pSwapchains);

SHIM_VK_API void shim_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                          const VkAllocationCallbacks *pAllocator);
)") + std::string(R"(
SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                               uint32_t queueFamilyIndex,
                                                               VkSurfaceKHR surface,
                                                               VkBool32 *pSupported);

SHIM_VK_API VkResult
shim_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                               VkSurfaceCapabilitiesKHR *pSurfaceCapabilities);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                                                               VkSurfaceKHR surface,
                                                               uint32_t *pSurfaceFormatCount,
                                                               VkSurfaceFormatKHR *pSurfaceFormats);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                    VkSurfaceKHR surface,
                                                                    uint32_t *pPresentModeCount,
                                                                    VkPresentModeKHR *pPresentModes);

SHIM_VK_API VkResult shim_vkCreateSwapchainKHR(VkDevice device,
                                               const VkSwapchainCreateInfoKHR *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkSwapchainKHR *pSwapchain);

SHIM_VK_API void shim_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                            const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                  uint32_t *pSwapchainImageCount,
                                                  VkImage *pSwapchainImages);

SHIM_VK_API VkResult shim_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                uint64_t timeout, VkSemaphore semaphore,
                                                VkFence fence, uint32_t *pImageIndex);

SHIM_VK_API VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo);

SHIM_VK_API VkResult shim_vkCreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback);

SHIM_VK_API void shim_vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                                      VkDebugReportCallbackEXT callback,
                                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                              VkDebugReportObjectTypeEXT objectType,
                                              uint64_t object, size_t location, int32_t messageCode,
                                              const char *pLayerPrefix, const char *pMessage);

SHIM_VK_API VkResult
shim_vkDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo);

SHIM_VK_API VkResult shim_vkDebugMarkerSetObjectTagEXT(VkDevice device,
                                                       const VkDebugMarkerObjectTagInfoEXT *pTagInfo);

SHIM_VK_API void shim_vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                               const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);

SHIM_VK_API void shim_vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);

SHIM_VK_API void shim_vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
                                                const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties);

SHIM_VK_API void shim_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                VkDeviceSize offset, VkBuffer countBuffer,
                                                VkDeviceSize countBufferOffset,
                                                uint32_t maxDrawCount, uint32_t stride);

SHIM_VK_API void shim_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                       VkDeviceSize offset, VkBuffer countBuffer,
                                                       VkDeviceSize countBufferOffset,
                                                       uint32_t maxDrawCount, uint32_t stride);

SHIM_VK_API void shim_vkCmdProcessCommandsNVX(VkCommandBuffer commandBuffer,
                                              const VkCmdProcessCommandsInfoNVX *pProcessCommandsInfo);

SHIM_VK_API void shim_vkCmdReserveSpaceForCommandsNVX(
    VkCommandBuffer commandBuffer, const VkCmdReserveSpaceForCommandsInfoNVX *pReserveSpaceInfo);

SHIM_VK_API VkResult shim_vkCreateIndirectCommandsLayoutNVX(
    VkDevice device, const VkIndirectCommandsLayoutCreateInfoNVX *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNVX *pIndirectCommandsLayout);

SHIM_VK_API void shim_vkDestroyIndirectCommandsLayoutNVX(
    VkDevice device, VkIndirectCommandsLayoutNVX indirectCommandsLayout,
    const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkCreateObjectTableNVX(VkDevice device,
                                                 const VkObjectTableCreateInfoNVX *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkObjectTableNVX *pObjectTable);

SHIM_VK_API void shim_vkDestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable,
                                              const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkRegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                               uint32_t objectCount,
                                               const VkObjectTableEntryNVX *const *ppObjectTableEntries,
                                               const uint32_t *pObjectIndices);

SHIM_VK_API VkResult shim_vkUnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                                 uint32_t objectCount,
                                                 const VkObjectEntryTypeNVX *pObjectEntryTypes,
                                                 const uint32_t *pObjectIndices);

SHIM_VK_API void shim_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice physicalDevice, VkDeviceGeneratedCommandsFeaturesNVX *pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX *pLimits);

SHIM_VK_API void shim_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceFeatures2 *pFeatures);

SHIM_VK_API void shim_vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                      VkPhysicalDeviceFeatures2 *pFeatures);

SHIM_VK_API void shim_vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                     VkPhysicalDeviceProperties2 *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                        VkPhysicalDeviceProperties2 *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice,
                                                           VkFormat format,
                                                           VkFormatProperties2 *pFormatProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                              VkFormat format,
                                                              VkFormatProperties2 *pFormatProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2 *pQueueFamilyProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2 *pQueueFamilyProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);

SHIM_VK_API void shim_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                                VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t set,
                                                uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet *pDescriptorWrites);

SHIM_VK_API void shim_vkTrimCommandPool(VkDevice device, VkCommandPool commandPool,
                                        VkCommandPoolTrimFlags flags);

SHIM_VK_API void shim_vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool,
                                           VkCommandPoolTrimFlags flags);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties);

SHIM_VK_API VkResult shim_vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo,
                                           int *pFd);

SHIM_VK_API VkResult shim_vkGetMemoryFdPropertiesKHR(VkDevice device,
                                                     VkExternalMemoryHandleTypeFlagBits handleType,
                                                     int fd,
                                                     VkMemoryFdPropertiesKHR *pMemoryFdProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties);

SHIM_VK_API VkResult shim_vkGetSemaphoreFdKHR(VkDevice device,
                                              const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd);

SHIM_VK_API VkResult shim_vkImportSemaphoreFdKHR(
    VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties);

SHIM_VK_API void shim_vkGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties);

SHIM_VK_API VkResult shim_vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo,
                                          int *pFd);

SHIM_VK_API VkResult shim_vkImportFenceFdKHR(VkDevice device,
                                             const VkImportFenceFdInfoKHR *pImportFenceFdInfo);

SHIM_VK_API VkResult shim_vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);

SHIM_VK_API VkResult shim_vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                                   const VkDisplayPowerInfoEXT *pDisplayPowerInfo);

SHIM_VK_API VkResult shim_vkRegisterDeviceEventEXT(VkDevice device,
                                                   const VkDeviceEventInfoEXT *pDeviceEventInfo,
                                                   const VkAllocationCallbacks *pAllocator,
                                                   VkFence *pFence);

SHIM_VK_API VkResult shim_vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                                    const VkDisplayEventInfoEXT *pDisplayEventInfo,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkFence *pFence);

SHIM_VK_API VkResult shim_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                   VkSurfaceCounterFlagBitsEXT counter,
                                                   uint64_t *pCounterValue);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
    VkSurfaceCapabilities2EXT *pSurfaceCapabilities);

SHIM_VK_API VkResult shim_vkEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);

SHIM_VK_API VkResult shim_vkEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);

SHIM_VK_API void shim_vkGetDeviceGroupPeerMemoryFeatures(
    VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
    VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);

SHIM_VK_API void shim_vkGetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
    VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);

SHIM_VK_API VkResult shim_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                              const VkBindBufferMemoryInfo *pBindInfos);

SHIM_VK_API VkResult shim_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindBufferMemoryInfo *pBindInfos);

SHIM_VK_API VkResult shim_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                             const VkBindImageMemoryInfo *pBindInfos);
)") + std::string(R"(
SHIM_VK_API VkResult shim_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                const VkBindImageMemoryInfo *pBindInfos);

SHIM_VK_API void shim_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);

SHIM_VK_API void shim_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);

SHIM_VK_API VkResult shim_vkGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities);

SHIM_VK_API VkResult shim_vkGetDeviceGroupSurfacePresentModesKHR(
    VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR *pModes);

SHIM_VK_API VkResult shim_vkAcquireNextImage2KHR(VkDevice device,
                                                 const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                 uint32_t *pImageIndex);

SHIM_VK_API void shim_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                        uint32_t baseGroupY, uint32_t baseGroupZ,
                                        uint32_t groupCountX, uint32_t groupCountY,
                                        uint32_t groupCountZ);

SHIM_VK_API void shim_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                                           uint32_t baseGroupY, uint32_t baseGroupZ,
                                           uint32_t groupCountX, uint32_t groupCountY,
                                           uint32_t groupCountZ);

SHIM_VK_API VkResult shim_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice,
                                                                  VkSurfaceKHR surface,
                                                                  uint32_t *pRectCount,
                                                                  VkRect2D *pRects);

SHIM_VK_API VkResult shim_vkCreateDescriptorUpdateTemplate(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);

SHIM_VK_API VkResult shim_vkCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);

SHIM_VK_API void shim_vkDestroyDescriptorUpdateTemplate(
    VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkDestroyDescriptorUpdateTemplateKHR(
    VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkUpdateDescriptorSetWithTemplate(
    VkDevice device, VkDescriptorSet descriptorSet,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);

SHIM_VK_API void shim_vkUpdateDescriptorSetWithTemplateKHR(
    VkDevice device, VkDescriptorSet descriptorSet,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);

SHIM_VK_API void shim_vkCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    VkPipelineLayout layout, uint32_t set, const void *pData);

SHIM_VK_API void shim_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount,
                                          const VkSwapchainKHR *pSwapchains,
                                          const VkHdrMetadataEXT *pMetadata);

SHIM_VK_API VkResult shim_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);

SHIM_VK_API VkResult
shim_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                     VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties);

SHIM_VK_API VkResult shim_vkGetPastPresentationTimingGOOGLE(
    VkDevice device, VkSwapchainKHR swapchain, uint32_t *pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE *pPresentationTimings);

SHIM_VK_API void shim_vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer,
                                                 uint32_t firstViewport, uint32_t viewportCount,
                                                 const VkViewportWScalingNV *pViewportWScalings);

SHIM_VK_API void shim_vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer,
                                                  uint32_t firstDiscardRectangle,
                                                  uint32_t discardRectangleCount,
                                                  const VkRect2D *pDiscardRectangles);

SHIM_VK_API void shim_vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                 const VkSampleLocationsInfoEXT *pSampleLocationsInfo);

SHIM_VK_API void shim_vkGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
    VkMultisamplePropertiesEXT *pMultisampleProperties);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities);

SHIM_VK_API VkResult shim_vkGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    uint32_t *pSurfaceFormatCount, VkSurfaceFormat2KHR *pSurfaceFormats);

SHIM_VK_API void shim_vkGetBufferMemoryRequirements2(VkDevice device,
                                                     const VkBufferMemoryRequirementsInfo2 *pInfo,
                                                     VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetBufferMemoryRequirements2KHR(VkDevice device,
                                                        const VkBufferMemoryRequirementsInfo2 *pInfo,
                                                        VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetImageMemoryRequirements2(VkDevice device,
                                                    const VkImageMemoryRequirementsInfo2 *pInfo,
                                                    VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetImageMemoryRequirements2KHR(VkDevice device,
                                                       const VkImageMemoryRequirementsInfo2 *pInfo,
                                                       VkMemoryRequirements2 *pMemoryRequirements);

SHIM_VK_API void shim_vkGetImageSparseMemoryRequirements2(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);

SHIM_VK_API void shim_vkGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);

SHIM_VK_API VkResult shim_vkCreateSamplerYcbcrConversion(
    VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);

SHIM_VK_API VkResult shim_vkCreateSamplerYcbcrConversionKHR(
    VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);

SHIM_VK_API void shim_vkDestroySamplerYcbcrConversion(VkDevice device,
                                                      VkSamplerYcbcrConversion ycbcrConversion,
                                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkDestroySamplerYcbcrConversionKHR(VkDevice device,
                                                         VkSamplerYcbcrConversion ycbcrConversion,
                                                         const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo,
                                        VkQueue *pQueue);

SHIM_VK_API VkResult shim_vkCreateValidationCacheEXT(VkDevice device,
                                                     const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator,
                                                     VkValidationCacheEXT *pValidationCache);

SHIM_VK_API void shim_vkDestroyValidationCacheEXT(VkDevice device,
                                                  VkValidationCacheEXT validationCache,
                                                  const VkAllocationCallbacks *pAllocator);

SHIM_VK_API VkResult shim_vkGetValidationCacheDataEXT(VkDevice device,
                                                      VkValidationCacheEXT validationCache,
                                                      size_t *pDataSize, void *pData);

SHIM_VK_API VkResult shim_vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache,
                                                     uint32_t srcCacheCount,
                                                     const VkValidationCacheEXT *pSrcCaches);

SHIM_VK_API void shim_vkGetDescriptorSetLayoutSupport(VkDevice device,
                                                      const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                      VkDescriptorSetLayoutSupport *pSupport);

SHIM_VK_API void shim_vkGetDescriptorSetLayoutSupportKHR(
    VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    VkDescriptorSetLayoutSupport *pSupport);

SHIM_VK_API VkResult shim_vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline,
                                             VkShaderStageFlagBits shaderStage,
                                             VkShaderInfoTypeAMD infoType, size_t *pInfoSize,
                                             void *pInfo);

SHIM_VK_API VkResult shim_vkSetDebugUtilsObjectNameEXT(VkDevice device,
                                                       const VkDebugUtilsObjectNameInfoEXT *pNameInfo);

SHIM_VK_API VkResult shim_vkSetDebugUtilsObjectTagEXT(VkDevice device,
                                                      const VkDebugUtilsObjectTagInfoEXT *pTagInfo);

SHIM_VK_API void shim_vkQueueBeginDebugUtilsLabelEXT(VkQueue queue,
                                                     const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API void shim_vkQueueEndDebugUtilsLabelEXT(VkQueue queue);

SHIM_VK_API void shim_vkQueueInsertDebugUtilsLabelEXT(VkQueue queue,
                                                      const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API void shim_vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                   const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API void shim_vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);

SHIM_VK_API void shim_vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                    const VkDebugUtilsLabelEXT *pLabelInfo);

SHIM_VK_API VkResult shim_vkCreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger);

SHIM_VK_API void shim_vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                      VkDebugUtilsMessengerEXT messenger,
                                                      const VkAllocationCallbacks *pAllocator);

SHIM_VK_API void shim_vkSubmitDebugUtilsMessageEXT(
    VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);

SHIM_VK_API VkResult shim_vkGetMemoryHostPointerPropertiesEXT(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void *pHostPointer,
    VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties);

SHIM_VK_API void shim_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer,
                                                VkPipelineStageFlagBits pipelineStage,
                                                VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                uint32_t marker);
)")},

    /******************************************************************************/
    /* TEMPLATE_FILE_GOLD_REFERENCE_SHIM_CPP                                         */
    /******************************************************************************/
    {"gold_reference_shim", "shim_vulkan.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: gold_reference_shim/shim_vulkan.cpp
//-----------------------------------------------------------------------------
#ifndef SHIM_VK_COMPILE_STATIC_LIB
#define SHIM_VK_EXPORT
#endif

#include <iostream>
#include <map>

#include "helper/helper.h"
#include "shim_vulkan.h"
#include "utils.h"

const char *RDOC_ENV_VAR =
    "RDOC_GOLD_FRAME_INDEX";    // environment variable for to-be-captured frame.
int captureFrame = 5;           // default frame index if RDOC_GOLD_FRAME_INDEX is not set.
int presentIndex = 0;
bool IsTargetFrame = true;      // default value doesn't matter. It's properly set in CreateInstance.

int renderPassCount = 0;
AuxVkTraceResources aux;
VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
VkSwapchainCreateInfoKHR swapchainCI;
std::map<VkSwapchainKHR, std::vector<VkImage>> swapchainImageMap;
std::map<VkFramebuffer, std::vector<VkImageView>> framebufferAttachements;
std::map<VkImageView, ImageAndView> imageAndViewMap;
std::map<VkImage, VkImageCreateInfo> imagesMap;
std::map<VkRenderPass, RenderPassInfo> renderPassInfos;
std::map<VkCommandBuffer, RenderPassInfo> cmdBufferRenderPassInfos;
std::map<VkCommandBuffer, std::vector<ReadbackInfos>> cmdBufferReadBackInfos;

/************************* shimmed functions *******************************/
void shim_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                              VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
  PFN_vkGetPhysicalDeviceMemoryProperties fn = vkGetPhysicalDeviceMemoryProperties;
  fn(physicalDevice, pMemoryProperties);
  physicalDeviceMemoryProperties = *pMemoryProperties;
  return;
}

VkResult shim_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkImage *pImage)
{
  VkImageCreateInfo *pCI = (VkImageCreateInfo *)pCreateInfo;
  pCI->usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // we might have to read back from this image.

  PFN_vkCreateImage fn = vkCreateImage;
  VkResult r = fn(device, pCreateInfo, pAllocator, pImage);
  imagesMap[*pImage] = *pCreateInfo;
  return r;
}

VkResult shim_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkImageView *pView)
{
  PFN_vkCreateImageView fn = vkCreateImageView;
  VkResult r = fn(device, pCreateInfo, pAllocator, pView);
  ImageAndView iav(pCreateInfo->image,
    imagesMap[pCreateInfo->image],
    *pView, *pCreateInfo);
  imageAndViewMap[*pView] = iav;
  return r;
}

VkResult shim_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator,
                                  VkFramebuffer *pFramebuffer)
{
  PFN_vkCreateFramebuffer fn = vkCreateFramebuffer;
  VkResult r = fn(device, pCreateInfo, pAllocator, pFramebuffer);
  std::vector<VkImageView> attachments(pCreateInfo->pAttachments,
                                       pCreateInfo->pAttachments + pCreateInfo->attachmentCount);
  framebufferAttachements[*pFramebuffer] = attachments;
  return r;
}

VkResult shim_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass)
{
  PFN_vkCreateRenderPass fn = vkCreateRenderPass;
  // Modify storeOp and stencilStoreOp to VK_ATTACHMENT_STORE_OP_STORE.
  VkRenderPassCreateInfo* CreateInfo = const_cast<VkRenderPassCreateInfo*>(pCreateInfo);
  for(uint32_t i = 0; i < pCreateInfo->attachmentCount; i++)
  {
    VkAttachmentDescription* desc = const_cast<VkAttachmentDescription*>(&CreateInfo->pAttachments[i]);
    desc->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  }

  VkResult r = fn(device, pCreateInfo, pAllocator, pRenderPass);
  RenderPassInfo rpInfo = {*pRenderPass};
  rpInfo.finalLayouts.resize(CreateInfo->attachmentCount);
  for(uint32_t i = 0; i < CreateInfo->attachmentCount; i++)
    rpInfo.finalLayouts[i] = CreateInfo->pAttachments[i].finalLayout;

  assert(renderPassInfos.find(*pRenderPass) == renderPassInfos.end());
  renderPassInfos[*pRenderPass] = rpInfo;
  return r;
}

void shim_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                               const VkRenderPassBeginInfo *pRenderPassBegin,
                               VkSubpassContents contents)
{
  PFN_vkCmdBeginRenderPass fn = vkCmdBeginRenderPass;
  fn(commandBuffer, pRenderPassBegin, contents);
  if(IsTargetFrame)
  {
    VkRenderPass rp = pRenderPassBegin->renderPass;
    // renderpass must be valid AND only a single renderpass can
    // be associated with current command buffer in "Begin" state.
    assert(renderPassInfos.find(rp) != renderPassInfos.end() &&
      cmdBufferRenderPassInfos.find(commandBuffer) == cmdBufferRenderPassInfos.end());
    RenderPassInfo rpInfo = renderPassInfos[rp];
    VkFramebuffer fb = pRenderPassBegin->framebuffer;
    for(uint32_t i = 0; i < framebufferAttachements[fb].size(); i++)
    {
      VkImageView v = framebufferAttachements[fb][i];
      rpInfo.attachments.push_back(imageAndViewMap[v]);
    }
    cmdBufferRenderPassInfos[commandBuffer] = rpInfo;
  }
  return;
}

void shim_vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
  PFN_vkCmdEndRenderPass fn = vkCmdEndRenderPass;
  fn(commandBuffer);
  if(IsTargetFrame)
  {
    assert(cmdBufferRenderPassInfos.find(commandBuffer) != cmdBufferRenderPassInfos.end());
    RenderPassInfo& rpInfo = cmdBufferRenderPassInfos[commandBuffer];
    // produce a readbacks structure that will store resources holding the attahcments data.
    ReadbackInfos readbacks = copyFramebufferAttachments(commandBuffer, &rpInfo);
    // current command buffer accumulates all readbacks so it can save images on queuesubmit.
    cmdBufferReadBackInfos[commandBuffer].push_back(readbacks);
    // clear the renderpass attachments associated with the current command buffer.
    rpInfo.attachments.clear();
    cmdBufferRenderPassInfos.erase(commandBuffer);
  }
  return;
}

VkResult shim_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                            VkFence fence)
{
  PFN_vkQueueSubmit fn = vkQueueSubmit;
  VkResult r = fn(queue, submitCount, pSubmits, fence);
  if(!IsTargetFrame)
    return r;
  vkQueueWaitIdle(queue);
  for(uint32_t i = 0; i < submitCount; i++)
  {
    for(uint32_t cbIndex = 0; cbIndex < pSubmits[i].commandBufferCount; cbIndex++)
    {
      std::vector<ReadbackInfos>& readbacks =
          cmdBufferReadBackInfos[pSubmits[i].pCommandBuffers[cbIndex]];
      for(uint32_t j = 0; j < readbacks.size(); j++)
      {
        ReadbackInfos infos = readbacks[j];
        for(uint32_t a = 0; a < infos.attachments.size(); a++)
        {
          ReadbackInfo info = infos.attachments[a];
          char handleStr[32];
          sprintf(handleStr, "%p", info.srcImage);
          std::string filename = std::to_string(renderPassCount) + "_attachment_" +
                                 std::to_string(info.index) + "_resource_" +
                                 std::string(handleStr) + "_" +
                                 FormatToString(info.format) + "_" +
                                 std::to_string(info.width) + "x" +
                                 std::to_string(info.height) + ".ppm";
          bufferToPpm(info.buffer, info.bufferDeviceMem, filename,
                      info.width, info.height, info.format);
          info.Clear(aux.device);
        }
        renderPassCount++;
      }
      cmdBufferReadBackInfos.erase(pSubmits[i].pCommandBuffers[cbIndex]);
    }
  }
  return r;
}

VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
  PFN_vkQueuePresentKHR fn =
    (PFN_vkQueuePresentKHR) vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
  if(IsTargetFrame)
  {
    // Save screenshots
    for(uint32_t i = 0; i < (*pPresentInfo).swapchainCount; i++)
    {
      VkImage srcImage =
        swapchainImageMap[(*pPresentInfo).pSwapchains[i]][(*pPresentInfo).pImageIndices[i]];
      char filename[128];
#if defined(__yeti__)
      sprintf(filename, "/var/game/screenshot_f%d_sw%d.ppm", presentIndex, i);
#else
      sprintf(filename, "screenshot_f%d_sw%d.ppm", presentIndex, i);
#endif
      screenshot(srcImage, filename);
    }
  }

  IsTargetFrame = (++presentIndex == captureFrame);
  VkResult r = fn(queue, pPresentInfo);
  return r;
}

VkResult shim_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                      uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages)
{
  PFN_vkGetSwapchainImagesKHR fn =
      (PFN_vkGetSwapchainImagesKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
  VkResult r = fn(device, swapchain, pSwapchainImageCount, pSwapchainImages);
  if(pSwapchainImages != NULL)
  {
    VkImageCreateInfo fakeCI{};
    fakeCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    fakeCI.imageType = VK_IMAGE_TYPE_2D;
    fakeCI.format = swapchainCI.imageFormat;
    fakeCI.extent.width = swapchainCI.imageExtent.width;
    fakeCI.extent.height = swapchainCI.imageExtent.height;
    fakeCI.extent.depth = 1;
    fakeCI.arrayLayers = swapchainCI.imageArrayLayers;
    fakeCI.mipLevels = 1;
    fakeCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    fakeCI.samples = VK_SAMPLE_COUNT_1_BIT;
    fakeCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    fakeCI.usage = swapchainCI.imageUsage;
    std::vector<VkImage> swapchainImages(*pSwapchainImageCount);
    for(uint32_t i = 0; i < *pSwapchainImageCount; i++)
    {
      swapchainImages[i] = pSwapchainImages[i];
      // To ensure vkCreateImageView finds present images in imagesMap
      imagesMap[swapchainImages[i]] = fakeCI;
    }
    swapchainImageMap[swapchain] = swapchainImages;
  }
  return r;
}

VkResult shim_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator,
                                   VkSwapchainKHR *pSwapchain)
{
  PFN_vkCreateSwapchainKHR fn =
      (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
  swapchainCI = *pCreateInfo;
  VkSwapchainCreateInfoKHR* pCI = const_cast<VkSwapchainCreateInfoKHR*>(pCreateInfo);
  pCI->imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // we will copy from presented images.
  VkResult r = fn(device, pCreateInfo, pAllocator, pSwapchain);
  return r;
}
/************************* boilerplates *******************************/
VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance)
{
  VkResult r = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
  aux.instance = *pInstance;
  char *envVal = getenv(RDOC_ENV_VAR);
  if(envVal != NULL)
    captureFrame = atoi(envVal);
  IsTargetFrame = presentIndex == captureFrame; // if captureFrame is '0', first frame needs to save images.
  return r;
}

VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
  VkResult r = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
  InitializeAuxResources(&aux, aux.instance, physicalDevice, *pDevice);
  return r;
}

void shim_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyInstance fn = vkDestroyInstance;
  fn(instance, pAllocator);
  return;
}

void shim_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                        VkPhysicalDeviceProperties *pProperties)
{
  PFN_vkGetPhysicalDeviceProperties fn = vkGetPhysicalDeviceProperties;
  fn(physicalDevice, pProperties);
  return;
}

VkResult shim_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                         VkPhysicalDevice *pPhysicalDevices)
{
  PFN_vkEnumeratePhysicalDevices fn = vkEnumeratePhysicalDevices;
  VkResult r = fn(instance, pPhysicalDeviceCount, pPhysicalDevices);
  return r;
}

void shim_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                   uint32_t *pQueueFamilyPropertyCount,
                                                   VkQueueFamilyProperties *pQueueFamilyProperties)
{
  PFN_vkGetPhysicalDeviceQueueFamilyProperties fn = vkGetPhysicalDeviceQueueFamilyProperties;
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  return;
}

void shim_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                      VkPhysicalDeviceFeatures *pFeatures)
{
  PFN_vkGetPhysicalDeviceFeatures fn = vkGetPhysicalDeviceFeatures;
  fn(physicalDevice, pFeatures);
  return;
}

void shim_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                              VkFormatProperties *pFormatProperties)
{
  PFN_vkGetPhysicalDeviceFormatProperties fn = vkGetPhysicalDeviceFormatProperties;
  fn(physicalDevice, format, pFormatProperties);
  return;
}

VkResult shim_vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice,
                                                       VkFormat format, VkImageType type,
                                                       VkImageTiling tiling, VkImageUsageFlags usage,
                                                       VkImageCreateFlags flags,
                                                       VkImageFormatProperties *pImageFormatProperties)
{
  PFN_vkGetPhysicalDeviceImageFormatProperties fn = vkGetPhysicalDeviceImageFormatProperties;
  VkResult r = fn(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
  return r;
}

void shim_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyDevice fn = vkDestroyDevice;
  fn(device, pAllocator);
  return;
}

VkResult shim_vkEnumerateInstanceVersion(uint32_t *pApiVersion)
{
  PFN_vkEnumerateInstanceVersion fn = vkEnumerateInstanceVersion;
  VkResult r = fn(pApiVersion);
  return r;
}

VkResult shim_vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                                 VkLayerProperties *pProperties)
{
  PFN_vkEnumerateInstanceLayerProperties fn = vkEnumerateInstanceLayerProperties;
  VkResult r = fn(pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount,
                                                     VkExtensionProperties *pProperties)
{
  PFN_vkEnumerateInstanceExtensionProperties fn = vkEnumerateInstanceExtensionProperties;
  VkResult r = fn(pLayerName, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                                               uint32_t *pPropertyCount,
                                               VkLayerProperties *pProperties)
{
  PFN_vkEnumerateDeviceLayerProperties fn = vkEnumerateDeviceLayerProperties;
  VkResult r = fn(physicalDevice, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                   const char *pLayerName, uint32_t *pPropertyCount,
                                                   VkExtensionProperties *pProperties)
{
  PFN_vkEnumerateDeviceExtensionProperties fn = vkEnumerateDeviceExtensionProperties;
  VkResult r = fn(physicalDevice, pLayerName, pPropertyCount, pProperties);
  return r;
}

void shim_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                           VkQueue *pQueue)
{
  PFN_vkGetDeviceQueue fn = vkGetDeviceQueue;
  fn(device, queueFamilyIndex, queueIndex, pQueue);
  return;
}

VkResult shim_vkQueueWaitIdle(VkQueue queue)
{
  PFN_vkQueueWaitIdle fn = vkQueueWaitIdle;
  VkResult r = fn(queue);
  return r;
}
)" + std::string(R"(
VkResult shim_vkDeviceWaitIdle(VkDevice device)
{
  PFN_vkDeviceWaitIdle fn = vkDeviceWaitIdle;
  VkResult r = fn(device);
  return r;
}

VkResult shim_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory)
{
  PFN_vkAllocateMemory fn = vkAllocateMemory;
  VkResult r = fn(device, pAllocateInfo, pAllocator, pMemory);
  return r;
}

void shim_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator)
{
  PFN_vkFreeMemory fn = vkFreeMemory;
  fn(device, memory, pAllocator);
  return;
}

VkResult shim_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset,
                          VkDeviceSize size, VkMemoryMapFlags flags, void **ppData)
{
  PFN_vkMapMemory fn = vkMapMemory;
  VkResult r = fn(device, memory, offset, size, flags, ppData);
  return r;
}

void shim_vkUnmapMemory(VkDevice device, VkDeviceMemory memory)
{
  PFN_vkUnmapMemory fn = vkUnmapMemory;
  fn(device, memory);
  return;
}

VkResult shim_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                        const VkMappedMemoryRange *pMemoryRanges)
{
  PFN_vkFlushMappedMemoryRanges fn = vkFlushMappedMemoryRanges;
  VkResult r = fn(device, memoryRangeCount, pMemoryRanges);
  return r;
}

VkResult shim_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                             const VkMappedMemoryRange *pMemoryRanges)
{
  PFN_vkInvalidateMappedMemoryRanges fn = vkInvalidateMappedMemoryRanges;
  VkResult r = fn(device, memoryRangeCount, pMemoryRanges);
  return r;
}

void shim_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                      VkDeviceSize *pCommittedMemoryInBytes)
{
  PFN_vkGetDeviceMemoryCommitment fn = vkGetDeviceMemoryCommitment;
  fn(device, memory, pCommittedMemoryInBytes);
  return;
}

void shim_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                        VkMemoryRequirements *pMemoryRequirements)
{
  PFN_vkGetBufferMemoryRequirements fn = vkGetBufferMemoryRequirements;
  fn(device, buffer, pMemoryRequirements);
  return;
}

VkResult shim_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                 VkDeviceSize memoryOffset)
{
  PFN_vkBindBufferMemory fn = vkBindBufferMemory;
  VkResult r = fn(device, buffer, memory, memoryOffset);
  return r;
}

void shim_vkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                       VkMemoryRequirements *pMemoryRequirements)
{
  PFN_vkGetImageMemoryRequirements fn = vkGetImageMemoryRequirements;
  fn(device, image, pMemoryRequirements);
  return;
}

VkResult shim_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                VkDeviceSize memoryOffset)
{
  PFN_vkBindImageMemory fn = vkBindImageMemory;
  VkResult r = fn(device, image, memory, memoryOffset);
  return r;
}

void shim_vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image,
                                             uint32_t *pSparseMemoryRequirementCount,
                                             VkSparseImageMemoryRequirements *pSparseMemoryRequirements)
{
  PFN_vkGetImageSparseMemoryRequirements fn = vkGetImageSparseMemoryRequirements;
  fn(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
  return;
}

void shim_vkGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
    VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties)
{
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties fn =
      vkGetPhysicalDeviceSparseImageFormatProperties;
  fn(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
  return;
}

VkResult shim_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount,
                                const VkBindSparseInfo *pBindInfo, VkFence fence)
{
  PFN_vkQueueBindSparse fn = vkQueueBindSparse;
  VkResult r = fn(queue, bindInfoCount, pBindInfo, fence);
  return r;
}

VkResult shim_vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkFence *pFence)
{
  PFN_vkCreateFence fn = vkCreateFence;
  VkResult r = fn(device, pCreateInfo, pAllocator, pFence);
  return r;
}

void shim_vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyFence fn = vkDestroyFence;
  fn(device, fence, pAllocator);
  return;
}

VkResult shim_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences)
{
  PFN_vkResetFences fn = vkResetFences;
  VkResult r = fn(device, fenceCount, pFences);
  return r;
}

VkResult shim_vkGetFenceStatus(VkDevice device, VkFence fence)
{
  PFN_vkGetFenceStatus fn = vkGetFenceStatus;
  VkResult r = fn(device, fence);
  return r;
}

VkResult shim_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                              VkBool32 waitAll, uint64_t timeout)
{
  PFN_vkWaitForFences fn = vkWaitForFences;
  VkResult r = fn(device, fenceCount, pFences, waitAll, timeout);
  return r;
}

VkResult shim_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore)
{
  PFN_vkCreateSemaphore fn = vkCreateSemaphore;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSemaphore);
  return r;
}

void shim_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                             const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroySemaphore fn = vkDestroySemaphore;
  fn(device, semaphore, pAllocator);
  return;
}

VkResult shim_vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkEvent *pEvent)
{
  PFN_vkCreateEvent fn = vkCreateEvent;
  VkResult r = fn(device, pCreateInfo, pAllocator, pEvent);
  return r;
}

void shim_vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyEvent fn = vkDestroyEvent;
  fn(device, event, pAllocator);
  return;
}

VkResult shim_vkGetEventStatus(VkDevice device, VkEvent event)
{
  PFN_vkGetEventStatus fn = vkGetEventStatus;
  VkResult r = fn(device, event);
  return r;
}

VkResult shim_vkSetEvent(VkDevice device, VkEvent event)
{
  PFN_vkSetEvent fn = vkSetEvent;
  VkResult r = fn(device, event);
  return r;
}

VkResult shim_vkResetEvent(VkDevice device, VkEvent event)
{
  PFN_vkResetEvent fn = vkResetEvent;
  VkResult r = fn(device, event);
  return r;
}

VkResult shim_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool)
{
  PFN_vkCreateQueryPool fn = vkCreateQueryPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pQueryPool);
  return r;
}

void shim_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                             const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyQueryPool fn = vkDestroyQueryPool;
  fn(device, queryPool, pAllocator);
  return;
}

VkResult shim_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                    uint32_t queryCount, size_t dataSize, void *pData,
                                    VkDeviceSize stride, VkQueryResultFlags flags)
{
  PFN_vkGetQueryPoolResults fn = vkGetQueryPoolResults;
  VkResult r = fn(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
  return r;
}

VkResult shim_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer)
{
  PFN_vkCreateBuffer fn = vkCreateBuffer;
  VkResult r = fn(device, pCreateInfo, pAllocator, pBuffer);
  return r;
}

void shim_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyBuffer fn = vkDestroyBuffer;
  fn(device, buffer, pAllocator);
  return;
}

VkResult shim_vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkBufferView *pView)
{
  PFN_vkCreateBufferView fn = vkCreateBufferView;
  VkResult r = fn(device, pCreateInfo, pAllocator, pView);
  return r;
}

void shim_vkDestroyBufferView(VkDevice device, VkBufferView bufferView,
                              const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyBufferView fn = vkDestroyBufferView;
  fn(device, bufferView, pAllocator);
  return;
}

void shim_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyImage fn = vkDestroyImage;
  fn(device, image, pAllocator);
  return;
}

void shim_vkGetImageSubresourceLayout(VkDevice device, VkImage image,
                                      const VkImageSubresource *pSubresource,
                                      VkSubresourceLayout *pLayout)
{
  PFN_vkGetImageSubresourceLayout fn = vkGetImageSubresourceLayout;
  fn(device, image, pSubresource, pLayout);
  return;
}

void shim_vkDestroyImageView(VkDevice device, VkImageView imageView,
                             const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyImageView fn = vkDestroyImageView;
  fn(device, imageView, pAllocator);
  return;
}

VkResult shim_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                   const VkAllocationCallbacks *pAllocator,
                                   VkShaderModule *pShaderModule)
{
  PFN_vkCreateShaderModule fn = vkCreateShaderModule;
  VkResult r = fn(device, pCreateInfo, pAllocator, pShaderModule);
  return r;
}

void shim_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyShaderModule fn = vkDestroyShaderModule;
  fn(device, shaderModule, pAllocator);
  return;
}

VkResult shim_vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator,
                                    VkPipelineCache *pPipelineCache)
{
  PFN_vkCreatePipelineCache fn = vkCreatePipelineCache;
  VkResult r = fn(device, pCreateInfo, pAllocator, pPipelineCache);
  return r;
}

void shim_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                 const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyPipelineCache fn = vkDestroyPipelineCache;
  fn(device, pipelineCache, pAllocator);
  return;
}

VkResult shim_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache,
                                     size_t *pDataSize, void *pData)
{
  PFN_vkGetPipelineCacheData fn = vkGetPipelineCacheData;
  VkResult r = fn(device, pipelineCache, pDataSize, pData);
  return r;
}

VkResult shim_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache,
                                    uint32_t srcCacheCount, const VkPipelineCache *pSrcCaches)
{
  PFN_vkMergePipelineCaches fn = vkMergePipelineCaches;
  VkResult r = fn(device, dstCache, srcCacheCount, pSrcCaches);
  return r;
}

VkResult shim_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                        uint32_t createInfoCount,
                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                        const VkAllocationCallbacks *pAllocator,
                                        VkPipeline *pPipelines)
{
  PFN_vkCreateGraphicsPipelines fn = vkCreateGraphicsPipelines;
  VkResult r = fn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
  return r;
}

VkResult shim_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                       uint32_t createInfoCount,
                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                       const VkAllocationCallbacks *pAllocator,
                                       VkPipeline *pPipelines)
{
  PFN_vkCreateComputePipelines fn = vkCreateComputePipelines;
  VkResult r = fn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
  return r;
}

void shim_vkDestroyPipeline(VkDevice device, VkPipeline pipeline,
                            const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyPipeline fn = vkDestroyPipeline;
  fn(device, pipeline, pAllocator);
  return;
}

VkResult shim_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkPipelineLayout *pPipelineLayout)
{
  PFN_vkCreatePipelineLayout fn = vkCreatePipelineLayout;
  VkResult r = fn(device, pCreateInfo, pAllocator, pPipelineLayout);
  return r;
}

void shim_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                  const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyPipelineLayout fn = vkDestroyPipelineLayout;
  fn(device, pipelineLayout, pAllocator);
  return;
}

VkResult shim_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler)
{
  PFN_vkCreateSampler fn = vkCreateSampler;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSampler);
  return r;
}

void shim_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroySampler fn = vkDestroySampler;
  fn(device, sampler, pAllocator);
  return;
}

VkResult shim_vkCreateDescriptorSetLayout(VkDevice device,
                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDescriptorSetLayout *pSetLayout)
{
  PFN_vkCreateDescriptorSetLayout fn = vkCreateDescriptorSetLayout;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSetLayout);
  return r;
}

void shim_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                       const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyDescriptorSetLayout fn = vkDestroyDescriptorSetLayout;
  fn(device, descriptorSetLayout, pAllocator);
  return;
}

VkResult shim_vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkDescriptorPool *pDescriptorPool)
{
  PFN_vkCreateDescriptorPool fn = vkCreateDescriptorPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorPool);
  return r;
}

void shim_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                  const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyDescriptorPool fn = vkDestroyDescriptorPool;
  fn(device, descriptorPool, pAllocator);
  return;
}

VkResult shim_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                    VkDescriptorPoolResetFlags flags)
{
  PFN_vkResetDescriptorPool fn = vkResetDescriptorPool;
  VkResult r = fn(device, descriptorPool, flags);
  return r;
}

VkResult shim_vkAllocateDescriptorSets(VkDevice device,
                                       const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                       VkDescriptorSet *pDescriptorSets)
{
  PFN_vkAllocateDescriptorSets fn = vkAllocateDescriptorSets;
  VkResult r = fn(device, pAllocateInfo, pDescriptorSets);
  return r;
}
)") + std::string(R"(
VkResult shim_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                   uint32_t descriptorSetCount,
                                   const VkDescriptorSet *pDescriptorSets)
{
  PFN_vkFreeDescriptorSets fn = vkFreeDescriptorSets;
  VkResult r = fn(device, descriptorPool, descriptorSetCount, pDescriptorSets);
  return r;
}

void shim_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                 const VkWriteDescriptorSet *pDescriptorWrites,
                                 uint32_t descriptorCopyCount,
                                 const VkCopyDescriptorSet *pDescriptorCopies)
{
  PFN_vkUpdateDescriptorSets fn = vkUpdateDescriptorSets;
  fn(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
  return;
}

void shim_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                               const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyFramebuffer fn = vkDestroyFramebuffer;
  fn(device, framebuffer, pAllocator);
  return;
}

void shim_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                              const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyRenderPass fn = vkDestroyRenderPass;
  fn(device, renderPass, pAllocator);
  return;
}

void shim_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass,
                                     VkExtent2D *pGranularity)
{
  PFN_vkGetRenderAreaGranularity fn = vkGetRenderAreaGranularity;
  fn(device, renderPass, pGranularity);
  return;
}

VkResult shim_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator,
                                  VkCommandPool *pCommandPool)
{
  PFN_vkCreateCommandPool fn = vkCreateCommandPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pCommandPool);
  return r;
}

void shim_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                               const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyCommandPool fn = vkDestroyCommandPool;
  fn(device, commandPool, pAllocator);
  return;
}

VkResult shim_vkResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                 VkCommandPoolResetFlags flags)
{
  PFN_vkResetCommandPool fn = vkResetCommandPool;
  VkResult r = fn(device, commandPool, flags);
  return r;
}

VkResult shim_vkAllocateCommandBuffers(VkDevice device,
                                       const VkCommandBufferAllocateInfo *pAllocateInfo,
                                       VkCommandBuffer *pCommandBuffers)
{
  PFN_vkAllocateCommandBuffers fn = vkAllocateCommandBuffers;
  VkResult r = fn(device, pAllocateInfo, pCommandBuffers);
  return r;
}

void shim_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                               uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers)
{
  PFN_vkFreeCommandBuffers fn = vkFreeCommandBuffers;
  fn(device, commandPool, commandBufferCount, pCommandBuffers);
  return;
}

VkResult shim_vkBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                   const VkCommandBufferBeginInfo *pBeginInfo)
{
  PFN_vkBeginCommandBuffer fn = vkBeginCommandBuffer;
  VkResult r = fn(commandBuffer, pBeginInfo);
  return r;
}

VkResult shim_vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
  PFN_vkEndCommandBuffer fn = vkEndCommandBuffer;
  VkResult r = fn(commandBuffer);
  return r;
}

VkResult shim_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
  PFN_vkResetCommandBuffer fn = vkResetCommandBuffer;
  VkResult r = fn(commandBuffer, flags);
  return r;
}

void shim_vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                            VkPipeline pipeline)
{
  PFN_vkCmdBindPipeline fn = vkCmdBindPipeline;
  fn(commandBuffer, pipelineBindPoint, pipeline);
  return;
}

void shim_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                           uint32_t viewportCount, const VkViewport *pViewports)
{
  PFN_vkCmdSetViewport fn = vkCmdSetViewport;
  fn(commandBuffer, firstViewport, viewportCount, pViewports);
  return;
}

void shim_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                          uint32_t scissorCount, const VkRect2D *pScissors)
{
  PFN_vkCmdSetScissor fn = vkCmdSetScissor;
  fn(commandBuffer, firstScissor, scissorCount, pScissors);
  return;
}

void shim_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
  PFN_vkCmdSetLineWidth fn = vkCmdSetLineWidth;
  fn(commandBuffer, lineWidth);
  return;
}

void shim_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                            float depthBiasClamp, float depthBiasSlopeFactor)
{
  PFN_vkCmdSetDepthBias fn = vkCmdSetDepthBias;
  fn(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
  return;
}

void shim_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
  PFN_vkCmdSetBlendConstants fn = vkCmdSetBlendConstants;
  fn(commandBuffer, blendConstants);
  return;
}

void shim_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                              float maxDepthBounds)
{
  PFN_vkCmdSetDepthBounds fn = vkCmdSetDepthBounds;
  fn(commandBuffer, minDepthBounds, maxDepthBounds);
  return;
}

void shim_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                     uint32_t compareMask)
{
  PFN_vkCmdSetStencilCompareMask fn = vkCmdSetStencilCompareMask;
  fn(commandBuffer, faceMask, compareMask);
  return;
}

void shim_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                   uint32_t writeMask)
{
  PFN_vkCmdSetStencilWriteMask fn = vkCmdSetStencilWriteMask;
  fn(commandBuffer, faceMask, writeMask);
  return;
}

void shim_vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                   uint32_t reference)
{
  PFN_vkCmdSetStencilReference fn = vkCmdSetStencilReference;
  fn(commandBuffer, faceMask, reference);
  return;
}

void shim_vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                  VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                  uint32_t firstSet, uint32_t descriptorSetCount,
                                  const VkDescriptorSet *pDescriptorSets,
                                  uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets)
{
  PFN_vkCmdBindDescriptorSets fn = vkCmdBindDescriptorSets;
  fn(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
     dynamicOffsetCount, pDynamicOffsets);
  return;
}

void shim_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                               VkIndexType indexType)
{
  PFN_vkCmdBindIndexBuffer fn = vkCmdBindIndexBuffer;
  fn(commandBuffer, buffer, offset, indexType);
  return;
}

void shim_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                 uint32_t bindingCount, const VkBuffer *pBuffers,
                                 const VkDeviceSize *pOffsets)
{
  PFN_vkCmdBindVertexBuffers fn = vkCmdBindVertexBuffers;
  fn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
  return;
}

void shim_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                    uint32_t firstVertex, uint32_t firstInstance)
{
  PFN_vkCmdDraw fn = vkCmdDraw;
  fn(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
  return;
}

void shim_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                           uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
  PFN_vkCmdDrawIndexed fn = vkCmdDrawIndexed;
  fn(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
  return;
}

void shim_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                            uint32_t drawCount, uint32_t stride)
{
  PFN_vkCmdDrawIndirect fn = vkCmdDrawIndirect;
  fn(commandBuffer, buffer, offset, drawCount, stride);
  return;
}

void shim_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                   VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
  PFN_vkCmdDrawIndexedIndirect fn = vkCmdDrawIndexedIndirect;
  fn(commandBuffer, buffer, offset, drawCount, stride);
  return;
}

void shim_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                        uint32_t groupCountZ)
{
  PFN_vkCmdDispatch fn = vkCmdDispatch;
  fn(commandBuffer, groupCountX, groupCountY, groupCountZ);
  return;
}

void shim_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
  PFN_vkCmdDispatchIndirect fn = vkCmdDispatchIndirect;
  fn(commandBuffer, buffer, offset);
  return;
}

void shim_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                          uint32_t regionCount, const VkBufferCopy *pRegions)
{
  PFN_vkCmdCopyBuffer fn = vkCmdCopyBuffer;
  fn(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
  return;
}

void shim_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                         VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                         uint32_t regionCount, const VkImageCopy *pRegions)
{
  PFN_vkCmdCopyImage fn = vkCmdCopyImage;
  fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
  return;
}

void shim_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                         VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                         uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter)
{
  PFN_vkCmdBlitImage fn = vkCmdBlitImage;
  fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
     filter);
  return;
}

void shim_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                 VkImage dstImage, VkImageLayout dstImageLayout,
                                 uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
  PFN_vkCmdCopyBufferToImage fn = vkCmdCopyBufferToImage;
  fn(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
  return;
}

void shim_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                 VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                 uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
  PFN_vkCmdCopyImageToBuffer fn = vkCmdCopyImageToBuffer;
  fn(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
  return;
}

void shim_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                            VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData)
{
  PFN_vkCmdUpdateBuffer fn = vkCmdUpdateBuffer;
  fn(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
  return;
}

void shim_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                          VkDeviceSize size, uint32_t data)
{
  PFN_vkCmdFillBuffer fn = vkCmdFillBuffer;
  fn(commandBuffer, dstBuffer, dstOffset, size, data);
  return;
}

void shim_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                               VkImageLayout imageLayout, const VkClearColorValue *pColor,
                               uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
  PFN_vkCmdClearColorImage fn = vkCmdClearColorImage;
  fn(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
  return;
}

void shim_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                      VkImageLayout imageLayout,
                                      const VkClearDepthStencilValue *pDepthStencil,
                                      uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
  PFN_vkCmdClearDepthStencilImage fn = vkCmdClearDepthStencilImage;
  fn(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
  return;
}

void shim_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                const VkClearAttachment *pAttachments, uint32_t rectCount,
                                const VkClearRect *pRects)
{
  PFN_vkCmdClearAttachments fn = vkCmdClearAttachments;
  fn(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
  return;
}

void shim_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                            VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount,
                            const VkImageResolve *pRegions)
{
  PFN_vkCmdResolveImage fn = vkCmdResolveImage;
  fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
  return;
}

void shim_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
  PFN_vkCmdSetEvent fn = vkCmdSetEvent;
  fn(commandBuffer, event, stageMask);
  return;
}

void shim_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
  PFN_vkCmdResetEvent fn = vkCmdResetEvent;
  fn(commandBuffer, event, stageMask);
  return;
}

void shim_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount,
                          const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                          VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                          const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                          const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                          uint32_t imageMemoryBarrierCount,
                          const VkImageMemoryBarrier *pImageMemoryBarriers)
{
  PFN_vkCmdWaitEvents fn = vkCmdWaitEvents;
  fn(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
     pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
     pImageMemoryBarriers);
  return;
}

void shim_vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                               uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                               uint32_t bufferMemoryBarrierCount,
                               const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                               uint32_t imageMemoryBarrierCount,
                               const VkImageMemoryBarrier *pImageMemoryBarriers)
{
  PFN_vkCmdPipelineBarrier fn = vkCmdPipelineBarrier;
  fn(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
     bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
  return;
}

void shim_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                          VkQueryControlFlags flags)
{
  PFN_vkCmdBeginQuery fn = vkCmdBeginQuery;
  fn(commandBuffer, queryPool, query, flags);
  return;
}

void shim_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
  PFN_vkCmdEndQuery fn = vkCmdEndQuery;
  fn(commandBuffer, queryPool, query);
  return;
}
)") + std::string(R"(
void shim_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                              uint32_t firstQuery, uint32_t queryCount)
{
  PFN_vkCmdResetQueryPool fn = vkCmdResetQueryPool;
  fn(commandBuffer, queryPool, firstQuery, queryCount);
  return;
}

void shim_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                              VkQueryPool queryPool, uint32_t query)
{
  PFN_vkCmdWriteTimestamp fn = vkCmdWriteTimestamp;
  fn(commandBuffer, pipelineStage, queryPool, query);
  return;
}

void shim_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                    uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                    VkDeviceSize dstOffset, VkDeviceSize stride,
                                    VkQueryResultFlags flags)
{
  PFN_vkCmdCopyQueryPoolResults fn = vkCmdCopyQueryPoolResults;
  fn(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
  return;
}

void shim_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                             VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                             const void *pValues)
{
  PFN_vkCmdPushConstants fn = vkCmdPushConstants;
  fn(commandBuffer, layout, stageFlags, offset, size, pValues);
  return;
}

void shim_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
  PFN_vkCmdNextSubpass fn = vkCmdNextSubpass;
  fn(commandBuffer, contents);
  return;
}

void shim_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                               const VkCommandBuffer *pCommandBuffers)
{
  PFN_vkCmdExecuteCommands fn = vkCmdExecuteCommands;
  fn(commandBuffer, commandBufferCount, pCommandBuffers);
  return;
}

VkResult shim_vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                      uint32_t *pPropertyCount,
                                                      VkDisplayPropertiesKHR *pProperties)
{
  PFN_vkGetPhysicalDeviceDisplayPropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
  VkResult r = fn(physicalDevice, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                           uint32_t *pPropertyCount,
                                                           VkDisplayPlanePropertiesKHR *pProperties)
{
  PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
  VkResult r = fn(physicalDevice, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice,
                                                    uint32_t planeIndex, uint32_t *pDisplayCount,
                                                    VkDisplayKHR *pDisplays)
{
  PFN_vkGetDisplayPlaneSupportedDisplaysKHR fn =
      (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
  VkResult r = fn(physicalDevice, planeIndex, pDisplayCount, pDisplays);
  return r;
}

VkResult shim_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                            uint32_t *pPropertyCount,
                                            VkDisplayModePropertiesKHR *pProperties)
{
  PFN_vkGetDisplayModePropertiesKHR fn = (PFN_vkGetDisplayModePropertiesKHR)vkGetInstanceProcAddr(
      aux.instance, "vkGetDisplayModePropertiesKHR");
  VkResult r = fn(physicalDevice, display, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                     const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode)
{
  PFN_vkCreateDisplayModeKHR fn =
      (PFN_vkCreateDisplayModeKHR)vkGetInstanceProcAddr(aux.instance, "vkCreateDisplayModeKHR");
  VkResult r = fn(physicalDevice, display, pCreateInfo, pAllocator, pMode);
  return r;
}

VkResult shim_vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                               VkDisplayModeKHR mode, uint32_t planeIndex,
                                               VkDisplayPlaneCapabilitiesKHR *pCapabilities)
{
  PFN_vkGetDisplayPlaneCapabilitiesKHR fn =
      (PFN_vkGetDisplayPlaneCapabilitiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetDisplayPlaneCapabilitiesKHR");
  VkResult r = fn(physicalDevice, mode, planeIndex, pCapabilities);
  return r;
}

VkResult shim_vkCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                             const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkSurfaceKHR *pSurface)
{
  PFN_vkCreateDisplayPlaneSurfaceKHR fn = (PFN_vkCreateDisplayPlaneSurfaceKHR)vkGetInstanceProcAddr(
      instance, "vkCreateDisplayPlaneSurfaceKHR");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pSurface);
  return r;
}

VkResult shim_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkSwapchainKHR *pSwapchains)
{
  PFN_vkCreateSharedSwapchainsKHR fn =
      (PFN_vkCreateSharedSwapchainsKHR)vkGetDeviceProcAddr(device, "vkCreateSharedSwapchainsKHR");
  VkResult r = fn(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
  return r;
}

void shim_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                              const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroySurfaceKHR fn =
      (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR");
  fn(instance, surface, pAllocator);
  return;
}

VkResult shim_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                   uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                   VkBool32 *pSupported)
{
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
  VkResult r = fn(physicalDevice, queueFamilyIndex, surface, pSupported);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                        VkSurfaceKHR surface,
                                                        VkSurfaceCapabilitiesKHR *pSurfaceCapabilities)
{
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
  VkResult r = fn(physicalDevice, surface, pSurfaceCapabilities);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice,
                                                   VkSurfaceKHR surface,
                                                   uint32_t *pSurfaceFormatCount,
                                                   VkSurfaceFormatKHR *pSurfaceFormats)
{
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
  VkResult r = fn(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                        VkSurfaceKHR surface,
                                                        uint32_t *pPresentModeCount,
                                                        VkPresentModeKHR *pPresentModes)
{
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fn =
      (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
  VkResult r = fn(physicalDevice, surface, pPresentModeCount, pPresentModes);
  return r;
}

void shim_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroySwapchainKHR fn =
      (PFN_vkDestroySwapchainKHR)vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
  fn(device, swapchain, pAllocator);
  return;
}

VkResult shim_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                    VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
{
  PFN_vkAcquireNextImageKHR fn =
      (PFN_vkAcquireNextImageKHR)vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
  VkResult r = fn(device, swapchain, timeout, semaphore, fence, pImageIndex);
  return r;
}

VkResult shim_vkCreateDebugReportCallbackEXT(VkInstance instance,
                                             const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkDebugReportCallbackEXT *pCallback)
{
  PFN_vkCreateDebugReportCallbackEXT fn = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugReportCallbackEXT");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pCallback);
  return r;
}

void shim_vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                          const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyDebugReportCallbackEXT fn = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugReportCallbackEXT");
  fn(instance, callback, pAllocator);
  return;
}

void shim_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                  VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                  size_t location, int32_t messageCode, const char *pLayerPrefix,
                                  const char *pMessage)
{
  PFN_vkDebugReportMessageEXT fn =
      (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");
  fn(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
  return;
}

VkResult shim_vkDebugMarkerSetObjectNameEXT(VkDevice device,
                                            const VkDebugMarkerObjectNameInfoEXT *pNameInfo)
{
  PFN_vkDebugMarkerSetObjectNameEXT fn = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(
      device, "vkDebugMarkerSetObjectNameEXT");
  VkResult r = fn(device, pNameInfo);
  return r;
}

VkResult shim_vkDebugMarkerSetObjectTagEXT(VkDevice device,
                                           const VkDebugMarkerObjectTagInfoEXT *pTagInfo)
{
  PFN_vkDebugMarkerSetObjectTagEXT fn =
      (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
  VkResult r = fn(device, pTagInfo);
  return r;
}

void shim_vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                   const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
  PFN_vkCmdDebugMarkerBeginEXT fn =
      (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(aux.device, "vkCmdDebugMarkerBeginEXT");
  fn(commandBuffer, pMarkerInfo);
  return;
}

void shim_vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer)
{
  PFN_vkCmdDebugMarkerEndEXT fn =
      (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(aux.device, "vkCmdDebugMarkerEndEXT");
  fn(commandBuffer);
  return;
}

void shim_vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
                                    const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
  PFN_vkCmdDebugMarkerInsertEXT fn =
      (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(aux.device, "vkCmdDebugMarkerInsertEXT");
  fn(commandBuffer, pMarkerInfo);
  return;
}

VkResult shim_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties)
{
  PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV fn =
      (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
  VkResult r = fn(physicalDevice, format, type, tiling, usage, flags, externalHandleType,
                  pExternalImageFormatProperties);
  return r;
}

void shim_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                    VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                    uint32_t stride)
{
  PFN_vkCmdDrawIndirectCountAMD fn =
      (PFN_vkCmdDrawIndirectCountAMD)vkGetDeviceProcAddr(aux.device, "vkCmdDrawIndirectCountAMD");
  fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
  return;
}

void shim_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                           VkDeviceSize offset, VkBuffer countBuffer,
                                           VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride)
{
  PFN_vkCmdDrawIndexedIndirectCountAMD fn = (PFN_vkCmdDrawIndexedIndirectCountAMD)vkGetDeviceProcAddr(
      aux.device, "vkCmdDrawIndexedIndirectCountAMD");
  fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
  return;
}

void shim_vkCmdProcessCommandsNVX(VkCommandBuffer commandBuffer,
                                  const VkCmdProcessCommandsInfoNVX *pProcessCommandsInfo)
{
  PFN_vkCmdProcessCommandsNVX fn =
      (PFN_vkCmdProcessCommandsNVX)vkGetDeviceProcAddr(aux.device, "vkCmdProcessCommandsNVX");
  fn(commandBuffer, pProcessCommandsInfo);
  return;
}

void shim_vkCmdReserveSpaceForCommandsNVX(VkCommandBuffer commandBuffer,
                                          const VkCmdReserveSpaceForCommandsInfoNVX *pReserveSpaceInfo)
{
  PFN_vkCmdReserveSpaceForCommandsNVX fn = (PFN_vkCmdReserveSpaceForCommandsNVX)vkGetDeviceProcAddr(
      aux.device, "vkCmdReserveSpaceForCommandsNVX");
  fn(commandBuffer, pReserveSpaceInfo);
  return;
}

VkResult shim_vkCreateIndirectCommandsLayoutNVX(
    VkDevice device, const VkIndirectCommandsLayoutCreateInfoNVX *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNVX *pIndirectCommandsLayout)
{
  PFN_vkCreateIndirectCommandsLayoutNVX fn =
      (PFN_vkCreateIndirectCommandsLayoutNVX)vkGetDeviceProcAddr(
          device, "vkCreateIndirectCommandsLayoutNVX");
  VkResult r = fn(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
  return r;
}

void shim_vkDestroyIndirectCommandsLayoutNVX(VkDevice device,
                                             VkIndirectCommandsLayoutNVX indirectCommandsLayout,
                                             const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyIndirectCommandsLayoutNVX fn =
      (PFN_vkDestroyIndirectCommandsLayoutNVX)vkGetDeviceProcAddr(
          device, "vkDestroyIndirectCommandsLayoutNVX");
  fn(device, indirectCommandsLayout, pAllocator);
  return;
}
)") + std::string(R"(
VkResult shim_vkCreateObjectTableNVX(VkDevice device, const VkObjectTableCreateInfoNVX *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkObjectTableNVX *pObjectTable)
{
  PFN_vkCreateObjectTableNVX fn =
      (PFN_vkCreateObjectTableNVX)vkGetDeviceProcAddr(device, "vkCreateObjectTableNVX");
  VkResult r = fn(device, pCreateInfo, pAllocator, pObjectTable);
  return r;
}

void shim_vkDestroyObjectTableNVX(VkDevice device, VkObjectTableNVX objectTable,
                                  const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyObjectTableNVX fn =
      (PFN_vkDestroyObjectTableNVX)vkGetDeviceProcAddr(device, "vkDestroyObjectTableNVX");
  fn(device, objectTable, pAllocator);
  return;
}

VkResult shim_vkRegisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                   uint32_t objectCount,
                                   const VkObjectTableEntryNVX *const *ppObjectTableEntries,
                                   const uint32_t *pObjectIndices)
{
  PFN_vkRegisterObjectsNVX fn =
      (PFN_vkRegisterObjectsNVX)vkGetDeviceProcAddr(device, "vkRegisterObjectsNVX");
  VkResult r = fn(device, objectTable, objectCount, ppObjectTableEntries, pObjectIndices);
  return r;
}

VkResult shim_vkUnregisterObjectsNVX(VkDevice device, VkObjectTableNVX objectTable,
                                     uint32_t objectCount,
                                     const VkObjectEntryTypeNVX *pObjectEntryTypes,
                                     const uint32_t *pObjectIndices)
{
  PFN_vkUnregisterObjectsNVX fn =
      (PFN_vkUnregisterObjectsNVX)vkGetDeviceProcAddr(device, "vkUnregisterObjectsNVX");
  VkResult r = fn(device, objectTable, objectCount, pObjectEntryTypes, pObjectIndices);
  return r;
}

void shim_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice physicalDevice, VkDeviceGeneratedCommandsFeaturesNVX *pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX *pLimits)
{
  PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX fn =
      (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX)vkGetDeviceProcAddr(
          aux.device, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
  fn(physicalDevice, pFeatures, pLimits);
  return;
}

void shim_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                       VkPhysicalDeviceFeatures2 *pFeatures)
{
  PFN_vkGetPhysicalDeviceFeatures2 fn = (PFN_vkGetPhysicalDeviceFeatures2)vkGetInstanceProcAddr(
      aux.instance, "vkGetPhysicalDeviceFeatures2");
  fn(physicalDevice, pFeatures);
  return;
}

void shim_vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                          VkPhysicalDeviceFeatures2 *pFeatures)
{
  PFN_vkGetPhysicalDeviceFeatures2KHR fn = (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(
      aux.instance, "vkGetPhysicalDeviceFeatures2KHR");
  fn(physicalDevice, pFeatures);
  return;
}

void shim_vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                         VkPhysicalDeviceProperties2 *pProperties)
{
  PFN_vkGetPhysicalDeviceProperties2 fn = (PFN_vkGetPhysicalDeviceProperties2)vkGetInstanceProcAddr(
      aux.instance, "vkGetPhysicalDeviceProperties2");
  fn(physicalDevice, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                            VkPhysicalDeviceProperties2 *pProperties)
{
  PFN_vkGetPhysicalDeviceProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceProperties2KHR");
  fn(physicalDevice, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                               VkFormatProperties2 *pFormatProperties)
{
  PFN_vkGetPhysicalDeviceFormatProperties2 fn =
      (PFN_vkGetPhysicalDeviceFormatProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceFormatProperties2");
  fn(physicalDevice, format, pFormatProperties);
  return;
}

void shim_vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                  VkFormatProperties2 *pFormatProperties)
{
  PFN_vkGetPhysicalDeviceFormatProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceFormatProperties2KHR");
  fn(physicalDevice, format, pFormatProperties);
  return;
}

VkResult shim_vkGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties)
{
  PFN_vkGetPhysicalDeviceImageFormatProperties2 fn =
      (PFN_vkGetPhysicalDeviceImageFormatProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceImageFormatProperties2");
  VkResult r = fn(physicalDevice, pImageFormatInfo, pImageFormatProperties);
  return r;
}

VkResult shim_vkGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties)
{
  PFN_vkGetPhysicalDeviceImageFormatProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
  VkResult r = fn(physicalDevice, pImageFormatInfo, pImageFormatProperties);
  return r;
}

void shim_vkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                    uint32_t *pQueueFamilyPropertyCount,
                                                    VkQueueFamilyProperties2 *pQueueFamilyProperties)
{
  PFN_vkGetPhysicalDeviceQueueFamilyProperties2 fn =
      (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  return;
}

void shim_vkGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                       uint32_t *pQueueFamilyPropertyCount,
                                                       VkQueueFamilyProperties2 *pQueueFamilyProperties)
{
  PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
  return;
}

void shim_vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                               VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
{
  PFN_vkGetPhysicalDeviceMemoryProperties2 fn =
      (PFN_vkGetPhysicalDeviceMemoryProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceMemoryProperties2");
  fn(physicalDevice, pMemoryProperties);
  return;
}

void shim_vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
{
  PFN_vkGetPhysicalDeviceMemoryProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
  fn(physicalDevice, pMemoryProperties);
  return;
}

void shim_vkGetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties)
{
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 fn =
      (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
  fn(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
  return;
}

void shim_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties)
{
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR fn =
      (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
  fn(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
  return;
}

void shim_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                    VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                    uint32_t set, uint32_t descriptorWriteCount,
                                    const VkWriteDescriptorSet *pDescriptorWrites)
{
  PFN_vkCmdPushDescriptorSetKHR fn =
      (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(aux.device, "vkCmdPushDescriptorSetKHR");
  fn(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
  return;
}

void shim_vkTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
{
  PFN_vkTrimCommandPool fn =
      (PFN_vkTrimCommandPool)vkGetDeviceProcAddr(device, "vkTrimCommandPool");
  fn(device, commandPool, flags);
  return;
}

void shim_vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool,
                               VkCommandPoolTrimFlags flags)
{
  PFN_vkTrimCommandPoolKHR fn =
      (PFN_vkTrimCommandPoolKHR)vkGetDeviceProcAddr(device, "vkTrimCommandPoolKHR");
  fn(device, commandPool, flags);
  return;
}

void shim_vkGetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties)
{
  PFN_vkGetPhysicalDeviceExternalBufferProperties fn =
      (PFN_vkGetPhysicalDeviceExternalBufferProperties)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalBufferProperties");
  fn(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
  return;
}

void shim_vkGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
    VkExternalBufferProperties *pExternalBufferProperties)
{
  PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
  fn(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
  return;
}

VkResult shim_vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd)
{
  PFN_vkGetMemoryFdKHR fn = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdKHR");
  VkResult r = fn(device, pGetFdInfo, pFd);
  return r;
}

VkResult shim_vkGetMemoryFdPropertiesKHR(VkDevice device,
                                         VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                         VkMemoryFdPropertiesKHR *pMemoryFdProperties)
{
  PFN_vkGetMemoryFdPropertiesKHR fn =
      (PFN_vkGetMemoryFdPropertiesKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdPropertiesKHR");
  VkResult r = fn(device, handleType, fd, pMemoryFdProperties);
  return r;
}

void shim_vkGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
{
  PFN_vkGetPhysicalDeviceExternalSemaphoreProperties fn =
      (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
  fn(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
  return;
}

void shim_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
{
  PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
  fn(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
  return;
}

VkResult shim_vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd)
{
  PFN_vkGetSemaphoreFdKHR fn =
      (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreFdKHR");
  VkResult r = fn(device, pGetFdInfo, pFd);
  return r;
}

VkResult shim_vkImportSemaphoreFdKHR(VkDevice device,
                                     const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo)
{
  PFN_vkImportSemaphoreFdKHR fn =
      (PFN_vkImportSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkImportSemaphoreFdKHR");
  VkResult r = fn(device, pImportSemaphoreFdInfo);
  return r;
}

void shim_vkGetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties)
{
  PFN_vkGetPhysicalDeviceExternalFenceProperties fn =
      (PFN_vkGetPhysicalDeviceExternalFenceProperties)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalFenceProperties");
  fn(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
  return;
}

void shim_vkGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
    VkExternalFenceProperties *pExternalFenceProperties)
{
  PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR fn =
      (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
  fn(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
  return;
}

VkResult shim_vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd)
{
  PFN_vkGetFenceFdKHR fn = (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(device, "vkGetFenceFdKHR");
  VkResult r = fn(device, pGetFdInfo, pFd);
  return r;
}

VkResult shim_vkImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo)
{
  PFN_vkImportFenceFdKHR fn =
      (PFN_vkImportFenceFdKHR)vkGetDeviceProcAddr(device, "vkImportFenceFdKHR");
  VkResult r = fn(device, pImportFenceFdInfo);
  return r;
}

VkResult shim_vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display)
{
  PFN_vkReleaseDisplayEXT fn =
      (PFN_vkReleaseDisplayEXT)vkGetInstanceProcAddr(aux.instance, "vkReleaseDisplayEXT");
  VkResult r = fn(physicalDevice, display);
  return r;
}

VkResult shim_vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display,
                                       const VkDisplayPowerInfoEXT *pDisplayPowerInfo)
{
  PFN_vkDisplayPowerControlEXT fn =
      (PFN_vkDisplayPowerControlEXT)vkGetDeviceProcAddr(device, "vkDisplayPowerControlEXT");
  VkResult r = fn(device, display, pDisplayPowerInfo);
  return r;
}

VkResult shim_vkRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT *pDeviceEventInfo,
                                       const VkAllocationCallbacks *pAllocator, VkFence *pFence)
{
  PFN_vkRegisterDeviceEventEXT fn =
      (PFN_vkRegisterDeviceEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDeviceEventEXT");
  VkResult r = fn(device, pDeviceEventInfo, pAllocator, pFence);
  return r;
}

VkResult shim_vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display,
                                        const VkDisplayEventInfoEXT *pDisplayEventInfo,
                                        const VkAllocationCallbacks *pAllocator, VkFence *pFence)
{
  PFN_vkRegisterDisplayEventEXT fn =
      (PFN_vkRegisterDisplayEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDisplayEventEXT");
  VkResult r = fn(device, display, pDisplayEventInfo, pAllocator, pFence);
  return r;
}
)") + std::string(R"(
VkResult shim_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain,
                                       VkSurfaceCounterFlagBitsEXT counter, uint64_t *pCounterValue)
{
  PFN_vkGetSwapchainCounterEXT fn =
      (PFN_vkGetSwapchainCounterEXT)vkGetDeviceProcAddr(device, "vkGetSwapchainCounterEXT");
  VkResult r = fn(device, swapchain, counter, pCounterValue);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                         VkSurfaceKHR surface,
                                                         VkSurfaceCapabilities2EXT *pSurfaceCapabilities)
{
  PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT fn =
      (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
  VkResult r = fn(physicalDevice, surface, pSurfaceCapabilities);
  return r;
}

VkResult shim_vkEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties)
{
  PFN_vkEnumeratePhysicalDeviceGroups fn = (PFN_vkEnumeratePhysicalDeviceGroups)vkGetInstanceProcAddr(
      instance, "vkEnumeratePhysicalDeviceGroups");
  VkResult r = fn(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
  return r;
}

VkResult shim_vkEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties)
{
  PFN_vkEnumeratePhysicalDeviceGroupsKHR fn =
      (PFN_vkEnumeratePhysicalDeviceGroupsKHR)vkGetInstanceProcAddr(
          instance, "vkEnumeratePhysicalDeviceGroupsKHR");
  VkResult r = fn(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
  return r;
}

void shim_vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex,
                                             uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
                                             VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
  PFN_vkGetDeviceGroupPeerMemoryFeatures fn =
      (PFN_vkGetDeviceGroupPeerMemoryFeatures)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupPeerMemoryFeatures");
  fn(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
  return;
}

void shim_vkGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex,
                                                uint32_t localDeviceIndex, uint32_t remoteDeviceIndex,
                                                VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
  PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR fn =
      (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
  fn(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
  return;
}

VkResult shim_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                  const VkBindBufferMemoryInfo *pBindInfos)
{
  PFN_vkBindBufferMemory2 fn =
      (PFN_vkBindBufferMemory2)vkGetDeviceProcAddr(device, "vkBindBufferMemory2");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

VkResult shim_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                     const VkBindBufferMemoryInfo *pBindInfos)
{
  PFN_vkBindBufferMemory2KHR fn =
      (PFN_vkBindBufferMemory2KHR)vkGetDeviceProcAddr(device, "vkBindBufferMemory2KHR");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

VkResult shim_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                 const VkBindImageMemoryInfo *pBindInfos)
{
  PFN_vkBindImageMemory2 fn =
      (PFN_vkBindImageMemory2)vkGetDeviceProcAddr(device, "vkBindImageMemory2");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

VkResult shim_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                    const VkBindImageMemoryInfo *pBindInfos)
{
  PFN_vkBindImageMemory2KHR fn =
      (PFN_vkBindImageMemory2KHR)vkGetDeviceProcAddr(device, "vkBindImageMemory2KHR");
  VkResult r = fn(device, bindInfoCount, pBindInfos);
  return r;
}

void shim_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
  PFN_vkCmdSetDeviceMask fn =
      (PFN_vkCmdSetDeviceMask)vkGetDeviceProcAddr(aux.device, "vkCmdSetDeviceMask");
  fn(commandBuffer, deviceMask);
  return;
}

void shim_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
  PFN_vkCmdSetDeviceMaskKHR fn =
      (PFN_vkCmdSetDeviceMaskKHR)vkGetDeviceProcAddr(aux.device, "vkCmdSetDeviceMaskKHR");
  fn(commandBuffer, deviceMask);
  return;
}

VkResult shim_vkGetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device, VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities)
{
  PFN_vkGetDeviceGroupPresentCapabilitiesKHR fn =
      (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupPresentCapabilitiesKHR");
  VkResult r = fn(device, pDeviceGroupPresentCapabilities);
  return r;
}

VkResult shim_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                     VkDeviceGroupPresentModeFlagsKHR *pModes)
{
  PFN_vkGetDeviceGroupSurfacePresentModesKHR fn =
      (PFN_vkGetDeviceGroupSurfacePresentModesKHR)vkGetDeviceProcAddr(
          device, "vkGetDeviceGroupSurfacePresentModesKHR");
  VkResult r = fn(device, surface, pModes);
  return r;
}

VkResult shim_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                     uint32_t *pImageIndex)
{
  PFN_vkAcquireNextImage2KHR fn =
      (PFN_vkAcquireNextImage2KHR)vkGetDeviceProcAddr(device, "vkAcquireNextImage2KHR");
  VkResult r = fn(device, pAcquireInfo, pImageIndex);
  return r;
}

void shim_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                            uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                            uint32_t groupCountZ)
{
  PFN_vkCmdDispatchBase fn =
      (PFN_vkCmdDispatchBase)vkGetDeviceProcAddr(aux.device, "vkCmdDispatchBase");
  fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
  return;
}

void shim_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                               uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX,
                               uint32_t groupCountY, uint32_t groupCountZ)
{
  PFN_vkCmdDispatchBaseKHR fn =
      (PFN_vkCmdDispatchBaseKHR)vkGetDeviceProcAddr(aux.device, "vkCmdDispatchBaseKHR");
  fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
  return;
}

VkResult shim_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice,
                                                      VkSurfaceKHR surface, uint32_t *pRectCount,
                                                      VkRect2D *pRects)
{
  PFN_vkGetPhysicalDevicePresentRectanglesKHR fn =
      (PFN_vkGetPhysicalDevicePresentRectanglesKHR)vkGetDeviceProcAddr(
          aux.device, "vkGetPhysicalDevicePresentRectanglesKHR");
  VkResult r = fn(physicalDevice, surface, pRectCount, pRects);
  return r;
}

VkResult shim_vkCreateDescriptorUpdateTemplate(VkDevice device,
                                               const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate)
{
  PFN_vkCreateDescriptorUpdateTemplate fn = (PFN_vkCreateDescriptorUpdateTemplate)vkGetDeviceProcAddr(
      device, "vkCreateDescriptorUpdateTemplate");
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
  return r;
}

VkResult shim_vkCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate)
{
  PFN_vkCreateDescriptorUpdateTemplateKHR fn =
      (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(
          device, "vkCreateDescriptorUpdateTemplateKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
  return r;
}

void shim_vkDestroyDescriptorUpdateTemplate(VkDevice device,
                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                            const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyDescriptorUpdateTemplate fn =
      (PFN_vkDestroyDescriptorUpdateTemplate)vkGetDeviceProcAddr(
          device, "vkDestroyDescriptorUpdateTemplate");
  fn(device, descriptorUpdateTemplate, pAllocator);
  return;
}

void shim_vkDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                               const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyDescriptorUpdateTemplateKHR fn =
      (PFN_vkDestroyDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(
          device, "vkDestroyDescriptorUpdateTemplateKHR");
  fn(device, descriptorUpdateTemplate, pAllocator);
  return;
}

void shim_vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                            VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                            const void *pData)
{
  PFN_vkUpdateDescriptorSetWithTemplate fn =
      (PFN_vkUpdateDescriptorSetWithTemplate)vkGetDeviceProcAddr(
          device, "vkUpdateDescriptorSetWithTemplate");
  fn(device, descriptorSet, descriptorUpdateTemplate, pData);
  return;
}

void shim_vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                               VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                               const void *pData)
{
  PFN_vkUpdateDescriptorSetWithTemplateKHR fn =
      (PFN_vkUpdateDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(
          device, "vkUpdateDescriptorSetWithTemplateKHR");
  fn(device, descriptorSet, descriptorUpdateTemplate, pData);
  return;
}

void shim_vkCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                VkPipelineLayout layout, uint32_t set,
                                                const void *pData)
{
  PFN_vkCmdPushDescriptorSetWithTemplateKHR fn =
      (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(
          aux.device, "vkCmdPushDescriptorSetWithTemplateKHR");
  fn(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
  return;
}

void shim_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount,
                              const VkSwapchainKHR *pSwapchains, const VkHdrMetadataEXT *pMetadata)
{
  PFN_vkSetHdrMetadataEXT fn =
      (PFN_vkSetHdrMetadataEXT)vkGetDeviceProcAddr(device, "vkSetHdrMetadataEXT");
  fn(device, swapchainCount, pSwapchains, pMetadata);
  return;
}

VkResult shim_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain)
{
  PFN_vkGetSwapchainStatusKHR fn =
      (PFN_vkGetSwapchainStatusKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainStatusKHR");
  VkResult r = fn(device, swapchain);
  return r;
}

VkResult shim_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                              VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties)
{
  PFN_vkGetRefreshCycleDurationGOOGLE fn = (PFN_vkGetRefreshCycleDurationGOOGLE)vkGetDeviceProcAddr(
      device, "vkGetRefreshCycleDurationGOOGLE");
  VkResult r = fn(device, swapchain, pDisplayTimingProperties);
  return r;
}

VkResult shim_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain,
                                                uint32_t *pPresentationTimingCount,
                                                VkPastPresentationTimingGOOGLE *pPresentationTimings)
{
  PFN_vkGetPastPresentationTimingGOOGLE fn =
      (PFN_vkGetPastPresentationTimingGOOGLE)vkGetDeviceProcAddr(
          device, "vkGetPastPresentationTimingGOOGLE");
  VkResult r = fn(device, swapchain, pPresentationTimingCount, pPresentationTimings);
  return r;
}

void shim_vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                     uint32_t viewportCount,
                                     const VkViewportWScalingNV *pViewportWScalings)
{
  PFN_vkCmdSetViewportWScalingNV fn =
      (PFN_vkCmdSetViewportWScalingNV)vkGetDeviceProcAddr(aux.device, "vkCmdSetViewportWScalingNV");
  fn(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
  return;
}

void shim_vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                      uint32_t discardRectangleCount,
                                      const VkRect2D *pDiscardRectangles)
{
  PFN_vkCmdSetDiscardRectangleEXT fn = (PFN_vkCmdSetDiscardRectangleEXT)vkGetDeviceProcAddr(
      aux.device, "vkCmdSetDiscardRectangleEXT");
  fn(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
  return;
}

void shim_vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                     const VkSampleLocationsInfoEXT *pSampleLocationsInfo)
{
  PFN_vkCmdSetSampleLocationsEXT fn =
      (PFN_vkCmdSetSampleLocationsEXT)vkGetDeviceProcAddr(aux.device, "vkCmdSetSampleLocationsEXT");
  fn(commandBuffer, pSampleLocationsInfo);
  return;
}

void shim_vkGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice,
                                                      VkSampleCountFlagBits samples,
                                                      VkMultisamplePropertiesEXT *pMultisampleProperties)
{
  PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT fn =
      (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)vkGetDeviceProcAddr(
          aux.device, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
  fn(physicalDevice, samples, pMultisampleProperties);
  return;
}

VkResult shim_vkGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities)
{
  PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
  VkResult r = fn(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
  return r;
}

VkResult shim_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                    const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                    uint32_t *pSurfaceFormatCount,
                                                    VkSurfaceFormat2KHR *pSurfaceFormats)
{
  PFN_vkGetPhysicalDeviceSurfaceFormats2KHR fn =
      (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)vkGetInstanceProcAddr(
          aux.instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
  VkResult r = fn(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
  return r;
}

void shim_vkGetBufferMemoryRequirements2(VkDevice device,
                                         const VkBufferMemoryRequirementsInfo2 *pInfo,
                                         VkMemoryRequirements2 *pMemoryRequirements)
{
  PFN_vkGetBufferMemoryRequirements2 fn = (PFN_vkGetBufferMemoryRequirements2)vkGetDeviceProcAddr(
      device, "vkGetBufferMemoryRequirements2");
  fn(device, pInfo, pMemoryRequirements);
  return;
}
)") + std::string(R"(
void shim_vkGetBufferMemoryRequirements2KHR(VkDevice device,
                                            const VkBufferMemoryRequirementsInfo2 *pInfo,
                                            VkMemoryRequirements2 *pMemoryRequirements)
{
  PFN_vkGetBufferMemoryRequirements2KHR fn =
      (PFN_vkGetBufferMemoryRequirements2KHR)vkGetDeviceProcAddr(
          device, "vkGetBufferMemoryRequirements2KHR");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                        VkMemoryRequirements2 *pMemoryRequirements)
{
  PFN_vkGetImageMemoryRequirements2 fn = (PFN_vkGetImageMemoryRequirements2)vkGetDeviceProcAddr(
      device, "vkGetImageMemoryRequirements2");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetImageMemoryRequirements2KHR(VkDevice device,
                                           const VkImageMemoryRequirementsInfo2 *pInfo,
                                           VkMemoryRequirements2 *pMemoryRequirements)
{
  PFN_vkGetImageMemoryRequirements2KHR fn = (PFN_vkGetImageMemoryRequirements2KHR)vkGetDeviceProcAddr(
      device, "vkGetImageMemoryRequirements2KHR");
  fn(device, pInfo, pMemoryRequirements);
  return;
}

void shim_vkGetImageSparseMemoryRequirements2(VkDevice device,
                                              const VkImageSparseMemoryRequirementsInfo2 *pInfo,
                                              uint32_t *pSparseMemoryRequirementCount,
                                              VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
  PFN_vkGetImageSparseMemoryRequirements2 fn =
      (PFN_vkGetImageSparseMemoryRequirements2)vkGetDeviceProcAddr(
          device, "vkGetImageSparseMemoryRequirements2");
  fn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
  return;
}

void shim_vkGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
  PFN_vkGetImageSparseMemoryRequirements2KHR fn =
      (PFN_vkGetImageSparseMemoryRequirements2KHR)vkGetDeviceProcAddr(
          device, "vkGetImageSparseMemoryRequirements2KHR");
  fn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
  return;
}

VkResult shim_vkCreateSamplerYcbcrConversion(VkDevice device,
                                             const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkSamplerYcbcrConversion *pYcbcrConversion)
{
  PFN_vkCreateSamplerYcbcrConversion fn = (PFN_vkCreateSamplerYcbcrConversion)vkGetDeviceProcAddr(
      device, "vkCreateSamplerYcbcrConversion");
  VkResult r = fn(device, pCreateInfo, pAllocator, pYcbcrConversion);
  return r;
}

VkResult shim_vkCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator,
                                                VkSamplerYcbcrConversion *pYcbcrConversion)
{
  PFN_vkCreateSamplerYcbcrConversionKHR fn =
      (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(
          device, "vkCreateSamplerYcbcrConversionKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pYcbcrConversion);
  return r;
}

void shim_vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                          const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroySamplerYcbcrConversion fn = (PFN_vkDestroySamplerYcbcrConversion)vkGetDeviceProcAddr(
      device, "vkDestroySamplerYcbcrConversion");
  fn(device, ycbcrConversion, pAllocator);
  return;
}

void shim_vkDestroySamplerYcbcrConversionKHR(VkDevice device,
                                             VkSamplerYcbcrConversion ycbcrConversion,
                                             const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroySamplerYcbcrConversionKHR fn =
      (PFN_vkDestroySamplerYcbcrConversionKHR)vkGetDeviceProcAddr(
          device, "vkDestroySamplerYcbcrConversionKHR");
  fn(device, ycbcrConversion, pAllocator);
  return;
}

void shim_vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue)
{
  PFN_vkGetDeviceQueue2 fn = vkGetDeviceQueue2;
  fn(device, pQueueInfo, pQueue);
  return;
}

VkResult shim_vkCreateValidationCacheEXT(VkDevice device,
                                         const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator,
                                         VkValidationCacheEXT *pValidationCache)
{
  PFN_vkCreateValidationCacheEXT fn =
      (PFN_vkCreateValidationCacheEXT)vkGetDeviceProcAddr(device, "vkCreateValidationCacheEXT");
  VkResult r = fn(device, pCreateInfo, pAllocator, pValidationCache);
  return r;
}

void shim_vkDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                      const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyValidationCacheEXT fn =
      (PFN_vkDestroyValidationCacheEXT)vkGetDeviceProcAddr(device, "vkDestroyValidationCacheEXT");
  fn(device, validationCache, pAllocator);
  return;
}

VkResult shim_vkGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                          size_t *pDataSize, void *pData)
{
  PFN_vkGetValidationCacheDataEXT fn =
      (PFN_vkGetValidationCacheDataEXT)vkGetDeviceProcAddr(device, "vkGetValidationCacheDataEXT");
  VkResult r = fn(device, validationCache, pDataSize, pData);
  return r;
}

VkResult shim_vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache,
                                         uint32_t srcCacheCount,
                                         const VkValidationCacheEXT *pSrcCaches)
{
  PFN_vkMergeValidationCachesEXT fn =
      (PFN_vkMergeValidationCachesEXT)vkGetDeviceProcAddr(device, "vkMergeValidationCachesEXT");
  VkResult r = fn(device, dstCache, srcCacheCount, pSrcCaches);
  return r;
}

void shim_vkGetDescriptorSetLayoutSupport(VkDevice device,
                                          const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                          VkDescriptorSetLayoutSupport *pSupport)
{
  PFN_vkGetDescriptorSetLayoutSupport fn = (PFN_vkGetDescriptorSetLayoutSupport)vkGetDeviceProcAddr(
      device, "vkGetDescriptorSetLayoutSupport");
  fn(device, pCreateInfo, pSupport);
  return;
}

void shim_vkGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                             const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                             VkDescriptorSetLayoutSupport *pSupport)
{
  PFN_vkGetDescriptorSetLayoutSupportKHR fn =
      (PFN_vkGetDescriptorSetLayoutSupportKHR)vkGetDeviceProcAddr(
          device, "vkGetDescriptorSetLayoutSupportKHR");
  fn(device, pCreateInfo, pSupport);
  return;
}

VkResult shim_vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline,
                                 VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType,
                                 size_t *pInfoSize, void *pInfo)
{
  PFN_vkGetShaderInfoAMD fn =
      (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(device, "vkGetShaderInfoAMD");
  VkResult r = fn(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
  return r;
}

VkResult shim_vkSetDebugUtilsObjectNameEXT(VkDevice device,
                                           const VkDebugUtilsObjectNameInfoEXT *pNameInfo)
{
  PFN_vkSetDebugUtilsObjectNameEXT fn = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
      aux.instance, "vkSetDebugUtilsObjectNameEXT");
  VkResult r = fn(device, pNameInfo);
  return r;
}

VkResult shim_vkSetDebugUtilsObjectTagEXT(VkDevice device,
                                          const VkDebugUtilsObjectTagInfoEXT *pTagInfo)
{
  PFN_vkSetDebugUtilsObjectTagEXT fn = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(
      aux.instance, "vkSetDebugUtilsObjectTagEXT");
  VkResult r = fn(device, pTagInfo);
  return r;
}

void shim_vkQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo)
{
  PFN_vkQueueBeginDebugUtilsLabelEXT fn = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkQueueBeginDebugUtilsLabelEXT");
  fn(queue, pLabelInfo);
  return;
}

void shim_vkQueueEndDebugUtilsLabelEXT(VkQueue queue)
{
  PFN_vkQueueEndDebugUtilsLabelEXT fn = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkQueueEndDebugUtilsLabelEXT");
  fn(queue);
  return;
}

void shim_vkQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo)
{
  PFN_vkQueueInsertDebugUtilsLabelEXT fn = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkQueueInsertDebugUtilsLabelEXT");
  fn(queue, pLabelInfo);
  return;
}

void shim_vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                       const VkDebugUtilsLabelEXT *pLabelInfo)
{
  PFN_vkCmdBeginDebugUtilsLabelEXT fn = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkCmdBeginDebugUtilsLabelEXT");
  fn(commandBuffer, pLabelInfo);
  return;
}

void shim_vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer)
{
  PFN_vkCmdEndDebugUtilsLabelEXT fn = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkCmdEndDebugUtilsLabelEXT");
  fn(commandBuffer);
  return;
}

void shim_vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                        const VkDebugUtilsLabelEXT *pLabelInfo)
{
  PFN_vkCmdInsertDebugUtilsLabelEXT fn = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(
      aux.instance, "vkCmdInsertDebugUtilsLabelEXT");
  fn(commandBuffer, pLabelInfo);
  return;
}

VkResult shim_vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkDebugUtilsMessengerEXT *pMessenger)
{
  PFN_vkCreateDebugUtilsMessengerEXT fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pMessenger);
  return r;
}

void shim_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                          const VkAllocationCallbacks *pAllocator)
{
  PFN_vkDestroyDebugUtilsMessengerEXT fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  fn(instance, messenger, pAllocator);
  return;
}

void shim_vkSubmitDebugUtilsMessageEXT(VkInstance instance,
                                       VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData)
{
  PFN_vkSubmitDebugUtilsMessageEXT fn = (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(
      instance, "vkSubmitDebugUtilsMessageEXT");
  fn(instance, messageSeverity, messageTypes, pCallbackData);
  return;
}

VkResult shim_vkGetMemoryHostPointerPropertiesEXT(
    VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void *pHostPointer,
    VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties)
{
  PFN_vkGetMemoryHostPointerPropertiesEXT fn =
      (PFN_vkGetMemoryHostPointerPropertiesEXT)vkGetDeviceProcAddr(
          device, "vkGetMemoryHostPointerPropertiesEXT");
  VkResult r = fn(device, handleType, pHostPointer, pMemoryHostPointerProperties);
  return r;
}

void shim_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer,
                                    VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer,
                                    VkDeviceSize dstOffset, uint32_t marker)
{
  PFN_vkCmdWriteBufferMarkerAMD fn =
      (PFN_vkCmdWriteBufferMarkerAMD)vkGetDeviceProcAddr(aux.device, "vkCmdWriteBufferMarkerAMD");
  fn(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
  return;
}
)")},

    /******************************************************************************/
    /* TEMPLATE_FILE_GOLD_REFERENCE_UTILS_H                                       */
    /******************************************************************************/
    {"gold_reference_shim", "utils.h",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: gold_reference_shim/utils.h
//-----------------------------------------------------------------------------
#pragma once
#include <algorithm>
#include <fstream>
#include <map>
#include <string>

#include <assert.h>

#include "helper/helper.h"

// Utility structure that associates VkImageView with parent VkImage
// and their corresponding CreateInfo structures.
struct ImageAndView {
  struct ImageAndCI {
    VkImage res;
    VkImageCreateInfo ci;
  } image;

  struct ImageViewAndCI {
    VkImageView res;
    VkImageViewCreateInfo ci;
  } view;

  ImageAndView() {
    memset(&image, 0, sizeof(image));
    memset(&view, 0, sizeof(view));
  }
  ImageAndView(VkImage i, VkImageCreateInfo ici, VkImageView v, VkImageViewCreateInfo vci) {
    image.res = i;
    image.ci = ici;
    view.res = v;
    view.ci = vci;
  }
};

// Utility structure that keeps track of renderpass attachment
// resources and their layouts based on renderpass'es CreateInfo.
struct RenderPassInfo
{
  VkRenderPass renderPass;
  std::vector<VkImageLayout> finalLayouts;
  std::vector<ImageAndView> attachments;
};

// Utility structure that stores information about an attachment
// readback resources.
struct ReadbackInfo
{
  VkImage srcImage = NULL;
  VkBuffer buffer = NULL;
  VkImage image = NULL;
  VkDeviceMemory bufferDeviceMem = NULL;
  VkDeviceMemory imageDeviceMem = NULL;
  uint32_t width = 0;
  uint32_t height = 0;
  VkFormat format = VK_FORMAT_UNDEFINED;
  int index = -1;

  ReadbackInfo(VkImage src, VkBuffer b, VkImage i,
    VkDeviceMemory bMem, VkDeviceMemory iMem,
    uint32_t w, uint32_t h, VkFormat f,
    int a) : srcImage(src),
    buffer(b), image(i), bufferDeviceMem(bMem),
    imageDeviceMem(iMem), width(w), height(h),
    format(f), index(a) {}

  void Clear(VkDevice device) {
    if (image)
      vkDestroyImage(device, image, NULL);
    if (imageDeviceMem)
      vkFreeMemory(device, imageDeviceMem, NULL);
    if (buffer)
      vkDestroyBuffer(device, buffer, NULL);
    if (bufferDeviceMem)
      vkFreeMemory(device, bufferDeviceMem, NULL);
    width = height = 0;
    srcImage = NULL;
    image = NULL;
    buffer = NULL;
    imageDeviceMem = NULL;
    bufferDeviceMem = NULL;
    index = -1;
    format = VK_FORMAT_UNDEFINED;
  }
};

struct ReadbackInfos {
  std::vector<ReadbackInfo> attachments;
};

VkDeviceMemory getStagingImage(VkImage &image, VkImageCreateInfo ci);
VkDeviceMemory getStagingBuffer(VkBuffer &buffer, uint32_t width, uint32_t height, uint32_t bytes);

void copyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage,
  VkImageSubresourceRange srcRange, VkImageSubresourceRange dstRange, uint32_t width,
  uint32_t height, VkFormat format, bool msaa);
void imgToBuffer(VkCommandBuffer commandBuffer, VkImage image, VkBuffer buffer, uint32_t width,
                 uint32_t height, uint32_t mip, uint32_t layer, VkFormat format);
bool fillPPM(void *buffer, void *input, uint32_t w, uint32_t h, VkFormat format,
             bool isStencil = false);
void bufferToPpm(VkBuffer buffer, VkDeviceMemory mem, std::string filename, uint32_t width,
                 uint32_t height, VkFormat format);
ReadbackInfos copyFramebufferAttachments(VkCommandBuffer cmdBuf, RenderPassInfo *rpInfo);
void screenshot(VkImage srcImage, const char *filename);

extern AuxVkTraceResources aux;
extern VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
extern VkSwapchainCreateInfoKHR swapchainCI;
extern int presentIndex;)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_GOLD_REFERENCE_UTILS_CPP                                     */
    /******************************************************************************/
    {"gold_reference_shim", "utils.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: gold_reference_shim/utils.cpp
//-----------------------------------------------------------------------------
#include <cmath>
#include <fstream>
#include <functional>
#include <map>

#include <float.h>

#include "helper/helper.h"
#include "shim_vulkan.h"
#include "utils.h"

void bufferToPpm(VkBuffer buffer, VkDeviceMemory mem, std::string filename, uint32_t width,
                 uint32_t height, VkFormat format)
{
  void *data;
  vkMapMemory(aux.device, mem, 0, VK_WHOLE_SIZE, 0, &data);

  // raw data
  std::string rawFilename = filename.substr(0, filename.find_last_of("."));
  std::ofstream rawFile(rawFilename, std::ios::out | std::ios::binary);
  rawFile.write((char *)data, width * height * kImageAspectsAndByteSize[format].second);
  rawFile.close();

  switch(format)
  {
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    {
      std::vector<uint8_t> outputDepth(width * height * 3);
      fillPPM(outputDepth.data(), data, width, height, format);

      std::string depthFilename(filename);
      depthFilename.insert(filename.find_last_of("."), "_DEPTH");
      std::ofstream depthFile(depthFilename, std::ios::out | std::ios::binary);
      depthFile << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";
      depthFile.write((char *)outputDepth.data(), width * height * 3);
      depthFile.close();

      std::vector<uint8_t> outputStencil(width * height * 3);
      fillPPM(outputStencil.data(), data, width, height, format, true);

      std::string stencilFilename(filename);
      stencilFilename.insert(filename.find_last_of("."), "_STENCIL");
      std::ofstream stencilFile(stencilFilename, std::ios::out | std::ios::binary);
      stencilFile << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";
      stencilFile.write((char *)outputStencil.data(), width * height * 3);
      stencilFile.close();
    }
    break;
    default:
    {
      std::vector<uint8_t> output(width * height * 3);
      fillPPM(output.data(), data, width, height, format);

      std::ofstream file(filename, std::ios::out | std::ios::binary);
      file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";
      file.write((char *)output.data(), width * height * 3);
      file.close();
    }
    break;
  }

  vkUnmapMemory(aux.device, mem);
}

// imgToBuffer copies the full image to buffer tightly-packed.
// For a depth/stencil format, the first region is depth aspect, and the second is
// stencil aspect.
void imgToBuffer(VkCommandBuffer cmdBuf, VkImage image, VkBuffer buffer, uint32_t width,
                 uint32_t height, uint32_t mip, uint32_t layer, VkFormat format)
{
  switch(format)
  {
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    {
      double depthSizeInBytes = SizeOfFormat(format, VK_IMAGE_ASPECT_DEPTH_BIT);
      VkBufferImageCopy regions[2] = {{}, {}};
      regions[0].imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, mip, layer, 1};
      regions[0].imageExtent = {width, height, 1};
      regions[1].bufferOffset = (VkDeviceSize)(width * height * depthSizeInBytes);
      regions[1].imageSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, mip, layer, 1};
      regions[1].imageExtent = {width, height, 1};
      vkCmdCopyImageToBuffer(cmdBuf, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 2, regions);
    }
    break;
    default:
    {
      VkBufferImageCopy region = {};
      region.imageSubresource = {kImageAspectsAndByteSize[format].first, mip, layer, 1};
      region.imageExtent = {width, height, 1};
      vkCmdCopyImageToBuffer(cmdBuf, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    }
    break;
  }
}

VkDeviceMemory getStagingImage(VkImage &dstImage, VkImageCreateInfo ci)
{
  vkCreateImage(aux.device, &ci, nullptr, &dstImage);

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(aux.device, dstImage, &memRequirements);

  VkMemoryAllocateInfo memAllocInfo{};
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memRequirements.size;
  memAllocInfo.memoryTypeIndex = // try allocating in CPU memory first.
      MemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memRequirements.memoryTypeBits,
                      physicalDeviceMemoryProperties);
  if (memAllocInfo.memoryTypeIndex == -1)
    memAllocInfo.memoryTypeIndex = // if CPU is not possible, try GPU memory
      MemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memRequirements.memoryTypeBits,
                      physicalDeviceMemoryProperties);
  assert(memAllocInfo.memoryTypeIndex != -1);
  VkDeviceMemory dstImageMemory;
  vkAllocateMemory(aux.device, &memAllocInfo, nullptr, &dstImageMemory);

  vkBindImageMemory(aux.device, dstImage, dstImageMemory, 0);
  return dstImageMemory;
}

VkDeviceMemory getStagingBuffer(VkBuffer &dstBuffer, uint32_t width, uint32_t height, uint32_t bytes)
{
  VkBufferCreateInfo bufCI = {};
  bufCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufCI.size = VkDeviceSize(width * height * bytes);
  VkResult result = vkCreateBuffer(aux.device, &bufCI, NULL, &dstBuffer);
  assert(result == VK_SUCCESS);

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(aux.device, dstBuffer, &memRequirements);

  VkMemoryAllocateInfo memAllocInfo{};
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memRequirements.size;
  memAllocInfo.memoryTypeIndex =
      MemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      memRequirements.memoryTypeBits, physicalDeviceMemoryProperties);
  assert(memAllocInfo.memoryTypeIndex != -1);
  VkDeviceMemory dstMemory;
  vkAllocateMemory(aux.device, &memAllocInfo, NULL, &dstMemory);
  vkBindBufferMemory(aux.device, dstBuffer, dstMemory, 0);
  return dstMemory;
}

void copyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage,
               VkImageSubresourceRange srcRange, VkImageSubresourceRange dstRange, uint32_t width,
               uint32_t height, VkFormat format, bool msaa)
{
  assert(sizeof(VkImageCopy) == sizeof(VkImageResolve));
  union {
    VkImageCopy copy; // these structures are actually identical.
    VkImageResolve resolve;
  } region{};

  region.copy.srcSubresource.aspectMask = kImageAspectsAndByteSize[format].first;
  region.copy.srcSubresource.baseArrayLayer = srcRange.baseArrayLayer;
  region.copy.srcSubresource.mipLevel = srcRange.baseMipLevel;
  region.copy.srcSubresource.layerCount = 1;
  region.copy.dstSubresource.aspectMask = kImageAspectsAndByteSize[format].first;
  region.copy.dstSubresource.baseArrayLayer = dstRange.baseArrayLayer;
  region.copy.dstSubresource.mipLevel = dstRange.baseMipLevel;
  region.copy.dstSubresource.layerCount = 1;
  region.copy.extent.width = width;
  region.copy.extent.height = height;
  region.copy.extent.depth = 1;
  if (!msaa)
    vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_GENERAL, dstImage,
      VK_IMAGE_LAYOUT_GENERAL, 1, &region.copy);
  else
    vkCmdResolveImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_GENERAL, dstImage,
      VK_IMAGE_LAYOUT_GENERAL, 1, &region.resolve);
}

// copyFramebufferAttachments copies framebuffer attachments to host visible buffers,
// so they can be saved to disk later and returns the list of ReadbackInfo structures
// that were allocated and that store the attachments data.
ReadbackInfos copyFramebufferAttachments(VkCommandBuffer cmdBuf, RenderPassInfo *rpInfo)
{
  ReadbackInfos readbacks;
  for(int i = 0; i < rpInfo->attachments.size(); i++)
  {
    VkImageCreateInfo image_ci = rpInfo->attachments[i].image.ci;
    VkImageViewCreateInfo view_ci = rpInfo->attachments[i].view.ci;
    if(kImageAspectsAndByteSize.find(image_ci.format) == kImageAspectsAndByteSize.end())
    {
#if defined(_WIN32)
      OutputDebugStringA(std::string("Invalid format " + FormatToString(image_ci.format) + "\n").c_str());
#endif
      continue;
    }

    VkImage srcImage = rpInfo->attachments[i].image.res;
    VkImageSubresourceRange view_subresource = view_ci.subresourceRange;
    assert(view_subresource.layerCount == 1 && view_subresource.levelCount == 1);

    // Source image subresource is defined by it's view. For now assume only one layer is used.
    uint32_t mip = view_subresource.baseMipLevel;
    uint32_t layer = view_subresource.baseArrayLayer;
    uint32_t mip_width = std::max<uint32_t>(image_ci.extent.width >> mip, 1);
    uint32_t mip_height = std::max<uint32_t>(image_ci.extent.height >> mip, 1);

    VkImageSubresourceRange srcRange = { FullAspectFromFormat(image_ci.format),
      mip, 1, layer, 1};

    // Transition source image to VK_IMAGE_LAYOUT_GENERAL.
    ImageLayoutTransition(cmdBuf, srcImage, srcRange, VK_IMAGE_LAYOUT_GENERAL,
      VK_QUEUE_FAMILY_IGNORED, rpInfo->finalLayouts[i], VK_QUEUE_FAMILY_IGNORED);

    bool msaa = image_ci.samples != VK_SAMPLE_COUNT_1_BIT;
    VkImage stagingImage = NULL;
    VkDeviceMemory stagingImageMem = NULL;
    if (msaa) { // If MSAA we'll do an image->image resolve first.
      VkImageCreateInfo stagingCI{};
      stagingCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      stagingCI.imageType = VK_IMAGE_TYPE_2D;
      stagingCI.format = image_ci.format;
      stagingCI.extent.width = mip_width;
      stagingCI.extent.height = mip_height;
      stagingCI.extent.depth = 1;
      stagingCI.arrayLayers = 1;
      stagingCI.mipLevels = 1;
      stagingCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      stagingCI.samples = VK_SAMPLE_COUNT_1_BIT;
      stagingCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      stagingCI.tiling = VK_IMAGE_TILING_OPTIMAL;
      stagingCI.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
      stagingImageMem = getStagingImage(stagingImage, stagingCI);

      // Transition staging image to VK_IMAGE_LAYOUT_GENERAL.
      VkImageSubresourceRange dstRange = {FullAspectFromFormat(stagingCI.format),
        0, 1, 0, 1};

      ImageLayoutTransition(cmdBuf, stagingImage, dstRange,
        VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_IGNORED);

      copyImage(cmdBuf, srcImage, stagingImage, srcRange, dstRange, mip_width, mip_height, stagingCI.format, msaa);

      // Because srcImage is VK_IMAGE_LAYOUT_GENERAL we can just keep it like this for subsequent copy.
      // override these arguments since now we'll send the stagingImage down to imgToBuffer() function
      srcImage = stagingImage;
      mip = layer = 0;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory bufMem = getStagingBuffer(stagingBuffer, mip_width, mip_height,
      kImageAspectsAndByteSize[image_ci.format].second);
    imgToBuffer(cmdBuf, srcImage, stagingBuffer, mip_width, mip_height, mip, layer, image_ci.format);

    // Transition the real source image back to the original layout.
    ImageLayoutTransition(cmdBuf, rpInfo->attachments[i].image.res, srcRange, rpInfo->finalLayouts[i],
      VK_QUEUE_FAMILY_IGNORED, VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED);

    ReadbackInfo readback(rpInfo->attachments[i].image.res,
                          stagingBuffer, stagingImage, bufMem, stagingImageMem,
                          mip_width, mip_height, image_ci.format, i);
    readbacks.attachments.push_back(readback);
  }

  return readbacks;
}

void screenshot(VkImage srcImage, const char *filename)
{
  uint32_t sw_width = swapchainCI.imageExtent.width;
  uint32_t sw_height = swapchainCI.imageExtent.height;
  VkFormat sw_format = swapchainCI.imageFormat;

  VkImageCreateInfo imageCreateCI{};
  imageCreateCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
  imageCreateCI.format = sw_format;
  imageCreateCI.extent.width = sw_width;
  imageCreateCI.extent.height = sw_height;
  imageCreateCI.extent.depth = 1;
  imageCreateCI.arrayLayers = 1;
  imageCreateCI.mipLevels = 1;
  imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMem = getStagingBuffer(stagingBuffer, sw_width, sw_height,
    kImageAspectsAndByteSize[sw_format].second);

  VkCommandBufferBeginInfo cmdbufBI{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  vkBeginCommandBuffer(aux.command_buffer, &cmdbufBI);
  {
    VkImageSubresourceRange fullRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    ImageLayoutTransition(aux, srcImage, fullRange,
      VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    imgToBuffer(aux.command_buffer, srcImage, stagingBuffer, sw_width, sw_height, 0, 0, sw_format);

    ImageLayoutTransition(aux, srcImage, fullRange,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_GENERAL);
  }
  vkEndCommandBuffer(aux.command_buffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &aux.command_buffer;
  vkQueueSubmit(aux.queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(aux.queue);

  bufferToPpm(stagingBuffer, stagingBufferMem, filename, sw_width, sw_height, sw_format);

  // cleanup
  vkDestroyBuffer(aux.device, stagingBuffer, NULL);
  vkFreeMemory(aux.device, stagingBufferMem, NULL);
})"},

    /******************************************************************************/
    /* TEMPLATE_FILE_GOLD_REFERENCE_UTILS_CPP                                        */
    /******************************************************************************/
    {"gold_reference_shim", "format_conversion.cpp",
     R"(//-----------------------------------------------------------------------------
// Generated with RenderDoc CPP Code Generator
// File: gold_reference_shim/format_conversion.cpp
//-----------------------------------------------------------------------------
#include "utils.h"

#include <array>
#include <limits>

#include "float.h"

float SFLOAT16ToSFLOAT32(uint32_t bits) {
  uint32_t M = bits & 0x3ff;
  uint32_t E = (bits >> 10) & 0x1f;
  uint32_t S = (bits >> 15) & 1;
  float sign = 1;
  if (S > 0)
    sign = -1;
  if (E == 0 && M == 0)
    return sign * 0.0f;
  else if (E == 0 && M != 0)
    return sign * float(M) / 1024.0f / 16384.0f;
  else if (E == 31 && M == 0)
    return sign * std::numeric_limits<float>::infinity();
  else if (E == 31 && M != 0) {
#if defined(_WIN32)
    OutputDebugStringA("NaN \n");
#endif
    return 0;
  } else if (E <= 15)
    return sign * (1 + float(M) / 1024.0f) / float(uint32_t(1 << (15 - E)));
  else
    return sign * (1 + float(M) / 1024.0f) * float(uint32_t(1 << (E - 15)));
}

struct half {
  uint16_t value;

  half(float v) {}

  operator float() {
    return SFLOAT16ToSFLOAT32(value);
  }

  operator double() {
    return SFLOAT16ToSFLOAT32(value);
  }
};

#pragma pack(push)
#pragma pack(1)

struct depth24 {
  uint8_t value[3];

  depth24(float v) {}

  operator float() {
    uint32_t v = 0;
    v |= ((uint32_t) value[0]) << 16;
    v |= ((uint32_t) value[1]) << 8;
    v |= ((uint32_t) value[2]);
    return 1.0f * v / 0x00FFFFFF;
  }

  operator double() {
    uint32_t v = 0;
    v |= ((uint32_t) value[0]) << 16;
    v |= ((uint32_t) value[1]) << 8;
    v |= ((uint32_t) value[2]);
    return 1.0 * v / 0x00FFFFFF;
  }
};

struct rgb8 {
  uint8_t value[3];

  template<typename T>
  rgb8(const T& lum) {
    value[0] = uint8_t(lum);
    value[1] = uint8_t(lum);
    value[2] = uint8_t(lum);
  }
};
#pragma pack(pop)


uint8_t SFLOAT32ToUint8(float f) {
  if (f < 0.0f) {
    return 0;
  }
  if (f > 1.0f) {
    return 255;
  }
  return uint8_t(f * 255);
}

uint8_t SRGBToUint8(uint8_t x) {
  float f = x / 255.0f;
  if (f <= 0.0f)
    return 0;
  else if (f >= 1.0f)
    return 255;
  else if (f < 0.04045f)
    return SFLOAT32ToUint8(f / 12.92f);
  else
    return SFLOAT32ToUint8(std::pow((f + 0.055f) / 1.055f, 2.4f));

  if (f < 0.0f) {
    return 0;
  }
  if (f > 1.0f) {
    return 255;
  }
  return uint8_t(f * 255);
}

uint8_t UFLOAT11ToUint8(uint32_t x) {
  return SFLOAT32ToUint8(SFLOAT16ToSFLOAT32((x & 0x7FF) << 4));
}

uint8_t UFLOAT10ToUint8(uint32_t x) {
  return SFLOAT32ToUint8(SFLOAT16ToSFLOAT32((x & 0x7FF) << 5));
}

template <class T>
bool isNormal(T v) {
  return true;
}

template <> bool isNormal<float>(float v) {
  return std::isnormal(v) && v < FLT_MAX && v > -FLT_MAX;
}

template <> bool isNormal<double>(double v) {
  return std::isnormal(v) && v < FLT_MAX && v > -FLT_MAX;
}

template <class T, typename LargeType, typename IntType>
void histogramEqualization(T * data, uint32_t w, uint32_t h, uint32_t channels,
  LargeType absMin, LargeType absMax) {
  std::vector<std::array<uint32_t, 256>> bins(channels);

  std::vector<LargeType> maxV(channels);
  std::vector<LargeType> minV(channels);
  for (uint32_t c = 0; c < channels; c++) {
    maxV[c] = absMin;
    minV[c] = absMax;
  }

  for (uint32_t j = 0; j < h; j++) {
    for (uint32_t i = 0; i < w; i++) {
      for (uint32_t c = 0; c < channels; c++) {
        LargeType v = (LargeType) data[(i + j * w) * channels + c];
        if (!(isNormal(v) || v == 0) || v > absMax || v < absMin)
          continue; // skip inf and nan values
        maxV[c] = maxV[c] > v ? maxV[c] : v;
        minV[c] = minV[c] < v ? minV[c] : v;
      }
    }
  }

  std::vector<LargeType> denom(channels);
  for (uint32_t c = 0; c < channels; c++) {
    denom[c] = maxV[c] - minV[c];
    if (abs((long double) denom[c]) < FLT_EPSILON)
      denom[c] = LargeType(1.0);
  }

  for (uint32_t j = 0; j < h; j++) {
    for (uint32_t i = 0; i < w; i++) {
      for (uint32_t c = 0; c < channels; c++) {
        LargeType v = data[(i + j * w) * channels + c];
        if (!(isNormal(v) || v == 0) || v > (absMax) || v < absMin) {
          continue;
        }
        uint32_t bin = uint32_t(255 * ((v - minV[c]) / denom[c]));
        bins[c][bin]++;
      }
    }
  }

  std::vector<std::array<double, 256>> probability(channels);
  for (uint32_t c = 0; c < channels; c++) {
    probability[c][0] = bins[c][0];
    for (uint32_t i = 1; i < 256; i++) {
      probability[c][i] = probability[c][i - 1] + bins[c][i];
    }
    for (uint32_t i = 0; i < 256; i++) {
      probability[c][i] /= double(probability[c][255]);
    }
  }

  IntType *ptr = (IntType *) data;
  for (uint32_t j = 0; j < h; j++) {
    for (uint32_t i = 0; i < w; i++) {
      for (uint32_t c = 0; c < channels; c++) {
        LargeType v = data[(i + j * w) * channels + c];
        if (!(isNormal(v) || v == 0) || v > absMax || v < absMin) {
          if (v > absMax)
            v = maxV[c];
          if (v < absMin)
            v = minV[c];
        }
        uint32_t bin = uint32_t(255 * (v - minV[c]) / denom[c]);
        ptr[(i + j * w) * channels + c] = IntType(255 * probability[c][bin]);
      }
    }
  }
}

#define HISTOGRAM_EQ_SWITCH_FP(bits, channels, minV, maxV) switch (bits) {\
    case 16: histogramEqualization<half, float, uint16_t>((half *) p, w, h, channels, minV, maxV); break; \
    case 24: histogramEqualization<depth24, float, rgb8>((depth24 *) p, w, h, channels, minV, maxV); break; \
    case 32: histogramEqualization<float, float, uint32_t>((float *) p, w, h, channels, minV, maxV); break; \
    case 64: histogramEqualization<double, double, uint64_t>((double *) p, w, h, channels, minV, maxV); break; \
    default: assert(0);                                                           \
  }

#define HISTOGRAM_EQ_SWITCH_UNSIGNED(bits, channels) switch (bits) {\
    case 8: histogramEqualization<uint8_t, uint8_t, uint8_t>((uint8_t *) p, w, h, channels, 0, UINT8_MAX); break; \
    case 16: histogramEqualization<uint16_t, uint16_t, uint16_t>((uint16_t *) p, w, h, channels, 0, UINT16_MAX); break; \
    case 32: histogramEqualization<uint32_t, uint32_t, uint32_t>((uint32_t *) p, w, h, channels, 0, UINT32_MAX); break; \
    case 64: histogramEqualization<uint64_t, uint64_t, uint64_t>((uint64_t *) p, w, h, channels, 0, UINT64_MAX); break; \
    default: assert(0);                                                           \
  }

#define HISTOGRAM_EQ_SWITCH_SIGNED(bits, channels) switch (bits) {\
    case 16: histogramEqualization<int16_t, int16_t, int16_t>((int16_t *) p, w, h, channels, INT16_MIN, INT16_MAX); break; \
    case 32: histogramEqualization<int32_t, int32_t, int32_t>((int32_t *) p, w, h, channels, INT32_MIN, INT32_MAX); break; \
    case 64: histogramEqualization<int64_t, int64_t, int64_t>((int64_t *) p, w, h, channels, INT64_MIN, INT64_MAX); break; \
    default: assert(0);                                                           \
  }

bool fillPPM(void *output, void *input, uint32_t w, uint32_t h, VkFormat format, bool isStencil) {
  uint8_t *out = (uint8_t *) output;
  uint8_t *p = (uint8_t *) input;
  uint32_t size_in_bytes = 0;
  uint32_t channels = ChannelsInFormat(format);
  uint32_t bits = BitsPerChannelInFormat(format);
  bool isFP = IsFPFormat(format);
  bool isHDR = isHDRFormat(format);
  bool isSigned = IsSignedFormat(format);
  bool isDepth = IsDepthFormat(format);
  if (!isStencil) {
    size_in_bytes = uint32_t(SizeOfFormat(format, VK_IMAGE_ASPECT_COLOR_BIT));
    if (isHDR) {
      if (isFP) {
        if (isDepth) {
          HISTOGRAM_EQ_SWITCH_FP(bits, channels, 0.0, 1.0);
        } else {
          HISTOGRAM_EQ_SWITCH_FP(bits, channels, -65505.0, 65505.0);
        }
      } else {
        if (isSigned) {
          HISTOGRAM_EQ_SWITCH_SIGNED(bits, channels);
        } else {
          HISTOGRAM_EQ_SWITCH_UNSIGNED(bits, channels);
        }
      }
    }
  } else {
    size_in_bytes = uint32_t(SizeOfFormat(format, VK_IMAGE_ASPECT_DEPTH_BIT));
    p += size_in_bytes * w * h; // skip depth bytes
    HISTOGRAM_EQ_SWITCH_UNSIGNED(8, 1);
    size_in_bytes = uint32_t(SizeOfFormat(format, VK_IMAGE_ASPECT_STENCIL_BIT));
  }

#undef HISTOGRAM_EQ_SWITCH_FP
#undef HISTOGRAM_EQ_SWITCH_SIGNED
#undef HISTOGRAM_EQ_SWITCH_UNSIGNED

  for (uint32_t r = 0; r < h; ++r) {
    for (uint32_t c = 0; c < w; ++c) {
      switch (format) {
        case VK_FORMAT_R4G4_UNORM_PACK8:
        {
          uint8_t v = *p;
          *out++ = ((v >> 4) & 0x0f) * 16;       // R
          *out++ = (v & 0x0f) * 16;              // G
          *out++ = 0;                            // B
        }
        break;

        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 12) & 0x000f) * 16;    // R
          *out++ = ((v >> 8) & 0x000f) * 16;     // G
          *out++ = ((v >> 4) & 0x000f) * 16;     // B
        }
        break;

        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 4) & 0x000f) * 16;     // R
          *out++ = ((v >> 8) & 0x000f) * 16;     // G
          *out++ = ((v >> 12) & 0x000f) * 16;    // B
        }
        break;

        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 11) & 0x001f) * 8;     // R
          *out++ = ((v >> 5) & 0x003f) * 4;      // G
          *out++ = (v & 0x001f) * 8;             // B
        }
        break;

        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = (v & 0x001f) * 8;             // R
          *out++ = ((v >> 5) & 0x003f) * 4;      // G
          *out++ = ((v >> 11) & 0x001f) * 8;     // B
        }
        break;

        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 11) & 0x001f) * 8;     // R
          *out++ = ((v >> 6) & 0x001f) * 8;      // G
          *out++ = ((v >> 1) & 0x001f) * 8;      // B
        }
        break;

        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 1) & 0x001f) * 8;      // R
          *out++ = ((v >> 6) & 0x001f) * 8;      // G
          *out++ = ((v >> 11) & 0x001f) * 8;     // B
        }
        break;

        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 10) & 0x001f) * 8;     // R
          *out++ = ((v >> 5) & 0x001f) * 8;      // G
          *out++ = (v & 0x001f) * 8;             // B
        }
        break;

        // All 8 bit unsigned formats can be copied without modifications.
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        // All 8 bit signed formats can be copied but we need to flip the sign bit.
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_SINT:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = isSigned ? p[c] ^ 0x80 : p[c];
          for (uint32_t c = channels; c < 3; c++) // alpha is dropped
            *out++ = 0;
        }
        break;

        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_R8G8B8A8_SRGB:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = SRGBToUint8(*(p + c));
          for (uint32_t c = channels; c < 3; c++) // alpha is dropped
            *out++ = 0;
        }
        break;

        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_SINT:
        {
          *out++ = isSigned ? p[2] ^ 0x80 : p[2];    // R
          *out++ = isSigned ? p[1] ^ 0x80 : p[1];    // G
          *out++ = isSigned ? p[0] ^ 0x80 : p[0];    // B
        }
        break;

        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_B8G8R8A8_SRGB:
        {
          *out++ = SRGBToUint8(p[2]);    // R
          *out++ = SRGBToUint8(p[1]);    // G
          *out++ = SRGBToUint8(p[0]);    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = v & 0xff;            // R
          *out++ = (v >> 8) & 0xff;     // G
          *out++ = (v >> 16) & 0xff;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0xff) ^ 0x80;            // R
          *out++ = ((v >> 8) & 0xff) ^ 0x80;     // G
          *out++ = ((v >> 16) & 0xff) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = v & 0xff;            // R
          *out++ = (v >> 8) & 0xff;     // G
          *out++ = (v >> 16) & 0xff;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0xff) ^ 0x80;            // R
          *out++ = ((v >> 8) & 0xff) ^ 0x80;     // G
          *out++ = ((v >> 16) & 0xff) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = v & 0xff;            // R
          *out++ = (v >> 8) & 0xff;     // G
          *out++ = (v >> 16) & 0xff;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0xff) ^ 0x80;            // R
          *out++ = ((v >> 8) & 0xff) ^ 0x80;     // G
          *out++ = ((v >> 16) & 0xff) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = SRGBToUint8(v & 0xff);            // R
          *out++ = SRGBToUint8((v >> 8) & 0xff);     // G
          *out++ = SRGBToUint8((v >> 16) & 0xff);    // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v >> 20) & 0x3ff) / 4;    // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = (v & 0x3ff) / 4;            // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v >> 20) & 0x3ff) / 4;    // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = (v & 0x3ff) / 4;            // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // B
        }
        break;
)" + std::string(R"(
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v >> 20) & 0x3ff) / 4;    // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = (v & 0x3ff) / 4;            // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0x3ff) / 4;            // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = ((v >> 20) & 0x3ff) / 4;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0x3ff) / 4;            // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = ((v >> 20) & 0x3ff) / 4;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0x3ff) / 4;            // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = ((v >> 20) & 0x3ff) / 4;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = isSigned ? (((uint16_t*) p)[c] / 256) ^ 0x80 : ((uint16_t*) p)[c] / 256;
          for (uint32_t c = channels; c < 3; c++)
            *out++ = 0;
        }
        break;

        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = ((uint16_t*) p)[c] & 0xFF;
          for (uint32_t c = channels; c < 3; c++)
            *out++ = 0;
        }
        break;

        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = ((uint32_t*) p)[c] & 0xFF;
          for (uint32_t c = channels; c < 3; c++)
            *out++ = 0;
        }
        break;

        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = ((uint64_t*) p)[c] & 0xFF;
          for (uint32_t c = channels; c < 3; c++)
            *out++ = 0;
        }
        break;

        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = UFLOAT11ToUint8(v & 0x7ff);            // R
          *out++ = UFLOAT11ToUint8((v >> 11) & 0x7ff);    // G
          *out++ = UFLOAT10ToUint8((v >> 22) & 0x3ff);    // B
        }
        break;

        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        {
          if (isStencil) {
            *out++ = p[0];    // R
            *out++ = p[0];    // G
            *out++ = p[0];    // B
          } else {
            *out++ = ((uint16_t *) p)[0] & 0xFF;    // R
            *out++ = ((uint16_t *) p)[0] & 0xFF;    // G
            *out++ = ((uint16_t *) p)[0] & 0xFF;    // B
          }
        }
        break;

        case VK_FORMAT_D24_UNORM_S8_UINT:
        {
          if (isStencil) {
            *out++ = p[0];    // R
            *out++ = p[0];    // G
            *out++ = p[0];    // B
          } else {
            *out++ = p[0];
            *out++ = p[1];
            *out++ = p[2];
          }
        }
        break;

        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        {
          if (isStencil) {
            *out++ = p[0];    // R
            *out++ = p[0];    // G
            *out++ = p[0];    // B
          } else {
            *out++ = ((uint32_t *) p)[0] & 0xFF;    // R
            *out++ = ((uint32_t *) p)[0] & 0xFF;    // G
            *out++ = ((uint32_t *) p)[0] & 0xFF;    // B
          }
        }
        break;

        default: return false;
      }
      p += size_in_bytes;
    }
  }

  return true;
})")},

    /******************************************************************************/
    /* TEMPLATE_FILE_GOLD_REFERENCE_SHIM_CMAKE                                       */
    /******************************************************************************/
    {"gold_reference_shim", "CMakeLists.txt",
     R"(SET (THIS_PROJECT_NAME gold_reference)
PROJECT(${THIS_PROJECT_NAME})

ADD_LIBRARY(${THIS_PROJECT_NAME} SHARED "shim_vulkan.h" "shim_vulkan.cpp" "utils.h" "utils.cpp" "format_conversion.cpp")

TARGET_COMPILE_DEFINITIONS(${THIS_PROJECT_NAME} PRIVATE
                           UNICODE _UNICODE)
IF (NOT WIN32)
  SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                        CXX_VISIBILITY_PRESET hidden)
ENDIF ()

TARGET_LINK_LIBRARIES(${THIS_PROJECT_NAME}
                      vulkan
                      helper)

SET_TARGET_PROPERTIES(${THIS_PROJECT_NAME} PROPERTIES
                      OUTPUT_NAME shim_vulkan
                      ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}"
                      LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${THIS_PROJECT_NAME}")

ADD_CUSTOM_COMMAND(TARGET ${THIS_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${THIS_PROJECT_NAME}> ${WORKING_DIRECTORY_DEBUG}sample_cpp_trace)
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_GEN_SCRIPT_WIN_VS                                            */
    /******************************************************************************/
    {"", "build_vs2015.bat",
     R"(rd /s /q Win_VS2015x64
mkdir Win_VS2015x64
rd /s /q build_x64
mkdir build_x64\Debug
mkdir build_x64\Release
cmake.exe -Wno-dev -G "Visual Studio 14 2015 Win64" --build "" -H. -BWin_VS2015x64
pause
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_GEN_SCRIPT_WIN_NINJA                                         */
    /******************************************************************************/
    {"", "build_vs2015_ninja.bat",
     R"(rd /s /q Win_Ninja
mkdir Win_Ninja
rd /s /q build_x64
mkdir build_x64\Debug
mkdir build_x64\Release
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
cmake.exe -Wno-dev -G Ninja --build "" -H. -BWin_Ninja -DCMAKE_BUILD_TYPE=Release
cd Win_Ninja
ninja
cd .. /
pause
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_GEN_SCRIPT_YETI                                              */
    /******************************************************************************/
    {"", "build_yeti.bat",
     R"(which cmake && which ninja && rm -rf yeti_build && mkdir yeti_build && cd yeti_build && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_YETI=ON -DCMAKE_TOOLCHAIN_FILE="$YETI_SDK_PATH/cmake/yeti.cmake" .. && ninja && echo "Build complete"
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_GEN_SCRIPT_LINUX                                             */
    /******************************************************************************/
    {"", "build_xlib.bat",
     R"(which cmake && which ninja && export CC=clang && export CXX=clang++ && rm -rf linux_build && mkdir linux_build && cd linux_build && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && ninja && echo "Build complete"
)"},

    /******************************************************************************/
    /* TEMPLATE_FILE_VS_USER_TEMPLATE                                             */
    /******************************************************************************/
    {"", "Template.user",
     R"(<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|@USERFILE_PLATFORM@'">
    <LocalDebuggerCommandArguments></LocalDebuggerCommandArguments>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    <LocalDebuggerWorkingDirectory>@USERFILE_WORKING_DIRECTORY_DEBUG@</LocalDebuggerWorkingDirectory>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|@USERFILE_PLATFORM@'">
    <LocalDebuggerCommandArguments></LocalDebuggerCommandArguments>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|@USERFILE_PLATFORM@'">
    <LocalDebuggerWorkingDirectory>@USERFILE_WORKING_DIRECTORY_RELEASE@</LocalDebuggerWorkingDirectory>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
</Project>
)"}};    // CodeWriter::TemplateFiles

}    // namespace vk_cpp_codec
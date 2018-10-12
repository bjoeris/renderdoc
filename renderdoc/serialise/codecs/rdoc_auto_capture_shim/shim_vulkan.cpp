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
#ifndef SHIM_VK_COMPILE_STATIC_LIB
#define SHIM_VK_EXPORT
#endif
#include <chrono>

#include "helper/helper.h"
#include "shim_vulkan.h"

const char RDOC_ENV_VAR[] = "RDOC_GOLD_FRAME_INDEX";    // env variable for to-be-captured frame.
const int kDefaultCaptureFrame = 5;    // default frame index if RDOC_GOLD_FRAME_INDEX is not set.
int captureFrame = kDefaultCaptureFrame;
int presentIndex = 0;
bool IsTargetFrame = true;    // default value doesn't matter. It's properly set in CreateInstance.
bool shouldQuit = false;
std::chrono::time_point<std::chrono::system_clock> quitAfter;

#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "renderdoc_app.h"

bool ShimShouldQuitNow()
{
  return shouldQuit && std::chrono::system_clock::now() > quitAfter;
}

typedef struct Versions
{
  union
  {
    // APIs 1_0_x are actually equal to 1_1_0
    RENDERDOC_API_1_1_0 _110;
    RENDERDOC_API_1_1_1 _111;
  };
} Versions;

typedef struct RDOCapture
{
  RENDERDOC_Version version;
  Versions *api;
  bool loaded;
  int capture_frame;
  int frame;
} RDOCapture;
RDOCapture capture;

#if defined(_WIN32) || defined(WIN32)
#define FetchRDocAPI(handle) GetProcAddress(handle, "RENDERDOC_GetAPI")
#elif defined(__linux__)
#define FetchRDocAPI(handle) dlsym(handle, "RENDERDOC_GetAPI")
#endif

#if defined(_WIN32) || defined(WIN32)
#define CloseRDocAPI(handle) FreeLibrary(handle)
#elif defined(__linux__)
#define CloseRDocAPI(handle) dlclose(handle)
#endif

void IsRenderDocLoaded()
{
  capture.api = NULL;
  capture.loaded = false;
  capture.capture_frame = 10;
  capture.frame = 0;
#if defined(_WIN32) || defined(WIN32)
  HMODULE rdoc = GetModuleHandle(L"renderdoc.dll");
  capture.loaded = (rdoc != NULL);
#elif defined(__linux)
  const char *renderdoc = "librenderdoc.so";
  std::string ld_preload = GetEnvString("LD_PRELOAD");
  printf("$LD_PRELOAD is %s\n", ld_preload.c_str());
  void *rdoc = dlopen(renderdoc, RTLD_NOLOAD);
  const char *dl_error = dlerror();
  if(dl_error != NULL)
    printf("RenderDoc dlopen error: %s\n", dl_error);
  if(rdoc == NULL)
  {
    capture.loaded = (ld_preload.find(renderdoc) != std::string::npos);
  }
  else
  {
    capture.loaded = true;
  }
#endif

  pRENDERDOC_GetAPI rdocGetAPI = (pRENDERDOC_GetAPI)FetchRDocAPI(rdoc);
  if(rdocGetAPI != NULL)
  {
    if(rdocGetAPI(eRENDERDOC_API_Version_1_1_1, (void **)&capture.api) != 1)
      if(rdocGetAPI(eRENDERDOC_API_Version_1_1_0, (void **)&capture.api) != 1)
        if(rdocGetAPI(eRENDERDOC_API_Version_1_0_2, (void **)&capture.api) != 1)
          if(rdocGetAPI(eRENDERDOC_API_Version_1_0_1, (void **)&capture.api) != 1)
            if(rdocGetAPI(eRENDERDOC_API_Version_1_0_0, (void **)&capture.api) != 1)
            {
              printf("RenderDoc version isn't valid");
              capture.loaded = false;
              return;
            }
  }
  else
  {
    capture.loaded = false;
    return;
  }
  int major, minor, patch;
  capture.api->_110.GetAPIVersion(&major, &minor, &patch);
  printf("RenderDoc version is (%d %d %d)\n", major, minor, patch);
  capture.api->_110.SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, 1);
  capture.api->_110.SetCaptureOptionU32(eRENDERDOC_Option_CaptureCallstacks, 1);
  capture.api->_110.SetCaptureOptionU32(eRENDERDOC_Option_HookIntoChildren, 1);

#if defined(__yeti__)
  capture.api->_110.SetLogFilePathTemplate("/var/game/");
#elif defined(__linux__)
  capture.api->_110.SetLogFilePathTemplate(".");
#elif defined(_WIN32) || defined(WIN32)
  capture.api->_110.SetLogFilePathTemplate("");
#endif

  if(rdoc != NULL)
    CloseRDocAPI(rdoc);
}
void DelayedCapture()
{
  if(capture.loaded && capture.api != NULL)
    capture.api->_110.TriggerCapture();
}
void DelayedCaptureCheck()
{
  if(capture.loaded && capture.api != NULL)
  {
    if(capture.api->_110.IsFrameCapturing())
    {
      printf("RenderDoc is capturing correctly");
    }
    else
    {
      printf("RenderDoc capturing failed");
    }
    shouldQuit = true;
    quitAfter = std::chrono::system_clock::now() + std::chrono::seconds(30);
  }
}

VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance,
                               const char *handleName)
{
  captureFrame = GetEnvInt(RDOC_ENV_VAR, kDefaultCaptureFrame);

  IsRenderDocLoaded();

  VkResult r = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
  assert(r == VK_SUCCESS);
  AddResourceName((uint64_t)*pInstance, "VkInstance", handleName);
  aux.instance = *pInstance;
  return r;
}

VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                             const char *handleName)
{
  VkResult r = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
  assert(r == VK_SUCCESS);
  AddResourceName((uint64_t)*pDevice, "VkDevice", handleName);
  InitializeAuxResources(&aux, aux.instance, physicalDevice, *pDevice);
  return r;
}

VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
  if(IsTargetFrame)
  {
    DelayedCaptureCheck();
  }

  IsTargetFrame = (++presentIndex == captureFrame);

  if(IsTargetFrame)
  {
    DelayedCapture();
  }

  static PFN_vkQueuePresentKHR fn =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
  return fn(queue, pPresentInfo);
}

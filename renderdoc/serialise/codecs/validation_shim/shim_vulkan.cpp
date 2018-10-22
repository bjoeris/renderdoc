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
#include <fstream>
#include <sstream>

#include "helper/helper.h"

#include "shim_vulkan.h"

VkDebugReportFlagBitsEXT debugReportFlags =
    VkDebugReportFlagBitsEXT(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);
/* Register the callback */
VkDebugReportCallbackEXT callback;
#if defined(__yeti__)
std::ofstream file("/var/game/validation_layer_output.txt");
#else
std::ofstream file("validation_layer_output.txt");
#endif

bool quitNow = false;
bool ShimShouldQuitNow()
{
  return quitNow;
}

void ShimRelease()
{
  PFN_vkDestroyDebugReportCallbackEXT fn = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
      aux.instance, "vkDestroyDebugReportCallbackEXT");
  if(fn && aux.callback != VK_NULL_HANDLE)
    fn(aux.instance, aux.callback, NULL);
  if(fn && callback != VK_NULL_HANDLE)
    fn(aux.instance, callback, NULL);
}

inline std::string VkObjectTypeToString(VkDebugReportObjectTypeEXT ty)
{
  switch(ty)
  {
#define RETURN_OBJECT_TYPE_STRING(t) \
  case VK_DEBUG_REPORT_OBJECT_TYPE_##t##_EXT: return #t
    RETURN_OBJECT_TYPE_STRING(INSTANCE);
    RETURN_OBJECT_TYPE_STRING(PHYSICAL_DEVICE);
    RETURN_OBJECT_TYPE_STRING(DEVICE);
    RETURN_OBJECT_TYPE_STRING(QUEUE);
    RETURN_OBJECT_TYPE_STRING(SEMAPHORE);
    RETURN_OBJECT_TYPE_STRING(COMMAND_BUFFER);
    RETURN_OBJECT_TYPE_STRING(FENCE);
    RETURN_OBJECT_TYPE_STRING(DEVICE_MEMORY);
    RETURN_OBJECT_TYPE_STRING(BUFFER);
    RETURN_OBJECT_TYPE_STRING(IMAGE);
    RETURN_OBJECT_TYPE_STRING(EVENT);
    RETURN_OBJECT_TYPE_STRING(QUERY_POOL);
    RETURN_OBJECT_TYPE_STRING(BUFFER_VIEW);
    RETURN_OBJECT_TYPE_STRING(IMAGE_VIEW);
    RETURN_OBJECT_TYPE_STRING(SHADER_MODULE);
    RETURN_OBJECT_TYPE_STRING(PIPELINE_CACHE);
    RETURN_OBJECT_TYPE_STRING(PIPELINE_LAYOUT);
    RETURN_OBJECT_TYPE_STRING(RENDER_PASS);
    RETURN_OBJECT_TYPE_STRING(PIPELINE);
    RETURN_OBJECT_TYPE_STRING(DESCRIPTOR_SET_LAYOUT);
    RETURN_OBJECT_TYPE_STRING(SAMPLER);
    RETURN_OBJECT_TYPE_STRING(DESCRIPTOR_POOL);
    RETURN_OBJECT_TYPE_STRING(DESCRIPTOR_SET);
    RETURN_OBJECT_TYPE_STRING(FRAMEBUFFER);
    RETURN_OBJECT_TYPE_STRING(COMMAND_POOL);
    RETURN_OBJECT_TYPE_STRING(SURFACE_KHR);
    RETURN_OBJECT_TYPE_STRING(SWAPCHAIN_KHR);
    RETURN_OBJECT_TYPE_STRING(DEBUG_REPORT);
    RETURN_OBJECT_TYPE_STRING(DISPLAY_KHR);
    RETURN_OBJECT_TYPE_STRING(DISPLAY_MODE_KHR);
    RETURN_OBJECT_TYPE_STRING(OBJECT_TABLE_NVX);
    RETURN_OBJECT_TYPE_STRING(INDIRECT_COMMANDS_LAYOUT_NVX);
#undef RETURN_OBJECT_TYPE_STRING
    default: return "UNKNOWN";
  }
}

VkBool32 VKAPI_PTR validationDebugReportCallback(VkDebugReportFlagsEXT flags,
                                                 VkDebugReportObjectTypeEXT objectType,
                                                 uint64_t object, size_t location,
                                                 int32_t messageCode, const char *pLayerPrefix,
                                                 const char *pMessage, void *pUserData)
{
  std::string prefix;
  if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
  {
    prefix += "ERROR: ";
  };
  if(flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
  {
    prefix += "WARNING: ";
  };
  if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
  {
    prefix += "PERFORMANCE: ";
  };
  if(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
  {
    prefix += "INFO: ";
  }
  if(flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
  {
    prefix += "DEBUG: ";
  }

  std::stringstream validationErrorStr;
  validationErrorStr << prefix << "ObjectType [" << VkObjectTypeToString(objectType) << "] Code "
                     << messageCode << ": \n\t" << pMessage << "\n";
  file << validationErrorStr.rdbuf();
  return VK_FALSE;
}

// shim_vkCreateInstance registers a debug report callback that writes validation layer output to a
// file.
VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance,
                               const char *handleName)
{
#if defined(_DEBUG) || defined(DEBUG)
  VkResult r = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
#else
  VkInstanceCreateInfo *instanceCI = const_cast<VkInstanceCreateInfo *>(pCreateInfo);
  instanceCI->enabledLayerCount += 1;
  VkResult r = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
#endif
  assert(r == VK_SUCCESS);
  AddResourceName(ResourceNames, (uint64_t)*pInstance, "VkInstance", handleName);
  aux.instance = *pInstance;
  RegisterDebugCallback(&aux, *pInstance, debugReportFlags, &callback, validationDebugReportCallback);
  return r;
}

VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
  static PFN_vkQueuePresentKHR fn =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
  VkResult r = fn(queue, pPresentInfo);
  quitNow = true;
  return r;
}
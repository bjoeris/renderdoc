/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2018, 2019 Google LLC
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
#pragma once

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#if defined(_WIN32)
#include <Windows.h>
#endif

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include "vulkan/vulkan.h"

#include "format_helper.h"

#define STRINGIFY(s) #s

class CheckStream {
 public:
  CheckStream(const char *file, int line, const char *condition);
  ~CheckStream();

  std::ostream &stream() { return m_stream; }

 private:
  std::ostringstream m_stream;
};

struct StreamToVoid {
  // Use an operator with lower precedence than operator<<() to implicitly
  // coerce ostream into void.
  void operator&(std::ostream &) {}
};

#define CHECK(condition) \
  (condition)            \
      ? (void)0          \
      : StreamToVoid() & \
            CheckStream(__FILE__, __LINE__, STRINGIFY(condition)).stream()

#define CHECK_VK_SUCCESS(result)                                               \
  {                                                                            \
    VkResult r = result;                                                       \
    r == VK_SUCCESS                                                            \
        ? (void)0                                                              \
        : StreamToVoid() &                                                     \
              CheckStream(__FILE__, __LINE__, STRINGIFY(result == VK_SUCCESS)) \
                      .stream()                                                \
                  << "VkResult is \"" << VkResultToString(r) << "\"";          \
  }

struct VkHandle {
  uint64_t handle;
  std::string type;
  VkHandle(uint64_t h, const char *t) : handle(h), type(t) {}
  bool operator<(const VkHandle &rhs) const {
    if (handle < rhs.handle)
      return true;
    else if (handle > rhs.handle)
      return false;
    else
      return (type < rhs.type);
  }
};

typedef std::map<VkHandle, std::string> ResourceNamesMap;

void AddResourceName(ResourceNamesMap &map, uint64_t handle, const char *type,
                     const char *name);
const char *GetResourceName(const ResourceNamesMap &map, VkHandle handle);

struct AuxVkTraceResources {
  VkInstance instance;
  VkDevice device;
  VkPhysicalDevice physDevice;
  VkPhysicalDeviceProperties physDeviceProperties;
  VkPhysicalDeviceMemoryProperties physDeviceMemoryProperties;
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;
  VkDebugReportCallbackEXT callback;
  VkDebugUtilsMessengerEXT messenger;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
  VkQueue queue;
  VkFence fence;
  VkSemaphore acquire_semaphore, present_semaphore;
#ifdef _WIN32
  HINSTANCE hInstance;
  HWND hWnd;
#endif
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  VkImageCreateInfo virtualSwapchainImageCreateInfo;
  std::vector<VkImage> virtualSwapchainImages;
  std::vector<VkImage> realSwapchainImages;
  std::vector<VkDeviceMemory> virtualSwapchainImageMemories;

  uint64_t getTimestampValidBits(VkQueue queue) {
    uint64_t bits = 0;
    if (!physDeviceProperties.limits.timestampComputeAndGraphics) return 0;
    for (uint32_t i = 0; i < queueFamilyProperties.size() && bits == 0; i++) {
      if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
          (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)
        continue;
      for (uint32_t j = 0; j < queueFamilyProperties[i].queueCount && bits == 0;
           j++) {
        VkQueue q = NULL;
        vkGetDeviceQueue(device, i, j, &q);
        if (q == queue) {
          bits =
              (queueFamilyProperties[i].timestampValidBits == 64)
                  ? UINT64_MAX
                  : (1ULL << queueFamilyProperties[i].timestampValidBits) - 1;
        }
      }
    }
    return bits;
  }

  int32_t queueBits(VkQueue queue, bool &isGraphics, bool &isCompute,
                    bool &isTransfer) {
    for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
      for (uint32_t j = 0; j < queueFamilyProperties[i].queueCount; j++) {
        VkQueue q = NULL;
        vkGetDeviceQueue(device, i, j, &q);
        if (q == queue) {
          isGraphics =
              queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
          isCompute =
              (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
              !isGraphics;
          isTransfer =
              (queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
              !isGraphics && !isCompute;
          return i;
        }
      }
    }
    // this should be unreachable code for any valid queue.
    assert(0);
    return -1;
  }

  bool isGraphicsQueue(VkQueue queue) {
    bool isGraphics, isCompute, isTransfer;
    if (queueBits(queue, isGraphics, isCompute, isTransfer) >= 0)
      return isGraphics;
    else
      return false;
  }
  bool isComputeQueue(VkQueue queue) {
    bool isGraphics, isCompute, isTransfer;
    if (queueBits(queue, isGraphics, isCompute, isTransfer) >= 0)
      return isCompute;
    else
      return false;
  }
};

VkResult CreateSurface(AuxVkTraceResources &aux);
VkResult CreateVirtualSwapchainImages(
    AuxVkTraceResources &aux, const VkSwapchainCreateInfoKHR &swapchainInfo,
    VkImage *images, uint32_t imageCount);

VkResult AcquireVirtualSwapchainImage(AuxVkTraceResources &aux, VkImage image);

VkResult PresentVirtualSwapchainImage(AuxVkTraceResources &aux, VkImage image);

struct Region {
  uint64_t offset = 0;
  uint64_t size = 0;
  Region(){};
  Region(uint64_t o, uint64_t s) : offset(o), size(s) {}
};

struct MemoryRemap {
  Region capture;
  Region replay;
};

typedef std::vector<MemoryRemap> MemoryRemapVec;

VkResult vkDebugMarkerSetObject(VkDevice device,
                                const VkDebugMarkerObjectTagInfoEXT *pTagInfo);
VkResult vkDebugMarkerSetObjectName(
    VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo);
void vkCmdDebugMarkerBegin(VkCommandBuffer commandBuffer,
                           const VkDebugMarkerMarkerInfoEXT *pMarker);
void vkCmdDebugMarkerEnd(VkCommandBuffer commandBuffer);
void vkCmdDebugMarkerInsert(VkCommandBuffer commandBuffer,
                            const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);

void vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                            VkDeviceSize offset, VkBuffer countBuffer,
                            VkDeviceSize countBufferOffset,
                            uint32_t maxDrawCount, uint32_t stride);

void vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer,
                                   VkBuffer buffer, VkDeviceSize offset,
                                   VkBuffer countBuffer,
                                   VkDeviceSize countBufferOffset,
                                   uint32_t maxDrawCount, uint32_t stride);

VkPresentModeKHR GetCompatiblePresentMode(
    VkPresentModeKHR captured, std::vector<VkPresentModeKHR> present);

int32_t MemoryTypeIndex(VkMemoryPropertyFlags mask, uint32_t bits,
                        const VkPhysicalDeviceMemoryProperties &memory_props);

uint32_t CompatibleMemoryTypeIndex(
    uint32_t type, const VkPhysicalDeviceMemoryProperties &captured,
    const VkPhysicalDeviceMemoryProperties &present, uint32_t bits);

VkResult CheckMemoryAllocationCompatibility(
    uint32_t type, const VkPhysicalDeviceMemoryProperties &captured,
    const VkPhysicalDeviceMemoryProperties &present,
    const VkMemoryRequirements &requirements);

void ReadBuffer(const char *name, std::vector<uint8_t> &buf);

void InitializeDestinationBuffer(VkDevice device, VkBuffer *dst_buffer,
                                 VkDeviceMemory dst_memory, uint64_t size);
void InitializeSourceBuffer(VkDevice device, VkBuffer *buffer,
                            VkDeviceMemory *memory, size_t size,
                            uint8_t *initial_data,
                            VkPhysicalDeviceMemoryProperties props,
                            MemoryRemapVec &remap);
void InitializeAuxResources(AuxVkTraceResources *aux, VkInstance instance,
                            VkPhysicalDevice physDevice, VkDevice device);

void ImageLayoutTransition(VkCommandBuffer cmdBuffer, VkImage dstImage,
                           VkImageSubresourceRange subresourceRange,
                           VkImageLayout newLayout, uint32_t dstQueueFamily,
                           VkImageLayout oldLayout, uint32_t srcQueueFamily);

void ImageLayoutTransition(
    const AuxVkTraceResources &aux, VkImage dst, VkImageCreateInfo dst_ci,
    VkImageLayout final_layout,
    VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED);
void ImageLayoutTransition(
    const AuxVkTraceResources &aux, VkImage dst,
    VkImageSubresourceRange subresourceRange, VkImageLayout final_layout,
    VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED);
void ImageLayoutTransition(const AuxVkTraceResources &aux, VkImage dstImg,
                           uint32_t arrayLayer, uint32_t mipLevel,
                           VkImageAspectFlagBits aspect,
                           VkImageLayout newLayout, VkImageLayout oldLayout);

void CopyResetImage(const AuxVkTraceResources &aux, VkImage dst, VkBuffer src,
                    VkImageCreateInfo dst_ci);
void CopyResetBuffer(const AuxVkTraceResources &aux, VkBuffer dst, VkBuffer src,
                     VkDeviceSize size);

void CopyImageToBuffer(const AuxVkTraceResources &aux, VkImage src,
                       VkBuffer dst, VkImageCreateInfo src_ci);
void DiffDeviceMemory(const AuxVkTraceResources &aux, VkDeviceMemory expected,
                      VkDeviceSize expected_offset, VkDeviceMemory actual,
                      VkDeviceSize actual_offset, VkDeviceSize size,
                      const char *name);
void InitializeDiffBuffer(VkDevice device, VkBuffer *buffer,
                          VkDeviceMemory *memory, size_t size,
                          VkPhysicalDeviceMemoryProperties props);

void MakePhysicalDeviceFeaturesMatch(
    const VkPhysicalDeviceFeatures &available,
    VkPhysicalDeviceFeatures *captured_request);

void RegisterDebugCallback(
    AuxVkTraceResources *aux, VkInstance instance, VkDebugReportFlagsEXT flags,
    VkDebugReportCallbackEXT *pCallback,
    PFN_vkDebugReportCallbackEXT pfnCallback = VK_NULL_HANDLE);

VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo(
    VkDebugUtilsMessageSeverityFlagsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    PFN_vkDebugUtilsMessengerCallbackEXT pfnCallback = NULL);

void RegisterDebugUtilsCallback(
    AuxVkTraceResources *aux, VkInstance instance,
    VkDebugUtilsMessageSeverityFlagsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type, VkDebugUtilsMessengerEXT *pMessenger,
    PFN_vkDebugUtilsMessengerCallbackEXT pfnCallBack = NULL);

void MapUpdate(const AuxVkTraceResources &aux, uint8_t *dst, uint8_t *src,
               VkMappedMemoryRange range, VkMemoryAllocateInfo &ai,
               MemoryRemapVec &remap, VkDevice dev);

inline uint64_t AlignedSize(uint64_t size, uint64_t alignment) {
  return ((size / alignment) + ((size % alignment) > 0 ? 1 : 0)) * alignment;
}

inline uint64_t AlignedDown(uint64_t size, uint64_t alignment) {
  return (uint64_t(size / alignment)) * alignment;
}

inline std::string VkResultToString(VkResult r) {
  switch (r) {
#define RETURN_VK_RESULT_STRING(r) \
  case VK_##r:                     \
    return #r
    RETURN_VK_RESULT_STRING(SUCCESS);
    RETURN_VK_RESULT_STRING(NOT_READY);
    RETURN_VK_RESULT_STRING(TIMEOUT);
    RETURN_VK_RESULT_STRING(EVENT_SET);
    RETURN_VK_RESULT_STRING(EVENT_RESET);
    RETURN_VK_RESULT_STRING(INCOMPLETE);
    RETURN_VK_RESULT_STRING(ERROR_OUT_OF_HOST_MEMORY);
    RETURN_VK_RESULT_STRING(ERROR_OUT_OF_DEVICE_MEMORY);
    RETURN_VK_RESULT_STRING(ERROR_INITIALIZATION_FAILED);
    RETURN_VK_RESULT_STRING(ERROR_DEVICE_LOST);
    RETURN_VK_RESULT_STRING(ERROR_MEMORY_MAP_FAILED);
    RETURN_VK_RESULT_STRING(ERROR_LAYER_NOT_PRESENT);
    RETURN_VK_RESULT_STRING(ERROR_EXTENSION_NOT_PRESENT);
    RETURN_VK_RESULT_STRING(ERROR_FEATURE_NOT_PRESENT);
    RETURN_VK_RESULT_STRING(ERROR_INCOMPATIBLE_DRIVER);
    RETURN_VK_RESULT_STRING(ERROR_TOO_MANY_OBJECTS);
    RETURN_VK_RESULT_STRING(ERROR_FORMAT_NOT_SUPPORTED);
    RETURN_VK_RESULT_STRING(ERROR_SURFACE_LOST_KHR);
    RETURN_VK_RESULT_STRING(ERROR_NATIVE_WINDOW_IN_USE_KHR);
    RETURN_VK_RESULT_STRING(SUBOPTIMAL_KHR);
    RETURN_VK_RESULT_STRING(ERROR_OUT_OF_DATE_KHR);
    RETURN_VK_RESULT_STRING(ERROR_INCOMPATIBLE_DISPLAY_KHR);
    RETURN_VK_RESULT_STRING(ERROR_VALIDATION_FAILED_EXT);
    RETURN_VK_RESULT_STRING(ERROR_INVALID_SHADER_NV);
#undef RETURN_VK_RESULT_STRING
    default:
      return "UNKNOWN_ERROR";
  }
}
bool IsExtEnabled(const char *const *extList, uint32_t count, const char *ext);
bool IsExtSupported(VkPhysicalDevice physicalDevice, const char *ext);

std::string StageProgressString(const char *stage, uint32_t i, uint32_t N);

std::string GetEnvString(const char *envVarName);
int GetEnvInt(const char *envVarName, int defVal = 0);
FILE *OpenFile(char const *fileName, char const *mode);
std::string NormalizePath(std::string path);
bool DirExist(const std::string &path);
bool CreateDir(const std::string &path);
bool SetOutputDir(const char *path, std::string *output);
bool SetOutputFileName(const char *fileName, std::string *output);
bool ParseDirCommandLineFlag(int argc, char **argv, int *i,
                             std::string *outputDir);
bool ParseFileCommandLineFlag(int argc, char **argv, int *i,
                              std::string *outputFileName);

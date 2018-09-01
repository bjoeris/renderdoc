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

std::string StageProgressString(const char *stage, uint32_t i, uint32_t N);
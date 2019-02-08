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
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(_WIN32)
#include <direct.h>
#endif

#include <string>
#include "helper.h"

PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag;
PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName;
PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin;
PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd;
PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert;

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

void RegisterDebugCallback(AuxVkTraceResources *aux, VkInstance instance,
                           VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT *pCallback,
                           PFN_vkDebugReportCallbackEXT pfnCallBack)
{
  PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
  CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugReportCallbackEXT");

  VkDebugReportCallbackCreateInfoEXT ci = {
      VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT, 0, flags,
      (pfnCallBack == VK_NULL_HANDLE) ? DebugCallback : pfnCallBack, NULL};
  if(CreateDebugReportCallback != NULL)
  {
    VkResult result = CreateDebugReportCallback(instance, &ci, NULL, pCallback);
    assert(result == VK_SUCCESS);
  }
}

VkPresentModeKHR GetCompatiblePresentMode(VkPresentModeKHR captured,
                                          std::vector<VkPresentModeKHR> present)
{
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

void ImageLayoutTransition(const AuxVkTraceResources &aux, VkImage dstImage,
                           VkImageSubresourceRange subresourceRange, VkImageLayout newLayout,
                           VkImageLayout oldLayout)
{
  ImageLayoutTransition(aux.command_buffer, dstImage, subresourceRange, newLayout,
                        VK_QUEUE_FAMILY_IGNORED, oldLayout, VK_QUEUE_FAMILY_IGNORED);
}

void ImageLayoutTransition(const AuxVkTraceResources &aux, VkImage dstImage,
                           VkImageCreateInfo dstCI, VkImageLayout newLayout, VkImageLayout oldLayout)
{
  VkImageSubresourceRange subresourceRange = {
      FullAspectFromFormat(dstCI.format), 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

  ImageLayoutTransition(aux, dstImage, subresourceRange, newLayout, oldLayout);
}

void ImageLayoutTransition(const AuxVkTraceResources &aux, VkImage dstImg, uint32_t arrayLayer,
                           uint32_t mipLevel, VkImageAspectFlagBits aspect, VkImageLayout newLayout,
                           VkImageLayout oldLayout)
{
  VkImageSubresourceRange subresourceRange = {VkImageAspectFlags(aspect), mipLevel, 1, arrayLayer, 1};

  ImageLayoutTransition(aux, dstImg, subresourceRange, newLayout, oldLayout);
}

void CopyResetImage(const AuxVkTraceResources &aux, VkImage dst, VkBuffer src,
                    VkImageCreateInfo dst_ci)
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
  }
  else
  {
    std::string msg = std::string(__FUNCTION__) + std::string(": resets MSAA resource with ") +
                      std::to_string(dst_ci.samples) +
                      std::string(" samples. Currently this is not implemented.\n");
    printf("%s", msg.c_str());
#if defined(_WIN32) || defined(WIN32)
    OutputDebugStringA(msg.c_str());
#endif
  }
}
void CopyResetBuffer(const AuxVkTraceResources &aux, VkBuffer dst, VkBuffer src, VkDeviceSize size)
{
  if(size == 0)
    return;
  VkBufferCopy region = {0, 0, size};
  vkCmdCopyBuffer(aux.command_buffer, src, dst, 1, &region);
}

void CopyImageToBuffer(const AuxVkTraceResources &aux, VkImage src, VkBuffer dst,
                       VkImageCreateInfo src_ci)
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
      for(uint32_t a = 0; a < src_ci.arrayLayers; a++)
      {
        VkExtent3D dim = src_ci.extent;
        uint32_t x = 0;
        FixCompressedSizes(src_ci.format, dim, x);
        for(uint32_t m = 0; m < src_ci.mipLevels; m++)
        {
          VkBufferImageCopy region = {offset,     dim.width,
                                      dim.height, {VkImageAspectFlags(aspects[j]), m, a, 1},
                                      {0, 0, 0},  dim};
          offset += (uint32_t)(dim.depth * dim.width * dim.height *
                               SizeOfFormat(src_ci.format, aspects[j]));
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

void DiffDeviceMemory(const AuxVkTraceResources &aux, VkDeviceMemory expected,
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
    std::string msg = std::string(__FUNCTION__) + std::string(": Resource ") + std::string(name) +
                      std::string(" has changed by the end of the frame.\n");
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

  if(IsExtSupported(aux->physDevice, "VK_EXT_debug_marker"))
  {
    vkDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetInstanceProcAddr(
        instance, "vkDebugMarkerSetObjectTagEXT");
    vkDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetInstanceProcAddr(
        instance, "vkDebugMarkerSetObjectNameEXT");
    vkCmdDebugMarkerBegin =
        (PFN_vkCmdDebugMarkerBeginEXT)vkGetInstanceProcAddr(instance, "vkCmdDebugMarkerBeginEXT");
    vkCmdDebugMarkerEnd =
        (PFN_vkCmdDebugMarkerEndEXT)vkGetInstanceProcAddr(instance, "vkCmdDebugMarkerEndEXT");
    vkCmdDebugMarkerInsert =
        (PFN_vkCmdDebugMarkerInsertEXT)vkGetInstanceProcAddr(instance, "vkCmdDebugMarkerInsertEXT");
  }

  uint32_t queueFamilyPropertyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(aux->physDevice, &queueFamilyPropertyCount, NULL);
  assert(queueFamilyPropertyCount != 0);
  aux->queueFamilyProperties.resize(queueFamilyPropertyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(aux->physDevice, &queueFamilyPropertyCount,
                                           aux->queueFamilyProperties.data());
}

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
  FILE *f = OpenFile(name, "rb");
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

void MapUpdate(const AuxVkTraceResources &aux, uint8_t *dst, uint8_t *src,
               const VkMappedMemoryRange &range, VkMemoryAllocateInfo &ai, MemoryRemapVec &remap,
               VkDevice dev)
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
        // If this memory range doesn't overlap with any captured resource
        // continue
        if(!RegionsOverlap(memory_region, captured_resource_region))
          continue;

        // Find the inteval where these two regions overlap. It is guaranteed to
        // be non-null.
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
}

std::string GetEnvString(const char *envVarName)
{
  std::string val;
#ifdef _WIN32
  char *buf = NULL;
  if(_dupenv_s(&buf, NULL, envVarName) == 0 && buf)
  {
    val = buf;
    free(buf);
  }
#else
  char *p = getenv(envVarName);
  if(p)
  {
    val = p;
  }
#endif
  return val;
}

int GetEnvInt(const char *envVarName, int defVal)
{
  std::string val = GetEnvString(envVarName);
  return val.empty() ? defVal : atoi(val.c_str());
}

FILE *OpenFile(char const *fileName, char const *mode)
{
  FILE *fp = NULL;
#ifdef _WIN32
  if(fopen_s(&fp, fileName, mode) != 0)
    fp = NULL;
#else
  fp = fopen(fileName, mode);
#endif
  return fp;
}

void AddResourceName(ResourceNamesMap &map, uint64_t handle, const char *type, const char *name)
{
  VkHandle h = VkHandle(handle, type);
  if(map.find(h) != map.end())
  {
    // Vulkan objects of a non-dispatchable type may have the same handle value,
    // Concatenate the names in this case.
    std::string newName = map[h] + "_" + std::string(name);
    map[h] = newName;
  }
  else
  {
    map[h] = std::string(name);
  }
}

const char *GetResourceName(ResourceNamesMap &map, VkHandle handle)
{
  if(map.find(handle) == map.end())
  {
#if defined(DEBUG) || defined(_DEBUG)
    assert(0);
#else
    fprintf(stdout, "Cannot get resource name with type %s and value %" PRIu64, handle.type.c_str(),
            handle.handle);
    exit(1);
#endif
  }
  return map[handle].c_str();
}

bool DirExist(const std::string &path)
{
  struct stat info;
  if(stat(path.c_str(), &info) != 0)
  {
    return false;
  }
  return (info.st_mode & S_IFDIR) != 0;
}

std::string NormalizePath(std::string path)
{
#if defined(_WIN32)
  // Replace slash with backslash and add trailing slash if needed.
  // Trailing slash is for concatenating file name to directory later.
  std::replace(path.begin(), path.end(), '/', '\\');
  if(!path.empty() && path.back() != '\\')
  {
    path.push_back('\\');
  }
#else
  // Add trailing slash if needed.
  // Trailing slash is for concatenating file name to directory later.
  if(!path.empty() && path.back() != '/')
  {
    path.append("/");
  }
#endif
  return path;
}

bool CreateDir(const std::string &path)
{
#if defined(_WIN32)
  int nError = _mkdir(path.c_str());
#else
  mode_t nMode = 0755;
  int nError = mkdir(path.c_str(), nMode);
#endif
  if(nError == 0)
    return true;

  switch(errno)
  {
    case ENOENT:    // something along the path does not exist
    {
#if defined(_WIN32)
      size_t pos = path.find_last_of('\\');
#else
      size_t pos = path.find_last_of('/');
#endif
      if(pos == std::string::npos)
        return false;
      if(!CreateDir(path.substr(0, pos)))
        return false;
#if defined(_WIN32)
      return (_mkdir(path.c_str()) == 0 || DirExist(path));
#else
      return (mkdir(path.c_str(), mode) == 0 || DirExist(path));
#endif
    }
    case EEXIST: return DirExist(path);
    default: return false;
  }
}

bool SetOutputFileName(const char *fileName, std::string *outputFileName)
{
  if(strlen(fileName) == 0)
    return false;
  *outputFileName = fileName;
  return true;
}

bool SetOutputDir(const char *path, std::string *outputDir)
{
  std::string dirPath = NormalizePath(path);
  if(DirExist(dirPath) || CreateDir(dirPath))
  {
    *outputDir = dirPath;
    return true;
  }
  else
  {
    return false;
  }
}

bool ParseDirCommandLineFlag(int argc, char **argv, int *i, std::string *outputDir)
{
  if(strcmp(argv[*i], "-d") == 0 || strcmp(argv[*i], "--dir") == 0)
  {
    *i = *i + 1;
    if(*i >= argc)
    {
      return false;
    }
    if(SetOutputDir(argv[*i], outputDir))
    {
      *i = *i + 1;
      return true;
    }
  }
  return false;
}

bool ParseFileCommandLineFlag(int argc, char **argv, int *i, std::string *outputFileName)
{
  if(strcmp(argv[*i], "-f") == 0 || strcmp(argv[*i], "--file") == 0)
  {
    *i = *i + 1;
    if(*i >= argc)
    {
      return false;
    }
    if(SetOutputFileName(argv[*i], outputFileName))
    {
      *i = *i + 1;
      return true;
    }
  }
  return false;
}

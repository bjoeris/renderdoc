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
#include "helper/helper.h"

#include "shim_vulkan.h"
#include "utils.h"

std::map<VkHandle, std::string> ResourceNames;

void AddResourceName(uint64_t handle, const char *type, const char *name)
{
  VkHandle h = VkHandle(handle, type);
  if(ResourceNames.find(h) != ResourceNames.end())
  {
    // Vulkan objects of a non-dispatchable type may have the same handle value,
    // Concatenate the names in this case.
    std::string newName = ResourceNames[h] + "_" + std::string(name);
    ResourceNames[h] = newName;
  }
  else
  {
    ResourceNames[h] = std::string(name);
  }
}

const char *GetResourceName(VkHandle handle)
{
  if(ResourceNames.find(handle) == ResourceNames.end())
  {
#if defined(DEBUG) || defined(_DEBUG)
    assert(0);
#else
    fprintf(stdout, "Cannot get resource name with type %s and value %" PRIu64, handle.type.c_str(),
            handle.handle);
    exit(1);
#endif
  }
  return ResourceNames[handle].c_str();
}

VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance,
                               const char *handleName)
{
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

void shim_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                   uint32_t *pQueueFamilyPropertyCount,
                                                   VkQueueFamilyProperties *pQueueFamilyProperties)
{
  static PFN_vkGetPhysicalDeviceQueueFamilyProperties fn = vkGetPhysicalDeviceQueueFamilyProperties;
  fn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
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
                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                               const char *handleName)
{
  static PFN_vkAllocateMemory fn = vkAllocateMemory;
  VkResult r = fn(device, pAllocateInfo, pAllocator, pMemory);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pMemory, "VkDeviceMemory", handleName);
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
                            const VkAllocationCallbacks *pAllocator, VkFence *pFence,
                            const char *handleName)
{
  static PFN_vkCreateFence fn = vkCreateFence;
  VkResult r = fn(device, pCreateInfo, pAllocator, pFence);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pFence, "VkFence", handleName);
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
                                const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                const char *handleName)
{
  static PFN_vkCreateSemaphore fn = vkCreateSemaphore;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSemaphore);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pSemaphore, "VkSemaphore", handleName);
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
                            const VkAllocationCallbacks *pAllocator, VkEvent *pEvent,
                            const char *handleName)
{
  static PFN_vkCreateEvent fn = vkCreateEvent;
  VkResult r = fn(device, pCreateInfo, pAllocator, pEvent);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pEvent, "VkEvent", handleName);
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
                                const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                const char *handleName)
{
  static PFN_vkCreateQueryPool fn = vkCreateQueryPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pQueryPool);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pQueryPool, "VkQueryPool", handleName);
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
                             const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                             const char *handleName)
{
  static PFN_vkCreateBuffer fn = vkCreateBuffer;
  VkResult r = fn(device, pCreateInfo, pAllocator, pBuffer);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pBuffer, "VkBuffer", handleName);
  return r;
}

void shim_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator)
{
  static PFN_vkDestroyBuffer fn = vkDestroyBuffer;
  fn(device, buffer, pAllocator);
  return;
}

VkResult shim_vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkBufferView *pView,
                                 const char *handleName)
{
  static PFN_vkCreateBufferView fn = vkCreateBufferView;
  VkResult r = fn(device, pCreateInfo, pAllocator, pView);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pView, "VkBufferView", handleName);
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
                            const VkAllocationCallbacks *pAllocator, VkImage *pImage,
                            const char *handleName)
{
  static PFN_vkCreateImage fn = vkCreateImage;
  VkResult r = fn(device, pCreateInfo, pAllocator, pImage);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pImage, "VkImage", handleName);
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
                                const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                const char *handleName)
{
  static PFN_vkCreateImageView fn = vkCreateImageView;
  VkResult r = fn(device, pCreateInfo, pAllocator, pView);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pView, "VkImageView", handleName);
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
                                   VkShaderModule *pShaderModule, const char *handleName)
{
  static PFN_vkCreateShaderModule fn = vkCreateShaderModule;
  VkResult r = fn(device, pCreateInfo, pAllocator, pShaderModule);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pShaderModule, "VkShaderModule", handleName);
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
                                    VkPipelineCache *pPipelineCache, const char *handleName)
{
  static PFN_vkCreatePipelineCache fn = vkCreatePipelineCache;
  VkResult r = fn(device, pCreateInfo, pAllocator, pPipelineCache);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pPipelineCache, "VkPipelineCache", handleName);
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
                                        VkPipeline *pPipelines, const char *handleName)
{
  static PFN_vkCreateGraphicsPipelines fn = vkCreateGraphicsPipelines;
  VkResult r = fn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pPipelines, "VkPipeline", handleName);
  return r;
}

VkResult shim_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                       uint32_t createInfoCount,
                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                       const VkAllocationCallbacks *pAllocator,
                                       VkPipeline *pPipelines, const char *handleName)
{
  static PFN_vkCreateComputePipelines fn = vkCreateComputePipelines;
  VkResult r = fn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pPipelines, "VkPipeline", handleName);
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
                                     VkPipelineLayout *pPipelineLayout, const char *handleName)
{
  static PFN_vkCreatePipelineLayout fn = vkCreatePipelineLayout;
  VkResult r = fn(device, pCreateInfo, pAllocator, pPipelineLayout);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pPipelineLayout, "VkPipelineLayout", handleName);
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
                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                              const char *handleName)
{
  static PFN_vkCreateSampler fn = vkCreateSampler;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSampler);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pSampler, "VkSampler", handleName);
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
                                          VkDescriptorSetLayout *pSetLayout, const char *handleName)
{
  static PFN_vkCreateDescriptorSetLayout fn = vkCreateDescriptorSetLayout;
  VkResult r = fn(device, pCreateInfo, pAllocator, pSetLayout);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pSetLayout, "VkDescriptorSetLayout", handleName);
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
                                     VkDescriptorPool *pDescriptorPool, const char *handleName)
{
  static PFN_vkCreateDescriptorPool fn = vkCreateDescriptorPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorPool);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pDescriptorPool, "VkDescriptorPool", handleName);
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
                                       VkDescriptorSet *pDescriptorSets, const char *handleName)
{
  static PFN_vkAllocateDescriptorSets fn = vkAllocateDescriptorSets;
  VkResult r = fn(device, pAllocateInfo, pDescriptorSets);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pDescriptorSets, "VkDescriptorSet", handleName);
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
                                  VkFramebuffer *pFramebuffer, const char *handleName)
{
  static PFN_vkCreateFramebuffer fn = vkCreateFramebuffer;
  VkResult r = fn(device, pCreateInfo, pAllocator, pFramebuffer);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pFramebuffer, "VkFramebuffer", handleName);
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
                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                 const char *handleName)
{
  static PFN_vkCreateRenderPass fn = vkCreateRenderPass;
  VkResult r = fn(device, pCreateInfo, pAllocator, pRenderPass);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pRenderPass, "VkRenderPass", handleName);
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
                                  VkCommandPool *pCommandPool, const char *handleName)
{
  static PFN_vkCreateCommandPool fn = vkCreateCommandPool;
  VkResult r = fn(device, pCreateInfo, pAllocator, pCommandPool);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pCommandPool, "VkCommandPool", handleName);
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
                                       VkCommandBuffer *pCommandBuffers, const char *handleName)
{
  static PFN_vkAllocateCommandBuffers fn = vkAllocateCommandBuffers;
  VkResult r = fn(device, pAllocateInfo, pCommandBuffers);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pCommandBuffers, "VkCommandBuffer", handleName);
  return r;
}

void shim_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                               uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers)
{
  static PFN_vkFreeCommandBuffers fn = vkFreeCommandBuffers;
  fn(device, commandPool, commandBufferCount, pCommandBuffers);
  return;
}

VkResult shim_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
  static PFN_vkResetCommandBuffer fn = vkResetCommandBuffer;
  VkResult r = fn(commandBuffer, flags);
  return r;
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
  static PFN_vkGetDisplayModePropertiesKHR fn =
      (PFN_vkGetDisplayModePropertiesKHR)vkGetInstanceProcAddr(aux.instance,
                                                               "vkGetDisplayModePropertiesKHR");
  VkResult r = fn(physicalDevice, display, pPropertyCount, pProperties);
  return r;
}

VkResult shim_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                     const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkDisplayModeKHR *pMode, const char *handleName)
{
  static PFN_vkCreateDisplayModeKHR fn =
      (PFN_vkCreateDisplayModeKHR)vkGetInstanceProcAddr(aux.instance, "vkCreateDisplayModeKHR");
  VkResult r = fn(physicalDevice, display, pCreateInfo, pAllocator, pMode);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pMode, "VkDisplayModeKHR", handleName);
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
                                             VkSurfaceKHR *pSurface, const char *handleName)
{
  static PFN_vkCreateDisplayPlaneSurfaceKHR fn =
      (PFN_vkCreateDisplayPlaneSurfaceKHR)vkGetInstanceProcAddr(instance,
                                                                "vkCreateDisplayPlaneSurfaceKHR");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pSurface);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pSurface, "VkSurfaceKHR", handleName);
  return r;
}

VkResult shim_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                          const VkSwapchainCreateInfoKHR *pCreateInfos,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkSwapchainKHR *pSwapchains, const char *handleName)
{
  static PFN_vkCreateSharedSwapchainsKHR fn =
      (PFN_vkCreateSharedSwapchainsKHR)vkGetDeviceProcAddr(device, "vkCreateSharedSwapchainsKHR");
  VkResult r = fn(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pSwapchains, "VkSwapchainKHR", handleName);
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
                                             VkDebugReportCallbackEXT *pCallback,
                                             const char *handleName)
{
  static PFN_vkCreateDebugReportCallbackEXT fn =
      (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance,
                                                                "vkCreateDebugReportCallbackEXT");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pCallback);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pCallback, "VkDebugReportCallbackEXT", handleName);
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
    const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNVX *pIndirectCommandsLayout,
    const char *handleName)
{
  static PFN_vkCreateIndirectCommandsLayoutNVX fn =
      (PFN_vkCreateIndirectCommandsLayoutNVX)vkGetDeviceProcAddr(
          device, "vkCreateIndirectCommandsLayoutNVX");
  VkResult r = fn(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pIndirectCommandsLayout, "VkIndirectCommandsLayoutNVX", handleName);
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
                                     VkObjectTableNVX *pObjectTable, const char *handleName)
{
  static PFN_vkCreateObjectTableNVX fn =
      (PFN_vkCreateObjectTableNVX)vkGetDeviceProcAddr(device, "vkCreateObjectTableNVX");
  VkResult r = fn(device, pCreateInfo, pAllocator, pObjectTable);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pObjectTable, "VkObjectTableNVX", handleName);
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
                                               VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                               const char *handleName)
{
  static PFN_vkCreateDescriptorUpdateTemplate fn =
      (PFN_vkCreateDescriptorUpdateTemplate)vkGetDeviceProcAddr(device,
                                                                "vkCreateDescriptorUpdateTemplate");
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pDescriptorUpdateTemplate, "VkDescriptorUpdateTemplate", handleName);
  return r;
}

VkResult shim_vkCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
    const char *handleName)
{
  static PFN_vkCreateDescriptorUpdateTemplateKHR fn =
      (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(
          device, "vkCreateDescriptorUpdateTemplateKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pDescriptorUpdateTemplate, "VkDescriptorUpdateTemplate", handleName);
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
                                             VkSamplerYcbcrConversion *pYcbcrConversion,
                                             const char *handleName)
{
  static PFN_vkCreateSamplerYcbcrConversion fn =
      (PFN_vkCreateSamplerYcbcrConversion)vkGetDeviceProcAddr(device,
                                                              "vkCreateSamplerYcbcrConversion");
  VkResult r = fn(device, pCreateInfo, pAllocator, pYcbcrConversion);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pYcbcrConversion, "VkSamplerYcbcrConversion", handleName);
  return r;
}

VkResult shim_vkCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator,
                                                VkSamplerYcbcrConversion *pYcbcrConversion,
                                                const char *handleName)
{
  static PFN_vkCreateSamplerYcbcrConversionKHR fn =
      (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(
          device, "vkCreateSamplerYcbcrConversionKHR");
  VkResult r = fn(device, pCreateInfo, pAllocator, pYcbcrConversion);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pYcbcrConversion, "VkSamplerYcbcrConversion", handleName);
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
                                         VkValidationCacheEXT *pValidationCache,
                                         const char *handleName)
{
  static PFN_vkCreateValidationCacheEXT fn =
      (PFN_vkCreateValidationCacheEXT)vkGetDeviceProcAddr(device, "vkCreateValidationCacheEXT");
  VkResult r = fn(device, pCreateInfo, pAllocator, pValidationCache);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pValidationCache, "VkValidationCacheEXT", handleName);
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
                                             VkDebugUtilsMessengerEXT *pMessenger,
                                             const char *handleName)
{
  static PFN_vkCreateDebugUtilsMessengerEXT fn =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                "vkCreateDebugUtilsMessengerEXT");
  VkResult r = fn(instance, pCreateInfo, pAllocator, pMessenger);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pMessenger, "VkDebugUtilsMessengerEXT", handleName);
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

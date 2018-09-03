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

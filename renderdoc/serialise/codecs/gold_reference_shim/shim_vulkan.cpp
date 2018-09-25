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

#include <iostream>
#include <map>

#include "helper/helper.h"
#include "shim_vulkan.h"
#include "utils.h"

const char *RDOC_ENV_VAR =
    "RDOC_GOLD_FRAME_INDEX";    // environment variable for to-be-captured frame.
int captureFrame = 5;           // default frame index if RDOC_GOLD_FRAME_INDEX is not set.
int presentIndex = 0;
bool IsTargetFrame = true;    // default value doesn't matter. It's properly set in CreateInstance.

int renderPassCount = 0;
bool quitNow = false;
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

bool ShimShouldQuitNow() { return quitNow; }

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
  pCI->usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;    // we might have to read back from this image.

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
  ImageAndView iav(pCreateInfo->image, imagesMap[pCreateInfo->image], *pView, *pCreateInfo);
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
  VkRenderPassCreateInfo *CreateInfo = const_cast<VkRenderPassCreateInfo *>(pCreateInfo);
  for(uint32_t i = 0; i < pCreateInfo->attachmentCount; i++)
  {
    VkAttachmentDescription *desc =
        const_cast<VkAttachmentDescription *>(&CreateInfo->pAttachments[i]);
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
    RenderPassInfo &rpInfo = cmdBufferRenderPassInfos[commandBuffer];
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
#ifdef SAVE_RENDERPASS_BUFFERS
  if(!IsTargetFrame)
    return r;
  vkQueueWaitIdle(queue);
  for(uint32_t i = 0; i < submitCount; i++)
  {
    for(uint32_t cbIndex = 0; cbIndex < pSubmits[i].commandBufferCount; cbIndex++)
    {
      std::vector<ReadbackInfos> &readbacks =
          cmdBufferReadBackInfos[pSubmits[i].pCommandBuffers[cbIndex]];
      for(uint32_t j = 0; j < readbacks.size(); j++)
      {
        ReadbackInfos infos = readbacks[j];
        for(uint32_t a = 0; a < infos.attachments.size(); a++)
        {
          ReadbackInfo info = infos.attachments[a];
          char handleStr[32];
          sprintf(handleStr, "%p", info.srcImage);
          std::string filename;
#if defined(__yeti__)
          filename = "/var/game/";
#endif
          filename += std::to_string(renderPassCount) + "_attachment_" + std::to_string(info.index) +
                      "_resource_" + std::string(handleStr) + "_" + FormatToString(info.format) +
                      "_" + std::to_string(info.width) + "x" + std::to_string(info.height) + ".ppm";
          bufferToPpm(info.buffer, info.bufferDeviceMem, filename, info.width, info.height,
                      info.format);
          info.Clear(aux.device);
        }
        renderPassCount++;
      }
      cmdBufferReadBackInfos.erase(pSubmits[i].pCommandBuffers[cbIndex]);
    }
  }
#endif    // SAVE_RENDERPASS_BUFFERS
  return r;
}

VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
  PFN_vkQueuePresentKHR fn =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
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
    quitNow = true;
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
  VkSwapchainCreateInfoKHR *pCI = const_cast<VkSwapchainCreateInfoKHR *>(pCreateInfo);
  pCI->imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;    // we will copy from presented images.
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
  IsTargetFrame =
      presentIndex == captureFrame;    // if captureFrame is '0', first frame needs to save images.
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

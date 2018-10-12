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

const char RDOC_ENV_VAR[] = "RDOC_GOLD_FRAME_INDEX";    // env variable for to-be-captured frame.
const int kDefaultCaptureFrame = 5;    // default frame index if RDOC_GOLD_FRAME_INDEX is not set.
int captureFrame = kDefaultCaptureFrame;
int presentIndex = 0;
bool IsTargetFrame = true;    // default value doesn't matter. It's properly set in CreateInstance.

int renderPassCount = 0;
bool quitNow = false;
VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
VkSwapchainCreateInfoKHR swapchainCI;
std::map<VkSwapchainKHR, std::vector<VkImage>> swapchainImageMap;
std::map<VkFramebuffer, std::vector<VkImageView>> framebufferAttachements;
std::map<VkImageView, ImageAndView> imageAndViewMap;
std::map<VkImage, VkImageCreateInfo> imagesMap;
std::map<VkRenderPass, RenderPassInfo> renderPassInfos;
std::map<VkCommandBuffer, RenderPassInfo> cmdBufferRenderPassInfos;
std::map<VkCommandBuffer, std::vector<ReadbackInfos>> cmdBufferReadBackInfos;

bool ShimShouldQuitNow()
{
  return quitNow;
}

VkResult shim_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance,
                               const char *handleName)
{
  VkResult r = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
  assert(r == VK_SUCCESS);
  AddResourceName((uint64_t)*pInstance, "VkInstance", handleName);
  aux.instance = *pInstance;
  captureFrame = GetEnvInt(RDOC_ENV_VAR, kDefaultCaptureFrame);
  // if captureFrame is '0', first frame needs to save images.
  IsTargetFrame = presentIndex == captureFrame;
  return r;
}

void shim_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                              VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
  static PFN_vkGetPhysicalDeviceMemoryProperties fn = vkGetPhysicalDeviceMemoryProperties;
  fn(physicalDevice, pMemoryProperties);
  physicalDeviceMemoryProperties = *pMemoryProperties;
  return;
}

VkResult shim_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                            const VkAllocationCallbacks *pAllocator, VkImage *pImage,
                            const char *handleName)
{
  VkImageCreateInfo *pCI = (VkImageCreateInfo *)pCreateInfo;
  pCI->usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;    // we might have to read back from this image.

  static PFN_vkCreateImage fn = vkCreateImage;
  VkResult r = fn(device, pCreateInfo, pAllocator, pImage);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pImage, "VkImage", handleName);
  imagesMap[*pImage] = *pCreateInfo;
  return r;
}

VkResult shim_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                const char *handleName)
{
  static PFN_vkCreateImageView fn = vkCreateImageView;
  VkResult r = fn(device, pCreateInfo, pAllocator, pView);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pView, "VkImageView", handleName);
  ImageAndView iav(pCreateInfo->image, imagesMap[pCreateInfo->image], *pView, *pCreateInfo);
  imageAndViewMap[*pView] = iav;
  return r;
}

VkResult shim_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator,
                                  VkFramebuffer *pFramebuffer, const char *handleName)
{
  static PFN_vkCreateFramebuffer fn = vkCreateFramebuffer;
  VkResult r = fn(device, pCreateInfo, pAllocator, pFramebuffer);
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pFramebuffer, "VkFramebuffer", handleName);
  std::vector<VkImageView> attachments(pCreateInfo->pAttachments,
                                       pCreateInfo->pAttachments + pCreateInfo->attachmentCount);
  framebufferAttachements[*pFramebuffer] = attachments;
  return r;
}

VkResult shim_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                 const char *handleName)
{
  static PFN_vkCreateRenderPass fn = vkCreateRenderPass;
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
  if(r == VK_SUCCESS)
    AddResourceName((uint64_t)*pRenderPass, "VkRenderPass", handleName);
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
  static PFN_vkCmdBeginRenderPass fn = vkCmdBeginRenderPass;
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
  static PFN_vkQueueSubmit fn = vkQueueSubmit;
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
          snprintf(handleStr, sizeof(handleStr), "%p", info.srcImage);
          std::string filename;
#if defined(__yeti__)
          filename = "/var/game/";
#endif
          filename += std::to_string(renderPassCount) + "_attachment_" + std::to_string(info.index) +
                      "_" + GetResourceName(VkHandle((uint64_t)info.srcImage, "VkImage")) + "_" +
                      FormatToString(info.format) + "_" + std::to_string(info.width) + "x" +
                      std::to_string(info.height) + ".ppm";
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
  static PFN_vkQueuePresentKHR fn =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
  if(IsTargetFrame)
  {
    // Save screenshots
    for(uint32_t i = 0; i < (*pPresentInfo).swapchainCount; i++)
    {
      VkImage srcImage =
          swapchainImageMap[(*pPresentInfo).pSwapchains[i]][(*pPresentInfo).pImageIndices[i]];
      std::string filename;
#if defined(__yeti__)
      filename = "/var/game/";
#endif
      filename +=
          "screenshot_f" + std::to_string(presentIndex) + "_sw" + std::to_string(i) + ".ppm";
      screenshot(srcImage, filename.c_str());
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
  static PFN_vkGetSwapchainImagesKHR fn =
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
  static PFN_vkCreateSwapchainKHR fn =
      (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
  swapchainCI = *pCreateInfo;
  VkSwapchainCreateInfoKHR *pCI = const_cast<VkSwapchainCreateInfoKHR *>(pCreateInfo);
  pCI->imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;    // we will copy from presented images.
  VkResult r = fn(device, pCreateInfo, pAllocator, pSwapchain);
  return r;
}

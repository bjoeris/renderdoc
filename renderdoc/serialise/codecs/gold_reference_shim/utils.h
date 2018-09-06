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
#include <algorithm>
#include <fstream>
#include <map>
#include <string>

#include <assert.h>

#include "helper/helper.h"

// Utility structure that associates VkImageView with parent VkImage
// and their corresponding CreateInfo structures.
struct ImageAndView
{
  struct ImageAndCI
  {
    VkImage res;
    VkImageCreateInfo ci;
  } image;

  struct ImageViewAndCI
  {
    VkImageView res;
    VkImageViewCreateInfo ci;
  } view;

  ImageAndView()
  {
    memset(&image, 0, sizeof(image));
    memset(&view, 0, sizeof(view));
  }
  ImageAndView(VkImage i, VkImageCreateInfo ici, VkImageView v, VkImageViewCreateInfo vci)
  {
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

  ReadbackInfo(VkImage src, VkBuffer b, VkImage i, VkDeviceMemory bMem, VkDeviceMemory iMem,
               uint32_t w, uint32_t h, VkFormat f, int a)
      : srcImage(src),
        buffer(b),
        image(i),
        bufferDeviceMem(bMem),
        imageDeviceMem(iMem),
        width(w),
        height(h),
        format(f),
        index(a)
  {
  }

  void Clear(VkDevice device)
  {
    if(image)
      vkDestroyImage(device, image, NULL);
    if(imageDeviceMem)
      vkFreeMemory(device, imageDeviceMem, NULL);
    if(buffer)
      vkDestroyBuffer(device, buffer, NULL);
    if(bufferDeviceMem)
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

struct ReadbackInfos
{
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
extern int presentIndex;
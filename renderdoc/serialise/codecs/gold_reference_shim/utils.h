#pragma once
#include <algorithm>
#include <fstream>
#include <map>
#include <string>

#include <assert.h>

#include "helper/helper.h"

struct RenderPassInfo
{
  VkRenderPass renderPass;
  std::vector<VkImageLayout> finalLayouts;
  std::vector<VkImageView> attachments;
  std::vector<VkImage> images;
  std::vector<VkImageCreateInfo> imageCIs;
};

VkImage getStagingImage(VkImageCreateInfo ci);
VkBuffer getStagingBuffer(uint32_t width, uint32_t height, uint32_t bytes);
void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height, VkFormat format);
void imgToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height, VkFormat format);
void bufferToPpm(VkBuffer buffer, std::string filename, uint32_t width, uint32_t height,
                 VkFormat format);
void copyFramebufferAttachments(RenderPassInfo *rpInfo);
void screenshot(VkImage srcImage, const char *filename);
void cleanup(RenderPassInfo *pRpInfo);

extern AuxVkTraceResources aux;
extern VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
extern VkFormat swapchainImageFormat;
extern int renderPassCount;
extern int presentIndex;

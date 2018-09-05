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
#include <cmath>
#include <fstream>
#include <functional>
#include <map>

#include <float.h>

#include "helper/helper.h"
#include "shim_vulkan.h"
#include "utils.h"

void bufferToPpm(VkBuffer buffer, VkDeviceMemory mem, std::string filename, uint32_t width,
                 uint32_t height, VkFormat format)
{
  void *data;
  vkMapMemory(aux.device, mem, 0, VK_WHOLE_SIZE, 0, &data);

  // raw data
  std::string rawFilename = filename.substr(0, filename.find_last_of("."));
  std::ofstream rawFile(rawFilename, std::ios::out | std::ios::binary);
  rawFile.write((char *)data, width * height * kImageAspectsAndByteSize[format].second);
  rawFile.close();

  switch(format)
  {
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    {
      std::vector<uint8_t> outputDepth(width * height * 3);
      fillPPM(outputDepth.data(), data, width, height, format);

      std::string depthFilename(filename);
      depthFilename.insert(filename.find_last_of("."), "_DEPTH");
      std::ofstream depthFile(depthFilename, std::ios::out | std::ios::binary);
      depthFile << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";
      depthFile.write((char *)outputDepth.data(), width * height * 3);
      depthFile.close();

      std::vector<uint8_t> outputStencil(width * height * 3);
      fillPPM(outputStencil.data(), data, width, height, format, true);

      std::string stencilFilename(filename);
      stencilFilename.insert(filename.find_last_of("."), "_STENCIL");
      std::ofstream stencilFile(stencilFilename, std::ios::out | std::ios::binary);
      stencilFile << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";
      stencilFile.write((char *)outputStencil.data(), width * height * 3);
      stencilFile.close();
    }
    break;
    default:
    {
      std::vector<uint8_t> output(width * height * 3);
      fillPPM(output.data(), data, width, height, format);

      std::ofstream file(filename, std::ios::out | std::ios::binary);
      file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";
      file.write((char *)output.data(), width * height * 3);
      file.close();
    }
    break;
  }

  vkUnmapMemory(aux.device, mem);
}

// imgToBuffer copies the full image to buffer tightly-packed.
// For a depth/stencil format, the first region is depth aspect, and the second is
// stencil aspect.
void imgToBuffer(VkCommandBuffer cmdBuf, VkImage image, VkBuffer buffer, uint32_t width,
                 uint32_t height, uint32_t mip, uint32_t layer, VkFormat format)
{
  switch(format)
  {
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    {
      double depthSizeInBytes = SizeOfFormat(format, VK_IMAGE_ASPECT_DEPTH_BIT);
      VkBufferImageCopy regions[2] = {{}, {}};
      regions[0].imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, mip, layer, 1};
      regions[0].imageExtent = {width, height, 1};
      regions[1].bufferOffset = (VkDeviceSize)(width * height * depthSizeInBytes);
      regions[1].imageSubresource = {VK_IMAGE_ASPECT_STENCIL_BIT, mip, layer, 1};
      regions[1].imageExtent = {width, height, 1};
      vkCmdCopyImageToBuffer(cmdBuf, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 2, regions);
    }
    break;
    default:
    {
      VkBufferImageCopy region = {};
      region.imageSubresource = {kImageAspectsAndByteSize[format].first, mip, layer, 1};
      region.imageExtent = {width, height, 1};
      vkCmdCopyImageToBuffer(cmdBuf, image, VK_IMAGE_LAYOUT_GENERAL, buffer, 1, &region);
    }
    break;
  }
}

VkDeviceMemory getStagingImage(VkImage &dstImage, VkImageCreateInfo ci)
{
  vkCreateImage(aux.device, &ci, nullptr, &dstImage);

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(aux.device, dstImage, &memRequirements);

  VkMemoryAllocateInfo memAllocInfo{};
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memRequirements.size;
  memAllocInfo.memoryTypeIndex = // try allocating in CPU memory first.
      MemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memRequirements.memoryTypeBits,
                      physicalDeviceMemoryProperties);
  if (memAllocInfo.memoryTypeIndex == -1)
    memAllocInfo.memoryTypeIndex = // if CPU is not possible, try GPU memory
      MemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memRequirements.memoryTypeBits,
                      physicalDeviceMemoryProperties);
  assert(memAllocInfo.memoryTypeIndex != -1);
  VkDeviceMemory dstImageMemory;
  vkAllocateMemory(aux.device, &memAllocInfo, nullptr, &dstImageMemory);

  vkBindImageMemory(aux.device, dstImage, dstImageMemory, 0);
  return dstImageMemory;
}

VkDeviceMemory getStagingBuffer(VkBuffer &dstBuffer, uint32_t width, uint32_t height, uint32_t bytes)
{
  VkBufferCreateInfo bufCI = {};
  bufCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufCI.size = VkDeviceSize(width * height * bytes);
  VkResult result = vkCreateBuffer(aux.device, &bufCI, NULL, &dstBuffer);
  assert(result == VK_SUCCESS);

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(aux.device, dstBuffer, &memRequirements);

  VkMemoryAllocateInfo memAllocInfo{};
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memRequirements.size;
  memAllocInfo.memoryTypeIndex =
      MemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      memRequirements.memoryTypeBits, physicalDeviceMemoryProperties);
  assert(memAllocInfo.memoryTypeIndex != -1);
  VkDeviceMemory dstMemory;
  vkAllocateMemory(aux.device, &memAllocInfo, NULL, &dstMemory);
  vkBindBufferMemory(aux.device, dstBuffer, dstMemory, 0);
  return dstMemory;
}

void copyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage,
               VkImageSubresourceRange srcRange, VkImageSubresourceRange dstRange, uint32_t width,
               uint32_t height, VkFormat format, bool msaa)
{
  assert(sizeof(VkImageCopy) == sizeof(VkImageResolve));
  union {
    VkImageCopy copy; // these structures are actually identical.
    VkImageResolve resolve;
  } region{};

  region.copy.srcSubresource.aspectMask = kImageAspectsAndByteSize[format].first;
  region.copy.srcSubresource.baseArrayLayer = srcRange.baseArrayLayer;
  region.copy.srcSubresource.mipLevel = srcRange.baseMipLevel;
  region.copy.srcSubresource.layerCount = 1;
  region.copy.dstSubresource.aspectMask = kImageAspectsAndByteSize[format].first;
  region.copy.dstSubresource.baseArrayLayer = dstRange.baseArrayLayer;
  region.copy.dstSubresource.mipLevel = dstRange.baseMipLevel;
  region.copy.dstSubresource.layerCount = 1;
  region.copy.extent.width = width;
  region.copy.extent.height = height;
  region.copy.extent.depth = 1;
  if (!msaa)
    vkCmdCopyImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_GENERAL, dstImage,
      VK_IMAGE_LAYOUT_GENERAL, 1, &region.copy);
  else
    vkCmdResolveImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_GENERAL, dstImage,
      VK_IMAGE_LAYOUT_GENERAL, 1, &region.resolve);
}

// copyFramebufferAttachments copies framebuffer attachments to host visible buffers,
// so they can be saved to disk later and returns the list of ReadbackInfo structures
// that were allocated and that store the attachments data.
ReadbackInfos copyFramebufferAttachments(VkCommandBuffer cmdBuf, RenderPassInfo *rpInfo)
{
  ReadbackInfos readbacks;
  for(int i = 0; i < rpInfo->attachments.size(); i++)
  {
    VkImageCreateInfo image_ci = rpInfo->attachments[i].image.ci;
    VkImageViewCreateInfo view_ci = rpInfo->attachments[i].view.ci;
    if(kImageAspectsAndByteSize.find(image_ci.format) == kImageAspectsAndByteSize.end())
    {
#if defined(_WIN32)
      OutputDebugStringA(std::string("Invalid format " + FormatToString(image_ci.format) + "\n").c_str());
#endif
      continue;
    }

    VkImage srcImage = rpInfo->attachments[i].image.res;
    VkImageSubresourceRange view_subresource = view_ci.subresourceRange;
    assert(view_subresource.layerCount == 1 && view_subresource.levelCount == 1);

    // Source image subresource is defined by it's view. For now assume only one layer is used.
    uint32_t mip = view_subresource.baseMipLevel;
    uint32_t layer = view_subresource.baseArrayLayer;
    uint32_t mip_width = std::max<uint32_t>(image_ci.extent.width >> mip, 1);
    uint32_t mip_height = std::max<uint32_t>(image_ci.extent.height >> mip, 1);

    VkImageSubresourceRange srcRange = { FullAspectFromFormat(image_ci.format),
      mip, 1, layer, 1};

    // Transition source image to VK_IMAGE_LAYOUT_GENERAL.
    ImageLayoutTransition(cmdBuf, srcImage, srcRange, VK_IMAGE_LAYOUT_GENERAL,
      VK_QUEUE_FAMILY_IGNORED, rpInfo->finalLayouts[i], VK_QUEUE_FAMILY_IGNORED);

    bool msaa = image_ci.samples != VK_SAMPLE_COUNT_1_BIT;
    VkImage stagingImage = NULL;
    VkDeviceMemory stagingImageMem = NULL;
    if (msaa) { // If MSAA we'll do an image->image resolve first.
      VkImageCreateInfo stagingCI{};
      stagingCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      stagingCI.imageType = VK_IMAGE_TYPE_2D;
      stagingCI.format = image_ci.format;
      stagingCI.extent.width = mip_width;
      stagingCI.extent.height = mip_height;
      stagingCI.extent.depth = 1;
      stagingCI.arrayLayers = 1;
      stagingCI.mipLevels = 1;
      stagingCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      stagingCI.samples = VK_SAMPLE_COUNT_1_BIT;
      stagingCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      stagingCI.tiling = VK_IMAGE_TILING_OPTIMAL;
      stagingCI.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
      stagingImageMem = getStagingImage(stagingImage, stagingCI);

      // Transition staging image to VK_IMAGE_LAYOUT_GENERAL.
      VkImageSubresourceRange dstRange = {FullAspectFromFormat(stagingCI.format),
        0, 1, 0, 1};

      ImageLayoutTransition(cmdBuf, stagingImage, dstRange,
        VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_IGNORED);

      copyImage(cmdBuf, srcImage, stagingImage, srcRange, dstRange, mip_width, mip_height, stagingCI.format, msaa);

      // Because srcImage is VK_IMAGE_LAYOUT_GENERAL we can just keep it like this for subsequent copy.
      // override these arguments since now we'll send the stagingImage down to imgToBuffer() function
      srcImage = stagingImage;
      mip = layer = 0;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory bufMem = getStagingBuffer(stagingBuffer, mip_width, mip_height,
      kImageAspectsAndByteSize[image_ci.format].second);
    imgToBuffer(cmdBuf, srcImage, stagingBuffer, mip_width, mip_height, mip, layer, image_ci.format);

    // Transition the real source image back to the original layout.
    ImageLayoutTransition(cmdBuf, rpInfo->attachments[i].image.res, srcRange, rpInfo->finalLayouts[i],
      VK_QUEUE_FAMILY_IGNORED, VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED);

    ReadbackInfo readback(rpInfo->attachments[i].image.res, 
                          stagingBuffer, stagingImage, bufMem, stagingImageMem,
                          mip_width, mip_height, image_ci.format, i);
    readbacks.attachments.push_back(readback);
  }

  return readbacks;
}

void screenshot(VkImage srcImage, const char *filename)
{
  uint32_t sw_width = swapchainCI.imageExtent.width;
  uint32_t sw_height = swapchainCI.imageExtent.height;
  VkFormat sw_format = swapchainCI.imageFormat;

  VkImageCreateInfo imageCreateCI{};
  imageCreateCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
  imageCreateCI.format = sw_format;
  imageCreateCI.extent.width = sw_width;
  imageCreateCI.extent.height = sw_height;
  imageCreateCI.extent.depth = 1;
  imageCreateCI.arrayLayers = 1;
  imageCreateCI.mipLevels = 1;
  imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMem = getStagingBuffer(stagingBuffer, sw_width, sw_height,
    kImageAspectsAndByteSize[sw_format].second);

  VkCommandBufferBeginInfo cmdbufBI{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  vkBeginCommandBuffer(aux.command_buffer, &cmdbufBI);
  {
    VkImageSubresourceRange fullRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    ImageLayoutTransition(aux, srcImage, fullRange,
      VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    imgToBuffer(aux.command_buffer, srcImage, stagingBuffer, sw_width, sw_height, 0, 0, sw_format);

    ImageLayoutTransition(aux, srcImage, fullRange,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_GENERAL);
  }
  vkEndCommandBuffer(aux.command_buffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &aux.command_buffer;
  vkQueueSubmit(aux.queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(aux.queue);

  bufferToPpm(stagingBuffer, stagingBufferMem, filename, sw_width, sw_height, sw_format);

  // cleanup
  vkDestroyBuffer(aux.device, stagingBuffer, NULL);
  vkFreeMemory(aux.device, stagingBufferMem, NULL);
}

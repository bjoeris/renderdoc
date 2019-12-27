/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 Baldur Karlsson
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

#include "vk_resources.h"
#include "maths/vec.h"
#include "vk_info.h"

WRAPPED_POOL_INST(WrappedVkInstance)
WRAPPED_POOL_INST(WrappedVkPhysicalDevice)
WRAPPED_POOL_INST(WrappedVkDevice)
WRAPPED_POOL_INST(WrappedVkQueue)
WRAPPED_POOL_INST(WrappedVkCommandBuffer)
WRAPPED_POOL_INST(WrappedVkFence)
WRAPPED_POOL_INST(WrappedVkDeviceMemory)
WRAPPED_POOL_INST(WrappedVkBuffer)
WRAPPED_POOL_INST(WrappedVkImage)
WRAPPED_POOL_INST(WrappedVkSemaphore)
WRAPPED_POOL_INST(WrappedVkEvent)
WRAPPED_POOL_INST(WrappedVkQueryPool)
WRAPPED_POOL_INST(WrappedVkBufferView)
WRAPPED_POOL_INST(WrappedVkImageView)
WRAPPED_POOL_INST(WrappedVkShaderModule)
WRAPPED_POOL_INST(WrappedVkPipelineCache)
WRAPPED_POOL_INST(WrappedVkPipelineLayout)
WRAPPED_POOL_INST(WrappedVkRenderPass)
WRAPPED_POOL_INST(WrappedVkPipeline)
WRAPPED_POOL_INST(WrappedVkDescriptorSetLayout)
WRAPPED_POOL_INST(WrappedVkSampler)
WRAPPED_POOL_INST(WrappedVkDescriptorPool)
WRAPPED_POOL_INST(WrappedVkDescriptorSet)
WRAPPED_POOL_INST(WrappedVkFramebuffer)
WRAPPED_POOL_INST(WrappedVkCommandPool)
WRAPPED_POOL_INST(WrappedVkSwapchainKHR)
WRAPPED_POOL_INST(WrappedVkSurfaceKHR)
WRAPPED_POOL_INST(WrappedVkDescriptorUpdateTemplate)
WRAPPED_POOL_INST(WrappedVkSamplerYcbcrConversion)

byte VkResourceRecord::markerValue[32] = {
    0xaa, 0xbb, 0xcc, 0xdd, 0x88, 0x77, 0x66, 0x55, 0x01, 0x23, 0x45, 0x67, 0x98, 0x76, 0x54, 0x32,
};

bool IsDispatchableRes(WrappedVkRes *ptr)
{
  return (WrappedVkPhysicalDevice::IsAlloc(ptr) || WrappedVkInstance::IsAlloc(ptr) ||
          WrappedVkDevice::IsAlloc(ptr) || WrappedVkQueue::IsAlloc(ptr) ||
          WrappedVkCommandBuffer::IsAlloc(ptr));
}

VkResourceType IdentifyTypeByPtr(WrappedVkRes *ptr)
{
  if(WrappedVkPhysicalDevice::IsAlloc(ptr))
    return eResPhysicalDevice;
  if(WrappedVkInstance::IsAlloc(ptr))
    return eResInstance;
  if(WrappedVkDevice::IsAlloc(ptr))
    return eResDevice;
  if(WrappedVkQueue::IsAlloc(ptr))
    return eResQueue;
  if(WrappedVkDeviceMemory::IsAlloc(ptr))
    return eResDeviceMemory;
  if(WrappedVkBuffer::IsAlloc(ptr))
    return eResBuffer;
  if(WrappedVkBufferView::IsAlloc(ptr))
    return eResBufferView;
  if(WrappedVkImage::IsAlloc(ptr))
    return eResImage;
  if(WrappedVkImageView::IsAlloc(ptr))
    return eResImageView;
  if(WrappedVkFramebuffer::IsAlloc(ptr))
    return eResFramebuffer;
  if(WrappedVkRenderPass::IsAlloc(ptr))
    return eResRenderPass;
  if(WrappedVkShaderModule::IsAlloc(ptr))
    return eResShaderModule;
  if(WrappedVkPipelineCache::IsAlloc(ptr))
    return eResPipelineCache;
  if(WrappedVkPipelineLayout::IsAlloc(ptr))
    return eResPipelineLayout;
  if(WrappedVkPipeline::IsAlloc(ptr))
    return eResPipeline;
  if(WrappedVkSampler::IsAlloc(ptr))
    return eResSampler;
  if(WrappedVkDescriptorPool::IsAlloc(ptr))
    return eResDescriptorPool;
  if(WrappedVkDescriptorSetLayout::IsAlloc(ptr))
    return eResDescriptorSetLayout;
  if(WrappedVkDescriptorSet::IsAlloc(ptr))
    return eResDescriptorSet;
  if(WrappedVkCommandPool::IsAlloc(ptr))
    return eResCommandPool;
  if(WrappedVkCommandBuffer::IsAlloc(ptr))
    return eResCommandBuffer;
  if(WrappedVkFence::IsAlloc(ptr))
    return eResFence;
  if(WrappedVkEvent::IsAlloc(ptr))
    return eResEvent;
  if(WrappedVkQueryPool::IsAlloc(ptr))
    return eResQueryPool;
  if(WrappedVkSemaphore::IsAlloc(ptr))
    return eResSemaphore;
  if(WrappedVkSwapchainKHR::IsAlloc(ptr))
    return eResSwapchain;
  if(WrappedVkSurfaceKHR::IsAlloc(ptr))
    return eResSurface;
  if(WrappedVkDescriptorUpdateTemplate::IsAlloc(ptr))
    return eResDescUpdateTemplate;
  if(WrappedVkSamplerYcbcrConversion::IsAlloc(ptr))
    return eResSamplerConversion;

  RDCERR("Unknown type for ptr 0x%p", ptr);

  return eResUnknown;
}

bool IsBlockFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return true;
    default: break;
  }

  return false;
}

bool IsDepthOrStencilFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return true;
    default: break;
  }

  return false;
}

bool IsDepthAndStencilFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return true;
    default: break;
  }

  return false;
}

bool IsStencilFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return true;
    default: break;
  }

  return false;
}

bool IsDepthOnlyFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT: return true;
    default: break;
  }

  return false;
}

bool IsStencilOnlyFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_S8_UINT: return true;
    default: break;
  }

  return false;
}

bool IsSRGBFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return true;
    default: break;
  }

  return false;
}

bool IsDoubleFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64A64_UINT: return true;
    default: break;
  }

  return false;
}

bool IsUIntFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64A64_UINT: return true;
    default: break;
  }

  return false;
}

bool IsSIntFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64A64_SINT: return true;
    default: break;
  }

  return false;
}

bool IsYUVFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_G8B8G8R8_422_UNORM:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
    case VK_FORMAT_R10X6_UNORM_PACK16:
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_R12X4_UNORM_PACK16:
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_G16B16G16R16_422_UNORM:
    case VK_FORMAT_B16G16R16G16_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: return true;
    default: break;
  }

  return false;
}

uint32_t GetYUVPlaneCount(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: return 2;
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: return 3;
    default: break;
  }

  return 1;
}

uint32_t GetYUVNumRows(VkFormat f, uint32_t height)
{
  switch(f)
  {
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
      // all of these are 4:2:0, so number of rows is equal to height + height/2
      return height + height / 2;
    default: break;
  }

  return height;
}

VkFormat GetYUVViewPlaneFormat(VkFormat f, uint32_t plane)
{
  switch(f)
  {
    case VK_FORMAT_G8B8G8R8_422_UNORM:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
    case VK_FORMAT_R10X6_UNORM_PACK16:
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
    case VK_FORMAT_R12X4_UNORM_PACK16:
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
    case VK_FORMAT_G16B16G16R16_422_UNORM:
    case VK_FORMAT_B16G16R16G16_422_UNORM: return f;
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM: return VK_FORMAT_R8_UNORM;
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
      return plane == 0 ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8G8_UNORM;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: return VK_FORMAT_R10X6_UNORM_PACK16;
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      return plane == 0 ? VK_FORMAT_R10X6_UNORM_PACK16 : VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: return VK_FORMAT_R12X4_UNORM_PACK16;
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      return plane == 0 ? VK_FORMAT_R12X4_UNORM_PACK16 : VK_FORMAT_R12X4G12X4_UNORM_2PACK16;
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: return VK_FORMAT_R16_UNORM;
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
      return plane == 0 ? VK_FORMAT_R16_UNORM : VK_FORMAT_R16G16_UNORM;
    default: break;
  }

  return f;
}

void GetYUVShaderParameters(VkFormat f, Vec4u &YUVDownsampleRate, Vec4u &YUVAChannels)
{
  if(IsYUVFormat(f))
  {
    ResourceFormat fmt = MakeResourceFormat(f);

    switch(fmt.YUVSubsampling())
    {
      case 444:
        YUVDownsampleRate.x = 1;
        YUVDownsampleRate.y = 1;
        break;
      case 422:
        YUVDownsampleRate.x = 2;
        YUVDownsampleRate.y = 1;
        break;
      case 420:
        YUVDownsampleRate.x = 2;
        YUVDownsampleRate.y = 2;
        break;
      default: break;
    }
    YUVDownsampleRate.z = fmt.YUVPlaneCount();
    switch(fmt.type)
    {
      case ResourceFormatType::YUV8: YUVDownsampleRate.w = 8; break;
      case ResourceFormatType::YUV10: YUVDownsampleRate.w = 10; break;
      case ResourceFormatType::YUV12: YUVDownsampleRate.w = 12; break;
      case ResourceFormatType::YUV16: YUVDownsampleRate.w = 16; break;
      default: break;
    }
    switch(f)
    {
      case VK_FORMAT_G8B8G8R8_422_UNORM: YUVAChannels = {0, 2, 1, 0xff}; break;
      case VK_FORMAT_B8G8R8G8_422_UNORM: YUVAChannels = {0, 2, 1, 0xff}; break;
      case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: YUVAChannels = {0, 2, 1, 0xff}; break;
      case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM: YUVAChannels = {0, 4, 5, 0xff}; break;
      case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM: YUVAChannels = {0, 4, 8, 0xff}; break;
      case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM: YUVAChannels = {0, 4, 5, 0xff}; break;
      case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM: YUVAChannels = {0, 4, 8, 0xff}; break;
      case VK_FORMAT_R10X6_UNORM_PACK16: YUVAChannels = {0, 0xff, 0xff, 0xff}; break;
      case VK_FORMAT_R10X6G10X6_UNORM_2PACK16: YUVAChannels = {0xff, 0, 1, 0xff}; break;
      case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16: YUVAChannels = {1, 2, 0, 3}; break;
      case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: YUVAChannels = {0, 2, 1, 0xff}; break;
      case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: YUVAChannels = {0, 2, 1, 0xff}; break;
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        YUVAChannels = {0, 4, 8, 0xff};
        break;
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        YUVAChannels = {0, 4, 5, 0xff};
        break;
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        YUVAChannels = {0, 4, 8, 0xff};
        break;
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        YUVAChannels = {0, 4, 5, 0xff};
        break;
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        YUVAChannels = {0, 4, 8, 0xff};
        break;
      case VK_FORMAT_R12X4_UNORM_PACK16: YUVAChannels = {0, 0xff, 0xff, 0xff}; break;
      case VK_FORMAT_R12X4G12X4_UNORM_2PACK16: YUVAChannels = {0xff, 0, 1, 0xff}; break;
      case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16: YUVAChannels = {1, 2, 0, 3}; break;
      case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: YUVAChannels = {0, 2, 1, 0xff}; break;
      case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: YUVAChannels = {0, 2, 1, 0xff}; break;
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        YUVAChannels = {0, 4, 8, 0xff};
        break;
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        YUVAChannels = {0, 4, 5, 0xff};
        break;
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        YUVAChannels = {0, 4, 8, 0xff};
        break;
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        YUVAChannels = {0, 4, 5, 0xff};
        break;
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        YUVAChannels = {0, 4, 8, 0xff};
        break;
      case VK_FORMAT_G16B16G16R16_422_UNORM: YUVAChannels = {0, 2, 1, 0xff}; break;
      case VK_FORMAT_B16G16R16G16_422_UNORM: YUVAChannels = {0, 2, 1, 0xff}; break;
      case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM: YUVAChannels = {0, 4, 8, 0xff}; break;
      case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM: YUVAChannels = {0, 4, 5, 0xff}; break;
      case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM: YUVAChannels = {0, 4, 8, 0xff}; break;
      case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM: YUVAChannels = {0, 4, 5, 0xff}; break;
      case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: YUVAChannels = {0, 4, 8, 0xff}; break;
      default: break;
    }
  }
}

VkFormat GetDepthOnlyFormat(VkFormat f)
{
  switch(f)
  {
    case VK_FORMAT_D16_UNORM_S8_UINT: return VK_FORMAT_D16_UNORM;
    case VK_FORMAT_D24_UNORM_S8_UINT: return VK_FORMAT_X8_D24_UNORM_PACK32;
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT;
    default: break;
  }

  return f;
}

VkFormat GetViewCastedFormat(VkFormat f, CompType typeCast)
{
  if(typeCast == CompType::Typeless)
    return f;

  switch(f)
  {
    case VK_FORMAT_R64G64B64A64_UINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    {
      if(typeCast == CompType::UInt)
        return VK_FORMAT_R64G64B64A64_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R64G64B64A64_SINT;
      else
        return VK_FORMAT_R64G64B64A64_SFLOAT;
    }
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    {
      if(typeCast == CompType::UInt)
        return VK_FORMAT_R64G64B64_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R64G64B64_SINT;
      else
        return VK_FORMAT_R64G64B64_SFLOAT;
    }
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT:
    {
      if(typeCast == CompType::UInt)
        return VK_FORMAT_R64G64_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R64G64_SINT;
      else
        return VK_FORMAT_R64G64_SFLOAT;
    }
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_SFLOAT:
    {
      if(typeCast == CompType::UInt)
        return VK_FORMAT_R64_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R64_SINT;
      else
        return VK_FORMAT_R64_SFLOAT;
    }
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    {
      if(typeCast == CompType::UInt)
        return VK_FORMAT_R32G32B32A32_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R32G32B32A32_SINT;
      else
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    {
      if(typeCast == CompType::UInt)
        return VK_FORMAT_R32G32B32_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R32G32B32_SINT;
      else
        return VK_FORMAT_R32G32B32_SFLOAT;
    }
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    {
      if(typeCast == CompType::UInt)
        return VK_FORMAT_R32G32_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R32G32_SINT;
      else
        return VK_FORMAT_R32G32_SFLOAT;
    }
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT:
    {
      if(typeCast == CompType::UInt)
        return VK_FORMAT_R32_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R32_SINT;
      else if(typeCast == CompType::Depth)
        return VK_FORMAT_D32_SFLOAT;
      else
        return VK_FORMAT_R32_SFLOAT;
    }
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_USCALED:
    case VK_FORMAT_R16G16B16A16_SSCALED:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    {
      if(typeCast == CompType::UNorm || typeCast == CompType::UNormSRGB)
        return VK_FORMAT_R16G16B16A16_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_R16G16B16A16_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_R16G16B16A16_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_R16G16B16A16_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_R16G16B16A16_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R16G16B16A16_SINT;
      else
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    }
    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16_USCALED:
    case VK_FORMAT_R16G16B16_SSCALED:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    {
      if(typeCast == CompType::UNorm || typeCast == CompType::UNormSRGB)
        return VK_FORMAT_R16G16B16_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_R16G16B16_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_R16G16B16_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_R16G16B16_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_R16G16B16_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R16G16B16_SINT;
      else
        return VK_FORMAT_R16G16B16_SFLOAT;
    }
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_USCALED:
    case VK_FORMAT_R16G16_SSCALED:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    {
      if(typeCast == CompType::UNorm || typeCast == CompType::UNormSRGB)
        return VK_FORMAT_R16G16_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_R16G16_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_R16G16_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_R16G16_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_R16G16_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R16G16_SINT;
      else
        return VK_FORMAT_R16G16_SFLOAT;
    }
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_USCALED:
    case VK_FORMAT_R16_SSCALED:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_D16_UNORM:
    {
      if(typeCast == CompType::UNorm || typeCast == CompType::UNormSRGB)
        return VK_FORMAT_R16_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_R16_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_R16_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_R16_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_R16_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R16_SINT;
      else if(typeCast == CompType::Depth)
        return VK_FORMAT_D16_UNORM;
      else
        return VK_FORMAT_R16_SFLOAT;
    }
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SRGB:
    {
      if(typeCast == CompType::UNorm)
        return VK_FORMAT_R8G8B8A8_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_R8G8B8A8_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_R8G8B8A8_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_R8G8B8A8_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_R8G8B8A8_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R8G8B8A8_SINT;
      else if(typeCast == CompType::UNormSRGB)
        return VK_FORMAT_R8G8B8A8_SRGB;
      else
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SRGB:
    {
      if(typeCast == CompType::UNorm)
        return VK_FORMAT_B8G8R8A8_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_B8G8R8A8_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_B8G8R8A8_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_B8G8R8A8_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_B8G8R8A8_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_B8G8R8A8_SINT;
      else if(typeCast == CompType::UNormSRGB)
        return VK_FORMAT_B8G8R8A8_SRGB;
      else
        return VK_FORMAT_B8G8R8A8_UNORM;
    }
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    {
      if(typeCast == CompType::UNorm)
        return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_A8B8G8R8_UINT_PACK32;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_A8B8G8R8_SINT_PACK32;
      else if(typeCast == CompType::UNormSRGB)
        return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
      else
        return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    }
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_USCALED:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8_SRGB:
    {
      if(typeCast == CompType::UNorm)
        return VK_FORMAT_R8G8B8_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_R8G8B8_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_R8G8B8_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_R8G8B8_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_R8G8B8_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R8G8B8_SINT;
      else if(typeCast == CompType::UNormSRGB)
        return VK_FORMAT_R8G8B8_SRGB;
      else
        return VK_FORMAT_R8G8B8_UNORM;
    }
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_USCALED:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8_SRGB:
    {
      if(typeCast == CompType::UNorm)
        return VK_FORMAT_B8G8R8_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_B8G8R8_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_B8G8R8_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_B8G8R8_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_B8G8R8_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_B8G8R8_SINT;
      else if(typeCast == CompType::UNormSRGB)
        return VK_FORMAT_B8G8R8_SRGB;
      else
        return VK_FORMAT_B8G8R8_UNORM;
    }
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_USCALED:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8_SRGB:
    {
      if(typeCast == CompType::UNorm)
        return VK_FORMAT_R8G8_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_R8G8_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_R8G8_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_R8G8_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_R8G8_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R8G8_SINT;
      else if(typeCast == CompType::UNormSRGB)
        return VK_FORMAT_R8G8_SRGB;
      else
        return VK_FORMAT_R8G8_UNORM;
    }
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_S8_UINT:
    {
      if(typeCast == CompType::UNorm)
        return VK_FORMAT_R8_UNORM;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_R8_SNORM;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_R8_USCALED;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_R8_SSCALED;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_R8_UINT;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_R8_SINT;
      else if(typeCast == CompType::UNormSRGB)
        return VK_FORMAT_R8_SRGB;
      else if(typeCast == CompType::Depth)
        return VK_FORMAT_S8_UINT;
      else
        return VK_FORMAT_R8_UNORM;
    }
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
    {
      if(typeCast == CompType::UNorm || typeCast == CompType::UNormSRGB)
        return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_A2B10G10R10_UINT_PACK32;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_A2B10G10R10_SINT_PACK32;
      else
        return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    }
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    {
      if(typeCast == CompType::UNorm || typeCast == CompType::UNormSRGB)
        return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
      else if(typeCast == CompType::SNorm)
        return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
      else if(typeCast == CompType::UScaled)
        return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
      else if(typeCast == CompType::SScaled)
        return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
      else if(typeCast == CompType::UInt)
        return VK_FORMAT_A2R10G10B10_UINT_PACK32;
      else if(typeCast == CompType::SInt)
        return VK_FORMAT_A2R10G10B10_SINT_PACK32;
      else
        return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    }
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_BC1_RGB_SRGB_BLOCK
                                               : VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_BC1_RGBA_SRGB_BLOCK
                                               : VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
      return (typeCast == CompType::SNorm) ? VK_FORMAT_BC4_SNORM_BLOCK : VK_FORMAT_BC4_UNORM_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK
                                               : VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK
                                               : VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK
                                               : VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
      return (typeCast == CompType::SNorm) ? VK_FORMAT_EAC_R11_SNORM_BLOCK
                                           : VK_FORMAT_EAC_R11_UNORM_BLOCK;
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
      return (typeCast == CompType::SNorm) ? VK_FORMAT_EAC_R11G11_SNORM_BLOCK
                                           : VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_BC2_SRGB_BLOCK : VK_FORMAT_BC2_UNORM_BLOCK;
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK;
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
      return (typeCast == CompType::SNorm) ? VK_FORMAT_BC5_SNORM_BLOCK : VK_FORMAT_BC5_UNORM_BLOCK;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
      return (typeCast == CompType::SNorm) ? VK_FORMAT_BC6H_SFLOAT_BLOCK
                                           : VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK;
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_4x4_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_5x4_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_5x5_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_6x5_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_6x6_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_8x5_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_8x6_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_8x8_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_10x5_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_10x6_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_10x8_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_10x10_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_12x10_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_ASTC_12x12_SRGB_BLOCK
                                               : VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG
                                               : VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG
                                               : VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG
                                               : VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
      return (typeCast == CompType::UNormSRGB) ? VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG
                                               : VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;

    // all other formats have no aliases so nothing to typecast
    default: break;
  }

  return f;
}

// The shape of blocks in (a plane of) an image format.
// Non-block formats are considered to have 1x1 blocks.
// For some planar formats, the block shape depends on the plane--
// e.g. VK_FORMAT_G8_B8R8_2PLANE_422_UNORM has 8 bits per 1x1 block in plane 0, but 16 bits per 1x1
// block in plane 1.
struct BlockShape
{
  // the width of a block, in texels (or 1 for non-block formats)
  uint32_t width;

  // the height of a block, in texels (or 1 for non-block formats)
  uint32_t height;

  // the number of bytes used to encode the block
  uint32_t bytes;
};

BlockShape GetBlockShape(VkFormat Format, uint32_t plane)
{
  switch(Format)
  {
    case VK_FORMAT_R64G64B64A64_UINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_SFLOAT: return {1, 1, 32};
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_SFLOAT: return {1, 1, 24};
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT: return {1, 1, 16};
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT: return {1, 1, 12};
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_USCALED:
    case VK_FORMAT_R16G16B16A16_SSCALED:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_SFLOAT: return {1, 1, 8};
    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16_USCALED:
    case VK_FORMAT_R16G16B16_SSCALED:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT: return {1, 1, 6};
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return {1, 1, 8};
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_USCALED:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_USCALED:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8_SRGB: return {1, 1, 3};
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_USCALED:
    case VK_FORMAT_R16G16_SSCALED:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return {1, 1, 4};
    case VK_FORMAT_D16_UNORM_S8_UINT: return {1, 1, 4};
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_USCALED:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_USCALED:
    case VK_FORMAT_R16_SSCALED:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return {1, 1, 2};
    case VK_FORMAT_R4G4_UNORM_PACK8:
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_S8_UINT: return {1, 1, 1};
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11_SNORM_BLOCK: return {4, 4, 8};
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return {4, 4, 16};
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT: return {4, 4, 16};
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT: return {5, 4, 16};
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT: return {5, 5, 16};
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT: return {6, 5, 16};
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT: return {6, 6, 16};
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT: return {8, 5, 16};
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT: return {8, 6, 16};
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT: return {8, 8, 16};
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT: return {10, 5, 16};
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT: return {10, 6, 16};
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT: return {10, 8, 16};
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT: return {10, 10, 16};
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT: return {12, 10, 16};
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT: return {12, 12, 16};

    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: return {8, 4, 8};
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
      return {4, 4, 8};

    /*
     * YUV planar/packed subsampled textures.
     *
     * In each diagram we indicate (maybe part) of the data for a 4x4 texture:
     *
     * +---+---+---+---+
     * | 0 | 1 | 2 | 3 |
     * +---+---+---+---+
     * | 4 | 5 | 6 | 7 |
     * +---+---+---+---+
     * | 8 | 9 | A | B |
     * +---+---+---+---+
     * | C | D | E | F |
     * +---+---+---+---+
     *
     *
     * FOURCC decoding:
     *  - char 0: 'Y' = packed, 'P' = planar
     *  - char 1: '4' = 4:4:4, '2' = 4:2:2, '1' = 4:2:1, '0' = 4:2:0
     *  - char 2+3: '16' = 16-bit, '10' = 10-bit, '08' = 8-bit
     *
     * planar = Y is first, all together, then UV comes second.
     * packed = YUV is interleaved
     *
     * ======================= 4:4:4 lossless packed =========================
     *
     * Equivalent to uncompressed formats, just YUV instead of RGB. For 8-bit:
     *
     * pixel:      0            1            2            3
     * byte:  0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F
     *        Y0 U0 V0 A0  Y1 U1 V1 A1  Y2 U2 V2 A2  Y3 U3 V3 A3
     *
     * 16-bit is similar with two bytes per sample, 10-bit for uncompressed is
     * equivalent to R10G10B10A2 but with RGB=>YUV
     *
     * ============================ 4:2:2 packed =============================
     *
     * 50% horizontal subsampling packed, two Y samples for each U/V sample pair. For 8-bit:
     *
     * pixel:   0  |  1      2  |  3      4  |  5      6  |  7
     * byte:  0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F
     *        Y0 U0 Y1 V0  Y2 U1 Y3 V1  Y4 U2 Y5 V2  Y6 U3 Y7 V3
     *
     * 16-bit is similar with two bytes per sample, 10-bit is stored identically to 16-bit but in
     * the most significant bits:
     *
     * bit:    FEDCBA9876543210
     * 16-bit: XXXXXXXXXXXXXXXX
     * 10-bit: XXXXXXXXXX000000
     *
     * Since the data is unorm this just spaces out valid values.
     *
     * ============================ 4:2:0 planar =============================
     *
     * 50% horizontal and vertical subsampled planar, four Y samples for each U/V sample pair.
     * For 8-bit:
     *
     *
     * pixel: 0  1  2  3   4  5  6  7
     * byte:  0  1  2  3   4  5  6  7
     *        Y0 Y1 Y2 Y3  Y4 Y5 Y6 Y7
     *
     * pixel: 8  9  A  B   C  D  E  F
     * byte:  8  9  A  B   C  D  E  F
     *        Y8 Y9 Ya Yb  Yc Yd Ye Yf
     *
     *        ... all of the rest of Y luma ...
     *
     * pixel:  T&4 | 1&5    2&6 | 3&7
     * byte:  0  1  2  3   4  5  6  7
     *        U0 V0 U1 V1  U2 V2 U3 V3
     *
     * pixel:  8&C | 9&D    A&E | B&F
     * byte:  8  9  A  B   C  D  E  F
     *        U4 V4 U5 V5  U6 V6 U7 V7
     */
    case VK_FORMAT_G8B8G8R8_422_UNORM:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
      // 4:2:2 packed 8-bit, so 1 byte per pixel for luma and 1 byte per pixel for chroma (2 chroma
      // samples, with 50% subsampling = 1 byte per pixel)
      return {2, 1, 4};
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM: return {1, 1, 1};
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
      if(plane == 0)
      {
        return {1, 1, 1};
      }
      else if(plane == 1)
      {
        return {1, 1, 2};
      }
      else
      {
        RDCERR("Invalid plane %d in 2-plane format", plane);
        return {1, 1, 1};
      }
    case VK_FORMAT_R10X6_UNORM_PACK16:
    case VK_FORMAT_R12X4_UNORM_PACK16:
      // basically just 16-bit format with only top 10-bits used
      // 10-bit and 12-bit formats are stored identically to 16-bit formats
      return {1, 1, 2};
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
      // just a 16-bit format with only top N-bits used
      // 10-bit and 12-bit formats are stored identically to 16-bit formats
      return {1, 1, 4};
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
      // just a 16-bit format with only top N-bits used
      // 10-bit and 12-bit formats are stored identically to 16-bit formats
      return {1, 1, 8};
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
    case VK_FORMAT_G16B16G16R16_422_UNORM:
    case VK_FORMAT_B16G16R16G16_422_UNORM:
      // 10-bit and 12-bit formats are stored identically to 16-bit formats
      // 4:2:2 packed 16-bit
      return {2, 1, 8};
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: return {1, 1, 2};
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
      if(plane == 0)
      {
        return {1, 1, 2};
      }
      else if(plane == 1)
      {
        return {1, 1, 4};
      }
      else
      {
        RDCERR("Invalid plane %d in 2-plane format", plane);
        return {1, 1, 2};
      }
    default: RDCERR("Unrecognised Vulkan Format: %d", Format);
  }

  return {1, 1, 1};
}

VkExtent2D GetPlaneShape(uint32_t Width, uint32_t Height, VkFormat Format, uint32_t plane)
{
  switch(Format)
  {
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
      if(plane == 0)
        return {Width, Height};
      else
        return {RDCMAX(1U, (Width + 1) / 2), RDCMAX(1U, (Height + 1) / 2)};

    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
      if(plane == 0)
        return {Width, Height};
      else
        return {RDCMAX(1U, (Width + 1) / 2), Height};

    default: return {Width, Height};
  }
}

uint32_t GetPlaneByteSize(uint32_t Width, uint32_t Height, uint32_t Depth, VkFormat Format,
                          uint32_t mip, uint32_t plane)
{
  uint32_t mipWidth = RDCMAX(Width >> mip, 1U);
  uint32_t mipHeight = RDCMAX(Height >> mip, 1U);
  uint32_t mipDepth = RDCMAX(Depth >> mip, 1U);

  VkExtent2D planeShape = GetPlaneShape(mipWidth, mipHeight, Format, plane);

  BlockShape blockShape = GetBlockShape(Format, plane);

  uint32_t widthInBlocks = (planeShape.width + blockShape.width - 1) / blockShape.width;
  uint32_t heightInBlocks = (planeShape.height + blockShape.height - 1) / blockShape.height;

  return blockShape.bytes * widthInBlocks * heightInBlocks * mipDepth;
}

uint32_t GetByteSize(uint32_t Width, uint32_t Height, uint32_t Depth, VkFormat Format, uint32_t mip)
{
  uint32_t planeCount = GetYUVPlaneCount(Format);
  uint32_t size = 0;
  for(uint32_t p = 0; p < planeCount; p++)
    size += GetPlaneByteSize(Width, Height, Depth, Format, mip, p);
  return size;
}

ResourceFormat MakeResourceFormat(VkFormat fmt)
{
  ResourceFormat ret;

  ret.type = ResourceFormatType::Regular;
  ret.compByteWidth = 0;
  ret.compCount = 0;
  ret.compType = CompType::Typeless;

  if(fmt == VK_FORMAT_UNDEFINED)
  {
    ret.type = ResourceFormatType::Undefined;
    return ret;
  }

  switch(fmt)
  {
    case VK_FORMAT_R4G4_UNORM_PACK8: ret.type = ResourceFormatType::R4G4; break;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16: ret.type = ResourceFormatType::R4G4B4A4; break;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32: ret.type = ResourceFormatType::R10G10B10A2; break;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32: ret.type = ResourceFormatType::R11G11B10; break;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: ret.type = ResourceFormatType::R9G9B9E5; break;
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_B5G6R5_UNORM_PACK16: ret.type = ResourceFormatType::R5G6B5; break;
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16: ret.type = ResourceFormatType::R5G5B5A1; break;
    case VK_FORMAT_D16_UNORM_S8_UINT: ret.type = ResourceFormatType::D16S8; break;
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D24_UNORM_S8_UINT: ret.type = ResourceFormatType::D24S8; break;
    case VK_FORMAT_D32_SFLOAT_S8_UINT: ret.type = ResourceFormatType::D32S8; break;
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: ret.type = ResourceFormatType::BC1; break;
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK: ret.type = ResourceFormatType::BC2; break;
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK: ret.type = ResourceFormatType::BC3; break;
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK: ret.type = ResourceFormatType::BC4; break;
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK: ret.type = ResourceFormatType::BC5; break;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK: ret.type = ResourceFormatType::BC6; break;
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK: ret.type = ResourceFormatType::BC7; break;
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: ret.type = ResourceFormatType::ETC2; break;
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: ret.type = ResourceFormatType::EAC; break;
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT: ret.type = ResourceFormatType::ASTC; break;
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: ret.type = ResourceFormatType::PVRTC; break;
    case VK_FORMAT_G8B8G8R8_422_UNORM:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM: ret.type = ResourceFormatType::YUV8; break;
    case VK_FORMAT_R10X6_UNORM_PACK16:
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      ret.type = ResourceFormatType::YUV10;
      break;
    case VK_FORMAT_R12X4_UNORM_PACK16:
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      ret.type = ResourceFormatType::YUV12;
      break;
    case VK_FORMAT_G16B16G16R16_422_UNORM:
    case VK_FORMAT_B16G16R16G16_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: ret.type = ResourceFormatType::YUV16; break;
    default: break;
  }

  switch(fmt)
  {
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_USCALED:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
    case VK_FORMAT_B16G16R16G16_422_UNORM: ret.SetBGRAOrder(true); break;
    default: break;
  }

  switch(fmt)
  {
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_USCALED:
    case VK_FORMAT_R16_SSCALED:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
    case VK_FORMAT_R10X6_UNORM_PACK16:
    case VK_FORMAT_R12X4_UNORM_PACK16: ret.compCount = 1; break;
    case VK_FORMAT_R4G4_UNORM_PACK8:
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_USCALED:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_USCALED:
    case VK_FORMAT_R16G16_SSCALED:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16: ret.compCount = 2; break;
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_USCALED:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16_USCALED:
    case VK_FORMAT_R16G16B16_SSCALED:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_USCALED:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_G8B8G8R8_422_UNORM:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_G16B16G16R16_422_UNORM:
    case VK_FORMAT_B16G16R16G16_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: ret.compCount = 3; break;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_USCALED:
    case VK_FORMAT_R16G16B16A16_SSCALED:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64B64A64_UINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16: ret.compCount = 4; break;
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT: ret.compCount = 4; break;
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: ret.compCount = 4; break;
    case VK_FORMAT_UNDEFINED:
    case VK_FORMAT_RANGE_SIZE:
    case VK_FORMAT_MAX_ENUM: ret.compCount = 1; break;
  }

  switch(fmt)
  {
    case VK_FORMAT_R4G4_UNORM_PACK8:
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG: ret.compType = CompType::UNorm; break;
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: ret.compType = CompType::UNormSRGB; break;
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32: ret.compType = CompType::SNorm; break;
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_R8G8_USCALED:
    case VK_FORMAT_R8G8B8_USCALED:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
    case VK_FORMAT_R16_USCALED:
    case VK_FORMAT_R16G16_USCALED:
    case VK_FORMAT_R16G16B16_USCALED:
    case VK_FORMAT_R16G16B16A16_USCALED:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_B8G8R8_USCALED:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32: ret.compType = CompType::UScaled; break;
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_R16_SSCALED:
    case VK_FORMAT_R16G16_SSCALED:
    case VK_FORMAT_R16G16B16_SSCALED:
    case VK_FORMAT_R16G16B16A16_SSCALED:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: ret.compType = CompType::SScaled; break;
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64A64_UINT:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32: ret.compType = CompType::UInt; break;
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32: ret.compType = CompType::SInt; break;
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT: ret.compType = CompType::Float; break;
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R64G64B64A64_SFLOAT: ret.compType = CompType::Double; break;
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT: ret.compType = CompType::Depth; break;
    case VK_FORMAT_G8B8G8R8_422_UNORM:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
    case VK_FORMAT_R10X6_UNORM_PACK16:
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_R12X4_UNORM_PACK16:
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_G16B16G16R16_422_UNORM:
    case VK_FORMAT_B16G16R16G16_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: ret.compType = CompType::UNorm; break;
    case VK_FORMAT_UNDEFINED:
    case VK_FORMAT_RANGE_SIZE:
    case VK_FORMAT_MAX_ENUM: ret.compType = CompType::Typeless; break;
  }

  switch(fmt)
  {
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_USCALED:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_USCALED:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_USCALED:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SRGB: ret.compByteWidth = 1; break;
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_USCALED:
    case VK_FORMAT_R16_SSCALED:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_USCALED:
    case VK_FORMAT_R16G16_SSCALED:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16_USCALED:
    case VK_FORMAT_R16G16B16_SSCALED:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_USCALED:
    case VK_FORMAT_R16G16B16A16_SSCALED:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_D16_UNORM: ret.compByteWidth = 2; break;
    case VK_FORMAT_X8_D24_UNORM_PACK32: ret.compByteWidth = 3; break;
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT: ret.compByteWidth = 4; break;
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64A64_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R64G64B64A64_SFLOAT: ret.compByteWidth = 8; break;
    case VK_FORMAT_R4G4_UNORM_PACK8:
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT:
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: ret.compByteWidth = 1; break;
    case VK_FORMAT_G8B8G8R8_422_UNORM:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
    case VK_FORMAT_R10X6_UNORM_PACK16:
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_R12X4_UNORM_PACK16:
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
    case VK_FORMAT_G16B16G16R16_422_UNORM:
    case VK_FORMAT_B16G16R16G16_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: ret.compByteWidth = 2; break;
    case VK_FORMAT_UNDEFINED:
    case VK_FORMAT_RANGE_SIZE:
    case VK_FORMAT_MAX_ENUM: ret.compByteWidth = 1; break;
  }

  if(IsYUVFormat(fmt))
  {
    ret.SetYUVPlaneCount(GetYUVPlaneCount(fmt));

    switch(fmt)
    {
      case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
      case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
      case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
      case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM: ret.SetYUVSubsampling(420); break;
      case VK_FORMAT_G8B8G8R8_422_UNORM:
      case VK_FORMAT_B8G8R8G8_422_UNORM:
      case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
      case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
      case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      case VK_FORMAT_G16B16G16R16_422_UNORM:
      case VK_FORMAT_B16G16R16G16_422_UNORM:
      case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
      case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM: ret.SetYUVSubsampling(422); break;
      case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
      case VK_FORMAT_R10X6_UNORM_PACK16:
      case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
      case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
      case VK_FORMAT_R12X4_UNORM_PACK16:
      case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
      case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16: ret.SetYUVSubsampling(444); break;
      default: break;
    }
  }

  return ret;
}

VkFormat MakeVkFormat(ResourceFormat fmt)
{
  VkFormat ret = VK_FORMAT_UNDEFINED;

  if(fmt.Special())
  {
    switch(fmt.type)
    {
      case ResourceFormatType::Undefined: return ret;
      case ResourceFormatType::BC1:
      {
        if(fmt.compCount == 3)
          ret = fmt.SRGBCorrected() ? VK_FORMAT_BC1_RGB_SRGB_BLOCK : VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        else
          ret = fmt.SRGBCorrected() ? VK_FORMAT_BC1_RGBA_SRGB_BLOCK : VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        break;
      }
      case ResourceFormatType::BC2:
        ret = fmt.SRGBCorrected() ? VK_FORMAT_BC2_SRGB_BLOCK : VK_FORMAT_BC2_UNORM_BLOCK;
        break;
      case ResourceFormatType::BC3:
        ret = fmt.SRGBCorrected() ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK;
        break;
      case ResourceFormatType::BC4:
        ret = fmt.compType == CompType::SNorm ? VK_FORMAT_BC4_SNORM_BLOCK : VK_FORMAT_BC4_UNORM_BLOCK;
        break;
      case ResourceFormatType::BC5:
        ret = fmt.compType == CompType::SNorm ? VK_FORMAT_BC5_SNORM_BLOCK : VK_FORMAT_BC5_UNORM_BLOCK;
        break;
      case ResourceFormatType::BC6:
        ret = fmt.compType == CompType::SNorm ? VK_FORMAT_BC6H_SFLOAT_BLOCK
                                              : VK_FORMAT_BC6H_UFLOAT_BLOCK;
        break;
      case ResourceFormatType::BC7:
        ret = fmt.SRGBCorrected() ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK;
        break;
      case ResourceFormatType::ETC2:
      {
        if(fmt.compCount == 3)
          ret = fmt.SRGBCorrected() ? VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK
                                    : VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        else
          ret = fmt.SRGBCorrected() ? VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK
                                    : VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        break;
      }
      case ResourceFormatType::EAC:
      {
        if(fmt.compCount == 1)
          ret = fmt.compType == CompType::SNorm ? VK_FORMAT_EAC_R11_SNORM_BLOCK
                                                : VK_FORMAT_EAC_R11_UNORM_BLOCK;
        else if(fmt.compCount == 2)
          ret = fmt.compType == CompType::SNorm ? VK_FORMAT_EAC_R11G11_SNORM_BLOCK
                                                : VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        else
          ret = fmt.SRGBCorrected() ? VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK
                                    : VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        break;
      }
      case ResourceFormatType::R10G10B10A2:
        if(fmt.compType == CompType::UNorm)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_UNORM_PACK32
                                : VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        else if(fmt.compType == CompType::UInt)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_UINT_PACK32
                                : VK_FORMAT_A2B10G10R10_UINT_PACK32;
        else if(fmt.compType == CompType::UScaled)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_USCALED_PACK32
                                : VK_FORMAT_A2B10G10R10_USCALED_PACK32;
        else if(fmt.compType == CompType::SNorm)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_SNORM_PACK32
                                : VK_FORMAT_A2B10G10R10_SNORM_PACK32;
        else if(fmt.compType == CompType::SInt)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_SINT_PACK32
                                : VK_FORMAT_A2B10G10R10_SINT_PACK32;
        else if(fmt.compType == CompType::SScaled)
          ret = fmt.BGRAOrder() ? VK_FORMAT_A2R10G10B10_SSCALED_PACK32
                                : VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
        break;
      case ResourceFormatType::R11G11B10: ret = VK_FORMAT_B10G11R11_UFLOAT_PACK32; break;
      case ResourceFormatType::R5G6B5:
        ret = fmt.BGRAOrder() ? VK_FORMAT_B5G6R5_UNORM_PACK16 : VK_FORMAT_R5G6B5_UNORM_PACK16;
        break;
      case ResourceFormatType::R5G5B5A1:
        ret = fmt.BGRAOrder() ? VK_FORMAT_B5G5R5A1_UNORM_PACK16 : VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        break;
      case ResourceFormatType::R9G9B9E5: ret = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32; break;
      case ResourceFormatType::R4G4B4A4:
        ret = fmt.BGRAOrder() ? VK_FORMAT_B4G4R4A4_UNORM_PACK16 : VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        break;
      case ResourceFormatType::R4G4: ret = VK_FORMAT_R4G4_UNORM_PACK8; break;
      case ResourceFormatType::D16S8: ret = VK_FORMAT_D16_UNORM_S8_UINT; break;
      case ResourceFormatType::D24S8: ret = VK_FORMAT_D24_UNORM_S8_UINT; break;
      case ResourceFormatType::D32S8: ret = VK_FORMAT_D32_SFLOAT_S8_UINT; break;
      case ResourceFormatType::YUV8:
      {
        int subsampling = fmt.YUVSubsampling();
        int planeCount = fmt.YUVPlaneCount();

        // don't support anything but 3 components
        if(fmt.compCount != 3)
          return VK_FORMAT_UNDEFINED;

        if(subsampling == 444)
        {
          // only support 3-plane
          return planeCount == 3 ? VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM : VK_FORMAT_UNDEFINED;
        }
        else if(subsampling == 422)
        {
          if(planeCount == 1)
            return fmt.BGRAOrder() ? VK_FORMAT_B8G8R8G8_422_UNORM : VK_FORMAT_G8B8G8R8_422_UNORM;
          else if(planeCount == 2)
            return VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
          else if(planeCount == 3)
            return VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM;
        }
        else if(subsampling == 420)
        {
          if(planeCount == 2)
            return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
          else if(planeCount == 3)
            return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
          else
            return VK_FORMAT_UNDEFINED;
        }

        return VK_FORMAT_UNDEFINED;
      }
      case ResourceFormatType::YUV10:
      {
        int subsampling = fmt.YUVSubsampling();
        int planeCount = fmt.YUVPlaneCount();

        if(fmt.compCount == 1)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R10X6_UNORM_PACK16;
          return VK_FORMAT_UNDEFINED;
        }
        else if(fmt.compCount == 2)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
          return VK_FORMAT_UNDEFINED;
        }
        else if(fmt.compCount == 4)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16;
          return VK_FORMAT_UNDEFINED;
        }

        if(subsampling == 444)
        {
          // only support 3-plane
          return planeCount == 3 ? VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16
                                 : VK_FORMAT_UNDEFINED;
        }
        else if(subsampling == 422)
        {
          if(planeCount == 1)
            return fmt.BGRAOrder() ? VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16
                                   : VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16;
          else if(planeCount == 2)
            return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16;
          else if(planeCount == 3)
            return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16;
        }
        else if(subsampling == 420)
        {
          if(planeCount == 2)
            return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
          else if(planeCount == 3)
            return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16;
          else
            return VK_FORMAT_UNDEFINED;
        }

        return VK_FORMAT_UNDEFINED;
      }
      case ResourceFormatType::YUV12:
      {
        int subsampling = fmt.YUVSubsampling();
        int planeCount = fmt.YUVPlaneCount();

        if(fmt.compCount == 1)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R12X4_UNORM_PACK16;
          return VK_FORMAT_UNDEFINED;
        }
        else if(fmt.compCount == 2)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R12X4G12X4_UNORM_2PACK16;
          return VK_FORMAT_UNDEFINED;
        }
        else if(fmt.compCount == 4)
        {
          if(subsampling == 444 && planeCount == 1)
            return VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16;
          return VK_FORMAT_UNDEFINED;
        }

        if(subsampling == 444)
        {
          // only support 3-plane
          return planeCount == 3 ? VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16
                                 : VK_FORMAT_UNDEFINED;
        }
        else if(subsampling == 422)
        {
          if(planeCount == 1)
            return fmt.BGRAOrder() ? VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16
                                   : VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16;
          else if(planeCount == 2)
            return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16;
          else if(planeCount == 3)
            return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16;
        }
        else if(subsampling == 420)
        {
          if(planeCount == 2)
            return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16;
          else if(planeCount == 3)
            return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16;
          else
            return VK_FORMAT_UNDEFINED;
        }

        return VK_FORMAT_UNDEFINED;
      }
      case ResourceFormatType::YUV16:
      {
        int subsampling = fmt.YUVSubsampling();
        int planeCount = fmt.YUVPlaneCount();

        if(subsampling == 444)
        {
          // only support 3-plane
          return planeCount == 3 ? VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM : VK_FORMAT_UNDEFINED;
        }
        else if(subsampling == 422)
        {
          if(planeCount == 1)
            return fmt.BGRAOrder() ? VK_FORMAT_B16G16R16G16_422_UNORM
                                   : VK_FORMAT_G16B16G16R16_422_UNORM;
          else if(planeCount == 2)
            return VK_FORMAT_G16_B16R16_2PLANE_422_UNORM;
          else if(planeCount == 3)
            return VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM;
        }
        else if(subsampling == 420)
        {
          if(planeCount == 2)
            return VK_FORMAT_G16_B16R16_2PLANE_420_UNORM;
          else if(planeCount == 3)
            return VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM;
          else
            return VK_FORMAT_UNDEFINED;
        }

        return VK_FORMAT_UNDEFINED;
      }
      default: RDCERR("Unsupported resource format type %u", fmt.type); break;
    }
  }
  else if(fmt.compCount == 4)
  {
    if(fmt.SRGBCorrected())
    {
      ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_R8G8B8A8_SRGB;
    }
    else if(fmt.compByteWidth == 8)
    {
      if(fmt.compType == CompType::Float || fmt.compType == CompType::Double)
        ret = VK_FORMAT_R64G64B64A64_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R64G64B64A64_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R64G64B64A64_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 4)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R32G32B32A32_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R32G32B32A32_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R32G32B32A32_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 2)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R16G16B16A16_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R16G16B16A16_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R16G16B16A16_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R16G16B16A16_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R16G16B16A16_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R16G16B16A16_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R16G16B16A16_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 1)
    {
      if(fmt.compType == CompType::SInt)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_SINT : VK_FORMAT_R8G8B8A8_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_UINT : VK_FORMAT_R8G8B8A8_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_SNORM : VK_FORMAT_R8G8B8A8_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_SSCALED : VK_FORMAT_R8G8B8A8_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8A8_USCALED : VK_FORMAT_R8G8B8A8_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else
    {
      RDCERR("Unrecognised 4-component byte width: %d", fmt.compByteWidth);
    }
  }
  else if(fmt.compCount == 3)
  {
    if(fmt.SRGBCorrected())
    {
      ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_SRGB : VK_FORMAT_R8G8B8_SRGB;
    }
    else if(fmt.compByteWidth == 8)
    {
      if(fmt.compType == CompType::Float || fmt.compType == CompType::Double)
        ret = VK_FORMAT_R64G64B64_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R64G64B64_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R64G64B64_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 4)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R32G32B32_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R32G32B32_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R32G32B32_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 2)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R16G16B16_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R16G16B16_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R16G16B16_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R16G16B16_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R16G16B16_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R16G16B16_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R16G16B16_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 1)
    {
      if(fmt.compType == CompType::SInt)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_SINT : VK_FORMAT_R8G8B8_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_UINT : VK_FORMAT_R8G8B8_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_SNORM : VK_FORMAT_R8G8B8_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_UNORM : VK_FORMAT_R8G8B8_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_SSCALED : VK_FORMAT_R8G8B8_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = fmt.BGRAOrder() ? VK_FORMAT_B8G8R8_USCALED : VK_FORMAT_R8G8B8_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else
    {
      RDCERR("Unrecognised 3-component byte width: %d", fmt.compByteWidth);
    }
  }
  else if(fmt.compCount == 2)
  {
    if(fmt.SRGBCorrected())
    {
      ret = VK_FORMAT_R8G8_SRGB;
    }
    else if(fmt.compByteWidth == 8)
    {
      if(fmt.compType == CompType::Float || fmt.compType == CompType::Double)
        ret = VK_FORMAT_R64G64_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R64G64_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R64G64_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 4)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R32G32_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R32G32_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R32G32_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 2)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R16G16_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R16G16_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R16G16_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R16G16_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R16G16_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R16G16_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R16G16_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 1)
    {
      if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R8G8_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R8G8_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R8G8_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R8G8_UNORM;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R8G8_SSCALED;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R8G8_USCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else
    {
      RDCERR("Unrecognised 2-component byte width: %d", fmt.compByteWidth);
    }
  }
  else if(fmt.compCount == 1)
  {
    if(fmt.SRGBCorrected())
    {
      ret = VK_FORMAT_R8_SRGB;
    }
    else if(fmt.compByteWidth == 8)
    {
      if(fmt.compType == CompType::Float || fmt.compType == CompType::Double)
        ret = VK_FORMAT_R64_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R64_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R64_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 4)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R32_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R32_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R32_UINT;
      else if(fmt.compType == CompType::Depth)
        ret = VK_FORMAT_D32_SFLOAT;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 2)
    {
      if(fmt.compType == CompType::Float)
        ret = VK_FORMAT_R16_SFLOAT;
      else if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R16_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R16_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R16_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R16_UNORM;
      else if(fmt.compType == CompType::Depth)
        ret = VK_FORMAT_D16_UNORM;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R16_USCALED;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R16_SSCALED;
      else
        RDCERR("Unrecognised component type");
    }
    else if(fmt.compByteWidth == 1)
    {
      if(fmt.compType == CompType::SInt)
        ret = VK_FORMAT_R8_SINT;
      else if(fmt.compType == CompType::UInt)
        ret = VK_FORMAT_R8_UINT;
      else if(fmt.compType == CompType::SNorm)
        ret = VK_FORMAT_R8_SNORM;
      else if(fmt.compType == CompType::UNorm)
        ret = VK_FORMAT_R8_UNORM;
      else if(fmt.compType == CompType::UScaled)
        ret = VK_FORMAT_R8_USCALED;
      else if(fmt.compType == CompType::SScaled)
        ret = VK_FORMAT_R8_SSCALED;
      else if(fmt.compType == CompType::Depth)
        ret = VK_FORMAT_S8_UINT;
      else
        RDCERR("Unrecognised component type");
    }
    else
    {
      RDCERR("Unrecognised 1-component byte width: %d", fmt.compByteWidth);
    }
  }
  else
  {
    RDCERR("Unrecognised component count: %d", fmt.compCount);
  }

  if(ret == VK_FORMAT_UNDEFINED)
    RDCERR("No known vulkan format corresponding to resource format!");

  return ret;
}

VkImageAspectFlags FormatImageAspects(VkFormat fmt)
{
  if(IsStencilOnlyFormat(fmt))
    return VK_IMAGE_ASPECT_STENCIL_BIT;
  else if(IsDepthOnlyFormat(fmt))
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  else if(IsDepthAndStencilFormat(fmt))
    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  else if(GetYUVPlaneCount(fmt) == 3)
    return VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
  else if(GetYUVPlaneCount(fmt) == 2)
    return VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;
  else
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

ImageSubresourceRange ImageInfo::FullRange() const
{
  return ImageSubresourceRange(
      /* aspectMask = */ Aspects(),
      /* baseMipLevel = */ 0u,
      /* levelCount = */ (uint32_t)levelCount,
      /* baseArrayLayer = */ 0u,
      /* layerCount = */ (uint32_t)layerCount,
      /* baseDepthSlice = */ 0u,
      /* sliceCount = */ extent.depth);
}

void ImageSubresourceState::Update(const ImageSubresourceState &other, FrameRefCompFunc compose)
{
  if(oldQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
    oldQueueFamilyIndex = other.oldQueueFamilyIndex;

  if(other.newQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED)
    newQueueFamilyIndex = other.newQueueFamilyIndex;

  if(oldLayout == UNKNOWN_PREV_IMG_LAYOUT)
    oldLayout = other.oldLayout;

  if(other.newLayout != UNKNOWN_PREV_IMG_LAYOUT)
    newLayout = other.newLayout;

  refType = compose(refType, other.refType);
}

bool ImageSubresourceState::Update(const ImageSubresourceState &other,
                                   ImageSubresourceState &result, FrameRefCompFunc compose) const
{
  result = *this;
  result.Update(other, compose);

  return result != *this;
}

template <typename Map, typename Pair>
typename ImageSubresourceMap::SubresourceRangeIterTemplate<Map, Pair>
    &ImageSubresourceMap::SubresourceRangeIterTemplate<Map, Pair>::operator++()
{
  if(!IsValid())
    return *this;
  FixSubRange();

  ++m_slice;
  if(IsDepthSplit(m_splitFlags) && m_slice < m_range.baseDepthSlice + m_range.sliceCount)
  {
    m_value.m_range.baseDepthSlice = m_slice;
    return *this;
  }
  m_value.m_range.baseDepthSlice = m_slice = m_range.baseDepthSlice;

  ++m_layer;
  if(AreLayersSplit(m_splitFlags) && m_layer < m_range.baseArrayLayer + m_range.layerCount)
  {
    m_value.m_range.baseArrayLayer = m_layer;
    return *this;
  }
  m_value.m_range.baseArrayLayer = m_layer = m_range.baseArrayLayer;

  ++m_level;
  if(AreLevelsSplit(m_splitFlags) && m_level < m_range.baseMipLevel + m_range.levelCount)
  {
    m_value.m_range.baseMipLevel = m_level;
    return *this;
  }
  m_value.m_range.baseMipLevel = m_level = m_range.baseMipLevel;

  ++m_aspectIndex;
  if(AreAspectsSplit(m_splitFlags) && m_aspectIndex < m_aspectCount)
  {
    auto aspectIt =
        ImageAspectFlagIter(m_map->m_aspectMask, (VkImageAspectFlagBits)m_value.m_range.aspectMask);
    ++aspectIt;
    m_value.m_range.aspectMask = *aspectIt;
    return *this;
  }

  m_aspectIndex = m_aspectCount;
  return *this;
}
template
    typename ImageSubresourceMap::SubresourceRangeIterTemplate<ImageSubresourceMap,
                                                               ImageSubresourceMap::SubresourcePairRef>
        &ImageSubresourceMap::SubresourceRangeIterTemplate<
            ImageSubresourceMap, ImageSubresourceMap::SubresourcePairRef>::operator++();
template typename ImageSubresourceMap::SubresourceRangeIterTemplate<
    const ImageSubresourceMap, ImageSubresourceMap::ConstSubresourcePairRef>
    &ImageSubresourceMap::SubresourceRangeIterTemplate<
        const ImageSubresourceMap, ImageSubresourceMap::ConstSubresourcePairRef>::operator++();

void ImageSubresourceMap::Split(bool splitAspects, bool splitLevels, bool splitLayers, bool splitDepth)
{
  uint16_t newFlags = m_flags;
  if(splitAspects)
    newFlags |= (uint16_t)FlagBits::AreAspectsSplit;
  else
    splitAspects = AreAspectsSplit();

  if(splitLevels)
    newFlags |= (uint16_t)FlagBits::AreLevelsSplit;
  else
    splitLevels = AreLevelsSplit();

  if(splitLayers)
    newFlags |= (uint16_t)FlagBits::AreLayersSplit;
  else
    splitLayers = AreLayersSplit();

  if(splitDepth)
    newFlags |= (uint16_t)FlagBits::IsDepthSplit;
  else
    splitDepth = IsDepthSplit();

  if(newFlags == m_flags)
    // not splitting anything new
    return;

  uint32_t oldSplitAspectCount = AreAspectsSplit() ? m_aspectCount : 1;
  uint32_t newSplitAspectCount = splitAspects ? m_aspectCount : oldSplitAspectCount;

  uint32_t oldSplitLevelCount = AreLevelsSplit() ? GetImageInfo().levelCount : 1;
  uint32_t newSplitLevelCount = splitLevels ? GetImageInfo().levelCount : oldSplitLevelCount;

  uint32_t oldSplitLayerCount = AreLayersSplit() ? GetImageInfo().layerCount : 1;
  uint32_t newSplitLayerCount = splitLayers ? GetImageInfo().layerCount : oldSplitLayerCount;

  uint32_t oldSplitSliceCount = IsDepthSplit() ? GetImageInfo().extent.depth : 1;
  uint32_t newSplitSliceCount = splitDepth ? GetImageInfo().extent.depth : oldSplitSliceCount;

  uint32_t oldSize = (uint32_t)m_values.size();
  RDCASSERT(oldSize > 0);

  uint32_t newSize =
      newSplitAspectCount * newSplitLevelCount * newSplitLayerCount * newSplitSliceCount;
  RDCASSERT(newSize > oldSize);

  m_values.resize(newSize);

  uint32_t newAspectIndex = newSplitAspectCount - 1;
  uint32_t oldAspectIndex = AreAspectsSplit() ? newAspectIndex : 0;
  uint32_t newLevel = newSplitLevelCount - 1;
  uint32_t oldLevel = AreLevelsSplit() ? newLevel : 0;
  uint32_t newLayer = newSplitLayerCount - 1;
  uint32_t oldLayer = AreLayersSplit() ? newLayer : 0;
  uint32_t newSlice = newSplitSliceCount - 1;
  uint32_t oldSlice = IsDepthSplit() ? newSlice : 0;
  uint32_t newIndex = newSize - 1;
  while(true)
  {
    uint32_t oldIndex =
        ((oldAspectIndex * oldSplitLevelCount + oldLevel) * oldSplitLayerCount + oldLayer) *
            oldSplitSliceCount +
        oldSlice;
    m_values[newIndex] = m_values[oldIndex];

    if(newIndex == 0)
    {
      RDCASSERT(oldIndex == 0);
      break;
    }
    --newIndex;

    if(newSlice > 0)
    {
      --newSlice;
      oldSlice = IsDepthSplit() ? newSlice : 0;
      continue;
    }
    newSlice = newSplitSliceCount - 1;
    oldSlice = oldSplitSliceCount - 1;

    if(newLayer > 0)
    {
      --newLayer;
      oldLayer = AreLayersSplit() ? newLayer : 0;
      continue;
    }
    newLayer = newSplitLayerCount - 1;
    oldLayer = oldSplitLayerCount - 1;

    if(newLevel > 0)
    {
      --newLevel;
      oldLevel = AreLevelsSplit() ? newLevel : 0;
      continue;
    }
    newLevel = newSplitLevelCount - 1;
    oldLevel = oldSplitLevelCount - 1;

    if(newAspectIndex > 0)
    {
      --newAspectIndex;
      oldAspectIndex = AreAspectsSplit() ? newAspectIndex : 0;
      continue;
    }
    RDCERR("Too many subresources in ImageSubresourceMap::Split");
    break;
  }

  m_flags = newFlags;
}

void ImageSubresourceMap::Unsplit(bool unsplitAspects, bool unsplitLevels, bool unsplitLayers,
                                  bool unsplitDepth)
{
  uint16_t newFlags = m_flags;
  if(unsplitAspects)
    newFlags &= ~(uint16_t)FlagBits::AreAspectsSplit;

  if(unsplitLevels)
    newFlags &= ~(uint16_t)FlagBits::AreLevelsSplit;

  if(unsplitLayers)
    newFlags &= ~(uint16_t)FlagBits::AreLayersSplit;

  if(unsplitDepth)
    newFlags &= ~(uint16_t)FlagBits::IsDepthSplit;

  if(newFlags == m_flags)
    // not splitting anything new
    return;

  uint32_t newSplitAspectCount = unsplitAspects ? 1 : m_aspectCount;

  uint32_t oldSplitLevelCount = AreLevelsSplit() ? GetImageInfo().levelCount : 1;
  uint32_t newSplitLevelCount = unsplitLevels ? 1 : GetImageInfo().levelCount;

  uint32_t oldSplitLayerCount = AreLayersSplit() ? GetImageInfo().layerCount : 1;
  uint32_t newSplitLayerCount = unsplitLayers ? 1 : GetImageInfo().layerCount;

  uint32_t oldSplitSliceCount = IsDepthSplit() ? GetImageInfo().extent.depth : 1;
  uint32_t newSplitSliceCount = unsplitDepth ? 1 : GetImageInfo().extent.depth;

  uint32_t oldSize = (uint32_t)m_values.size();
  RDCASSERT(oldSize > 0);

  uint32_t newSize =
      newSplitAspectCount * newSplitLevelCount * newSplitLayerCount * newSplitSliceCount;
  RDCASSERT(newSize < oldSize);

  rdcarray<ImageSubresourceState> newValues;
  newValues.resize(newSize);

  uint32_t aspectIndex = 0;
  uint32_t level = 0;
  uint32_t layer = 0;
  uint32_t slice = 0;
  uint32_t newIndex = 0;

  while(newIndex < newValues.size())
  {
    uint32_t oldIndex = ((aspectIndex * oldSplitLevelCount + level) * oldSplitLayerCount + layer) *
                            oldSplitSliceCount +
                        slice;
    newValues[newIndex] = m_values[oldIndex];

    ++newIndex;

    ++slice;
    if(slice < newSplitSliceCount)
      continue;
    slice = 0;

    ++layer;
    if(layer < newSplitLayerCount)
      continue;
    layer = 0;

    ++level;
    if(level < newSplitLevelCount)
      continue;
    level = 0;

    ++aspectIndex;
  }

  newValues.swap(m_values);
  m_flags = newFlags;
}

void ImageSubresourceMap::Unsplit()
{
  if(m_values.size() == 1)
    return;

  uint32_t aspectCount = AreAspectsSplit() ? m_aspectCount : 1;
  uint32_t aspectIndex = 0;
  uint32_t levelCount = AreLevelsSplit() ? m_imageInfo.levelCount : 1;
  uint32_t level = 0;
  uint32_t layerCount = AreLayersSplit() ? m_imageInfo.layerCount : 1;
  uint32_t layer = 0;
  uint32_t sliceCount = IsDepthSplit() ? m_imageInfo.extent.depth : 1;
  uint32_t slice = 0;
  uint32_t index = 0;

  bool canUnsplitAspects = aspectCount > 1;
  bool canUnsplitLevels = levelCount > 1;
  bool canUnsplitLayers = layerCount > 1;
  bool canUnsplitDepth = sliceCount > 1;

  RDCASSERT(aspectCount * levelCount * layerCount * sliceCount == m_values.size());
#define UNSPLIT_INDEX(ASPECT, LEVEL, LAYER, SLICE) \
  (((ASPECT * levelCount + LEVEL) * layerCount + LAYER) * sliceCount + SLICE)
  while(index < m_values.size() &&
        (canUnsplitAspects || canUnsplitLevels || canUnsplitLayers || canUnsplitDepth))
  {
    if(canUnsplitAspects && aspectIndex > 0)
    {
      uint32_t index0 = UNSPLIT_INDEX(0, level, layer, slice);
      if(m_values[index] != m_values[index0])
        canUnsplitAspects = false;
    }
    if(canUnsplitLevels && level > 0)
    {
      uint32_t index0 = UNSPLIT_INDEX(aspectIndex, 0, layer, slice);
      if(m_values[index] != m_values[index0])
        canUnsplitLevels = false;
    }
    if(canUnsplitLayers && layer > 0)
    {
      uint32_t index0 = UNSPLIT_INDEX(aspectIndex, level, 0, slice);
      if(m_values[index] != m_values[index0])
        canUnsplitLayers = false;
    }
    if(canUnsplitDepth && slice > 0)
    {
      uint32_t index0 = UNSPLIT_INDEX(aspectIndex, level, layer, 0);
      if(m_values[index] != m_values[index0])
        canUnsplitDepth = false;
    }

    ++index;

    ++slice;
    if(slice < sliceCount)
      continue;
    slice = 0;

    ++layer;
    if(layer < layerCount)
      continue;
    layer = 0;

    ++level;
    if(level < levelCount)
      continue;
    level = 0;

    ++aspectIndex;
    if(aspectIndex >= aspectCount)
      break;
  }
#undef UNSPLIT_INDEX

  Unsplit(canUnsplitAspects, canUnsplitLevels, canUnsplitLayers, canUnsplitDepth);
}

size_t ImageSubresourceMap::SubresourceIndex(uint32_t aspectIndex, uint32_t level, uint32_t layer,
                                             uint32_t slice) const
{
  if(!AreAspectsSplit())
    aspectIndex = 0;
  int splitLevelCount = 1;
  if(AreLevelsSplit())
    splitLevelCount = GetImageInfo().levelCount;
  else
    level = 0;
  int splitLayerCount = 1;
  if(AreLayersSplit())
    splitLayerCount = GetImageInfo().layerCount;
  else
    layer = 0;
  int splitSliceCount = 1;
  if(IsDepthSplit())
    splitSliceCount = GetImageInfo().extent.depth;
  else
    slice = 0;
  return ((aspectIndex * splitLevelCount + level) * splitLayerCount + layer) * splitSliceCount +
         slice;
}

void ImageSubresourceMap::ToArray(rdcarray<ImageSubresourceStateForRange> &arr)
{
  arr.reserve(arr.size() + m_values.size());
  for(auto src = begin(); src != end(); ++src)
  {
    arr.push_back(*src);
  }
}

void ImageSubresourceMap::FromArray(const rdcarray<ImageSubresourceStateForRange> &arr)
{
  if(arr.empty())
  {
    RDCERR("No values for ImageSubresourceMap");
    return;
  }
  Split(arr.front().range);
  if(m_values.size() != arr.size())
  {
    RDCERR("Incorrect number of values for ImageSubresourceMap");
    return;
  }
  auto src = arr.begin();
  auto dst = begin();
  while(src != arr.end())
  {
    if(src->range != dst->range())
      RDCERR("Subresource range mismatch in ImageSubresourceMap");
    else
      dst->SetState(src->state);
    ++src;
    ++dst;
  }
}

void ImageSubresourceMap::FromImgRefs(const ImgRefs &imgRefs)
{
  bool splitLayers = imgRefs.areLayersSplit;
  bool splitDepth = false;
  if(GetImageInfo().extent.depth > 1)
  {
    RDCASSERT(GetImageInfo().layerCount == 1);
    splitLayers = false;
    splitDepth = true;
  }
  Split(imgRefs.areAspectsSplit, imgRefs.areLevelsSplit, splitLayers, splitDepth);
  RDCASSERT(!(AreLayersSplit() && IsDepthSplit()));

  for(auto dstIt = begin(); dstIt != end(); ++dstIt)
  {
    int aspectIndex = imgRefs.AspectIndex((VkImageAspectFlagBits)dstIt->range().aspectMask);
    int level = (int)dstIt->range().baseMipLevel;
    int layer = (int)(dstIt->range().baseArrayLayer + dstIt->range().baseDepthSlice);
    dstIt->state().refType = imgRefs.SubresourceRef(aspectIndex, level, layer);
  }
}

bool IntervalsOverlap(uint32_t base1, uint32_t count1, uint32_t base2, uint32_t count2)
{
  if((base1 + count1) < base1)
  {
    // integer overflow
    if(count1 != VK_REMAINING_MIP_LEVELS)
      RDCWARN("Integer overflow in interval: base=%u, count=%u", base1, count1);
    count1 = UINT32_MAX - base1;
  }
  if((base2 + count2) < base2)
  {
    // integer overflow
    if(count2 != VK_REMAINING_MIP_LEVELS)
      RDCWARN("Integer overflow in interval: base=%u, count=%u", base2, count2);
    count2 = UINT32_MAX - base2;
  }
  if(count1 == 0 || count2 == 0)
    return false;    // one of the intervals is empty, so no overlap
  if(base1 > base2)
  {
    std::swap(base1, base2);
    std::swap(count1, count2);
  }
  return base2 < base1 + count1;
}

bool IntervalContainedIn(uint32_t base1, uint32_t count1, uint32_t base2, uint32_t count2)
{
  if((base1 + count1) < base1)
  {
    // integer overflow
    if(count1 != VK_REMAINING_MIP_LEVELS)
      RDCWARN("Integer overflow in interval: base=%u, count=%u", base1, count1);
    count1 = UINT32_MAX - base1;
  }
  if((base2 + count2) < base2)
  {
    // integer overflow
    if(count2 != VK_REMAINING_MIP_LEVELS)
      RDCWARN("Integer overflow in interval: base=%u, count=%u", base2, count2);
    count2 = UINT32_MAX - base2;
  }
  return base1 >= base2 && base1 + count1 <= base2 + count2;
}

bool ValidateLevelRange(uint32_t &baseMipLevel, uint32_t &levelCount, uint32_t imageLevelCount)
{
  bool res = true;
  if(baseMipLevel > imageLevelCount)
  {
    RDCWARN("baseMipLevel (%u) is greater than image levelCount (%u)", baseMipLevel, imageLevelCount);
    baseMipLevel = imageLevelCount;
    res = false;
  }
  if(levelCount == VK_REMAINING_MIP_LEVELS)
  {
    levelCount = imageLevelCount - baseMipLevel;
  }
  else if(levelCount > imageLevelCount - baseMipLevel)
  {
    RDCWARN("baseMipLevel (%u) + levelCount (%u) is greater than the image levelCount (%u)",
            baseMipLevel, levelCount, imageLevelCount);
    levelCount = imageLevelCount - baseMipLevel;
    res = false;
  }
  return res;
}

bool ValidateLayerRange(uint32_t &baseArrayLayer, uint32_t &layerCount, uint32_t imageLayerCount)
{
  bool res = true;
  if(baseArrayLayer > imageLayerCount)
  {
    RDCWARN("baseArrayLayer (%u) is greater than image layerCount (%u)", baseArrayLayer,
            imageLayerCount);
    baseArrayLayer = imageLayerCount;
    res = false;
  }
  if(layerCount == VK_REMAINING_ARRAY_LAYERS)
  {
    layerCount = imageLayerCount - baseArrayLayer;
  }
  else if(layerCount > imageLayerCount - baseArrayLayer)
  {
    RDCWARN("baseArrayLayer (%u) + layerCount (%u) is greater than the image layerCount (%u)",
            baseArrayLayer, layerCount, imageLayerCount);
    layerCount = imageLayerCount - baseArrayLayer;
    res = false;
  }
  return res;
}

bool ValidateSliceRange(uint32_t &baseSlice, uint32_t &sliceCount, uint32_t imageSliceCount)
{
  bool res = true;
  if(baseSlice > imageSliceCount)
  {
    RDCWARN("baseSlice (%u) is greater than image sliceCount (%u)", baseSlice, imageSliceCount);
    baseSlice = imageSliceCount;
    res = false;
  }
  if(sliceCount == VK_REMAINING_ARRAY_LAYERS)
  {
    sliceCount = imageSliceCount - baseSlice;
  }
  else if(sliceCount > imageSliceCount - baseSlice)
  {
    RDCWARN("baseSlice (%u) + sliceCount (%u) is greater than the image sliceCount (%u)", baseSlice,
            sliceCount, imageSliceCount);
    sliceCount = imageSliceCount - baseSlice;
    res = false;
  }
  return res;
}

template <typename Map, typename Pair>
ImageSubresourceMap::SubresourceRangeIterTemplate<Map, Pair>::SubresourceRangeIterTemplate(
    Map &map, const ImageSubresourceRange &range)
    : m_map(&map),
      m_range(range),
      m_level(range.baseMipLevel),
      m_layer(range.baseArrayLayer),
      m_slice(range.baseDepthSlice)
{
  m_range.Validate(m_map->GetImageInfo());
  m_aspectCount = 0;
  for(auto aspectIt = ImageAspectFlagIter::begin(range.aspectMask);
      aspectIt != ImageAspectFlagIter::end(); ++aspectIt)
    ++m_aspectCount;
  m_splitFlags = (uint16_t)ImageSubresourceMap::FlagBits::IsUninitialized;
  FixSubRange();
}
template ImageSubresourceMap::SubresourceRangeIterTemplate<ImageSubresourceMap,
                                                           ImageSubresourceMap::SubresourcePairRef>::
    SubresourceRangeIterTemplate(ImageSubresourceMap &map, const ImageSubresourceRange &range);
template ImageSubresourceMap::SubresourceRangeIterTemplate<
    const ImageSubresourceMap, ImageSubresourceMap::ConstSubresourcePairRef>::
    SubresourceRangeIterTemplate(const ImageSubresourceMap &map, const ImageSubresourceRange &range);

template <typename Map, typename Pair>
void ImageSubresourceMap::SubresourceRangeIterTemplate<Map, Pair>::FixSubRange()
{
  if(m_splitFlags == m_map->m_flags)
    return;
  uint16_t oldFlags = m_splitFlags;
  m_splitFlags = m_map->m_flags;

  if(IsDepthSplit(m_splitFlags))
  {
    m_value.m_range.baseDepthSlice = m_slice;
    m_value.m_range.sliceCount = 1u;
  }
  else
  {
    m_value.m_range.baseDepthSlice = 0u;
    m_value.m_range.sliceCount = m_map->GetImageInfo().extent.depth;
  }

  if(AreLayersSplit(m_splitFlags))
  {
    m_value.m_range.baseArrayLayer = m_layer;
    m_value.m_range.layerCount = 1u;
  }
  else
  {
    m_value.m_range.baseArrayLayer = 0u;
    m_value.m_range.layerCount = m_map->GetImageInfo().layerCount;
  }

  if(AreLevelsSplit(m_splitFlags))
  {
    m_value.m_range.baseMipLevel = m_level;
    m_value.m_range.levelCount = 1u;
  }
  else
  {
    m_value.m_range.baseMipLevel = 0u;
    m_value.m_range.levelCount = m_map->GetImageInfo().levelCount;
  }

  if(!AreAspectsSplit(m_splitFlags))
  {
    m_value.m_range.aspectMask = m_map->m_aspectMask;
  }
  else if(!AreAspectsSplit(oldFlags))
  {
    // aspects are split in the map, but are not yet split in this iterator.
    // We need to find the aspectMask.
    uint32_t i = 0;
    for(auto it = ImageAspectFlagIter::begin(m_map->m_aspectMask); it != ImageAspectFlagIter::end();
        ++it, ++i)
    {
      if(i >= m_aspectIndex && (((*it) & m_range.aspectMask) != 0))
      {
        m_value.m_range.aspectMask = *it;
        break;
      }
    }
    m_aspectIndex = i;
  }
}
template void ImageSubresourceMap::SubresourceRangeIterTemplate<
    ImageSubresourceMap, ImageSubresourceMap::SubresourcePairRef>::FixSubRange();
template void ImageSubresourceMap::SubresourceRangeIterTemplate<
    const ImageSubresourceMap, ImageSubresourceMap::ConstSubresourcePairRef>::FixSubRange();

template <typename Map, typename Pair>
Pair *ImageSubresourceMap::SubresourceRangeIterTemplate<Map, Pair>::operator->()
{
  FixSubRange();
  m_value.m_state = &m_map->SubresourceValue(m_aspectIndex, m_level, m_layer, m_slice);
  return &m_value;
}
template ImageSubresourceMap::SubresourcePairRef *ImageSubresourceMap::SubresourceRangeIterTemplate<
    ImageSubresourceMap, ImageSubresourceMap::SubresourcePairRef>::operator->();
template ImageSubresourceMap::ConstSubresourcePairRef *ImageSubresourceMap::SubresourceRangeIterTemplate<
    const ImageSubresourceMap, ImageSubresourceMap::ConstSubresourcePairRef>::operator->();

template <typename Map, typename Pair>
Pair &ImageSubresourceMap::SubresourceRangeIterTemplate<Map, Pair>::operator*()
{
  FixSubRange();
  m_value.m_state = &m_map->SubresourceValue(m_aspectIndex, m_level, m_layer, m_slice);
  return m_value;
}
template ImageSubresourceMap::SubresourcePairRef &ImageSubresourceMap::SubresourceRangeIterTemplate<
    ImageSubresourceMap, ImageSubresourceMap::SubresourcePairRef>::operator*();
template ImageSubresourceMap::ConstSubresourcePairRef &ImageSubresourceMap::SubresourceRangeIterTemplate<
    const ImageSubresourceMap, ImageSubresourceMap::ConstSubresourcePairRef>::operator*();

template <typename Barrier>
void BarrierSequence<Barrier>::Add(uint32_t batchIndex, uint32_t queueFamilyIndex,
                                   const Barrier &barrier)
{
  if(batches.size() <= batchIndex)
    batches.resize(batchIndex + 1);
  rdcarray<rdcarray<Barrier> > &batch = batches[batchIndex];
  if(batch.size() <= queueFamilyIndex)
    batch.resize(queueFamilyIndex + 1);
  batch[queueFamilyIndex].push_back(barrier);
  ++barrierCount;
}
template void BarrierSequence<VkImageMemoryBarrier>::Add(uint32_t batchIndex,
                                                         uint32_t queueFamilyIndex,
                                                         const VkImageMemoryBarrier &barrier);

template <typename Barrier>
void BarrierSequence<Barrier>::Merge(const BarrierSequence<Barrier> &other)
{
  if(other.batches.size() > batches.size())
    batches.resize(other.batches.size());
  for(uint32_t batchIndex = 0; batchIndex < other.batches.size(); ++batchIndex)
  {
    rdcarray<rdcarray<Barrier> > &batch = batches[batchIndex];
    const rdcarray<rdcarray<Barrier> > &otherBatch = other.batches[batchIndex];
    if(otherBatch.size() > batch.size())
      batch.resize(otherBatch.size());
    for(uint32_t queueFamilyIndex = 0; queueFamilyIndex < otherBatch.size(); ++queueFamilyIndex)
    {
      rdcarray<Barrier> &barriers = batch[queueFamilyIndex];
      const rdcarray<Barrier> &otherBarriers = otherBatch[queueFamilyIndex];
      barriers.insert(barriers.size(), otherBarriers.begin(), otherBarriers.size());
      barrierCount += otherBarriers.size();
    }
  }
}
template void BarrierSequence<VkImageMemoryBarrier>::Merge(
    const BarrierSequence<VkImageMemoryBarrier> &other);

template <typename Barrier>
bool BarrierSequence<Barrier>::IsBatchEmpty(uint32_t batchIndex) const
{
  if(batchIndex >= batches.size())
    return true;
  for(const rdcarray<Barrier> *it = batches[batchIndex].begin(); it != batches[batchIndex].end(); ++it)
  {
    if(!it->empty())
      return false;
  }
  return true;
}
template bool BarrierSequence<VkImageMemoryBarrier>::IsBatchEmpty(uint32_t batchIndex) const;

template <typename Barrier>
void BarrierSequence<Barrier>::ExtractBatch(uint32_t batchIndex, rdcarray<rdcarray<Barrier> > &result)
{
  if(batchIndex >= batches.size())
    return;
  batches[batchIndex].swap(result);
  batches[batchIndex].clear();
  for(rdcarray<Barrier> *it = result.begin(); it != result.end(); ++it)
    barrierCount -= it->size();
}
template void BarrierSequence<VkImageMemoryBarrier>::ExtractBatch(
    uint32_t batchIndex, rdcarray<rdcarray<VkImageMemoryBarrier> > &result);

template <typename Barrier>
void BarrierSequence<Barrier>::ExtractFirstBatchForQueue(uint32_t queueFamilyIndex,
                                                         rdcarray<Barrier> &result)
{
  for(uint32_t batchIndex = 0; batchIndex < batches.size(); ++batchIndex)
  {
    if(!IsBatchEmpty(batchIndex))
    {
      batches[batchIndex][queueFamilyIndex].swap(result);
      batches[batchIndex][queueFamilyIndex].clear();
      barrierCount -= result.size();
      return;
    }
  }
}
template void BarrierSequence<VkImageMemoryBarrier>::ExtractFirstBatchForQueue(
    uint32_t queueFamilyIndex, rdcarray<VkImageMemoryBarrier> &result);

template <typename Barrier>
void BarrierSequence<Barrier>::ExtractLastBatchForQueue(uint32_t queueFamilyIndex,
                                                        rdcarray<Barrier> &result)
{
  for(uint32_t batchIndex = (uint32_t)batches.size(); batchIndex > 0;)
  {
    --batchIndex;
    if(!IsBatchEmpty(batchIndex))
    {
      batches[batchIndex][queueFamilyIndex].swap(result);
      batches[batchIndex][queueFamilyIndex].clear();
      barrierCount -= result.size();
      return;
    }
  }
}
template void BarrierSequence<VkImageMemoryBarrier>::ExtractLastBatchForQueue(
    uint32_t queueFamilyIndex, rdcarray<VkImageMemoryBarrier> &result);

ImageState ImageState::InitialState() const
{
  ImageState result(handle, GetImageInfo());
  InitialState(result);
  return result;
}

void ImageState::InitialState(ImageState &result) const
{
  result.subresourceStates = subresourceStates;
  for(auto it = result.subresourceStates.begin(); it != result.subresourceStates.end(); ++it)
  {
    ImageSubresourceState &sub = it->state();
    sub.newLayout = sub.oldLayout = GetImageInfo().initialLayout;
    sub.newQueueFamilyIndex = sub.oldQueueFamilyIndex;
    sub.refType = eFrameRef_None;
  }
}

ImageState ImageState::CommandBufferInitialState() const
{
  return ImageState(handle, GetImageInfo());
}

ImageState ImageState::UniformState(const ImageSubresourceState &sub) const
{
  ImageState result(handle, GetImageInfo());
  result.subresourceStates.begin()->SetState(sub);
  return result;
}

ImageState ImageState::ContentInitializationState(InitPolicy policy, bool initialized,
                                                  uint32_t queueFamilyIndex, VkImageLayout copyLayout,
                                                  VkImageLayout clearLayout) const
{
  ImageState result = *this;
  for(auto it = result.subresourceStates.begin(); it != result.subresourceStates.end(); ++it)
  {
    ImageSubresourceState &sub = it->state();
    InitReqType initReq = InitReq(sub.refType, policy, initialized);
    if(initReq == eInitReq_None)
      continue;
    sub.newQueueFamilyIndex = queueFamilyIndex;
    if(initReq == eInitReq_Copy)
      sub.newLayout = copyLayout;
    else if(initReq == eInitReq_Clear)
      sub.newLayout = clearLayout;
  }
  return result;
}

void ImageState::RemoveQueueFamilyTransfer(VkImageMemoryBarrier *it)
{
  if(it < newQueueFamilyTransfers.begin() || it >= newQueueFamilyTransfers.end())
    RDCERR("Attempting to remove queue family transfer at invalid address");
  std::swap(*it, newQueueFamilyTransfers.back());
  newQueueFamilyTransfers.erase(newQueueFamilyTransfers.size() - 1);
}

void ImageState::Update(ImageSubresourceRange range, const ImageSubresourceState &dst,
                        FrameRefCompFunc compose)
{
  range.Validate(GetImageInfo());

  bool didSplit = false;
  for(auto it = subresourceStates.RangeBegin(range); it != subresourceStates.end(); ++it)
  {
    ImageSubresourceState subState;
    if(it->state().Update(dst, subState, compose))
    {
      if(!didSplit)
      {
        subresourceStates.Split(range);
        didSplit = true;
      }
      RDCASSERT(it->range().ContainedIn(range));
      it->SetState(subState);
      maxRefType = ComposeFrameRefsDisjoint(maxRefType, subState.refType);
    }
  }
}

void ImageState::Merge(const ImageState &other, FrameRefCompFunc compose)
{
  if(handle == VK_NULL_HANDLE)
    handle = other.handle;
  for(auto it = other.oldQueueFamilyTransfers.begin(); it != other.oldQueueFamilyTransfers.end(); ++it)
  {
    RecordQueueFamilyAcquire(*it);
  }
  bool didSplit = false;
  for(auto oIt = other.subresourceStates.begin(); oIt != other.subresourceStates.end(); ++oIt)
  {
    for(auto it = subresourceStates.RangeBegin(oIt->range()); it != subresourceStates.end(); ++it)
    {
      ImageSubresourceState subState;
      if(it->state().Update(oIt->state(), subState, compose))
      {
        if(!didSplit)
        {
          subresourceStates.Split(oIt->range());
          didSplit = true;
        }
        RDCASSERT(it->range().ContainedIn(oIt->range()));
        it->SetState(subState);
        maxRefType = ComposeFrameRefsDisjoint(maxRefType, subState.refType);
      }
    }
  }
  for(auto it = other.newQueueFamilyTransfers.begin(); it != other.newQueueFamilyTransfers.end(); ++it)
  {
    RecordQueueFamilyRelease(*it);
  }
}

void ImageState::MergeCaptureBeginState(const ImageState &initialState)
{
  RDCASSERT(oldQueueFamilyTransfers.empty());
  RDCASSERT(newQueueFamilyTransfers.empty());
  oldQueueFamilyTransfers = initialState.oldQueueFamilyTransfers;
  subresourceStates = initialState.subresourceStates;
  maxRefType = initialState.maxRefType;
}

void ImageState::Merge(std::map<ResourceId, ImageState> &states,
                       const std::map<ResourceId, ImageState> &dstStates, FrameRefCompFunc compose)
{
  auto it = states.begin();
  auto dstIt = dstStates.begin();
  while(dstIt != dstStates.end())
  {
    if(it == states.end() || dstIt->first < it->first)
    {
      it = states.insert(it, {dstIt->first, dstIt->second.InitialState()});
    }
    else if(it->first < dstIt->first)
    {
      ++it;
      continue;
    }

    it->second.Merge(dstIt->second, compose);
    ++it;
    ++dstIt;
  }
}

void ImageState::DiscardContents(const ImageSubresourceRange &range)
{
  Update(range, ImageSubresourceState(VK_QUEUE_FAMILY_IGNORED, VK_IMAGE_LAYOUT_UNDEFINED));
}

void ImageState::RecordQueueFamilyRelease(const VkImageMemoryBarrier &barrier)
{
  for(auto it = newQueueFamilyTransfers.begin(); it != newQueueFamilyTransfers.end(); ++it)
  {
    if(ImageSubresourceRange(barrier.subresourceRange).Overlaps(it->subresourceRange))
    {
      RDCWARN("Queue family release barriers overlap");
      RemoveQueueFamilyTransfer(it);
      --it;
    }
  }
  newQueueFamilyTransfers.push_back(barrier);
}

void ImageState::RecordQueueFamilyAcquire(const VkImageMemoryBarrier &barrier)
{
  bool foundRelease = false;
  ImageSubresourceRange acquireRange(barrier.subresourceRange);
  for(auto it = newQueueFamilyTransfers.begin(); it != newQueueFamilyTransfers.end(); ++it)
  {
    ImageSubresourceRange releaseRange(it->subresourceRange);
    if(acquireRange.Overlaps(releaseRange))
    {
      if(acquireRange != releaseRange)
        RDCWARN(
            "Overlapping queue family release and acquire barriers have different "
            "subresourceRange");
      if(barrier.srcQueueFamilyIndex != it->srcQueueFamilyIndex ||
         barrier.dstQueueFamilyIndex != it->dstQueueFamilyIndex)
        RDCWARN("Queue family mismatch between release and acquire barriers");
      if(barrier.oldLayout != it->oldLayout || barrier.newLayout != it->newLayout)
        RDCWARN("Image layouts mismatch between release and acquire barriers");
      if(foundRelease)
        RDCWARN("Found multiple release barriers for acquire barrier");
      RemoveQueueFamilyTransfer(it);
      --it;
      foundRelease = true;
    }
  }
  if(!foundRelease)
  {
    oldQueueFamilyTransfers.push_back(barrier);
  }
}

void ImageState::RecordBarrier(VkImageMemoryBarrier barrier, uint32_t queueFamilyIndex)
{
  if(barrier.srcQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
     barrier.srcQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT ||
     barrier.dstQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
     barrier.dstQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT)
  {
    RDCERR("External/foreign queue families are not supported");
    return;
  }
  if(GetImageInfo().sharingMode == VK_SHARING_MODE_CONCURRENT)
  {
    if(!(barrier.srcQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED &&
         barrier.dstQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED))
    {
      RDCWARN("Barrier contains invalid queue families for VK_SHARING_MODE_CONCURRENT");
    }
    barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = queueFamilyIndex;
  }
  else if(GetImageInfo().sharingMode == VK_SHARING_MODE_EXCLUSIVE)
  {
    if(barrier.srcQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED ||
       barrier.dstQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
    {
      if(barrier.srcQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED ||
         barrier.dstQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED)
      {
        RDCERR("Barrier contains invalid queue families for VK_SHARING_MODE_EXCLUSIVE: (%s, %s)",
               ToStr(barrier.srcQueueFamilyIndex).c_str(),
               ToStr(barrier.dstQueueFamilyIndex).c_str());
        return;
      }
      barrier.srcQueueFamilyIndex = queueFamilyIndex;
      barrier.dstQueueFamilyIndex = queueFamilyIndex;
    }
    else if(barrier.srcQueueFamilyIndex == queueFamilyIndex)
    {
      if(barrier.dstQueueFamilyIndex != queueFamilyIndex)
      {
        RecordQueueFamilyRelease(barrier);
        // Skip the updates to the subresource states.
        // These will be updated by the acquire.
        // This allows us to restore a released-but-not-acquired state by first transitioning to the
        // subresource states (which will match the srcQueueFamilyIndex/oldLayout), and then
        // applying the release barrier.
        return;
      }
    }
    else if(barrier.dstQueueFamilyIndex == queueFamilyIndex)
    {
      RecordQueueFamilyAcquire(barrier);
    }
    else
    {
      RDCERR("Ownership transfer from queue family %u to %u submitted to queue family %u",
             barrier.srcQueueFamilyIndex, barrier.dstAccessMask, queueFamilyIndex);
    }
  }

  Update(barrier.subresourceRange, ImageSubresourceState(barrier),
         ComposeFrameRefs);    // TODO: we should avoid updating the frame refs when replaying
}

bool ImageState::CloseTransfers(uint32_t batchIndex, VkAccessFlags dstAccessMask,
                                ImageBarrierSequence &barriers, ImageTransitionInfo info)
{
  if(newQueueFamilyTransfers.empty())
    return false;
  for(auto it = newQueueFamilyTransfers.begin(); it != newQueueFamilyTransfers.end(); ++it)
  {
    Update(it->subresourceRange, ImageSubresourceState(it->dstQueueFamilyIndex, it->newLayout));

    it->dstAccessMask = dstAccessMask;
    it->image = handle;
    barriers.Add(batchIndex, it->dstQueueFamilyIndex, *it);
  }
  newQueueFamilyTransfers.clear();
  return true;
}

bool ImageState::RestoreTransfers(uint32_t batchIndex,
                                  const rdcarray<VkImageMemoryBarrier> &transfers,
                                  VkAccessFlags srcAccessMask, ImageBarrierSequence &barriers,
                                  ImageTransitionInfo info)
{
  // TODO: figure out why `transfers` has duplicate entries
  if(transfers.empty())
    return false;
  for(auto it = transfers.begin(); it != transfers.end(); ++it)
  {
    VkImageMemoryBarrier barrier = *it;
    barrier.srcAccessMask = srcAccessMask;
    barrier.image = handle;
    barriers.Add(batchIndex, barrier.srcQueueFamilyIndex, barrier);
    RecordQueueFamilyRelease(barrier);
  }
  return true;
}

void ImageState::ResetToOldState(ImageBarrierSequence &barriers, ImageTransitionInfo info)
{
  VkAccessFlags srcAccessMask = VK_ACCESS_ALL_WRITE_BITS;
  VkAccessFlags dstAccessMask = VK_ACCESS_ALL_READ_BITS;
  const uint32_t CLOSE_TRANSFERS_BATCH_INDEX = 0;
  const uint32_t MAIN_BATCH_INDEX = 1;
  const uint32_t ACQUIRE_BATCH_INDEX = 2;
  const uint32_t RESTORE_TRANSFERS_BATCH_INDEX = 3;
  CloseTransfers(CLOSE_TRANSFERS_BATCH_INDEX, dstAccessMask, barriers, info);

  for(auto subIt = subresourceStates.begin(); subIt != subresourceStates.end(); ++subIt)
  {
    VkImageLayout oldLayout = subIt->state().newLayout;
    if(oldLayout == UNKNOWN_PREV_IMG_LAYOUT)
      oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout newLayout = subIt->state().oldLayout;
    subIt->state().newLayout = subIt->state().oldLayout;
    if(newLayout == UNKNOWN_PREV_IMG_LAYOUT || newLayout == VK_IMAGE_LAYOUT_UNDEFINED)
    {
      // contents discarded, no barrier necessary
      continue;
    }
    SanitiseReplayImageLayout(oldLayout);
    SanitiseReplayImageLayout(newLayout);
    if(oldLayout != VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
    {
      // Transitioning back to PREINITIALIZED; this is impossible, so transition to GENERAL instead.
      newLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    uint32_t srcQueueFamilyIndex = subIt->state().newQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex = subIt->state().oldQueueFamilyIndex;

    if(srcQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
       srcQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT)
    {
      srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }
    if(dstQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
       dstQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT)
    {
      dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }

    uint32_t submitQueueFamilyIndex = srcQueueFamilyIndex;

    if(GetImageInfo().sharingMode == VK_SHARING_MODE_EXCLUSIVE)
    {
      if(srcQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
      {
        submitQueueFamilyIndex = dstQueueFamilyIndex;
        dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      }
      else if(dstQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
      {
        srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      }
    }
    else
    {
      if(submitQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
        submitQueueFamilyIndex = dstQueueFamilyIndex;
      srcQueueFamilyIndex = dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }

    if(srcQueueFamilyIndex == dstQueueFamilyIndex && oldLayout == newLayout)
    {
      subIt->state().newQueueFamilyIndex = subIt->state().oldQueueFamilyIndex;
      continue;
    }

    if(submitQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
    {
      RDCWARN("ResetToOldState: barrier submitted to VK_QUEUE_FAMILY_IGNORED; defaulting to queue family %u", info.defaultQueueFamilyIndex);
      submitQueueFamilyIndex = info.defaultQueueFamilyIndex;
    }
    subIt->state().newQueueFamilyIndex = subIt->state().oldQueueFamilyIndex;

    ImageSubresourceRange subRange = subIt->range();

    if(subRange.baseDepthSlice != 0)
    {
      // We can't issue barriers per depth slice, so skip the barriers for non-zero depth slices.
      // The zero depth slice barrier will implicitly cover the non-zerp depth slices.
      continue;
    }

    if((GetImageInfo().Aspects() & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) ==
           (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) &&
       !info.separateDepthStencilLayouts)
    {
      // This is a subresource of a depth and stencil image, and
      // VK_KHR_separate_depth_stencil_layouts is not enabled, so the barrier needs to include both
      // depth and stencil aspects. We skip the stencil-only aspect and expand the barrier for the
      // depth-only aspect to include both depth and stencil aspects.
      if(subRange.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT)
        continue;
      if(subRange.aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT)
        subRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageMemoryBarrier barrier = {
        /* sType = */ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        /* pNext = */ NULL,
        /* srcAccessMask = */ srcAccessMask,
        /* dstAccessMask = */ dstAccessMask,
        /* oldLayout = */ oldLayout,
        /* newLayout = */ newLayout,
        /* srcQueueFamilyIndex = */ srcQueueFamilyIndex,
        /* dstQueueFamilyIndex = */ dstQueueFamilyIndex,
        /* image = */ handle,
        /* subresourceRange = */ subRange,
    };
    barriers.Add(MAIN_BATCH_INDEX, submitQueueFamilyIndex, barrier);

    // acquire the subresource in the dstQueueFamily, if necessary
    if(barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex)
    {
      barriers.Add(ACQUIRE_BATCH_INDEX, barrier.dstQueueFamilyIndex, barrier);
    }
  }
  RestoreTransfers(RESTORE_TRANSFERS_BATCH_INDEX, oldQueueFamilyTransfers, srcAccessMask, barriers,
                   info);
}

void ImageState::Transition(const ImageState &dstState, VkAccessFlags srcAccessMask,
                            VkAccessFlags dstAccessMask, ImageBarrierSequence &barriers,
                            ImageTransitionInfo info)
{
  const uint32_t CLOSE_TRANSFERS_BATCH_INDEX = 0;
  const uint32_t MAIN_BATCH_INDEX = 1;
  const uint32_t ACQUIRE_BATCH_INDEX = 2;
  const uint32_t RESTORE_TRANSFERS_BATCH_INDEX = 3;
  CloseTransfers(CLOSE_TRANSFERS_BATCH_INDEX, dstAccessMask, barriers, info);

  for(auto dstIt = dstState.subresourceStates.begin(); dstIt != dstState.subresourceStates.end();
      ++dstIt)
  {
    const ImageSubresourceRange &dstRng = dstIt->range();
    const ImageSubresourceState &dstSub = dstIt->state();
    for(auto it = subresourceStates.RangeBegin(dstRng); it != subresourceStates.end(); ++it)
    {
      ImageSubresourceState srcSub;
      if(!it->state().Update(dstSub, srcSub, info.GetFrameRefCompFunc()))
        // subresource state did not change, so no need for a barrier
        continue;

      subresourceStates.Split(dstRng);
      std::swap(it->state(), srcSub);

      ImageSubresourceRange srcRng = it->range();

      VkImageLayout oldLayout = srcSub.newLayout;
      if(oldLayout == UNKNOWN_PREV_IMG_LAYOUT)
        oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkImageLayout newLayout = dstSub.newLayout;
      if(newLayout == UNKNOWN_PREV_IMG_LAYOUT || newLayout == VK_IMAGE_LAYOUT_UNDEFINED)
        // ignore transitions to undefined
        continue;
      uint32_t srcQueueFamilyIndex = srcSub.newQueueFamilyIndex;
      uint32_t dstQueueFamilyIndex = dstSub.newQueueFamilyIndex;

      if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
        // transitions from undefined discard the contents anyway, so no queue family ownership
        // transfer is necessary
        srcQueueFamilyIndex = dstQueueFamilyIndex;

      if(newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && oldLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)
      {
        // Transitioning to PREINITIALIZED, which is invalid. This happens when we are resetting to
        // an earlier image state.
        // Instead, we transition to GENERAL, and make the image owned by oldQueueFamilyIndex.
        newLayout = VK_IMAGE_LAYOUT_GENERAL;
        dstQueueFamilyIndex = srcSub.oldQueueFamilyIndex;
        RDCASSERT(dstQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED);
      }

      if(IsReplayMode(info.capState))
      {
        // Get rid of PRESENT layouts
        SanitiseReplayImageLayout(oldLayout);
        SanitiseReplayImageLayout(newLayout);
      }

      uint32_t submitQueueFamilyIndex = (srcQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED)
                                            ? srcQueueFamilyIndex
                                            : dstQueueFamilyIndex;
      if(submitQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED ||
         submitQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
         submitQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT)
      {
        RDCERR("Ignoring state transition submitted to invalid queue family %u",
               submitQueueFamilyIndex);
        continue;
      }
      if(GetImageInfo().sharingMode == VK_SHARING_MODE_CONCURRENT)
      {
        srcQueueFamilyIndex = dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      }
      else
      {
        if(srcQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
        {
          RDCWARN("ImageState::Transition: src queue family == VK_QUEUE_FAMILY_IGNORED.");
          srcQueueFamilyIndex = dstQueueFamilyIndex;
        }
        if(dstQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED)
        {
          RDCWARN("ImageState::Transition: dst queue family == VK_QUEUE_FAMILY_IGNORED.");
          dstQueueFamilyIndex = srcQueueFamilyIndex;
        }
      }

      if(srcQueueFamilyIndex == dstQueueFamilyIndex && oldLayout == newLayout)
        // Skip the barriers, because it would do nothing
        continue;

      if(srcRng.baseDepthSlice != 0 || dstRng.baseDepthSlice != 0)
      {
        // We can't issue barriers per depth slice, so skip the barriers for non-zero depth slices.
        // The zero depth slice barrier will implicitly cover the non-zerp depth slices.
        continue;
      }

      VkImageAspectFlags aspectMask = srcRng.aspectMask & dstRng.aspectMask;
      if((GetImageInfo().Aspects() & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) ==
             (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) &&
         !info.separateDepthStencilLayouts)
      {
        // This is a subresource of a depth and stencil image, and
        // VK_KHR_separate_depth_stencil_layouts is not enabled, so the barrier needs to include
        // both depth and stencil aspects. We skip the stencil-only aspect and expand the barrier
        // for the depth-only aspect to include both depth and stencil aspects.
        if(aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT)
          continue;
        if(aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT)
          aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
      }
      uint32_t baseMipLevel = RDCMAX(dstRng.baseMipLevel, srcRng.baseMipLevel);
      uint32_t endMipLevel =
          RDCMIN(dstRng.baseMipLevel + dstRng.levelCount, srcRng.baseMipLevel + srcRng.levelCount);
      uint32_t baseArrayLayer = RDCMAX(dstRng.baseArrayLayer, srcRng.baseArrayLayer);
      uint32_t endArrayLayer = RDCMIN(dstRng.baseArrayLayer + dstRng.layerCount,
                                      srcRng.baseArrayLayer + srcRng.layerCount);
      VkImageMemoryBarrier barrier = {
          /* sType = */ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          /* pNext = */ NULL,
          /* srcAccessMask = */ srcAccessMask,
          /* dstAccessMask = */ dstAccessMask,
          /* oldLayout = */ oldLayout,
          /* newLayout = */ newLayout,
          /* srcQueueFamilyIndex = */ srcQueueFamilyIndex,
          /* dstQueueFamilyIndex = */ dstQueueFamilyIndex,
          /* image = */ handle,
          /* subresourceRange = */
          {
              /* aspectMask = */ aspectMask,
              /* baseMipLevel = */ baseMipLevel,
              /* levelCount = */ endMipLevel - baseMipLevel,
              /* baseArrayLayer = */ baseArrayLayer,
              /* layerCount = */ endArrayLayer - baseArrayLayer,
          },
      };
      barriers.Add(MAIN_BATCH_INDEX, submitQueueFamilyIndex, barrier);

      // acquire the subresource in the dstQueueFamily, if necessary
      if(barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex)
      {
        barriers.Add(ACQUIRE_BATCH_INDEX, barrier.dstQueueFamilyIndex, barrier);
      }
    }
  }
  RestoreTransfers(RESTORE_TRANSFERS_BATCH_INDEX, dstState.newQueueFamilyTransfers, srcAccessMask,
                   barriers, info);
}

void ImageState::Transition(uint32_t queueFamilyIndex, VkImageLayout layout,
                            VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                            ImageBarrierSequence &barriers, ImageTransitionInfo info)
{
  Transition(UniformState(ImageSubresourceState(queueFamilyIndex, layout)), srcAccessMask,
             dstAccessMask, barriers, info);
}

void ImageState::TempTransition(const ImageState &dstState, VkAccessFlags preSrcAccessMask,
                                VkAccessFlags preDstAccessMask, VkAccessFlags postSrcAccessmask,
                                VkAccessFlags postDstAccessMask, ImageBarrierSequence &setupBarriers,
                                ImageBarrierSequence &cleanupBarriers, ImageTransitionInfo info) const
{
  ImageState temp(*this);
  temp.Transition(dstState, preSrcAccessMask, preDstAccessMask, setupBarriers, info);
  temp.Transition(*this, postSrcAccessmask, postDstAccessMask, cleanupBarriers, info);
}

void ImageState::TempTransition(uint32_t queueFamilyIndex, VkImageLayout layout,
                                VkAccessFlags accessMask, ImageBarrierSequence &setupBarriers,
                                ImageBarrierSequence &cleanupBarriers, ImageTransitionInfo info) const
{
  TempTransition(UniformState(ImageSubresourceState(queueFamilyIndex, layout)),
                 VK_ACCESS_ALL_WRITE_BITS, accessMask, accessMask, VK_ACCESS_ALL_READ_BITS,
                 setupBarriers, cleanupBarriers, info);
}

void ImageState::InlineTransition(VkCommandBuffer cmd, uint32_t queueFamilyIndex,
                                  const ImageState &dstState, VkAccessFlags srcAccessMask,
                                  VkAccessFlags dstAccessMask, ImageTransitionInfo info)
{
  ImageBarrierSequence barriers;
  Transition(dstState, srcAccessMask, dstAccessMask, barriers, info);
  if(barriers.empty())
    return;
  rdcarray<VkImageMemoryBarrier> barriersArray;
  barriers.ExtractFirstBatchForQueue(queueFamilyIndex, barriersArray);
  if(!barriersArray.empty())
    DoPipelineBarrier(cmd, (uint32_t)barriersArray.size(), barriersArray.data());
  if(!barriers.empty())
  {
    RDCERR("Could not inline all image state transition barriers");
  }
}

void ImageState::InlineTransition(VkCommandBuffer cmd, uint32_t queueFamilyIndex,
                                  VkImageLayout layout, VkAccessFlags srcAccessMask,
                                  VkAccessFlags dstAccessMask, ImageTransitionInfo info)
{
  InlineTransition(cmd, queueFamilyIndex,
                   UniformState(ImageSubresourceState(queueFamilyIndex, layout)), srcAccessMask,
                   dstAccessMask, info);
}

InitReqType ImageState::MaxInitReq(const ImageSubresourceRange &range, InitPolicy policy,
                                   bool initialized) const
{
  FrameRefType refType = eFrameRef_None;
  for(auto it = subresourceStates.RangeBegin(range); it != subresourceStates.end(); ++it)
  {
    ComposeFrameRefsDisjoint(refType, it->state().refType);
  }
  return InitReq(refType, policy, initialized);
}

void ImageState::BeginCapture()
{
  maxRefType = eFrameRef_None;

  // Forget any pending queue family release operations.
  // If the matching queue family acquire operation happens during the frame,
  // an implicit release operation will be put into `oldQueueFamilyTransfers`.
  newQueueFamilyTransfers.clear();

  // Also clear implicit queue family acquire operations because these correspond to release
  // operations already submitted (and therefore not part of the capture).
  oldQueueFamilyTransfers.clear();

  for(auto it = subresourceStates.begin(); it != subresourceStates.end(); ++it)
  {
    ImageSubresourceState state = it->state();
    state.oldLayout = state.newLayout;
    state.oldQueueFamilyIndex = state.newQueueFamilyIndex;
    state.refType = eFrameRef_None;
    it->SetState(state);
  }
}

int ImgRefs::GetAspectCount() const
{
  int aspectCount = 0;
  for(auto aspectIt = ImageAspectFlagIter::begin(aspectMask);
      aspectIt != ImageAspectFlagIter::end(); ++aspectIt)
  {
    ++aspectCount;
  }
  return aspectCount;
}

int ImgRefs::AspectIndex(VkImageAspectFlagBits aspect) const
{
  int aspectIndex = 0;
  if(areAspectsSplit)
  {
    for(auto aspectIt = ImageAspectFlagIter::begin(aspectMask);
        aspectIt != ImageAspectFlagIter::end(); ++aspectIt)
    {
      if(*aspectIt == aspect)
        break;
      ++aspectIndex;
    }
  }
  return aspectIndex;
}

int ImgRefs::SubresourceIndex(int aspectIndex, int level, int layer) const
{
  if(!areAspectsSplit)
    aspectIndex = 0;
  int splitLevelCount = 1;
  if(areLevelsSplit)
    splitLevelCount = imageInfo.levelCount;
  else
    level = 0;
  int splitLayerCount = 1;
  if(areLayersSplit)
    splitLayerCount = imageInfo.layerCount;
  else
    layer = 0;
  return (aspectIndex * splitLevelCount + level) * splitLayerCount + layer;
}

InitReqType ImgRefs::SubresourceRangeMaxInitReq(VkImageSubresourceRange range, InitPolicy policy,
                                                bool initialized) const
{
  InitReqType initReq = eInitReq_None;
  rdcarray<int> splitAspectIndices;
  if(areAspectsSplit)
  {
    int aspectIndex = 0;
    for(auto aspectIt = ImageAspectFlagIter::begin(aspectMask);
        aspectIt != ImageAspectFlagIter::end(); ++aspectIt, ++aspectIndex)
    {
      if(((*aspectIt) & range.aspectMask) != 0)
        splitAspectIndices.push_back(aspectIndex);
    }
  }
  else
  {
    splitAspectIndices.push_back(0);
  }

  int splitLevelCount = 1;
  if(areLevelsSplit || range.baseMipLevel != 0 || range.levelCount < (uint32_t)imageInfo.levelCount)
  {
    splitLevelCount = range.levelCount;
  }
  int splitLayerCount = 1;
  if(areLayersSplit || range.baseArrayLayer != 0 || range.layerCount < (uint32_t)imageInfo.layerCount)
  {
    splitLayerCount = range.layerCount;
  }
  for(auto aspectIndexIt = splitAspectIndices.begin(); aspectIndexIt != splitAspectIndices.end();
      ++aspectIndexIt)
  {
    for(int level = range.baseMipLevel; level < splitLevelCount; ++level)
    {
      for(int layer = range.baseArrayLayer; layer < splitLayerCount; ++layer)
      {
        initReq =
            RDCMAX(initReq, SubresourceInitReq(*aspectIndexIt, level, layer, policy, initialized));
      }
    }
  }
  return initReq;
}

rdcarray<rdcpair<VkImageSubresourceRange, InitReqType> > ImgRefs::SubresourceRangeInitReqs(
    VkImageSubresourceRange range, InitPolicy policy, bool initialized) const
{
  VkImageSubresourceRange out(range);
  rdcarray<rdcpair<VkImageSubresourceRange, InitReqType> > res;
  rdcarray<rdcpair<int, VkImageAspectFlags> > splitAspects;
  if(areAspectsSplit)
  {
    int aspectIndex = 0;
    for(auto aspectIt = ImageAspectFlagIter::begin(aspectMask);
        aspectIt != ImageAspectFlagIter::end(); ++aspectIt, ++aspectIndex)
    {
      if(((*aspectIt) & range.aspectMask) != 0)
        splitAspects.push_back({aspectIndex, (VkImageAspectFlags)*aspectIt});
    }
  }
  else
  {
    splitAspects.push_back({0, aspectMask});
  }

  int splitLevelCount = 1;
  if(areLevelsSplit || range.baseMipLevel != 0 || range.levelCount < (uint32_t)imageInfo.levelCount)
  {
    splitLevelCount = range.levelCount;
    out.levelCount = 1;
  }
  int splitLayerCount = 1;
  if(areLayersSplit || range.baseArrayLayer != 0 || range.layerCount < (uint32_t)imageInfo.layerCount)
  {
    splitLayerCount = range.layerCount;
    out.layerCount = 1;
  }
  for(auto aspectIt = splitAspects.begin(); aspectIt != splitAspects.end(); ++aspectIt)
  {
    int aspectIndex = aspectIt->first;
    out.aspectMask = aspectIt->second;
    for(int level = range.baseMipLevel; level < splitLevelCount; ++level)
    {
      out.baseMipLevel = level;
      for(int layer = range.baseArrayLayer; layer < splitLayerCount; ++layer)
      {
        out.baseArrayLayer = layer;
        res.push_back(
            make_rdcpair(out, SubresourceInitReq(aspectIndex, level, layer, policy, initialized)));
      }
    }
  }
  return res;
}

void ImgRefs::Split(bool splitAspects, bool splitLevels, bool splitLayers)
{
  int newSplitAspectCount = 1;
  if(splitAspects || areAspectsSplit)
  {
    newSplitAspectCount = GetAspectCount();
  }

  int oldSplitLevelCount = areLevelsSplit ? imageInfo.levelCount : 1;
  int newSplitLevelCount = splitLevels ? imageInfo.levelCount : oldSplitLevelCount;

  int oldSplitLayerCount = areLayersSplit ? imageInfo.layerCount : 1;
  int newSplitLayerCount = splitLayers ? imageInfo.layerCount : oldSplitLayerCount;

  int newSize = newSplitAspectCount * newSplitLevelCount * newSplitLayerCount;
  if(newSize == (int)rangeRefs.size())
    return;
  rangeRefs.resize(newSize);

  for(int newAspectIndex = newSplitAspectCount - 1; newAspectIndex >= 0; --newAspectIndex)
  {
    int oldAspectIndex = areAspectsSplit ? newAspectIndex : 0;
    for(int newLevel = newSplitLevelCount - 1; newLevel >= 0; --newLevel)
    {
      int oldLevel = areLevelsSplit ? newLevel : 0;
      for(int newLayer = newSplitLayerCount - 1; newLayer >= 0; --newLayer)
      {
        int oldLayer = areLayersSplit ? newLayer : 0;
        int oldIndex =
            (oldAspectIndex * oldSplitLevelCount + oldLevel) * oldSplitLayerCount + oldLayer;
        int newIndex =
            (newAspectIndex * newSplitLevelCount + newLevel) * newSplitLayerCount + newLayer;
        rangeRefs[newIndex] = rangeRefs[oldIndex];
      }
    }
  }
  areAspectsSplit = newSplitAspectCount > 1;
  areLevelsSplit = newSplitLevelCount > 1;
  areLayersSplit = newSplitLayerCount > 1;
}

VkResourceRecord::~VkResourceRecord()
{
  VkResourceType resType = Resource != NULL ? IdentifyTypeByPtr(Resource) : eResUnknown;

  // bufferviews and imageviews have non-owning pointers to the sparseinfo struct
  if(resType == eResBuffer || resType == eResImage)
    SAFE_DELETE(resInfo);

  if(resType == eResInstance || resType == eResDevice || resType == eResPhysicalDevice)
    SAFE_DELETE(instDevInfo);

  if(resType == eResSwapchain)
    SAFE_DELETE(swapInfo);

  if(resType == eResDeviceMemory && memMapState)
  {
    FreeAlignedBuffer(memMapState->refData);

    SAFE_DELETE(memMapState);
  }

  if(resType == eResCommandBuffer)
    SAFE_DELETE(cmdInfo);

  if(resType == eResFramebuffer || resType == eResRenderPass)
    SAFE_DELETE_ARRAY(imageAttachments);

  // only the descriptor set layout actually owns this pointer, descriptor sets
  // have a pointer to it but don't own it
  if(resType == eResDescriptorSetLayout)
    SAFE_DELETE(descInfo->layout);

  if(resType == eResDescriptorSetLayout || resType == eResDescriptorSet)
    SAFE_DELETE(descInfo);

  if(resType == eResPipelineLayout)
    SAFE_DELETE(pipeLayoutInfo);

  if(resType == eResDescUpdateTemplate)
    SAFE_DELETE(descTemplateInfo);
}

void VkResourceRecord::MarkImageFrameReferenced(VkResourceRecord *img, const ImageRange &range,
                                                FrameRefType refType)
{
  // mark backing memory as read
  MarkResourceFrameReferenced(img->baseResource, eFrameRef_Read);

  ResourceId id = img->GetResourceID();
  if(refType != eFrameRef_Read && refType != eFrameRef_None)
    cmdInfo->dirtied.insert(id);
  if(img->resInfo && img->resInfo->IsSparse())
    cmdInfo->sparse.insert(img->resInfo);

  ImageSubresourceRange range2(range);

  FrameRefType maxRef = MarkImageReferenced(cmdInfo->imageStates, id, img->resInfo->imageInfo,
                                            range2, pool->queueFamilyIndex, refType);

  // maintain the reference type of the image itself as the maximum reference type of any
  // subresource
  MarkResourceFrameReferenced(id, maxRef, ComposeFrameRefsDisjoint);
}

void VkResourceRecord::MarkImageViewFrameReferenced(VkResourceRecord *view, const ImageRange &range,
                                                    FrameRefType refType)
{
  ResourceId img = view->baseResource;
  ResourceId mem = view->baseResourceMem;

  // mark image view as read
  MarkResourceFrameReferenced(view->GetResourceID(), eFrameRef_Read);

  // mark memory backing image as read
  MarkResourceFrameReferenced(mem, eFrameRef_Read);

  if(refType != eFrameRef_Read && refType != eFrameRef_None)
    cmdInfo->dirtied.insert(img);

  ImageSubresourceRange imgRange;
  imgRange.aspectMask = view->viewRange.aspectMask;

  imgRange.baseMipLevel = range.baseMipLevel;
  imgRange.levelCount = range.levelCount;
  ValidateLevelRange(imgRange.baseMipLevel, imgRange.levelCount, view->viewRange.levelCount());
  imgRange.baseMipLevel += view->viewRange.baseMipLevel;

  if(view->resInfo->imageInfo.imageType == VK_IMAGE_TYPE_3D &&
     view->viewRange.viewType() != VK_IMAGE_VIEW_TYPE_3D)
  {
    imgRange.baseDepthSlice = range.baseArrayLayer;
    imgRange.sliceCount = range.layerCount;
    ValidateLayerRange(imgRange.baseDepthSlice, imgRange.sliceCount, view->viewRange.layerCount());
    imgRange.baseDepthSlice += view->viewRange.baseArrayLayer;
  }
  else
  {
    imgRange.baseArrayLayer = range.baseArrayLayer;
    imgRange.layerCount = range.layerCount;
    ValidateLayerRange(imgRange.baseDepthSlice, imgRange.sliceCount, view->viewRange.layerCount());
    imgRange.baseArrayLayer += view->viewRange.baseArrayLayer;
  }
  imgRange.Validate(view->resInfo->imageInfo);

  FrameRefType maxRef = MarkImageReferenced(cmdInfo->imageStates, img, view->resInfo->imageInfo,
                                            imgRange, pool->queueFamilyIndex, refType);

  // maintain the reference type of the image itself as the maximum reference type of any
  // subresource
  MarkResourceFrameReferenced(img, maxRef, ComposeFrameRefsDisjoint);
}

void VkResourceRecord::MarkMemoryFrameReferenced(ResourceId mem, VkDeviceSize offset,
                                                 VkDeviceSize size, FrameRefType refType)
{
  if(refType != eFrameRef_Read && refType != eFrameRef_None)
    cmdInfo->dirtied.insert(mem);
  FrameRefType maxRef = MarkMemoryReferenced(cmdInfo->memFrameRefs, mem, offset, size, refType);
  MarkResourceFrameReferenced(mem, maxRef, ComposeFrameRefsDisjoint);
}

void VkResourceRecord::MarkBufferFrameReferenced(VkResourceRecord *buf, VkDeviceSize offset,
                                                 VkDeviceSize size, FrameRefType refType)
{
  // mark buffer just as read
  MarkResourceFrameReferenced(buf->GetResourceID(), eFrameRef_Read);

  if(size == VK_WHOLE_SIZE)
  {
    size = buf->memSize;
  }
  if(buf->resInfo && buf->resInfo->IsSparse())
    cmdInfo->sparse.insert(buf->resInfo);
  if(buf->baseResource != ResourceId())
    MarkMemoryFrameReferenced(buf->baseResource, buf->memOffset + offset, size, refType);
}

void VkResourceRecord::MarkBufferImageCopyFrameReferenced(VkResourceRecord *buf,
                                                          VkResourceRecord *img, uint32_t regionCount,
                                                          const VkBufferImageCopy *regions,
                                                          FrameRefType bufRefType,
                                                          FrameRefType imgRefType)
{
  if(IsDirtyFrameRef(imgRefType))
    cmdInfo->dirtied.insert(img->GetResourceID());

  // mark buffer just as read
  MarkResourceFrameReferenced(buf->GetResourceID(), eFrameRef_Read);

  VkFormat imgFormat = img->resInfo->imageInfo.format;

  for(uint32_t ri = 0; ri < regionCount; ri++)
  {
    const VkBufferImageCopy &region = regions[ri];

    ImageRange range(region.imageSubresource);
    range.offset = region.imageOffset;
    range.extent = region.imageExtent;

    MarkImageFrameReferenced(img, range, imgRefType);

    VkFormat regionFormat = imgFormat;
    uint32_t plane = 0;
    switch(region.imageSubresource.aspectMask)
    {
      case VK_IMAGE_ASPECT_STENCIL_BIT: regionFormat = VK_FORMAT_S8_UINT; break;
      case VK_IMAGE_ASPECT_DEPTH_BIT: regionFormat = GetDepthOnlyFormat(imgFormat); break;
      case VK_IMAGE_ASPECT_PLANE_1_BIT: plane = 1; break;
      case VK_IMAGE_ASPECT_PLANE_2_BIT: plane = 2; break;
      default: break;
    }

    // The shape of the texel blocks;
    // non-block formats are treated as having 1x1 blocks
    BlockShape blockShape = GetBlockShape(regionFormat, plane);

    // width of copied region, in blocks
    uint32_t widthInBlocks = (region.imageExtent.width + blockShape.width - 1) / blockShape.width;

    // width of copied region, in bytes (in the buffer);
    uint32_t widthInBytes = blockShape.bytes * widthInBlocks;

    // height of copied region, in blocks
    uint32_t heightInBlocks = (region.imageExtent.height + blockShape.height - 1) / blockShape.height;

    // total number of depth slices to be copied.
    uint32_t sliceCount = region.imageExtent.depth * region.imageSubresource.layerCount;

    // stride_y: number of bytes in the buffer between the start of one row of
    // blocks and the next. The buffer may have space for more blocks per row than
    // are actually being copied (specified by bufferRowLength).
    uint32_t stride_y;
    if(region.bufferRowLength == 0)
      stride_y = widthInBytes;
    else
      stride_y = blockShape.bytes * region.bufferRowLength;

    // stride_z: number of bytes in the buffer between the start of one depth
    // slice and the next. The buffer may have space for more rows per slice
    // than are actually being copied (specified by bufferImageHeight).
    uint32_t stride_z;
    if(region.bufferImageHeight == 0)
      stride_z = stride_y * heightInBlocks;
    else
      stride_z = stride_y * region.bufferImageHeight;

    // memory offset of the first byte to be copied to/from the buffer
    VkDeviceSize startRegion = buf->memOffset + region.bufferOffset;

    if(stride_z == widthInBytes * heightInBlocks)
    {
      // no gaps between slices nor between rows; single copy for entire region
      MarkMemoryFrameReferenced(buf->baseResource, startRegion,
                                widthInBytes * heightInBlocks * sliceCount, bufRefType);
    }
    else if(stride_y == widthInBytes)
    {
      // gaps between slices, but no gaps between rows; separate copies per slice
      for(uint32_t z = 0; z < sliceCount; z++)
      {
        VkDeviceSize startSlice = startRegion + z * stride_z;
        MarkMemoryFrameReferenced(buf->baseResource, startSlice, widthInBytes * heightInBlocks,
                                  bufRefType);
      }
    }
    else
    {
      // gaps between rows; separate copies for each row in each slice
      for(uint32_t z = 0; z < sliceCount; z++)
      {
        VkDeviceSize startSlice = startRegion + z * stride_z;
        for(uint32_t y = 0; y < heightInBlocks; y++)
        {
          VkDeviceSize startRow = startSlice + y * stride_y;
          MarkMemoryFrameReferenced(buf->baseResource, startRow, widthInBytes, bufRefType);
        }
      }
    }
  }
}

void VkResourceRecord::MarkBufferViewFrameReferenced(VkResourceRecord *bufView, FrameRefType refType)
{
  // mark the VkBufferView and VkBuffer as read
  MarkResourceFrameReferenced(bufView->GetResourceID(), eFrameRef_Read);
  if(bufView->baseResource != ResourceId())
    MarkResourceFrameReferenced(bufView->baseResource, eFrameRef_Read);

  if(bufView->resInfo && bufView->resInfo->IsSparse())
    cmdInfo->sparse.insert(bufView->resInfo);
  if(bufView->baseResourceMem != ResourceId())
    MarkMemoryFrameReferenced(bufView->baseResourceMem, bufView->memOffset, bufView->memSize,
                              refType);
}

void ResourceInfo::Update(uint32_t numBindings, const VkSparseImageMemoryBind *pBindings)
{
  // update image page table mappings

  for(uint32_t b = 0; b < numBindings; b++)
  {
    const VkSparseImageMemoryBind &newBind = pBindings[b];

    // VKTODOMED handle sparse image arrays or sparse images with mips
    RDCASSERT(newBind.subresource.arrayLayer == 0 && newBind.subresource.mipLevel == 0);

    rdcpair<VkDeviceMemory, VkDeviceSize> *pageTable = pages[newBind.subresource.aspectMask];

    VkOffset3D offsInPages = newBind.offset;
    offsInPages.x /= pagedim.width;
    offsInPages.y /= pagedim.height;
    offsInPages.z /= pagedim.depth;

    VkExtent3D extInPages = newBind.extent;
    extInPages.width /= pagedim.width;
    extInPages.height /= pagedim.height;
    extInPages.depth /= pagedim.depth;

    rdcpair<VkDeviceMemory, VkDeviceSize> mempair =
        make_rdcpair(newBind.memory, newBind.memoryOffset);

    for(uint32_t z = offsInPages.z; z < offsInPages.z + extInPages.depth; z++)
    {
      for(uint32_t y = offsInPages.y; y < offsInPages.y + extInPages.height; y++)
      {
        for(uint32_t x = offsInPages.x; x < offsInPages.x + extInPages.width; x++)
        {
          pageTable[z * imgdim.width * imgdim.height + y * imgdim.width + x] = mempair;
        }
      }
    }
  }
}

void ResourceInfo::Update(uint32_t numBindings, const VkSparseMemoryBind *pBindings)
{
  // update opaque mappings

  for(uint32_t b = 0; b < numBindings; b++)
  {
    const VkSparseMemoryBind &newRange = pBindings[b];

    bool found = false;

    // this could be improved to do a binary search since the vector is sorted.
    // for(auto it = opaquemappings.begin(); it != opaquemappings.end(); ++it)
    for(size_t i = 0; i < opaquemappings.size(); i++)
    {
      VkSparseMemoryBind &curRange = opaquemappings[i];

      // the binding we're applying is after this item in the list,
      // keep searching
      if(curRange.resourceOffset + curRange.size <= newRange.resourceOffset)
        continue;

      // the binding we're applying is before this item, but doesn't
      // overlap. Insert before us in the list
      if(curRange.resourceOffset >= newRange.resourceOffset + newRange.size)
      {
        opaquemappings.insert(i, newRange);
        found = true;
        break;
      }

      // with sparse mappings it will be reasonably common to update an exact
      // existing range, so check that first
      if(newRange.resourceOffset == curRange.resourceOffset && newRange.size == curRange.size)
      {
        curRange = newRange;
        found = true;
        break;
      }

      // handle subranges within the current range
      if(newRange.resourceOffset >= curRange.resourceOffset &&
         newRange.resourceOffset + newRange.size <= curRange.resourceOffset + curRange.size)
      {
        // they start in the same place
        if(newRange.resourceOffset == curRange.resourceOffset)
        {
          // change the current range to be the leftover second half
          curRange.resourceOffset += newRange.size;
          curRange.size -= newRange.size;

          // insert the new mapping before our current one
          opaquemappings.insert(i, newRange);
          found = true;
          break;
        }
        // they end in the same place
        else if(newRange.resourceOffset + newRange.size == curRange.resourceOffset + curRange.size)
        {
          // save a copy
          VkSparseMemoryBind first = curRange;

          // set the new size of the first half
          first.size = newRange.resourceOffset - newRange.resourceOffset;

          // add the new range where the current iterator was
          curRange = newRange;

          // insert the old truncated mapping before our current position
          opaquemappings.insert(i, first);
          found = true;
          break;
        }
        // the new range is a subsection
        else
        {
          // save a copy
          VkSparseMemoryBind first = curRange;

          // set the new size of the first part
          first.size = newRange.resourceOffset - first.resourceOffset;

          // set the current range (third part) to start after the new range ends
          curRange.size =
              (curRange.resourceOffset + curRange.size) - (newRange.resourceOffset + newRange.size);
          curRange.resourceOffset = newRange.resourceOffset + newRange.size;

          // first insert the new range before our current range
          opaquemappings.insert(i, newRange);

          // now insert the remaining first part before that
          opaquemappings.insert(i, first);

          found = true;
          break;
        }
      }

      // this new range overlaps the current one and some subsequent ranges. Merge together

      // find where this new range stops overlapping
      size_t endi = i;
      for(; endi < opaquemappings.size(); endi++)
      {
        if(newRange.resourceOffset + newRange.size <=
           opaquemappings[endi].resourceOffset + opaquemappings[endi].size)
          break;
      }

      VkSparseMemoryBind &endRange = opaquemappings[endi];

      // see if there are any leftovers of the overlapped ranges at the start or end
      bool leftoverstart = (newRange.resourceOffset < curRange.resourceOffset);
      bool leftoverend = (endi < opaquemappings.size() && (endRange.resourceOffset + endRange.size >
                                                           curRange.resourceOffset + curRange.size));

      // no leftovers, the new range entirely covers the current and last (if there is one)
      if(!leftoverstart && !leftoverend)
      {
        // erase all of the ranges. If endi is a valid index, it won't be erased, so we overwrite
        // it. Otherwise there was no subsequent range so we just push_back()
        opaquemappings.erase(i, endi - i);
        if(endi < opaquemappings.size())
          endRange = newRange;
        else
          opaquemappings.push_back(newRange);
      }
      // leftover at the start, but not the end
      else if(leftoverstart && !leftoverend)
      {
        // save the current range
        VkSparseMemoryBind first = curRange;

        // modify the size to reflect what's left over
        first.size = newRange.resourceOffset - first.resourceOffset;

        // as above, erase and either re-insert or push_back()
        opaquemappings.erase(i, endi - i);
        if(endi < opaquemappings.size())
        {
          endRange = newRange;
          opaquemappings.insert(endi, first);
        }
        else
        {
          opaquemappings.push_back(first);
          opaquemappings.push_back(newRange);
        }
      }
      // leftover at the end, but not the start
      else if(!leftoverstart && leftoverend)
      {
        // erase up to but not including endit
        opaquemappings.erase(i, endi - i);
        // modify the leftovers at the end
        endRange.resourceOffset = newRange.resourceOffset + newRange.size;
        // insert the new range before
        opaquemappings.insert(i, newRange);
      }
      // leftovers at both ends
      else
      {
        // save the current range
        VkSparseMemoryBind first = curRange;

        // modify the size to reflect what's left over
        first.size = newRange.resourceOffset - first.resourceOffset;

        // erase up to but not including endit
        opaquemappings.erase(i, endi - i);
        // modify the leftovers at the end
        endRange.size =
            (endRange.resourceOffset + endRange.size) - (newRange.resourceOffset + newRange.size);
        endRange.resourceOffset = newRange.resourceOffset + newRange.size;
        // insert the new range before
        opaquemappings.insert(i, newRange);
        // insert the modified leftovers before that
        opaquemappings.insert(i, first);
      }

      found = true;
      break;
    }

    // if it wasn't found, this binding is after all mappings in our list
    if(!found)
      opaquemappings.push_back(newRange);
  }
}

FrameRefType MarkImageReferenced(std::map<ResourceId, ImageState> &imageStates, ResourceId img,
                                 const ImageInfo &imageInfo, const ImageSubresourceRange &range,
                                 uint32_t queueFamilyIndex, FrameRefType refType,
                                 FrameRefCompFunc compose)
{
  if(refType == eFrameRef_None)
    return refType;
  auto it = imageStates.find(img);
  if(it == imageStates.end())
    it = imageStates.insert({img, ImageState(VK_NULL_HANDLE, imageInfo)}).first;
  it->second.Update(
      range, ImageSubresourceState(queueFamilyIndex, UNKNOWN_PREV_IMG_LAYOUT, refType), compose);
  return it->second.maxRefType;
}

#if ENABLED(ENABLE_UNIT_TESTS)

#undef None

#include "3rdparty/catch/catch.hpp"

TEST_CASE("Vulkan formats", "[format][vulkan]")
{
  // must be updated by hand
  std::initializer_list<VkFormat> formats = {
      VK_FORMAT_UNDEFINED,
      VK_FORMAT_R4G4_UNORM_PACK8,
      VK_FORMAT_R4G4B4A4_UNORM_PACK16,
      VK_FORMAT_B4G4R4A4_UNORM_PACK16,
      VK_FORMAT_R5G6B5_UNORM_PACK16,
      VK_FORMAT_B5G6R5_UNORM_PACK16,
      VK_FORMAT_R5G5B5A1_UNORM_PACK16,
      VK_FORMAT_B5G5R5A1_UNORM_PACK16,
      VK_FORMAT_A1R5G5B5_UNORM_PACK16,
      VK_FORMAT_R8_UNORM,
      VK_FORMAT_R8_SNORM,
      VK_FORMAT_R8_USCALED,
      VK_FORMAT_R8_SSCALED,
      VK_FORMAT_R8_UINT,
      VK_FORMAT_R8_SINT,
      VK_FORMAT_R8_SRGB,
      VK_FORMAT_R8G8_UNORM,
      VK_FORMAT_R8G8_SNORM,
      VK_FORMAT_R8G8_USCALED,
      VK_FORMAT_R8G8_SSCALED,
      VK_FORMAT_R8G8_UINT,
      VK_FORMAT_R8G8_SINT,
      VK_FORMAT_R8G8_SRGB,
      VK_FORMAT_R8G8B8_UNORM,
      VK_FORMAT_R8G8B8_SNORM,
      VK_FORMAT_R8G8B8_USCALED,
      VK_FORMAT_R8G8B8_SSCALED,
      VK_FORMAT_R8G8B8_UINT,
      VK_FORMAT_R8G8B8_SINT,
      VK_FORMAT_R8G8B8_SRGB,
      VK_FORMAT_B8G8R8_UNORM,
      VK_FORMAT_B8G8R8_SNORM,
      VK_FORMAT_B8G8R8_USCALED,
      VK_FORMAT_B8G8R8_SSCALED,
      VK_FORMAT_B8G8R8_UINT,
      VK_FORMAT_B8G8R8_SINT,
      VK_FORMAT_B8G8R8_SRGB,
      VK_FORMAT_R8G8B8A8_UNORM,
      VK_FORMAT_R8G8B8A8_SNORM,
      VK_FORMAT_R8G8B8A8_USCALED,
      VK_FORMAT_R8G8B8A8_SSCALED,
      VK_FORMAT_R8G8B8A8_UINT,
      VK_FORMAT_R8G8B8A8_SINT,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_FORMAT_B8G8R8A8_UNORM,
      VK_FORMAT_B8G8R8A8_SNORM,
      VK_FORMAT_B8G8R8A8_USCALED,
      VK_FORMAT_B8G8R8A8_SSCALED,
      VK_FORMAT_B8G8R8A8_UINT,
      VK_FORMAT_B8G8R8A8_SINT,
      VK_FORMAT_B8G8R8A8_SRGB,
      VK_FORMAT_A8B8G8R8_UNORM_PACK32,
      VK_FORMAT_A8B8G8R8_SNORM_PACK32,
      VK_FORMAT_A8B8G8R8_USCALED_PACK32,
      VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
      VK_FORMAT_A8B8G8R8_UINT_PACK32,
      VK_FORMAT_A8B8G8R8_SINT_PACK32,
      VK_FORMAT_A8B8G8R8_SRGB_PACK32,
      VK_FORMAT_A2R10G10B10_UNORM_PACK32,
      VK_FORMAT_A2R10G10B10_SNORM_PACK32,
      VK_FORMAT_A2R10G10B10_USCALED_PACK32,
      VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
      VK_FORMAT_A2R10G10B10_UINT_PACK32,
      VK_FORMAT_A2R10G10B10_SINT_PACK32,
      VK_FORMAT_A2B10G10R10_UNORM_PACK32,
      VK_FORMAT_A2B10G10R10_SNORM_PACK32,
      VK_FORMAT_A2B10G10R10_USCALED_PACK32,
      VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
      VK_FORMAT_A2B10G10R10_UINT_PACK32,
      VK_FORMAT_A2B10G10R10_SINT_PACK32,
      VK_FORMAT_R16_UNORM,
      VK_FORMAT_R16_SNORM,
      VK_FORMAT_R16_USCALED,
      VK_FORMAT_R16_SSCALED,
      VK_FORMAT_R16_UINT,
      VK_FORMAT_R16_SINT,
      VK_FORMAT_R16_SFLOAT,
      VK_FORMAT_R16G16_UNORM,
      VK_FORMAT_R16G16_SNORM,
      VK_FORMAT_R16G16_USCALED,
      VK_FORMAT_R16G16_SSCALED,
      VK_FORMAT_R16G16_UINT,
      VK_FORMAT_R16G16_SINT,
      VK_FORMAT_R16G16_SFLOAT,
      VK_FORMAT_R16G16B16_UNORM,
      VK_FORMAT_R16G16B16_SNORM,
      VK_FORMAT_R16G16B16_USCALED,
      VK_FORMAT_R16G16B16_SSCALED,
      VK_FORMAT_R16G16B16_UINT,
      VK_FORMAT_R16G16B16_SINT,
      VK_FORMAT_R16G16B16_SFLOAT,
      VK_FORMAT_R16G16B16A16_UNORM,
      VK_FORMAT_R16G16B16A16_SNORM,
      VK_FORMAT_R16G16B16A16_USCALED,
      VK_FORMAT_R16G16B16A16_SSCALED,
      VK_FORMAT_R16G16B16A16_UINT,
      VK_FORMAT_R16G16B16A16_SINT,
      VK_FORMAT_R16G16B16A16_SFLOAT,
      VK_FORMAT_R32_UINT,
      VK_FORMAT_R32_SINT,
      VK_FORMAT_R32_SFLOAT,
      VK_FORMAT_R32G32_UINT,
      VK_FORMAT_R32G32_SINT,
      VK_FORMAT_R32G32_SFLOAT,
      VK_FORMAT_R32G32B32_UINT,
      VK_FORMAT_R32G32B32_SINT,
      VK_FORMAT_R32G32B32_SFLOAT,
      VK_FORMAT_R32G32B32A32_UINT,
      VK_FORMAT_R32G32B32A32_SINT,
      VK_FORMAT_R32G32B32A32_SFLOAT,
      VK_FORMAT_R64_UINT,
      VK_FORMAT_R64_SINT,
      VK_FORMAT_R64_SFLOAT,
      VK_FORMAT_R64G64_UINT,
      VK_FORMAT_R64G64_SINT,
      VK_FORMAT_R64G64_SFLOAT,
      VK_FORMAT_R64G64B64_UINT,
      VK_FORMAT_R64G64B64_SINT,
      VK_FORMAT_R64G64B64_SFLOAT,
      VK_FORMAT_R64G64B64A64_UINT,
      VK_FORMAT_R64G64B64A64_SINT,
      VK_FORMAT_R64G64B64A64_SFLOAT,
      VK_FORMAT_B10G11R11_UFLOAT_PACK32,
      VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
      VK_FORMAT_D16_UNORM,
      VK_FORMAT_X8_D24_UNORM_PACK32,
      VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_S8_UINT,
      VK_FORMAT_D16_UNORM_S8_UINT,
      VK_FORMAT_D24_UNORM_S8_UINT,
      VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_BC1_RGB_UNORM_BLOCK,
      VK_FORMAT_BC1_RGB_SRGB_BLOCK,
      VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
      VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
      VK_FORMAT_BC2_UNORM_BLOCK,
      VK_FORMAT_BC2_SRGB_BLOCK,
      VK_FORMAT_BC3_UNORM_BLOCK,
      VK_FORMAT_BC3_SRGB_BLOCK,
      VK_FORMAT_BC4_UNORM_BLOCK,
      VK_FORMAT_BC4_SNORM_BLOCK,
      VK_FORMAT_BC5_UNORM_BLOCK,
      VK_FORMAT_BC5_SNORM_BLOCK,
      VK_FORMAT_BC6H_UFLOAT_BLOCK,
      VK_FORMAT_BC6H_SFLOAT_BLOCK,
      VK_FORMAT_BC7_UNORM_BLOCK,
      VK_FORMAT_BC7_SRGB_BLOCK,
      VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
      VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
      VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
      VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
      VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
      VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
      VK_FORMAT_EAC_R11_UNORM_BLOCK,
      VK_FORMAT_EAC_R11_SNORM_BLOCK,
      VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
      VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
      VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
      VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
      VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
      VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
      VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
      VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
      VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
      VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
      VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
      VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
      VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
      VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
      VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
      VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
      VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
      VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
      VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
      VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
      VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
      VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
      VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
      VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
      VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
      VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
      VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
      VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
      VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
      VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
      VK_FORMAT_G8B8G8R8_422_UNORM,
      VK_FORMAT_B8G8R8G8_422_UNORM,
      VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
      VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
      VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
      VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
      VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
      VK_FORMAT_R10X6_UNORM_PACK16,
      VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
      VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
      VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
      VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
      VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
      VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
      VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
      VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
      VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
      VK_FORMAT_R12X4_UNORM_PACK16,
      VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
      VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
      VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
      VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
      VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
      VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
      VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
      VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
      VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
      VK_FORMAT_G16B16G16R16_422_UNORM,
      VK_FORMAT_B16G16R16G16_422_UNORM,
      VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
      VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
      VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
      VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
      VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
      VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
      VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
      VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
      VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
      VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
      VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
      VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
      VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
      VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT,
      VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT,
  };

  SECTION("Only VK_FORMAT_UNDEFINED is ResourceFormatType::Undefined")
  {
    for(VkFormat f : formats)
    {
      ResourceFormat fmt = MakeResourceFormat(f);

      if(f == VK_FORMAT_UNDEFINED)
        CHECK(fmt.type == ResourceFormatType::Undefined);
      else
        CHECK(fmt.type != ResourceFormatType::Undefined);
    }
  };

  SECTION("MakeVkFormat is reflexive with MakeResourceFormat")
  {
    for(VkFormat f : formats)
    {
      VkFormat original = f;
      ResourceFormat fmt = MakeResourceFormat(f);

      // astc and pvrtc are not properly supported, collapse to a single type
      if((f >= VK_FORMAT_ASTC_4x4_UNORM_BLOCK && f <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK) ||
         (f >= VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT && f <= VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT))
      {
        CHECK(fmt.type == ResourceFormatType::ASTC);
        continue;
      }
      if(f >= VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG && f <= VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG)
      {
        CHECK(fmt.type == ResourceFormatType::PVRTC);
        continue;
      }

      VkFormat reconstructed = MakeVkFormat(fmt);

      // we are OK with remapping these variants to another similar one, where our format doesn't
      // have enough flexibility to represent the exact type (as a trade-off vs simplicity of
      // processing/storage).
      if(f == VK_FORMAT_A1R5G5B5_UNORM_PACK16)
      {
        CHECK(reconstructed == VK_FORMAT_R5G5B5A1_UNORM_PACK16);
      }
      else if(f == VK_FORMAT_A8B8G8R8_UNORM_PACK32)
      {
        CHECK(reconstructed == VK_FORMAT_R8G8B8A8_UNORM);
      }
      else if(f == VK_FORMAT_A8B8G8R8_SNORM_PACK32)
      {
        CHECK(reconstructed == VK_FORMAT_R8G8B8A8_SNORM);
      }
      else if(f == VK_FORMAT_A8B8G8R8_USCALED_PACK32)
      {
        CHECK(reconstructed == VK_FORMAT_R8G8B8A8_USCALED);
      }
      else if(f == VK_FORMAT_A8B8G8R8_SSCALED_PACK32)
      {
        CHECK(reconstructed == VK_FORMAT_R8G8B8A8_SSCALED);
      }
      else if(f == VK_FORMAT_A8B8G8R8_UINT_PACK32)
      {
        CHECK(reconstructed == VK_FORMAT_R8G8B8A8_UINT);
      }
      else if(f == VK_FORMAT_A8B8G8R8_SINT_PACK32)
      {
        CHECK(reconstructed == VK_FORMAT_R8G8B8A8_SINT);
      }
      else if(f == VK_FORMAT_A8B8G8R8_SRGB_PACK32)
      {
        CHECK(reconstructed == VK_FORMAT_R8G8B8A8_SRGB);
      }
      else if(f == VK_FORMAT_X8_D24_UNORM_PACK32)
      {
        CHECK(reconstructed == VK_FORMAT_D24_UNORM_S8_UINT);
      }
      else
      {
        CHECK(reconstructed == original);
      }
    }
  };

  SECTION("MakeVkFormat concurs with helpers")
  {
    for(VkFormat f : formats)
    {
      ResourceFormat fmt = MakeResourceFormat(f);

      INFO("Format is " << ToStr(f));

      if(IsBlockFormat(f))
      {
        INFO("Format type is " << ToStr(fmt.type));

        bool bcn = fmt.type >= ResourceFormatType::BC1 && fmt.type <= ResourceFormatType::BC7;

        CHECK((bcn || fmt.type == ResourceFormatType::ASTC || fmt.type == ResourceFormatType::EAC ||
               fmt.type == ResourceFormatType::ETC2 || fmt.type == ResourceFormatType::PVRTC));
      }

      if(IsYUVFormat(f))
      {
        CHECK(fmt.type >= ResourceFormatType::YUV8);
        CHECK(fmt.type <= ResourceFormatType::YUV16);
      }

      if(IsDepthOrStencilFormat(f))
      {
        CHECK(fmt.compType == CompType::Depth);
      }
      else if(IsUIntFormat(f))
      {
        CHECK(fmt.compType == CompType::UInt);
      }
      else if(IsSIntFormat(f))
      {
        CHECK(fmt.compType == CompType::SInt);
      }

      if(IsSRGBFormat(f))
      {
        CHECK(fmt.SRGBCorrected());
      }
    }
  };

  SECTION("GetByteSize return expected values for regular formats")
  {
    for(VkFormat f : formats)
    {
      ResourceFormat fmt = MakeResourceFormat(f);

      if(fmt.type != ResourceFormatType::Regular)
        continue;

      INFO("Format is " << ToStr(f));

      uint32_t size = fmt.compCount * fmt.compByteWidth * 123 * 456;

      CHECK(size == GetByteSize(123, 456, 1, f, 0));
    }
  };

  SECTION("GetByteSize for BCn formats")
  {
    const uint32_t width = 24, height = 24;

    // reference: 24x24 = 576, 576/2 = 288

    const uint32_t bcnsizes[] = {
        288,    // VK_FORMAT_BC1_RGB_UNORM_BLOCK
        288,    // VK_FORMAT_BC1_RGB_SRGB_BLOCK
        288,    // VK_FORMAT_BC1_RGBA_UNORM_BLOCK
        288,    // VK_FORMAT_BC1_RGBA_SRGB_BLOCK = 0.5 byte/px
        576,    // VK_FORMAT_BC2_UNORM_BLOCK
        576,    // VK_FORMAT_BC2_SRGB_BLOCK = 1 byte/px
        576,    // VK_FORMAT_BC3_UNORM_BLOCK
        576,    // VK_FORMAT_BC3_SRGB_BLOCK = 1 byte/px
        288,    // VK_FORMAT_BC4_UNORM_BLOCK
        288,    // VK_FORMAT_BC4_SNORM_BLOCK = 0.5 byte/px
        576,    // VK_FORMAT_BC5_UNORM_BLOCK
        576,    // VK_FORMAT_BC5_SNORM_BLOCK = 1 byte/px
        576,    // VK_FORMAT_BC6H_UFLOAT_BLOCK
        576,    // VK_FORMAT_BC6H_SFLOAT_BLOCK = 1 byte/px
        576,    // VK_FORMAT_BC7_UNORM_BLOCK
        576,    // VK_FORMAT_BC7_SRGB_BLOCK = 1 byte/px
    };

    int i = 0;
    for(VkFormat f : {
            VK_FORMAT_BC1_RGB_UNORM_BLOCK, VK_FORMAT_BC1_RGB_SRGB_BLOCK,
            VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
            VK_FORMAT_BC2_UNORM_BLOCK, VK_FORMAT_BC2_SRGB_BLOCK, VK_FORMAT_BC3_UNORM_BLOCK,
            VK_FORMAT_BC3_SRGB_BLOCK, VK_FORMAT_BC4_UNORM_BLOCK, VK_FORMAT_BC4_SNORM_BLOCK,
            VK_FORMAT_BC5_UNORM_BLOCK, VK_FORMAT_BC5_SNORM_BLOCK, VK_FORMAT_BC6H_UFLOAT_BLOCK,
            VK_FORMAT_BC6H_SFLOAT_BLOCK, VK_FORMAT_BC7_UNORM_BLOCK, VK_FORMAT_BC7_SRGB_BLOCK,
        })
    {
      INFO("Format is " << ToStr(f));

      CHECK(bcnsizes[i++] == GetByteSize(width, height, 1, f, 0));
    }
  };

  SECTION("GetByteSize for YUV formats")
  {
    const uint32_t width = 24, height = 24;

    const uint32_t yuvsizes[] = {
        1152,    // VK_FORMAT_G8B8G8R8_422_UNORM (4:2:2 8-bit packed)
        1152,    // VK_FORMAT_B8G8R8G8_422_UNORM (4:2:2 8-bit packed)
        864,     // VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM (4:2:0 8-bit 3-plane)
        864,     // VK_FORMAT_G8_B8R8_2PLANE_420_UNORM (4:2:0 8-bit 2-plane)
        1152,    // VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM (4:2:2 8-bit 3-plane)
        1152,    // VK_FORMAT_G8_B8R8_2PLANE_422_UNORM (4:2:2 8-bit 2-plane)
        1728,    // VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM (4:4:4 8-bit 3-plane)
        1152,    // VK_FORMAT_R10X6_UNORM_PACK16 (4:4:4 10-bit packed)
        2304,    // VK_FORMAT_R10X6G10X6_UNORM_2PACK16 (4:4:4 10-bit packed)
        4608,    // VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 (4:4:4 10-bit packed)
        2304,    // VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 (4:2:2 10-bit packed)
        2304,    // VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 (4:2:2 10-bit packed)
        1728,    // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 (4:2:0 10-bit 3-plane)
        1728,    // VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 (4:2:0 10-bit 2-plane)
        2304,    // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 (4:2:2 10-bit 3-plane)
        2304,    // VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 (4:2:2 10-bit 2-plane)
        3456,    // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 (4:4:4 10-bit 3-plane)
        1152,    // VK_FORMAT_R12X4_UNORM_PACK16 (4:4:4 12-bit packed)
        2304,    // VK_FORMAT_R12X4G12X4_UNORM_2PACK16 (4:4:4 12-bit packed)
        4608,    // VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 (4:4:4 12-bit packed)
        2304,    // VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 (4:2:2 12-bit packed)
        2304,    // VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 (4:2:2 12-bit packed)
        1728,    // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 (4:2:0 12-bit 3-plane)
        1728,    // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 (4:2:0 12-bit 2-plane)
        2304,    // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 (4:2:2 12-bit 3-plane)
        2304,    // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 (4:2:2 12-bit 2-plane)
        3456,    // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 (4:4:4 12-bit 3-plane)
        2304,    // VK_FORMAT_G16B16G16R16_422_UNORM (4:2:2 16-bit packed)
        2304,    // VK_FORMAT_B16G16R16G16_422_UNORM (4:2:2 16-bit packed)
        1728,    // VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM (4:2:0 16-bit 3-plane)
        1728,    // VK_FORMAT_G16_B16R16_2PLANE_420_UNORM (4:2:0 16-bit 2-plane)
        2304,    // VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM (4:2:2 16-bit 3-plane)
        2304,    // VK_FORMAT_G16_B16R16_2PLANE_422_UNORM (4:2:2 16-bit 2-plane)
        3456,    // VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM (4:4:4 16-bit 3-plane)
    };

    int i = 0;
    for(VkFormat f : {
            VK_FORMAT_G8B8G8R8_422_UNORM,
            VK_FORMAT_B8G8R8G8_422_UNORM,
            VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
            VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
            VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
            VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
            VK_FORMAT_R10X6_UNORM_PACK16,
            VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
            VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
            VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
            VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
            VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
            VK_FORMAT_R12X4_UNORM_PACK16,
            VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
            VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
            VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
            VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
            VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
            VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
            VK_FORMAT_G16B16G16R16_422_UNORM,
            VK_FORMAT_B16G16R16G16_422_UNORM,
            VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
            VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
            VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
            VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
            VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
        })
    {
      INFO("Format is " << ToStr(f));

      CHECK(yuvsizes[i++] == GetByteSize(width, height, 1, f, 0));
    }
  };

  SECTION("GetPlaneByteSize for planar YUV formats")
  {
    const uint32_t width = 24, height = 24;

    rdcarray<rdcpair<VkFormat, rdcarray<uint32_t> > > tests = {
        {VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, {576, 144, 144}},
        {VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, {576, 288}},
        {VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM, {576, 288, 288}},
        {VK_FORMAT_G8_B8R8_2PLANE_422_UNORM, {576, 576}},
        {VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, {576, 576, 576}},
        {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, {1152, 288, 288}},
        {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16, {1152, 576}},
        {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, {1152, 576, 576}},
        {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16, {1152, 1152}},
        {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, {1152, 1152, 1152}},
        {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, {1152, 288, 288}},
        {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16, {1152, 576}},
        {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, {1152, 576, 576}},
        {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16, {1152, 1152}},
        {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, {1152, 1152, 1152}},
        {VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM, {1152, 288, 288}},
        {VK_FORMAT_G16_B16R16_2PLANE_420_UNORM, {1152, 576}},
        {VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM, {1152, 576, 576}},
        {VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, {1152, 1152}},
        {VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM, {1152, 1152, 1152}},
    };

    for(rdcpair<VkFormat, rdcarray<uint32_t> > e : tests)
    {
      INFO("Format is " << ToStr(e.first));
      for(uint32_t p = 0; p < e.second.size(); p++)
        CHECK(e.second[p] == GetPlaneByteSize(width, height, 1, e.first, 0, p));
    }
  };

  SECTION("GetPlaneByteSize is consistent with GetByteSize")
  {
    const uint32_t width = 24, height = 24;

    for(VkFormat f : formats)
    {
      if(f == VK_FORMAT_UNDEFINED)
        continue;

      INFO("Format is " << ToStr(f));

      uint32_t planeCount = GetYUVPlaneCount(f);

      uint32_t planeSum = 0;
      for(uint32_t p = 0; p < planeCount; p++)
        planeSum += GetPlaneByteSize(width, height, 1, f, 0, p);

      CHECK(planeSum == GetByteSize(width, height, 1, f, 0));
    }
  };
};

#endif    // ENABLED(ENABLE_UNIT_TESTS)

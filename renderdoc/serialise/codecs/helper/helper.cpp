#include "helper.h"
#include <string>

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

void RegisterDebugCallback(AuxVkTraceResources aux, VkInstance instance,
                           VkDebugReportFlagBitsEXT flags)
{
  PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
  CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugReportCallbackEXT");

  VkDebugReportCallbackCreateInfoEXT ci = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
                                           0, flags, DebugCallback, NULL};
  if(CreateDebugReportCallback != NULL)
  {
    VkResult result = CreateDebugReportCallback(instance, &ci, NULL, &aux.callback);
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

uint32_t FixCompressedSizes(VkFormat fmt, VkExtent3D &dim, uint32_t &offset)
{
  switch(fmt)
  {
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
      dim.width = (uint32_t)AlignedSize(dim.width, 4);
      dim.height = (uint32_t)AlignedSize(dim.height, 4);
      dim.depth = (uint32_t)AlignedSize(dim.depth, 1);
      offset = (uint32_t)AlignedSize(offset, 4);
      return 1;
  }

  offset = (uint32_t)AlignedSize(offset, 4);
  return 0;
}

double SizeOfFormat(VkFormat fmt)
{
  switch(fmt)
  {
    case VK_FORMAT_R4G4_UNORM_PACK8:
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SRGB: return 1.0;

    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
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

    case VK_FORMAT_D16_UNORM: return 2.0;

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
    case VK_FORMAT_B8G8R8_SRGB: return 3.0;

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
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
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
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return 4.0;

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
    case VK_FORMAT_R64_SFLOAT: return 8.0;

    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT: return 16.0;

    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK: return 1.0;

    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK: return 0.5;

    default: assert(0);
  }
  return 0.0;
}

int MinDimensionSize(VkFormat format)
{
  switch(format)
  {
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK: return 4;

    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return 4;

    default: return 1;
  }
}

VkImageAspectFlags GetFullAspectFromFormat(VkFormat fmt)
{
  if(fmt == VK_FORMAT_D16_UNORM || fmt == VK_FORMAT_D32_SFLOAT)
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  else if(fmt == VK_FORMAT_D16_UNORM_S8_UINT || fmt == VK_FORMAT_D24_UNORM_S8_UINT ||
          fmt == VK_FORMAT_D32_SFLOAT_S8_UINT)
    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  else
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageAspectFlags GetAspectFromFormat(VkFormat fmt)
{
  if(fmt == VK_FORMAT_D16_UNORM || fmt == VK_FORMAT_D32_SFLOAT || fmt == VK_FORMAT_D16_UNORM_S8_UINT ||
     fmt == VK_FORMAT_D24_UNORM_S8_UINT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT)
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  else
    return VK_IMAGE_ASPECT_COLOR_BIT;
}


void ImageLayoutTransition(VkCommandBuffer cmdBuffer, VkImage dstImage,
  VkImageSubresourceRange subresourceRange, VkImageLayout newLayout, 
  uint32_t dstQueueFamily, VkImageLayout oldLayout, uint32_t srcQueueFamily) {
  uint32_t all_access =
    VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
    VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
    VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_READ_BIT |
    VK_ACCESS_HOST_WRITE_BIT;

  VkImageMemoryBarrier imgBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    NULL, all_access, VK_ACCESS_TRANSFER_WRITE_BIT,
    oldLayout, newLayout,
    srcQueueFamily, dstQueueFamily,
    dstImage, subresourceRange};

  vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imgBarrier);
}

void ImageLayoutTransition(AuxVkTraceResources aux, VkImage dstImage,
                           VkImageSubresourceRange subresourceRange, 
                           VkImageLayout newLayout, VkImageLayout oldLayout)
{
  ImageLayoutTransition(aux.command_buffer, dstImage, subresourceRange,
    newLayout, VK_QUEUE_FAMILY_IGNORED, oldLayout, VK_QUEUE_FAMILY_IGNORED);
}

void ImageLayoutTransition(AuxVkTraceResources aux, VkImage dstImage, VkImageCreateInfo dstCI,
                           VkImageLayout newLayout, VkImageLayout oldLayout)
{
  VkImageSubresourceRange subresourceRange = {GetFullAspectFromFormat(dstCI.format), 0,
                                              VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};

  ImageLayoutTransition(aux, dstImage, subresourceRange,
    newLayout, oldLayout);
}

void CopyResetImage(AuxVkTraceResources aux, VkImage dst, VkBuffer src, VkImageCreateInfo dst_ci)
{
  ImageLayoutTransition(aux, dst, dst_ci, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  if(dst_ci.samples == VK_SAMPLE_COUNT_1_BIT)
  {
    std::vector<VkBufferImageCopy> regions;
    uint32_t offset = 0;
    for(uint32_t a = 0; a < dst_ci.arrayLayers; a++)
    {
      VkExtent3D dim = dst_ci.extent;
      uint32_t x = 0;
      FixCompressedSizes(dst_ci.format, dim, x);
      for(uint32_t i = 0; i < dst_ci.mipLevels; i++)
      {
        VkBufferImageCopy region = {offset,     dim.width,
                                    dim.height, {GetAspectFromFormat(dst_ci.format), i, a, 1},
                                    {0, 0, 0},  dim};
        offset += (uint32_t)(dim.depth * dim.width * dim.height * SizeOfFormat(dst_ci.format));
        dim.height = std::max<int>(dim.height / 2, 1);
        dim.width = std::max<int>(dim.width / 2, 1);
        dim.depth = std::max<int>(dim.depth / 2, 1);
        FixCompressedSizes(dst_ci.format, dim, offset);
        regions.push_back(region);
      }
    }
    const uint32_t kMaxUpdate = 100;
    for(uint32_t i = 0; i * kMaxUpdate < regions.size(); i++)
    {
      uint32_t count = std::min<uint32_t>(kMaxUpdate, (uint32_t)regions.size() - i * kMaxUpdate);
      uint32_t offset = i * kMaxUpdate;
      vkCmdCopyBufferToImage(aux.command_buffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             count, regions.data() + offset);
    }
  }
}
void CopyResetBuffer(AuxVkTraceResources aux, VkBuffer dst, VkBuffer src, VkDeviceSize size)
{
  if(size == 0)
    return;
  VkBufferCopy region = {0, 0, size};
  vkCmdCopyBuffer(aux.command_buffer, src, dst, 1, &region);
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

void InitializeAuxResources(AuxVkTraceResources *aux, VkInstance instance, VkPhysicalDevice physDevice, VkDevice device)
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
  FILE *f = fopen(name, "rb");
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

/* This function is complication enough to have a comment. First, consider the following:

1. When capturing was done, a memory allocation was created and resource where mapped to it. For
exammple like this:

Captured Memory Allocation #1:
/---------------------------------------------------------------------------------------------------\
|--|*Res0*|---|***Res1***|-------|**Res2**|------|********Res3********|---------|****Res4****|------|
\---------------------------------------------------------------------------------------------------/

'-' depicts space between allocations.
|****| - depicts a resource.

 These allocations are not tightly packed and there can be at least 2 reasons for that:
A. There are memory alignment requirements that make for extra bytes between resources.
B. The application had more resources in that memory allocations, but because they are
 not used inside the current trace, they are ignored and never created.

 During Replay process all these resources will get realligned and packed as tightly
 as (A) allows. Let's assume that at least one '-' is required between resources. During
 replay that memory allocation will look like this:

 Replayed Memory Allocation #1:
 /------------------------------------------------------------------------\
 |-|*Res0*|-|***Res1***|-|**Res2**|-|********Res3********|-|****Res4****|-|
 \------------------------------------------------------------------------/
 Note, how this allocation can be potentially smaller. The remap argument to MapUpdateAliased and
MapUpdate
 functions contains the (offset, size) pairs that map resource binding between capture and replay.

 Now consider that application did some crazy aliasing during capture. Below is the same allocation,
with
 different rows representing how different resources can be aliased:

 Captured Memory Allocation #2:
 /---------------------------------------------------------------------------------------------------\
 |--|*Res0*|---|***Res1***|-------|**Res2**|------|********Res3********|---------|****Res4****|------|
 |---|Res5|---|***********Res6***********|-------|**Res7**|------|*************Res8***********|------|
 \---------------------------------------------------------------------------------------------------/
 Res5 and Res0 overlap in memory, Res1 and Res2 overlap with Res6, Res3 overlaps with Res7 and Res8,
 Res4 overlaps with Res8.
 It's hard to tell, how these resources need to be remapped and potentially it may be impossible
 to remap them properly because there isn't enough information. This is why during replay all of the
aliased
 resources are linearized in memory, and here is how this memory allocation would look in replay:
 Replayed Memory Allocation #2:
 /------------------------------------------------------------------------------------------------------------------------------------------------------\
 |-|*Res0*|-|***Res1***|-|**Res2**|-|********Res3********|-|****Res4****|-|Res5|-|***********Res6***********|-|**Res7**|-|*************Res8***********|-|
 \------------------------------------------------------------------------------------------------------------------------------------------------------/
 Note, how in this case the replayed memory allocation becomes larger.

 Now consider what happens when a piece of memory is updated. In contrast to other APIs, where the
update
 happens per resource, in Vulkan, the update happens on a piece of memory:

 Captured Memory Allocation #2 with a Map:
 /---------------------------------------------------------------------------------------------------\
 |--|*Res0*|---|***Res1***|-------|**Res2**|------|********Res3********|---------|****Res4****|------|
 |---|Res5|---|***********Res6***********|-------|**Res7**|------|*************Res8***********|------|
 \---------------------------------------------------------------------------------------------------/
 \--------------|+++++++++++ Map
+++++++++++|--------------------------------------------------------/
 As you can see here, this Map region updates Res1, Res2 and Res6. Res6 is aliased with Res1 and
Res2.

 During replay, all these memory resources are bound to their own, private memory chunks, so all of
them
 have to be updated independently.
*/
void MapUpdateAliased(uint8_t *dst, uint8_t *src, const VkMappedMemoryRange &range,
                      VkMemoryAllocateInfo &ai, MemoryRemapVec &remap, VkDevice dev)
{
  if(dst != NULL)
  {
    std::vector<VkMappedMemoryRange> ranges;
    Region memory_region(range.offset, range.size);

    if(remap.size() > 0)
    {
      for(uint32_t i = 0; i < remap.size(); i++)
      {
        MemoryRemap mr = remap[i];
        Region captured_resource_region(mr.capture.offset, mr.capture.size);
        // If this memory range doesn't overlap with any captured resource continue
        if(!RegionsOverlap(memory_region, captured_resource_region))
          continue;

        // Find the inteval where these two regions overlap. It is guaranteed to be non-null.
        Region intersect = RegionsIntersect(memory_region, captured_resource_region);

        // The captured region for the resource can be larger than the replay region for the
        // same resource. Here is a drawing:
        // captured memory:
        //          Res0 start             Res0 end
        //         \/                         \/
        // |--------|******* Res0 *******bbbbb|-------|******* Res1 *******bbbbbbb|---|
        // map:
        // |--------------------|++++++++???????????? Map +++++++++++++++++?|---------|
        //         /\ Unchanged /\
        //            These (intersect.offset - capture.offset) bytes are unaffected by Map
        // replay memory
        // |-|******* Res0 *******bb|-|******* Res1 *******bb|-|
        // So in the drawing above, 'bbb' are auxilary bytes that the driver requires a Vulkan
        // appliction to allocate on top of the app-defined size in bytes. This typically comes
        // from VkMemoryRequirements.size value. That value can be different on different IHVs,
        // meaning that the sizes for the capture and replay regions can be different. The '?'
        // marks indicate the bytes that need to be carefully copied.
        // The size can be no more than to cover the replay.offset + replay.size point.
        uint64_t skipped_resource_bytes = intersect.offset - mr.capture.offset;
        uint64_t skipped_memory_bytes = intersect.offset - memory_region.offset;
        intersect.size = std::min<uint64_t>(intersect.size, mr.replay.size);

        memcpy(dst + mr.replay.offset + skipped_resource_bytes, src + skipped_memory_bytes,
               intersect.size);

        VkMappedMemoryRange r = range;
        r.offset = mr.replay.offset + skipped_resource_bytes;
        r.size = intersect.size;
        ranges.push_back(r);
      }
    }
    else
    {
      assert(0);    // this should never be called here.
    }

    VkResult result = vkFlushMappedMemoryRanges(dev, (uint32_t)ranges.size(), ranges.data());
    assert(result == VK_SUCCESS);
  }
}

void MapUpdate(AuxVkTraceResources aux, uint8_t *dst, uint8_t *src, const VkMappedMemoryRange &range,
               VkMemoryAllocateInfo &ai, MemoryRemapVec &remap, VkDevice dev)
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
        // If this memory range doesn't overlap with any captured resource continue
        if(!RegionsOverlap(memory_region, captured_resource_region))
          continue;

        // Find the inteval where these two regions overlap. It is guaranteed to be non-null.
        Region intersect = RegionsIntersect(memory_region, captured_resource_region);

        uint64_t skipped_resource_bytes = intersect.offset - mr.capture.offset;
        uint64_t skipped_memory_bytes = intersect.offset - memory_region.offset;
        intersect.size = std::min<uint64_t>(intersect.size, mr.replay.size);

        memcpy(dst + mr.replay.offset + skipped_resource_bytes, src + skipped_memory_bytes,
               intersect.size);

        VkMappedMemoryRange r = range;
        r.offset = mr.replay.offset + skipped_resource_bytes;
        r.size = AlignedSize(r.offset + intersect.size, aux.physDeviceProperties.limits.nonCoherentAtomSize);
        r.offset = AlignedDown(r.offset, aux.physDeviceProperties.limits.nonCoherentAtomSize);
        r.size = r.size - r.offset;
        if (r.offset + r.size > range.offset + range.size || 
            r.offset + r.size > ai.allocationSize) {
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

std::string StageProgressString(const char *stage, uint32_t i, uint32_t N) {
  return std::string("RenderDoc Frame Loop: " + std::string(stage) + " part" + std::to_string(i) + " of" + std::to_string(N));
}
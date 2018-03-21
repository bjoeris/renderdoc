/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2018 Baldur Karlsson
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

#include "../vk_core.h"

static char fakeRenderDocUUID[VK_UUID_SIZE + 1] = {};

void MakeFakeUUID()
{
  // assign a fake UUID, so that we get SPIR-V instead of cached pipeline data.
  // the start is "rdoc", and the end is the time that this call was first made
  if(fakeRenderDocUUID[0] == 0)
  {
    // 0123456789ABCDEF
    // rdocyymmddHHMMSS
    // we pass size+1 so that there's room for a null terminator (the UUID doesn't
    // need a null terminator as it's a fixed size non-string array)
    StringFormat::sntimef(fakeRenderDocUUID, VK_UUID_SIZE + 1, "rdoc%y%m%d%H%M%S");
  }
}

void WrappedVulkan::vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                                VkPhysicalDeviceFeatures *pFeatures)
{
  ObjDisp(physicalDevice)->GetPhysicalDeviceFeatures(Unwrap(physicalDevice), pFeatures);
}

void WrappedVulkan::vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice,
                                                        VkFormat format,
                                                        VkFormatProperties *pFormatProperties)
{
  ObjDisp(physicalDevice)
      ->GetPhysicalDeviceFormatProperties(Unwrap(physicalDevice), format, pFormatProperties);
}

VkResult WrappedVulkan::vkGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkImageFormatProperties *pImageFormatProperties)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceImageFormatProperties(Unwrap(physicalDevice), format, type, tiling, usage,
                                               flags, pImageFormatProperties);
}

void WrappedVulkan::vkGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
    VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties)
{
  ObjDisp(physicalDevice)
      ->GetPhysicalDeviceSparseImageFormatProperties(Unwrap(physicalDevice), format, type, samples,
                                                     usage, tiling, pPropertyCount, pProperties);
}

void WrappedVulkan::vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceProperties *pProperties)
{
  ObjDisp(physicalDevice)->GetPhysicalDeviceProperties(Unwrap(physicalDevice), pProperties);

  MakeFakeUUID();

  memcpy(pProperties->pipelineCacheUUID, fakeRenderDocUUID, VK_UUID_SIZE);
}

void WrappedVulkan::vkGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice physicalDevice, uint32_t *pCount, VkQueueFamilyProperties *pQueueFamilyProperties)
{
  // pretend to only have one queue, the one with graphics capability
  if(pCount)
    *pCount = 1;

  if(pQueueFamilyProperties)
  {
    // find the matching physical device
    for(size_t i = 0; i < m_PhysicalDevices.size(); i++)
      if(m_PhysicalDevices[i] == physicalDevice)
        *pQueueFamilyProperties = m_SupportedQueueFamilies[i].second;
    return;
  }
}

void WrappedVulkan::vkGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
  if(pMemoryProperties)
  {
    *pMemoryProperties = *GetRecord(physicalDevice)->memProps;
    return;
  }

  ObjDisp(physicalDevice)->GetPhysicalDeviceMemoryProperties(Unwrap(physicalDevice), pMemoryProperties);
}

void WrappedVulkan::vkGetImageSubresourceLayout(VkDevice device, VkImage image,
                                                const VkImageSubresource *pSubresource,
                                                VkSubresourceLayout *pLayout)
{
  ObjDisp(device)->GetImageSubresourceLayout(Unwrap(device), Unwrap(image), pSubresource, pLayout);
}

void WrappedVulkan::vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                  VkMemoryRequirements *pMemoryRequirements)
{
  ObjDisp(device)->GetBufferMemoryRequirements(Unwrap(device), Unwrap(buffer), pMemoryRequirements);

  // don't do remapping here on replay.
  if(IsReplayMode(m_State))
    return;

  uint32_t bits = pMemoryRequirements->memoryTypeBits;
  uint32_t *memIdxMap = GetRecord(device)->memIdxMap;

  pMemoryRequirements->memoryTypeBits = 0;

  // for each of our fake memory indices, check if the real
  // memory type it points to is set - if so, set our fake bit
  for(uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
    if(memIdxMap[i] < 32U && (bits & (1U << memIdxMap[i])))
      pMemoryRequirements->memoryTypeBits |= (1U << i);
}

void WrappedVulkan::vkGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                 VkMemoryRequirements *pMemoryRequirements)
{
  ObjDisp(device)->GetImageMemoryRequirements(Unwrap(device), Unwrap(image), pMemoryRequirements);

  // don't do remapping here on replay.
  if(IsReplayMode(m_State))
    return;

  uint32_t bits = pMemoryRequirements->memoryTypeBits;
  uint32_t *memIdxMap = GetRecord(device)->memIdxMap;

  pMemoryRequirements->memoryTypeBits = 0;

  // for each of our fake memory indices, check if the real
  // memory type it points to is set - if so, set our fake bit
  for(uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
    if(memIdxMap[i] < 32U && (bits & (1U << memIdxMap[i])))
      pMemoryRequirements->memoryTypeBits |= (1U << i);

  // AMD can have some variability in the returned size, so we need to pad the reported size to
  // allow for this. The variability isn't quite clear, but for now we assume aligning size to
  // alignment * 4 should be sufficient (adding on a fixed padding won't help the problem as it
  // won't remove the variability, nor will adding then aligning for the same reason).
  if(GetDriverVersion().Vendor() == GPUVendor::AMD && pMemoryRequirements->size > 0)
  {
    VkMemoryRequirements &memreq = *pMemoryRequirements;

    VkDeviceSize oldsize = memreq.size;
    memreq.size = AlignUp(memreq.size, memreq.alignment * 4);

    // if it's already 'super aligned', then bump it up a little. We assume that this case
    // represents the low-end of the variation range, and other variations will be a little higher.
    // The other alternative is the variations are all lower and this one happened to be super
    // aligned, which I think (arbitrarily really) is less likely.
    if(oldsize == memreq.size)
      memreq.size = AlignUp(memreq.size + 1, memreq.alignment * 4);

    RDCDEBUG(
        "Padded image memory requirements from %llu to %llu (base alignment %llu) (%f%% increase)",
        oldsize, memreq.size, memreq.alignment,
        (100.0 * double(memreq.size - oldsize)) / double(oldsize));
  }
}

void WrappedVulkan::vkGetImageSparseMemoryRequirements(
    VkDevice device, VkImage image, uint32_t *pNumRequirements,
    VkSparseImageMemoryRequirements *pSparseMemoryRequirements)
{
  ObjDisp(device)->GetImageSparseMemoryRequirements(Unwrap(device), Unwrap(image), pNumRequirements,
                                                    pSparseMemoryRequirements);
}

void WrappedVulkan::vkGetBufferMemoryRequirements2KHR(VkDevice device,
                                                      const VkBufferMemoryRequirementsInfo2KHR *pInfo,
                                                      VkMemoryRequirements2KHR *pMemoryRequirements)
{
  VkBufferMemoryRequirementsInfo2KHR unwrappedInfo = *pInfo;
  unwrappedInfo.buffer = Unwrap(unwrappedInfo.buffer);
  ObjDisp(device)->GetBufferMemoryRequirements2KHR(Unwrap(device), &unwrappedInfo,
                                                   pMemoryRequirements);

  // don't do remapping here on replay.
  if(IsReplayMode(m_State))
    return;

  uint32_t bits = pMemoryRequirements->memoryRequirements.memoryTypeBits;
  uint32_t *memIdxMap = GetRecord(device)->memIdxMap;

  pMemoryRequirements->memoryRequirements.memoryTypeBits = 0;

  // for each of our fake memory indices, check if the real
  // memory type it points to is set - if so, set our fake bit
  for(uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
    if(memIdxMap[i] < 32U && (bits & (1U << memIdxMap[i])))
      pMemoryRequirements->memoryRequirements.memoryTypeBits |= (1U << i);
}

void WrappedVulkan::vkGetImageMemoryRequirements2KHR(VkDevice device,
                                                     const VkImageMemoryRequirementsInfo2KHR *pInfo,
                                                     VkMemoryRequirements2KHR *pMemoryRequirements)
{
  VkImageMemoryRequirementsInfo2KHR unwrappedInfo = *pInfo;
  unwrappedInfo.image = Unwrap(unwrappedInfo.image);
  ObjDisp(device)->GetImageMemoryRequirements2KHR(Unwrap(device), &unwrappedInfo,
                                                  pMemoryRequirements);

  // don't do remapping here on replay.
  if(IsReplayMode(m_State))
    return;

  uint32_t bits = pMemoryRequirements->memoryRequirements.memoryTypeBits;
  uint32_t *memIdxMap = GetRecord(device)->memIdxMap;

  pMemoryRequirements->memoryRequirements.memoryTypeBits = 0;

  // for each of our fake memory indices, check if the real
  // memory type it points to is set - if so, set our fake bit
  for(uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
    if(memIdxMap[i] < 32U && (bits & (1U << memIdxMap[i])))
      pMemoryRequirements->memoryRequirements.memoryTypeBits |= (1U << i);

  // AMD can have some variability in the returned size, so we need to pad the reported size to
  // allow for this. The variability isn't quite clear, but for now we assume aligning size to
  // alignment * 4 should be sufficient (adding on a fixed padding won't help the problem as it
  // won't remove the variability, nor will adding then aligning for the same reason).
  if(GetDriverVersion().Vendor() == GPUVendor::AMD && pMemoryRequirements->memoryRequirements.size > 0)
  {
    VkMemoryRequirements &memreq = pMemoryRequirements->memoryRequirements;

    VkDeviceSize oldsize = memreq.size;
    memreq.size = AlignUp(memreq.size, memreq.alignment * 4);

    // if it's already 'super aligned', then bump it up a little. We assume that this case
    // represents the low-end of the variation range, and other variations will be a little higher.
    // The other alternative is the variations are all lower and this one happened to be super
    // aligned, which I think (arbitrarily really) is less likely.
    if(oldsize == memreq.size)
      memreq.size = AlignUp(memreq.size + 1, memreq.alignment * 4);

    RDCDEBUG(
        "Padded image memory requirements from %llu to %llu (base alignment %llu) (%f%% increase)",
        oldsize, memreq.size, memreq.alignment,
        (100.0 * double(memreq.size - oldsize)) / double(oldsize));
  }
}

void WrappedVulkan::vkGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2KHR *pInfo,
    uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2KHR *pSparseMemoryRequirements)
{
  VkImageSparseMemoryRequirementsInfo2KHR unwrappedInfo = *pInfo;
  unwrappedInfo.image = Unwrap(unwrappedInfo.image);
  ObjDisp(device)->GetImageSparseMemoryRequirements2KHR(
      Unwrap(device), &unwrappedInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void WrappedVulkan::vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                                VkDeviceSize *pCommittedMemoryInBytes)
{
  ObjDisp(device)->GetDeviceMemoryCommitment(Unwrap(device), Unwrap(memory), pCommittedMemoryInBytes);
}

void WrappedVulkan::vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass,
                                               VkExtent2D *pGranularity)
{
  return ObjDisp(device)->GetRenderAreaGranularity(Unwrap(device), Unwrap(renderPass), pGranularity);
}

VkResult WrappedVulkan::vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache,
                                               size_t *pDataSize, void *pData)
{
  size_t totalSize = 16 + VK_UUID_SIZE + 4;    // required header (16+UUID) and 4 0 bytes

  if(pDataSize && !pData)
    *pDataSize = totalSize;

  if(pDataSize && pData)
  {
    if(*pDataSize < totalSize)
    {
      memset(pData, 0, *pDataSize);
      return VK_INCOMPLETE;
    }

    uint32_t *ptr = (uint32_t *)pData;

    ptr[0] = (uint32_t)totalSize;
    ptr[1] = VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
    // just in case the user expects a valid vendorID/deviceID, write the real one
    // MULTIDEVICE need to get the right physical device for this device
    ptr[2] = m_PhysicalDeviceData.props.vendorID;
    ptr[3] = m_PhysicalDeviceData.props.deviceID;

    MakeFakeUUID();

    memcpy(ptr + 4, fakeRenderDocUUID, VK_UUID_SIZE);
    // [4], [5], [6], [7]

    RDCCOMPILE_ASSERT(VK_UUID_SIZE == 16, "VK_UUID_SIZE has changed");

    // empty bytes
    ptr[8] = 0;
  }

  // we don't want the application to use pipeline caches at all, and especially
  // don't want to return any data for future use. We thus return a technically
  // valid but empty pipeline cache. Our UUID changes every run so in theory the
  // application should never provide an old cache, but just in case we will nop
  // it out in create pipeline cache
  return VK_SUCCESS;
}

VkResult WrappedVulkan::vkMergePipelineCaches(VkDevice device, VkPipelineCache destCache,
                                              uint32_t srcCacheCount,
                                              const VkPipelineCache *pSrcCaches)
{
  // do nothing, our pipeline caches are always dummies
  return VK_SUCCESS;
}

VkResult WrappedVulkan::vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling,
    VkImageUsageFlags usage, VkImageCreateFlags flags,
    VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceExternalImageFormatPropertiesNV(Unwrap(physicalDevice), format, type,
                                                         tiling, usage, flags, externalHandleType,
                                                         pExternalImageFormatProperties);
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult WrappedVulkan::vkGetMemoryWin32HandleNV(VkDevice device, VkDeviceMemory memory,
                                                 VkExternalMemoryHandleTypeFlagsNV handleType,
                                                 HANDLE *pHandle)
{
  return ObjDisp(device)->GetMemoryWin32HandleNV(Unwrap(device), Unwrap(memory), handleType, pHandle);
}

VkResult WrappedVulkan::vkGetMemoryWin32HandleKHR(
    VkDevice device, const VkMemoryGetWin32HandleInfoKHR *pGetWin32HandleInfo, HANDLE *pHandle)
{
  VkMemoryGetWin32HandleInfoKHR unwrappedInfo = *pGetWin32HandleInfo;
  unwrappedInfo.memory = Unwrap(unwrappedInfo.memory);
  return ObjDisp(device)->GetMemoryWin32HandleKHR(Unwrap(device), &unwrappedInfo, pHandle);
}

VkResult WrappedVulkan::vkGetMemoryWin32HandlePropertiesKHR(
    VkDevice device, VkExternalMemoryHandleTypeFlagBitsKHR handleType, HANDLE handle,
    VkMemoryWin32HandlePropertiesKHR *pMemoryWin32HandleProperties)
{
  return ObjDisp(device)->GetMemoryWin32HandlePropertiesKHR(Unwrap(device), handleType, handle,
                                                            pMemoryWin32HandleProperties);
}
#endif

VkResult WrappedVulkan::vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo,
                                         int *pFd)
{
  VkMemoryGetFdInfoKHR unwrappedInfo = *pGetFdInfo;
  unwrappedInfo.memory = Unwrap(unwrappedInfo.memory);
  return ObjDisp(device)->GetMemoryFdKHR(Unwrap(device), &unwrappedInfo, pFd);
}

VkResult WrappedVulkan::vkGetMemoryFdPropertiesKHR(VkDevice device,
                                                   VkExternalMemoryHandleTypeFlagBitsKHR handleType,
                                                   int fd,
                                                   VkMemoryFdPropertiesKHR *pMemoryFdProperties)
{
  return ObjDisp(device)->GetMemoryFdPropertiesKHR(Unwrap(device), handleType, fd,
                                                   pMemoryFdProperties);
}

void WrappedVulkan::vkGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfoKHR *pExternalBufferInfo,
    VkExternalBufferPropertiesKHR *pExternalBufferProperties)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceExternalBufferPropertiesKHR(Unwrap(physicalDevice), pExternalBufferInfo,
                                                     pExternalBufferProperties);
}

void WrappedVulkan::vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfoKHR *pExternalSemaphoreInfo,
    VkExternalSemaphorePropertiesKHR *pExternalSemaphoreProperties)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceExternalSemaphorePropertiesKHR(
          Unwrap(physicalDevice), pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

void WrappedVulkan::vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceFeatures2KHR *pFeatures)
{
  return ObjDisp(physicalDevice)->GetPhysicalDeviceFeatures2KHR(Unwrap(physicalDevice), pFeatures);
}

void WrappedVulkan::vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                      VkPhysicalDeviceProperties2KHR *pProperties)
{
  return ObjDisp(physicalDevice)->GetPhysicalDeviceProperties2KHR(Unwrap(physicalDevice), pProperties);
}

void WrappedVulkan::vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                            VkFormat format,
                                                            VkFormatProperties2KHR *pFormatProperties)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceFormatProperties2KHR(Unwrap(physicalDevice), format, pFormatProperties);
}

VkResult WrappedVulkan::vkGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2KHR *pImageFormatInfo,
    VkImageFormatProperties2KHR *pImageFormatProperties)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceImageFormatProperties2KHR(Unwrap(physicalDevice), pImageFormatInfo,
                                                   pImageFormatProperties);
}

void WrappedVulkan::vkGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pCount,
    VkQueueFamilyProperties2KHR *pQueueFamilyProperties)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceQueueFamilyProperties2KHR(Unwrap(physicalDevice), pCount,
                                                   pQueueFamilyProperties);
}

void WrappedVulkan::vkGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2KHR *pMemoryProperties)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceMemoryProperties2KHR(Unwrap(physicalDevice), pMemoryProperties);
}

void WrappedVulkan::vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2KHR *pFormatInfo,
    uint32_t *pPropertyCount, VkSparseImageFormatProperties2KHR *pProperties)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceSparseImageFormatProperties2KHR(Unwrap(physicalDevice), pFormatInfo,
                                                         pPropertyCount, pProperties);
}

VkResult WrappedVulkan::vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline,
                                           VkShaderStageFlagBits shaderStage,
                                           VkShaderInfoTypeAMD infoType, size_t *pInfoSize,
                                           void *pInfo)
{
  return ObjDisp(device)->GetShaderInfoAMD(Unwrap(device), Unwrap(pipeline), shaderStage, infoType,
                                           pInfoSize, pInfo);
}
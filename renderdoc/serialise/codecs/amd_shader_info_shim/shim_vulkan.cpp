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

bool extAvailable = false;
bool quitNow = false;
bool ShimShouldQuitNow()
{
  return quitNow;
}

VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                             const char *handleName)
{
  // Enable VK_AMD_shader_info extension if supported.
  uint32_t extCount;
  VkResult r = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extCount, NULL);
  assert(r == VK_SUCCESS);

  std::vector<VkExtensionProperties> extensions(extCount);
  r = vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extCount, extensions.data());
  assert(r == VK_SUCCESS);
  for(const auto &v : extensions)
  {
    if(strcmp(v.extensionName, VK_AMD_SHADER_INFO_EXTENSION_NAME) == 0)
    {
      extAvailable = true;
      break;
    }
  }
  if(extAvailable)
  {
    std::vector<const char *> enabledExtensionNames(pCreateInfo->enabledExtensionCount + 1);
    for(uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++)
    {
      enabledExtensionNames[i] = pCreateInfo->ppEnabledExtensionNames[i];
    }
    enabledExtensionNames[pCreateInfo->enabledExtensionCount] = VK_AMD_SHADER_INFO_EXTENSION_NAME;
    VkDeviceCreateInfo newCI = {
        /* sType = */ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        /* pNext */ pCreateInfo->pNext,
        /* flags */ pCreateInfo->flags,
        /* queueCreateInfoCount = */ pCreateInfo->queueCreateInfoCount,
        /* pQueueCreateInfos = */ pCreateInfo->pQueueCreateInfos,
        /* enabledLayerCount = */ pCreateInfo->enabledLayerCount,
        /* ppEnabledLayerNames = */ pCreateInfo->ppEnabledLayerNames,
        /* enabledExtensionCount = */ static_cast<uint32_t>(enabledExtensionNames.size()),
        /* ppEnabledExtensionNames = */ enabledExtensionNames.data(),
        /* pEnabledFeatures = */ pCreateInfo->pEnabledFeatures,
    };
    r = vkCreateDevice(physicalDevice, &newCI, pAllocator, pDevice);
    assert(r == VK_SUCCESS);
    AddResourceName(ResourceNames, (uint64_t)*pDevice, "VkDevice", handleName);
    InitializeAuxResources(&aux, aux.instance, physicalDevice, *pDevice);
  }
  else
  {
    r = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
  }
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
    AddResourceName(ResourceNames, (uint64_t)*pPipelines, "VkPipeline", handleName);
  if(r != VK_SUCCESS || !extAvailable)
    return r;

  // Get shader info.
  static PFN_vkGetShaderInfoAMD pfnGetShaderInfoAMD =
      (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(device, "vkGetShaderInfoAMD");

  if(pfnGetShaderInfoAMD == NULL)
    return r;

  VkShaderStatisticsInfoAMD statistics = {};
  size_t dataSize;

  for(uint32_t i = 0; i < createInfoCount; i++)
  {
    VkShaderStageFlagBits shaderStage = pCreateInfos[i].stage.stage;
    VkPipeline p = pPipelines[i];

    dataSize = sizeof(statistics);
    // Print statistics data.
    r = pfnGetShaderInfoAMD(device, p, shaderStage, VK_SHADER_INFO_TYPE_STATISTICS_AMD, &dataSize,
                            &statistics);
    assert(r == VK_SUCCESS);
    printShaderInfo(p, shaderStage, statistics);

    // Print disassembly data.
    r = pfnGetShaderInfoAMD(device, p, shaderStage, VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD, &dataSize,
                            NULL);
    assert(r == VK_SUCCESS);
    std::vector<uint8_t> disassembly(dataSize);
    r = pfnGetShaderInfoAMD(device, p, shaderStage, VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD, &dataSize,
                            disassembly.data());
    assert(r == VK_SUCCESS);
    printShaderInfo(p, shaderStage, (char *)disassembly.data(), disassembly.size());
  }

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
    AddResourceName(ResourceNames, (uint64_t)*pPipelines, "VkPipeline", handleName);
  if(r != VK_SUCCESS || !extAvailable)
    return r;

  // Get shader info.
  static PFN_vkGetShaderInfoAMD pfnGetShaderInfoAMD =
      (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(device, "vkGetShaderInfoAMD");

  if(pfnGetShaderInfoAMD == NULL)
    return r;

  VkShaderStatisticsInfoAMD statistics = {};
  size_t dataSize;

  for(uint32_t i = 0; i < createInfoCount; i++)
  {
    std::vector<VkShaderStageFlagBits> shaderStages;
    for(uint32_t s = 0; s < pCreateInfos[i].stageCount; s++)
      shaderStages.push_back(pCreateInfos[i].pStages[s].stage);
    VkPipeline p = pPipelines[i];
    for(uint32_t s = 0; s < shaderStages.size(); s++)
    {
      dataSize = sizeof(statistics);
      // Print statistics data.
      r = pfnGetShaderInfoAMD(device, p, shaderStages[s], VK_SHADER_INFO_TYPE_STATISTICS_AMD,
                              &dataSize, &statistics);
      assert(r == VK_SUCCESS);
      printShaderInfo(p, shaderStages[s], statistics);

      // Print disassembly data.
      r = pfnGetShaderInfoAMD(device, p, shaderStages[s], VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD,
                              &dataSize, NULL);
      assert(r == VK_SUCCESS);
      std::vector<uint8_t> disassembly(dataSize);
      r = pfnGetShaderInfoAMD(device, p, shaderStages[s], VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD,
                              &dataSize, disassembly.data());
      assert(r == VK_SUCCESS);
      printShaderInfo(p, shaderStages[s], (char *)disassembly.data(), disassembly.size());
    }
  }

  return r;
}

VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
  static PFN_vkQueuePresentKHR fn =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
  VkResult r = fn(queue, pPresentInfo);
#if defined(__yeti__)
  // Create a trigger file to indicate that we have dumped all the shaders already
  FILE *fp = OpenFile("/var/game/shader.trigger", "wb");
  if(fp)
    fclose(fp);
#endif
  quitNow = true;
  return r;
}

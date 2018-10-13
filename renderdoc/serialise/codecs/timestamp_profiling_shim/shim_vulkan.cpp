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
#include <cinttypes>
#include <map>
#include <set>
#include <unordered_map>

#include <assert.h>

#include "helper/helper.h"
#include "shim_vulkan.h"
#include "utils.h"

#if defined(DEBUG) || defined(_DEBUG)
const int START_TS_FRAME = 10;
#else
const int START_TS_FRAME = 500;
#endif
const int END_TS_FRAME = START_TS_FRAME + 50;
const double DELTA = 0.001;

#define FIRST_TS_STAGE VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
#define SECOND_TS_STAGE VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT

ShimVkTraceResources aux;
int presentIndex = 0;
bool quitNow = false;

bool ShimShouldQuitNow()
{
  return quitNow;
}

std::unordered_map<std::string, bool> timestampedCalls = {
  {"shim_vkCmdBindPipeline", true},
  {"shim_vkCmdSetViewport", false},
  {"shim_vkCmdSetScissor", false},
  {"shim_vkCmdSetLineWidth", false},
  {"shim_vkCmdSetDepthBias", false},
  {"shim_vkCmdSetBlendConstants", false},
  {"shim_vkCmdSetDepthBounds", false},
  {"shim_vkCmdSetStencilCompareMask", false},
  {"shim_vkCmdSetStencilWriteMask", false},
  {"shim_vkCmdSetStencilReference", false},
  {"shim_vkCmdBindDescriptorSets", false},
  {"shim_vkCmdBindIndexBuffer", false},
  {"shim_vkCmdBindVertexBuffers", false},
  {"shim_vkCmdDraw", true},
  {"shim_vkCmdDrawIndexed", true},
  {"shim_vkCmdDrawIndirect", true},
  {"shim_vkCmdDrawIndexedIndirect", true},
  {"shim_vkCmdDispatch", true},
  {"shim_vkCmdDispatchIndirect", true},
  {"shim_vkCmdCopyBuffer", true},
  {"shim_vkCmdCopyImage", true},
  {"shim_vkCmdBlitImage", true},
  {"shim_vkCmdCopyBufferToImage", true},
  {"shim_vkCmdCopyImageToBuffer", true},
  {"shim_vkCmdUpdateBuffer", true},
  {"shim_vkCmdFillBuffer", true},
  {"shim_vkCmdClearColorImage", true},
  {"shim_vkCmdClearDepthStencilImage", true},
  {"shim_vkCmdClearAttachments", true},
  {"shim_vkCmdResolveImage", true},
  {"shim_vkCmdSetEvent", false},
  {"shim_vkCmdResetEvent", false},
  {"shim_vkCmdWaitEvents", false},
  {"shim_vkCmdPipelineBarrier", true},
  {"shim_vkCmdBeginQuery", false},
  {"shim_vkCmdEndQuery", false},
  {"shim_vkCmdResetQueryPool", false},
  {"shim_vkCmdWriteTimestamp", false},
  {"shim_vkCmdCopyQueryPoolResults", false},
  {"shim_vkCmdPushConstants", true},
  {"shim_vkCmdBeginRenderPass", true},
  {"shim_vkCmdNextSubpass", true},
  {"shim_vkCmdEndRenderPass", true},
  {"shim_vkCmdExecuteCommands", true},
  {"shim_vkCmdDebugMarkerBeginEXT", false},
  {"shim_vkCmdDebugMarkerEndEXT", false},
  {"shim_vkCmdDebugMarkerInsertEXT", false},
  {"shim_vkCmdDrawIndirectCountAMD", false},
  {"shim_vkCmdDrawIndexedIndirectCountAMD", false},
  {"shim_vkCmdProcessCommandsNVX", false},
  {"shim_vkCmdReserveSpaceForCommandsNVX", false},
  {"shim_vkCmdPushDescriptorSetKHR", false},
  {"shim_vkCmdSetDeviceMask", false},
  {"shim_vkCmdSetDeviceMaskKHR", false},
  {"shim_vkCmdDispatchBase", false},
  {"shim_vkCmdDispatchBaseKHR", false},
  {"shim_vkCmdPushDescriptorSetWithTemplateKHR", false},
  {"shim_vkCmdSetViewportWScalingNV", false},
  {"shim_vkCmdSetDiscardRectangleEXT", false},
  {"shim_vkCmdSetSampleLocationsEXT", false},
  {"shim_vkCmdBeginDebugUtilsLabelEXT", false},
  {"shim_vkCmdEndDebugUtilsLabelEXT", false},
  {"shim_vkCmdInsertDebugUtilsLabelEXT", false},
  {"shim_vkCmdWriteBufferMarkerAMD", false},
};

VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
  VkDeviceCreateInfo * pCI = const_cast<VkDeviceCreateInfo *>(pCreateInfo);
  VkPhysicalDeviceFeatures *pEF = const_cast<VkPhysicalDeviceFeatures *>(pCI->pEnabledFeatures);
  // Make sure these features are enabled.
  pEF->pipelineStatisticsQuery = 1;
  pEF->geometryShader = 1;
  pEF->tessellationShader = 1;
  VkResult r = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
  InitializeAuxResources(&aux, aux.instance, physicalDevice, *pDevice);
  return r;
}

void shim_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                              VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
  static PFN_vkGetPhysicalDeviceMemoryProperties fn = vkGetPhysicalDeviceMemoryProperties;
  fn(physicalDevice, pMemoryProperties);
  aux.physDeviceMemoryProperties = *pMemoryProperties;
  return;
}

void shim_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                        VkPhysicalDeviceProperties *pProperties)
{
  static PFN_vkGetPhysicalDeviceProperties fn = vkGetPhysicalDeviceProperties;
  fn(physicalDevice, pProperties);
  aux.physDeviceProperties = *pProperties;
  return;
}

VkResult shim_vkBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                   const VkCommandBufferBeginInfo *pBeginInfo)
{
  // We assume there is no use of VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
  // flag in command buffers
  // in all traces.
  assert((pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) == 0);
  static PFN_vkBeginCommandBuffer fn = vkBeginCommandBuffer;
  VkResult r = fn(commandBuffer, pBeginInfo);
  if(presentIndex == 0)
  {
    // Currently all command buffers are uniquely recorded in a frame.
    // Is this always true? Can't use VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    // for figuring this out.
    assert(aux.cbCommandInfo.count(commandBuffer) == 0);
    aux.addCommandInfo(
        commandBuffer,
        CommandInfo(__FUNCTION__,
                    GetResourceName(ResourceNames, VkHandle((uint64_t)commandBuffer, "VkCommandBuffer")), 1));
    return r;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME)
  {
    if(!aux.isSecondary(commandBuffer))
    {
      aux.resetTimestampQueries(commandBuffer);
      uint32_t offset = aux.timestampQueryOffset(commandBuffer);
      uint32_t count = aux.timestampQueryCount(commandBuffer);
      vkCmdResetQueryPool(commandBuffer, aux.timestampQueryPool(commandBuffer), offset, count);
    }
    // Before submitting the secondary command buffers, the primary
    // command buffer needs to reset their all query pools.
    for(uint32_t cb = 0; cb < aux.cbExecCmdBufs[commandBuffer].size(); cb++)
    {
      VkCommandBuffer execCB = aux.cbExecCmdBufs[commandBuffer][cb].cb;
      aux.resetTimestampQueries(execCB);
      uint32_t offset = aux.timestampQueryOffset(execCB);
      uint32_t count = aux.timestampQueryCount(execCB);
      vkCmdResetQueryPool(commandBuffer, aux.timestampQueryPool(execCB), offset, count);
    }

    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  } else if (presentIndex == END_TS_FRAME) { // last frame: do pipelinestats now
    if (!aux.isSecondary(commandBuffer)) {
      aux.resetPipelinestatsQueries(commandBuffer);
      uint32_t offset = aux.pipelinestatsQueryOffset(commandBuffer);
      uint32_t count = aux.pipelinestatsQueryCount(commandBuffer);
      VkQueryPool pool = aux.pipelinestatsQueryPool(commandBuffer);
      if (pool != VK_NULL_HANDLE)
        vkCmdResetQueryPool(commandBuffer, pool, offset, count);
    }

    // Before submitting the secondary command buffers, the primary
    // command buffer needs to reset their all query pools.
    for (uint32_t cb = 0; cb < aux.cbExecCmdBufs[commandBuffer].size(); cb++) {
      VkCommandBuffer execCB = aux.cbExecCmdBufs[commandBuffer][cb].cb;
      aux.resetPipelinestatsQueries(execCB);
      uint32_t offset = aux.pipelinestatsQueryOffset(execCB);
      uint32_t count = aux.pipelinestatsQueryCount(execCB);
      VkQueryPool pool = aux.pipelinestatsQueryPool(execCB);
      if (pool != VK_NULL_HANDLE)
        vkCmdResetQueryPool(commandBuffer, pool, offset, count);
    }

  }
  return r;
}

VkResult shim_vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
  static PFN_vkEndCommandBuffer fn = vkEndCommandBuffer;
  if(presentIndex == 0)
  {
    aux.addCommandInfo(
        commandBuffer,
        CommandInfo(__FUNCTION__,
                    GetResourceName(ResourceNames, VkHandle((uint64_t)commandBuffer, "VkCommandBuffer")), 1));
    VkResult r = fn(commandBuffer);
    return r;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME)
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  VkResult r = fn(commandBuffer);
  return r;
}

VkResult shim_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                            VkFence fence)
{
  static PFN_vkQueueSubmit fn = vkQueueSubmit;
  VkResult r = fn(queue, submitCount, pSubmits, fence);
  if(presentIndex == 0)
  {
    for(uint32_t i = 0; i < submitCount; i++)
    {
      aux.queueSubmit(queue, pSubmits[i].commandBufferCount,
                      const_cast<VkCommandBuffer *>(pSubmits[i].pCommandBuffers));
    }
    return r;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME)
  {
    vkQueueWaitIdle(queue);
    for(uint32_t i = 0; i < submitCount; i++)
    {
      for(uint32_t cbi = 0; cbi < pSubmits[i].commandBufferCount; cbi++)
      {
        VkCommandBuffer currentCB = pSubmits[i].pCommandBuffers[cbi];
        if(!aux.isPresent(currentCB))
        {
          assert(0);
          continue;    // this should really never happen.
        }

        aux.accumulateAllTimestamps(currentCB, presentIndex);
      }
    }
  }
  return r;
}

VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
  static PFN_vkQueuePresentKHR fn =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
  VkResult r = fn(queue, pPresentInfo);

  // In the first frame, create querypools for all CommandBuffers.
  if(presentIndex == 0)
  {
    aux.createQueryPools();
  }

  if(presentIndex == END_TS_FRAME) {

    aux.fetchPipelineStats(END_TS_FRAME);

    for(auto it : aux.cbAccumTimestamps) {
      float ts = float((it.second[0][0] + it.second[0][1]) /
        (1000000.0 * (END_TS_FRAME - START_TS_FRAME)));
      std::string cbTime = "Command Buffer Time = " +
                           std::to_string(ts) +
                           " (ms)\n";
#if defined(_WIN32)
      OutputDebugStringA(cbTime.c_str());
#else
      fprintf(stdout, "%s", cbTime.c_str());
#endif
    }
#if defined(__yeti__)
    const char csvFileName[] = "/var/game/timestamps.csv";
#else
    const char csvFileName[] = "timestamps.csv";
#endif
    aux.writeAllCSV(csvFileName);
  }

  quitNow = presentIndex++ > END_TS_FRAME;
  return r;
}

void shim_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                               const VkCommandBuffer *pCommandBuffers)
{
  static PFN_vkCmdExecuteCommands fn = vkCmdExecuteCommands;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, commandBufferCount, pCommandBuffers);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, "", 0, false));
    for(uint32_t cb = 0; cb < commandBufferCount; cb++)
    {
      aux.executeCommands(commandBuffer, pCommandBuffers[cb], commandBufferCount - cb);
    }
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    assert(!aux.isSecondary(commandBuffer));
    uint32_t offset = aux.timestampQueryOffset(commandBuffer);

    // Here is what the spec says:
    // VK_SUBPASS_CONTENTS_INLINE specifies that the contents of the subpass
    // will be recorded inline in the primary command buffer, and secondary
    // command buffers must not be executed within the subpass.
    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS specifies that the
    // contents are recorded in secondary command buffers that will be called
    // from the primary command buffer, and vkCmdExecuteCommands is the only
    // valid command on the command buffer until vkCmdNextSubpass or
    // vkCmdEndRenderPass.
    fn(commandBuffer, commandBufferCount, pCommandBuffers);
  }
  else
  {
    fn(commandBuffer, commandBufferCount, pCommandBuffers);
  }
}

void shim_vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                            VkPipeline pipeline)
{
  PFN_vkCmdBindPipeline fn = vkCmdBindPipeline;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, pipelineBindPoint, pipeline);
    std::string infoStr = std::string(GetResourceName(ResourceNames, VkHandle((uint64_t)pipeline, "VkPipeline")));
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, pipelineBindPoint, pipeline);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, pipelineBindPoint, pipeline);
  }
  return;
}

void shim_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                           uint32_t viewportCount, const VkViewport *pViewports)
{
  PFN_vkCmdSetViewport fn = vkCmdSetViewport;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, firstViewport, viewportCount, pViewports);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, firstViewport, viewportCount, pViewports);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, firstViewport, viewportCount, pViewports);
  }
  return;
}

void shim_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                          uint32_t scissorCount, const VkRect2D *pScissors)
{
  PFN_vkCmdSetScissor fn = vkCmdSetScissor;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, firstScissor, scissorCount, pScissors);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, firstScissor, scissorCount, pScissors);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, firstScissor, scissorCount, pScissors);
  }
  return;
}

void shim_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
  PFN_vkCmdSetLineWidth fn = vkCmdSetLineWidth;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, lineWidth);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, lineWidth);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, lineWidth);
  }
  return;
}

void shim_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                            float depthBiasClamp, float depthBiasSlopeFactor)
{
  PFN_vkCmdSetDepthBias fn = vkCmdSetDepthBias;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
  }
  return;
}

void shim_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
  PFN_vkCmdSetBlendConstants fn = vkCmdSetBlendConstants;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, blendConstants);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, blendConstants);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, blendConstants);
  }
  return;
}

void shim_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                              float maxDepthBounds)
{
  PFN_vkCmdSetDepthBounds fn = vkCmdSetDepthBounds;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, minDepthBounds, maxDepthBounds);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, minDepthBounds, maxDepthBounds);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, minDepthBounds, maxDepthBounds);
  }
  return;
}

void shim_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                     uint32_t compareMask)
{
  PFN_vkCmdSetStencilCompareMask fn = vkCmdSetStencilCompareMask;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, faceMask, compareMask);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, faceMask, compareMask);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, faceMask, compareMask);
  }
  return;
}

void shim_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                   uint32_t writeMask)
{
  PFN_vkCmdSetStencilWriteMask fn = vkCmdSetStencilWriteMask;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, faceMask, writeMask);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, faceMask, writeMask);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, faceMask, writeMask);
  }
  return;
}

void shim_vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                   uint32_t reference)
{
  PFN_vkCmdSetStencilReference fn = vkCmdSetStencilReference;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, faceMask, reference);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, faceMask, reference);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, faceMask, reference);
  }
  return;
}

void shim_vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                  VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                  uint32_t firstSet, uint32_t descriptorSetCount,
                                  const VkDescriptorSet *pDescriptorSets,
                                  uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets)
{
  PFN_vkCmdBindDescriptorSets fn = vkCmdBindDescriptorSets;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
       dynamicOffsetCount, pDynamicOffsets);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
       dynamicOffsetCount, pDynamicOffsets);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
       dynamicOffsetCount, pDynamicOffsets);
  }
  return;
}

void shim_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                               VkIndexType indexType)
{
  PFN_vkCmdBindIndexBuffer fn = vkCmdBindIndexBuffer;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, buffer, offset, indexType);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, indexType);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, buffer, offset, indexType);
  }
  return;
}

void shim_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                 uint32_t bindingCount, const VkBuffer *pBuffers,
                                 const VkDeviceSize *pOffsets)
{
  PFN_vkCmdBindVertexBuffers fn = vkCmdBindVertexBuffers;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
  }
  return;
}

void shim_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                    uint32_t firstVertex, uint32_t firstInstance)
{
  PFN_vkCmdDraw fn = vkCmdDraw;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    char infoStr[256];
    snprintf(infoStr, sizeof(infoStr),
             "Vertex Count: %d "
             "Instance Count: %d "
             "First Vertex: %d "
             "First Instance: %d ",
             vertexCount, instanceCount, firstVertex, firstInstance);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else if (presentIndex == END_TS_FRAME)
  {
    vkCmdBeginQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryOffset(commandBuffer), 0);
    fn(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    vkCmdEndQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryInc(commandBuffer));
  } else {
    fn(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
  }
  return;
}

void shim_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                           uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
  PFN_vkCmdDrawIndexed fn = vkCmdDrawIndexed;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    char infoStr[256];
    snprintf(infoStr, sizeof(infoStr),
             "Index Count: %d "
             "Instance Count: %d "
             "First Index: %d "
             "Vertex Offset: %d "
             "First Instance: %d ",
             indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  } else if (presentIndex == END_TS_FRAME) {
    vkCmdBeginQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryOffset(commandBuffer), 0);
    fn(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    vkCmdEndQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
  }
  return;
}

void shim_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                            uint32_t drawCount, uint32_t stride)
{
  PFN_vkCmdDrawIndirect fn = vkCmdDrawIndirect;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, buffer, offset, drawCount, stride);
    char infoStr[256];
    snprintf(infoStr, sizeof(infoStr),
             "Draw Count: %d "
             "Stride Count: %d ",
             drawCount, stride);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, drawCount, stride);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  } else if (presentIndex == END_TS_FRAME) {
    vkCmdBeginQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryOffset(commandBuffer), 0);
    fn(commandBuffer, buffer, offset, drawCount, stride);
    vkCmdEndQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, buffer, offset, drawCount, stride);
  }
  return;
}

void shim_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                   VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
  PFN_vkCmdDrawIndexedIndirect fn = vkCmdDrawIndexedIndirect;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, buffer, offset, drawCount, stride);
    char infoStr[256];
    snprintf(infoStr, sizeof(infoStr), "Offset: %" PRIu64
                                       " "
                                       "Draw Count: %u "
                                       "Stride Count: %u ",
             offset, drawCount, stride);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, drawCount, stride);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  } else if (presentIndex == END_TS_FRAME) {
    vkCmdBeginQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryOffset(commandBuffer), 0);
    fn(commandBuffer, buffer, offset, drawCount, stride);
    vkCmdEndQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, buffer, offset, drawCount, stride);
  }
  return;
}

void shim_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                        uint32_t groupCountZ)
{
  PFN_vkCmdDispatch fn = vkCmdDispatch;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, groupCountX, groupCountY, groupCountZ);
    char infoStr[256];
    snprintf(infoStr, sizeof(infoStr),
             "Groups X: %d "
             "Groups Y: %d "
             "Groups Z: %d ",
             groupCountX, groupCountY, groupCountZ);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, groupCountX, groupCountY, groupCountZ);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  } else if (presentIndex == END_TS_FRAME) {
    vkCmdBeginQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryOffset(commandBuffer), 0);
    fn(commandBuffer, groupCountX, groupCountY, groupCountZ);
    vkCmdEndQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, groupCountX, groupCountY, groupCountZ);
  }
  return;
}

void shim_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
  PFN_vkCmdDispatchIndirect fn = vkCmdDispatchIndirect;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, buffer, offset);
    char infoStr[256];
    snprintf(infoStr, sizeof(infoStr), "Offset: %" PRIu64 " ", offset);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, buffer, offset);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  } else if (presentIndex == END_TS_FRAME) {
    vkCmdBeginQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryOffset(commandBuffer), 0);
    fn(commandBuffer, buffer, offset);
    vkCmdEndQuery(commandBuffer, aux.pipelinestatsQueryPool(commandBuffer), 
      aux.pipelinestatsQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, buffer, offset);
  }
  return;
}

void shim_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                          uint32_t regionCount, const VkBufferCopy *pRegions)
{
  PFN_vkCmdCopyBuffer fn = vkCmdCopyBuffer;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
  }
  return;
}

void shim_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                         VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                         uint32_t regionCount, const VkImageCopy *pRegions)
{
  PFN_vkCmdCopyImage fn = vkCmdCopyImage;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
  }
  return;
}

void shim_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                         VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                         uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter)
{
  PFN_vkCmdBlitImage fn = vkCmdBlitImage;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
       filter);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
       filter);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
       filter);
  }
  return;
}

void shim_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer,
                                 VkImage dstImage, VkImageLayout dstImageLayout,
                                 uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
  PFN_vkCmdCopyBufferToImage fn = vkCmdCopyBufferToImage;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
  }
  return;
}

void shim_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                 VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                 uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
  PFN_vkCmdCopyImageToBuffer fn = vkCmdCopyImageToBuffer;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
  }
  return;
}

void shim_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                            VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData)
{
  PFN_vkCmdUpdateBuffer fn = vkCmdUpdateBuffer;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
  }
  return;
}

void shim_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                          VkDeviceSize size, uint32_t data)
{
  PFN_vkCmdFillBuffer fn = vkCmdFillBuffer;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, dstBuffer, dstOffset, size, data);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, dstBuffer, dstOffset, size, data);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, dstBuffer, dstOffset, size, data);
  }
  return;
}

void shim_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                               VkImageLayout imageLayout, const VkClearColorValue *pColor,
                               uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
  PFN_vkCmdClearColorImage fn = vkCmdClearColorImage;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
  }
  return;
}

void shim_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                      VkImageLayout imageLayout,
                                      const VkClearDepthStencilValue *pDepthStencil,
                                      uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
  PFN_vkCmdClearDepthStencilImage fn = vkCmdClearDepthStencilImage;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
  }
  return;
}

void shim_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                const VkClearAttachment *pAttachments, uint32_t rectCount,
                                const VkClearRect *pRects)
{
  PFN_vkCmdClearAttachments fn = vkCmdClearAttachments;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
  }
  return;
}

void shim_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                            VkImageLayout srcImageLayout, VkImage dstImage,
                            VkImageLayout dstImageLayout, uint32_t regionCount,
                            const VkImageResolve *pRegions)
{
  PFN_vkCmdResolveImage fn = vkCmdResolveImage;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
  }
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
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
       pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
       pImageMemoryBarriers);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
       pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
       pImageMemoryBarriers);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
       pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
       pImageMemoryBarriers);
  }
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
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
       pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
       pImageMemoryBarriers);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
       pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
       pImageMemoryBarriers);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
       pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
       pImageMemoryBarriers);
  }
  return;
}

void shim_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                             VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                             const void *pValues)
{
  PFN_vkCmdPushConstants fn = vkCmdPushConstants;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, layout, stageFlags, offset, size, pValues);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, layout, stageFlags, offset, size, pValues);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, layout, stageFlags, offset, size, pValues);
  }
  return;
}

void shim_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                               const VkRenderPassBeginInfo *pRenderPassBegin,
                               VkSubpassContents contents)
{
  bool isInline = contents == VK_SUBPASS_CONTENTS_INLINE;
  PFN_vkCmdBeginRenderPass fn = vkCmdBeginRenderPass;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, pRenderPassBegin, contents);
    std::string infoStr =
        GetResourceName(ResourceNames, VkHandle((uint64_t)pRenderPassBegin->renderPass, "VkRenderPass"));
    infoStr +=
        isInline ? ": Subpass with INLINE contents" : ": Subpass with SECONDARY command buffers";
    uint32_t ts = isInline ? 2 : 1;
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr, ts, isInline));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    assert(!aux.isSecondary(commandBuffer));

    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, pRenderPassBegin, contents);
    if(isInline)
      vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                          aux.timestampQueryInc(commandBuffer));

    aux.isInline(commandBuffer, isInline);
  }
  else
  {
    fn(commandBuffer, pRenderPassBegin, contents);
  }
  return;
}

void shim_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
  bool isInline = contents == VK_SUBPASS_CONTENTS_INLINE;
  bool wasInline = aux.isInline(commandBuffer);
  PFN_vkCmdNextSubpass fn = vkCmdNextSubpass;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, contents);
    std::string infoStr =
        isInline ? "Subpass with INLINE contents" : "Subpass with SECONDARY command buffers";

    uint32_t ts = 0;    // value for !wasInline && !isInline;
    if(wasInline && isInline)
      ts = 2;
    if((!wasInline && isInline) || (wasInline && !isInline))
      ts = 1;
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr, ts, isInline));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    if(wasInline)
      vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                          aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, contents);
    if(isInline)
      vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                          aux.timestampQueryInc(commandBuffer));

    aux.isInline(commandBuffer, isInline);
  }
  else
  {
    fn(commandBuffer, contents);
  }
  return;
}

void shim_vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
  bool wasInline = aux.isInline(commandBuffer);
  PFN_vkCmdEndRenderPass fn = vkCmdEndRenderPass;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer);
    std::string infoStr =
        wasInline ? "Subpass with INLINE contents" : "Subpass with SECONDARY command buffers";

    uint32_t ts = 1;    // value for !wasInline;
    if(wasInline)
      ts = 2;
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, infoStr, ts, true));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    assert(!aux.isSecondary(commandBuffer));
    if(wasInline)
      vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                          aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));

    aux.isInline(commandBuffer, true);
  }
  else
  {
    fn(commandBuffer);
  }
  return;
}

void shim_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                    VkDeviceSize offset, VkBuffer countBuffer,
                                    VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                    uint32_t stride)
{
  static PFN_vkCmdDrawIndirectCountAMD fn =
      (PFN_vkCmdDrawIndirectCountAMD)vkGetDeviceProcAddr(aux.device, "vkCmdDrawIndirectCountAMD");
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, ""));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
  }
  return;
}

void shim_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                           VkDeviceSize offset, VkBuffer countBuffer,
                                           VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride)
{
  static PFN_vkCmdDrawIndexedIndirectCountAMD fn =
      (PFN_vkCmdDrawIndexedIndirectCountAMD)vkGetDeviceProcAddr(aux.device,
                                                                "vkCmdDrawIndexedIndirectCountAMD");
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, ""));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
  }
  return;
}

void shim_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                    VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                    uint32_t set, uint32_t descriptorWriteCount,
                                    const VkWriteDescriptorSet *pDescriptorWrites)
{
  static PFN_vkCmdPushDescriptorSetKHR fn =
      (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(aux.device, "vkCmdPushDescriptorSetKHR");
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
  }
  return;
}

void shim_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
  static PFN_vkCmdSetDeviceMask fn =
      (PFN_vkCmdSetDeviceMask)vkGetDeviceProcAddr(aux.device, "vkCmdSetDeviceMask");
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, deviceMask);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, deviceMask);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, deviceMask);
  }
  return;
}

void shim_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
  static PFN_vkCmdSetDeviceMaskKHR fn =
      (PFN_vkCmdSetDeviceMaskKHR)vkGetDeviceProcAddr(aux.device, "vkCmdSetDeviceMaskKHR");
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, deviceMask);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, deviceMask);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, deviceMask);
  }
  return;
}

void shim_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                            uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY,
                            uint32_t groupCountZ)
{
  static PFN_vkCmdDispatchBase fn =
      (PFN_vkCmdDispatchBase)vkGetDeviceProcAddr(aux.device, "vkCmdDispatchBase");
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, ""));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
  }
  return;
}

void shim_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX,
                               uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX,
                               uint32_t groupCountY, uint32_t groupCountZ)
{
  static PFN_vkCmdDispatchBaseKHR fn =
      (PFN_vkCmdDispatchBaseKHR)vkGetDeviceProcAddr(aux.device, "vkCmdDispatchBaseKHR");
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__, ""));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
  }
  return;
}

void shim_vkCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                VkPipelineLayout layout, uint32_t set,
                                                const void *pData)
{
  static PFN_vkCmdPushDescriptorSetWithTemplateKHR fn =
      (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(
          aux.device, "vkCmdPushDescriptorSetWithTemplateKHR");
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
  }
  return;
}

void shim_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer,
                                    VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer,
                                    VkDeviceSize dstOffset, uint32_t marker)
{
  static PFN_vkCmdWriteBufferMarkerAMD fn =
      (PFN_vkCmdWriteBufferMarkerAMD)vkGetDeviceProcAddr(aux.device, "vkCmdWriteBufferMarkerAMD");
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    aux.addCommandInfo(commandBuffer, CommandInfo(__FUNCTION__));
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME &&
          timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
    fn(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.timestampQueryPool(commandBuffer),
                        aux.timestampQueryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
  }
  return;
}

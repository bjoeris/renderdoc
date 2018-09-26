#ifndef SHIM_VK_COMPILE_STATIC_LIB
#define SHIM_VK_EXPORT
#endif
#include <cinttypes>
#include <map>
#include <unordered_map>
#include <set>

#include <assert.h>

#include "helper/helper.h"
#include "shim_vulkan.h"
#include "utils.h"

const int START_TS_FRAME = 10000;
const int END_TS_FRAME = START_TS_FRAME + 1500;
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
  {"shim_vkCmdSetViewport", true},
  {"shim_vkCmdSetScissor", true},
  {"shim_vkCmdSetLineWidth", true},
  {"shim_vkCmdSetDepthBias", true},
  {"shim_vkCmdSetBlendConstants", true},
  {"shim_vkCmdSetDepthBounds", true},
  {"shim_vkCmdSetStencilCompareMask", true},
  {"shim_vkCmdSetStencilWriteMask", true},
  {"shim_vkCmdSetStencilReference", true},
  {"shim_vkCmdBindDescriptorSets", true},
  {"shim_vkCmdBindIndexBuffer", true},
  {"shim_vkCmdBindVertexBuffers", true},
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

/************************* shimmed functions *******************************/
VkResult shim_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
  VkResult r = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
  InitializeAuxResources(&aux, aux.instance, physicalDevice, *pDevice);
  return r;
}

void shim_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                              VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
  PFN_vkGetPhysicalDeviceMemoryProperties fn = vkGetPhysicalDeviceMemoryProperties;
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

// We assume there is no use of VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT flag in command buffers
// in all traces.
VkResult shim_vkBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                   const VkCommandBufferBeginInfo *pBeginInfo)
{
  assert((pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) == 0);
  static PFN_vkBeginCommandBuffer fn = vkBeginCommandBuffer;
  VkResult r = fn(commandBuffer, pBeginInfo);
  if(presentIndex == 0)
  {
    // Currently all command buffers are uniquely recorded in a frame. Is this
    // always true?
    assert(aux.cbCommandInfo.find(commandBuffer) == aux.cbCommandInfo.end());
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return r;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME)
  {
    uint32_t offset = aux.queryOffset(commandBuffer);
    uint32_t count = aux.queryCount(commandBuffer);
    aux.resetQueries(commandBuffer);
    vkCmdResetQueryPool(commandBuffer, aux.queryPool(commandBuffer), offset, count);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
  }
  return r;
}

VkResult shim_vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
  static PFN_vkEndCommandBuffer fn = vkEndCommandBuffer;
  if(presentIndex == 0)
  {
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    VkResult r = fn(commandBuffer);
    return r;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME)
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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

/*
* In the first frame, create querypool and get timestampValidBits from VkQueueFamilyProperties.
*/
VkResult shim_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
  static PFN_vkQueuePresentKHR fn =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(aux.device, "vkQueuePresentKHR");
  VkResult r = fn(queue, pPresentInfo);
  if(presentIndex == 0)
  {
    aux.createQueryPools();
  }

  if(presentIndex == END_TS_FRAME)
  {
    for(auto it : aux.cbAccumTimestamps)
    {
      std::string cbTime = "Command Buffer Time = " +
                           std::to_string((it.second[0][0] + it.second[0][1]) /
                                          (1000000.0 * (END_TS_FRAME - START_TS_FRAME))) +
                           " (ms)\n";
#if defined(_WIN32) || defined(WIN32)
      OutputDebugStringA(cbTime.c_str());
#else
      fprintf(stdout, "%s", cbTime.c_str());
#endif
    }
    aux.writeCSV("timestamps.csv");
    quitNow = true;
  }

  presentIndex++;
  return r;
}

// vkCmdExecuteCommand moves query index vector of secondary command buffers to primary command
// buffer's.
void shim_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                               const VkCommandBuffer *pCommandBuffers)
{
  static PFN_vkCmdExecuteCommands fn = vkCmdExecuteCommands;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, commandBufferCount, pCommandBuffers);
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    uint32_t offset = aux.queryOffset(commandBuffer);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, commandBufferCount, pCommandBuffers);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    for (uint32_t cb = 0; cb < commandBufferCount; cb++) {
      aux.addExecCmdBufRelation(commandBuffer, pCommandBuffers[cb], offset);
    }
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, pipelineBindPoint, pipeline);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, firstViewport, viewportCount, pViewports);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, firstScissor, scissorCount, pScissors);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, lineWidth);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, blendConstants);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, minDepthBounds, maxDepthBounds);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, faceMask, compareMask);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, faceMask, writeMask);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, faceMask, reference);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets,
       dynamicOffsetCount, pDynamicOffsets);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, indexType);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    sprintf(infoStr,
            "Vertex Count: %d "
            "Instance Count: %d "
            "First Vertex: %d "
            "First Instance: %d ",
            vertexCount, instanceCount, firstVertex, firstInstance);
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__, infoStr});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
  }
  else
  {
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
    sprintf(infoStr,
            "Index Count: %d "
            "Instance Count: %d "
            "First Index: %d "
            "Vertex Offset: %d "
            "First Instance: %d ",
            indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__, infoStr});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    sprintf(infoStr,
            "Draw Count: %d "
            "Stride Count: %d ",
            drawCount, stride);
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__, infoStr});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, drawCount, stride);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    sprintf(infoStr,
            "Offset: %" PRIu64
            " "
            "Draw Count: %u "
            "Stride Count: %u ",
            offset, drawCount, stride);
aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__, infoStr});
return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, drawCount, stride);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    sprintf(infoStr,
            "Groups X: %d "
            "Groups Y: %d "
            "Groups Z: %d ",
            groupCountX, groupCountY, groupCountZ);
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__, infoStr});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, groupCountX, groupCountY, groupCountZ);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    sprintf(infoStr, "Offset: %" PRIu64 " ", offset);
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__, infoStr});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, buffer, offset);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
       filter);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, dstBuffer, dstOffset, size, data);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
       pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
       pImageMemoryBarriers);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
       pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
       pImageMemoryBarriers);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, layout, stageFlags, offset, size, pValues);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
  PFN_vkCmdBeginRenderPass fn = vkCmdBeginRenderPass;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, pRenderPassBegin, contents);
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, pRenderPassBegin, contents);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, pRenderPassBegin, contents);
  }
  return;
}

void shim_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
  PFN_vkCmdNextSubpass fn = vkCmdNextSubpass;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer, contents);
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, contents);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, contents);
  }
  return;
}

void shim_vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
  PFN_vkCmdEndRenderPass fn = vkCmdEndRenderPass;
  if(presentIndex == 0 && timestampedCalls[__FUNCTION__])
  {
    fn(commandBuffer);
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, deviceMask);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, deviceMask);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, FIRST_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
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
    aux.cbCommandInfo[commandBuffer].push_back({__FUNCTION__});
    return;
  }
  else if(presentIndex > START_TS_FRAME && presentIndex < END_TS_FRAME && timestampedCalls[__FUNCTION__])
  {
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
    fn(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    vkCmdWriteTimestamp(commandBuffer, SECOND_TS_STAGE, aux.queryPool(commandBuffer),
                        aux.queryInc(commandBuffer));
  }
  else
  {
    fn(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
  }
  return;
}

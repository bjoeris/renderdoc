/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2017-2018 Baldur Karlsson
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "common/common.h"
#include "core/core.h"
#include "driver/vulkan/vk_common.h"
#include "driver/vulkan/vk_resources.h"
#include "serialise/rdcfile.h"
#include "ext_object.h"
#include "vk_cpp_codec_tracker.h"
#include "vk_cpp_codec_writer.h"

namespace vk_cpp_codec
{
#ifndef CODE_ACQUIRE_NEXT_IMAGE
#define CODE_ACQUIRE_NEXT_IMAGE(ext, pass) \
  if(pass == CodeWriter::ID_CREATE)        \
  {                                        \
    pass = CodeWriter::ID_PRERESET;        \
    code.AcquireNextImage(ext, pass);      \
  }
#endif

static ReplayStatus Structured2Code(CodeWriter &code, TraceTracker &tracker, const RDCFile &file,
                                    uint64_t version, const StructuredChunkList &chunks,
                                    RENDERDOC_ProgressCallback progress)
{
  code.Resolution(CodeWriter::ID_VAR);
  uint32_t pass = CodeWriter::ID_CREATE;

  for(size_t c = 0; c < chunks.size(); c++)
  {
    code.MultiPartSplit();
    tracker.CopiesClear();

    SDChunk *chunk = chunks[c];
    if(chunk->metadata.chunkID >= (uint32_t)VulkanChunk::vkEnumeratePhysicalDevices)
    {
      chunk->name = code.shimPrefix + string(chunk->name);
    }
    ExtObject *ext = as_ext(chunk);
    switch(chunk->metadata.chunkID)
    {
      case(uint32_t)SystemChunk::DriverInit: code.CreateInstance(ext, pass); break;
      case(uint32_t)SystemChunk::InitialContents:
        CODE_ACQUIRE_NEXT_IMAGE(ext, pass);
        code.InitialContent(ext);
        break;
      case(uint32_t)SystemChunk::InitialContentsList: CODE_ACQUIRE_NEXT_IMAGE(ext, pass); break;
      case(uint32_t)SystemChunk::CaptureScope: break;
      case(uint32_t)SystemChunk::CaptureBegin:
        CODE_ACQUIRE_NEXT_IMAGE(ext, pass);
        code.InitialLayouts(ext, pass);
        pass = CodeWriter::ID_RENDER;
        break;
      case(uint32_t)SystemChunk::CaptureEnd:
        code.EndFramePresent(ext, pass);
        code.EndFrameWaitIdle(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkEnumeratePhysicalDevices:
        code.EnumeratePhysicalDevices(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateDevice:
        code.CreateDevice(ext, pass);
        // akharlamov: besides creating the device, resource creation,
        // memory allocation and resource binding happens on CreateDevice.
        // The reason behind this organization is that resource memory
        // type requirement can be different on replay system and memory
        // allocation needs to find an intersection of memory types of all
        // the resources that would be bound to that allocation.
        // In the code gen, this is achieved by:
        // 1. Creating the device
        // 2. For each memory allocation
        //   a. Go over the list of resources that are bound to that allocation
        //   b. Create those resources and get their memory requirements
        //   c. Bitmask and the memoryTypeBits
        //   d. The resulting bitmask of memoryTypeBits is used for memory allocation
        //   (and thus intersection of all memoryTypeBits needs to be != 0)
        //   If intersection is '0', the trace can't be replayed on this system.
        //   e. Additionally if the memory allocation doesn't host aliased resources
        //   then the size and binding offset of each resource is recalculated
        //   and stored in a 'Remap' vector.
        code.HandleMemoryAllocationAndResourceCreation(pass);
        break;
      case(uint32_t)VulkanChunk::vkGetDeviceQueue: code.GetDeviceQueue(ext, pass); break;
      case(uint32_t)VulkanChunk::vkAllocateMemory:
        // akharlamov: memory allocation happens right after device was created.
        // code.AllocateMemory(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkUnmapMemory: code.UnmapMemory(ext, pass); break;
      case(uint32_t)VulkanChunk::vkFlushMappedMemoryRanges:
        code.FlushMappedMemoryRegions(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateCommandPool: code.CreateCommandPool(ext, pass); break;
      case(uint32_t)VulkanChunk::vkAllocateCommandBuffers:
        code.AllocateCommandBuffers(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateFramebuffer:
        if(tracker.CreateFramebuffer(ext))
          code.CreatePresentFramebuffer(ext, pass);
        else
          code.CreateFramebuffer(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateRenderPass: code.CreateRenderPass(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCreateDescriptorPool:
        code.CreateDescriptorPool(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateDescriptorSetLayout:
        code.CreateDescriptorSetLayout(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateDescriptorUpdateTemplate:
        code.CreateDescriptorUpdateTemplate(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateBuffer:
        // akharlamov: buffer creation happens right after device was created.
        // code.CreateBuffer(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateBufferView: code.CreateBufferView(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCreateImage:
        // akharlamov: image creation happens right after device was created.
        // code.CreateImage(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateImageView:
        if(tracker.CreateImageView(ext))
          code.CreatePresentImageView(ext, pass);
        else
          code.CreateImageView(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateSampler: code.CreateSampler(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCreateShaderModule: code.CreateShaderModule(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCreatePipelineLayout:
        code.CreatePipelineLayout(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreatePipelineCache: code.CreatePipelineCache(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCreateGraphicsPipelines:
        code.CreateGraphicsPipelines(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateComputePipelines:
        code.CreateGraphicsPipelines(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkGetSwapchainImagesKHR:
        code.GetSwapChainImagesKHR(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCreateSemaphore: code.CreateSemaphoreOrFence(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCreateFence: code.CreateSemaphoreOrFence(ext, pass); break;
      case(uint32_t)VulkanChunk::vkRegisterDeviceEventEXT: RDCASSERT(0); break;
      case(uint32_t)VulkanChunk::vkRegisterDisplayEventEXT: RDCASSERT(0); break;
      case(uint32_t)VulkanChunk::vkGetFenceStatus: code.GetFenceStatus(ext, pass); break;
      case(uint32_t)VulkanChunk::vkResetFences: code.ResetFences(ext, pass); break;
      case(uint32_t)VulkanChunk::vkWaitForFences: code.WaitForFences(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCreateEvent: code.GenericVkCreate(ext, pass); break;
      case(uint32_t)VulkanChunk::vkGetEventStatus: RDCASSERT(0); break;
      case(uint32_t)VulkanChunk::vkSetEvent: code.Event(ext, pass); break;
      case(uint32_t)VulkanChunk::vkResetEvent: code.Event(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCreateQueryPool: code.GenericVkCreate(ext, pass); break;
      case(uint32_t)VulkanChunk::vkAllocateDescriptorSets:
        code.AllocateDescriptorSets(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkUpdateDescriptorSets:
        code.UpdateDescriptorSets(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkUpdateDescriptorSetWithTemplate:
        code.UpdateDescriptorSetWithTemplate(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkBeginCommandBuffer:
        tracker.BeginCommandBuffer(ext);
        code.BeginCommandBuffer(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkEndCommandBuffer: code.EndCommandBuffer(ext, pass); break;
      case(uint32_t)VulkanChunk::vkQueueWaitIdle:
      case(uint32_t)VulkanChunk::vkDeviceWaitIdle: code.QueueOrDeviceWaitIdle(ext, pass); break;
      case(uint32_t)VulkanChunk::vkQueueSubmit:
        tracker.QueueSubmit(ext);
        code.QueueSubmit(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkBindBufferMemory:
        // akharlamov: buffer binding to memory happens right after device was created.
        // code.BindImageOrBufferMemory(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkBindImageMemory:
        // akharlamov: image binding to memory happens right after device was created.
        // code.BindImageOrBufferMemory(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkQueueBindSparse: RDCASSERT(0); break;
      case(uint32_t)VulkanChunk::vkCmdBeginRenderPass:
        tracker.CmdBeginRenderPass(ext);
        code.CmdBeginRenderPass(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdNextSubpass: code.CmdNextSubpass(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdExecuteCommands: code.CmdExecuteCommands(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdEndRenderPass: code.CmdEndRenderPass(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdBindPipeline: code.CmdBindPipeline(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdSetViewport: code.CmdSetViewportOrScissor(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdSetScissor: code.CmdSetViewportOrScissor(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdSetLineWidth: code.CmdSetLineWidth(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdSetDepthBias: code.CmdSetDepthBias(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdSetBlendConstants:
        code.CmdSetBlendConstants(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdSetDepthBounds: code.CmdSetDepthBounds(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdSetStencilCompareMask:
        code.CmdSetStencilParam(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdSetStencilWriteMask:
        code.CmdSetStencilParam(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdSetStencilReference:
        code.CmdSetStencilParam(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdBindDescriptorSets:
        code.CmdBindDescriptorSets(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdBindIndexBuffer: code.CmdBindIndexBuffer(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdBindVertexBuffers:
        code.CmdBindVertexBuffers(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdCopyBufferToImage:
        code.CmdCopyBufferToImage(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdCopyImageToBuffer:
        code.CmdCopyImageToBuffer(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdCopyImage: code.CmdCopyImage(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdBlitImage: code.CmdBlitImage(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdResolveImage: code.CmdResolveImage(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdCopyBuffer: code.CmdCopyBuffer(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdUpdateBuffer: code.CmdUpdateBuffer(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdFillBuffer: code.CmdFillBuffer(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdPushConstants: code.CmdPushConstants(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdClearColorImage: code.CmdClearColorImage(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdClearDepthStencilImage:
        code.CmdClearDepthStencilImage(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdClearAttachments: code.CmdClearAttachments(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdPipelineBarrier:
        if(tracker.CmdPipelineBarrier(ext))
          code.CmdPipelineBarrier(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdWriteTimestamp:
        // RDCASSERT(0);
        break;
      case(uint32_t)VulkanChunk::vkCmdCopyQueryPoolResults:
        // RDCASSERT(0);
        break;
      case(uint32_t)VulkanChunk::vkCmdBeginQuery:
        // RDCASSERT(0);
        break;
      case(uint32_t)VulkanChunk::vkCmdEndQuery:
        // RDCASSERT(0);
        break;
      case(uint32_t)VulkanChunk::vkCmdResetQueryPool:
        // RDCASSERT(0);
        break;
      case(uint32_t)VulkanChunk::vkCmdSetEvent: code.CmdEvent(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdResetEvent: code.CmdEvent(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdWaitEvents:
        if(tracker.CmdWaitEvents(ext))
          code.CmdWaitEvents(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdDraw: code.CmdDraw(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdDrawIndirect:
        code.CmdDrawIndirectOrIndexedIndirect(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdDrawIndexed: code.CmdDrawIndexed(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdDrawIndexedIndirect:
        code.CmdDrawIndirectOrIndexedIndirect(ext, pass);
        break;
      case(uint32_t)VulkanChunk::vkCmdDispatch: code.CmdDispatch(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdDispatchIndirect: code.CmdDispatchIndirect(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdDebugMarkerBeginEXT:
        // RDCASSERT(0);
        break;
      case(uint32_t)VulkanChunk::vkCmdDebugMarkerInsertEXT:
        // RDCASSERT(0);
        break;
      case(uint32_t)VulkanChunk::vkCmdDebugMarkerEndEXT:
        // RDCASSERT(0);
        break;
      case(uint32_t)VulkanChunk::vkDebugMarkerSetObjectNameEXT:
        // RDCASSERT(0);
        break;
      case(uint32_t)VulkanChunk::SetShaderDebugPath: RDCASSERT(0); break;
      case(uint32_t)VulkanChunk::vkCreateSwapchainKHR: code.CreateSwapChain(ext, pass); break;
      case(uint32_t)VulkanChunk::vkCmdIndirectSubCommand: RDCASSERT(0); break;
      default:
        RDCWARN("%s Vulkan call not implemented", ext->Name());
        RDCASSERT(0);
        break;
    }
  }
  return ReplayStatus::Succeeded;
}

bool OptimizationDisabled(const char *name)
{
#ifdef _WIN32
  size_t len = 0;
  char var[8];
  errno_t err = getenv_s(&len, var, sizeof(var), name);
  if (err)
    return false;
#else
  const char* var = getenv(name);
  if (!var)
    return false;
#endif
  return strcmp(var, "false") == 0;
}

CodeGenOpts GetEnvOpts()
{
  CodeGenOpts optimizations = CODE_GEN_OPT_ALL_OPTS;
  if(OptimizationDisabled("RDOC_CODE_GEN_ALL_OPTS"))
  {
    optimizations = 0;
  }
  if(OptimizationDisabled("RDOC_CODE_GEN_OPT_BUFFER_INIT"))
  {
    optimizations &= ~CODE_GEN_OPT_BUFFER_INIT_BIT;
  }
  if(OptimizationDisabled("RDOC_CODE_GEN_OPT_BUFFER_RESET"))
  {
    optimizations &= ~CODE_GEN_OPT_BUFFER_RESET_BIT;
  }
  if(OptimizationDisabled("RDOC_CODE_GEN_OPT_IMAGE_INIT"))
  {
    optimizations &= ~CODE_GEN_OPT_IMAGE_INIT_BIT;
  }
  if(OptimizationDisabled("RDOC_CODE_GEN_OPT_IMAGE_RESET"))
  {
    optimizations &= ~CODE_GEN_OPT_IMAGE_RESET_BIT;
  }
  if(OptimizationDisabled("RDOC_CODE_GEN_OPT_IMAGE_MEMORY"))
  {
    optimizations &= ~CODE_GEN_OPT_IMAGE_MEMORY_BIT;
    optimizations &= ~CODE_GEN_OPT_IMAGE_INIT_BIT;
    optimizations &= ~CODE_GEN_OPT_IMAGE_RESET_BIT;
  }
  return optimizations;
}

}    // namespace vk_cpp_codec

ReplayStatus exportCPPZ(const char *filename, const RDCFile &rdc, const SDFile &structData,
                        RENDERDOC_ProgressCallback progress)
{
  std::string s_filename(filename);
  std::size_t found = s_filename.find_last_of(".");
  RDCASSERT(found != string::npos);

  vk_cpp_codec::CodeWriter code(s_filename.substr(0, found));
  vk_cpp_codec::TraceTracker tracker(s_filename.substr(0, found));
  tracker.SetOptimizations(vk_cpp_codec::GetEnvOpts());

  code.Set(&tracker);

  StructuredChunkList chunks;
  StructuredBufferList buffers;
  for(uint32_t i = 0; i < structData.chunks.size(); i++)
  {
    chunks.push_back(structData.chunks[i]);
  }
  for(uint32_t i = 0; i < structData.buffers.size(); i++)
  {
    buffers.push_back(structData.buffers[i]);
  }

  tracker.Scan(chunks, buffers);

  ReplayStatus status =
      vk_cpp_codec::Structured2Code(code, tracker, rdc, structData.version, chunks, progress);

  code.Close();

  return status;
}

static ConversionRegistration CPPConversionRegistration(
    &exportCPPZ, {
                     "cpp", "CPP capture project",
                     R"(Stores the structured data in an cpp project, with large buffer data
 stored in indexed blobs in binary files. It cannot be reimported.)",
                     false,
                 });

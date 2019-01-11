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
#include "vk_cpp_codec_tracker.h"
#include "vk_cpp_codec_writer.h"
#include "core/intervals.h"

#ifdef TT_VK_CALL_INTERNAL_SWITCH
#undef TT_VK_CALL_INTERNAL_SWITCH
#endif
#define TT_VK_CALL_INTERNAL_SWITCH(call, arg) \
  case(uint32_t)VulkanChunk::vk##call:        \
    call##Internal(arg);                      \
    break;


namespace vk_cpp_codec {

// The 'Scan' pass bellow analyzes, filters and patches a deserialized renderdoc trace.

void TraceTracker::Scan(StructuredChunkList &chunks, StructuredBufferList &buffers) {
  ScanChunks(chunks);
  ScanResourceCreation(chunks, buffers);
  ScanFilter(chunks);
  ScanInitialContents(chunks);
  ScanQueueSubmits(chunks);
  ScanBinaryData(buffers);
  AnalyzeMemoryAllocations();
  AnalyzeInitResources();
  AnalyzeMemoryResetRequirements();
}

void TraceTracker::ScanChunks(StructuredChunkList &chunks) {
  for (size_t c = 0; c < chunks.size();) {
    SDChunk *ext = chunks[c];
    bool removed = false;
    switch (ext->metadata.chunkID) {
      case(uint32_t) VulkanChunk::vkCreateImage:
      if (!CreateImageFilter(ext)) {
        chunks.removeOne(chunks[c]);
        removed = true;
      } break;

      case (uint32_t) VulkanChunk::vkCreateBufferView: // fallthrough intended
      case(uint32_t) VulkanChunk::vkCreateImageView:
      if (!CreateResourceViewFilter(ext)) {
        chunks.removeOne(chunks[c]);
        removed = true;
      } break;

      default: break;
    }

    if (!removed)
      c++;
  }
}

void TraceTracker::ScanResourceCreation(StructuredChunkList &chunks, StructuredBufferList &buffers) {
  for (size_t c = 0; c < chunks.size(); c++) {
    SDChunk *ext = chunks[c];
    switch (ext->metadata.chunkID) {
      case(uint32_t) VulkanChunk::vkCreateBuffer:
      case(uint32_t) VulkanChunk::vkCreateImage: CreateResourceInternal(ext); break;
      case(uint32_t) VulkanChunk::vkCreateBufferView:
      case(uint32_t) VulkanChunk::vkCreateImageView:
      CreateResourceViewInternal(ext);
      break;
      TT_VK_CALL_INTERNAL_SWITCH(CreateDevice, ext);
      TT_VK_CALL_INTERNAL_SWITCH(GetDeviceQueue, ext);
      TT_VK_CALL_INTERNAL_SWITCH(AllocateMemory, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateFramebuffer, ext);
      TT_VK_CALL_INTERNAL_SWITCH(BindBufferMemory, ext);
      TT_VK_CALL_INTERNAL_SWITCH(BindImageMemory, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateSampler, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateShaderModule, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateSwapchainKHR, ext);
      TT_VK_CALL_INTERNAL_SWITCH(GetSwapchainImagesKHR, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreatePipelineCache, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateRenderPass, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateDescriptorSetLayout, ext);
      TT_VK_CALL_INTERNAL_SWITCH(AllocateDescriptorSets, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateDescriptorPool, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateDescriptorUpdateTemplate, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateCommandPool, ext);
      TT_VK_CALL_INTERNAL_SWITCH(AllocateCommandBuffers, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreatePipelineLayout, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateGraphicsPipelines, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateComputePipelines, ext);
      TT_VK_CALL_INTERNAL_SWITCH(EnumeratePhysicalDevices, ext);
      TT_VK_CALL_INTERNAL_SWITCH(CreateEvent, ext);
      default: break;
    }
  }
}
void TraceTracker::ScanFilter(StructuredChunkList &chunks) {
  for (size_t c = 0; c < chunks.size();) {
    SDChunk *ext = chunks[c];
    bool removed = false;
    switch (ext->metadata.chunkID) {
      case(uint32_t) SystemChunk::InitialContents:
      {
        if (ext->GetChild(0)->AsUInt64() == VkResourceType::eResDescriptorSet && !InitDescSetFilter(ext)) {
          chunks.removeOne(chunks[c]);
          removed = true;
        }
      }
      break;
      case(uint32_t) VulkanChunk::vkUpdateDescriptorSets:
      {
        if (!UpdateDescriptorSetsFilter(ext)) {
          chunks.removeOne(chunks[c]);
          removed = true;
        }
      }
      break;
      case(uint32_t) VulkanChunk::vkUpdateDescriptorSetWithTemplate:
      if (!UpdateDescriptorSetWithTemplateFilter(ext)) {
        chunks.removeOne(chunks[c]);
        removed = true;
      }
      break;

      case(uint32_t) VulkanChunk::vkCreateGraphicsPipelines:
      CreateGraphicsPipelinesFilter(ext);
      break;
      case(uint32_t) VulkanChunk::vkCreateComputePipelines:
      CreateComputePipelinesFilter(ext);
      break;
      case(uint32_t) VulkanChunk::vkCmdCopyImageToBuffer:
      CmdCopyImageToBufferFilter(ext);
      break;
      case(uint32_t) VulkanChunk::vkCmdCopyImage: CmdCopyImageFilter(ext); break;
      case(uint32_t) VulkanChunk::vkCmdBlitImage: CmdBlitImageFilter(ext); break;
      case(uint32_t) VulkanChunk::vkCmdResolveImage: CmdResolveImageFilter(ext); break;
      case(uint32_t) VulkanChunk::vkCreateDevice: CreateDeviceFilter(ext); break;
      case(uint32_t) VulkanChunk::vkCmdPipelineBarrier:
      if (!CmdPipelineBarrierFilter(ext)) {
        chunks.removeOne(chunks[c]);
        removed = true;
      }
      break;
      case(uint32_t) VulkanChunk::vkCmdSetEvent:      // fallthrough intended
      case(uint32_t) VulkanChunk::vkSetEvent:         // fallthrough intended
      case(uint32_t) VulkanChunk::vkCmdResetEvent:    // fallthrough intended
      case(uint32_t) VulkanChunk::vkResetEvent:       // fallthrough intended
      case(uint32_t) VulkanChunk::vkGetEventStatus:
      if (EventFuncFilter(ext)) {
        chunks.removeOne(chunks[c]);
        removed = true;
      }
      break;
      default: break;
    }

    if (!removed)
      c++;
  }
}

void TraceTracker::ScanInitialContents(StructuredChunkList &chunks) {
  for (size_t c = 0; c < chunks.size(); c++) {
    switch (chunks[c]->metadata.chunkID) {
      case(uint32_t) SystemChunk::CaptureBegin:
      InitialLayoutsInternal(chunks[c]);
      break;
      case(uint32_t) SystemChunk::InitialContents: InitialContentsInternal(chunks[c]); break;
      default: break;
    }
  }
}

void TraceTracker::ScanQueueSubmits(StructuredChunkList &chunks) {
  for (size_t c = 0; c < chunks.size(); c++) {
    switch (chunks[c]->metadata.chunkID) {
      TT_VK_CALL_INTERNAL_SWITCH(FlushMappedMemoryRanges, chunks[c]);
      TT_VK_CALL_INTERNAL_SWITCH(UpdateDescriptorSets, chunks[c]);
      TT_VK_CALL_INTERNAL_SWITCH(UpdateDescriptorSetWithTemplate, chunks[c]);
      TT_VK_CALL_INTERNAL_SWITCH(BeginCommandBuffer, chunks[c]);
      TT_VK_CALL_INTERNAL_SWITCH(QueueSubmit, chunks[c]);
      case(uint32_t) VulkanChunk::vkCmdBeginRenderPass:
      case(uint32_t) VulkanChunk::vkCmdNextSubpass:
      case(uint32_t) VulkanChunk::vkCmdExecuteCommands:
      case(uint32_t) VulkanChunk::vkCmdEndRenderPass:
      case(uint32_t) VulkanChunk::vkCmdCopyImage:
      case(uint32_t) VulkanChunk::vkCmdBlitImage:
      case(uint32_t) VulkanChunk::vkCmdResolveImage:
      case(uint32_t) VulkanChunk::vkCmdClearColorImage:
      case(uint32_t) VulkanChunk::vkCmdClearDepthStencilImage:
      case(uint32_t) VulkanChunk::vkCmdClearAttachments:
      case(uint32_t) VulkanChunk::vkCmdCopyBufferToImage:
      case(uint32_t) VulkanChunk::vkCmdCopyImageToBuffer:
      case(uint32_t) VulkanChunk::vkCmdPipelineBarrier:
      case(uint32_t) VulkanChunk::vkCmdWaitEvents:
      case(uint32_t) VulkanChunk::vkCmdBindDescriptorSets:
      case(uint32_t) VulkanChunk::vkCmdBindIndexBuffer:
      case(uint32_t) VulkanChunk::vkCmdBindVertexBuffers:
      case(uint32_t) VulkanChunk::vkCmdCopyBuffer:
      case(uint32_t) VulkanChunk::vkCmdUpdateBuffer:
      case(uint32_t) VulkanChunk::vkCmdFillBuffer:
      case(uint32_t) VulkanChunk::vkCmdDispatch:
      case(uint32_t) VulkanChunk::vkCmdDispatchIndirect:
      case(uint32_t) VulkanChunk::vkCmdDrawIndirect:
      case(uint32_t) VulkanChunk::vkCmdDrawIndexedIndirect:
      case(uint32_t) VulkanChunk::vkCmdDraw:
      case(uint32_t) VulkanChunk::vkCmdDrawIndexed:
      case(uint32_t) VulkanChunk::vkCmdBindPipeline: AddCommandBufferToFrameGraph(chunks[c]); break;
      default: break;
    }
  }
}

void TraceTracker::ScanBinaryData(StructuredBufferList &buffers) {
  for (size_t i = 0; i < buffers.size(); i++) {
    if (buffers[i]->size() == 0)
      continue;

    const char *name = GetVarFromMap(dataBlobs, "std::vector<uint8_t>", "buffer", i);
    std::string full_name = file_dir + "/" + "sample_cpp_trace" + "/" + name;
    FileIO::CreateParentDirectory(full_name);
    FILE *fbin = FileIO::fopen(full_name.c_str(), "wb");
    RDCASSERT(fbin != NULL);
    FileIO::fwrite(buffers[i]->data(), 1, buffers[i]->size(), fbin);
    FileIO::fclose(fbin);
  }
}

void TraceTracker::AnalyzeMemoryAllocations() {
  // ma_it: memory allocation iterator.
  for (MemAllocWithResourcesMapIter ma_it = MemAllocBegin(); ma_it != MemAllocEnd(); ma_it++) {
    // For each bound resource check if it's memory range overlaps with
    // any previously bound resources to determine if resource aliasing
    // takes place.
    // br_it: bound resource iterator.
    for (BoundResourcesIter br_it = ma_it->second.FirstBoundResource();
      br_it != ma_it->second.EndOfBoundResources(); br_it++) {
      MemRange range;
      range.MakeRange(br_it->offset, br_it->requirement);
      if (ma_it->second.CheckAliasedResources(range))
        break;
    }
  }
}

void TraceTracker::AnalyzeInitResources() {
  uint64_t memory_updates = 0;
  uint64_t descset_updates = 0;
  for (QueueSubmitsIter qs = fg.submits.begin(); qs != fg.submits.end(); qs++) {
    cmdQueue = qs->q->AsUInt64();

    SDObjectIDMapIter queue_it = deviceQueues.find(cmdQueue);
    RDCASSERT(queue_it != deviceQueues.end());
    cmdQueueFamily = queue_it->second->FindChild("queueFamilyIndex")->AsUInt64();

    for (; memory_updates < qs->memory_updates; memory_updates++) {
      ApplyMemoryUpdate(fg.updates.memory[memory_updates]);
    }
    for (; descset_updates < qs->descset_updates; descset_updates++) {
      ApplyDescSetUpdate(fg.updates.descset[descset_updates]);
    }
    SDObject *submitInfos = qs->sdobject->GetChild(2);
    for (uint64_t i = 0; i < submitInfos->NumChildren(); i++) {
      SDObject *submitInfo = submitInfos->GetChild(i);
      SDObject *commandBuffers = submitInfo->GetChild(6);
      for (uint64_t j = 0; j < commandBuffers->NumChildren(); j++) {
        uint32_t recordIndex = fg.FindCmdBufferIndex(commandBuffers->GetChild(j));
        CmdBufferRecord &record = fg.records[recordIndex];
        for (uint64_t k = 0; k < record.cmds.size(); k++) {
          AnalyzeCmd(record.cmds[k]);
        }
        // Reset the binding state at the end of the command buffer
        bindingState = BindingState();
      }
    }
  }
}

void TraceTracker::AnalyzeMemoryResetRequirements() {
  for (MemAllocWithResourcesMapIter ma_it = MemAllocBegin(); ma_it != MemAllocEnd(); ma_it++) {
    MemoryAllocationWithBoundResources &mem = ma_it->second;

    for (BoundResourcesIter br_it = mem.FirstBoundResource(); br_it != mem.EndOfBoundResources();
      br_it++) {
      BoundResource &abr = *br_it;
      abr.reset = RESET_REQUIREMENT_NO_RESET;

      switch (abr.bindSDObj->metadata.chunkID) {
        case(uint32_t) VulkanChunk::vkBindImageMemory:
        {
          SDObject *image_ci = abr.createSDObj->GetChild(1);
          VkImageLayout initialLayout = (VkImageLayout) image_ci->GetChild(14)->AsUInt64();
          if (initialLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            abr.reset = RESET_REQUIREMENT_INIT;
          }
          if ((optimizations & CODE_GEN_OPT_IMAGE_MEMORY_BIT) == 0) {
            abr.reset = RESET_REQUIREMENT_RESET;
          }
          break;
        }
        case(uint32_t) VulkanChunk::vkBindBufferMemory:
        {
          MemRange range;
          range.MakeRange(abr.offset, abr.requirement);
          for (IntervalsIter<MemoryState> it = mem.memoryState.find(range.start);
            it != mem.memoryState.end() && it.start() < range.end; it++) {
            switch (it.value().accessState) {
              case ACCESS_STATE_READ:
              abr.reset = std::min(RESET_REQUIREMENT_INIT, abr.reset);
              break;
              case ACCESS_STATE_RESET:
              abr.reset = std::min(RESET_REQUIREMENT_RESET, abr.reset);
              break;
              default: break;
            }
          }
          if ((optimizations & CODE_GEN_OPT_BUFFER_INIT_BIT) == 0) {
            abr.reset = std::min(RESET_REQUIREMENT_INIT, abr.reset);
          }
          if ((optimizations & CODE_GEN_OPT_BUFFER_RESET_BIT) == 0) {
            abr.reset = std::min(RESET_REQUIREMENT_RESET, abr.reset);
          }
          break;
        }
        default: RDCASSERT(0);
      }
    }
  }
}

}    // namespace vk_cpp_codec
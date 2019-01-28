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

namespace vk_cpp_codec
{
/**************************************************************************
* Cmd*Analyze() methods.
* These methods are called by AnalyzeCmds() for each vkCmd* call in the
* capture. These track things like reads/writes of memory and images.
***************************************************************************/

void TraceTracker::CmdBeginRenderPassAnalyze(SDChunk *o)
{
  SDObject *bi = o->FindChild("RenderPassBegin");
  uint64_t renderPass = bi->FindChild("renderPass")->AsUInt64();
  uint64_t framebuffer = bi->FindChild("framebuffer")->AsUInt64();
  SDObject *renderPassCI = ResourceCreateFind(renderPass)->second.sdobj->GetChild(1);
  SDObject *framebufferCI = ResourceCreateFind(framebuffer)->second.sdobj->GetChild(1);

  // Add the cmdBuffer ID and current SDObject to beginRenderPassCmdBuffer map
  // to allow fetching it later on vkEndCmdRenderpass.
  uint64_t cmdBuffer = o->FindChild("commandBuffer")->AsUInt64();
  RDCASSERT(beginRenderPassCmdBuffer.find(cmdBuffer) == beginRenderPassCmdBuffer.end());
  beginRenderPassCmdBuffer[cmdBuffer] = o;

  // Renderpass attachment list should be the same as framebuffers list
  RDCASSERT(renderPassCI->FindChild("pAttachments")->NumChildren() ==
            framebufferCI->FindChild("pAttachments")->NumChildren());

  bindingState.BeginRenderPass(renderPassCI, framebufferCI, bi->GetChild(4));
  BeginSubpass();
}

void TraceTracker::CmdNextSubpassAnalyze(SDChunk *o)
{
  EndSubpass();
  bindingState.subpassIndex++;
  BeginSubpass();
}

void TraceTracker::CmdEndRenderPassAnalyze(SDChunk *end)
{
  EndSubpass();
  uint64_t commandBuffer = end->FindChild("commandBuffer")->AsUInt64();
  RDCASSERT(beginRenderPassCmdBuffer.find(commandBuffer) != beginRenderPassCmdBuffer.end());
  SDObject *cmdBeginRenderPass = beginRenderPassCmdBuffer[commandBuffer];
  SDObject *renderPassBegin = cmdBeginRenderPass->FindChild("RenderPassBegin");
  uint64_t renderPassID = renderPassBegin->FindChild("renderPass")->AsUInt64();
  uint64_t framebufferID = renderPassBegin->FindChild("framebuffer")->AsUInt64();
  ResourceWithViewsMapIter rp_it = ResourceCreateFind(renderPassID);
  ResourceWithViewsMapIter fb_it = ResourceCreateFind(framebufferID);
  RDCASSERT(rp_it != ResourceCreateEnd() && fb_it != ResourceCreateEnd());
  SDObject *renderPassCI = rp_it->second.sdobj->FindChild("CreateInfo");
  SDObject *framebufferCI = fb_it->second.sdobj->FindChild("CreateInfo");
  SDObject *renderPassAttachments = renderPassCI->FindChild("pAttachments");
  SDObject *framebufferAttachments = framebufferCI->FindChild("pAttachments");
  RDCASSERT(renderPassAttachments->NumChildren() == framebufferAttachments->NumChildren());
  for(uint32_t a = 0; a < framebufferAttachments->NumChildren(); a++)
  {
    uint64_t viewID = framebufferAttachments->GetChild(a)->AsUInt64();
    SDObject *attachmentDesc = renderPassAttachments->GetChild(a);
    VkImageLayout finalLayout = (VkImageLayout)attachmentDesc->FindChild("finalLayout")->AsUInt64();

    TransitionImageViewLayout(viewID, bindingState.attachmentLayout[a], finalLayout,
                              VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
  }

  beginRenderPassCmdBuffer.erase(commandBuffer);
  RDCASSERT(bindingState.subpassIndex == bindingState.renderPass->GetChild(6)->NumChildren() - 1);
}

void TraceTracker::CmdExecuteCommandsAnalyze(SDChunk *o)
{
  SDObject *commandBuffers = o->GetChild(2);
  for(uint64_t j = 0; j < commandBuffers->NumChildren(); j++)
  {
    uint32_t recordIndex = fg.FindCmdBufferIndex(commandBuffers->GetChild(j));
    CmdBufferRecord &record = fg.records[recordIndex];
    for(uint64_t k = 0; k < record.cmds.size(); k++)
    {
      AnalyzeCmd(record.cmds[k]);
    }
  }
}

void TraceTracker::CmdBindPipelineAnalyze(SDChunk *o)
{
  uint64_t pipelineBindPoint = o->GetChild(1)->AsUInt64();
  uint64_t pipeline = o->GetChild(2)->AsUInt64();
  RDCASSERT(createdPipelines.find(pipeline) != createdPipelines.end());
  switch(pipelineBindPoint)
  {
    case VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE:
      bindingState.computePipeline.pipeline = pipeline;
      break;
    case VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS:
      bindingState.graphicsPipeline.pipeline = pipeline;
      break;
    default: RDCASSERT(0);
  }
}

void TraceTracker::CmdBindDescriptorSetsAnalyze(SDChunk *o)
{
  // TODO(akharlamov) image can be bound to pipeline
  uint64_t pipelineBindPoint = o->GetChild(1)->AsUInt64();
  uint64_t firstSet = o->GetChild(3)->AsUInt64();
  uint64_t descriptorSetCount = o->GetChild(4)->AsUInt64();
  SDObject *descriptorSets = o->GetChild(5);
  uint64_t dynamicOffsetCount = o->GetChild(6)->AsUInt64();
  SDObject *dynamicOffsets = o->GetChild(7);

  RDCASSERT(descriptorSetCount == descriptorSets->NumChildren());
  RDCASSERT(dynamicOffsetCount == dynamicOffsets->NumChildren());

  BoundPipeline *boundPipeline = NULL;
  switch(pipelineBindPoint)
  {
    case VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE:
      boundPipeline = &bindingState.computePipeline;
      break;
    case VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS:
      boundPipeline = &bindingState.graphicsPipeline;
      break;
    default: RDCASSERT(0); break;
  }

  uint64_t dynamicOffsetIndex = 0;
  for(uint64_t i = 0; i < descriptorSetCount; i++)
  {
    uint64_t descSet_id = descriptorSets->GetChild(i)->AsUInt64();
    uint64_t descSet_num = i + firstSet;
    boundPipeline->descriptorSets[descSet_num] = descSet_id;

    DescriptorSetInfoMapIter descSet_it = descriptorSetInfos.find(descSet_id);
    RDCASSERT(descSet_it != descriptorSetInfos.end());

    for(DescriptorBindingMapIter binding_it = descSet_it->second.bindings.begin();
        binding_it != descSet_it->second.bindings.end(); binding_it++)
    {
      switch(binding_it->second.type)
      {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
          for(uint64_t j = 0; j < binding_it->second.bufferBindings.size(); j++)
          {
            RDCASSERT(dynamicOffsetIndex < dynamicOffsets->NumChildren());
            binding_it->second.bufferBindings[j].dynamicOffset =
                dynamicOffsets->GetChild(dynamicOffsetIndex)->AsUInt64();
            dynamicOffsetIndex++;
          }
        default: break;
      }
    }
  }
}

void TraceTracker::CmdBindIndexBufferAnalyze(SDChunk *o)
{
  uint64_t buf_id = o->GetChild(1)->AsUInt64();
  uint64_t offset = o->GetChild(2)->AsUInt64();
  uint64_t indexType = o->GetChild(3)->AsUInt64();
  ResourceWithViewsMapIter bufCreate_it = ResourceCreateFind(buf_id);
  RDCASSERT(bufCreate_it != ResourceCreateEnd());
  SDObject *ci = bufCreate_it->second.sdobj->GetChild(1);
  uint64_t bufSize = ci->GetChild(3)->AsUInt64();
  bindingState.indexBuffer = BoundBuffer(buf_id, offset, bufSize - offset, 0);
  bindingState.indexBufferType = indexType;
}

void TraceTracker::CmdBindVertexBuffersAnalyze(SDChunk *o)
{
  uint64_t firstBinding = o->GetChild(1)->AsUInt64();
  uint64_t bindingCount = o->GetChild(2)->AsUInt64();
  SDObject *buffers = o->GetChild(3);
  SDObject *offsets = o->GetChild(4);
  RDCASSERT(bindingCount == buffers->NumChildren());
  RDCASSERT(bindingCount == offsets->NumChildren());
  for(uint64_t i = 0; i < bindingCount; i++)
  {
    uint64_t buf_id = buffers->GetChild(i)->AsUInt64();
    uint64_t offset = offsets->GetChild(i)->AsUInt64();
    ResourceWithViewsMapIter bufCreate_it = ResourceCreateFind(buf_id);
    RDCASSERT(bufCreate_it != ResourceCreateEnd());
    SDObject *ci = bufCreate_it->second.sdobj->GetChild(1);
    uint64_t bufSize = ci->GetChild(3)->AsUInt64();
    bindingState.vertexBuffers[firstBinding + i] = BoundBuffer(buf_id, offset, bufSize - offset, 0);
  }
}

void TraceTracker::CmdCopyBufferToImageAnalyze(SDChunk *o)
{
  SDObject *src = o->GetChild(1);
  uint64_t src_id = src->AsUInt64();
  SDObject *dst = o->GetChild(2);
  VkImageLayout dst_layout = (VkImageLayout)o->GetChild(3)->AsUInt64();
  uint64_t dst_id = dst->AsUInt64();
  SDObject *regions = o->GetChild(5);
  RDCASSERT(o->GetChild(4)->AsUInt64() == regions->NumChildren());

  BufferImageCopyHelper(src_id, dst_id, regions, dst_layout, ACCESS_ACTION_READ, ACCESS_ACTION_CLEAR);
}

void TraceTracker::CmdCopyImageToBufferAnalyze(SDChunk *o)
{
  SDObject *src = o->GetChild(1);
  VkImageLayout src_layout = (VkImageLayout)o->GetChild(2)->AsUInt64();
  uint64_t src_id = src->AsUInt64();
  SDObject *dst = o->GetChild(3);
  uint64_t dst_id = dst->AsUInt64();
  SDObject *regions = o->GetChild(5);
  RDCASSERT(o->GetChild(4)->AsUInt64() == regions->NumChildren());

  BufferImageCopyHelper(dst_id, src_id, regions, src_layout, ACCESS_ACTION_CLEAR, ACCESS_ACTION_READ);
}

void TraceTracker::CmdCopyImageAnalyze(SDChunk *o)
{
  SDObject *src = o->GetChild(1);
  VkImageLayout src_layout = (VkImageLayout)o->GetChild(2)->AsUInt64();
  uint64_t src_id = src->AsUInt64();
  SDObject *dst = o->GetChild(3);
  VkImageLayout dst_layout = (VkImageLayout)o->GetChild(4)->AsUInt64();
  uint64_t dst_id = dst->AsUInt64();

  SDObject *regions = o->GetChild(6);
  for(uint64_t i = 0; i < regions->NumChildren(); i++)
  {
    SDObject *region = regions->GetChild(i);
    SDObject *srcSubresource = region->GetChild(0);
    SDObject *srcOffset = region->GetChild(1);
    SDObject *dstSubresource = region->GetChild(2);
    SDObject *dstOffset = region->GetChild(3);
    SDObject *extent = region->GetChild(4);

    AccessImage(src_id, srcSubresource, srcOffset, extent, src_layout, ACCESS_ACTION_READ);
    AccessImage(dst_id, dstSubresource, dstOffset, extent, dst_layout, ACCESS_ACTION_CLEAR);
  }
}

void TraceTracker::CmdBlitImageAnalyze(SDChunk *o)
{
  SDObject *src = o->GetChild(1);
  VkImageLayout src_layout = (VkImageLayout)o->GetChild(2)->AsUInt64();
  uint64_t src_id = src->AsUInt64();
  SDObject *dst = o->GetChild(3);
  VkImageLayout dst_layout = (VkImageLayout)o->GetChild(4)->AsUInt64();
  uint64_t dst_id = dst->AsUInt64();

  SDObject *regions = o->GetChild(6);
  for(uint64_t i = 0; i < regions->NumChildren(); i++)
  {
    SDObject *region = regions->GetChild(i);
    SDObject *srcSubresource = region->GetChild(0);
    SDObject *srcOffsets = region->GetChild(1);
    SDObject *dstSubresource = region->GetChild(2);
    SDObject *dstOffsets = region->GetChild(3);

    // Convert the two srcOffsets to a srcOffset and srcExtent,
    // and similarly for dst.
    SDObject srcOffset("srcOffset", "VkOffset3D");
    SDObject dstOffset("dstOffset", "VkOffset3D");
    SDObject srcExtent("srcExtent", "VkExtent3D");
    SDObject dstExtent("dstExtent", "VkExtent3D");
    const char *offset_names[3] = {"x", "y", "z"};
    const char *extent_names[3] = {"width", "height", "depth"};
    for(uint32_t j = 0; j < 3; j++)
    {
      uint64_t src_0 = srcOffsets->GetChild(0)->GetChild(j)->AsUInt64();
      uint64_t src_1 = srcOffsets->GetChild(1)->GetChild(j)->AsUInt64();
      uint64_t dst_0 = dstOffsets->GetChild(0)->GetChild(j)->AsUInt64();
      uint64_t dst_1 = dstOffsets->GetChild(1)->GetChild(j)->AsUInt64();
      srcOffset.data.children.push_back(
          (SDObject *)makeSDObject(offset_names[j], std::min(src_0, src_1)));
      dstOffset.data.children.push_back(
          (SDObject *)makeSDObject(offset_names[j], std::min(dst_0, dst_1)));
      srcExtent.data.children.push_back((SDObject *)makeSDObject(
          extent_names[j], std::max(src_0, src_1) - std::min(src_0, src_1)));
      dstExtent.data.children.push_back((SDObject *)makeSDObject(
          extent_names[j], std::max(dst_0, dst_1) - std::min(dst_0, dst_1)));
    }

    AccessImage(src_id, srcSubresource, &srcOffset, &srcExtent, src_layout, ACCESS_ACTION_READ);
    AccessImage(dst_id, dstSubresource, &dstOffset, &dstExtent, dst_layout, ACCESS_ACTION_CLEAR);
  }
}

void TraceTracker::CmdResolveImageAnalyze(SDChunk *o)
{
  SDObject *src = o->GetChild(1);
  VkImageLayout src_layout = (VkImageLayout)o->GetChild(2)->AsUInt64();
  uint64_t src_id = src->AsUInt64();
  SDObject *dst = o->GetChild(3);
  VkImageLayout dst_layout = (VkImageLayout)o->GetChild(4)->AsUInt64();
  uint64_t dst_id = dst->AsUInt64();

  SDObject *regions = o->GetChild(6);
  for(uint64_t i = 0; i < regions->NumChildren(); i++)
  {
    SDObject *region = regions->GetChild(i);
    SDObject *srcSubresource = region->GetChild(0);
    SDObject *srcOffset = region->GetChild(1);
    SDObject *dstSubresource = region->GetChild(2);
    SDObject *dstOffset = region->GetChild(3);
    SDObject *extent = region->GetChild(4);

    AccessImage(src_id, srcSubresource, srcOffset, extent, src_layout, ACCESS_ACTION_READ);
    AccessImage(dst_id, dstSubresource, dstOffset, extent, dst_layout, ACCESS_ACTION_CLEAR);
  }
}

void TraceTracker::CmdCopyBufferAnalyze(SDChunk *o)
{
  uint64_t src_id = o->GetChild(1)->AsUInt64();
  uint64_t dst_id = o->GetChild(2)->AsUInt64();
  SDObject *regions = o->GetChild(4);
  RDCASSERT(regions->NumChildren() == o->GetChild(3)->AsUInt64());

  for(uint64_t i = 0; i < regions->NumChildren(); i++)
  {
    SDObject *region = regions->GetChild(i);
    uint64_t srcOffset = region->GetChild(0)->AsUInt64();
    uint64_t dstOffset = region->GetChild(1)->AsUInt64();
    uint64_t size = region->GetChild(2)->AsUInt64();
    AccessBufferMemory(src_id, srcOffset, size, ACCESS_ACTION_READ);
    AccessBufferMemory(dst_id, dstOffset, size, ACCESS_ACTION_CLEAR);
  }
}

void TraceTracker::CmdUpdateBufferAnalyze(SDChunk *o)
{
  uint64_t dst_id = o->GetChild(1)->AsUInt64();
  uint64_t offset = o->GetChild(2)->AsUInt64();
  uint64_t size = o->GetChild(3)->AsUInt64();
  AccessBufferMemory(dst_id, offset, size, ACCESS_ACTION_CLEAR);
}

void TraceTracker::CmdFillBufferAnalyze(SDChunk *o)
{
  uint64_t dst_id = o->GetChild(1)->AsUInt64();
  uint64_t offset = o->GetChild(2)->AsUInt64();
  uint64_t size = o->GetChild(3)->AsUInt64();
  AccessBufferMemory(dst_id, offset, size, ACCESS_ACTION_CLEAR);
}

void TraceTracker::CmdClearColorImageAnalyze(SDChunk *o)
{
  SDObject *image = o->GetChild(1);
  VkImageLayout image_layout = (VkImageLayout)o->GetChild(2)->AsUInt64();
  uint64_t image_id = image->AsUInt64();

  SDObject *regions = o->GetChild(5);
  for(uint64_t i = 0; i < regions->NumChildren(); i++)
  {
    AccessImage(image_id, regions->GetChild(i), image_layout, ACCESS_ACTION_CLEAR);
  }
}

void TraceTracker::CmdClearDepthStencilImageAnalyze(SDChunk *o)
{
  SDObject *image = o->GetChild(1);
  VkImageLayout image_layout = (VkImageLayout)o->GetChild(2)->AsUInt64();
  uint64_t image_id = image->AsUInt64();

  SDObject *regions = o->GetChild(5);
  for(uint64_t i = 0; i < regions->NumChildren(); i++)
  {
    AccessImage(image_id, regions->GetChild(i), image_layout, ACCESS_ACTION_CLEAR);
  }
}

void TraceTracker::CmdClearAttachmentsAnalyze(SDChunk *o)
{
  SDObject *subpasses = bindingState.renderPass->GetChild(6);
  uint64_t fbWidth = bindingState.framebuffer->GetChild(6)->AsUInt64();
  uint64_t fbHeight = bindingState.framebuffer->GetChild(7)->AsUInt64();

  // TODO: multiview changes layers
  uint64_t fbLayers = bindingState.framebuffer->GetChild(8)->AsUInt64();

  RDCASSERT(bindingState.subpassIndex < subpasses->NumChildren());
  SDObject *subpass = subpasses->GetChild(bindingState.subpassIndex);
  SDObject *colorAttachments = subpass->GetChild(5);
  SDObject *depthStencilAttachment = subpass->GetChild(7);

  SDObject *attachments = o->GetChild(2);
  SDObject *rects = o->GetChild(4);

  std::vector<AccessAction> layerActions(fbLayers, ACCESS_ACTION_NONE);

  for(uint32_t i = 0; i < rects->NumChildren(); i++)
  {
    SDObject *clearRect = rects->GetChild(i);
    SDObject *rect2D = clearRect->GetChild(0);
    SDObject *offset = rect2D->GetChild(0);
    uint64_t offset_x = offset->GetChild(0)->AsUInt64();
    uint64_t offset_y = offset->GetChild(1)->AsUInt64();
    SDObject *extent = rect2D->GetChild(1);
    uint64_t width = extent->GetChild(0)->AsUInt64();
    uint64_t height = extent->GetChild(1)->AsUInt64();
    uint64_t baseArrayLayer = clearRect->GetChild(1)->AsUInt64();
    uint64_t layerCount = clearRect->GetChild(2)->AsUInt64();
    RDCASSERT(layerCount < VK_REMAINING_ARRAY_LAYERS);

    bool fullFrame = (offset_x == 0 && offset_y == 0 && width == fbWidth && height == fbHeight);
    for(uint32_t j = 0; j < layerCount; j++)
    {
      uint64_t layer = baseArrayLayer + j;
      if(fullFrame)
      {
        layerActions[layer] = ACCESS_ACTION_CLEAR;
      }
      else
      {
        layerActions[layer] = std::max(layerActions[layer], ACCESS_ACTION_WRITE);
      }
    }
  }

  for(uint32_t i = 0; i < attachments->NumChildren(); i++)
  {
    SDObject *attachment = attachments->GetChild(i);
    VkImageAspectFlags aspectMask =
        static_cast<VkImageAspectFlags>(attachment->GetChild(0)->AsUInt64());
    uint64_t colorAttachment = attachment->GetChild(1)->AsUInt64();
    for(uint32_t layer = 0; layer < fbLayers; layer++)
    {
      AccessAction action = layerActions[layer];
      if(action == ACCESS_ACTION_NONE)
      {
        continue;
      }

      if(aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
      {
        AccessAttachment(colorAttachments->GetChild(colorAttachment)->GetChild(0)->AsUInt64(),
                         action, aspectMask, layer, 1);
      }

      if(aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
      {
        AccessAttachment(depthStencilAttachment->GetChild(0)->AsUInt64(), action, aspectMask, layer,
                         1);
      }
    }
  }
}

void TraceTracker::CmdPipelineBarrierAnalyze(SDChunk *o)
{
  SDObject *bufBarriers = o->FindChild("pBufferMemoryBarriers");
  for(uint32_t i = 0; i < bufBarriers->NumChildren(); i++)
  {
    SDObject *barrier = bufBarriers->GetChild(i);
    uint64_t bufID = barrier->FindChild("buffer")->AsUInt64();
    uint64_t offset = barrier->FindChild("offset")->AsUInt64();
    uint64_t size = barrier->FindChild("size")->AsUInt64();

    uint64_t srcQueueFamilyIndex = barrier->FindChild("srcQueueFamilyIndex")->AsUInt64();
    uint64_t dstQueueFamilyIndex = barrier->FindChild("dstQueueFamilyIndex")->AsUInt64();

    TransitionBufferQueueFamily(bufID, srcQueueFamilyIndex, dstQueueFamilyIndex, offset, size);
  }
  SDObject *imgBarriers = o->FindChild("pImageMemoryBarriers");
  for(uint32_t i = 0; i < imgBarriers->NumChildren(); i++)
  {
    SDObject *barrier = imgBarriers->GetChild(i);
    uint64_t imageID = barrier->FindChild("image")->AsUInt64();

    // look for imageID in the createdResource map
    ImageStateMapIter image_it = ImageStateFind(imageID);

    // This resource wasn't properly created, skip.
    if(image_it == ImageStateEnd())
      continue;

    SDObject *range = barrier->FindChild("subresourceRange");
    VkImageLayout oldLayout = (VkImageLayout)barrier->FindChild("oldLayout")->AsUInt64();
    VkImageLayout newLayout = (VkImageLayout)barrier->FindChild("newLayout")->AsUInt64();
    uint64_t srcQueueFamilyIndex = barrier->FindChild("srcQueueFamilyIndex")->AsUInt64();
    uint64_t dstQueueFamilyIndex = barrier->FindChild("dstQueueFamilyIndex")->AsUInt64();

    TransitionImageLayout(imageID, range, oldLayout, newLayout, srcQueueFamilyIndex,
                          dstQueueFamilyIndex);
  }
}

void TraceTracker::CmdWaitEventsAnalyze(SDChunk *o)
{
}

void TraceTracker::CmdDispatchAnalyze(SDChunk *o)
{
  // Pessimistically read/write all memory and images accessible through bound descriptor sets
  AccessMemoryInBoundDescriptorSets(bindingState.computePipeline);
}

void TraceTracker::CmdDispatchIndirectAnalyze(SDChunk *o)
{
  uint64_t buf_id = o->GetChild(1)->AsUInt64();
  uint64_t offset = o->GetChild(2)->AsUInt64();
  uint64_t size = 3 * sizeof(uint32_t);
  AccessBufferMemory(buf_id, offset, size, ACCESS_ACTION_READ);

  // Pessimistically read/write all memory and images accessible through bound descriptor sets
  AccessMemoryInBoundDescriptorSets(bindingState.computePipeline);
}

void TraceTracker::CmdDrawIndirectAnalyze(SDChunk *o)
{
  uint64_t buf_id = o->GetChild(1)->AsUInt64();
  uint64_t offset = o->GetChild(2)->AsUInt64();
  uint64_t drawCount = o->GetChild(3)->AsUInt64();
  uint64_t stride = o->GetChild(3)->AsUInt64();
  uint64_t drawSize = 4 * sizeof(uint32_t);
  if(stride == drawSize)
  {
    AccessBufferMemory(buf_id, offset, drawSize * drawCount, ACCESS_ACTION_READ);
  }
  else
  {
    for(uint64_t i = 0; i < drawCount; i++)
    {
      AccessBufferMemory(buf_id, offset + i * stride, drawSize, ACCESS_ACTION_READ);
    }
  }

  // Pessimistically read all bound vertices
  ReadBoundVertexBuffers(UINT64_MAX, UINT64_MAX, 0, 0);

  // Pessimistically read/write all memory and images accessible through bound descriptor sets
  AccessMemoryInBoundDescriptorSets(bindingState.graphicsPipeline);

  // Pessimistically read/write accessible through the subpass image attachments
  AccessSubpassAttachments();
}

void TraceTracker::CmdDrawIndexedIndirectAnalyze(SDChunk *o)
{
  uint64_t buf_id = o->GetChild(1)->AsUInt64();
  uint64_t offset = o->GetChild(2)->AsUInt64();
  uint64_t drawCount = o->GetChild(3)->AsUInt64();
  uint64_t stride = o->GetChild(3)->AsUInt64();
  uint64_t drawSize = 5 * sizeof(uint32_t);

  // Read indirect buffer
  if(stride == drawSize)
  {
    AccessBufferMemory(buf_id, offset, drawSize * drawCount, ACCESS_ACTION_READ);
  }
  else
  {
    for(uint64_t i = 0; i < drawCount; i++)
    {
      AccessBufferMemory(buf_id, offset + i * stride, drawSize, ACCESS_ACTION_READ);
    }
  }

  // Pessimistically read entire index buffer (we can't know at code gen time which parts of the
  // index buffer are actually read).
  AccessBufferMemory(bindingState.indexBuffer.buffer, bindingState.indexBuffer.offset,
                     bindingState.indexBuffer.size, ACCESS_ACTION_READ);

  // Pessimistically read all bound vertices
  ReadBoundVertexBuffers(UINT64_MAX, UINT64_MAX, 0, 0);

  // Pessimistically read/write all memory and images accessible through bound descriptor sets
  AccessMemoryInBoundDescriptorSets(bindingState.graphicsPipeline);

  // Pessimistically read/write accessible through the subpass image attachments
  AccessSubpassAttachments();
}

void TraceTracker::CmdDrawAnalyze(SDChunk *o)
{
  uint64_t vertexCount = o->GetChild(1)->AsUInt64();
  uint64_t instanceCount = o->GetChild(2)->AsUInt64();
  uint64_t firstVertex = o->GetChild(3)->AsUInt64();
  uint64_t firstInstance = o->GetChild(4)->AsUInt64();
  ReadBoundVertexBuffers(vertexCount, instanceCount, firstVertex, firstInstance);

  // Pessimistically read/write all memory and images accessible through bound descriptor sets
  AccessMemoryInBoundDescriptorSets(bindingState.graphicsPipeline);

  // Pessimistically read/write accessible through the subpass image attachments
  AccessSubpassAttachments();
}

void TraceTracker::CmdDrawIndexedAnalyze(SDChunk *o)
{
  uint64_t indexCount = o->GetChild(1)->AsUInt64();
  uint64_t instanceCount = o->GetChild(2)->AsUInt64();
  uint64_t firstIndex = o->GetChild(3)->AsUInt64();
  uint64_t firstInstance = o->GetChild(5)->AsUInt64();
  uint64_t indexElemSize;
  switch(bindingState.indexBufferType)
  {
    case VkIndexType::VK_INDEX_TYPE_UINT16: indexElemSize = sizeof(uint16_t); break;
    case VkIndexType::VK_INDEX_TYPE_UINT32: indexElemSize = sizeof(uint32_t); break;
    default:
      RDCASSERT(0);
      indexElemSize = 1;
      break;
  }
  uint64_t indexSize = indexCount * indexElemSize;
  uint64_t indexOffset = bindingState.indexBuffer.offset + indexElemSize * firstIndex;
  AccessBufferMemory(bindingState.indexBuffer.buffer, indexOffset, indexSize, ACCESS_ACTION_READ);
  ReadBoundVertexBuffers(UINT64_MAX, instanceCount, 0, firstInstance);

  // Pessimistically read/write all memory and images accessible through bound descriptor sets
  AccessMemoryInBoundDescriptorSets(bindingState.graphicsPipeline);

  // Pessimistically read/write accessible through the subpass image attachments
  AccessSubpassAttachments();
}
}    // namespace vk_cpp_codec

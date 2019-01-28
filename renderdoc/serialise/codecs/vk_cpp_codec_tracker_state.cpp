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
* Helpers for the Cmd...Analyze() methods.
* These methods update state variables used to track reads/writes of memory
* and images.
***************************************************************************/

void TraceTracker::AccessBufferMemory(uint64_t buf_id, uint64_t offset, uint64_t size,
                                      AccessAction action)
{
  RDCASSERT(IsValidNonNullResouce(buf_id));
  SDObject *memBinding = FindBufferMemBinding(buf_id);
  uint64_t mem_id = memBinding->FindChild("memory")->AsUInt64();
  uint64_t mem_offset = memBinding->FindChild("memoryOffset")->AsUInt64();
  MemAllocWithResourcesMapIter mem_it = MemAllocFind(mem_id);
  RDCASSERT(mem_it != MemAllocEnd());

  ResourceWithViewsMapIter buf_it = ResourceCreateFind(buf_id);
  RDCASSERT(buf_it != ResourceCreateEnd());
  SDObject *ci = buf_it->second.sdobj->FindChild("CreateInfo");
  VkSharingMode sharingMode = (VkSharingMode)ci->FindChild("sharingMode")->AsUInt64();
  uint64_t buf_size = ci->FindChild("size")->AsUInt64();

  if(size > buf_size - offset)
  {
    if(size != VK_WHOLE_SIZE)
    {
      RDCWARN("Buffer used in descriptor set update has size (%llu) but range listed is (%llu)",
              buf_size, size);
    }
    size = buf_size - offset;
  }

  mem_it->second.Access(cmdQueueFamily, sharingMode, action, offset + mem_offset, size);
}

void TraceTracker::TransitionBufferQueueFamily(uint64_t buf_id, uint64_t srcQueueFamily,
                                               uint64_t dstQueueFamily, uint64_t offset,
                                               uint64_t size)
{
  RDCASSERT(IsValidNonNullResouce(buf_id));
  SDObject *memBinding = FindBufferMemBinding(buf_id);
  uint64_t mem_id = memBinding->FindChild("memory")->AsUInt64();
  uint64_t mem_offset = memBinding->FindChild("memoryOffset")->AsUInt64();
  MemAllocWithResourcesMapIter mem_it = MemAllocFind(mem_id);
  RDCASSERT(mem_it != MemAllocEnd());

  ResourceWithViewsMapIter buf_it = ResourceCreateFind(buf_id);
  RDCASSERT(buf_it != ResourceCreateEnd());
  SDObject *ci = buf_it->second.sdobj->FindChild("CreateInfo");
  VkSharingMode sharingMode = (VkSharingMode)ci->FindChild("sharingMode")->AsUInt64();
  uint64_t buf_size = ci->FindChild("size")->AsUInt64();

  if(size > buf_size - offset)
  {
    if(size != VK_WHOLE_SIZE)
    {
      RDCWARN("Buffer used in descriptor set update has size (%llu) but range listed is (%llu)",
              buf_size, size);
    }
    size = buf_size - offset;
  }

  mem_it->second.TransitionQueueFamily(cmdQueueFamily, sharingMode, srcQueueFamily, dstQueueFamily,
                                       mem_offset + offset, size);
}

void TraceTracker::ReadBoundVertexBuffers(uint64_t vertexCount, uint64_t instanceCount,
                                          uint64_t firstVertex, uint64_t firstInstance)
{
  SDChunkIDMapIter pipeline_it = createdPipelines.find(bindingState.graphicsPipeline.pipeline);
  RDCASSERT(pipeline_it != createdPipelines.end());
  SDObject *vertexInputState = pipeline_it->second->GetChild(3)->GetChild(5);
  SDObject *boundVertexDescriptions = vertexInputState->GetChild(4);
  for(uint64_t i = 0; i < boundVertexDescriptions->NumChildren(); i++)
  {
    uint64_t bindingNum = boundVertexDescriptions->GetChild(i)->GetChild(0)->AsUInt64();
    uint64_t stride = boundVertexDescriptions->GetChild(i)->GetChild(1)->AsUInt64();
    uint64_t inputRate = boundVertexDescriptions->GetChild(i)->GetChild(2)->AsUInt64();
    uint64_t startVertex, numVertices;
    switch(inputRate)
    {
      case VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX:
        startVertex = firstVertex;
        numVertices = vertexCount;
        break;
      case VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE:
        startVertex = firstInstance;
        numVertices = instanceCount;
        break;
      default:
        RDCASSERT(0);
        startVertex = numVertices = 0;
        break;
    }
    std::map<uint64_t, BoundBuffer>::iterator boundBuffer_it =
        bindingState.vertexBuffers.find(bindingNum);
    if(boundBuffer_it != bindingState.vertexBuffers.end())
    {
      uint64_t offset = startVertex * stride;
      uint64_t size;
      if(numVertices == UINT64_MAX || (uint32_t)numVertices == UINT32_MAX)
      {
        RDCASSERT(boundBuffer_it->second.size >= offset);
        size = boundBuffer_it->second.size - offset;
      }
      else
      {
        size = numVertices * stride;
      }
      offset += boundBuffer_it->second.offset;
      AccessBufferMemory(boundBuffer_it->second.buffer, offset, size, ACCESS_ACTION_READ);
    }
  }
}

void TraceTracker::AccessMemoryInBoundDescriptorSets(BoundPipeline &boundPipeline)
{
  SDChunkIDMapIter pipeline_it = createdPipelines.find(boundPipeline.pipeline);
  RDCASSERT(pipeline_it != createdPipelines.end());

  uint64_t pipelineLayout_id = 0;
  switch(pipeline_it->second->metadata.chunkID)
  {
    case(uint32_t)VulkanChunk::vkCreateGraphicsPipelines:
      pipelineLayout_id = pipeline_it->second->GetChild(3)->GetChild(14)->AsUInt64();
      break;
    case(uint32_t)VulkanChunk::vkCreateComputePipelines:
      pipelineLayout_id = pipeline_it->second->GetChild(3)->GetChild(4)->AsUInt64();
      break;
    default: RDCASSERT(0);
  }
  ResourceWithViewsMapIter pipelineLayout_it = ResourceCreateFind(pipelineLayout_id);
  RDCASSERT(pipelineLayout_it != ResourceCreateEnd());
  SDObject *pipelineLayout_ci = pipelineLayout_it->second.sdobj->GetChild(1);

  uint64_t setLayoutCount = pipelineLayout_ci->GetChild(3)->AsUInt64();
  SDObject *setLayouts = pipelineLayout_ci->GetChild(4);
  RDCASSERT(setLayoutCount == setLayouts->NumChildren());

  for(uint64_t i = 0; i < setLayoutCount; i++)
  {
    U64MapIter descriptorSet_it = boundPipeline.descriptorSets.find(i);
    if(descriptorSet_it != boundPipeline.descriptorSets.end())
    {
      uint64_t descriptorSet = descriptorSet_it->second;
      uint64_t setLayout = setLayouts->GetChild(i)->AsUInt64();
      AccessMemoryInDescriptorSet(descriptorSet, setLayout);
    }
  }
}

void TraceTracker::AccessMemoryInDescriptorSet(uint64_t descriptorSet_id, uint64_t setLayout_id)
{
  DescriptorSetInfoMapIter descriptorSet_it = descriptorSetInfos.find(descriptorSet_id);
  RDCASSERT(descriptorSet_it != descriptorSetInfos.end());
  DescriptorSetInfo &descriptorSet = descriptorSet_it->second;
  for(DescriptorBindingMapIter it = descriptorSet.bindings.begin();
      it != descriptorSet.bindings.end(); it++)
  {
    AccessAction action = ACCESS_ACTION_READ;
    DescriptorBinding &binding = it->second;
    switch(binding.type)
    {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
        // Only a sampler, no image to access
        break;
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        action = ACCESS_ACTION_READ_WRITE;
      // Fall through; storage can also be read.
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        for(uint64_t i = 0; i < binding.imageBindings.size(); i++)
        {
          BoundImage imageBinding = binding.imageBindings[i];
          if(!imageBinding.bound)
          {
// TODO(bjoeris): This warning is extremely noisy for some traces.
// Figure out whether this is:
//  1. A code gen bug,
//  2. RenderDoc not serializing some the descriptor sets, or
//  3. Valid application behaviour. Does the validation layer allow this?
#if 0
              RDCWARN("Descriptor set %llu, binding %llu, index %d, is not bound to any image view.",
                descriptorSet_id, it->first, i);
#endif
            continue;
          }
          else if(!IsValidNonNullResouce(imageBinding.imageView))
          {
            RDCWARN("Descriptor set %llu, binding %llu, index %d, bound to invalid image view %llu",
                    descriptorSet_id, it->first, i, imageBinding.imageView);
            continue;
          }
          AccessImageView(imageBinding.imageView, imageBinding.imageLayout, action);
          // TODO: Is any layout analysis needed here?
        }
        break;

      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        action = ACCESS_ACTION_READ_WRITE;
      // Fall through; storage can also be read.
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        for(uint64_t i = 0; i < binding.bufferBindings.size(); i++)
        {
          BoundBuffer bufferBinding = binding.bufferBindings[i];
          if(!bufferBinding.bound)
          {
// TODO(bjoeris): This warning is extremely noisy for some traces.
// Figure out whether this is:
//  1. A code gen bug,
//  2. RenderDoc not serializing some the descriptor sets, or
//  3. Valid application behaviour. Does the validation layer allow this?
#if 0
              RDCWARN("Descriptor set %llu, binding %llu, index %d, is not bound to any buffer.",
                descriptorSet_id, it->first, i);
#endif
            continue;
          }
          else if(!IsValidNonNullResouce(bufferBinding.buffer))
          {
            RDCWARN("Descriptor set %llu, binding %llu, index %d, bound to invalid buffer %llu",
                    descriptorSet_id, it->first, i, bufferBinding.buffer);
            continue;
          }
          uint64_t offset = bufferBinding.offset + bufferBinding.dynamicOffset;
          AccessBufferMemory(bufferBinding.buffer, offset, bufferBinding.size, action);
        }
        break;

      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        action = ACCESS_ACTION_READ_WRITE;
      // Fall through; storage can also be read.
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        for(uint64_t i = 0; i < binding.texelViewBindings.size(); i++)
        {
          uint64_t view_id = binding.texelViewBindings[i].texelBufferView;
          if(view_id == 0)
          {
// TODO(bjoeris): This warning is extremely noisy for some traces.
// Figure out whether this is:
//  1. A code gen bug,
//  2. RenderDoc not serializing some the descriptor sets, or
//  3. Valid application behaviour. Does the validation layer allow this?
#if 0
              RDCWARN("Descriptor set %llu, binding %llu, index %d, not bound to any buffer view",
                descriptorSet_id, it->first, i);
#endif
            continue;
          }
          ResourceWithViewsMapIter view_it = ResourceCreateFind(view_id);
          // TODO(akharlamov) why is this checking ResourceCreateEnd and not
          // IsValidNonNullResouce?..
          if(view_it == ResourceCreateEnd())
          {
            RDCWARN(
                "Descriptor set %llu, binding %llu, index %d, bound to invalid buffer view %llu",
                descriptorSet_id, it->first, i, view_id);
            continue;
          }
          SDObject *ci = view_it->second.sdobj->GetChild(1);
          uint64_t buffer = ci->GetChild(3)->AsUInt64();
          uint64_t offset = ci->GetChild(5)->AsUInt64();
          uint64_t size = ci->GetChild(6)->AsUInt64();
          if(!IsValidNonNullResouce(buffer))
          {
            RDCWARN(
                "Descriptor set %llu, binding %llu, index %d, bound to invalid buffer %llu via "
                "buffer view %llu",
                descriptorSet_id, it->first, i, buffer, view_id);
            continue;
          }
          AccessBufferMemory(buffer, offset, size, action);
        }
        break;
      default: break;
    }
  }
}

void TraceTracker::AccessImage(uint64_t image, VkImageAspectFlags aspectMask, uint64_t baseMipLevel,
                               uint64_t levelCount, uint64_t baseArrayLayer, uint64_t layerCount,
                               bool is2DView, VkImageLayout layout, AccessAction action)
{
  std::function<AccessState(AccessState)> transition = GetAccessStateTransition(action);
  ResourceWithViewsMapIter image_it = ResourceCreateFind(image);
  if(image_it == ResourceCreateEnd() && !IsPresentationResource(image))
  {
    RDCASSERT(0);    // TODO: should this ever happen?
    return;
  }
  ImageStateMapIter imageState_it = imageStates.find(image);
  RDCASSERT(imageState_it != imageStates.end());
  ImageState &imageState = imageState_it->second;

  ImageSubresourceRange range =
      imageState.Range(aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount, is2DView);
  for(ImageSubresourceRangeIter res_it = range.begin(); res_it != range.end(); res_it++)
  {
    ImageSubresourceState &resState = imageState.At(*res_it);
    resState.Access(cmdQueueFamily, layout, transition);
  }
}

void TraceTracker::AccessImage(uint64_t image, SDObject *subresource, VkImageLayout layout,
                               AccessAction action)
{
  RDCASSERT(std::string(subresource->Type()) == "VkImageSubresourceRange");

  VkImageAspectFlags aspectMask = (VkImageAspectFlags)subresource->GetChild(0)->AsUInt64();
  uint64_t baseMipLevel = subresource->GetChild(1)->AsUInt64();
  uint64_t levelCount = subresource->GetChild(2)->AsUInt64();
  uint64_t baseArrayLayer = subresource->GetChild(3)->AsUInt64();
  uint64_t layerCount = subresource->GetChild(4)->AsUInt64();

  AccessImage(image, aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount, false,
              layout, action);
}

bool isFullImage(SDObject *imageExtent, SDObject *offset, SDObject *extent, uint64_t mipLevel)
{
  uint64_t offsetV[3]{0, 0, 0};
  uint64_t imageExtentV[3];
  uint64_t extentV[3];
  for(uint64_t i = 0; i < 3; i++)
  {
    uint64_t d = imageExtent->GetChild(i)->AsUInt64();
    imageExtentV[i] = extentV[i] =
        (d + (1ull << mipLevel) - 1) >> mipLevel;    // ceil(d / (2^mipLevel))
  }
  if(offset != NULL)
  {
    RDCASSERT(std::string(offset->Type()).substr(0, 8) == "VkOffset");
    for(uint64_t i = 0; i < offset->NumChildren(); i++)
    {
      offsetV[i] = offset->GetChild(i)->AsUInt64();
    }
  }
  if(extent != NULL)
  {
    RDCASSERT(std::string(extent->Type()).substr(0, 8) == "VkExtent");
    for(uint64_t i = 0; i < extent->NumChildren(); i++)
    {
      extentV[i] = extent->GetChild(i)->AsUInt64();
    }
  }
  bool fullImage = true;
  for(uint64_t i = 0; i < 3; i++)
  {
    if(offsetV[i] != 0 || extentV[i] != imageExtentV[i])
    {
      fullImage = false;
      RDCASSERT(extentV[i] < imageExtentV[i]);    // TODO: are there magic values like
                                                  // VK_REMAINING_MIP_LEVELS that indicate "full
                                                  // dimension"?
    }
  }
  return fullImage;
}

void TraceTracker::AccessImage(uint64_t image, SDObject *subresource, SDObject *offset,
                               SDObject *extent, VkImageLayout layout, AccessAction action)
{
  RDCASSERT(std::string(subresource->Type()) == "VkImageSubresourceLayers");
  VkImageAspectFlags aspectMask = (VkImageAspectFlags)subresource->GetChild(0)->AsUInt64();
  uint64_t mipLevel = subresource->GetChild(1)->AsUInt64();
  uint64_t baseArrayLayer = subresource->GetChild(2)->AsUInt64();
  uint64_t layerCount = subresource->GetChild(3)->AsUInt64();

  if(action == ACCESS_ACTION_CLEAR)
  {
    // The image subresource is being 'cleared', but we need to check whether
    // the whole image is cleared, or only part.

    ResourceWithViewsMapIter image_it = ResourceCreateFind(image);
    if(image_it == ResourceCreateEnd())
    {
      // TODO: this happens a lot. Is that expected?
      // RDCASSERT(0); // TODO: should this ever happen?
      return;
    }
    SDObject *image_ci = image_it->second.sdobj->GetChild(1);
    SDObject *imageExtent = image_ci->GetChild(5);

    // TODO(akharlamov, bjoeris) I think this should include aspect for depth stencil resources.
    if(!isFullImage(imageExtent, offset, extent, mipLevel))
    {
      // action is 'clear', but only part of the image is cleared, which is actually a 'write'.
      action = ACCESS_ACTION_WRITE;
    }
  }

  AccessImage(image, aspectMask, mipLevel, 1, baseArrayLayer, layerCount, false, layout, action);
}

void TraceTracker::AccessImageView(uint64_t view, VkImageLayout layout, AccessAction action,
                                   VkImageAspectFlags aspectMask, uint64_t baseArrayLayer,
                                   uint64_t layerCount)
{
  ResourceWithViewsMapIter view_it = ResourceCreateFind(view);
  SDObjectIDMapIter present_it = presentResources.find(view);
  if(view_it == ResourceCreateEnd() || present_it != presentResources.end())
  {
    // This can happen for views of swapchain images
    return;
  }
  SDObject *view_ci = view_it->second.sdobj->GetChild(1);
  uint64_t image = view_ci->GetChild(3)->AsUInt64();
  SDObject *subresource = view_ci->GetChild(7);

  VkImageViewType viewType = (VkImageViewType)view_ci->GetChild(4)->AsUInt64();

  bool is2DView = (viewType == VK_IMAGE_VIEW_TYPE_2D) || (viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY);

  uint64_t viewAspectMask = subresource->GetChild(0)->AsUInt64();
  uint64_t baseMipLevel = subresource->GetChild(1)->AsUInt64();
  uint64_t levelCount = subresource->GetChild(2)->AsUInt64();
  uint64_t viewBaseArrayLayer = subresource->GetChild(3)->AsUInt64();
  uint64_t viewLayerCount = subresource->GetChild(4)->AsUInt64();
  uint64_t lastArrayLayer, viewLastArrayLayer;

  if(layerCount == VK_REMAINING_ARRAY_LAYERS)
  {
    lastArrayLayer = VK_REMAINING_ARRAY_LAYERS;
  }
  else
  {
    lastArrayLayer = baseArrayLayer + layerCount;
  }
  if(viewLayerCount == VK_REMAINING_ARRAY_LAYERS)
  {
    viewLastArrayLayer = VK_REMAINING_ARRAY_LAYERS;
  }
  else
  {
    viewLastArrayLayer = viewBaseArrayLayer + viewLayerCount;
  }

  baseArrayLayer = std::max(baseArrayLayer, viewBaseArrayLayer);
  lastArrayLayer = std::min(lastArrayLayer, viewLastArrayLayer);
  layerCount = lastArrayLayer - baseArrayLayer;

  aspectMask &= viewAspectMask;

  AccessImage(image, aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount, is2DView,
              layout, action);
}

void TraceTracker::AccessAttachment(uint64_t attachment, AccessAction action,
                                    VkImageAspectFlags aspectMask, uint64_t baseArrayLayer,
                                    uint64_t layerCount)
{
  if(attachment == VK_ATTACHMENT_UNUSED)
  {
    return;
  }
  uint64_t view_id = bindingState.framebuffer->GetChild(5)->GetChild(attachment)->AsUInt64();
  VkImageLayout layout = bindingState.attachmentLayout[attachment];
  RDCASSERT(layout != VK_IMAGE_LAYOUT_MAX_ENUM);

  AccessImageView(view_id, layout, action, aspectMask, baseArrayLayer, layerCount);
}

void TraceTracker::AccessSubpassAttachments()
{
  SDObject *subpasses = bindingState.renderPass->GetChild(6);

  RDCASSERT(bindingState.subpassIndex < subpasses->NumChildren());
  SDObject *subpass = subpasses->GetChild(bindingState.subpassIndex);
  SDObject *colorAttachments = subpass->GetChild(5);
  SDObject *resolveAttachments = subpass->GetChild(6);
  SDObject *depthStencilAttachment = subpass->GetChild(7);

  SDChunkIDMapIter pipeline_it = createdPipelines.find(bindingState.graphicsPipeline.pipeline);
  if(pipeline_it == createdPipelines.end())
  {
    return;
  }
  SDObject *pipeline_ci = pipeline_it->second->FindChild("CreateInfo");
  SDObject *depthStencilState = pipeline_ci->FindChild("pDepthStencilState");
  SDObject *blendState = pipeline_ci->FindChild("pColorBlendState");
  SDObject *blendAttachmets = blendState->FindChild("pAttachments");

  for(uint64_t i = 0; i < colorAttachments->NumChildren(); i++)
  {
    SDObject *blendAttachmentState = blendAttachmets->GetChild(i);
    uint64_t blendEnabled = blendAttachmentState->FindChild("blendEnable")->AsUInt64();
    VkColorComponentFlags colorWriteMask =
        (uint32_t)blendAttachmentState->FindChild("colorWriteMask")->AsUInt64();
    AccessAction action;
    if(blendEnabled == 0)
    {
      // Previous color is completely ignored, so write only
      action = ACCESS_ACTION_WRITE;
    }
    else
    {
      // Blending may use the previous color, so read-write.
      action = ACCESS_ACTION_READ_WRITE;
    }
    if(colorWriteMask != 0)
    {
      // Writing is enabled for at least some color component
      AccessAttachment(colorAttachments->GetChild(i)->GetChild(0)->AsUInt64(), action);
    }
  }
  for(uint64_t i = 0; i < resolveAttachments->NumChildren(); i++)
  {
    AccessAttachment(resolveAttachments->GetChild(i)->GetChild(0)->AsUInt64(), ACCESS_ACTION_WRITE);
  }
  if(!depthStencilAttachment->IsNULL())
  {
    AccessAction action = ACCESS_ACTION_READ_WRITE;
    if(depthStencilState->FindChild("depthTestEnable")->AsUInt64() == 0)
    {
      action = ACCESS_ACTION_WRITE;
    }
    if(depthStencilState->FindChild("depthWriteEnable")->AsUInt64() == 0)
    {
      switch(action)
      {
        case ACCESS_ACTION_READ_WRITE: action = ACCESS_ACTION_READ; break;
        case ACCESS_ACTION_WRITE: action = ACCESS_ACTION_NONE; break;
        default: RDCASSERT(0);
      }
    }
    if(action != ACCESS_ACTION_NONE)
    {
      AccessAttachment(depthStencilAttachment->GetChild(0)->AsUInt64(), action);
    }
  }
}

void TraceTracker::TransitionImageLayout(uint64_t image, SDObject *range, VkImageLayout oldLayout,
                                         VkImageLayout newLayout, uint64_t srcQueueFamily,
                                         uint64_t dstQueueFamily)
{
  VkImageAspectFlags aspectMask = (VkImageAspectFlags)range->FindChild("aspectMask")->AsUInt64();
  uint64_t baseMip = range->FindChild("baseMipLevel")->AsUInt64();
  uint64_t levelCount = range->FindChild("levelCount")->AsUInt64();
  uint64_t baseLayer = range->FindChild("baseArrayLayer")->AsUInt64();
  uint64_t layerCount = range->FindChild("layerCount")->AsUInt64();

  ImageStateMapIter imageState_it = imageStates.find(image);
  RDCASSERT(imageState_it != imageStates.end());
  ImageState &imageState = imageState_it->second;

  ImageSubresourceRange imageRange =
      imageState.Range(aspectMask, baseMip, levelCount, baseLayer, layerCount);
  for(ImageSubresourceRangeIter res_it = imageRange.begin(); res_it != imageRange.end(); res_it++)
  {
    imageState.At(*res_it).Transition(cmdQueueFamily, oldLayout, newLayout, srcQueueFamily,
                                      dstQueueFamily);
  }
}

void TraceTracker::TransitionImageViewLayout(uint64_t viewID, VkImageLayout oldLayout,
                                             VkImageLayout newLayout, uint64_t srcQueueFamily,
                                             uint64_t dstQueueFamily)
{
  ResourceWithViewsMapIter view_it = ResourceCreateFind(viewID);
  RDCASSERT(view_it != ResourceCreateEnd());
  SDObject *view = view_it->second.sdobj;                       // get the vkCreateImageView call
  SDObject *viewCI = view->FindChild("CreateInfo");             // the VkImageViewCreateInfo
  uint64_t imageID = viewCI->FindChild("image")->AsUInt64();    // get the image ID
  // look for it in the createdResource map

  SDObject *subresource = viewCI->FindChild("subresourceRange");
  VkImageViewType viewType = (VkImageViewType)viewCI->FindChild("viewType")->AsUInt64();
  bool is2DView = viewType == VK_IMAGE_VIEW_TYPE_2D || viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  VkImageAspectFlags aspectMask =
      (VkImageAspectFlags)subresource->FindChild("aspectMask")->AsUInt64();
  uint64_t baseMip = subresource->FindChild("baseMipLevel")->AsUInt64();
  uint64_t levelCount = subresource->FindChild("levelCount")->AsUInt64();
  uint64_t baseLayer = subresource->FindChild("baseArrayLayer")->AsUInt64();
  uint64_t layerCount = subresource->FindChild("layerCount")->AsUInt64();

  ImageStateMapIter imageState_it = imageStates.find(imageID);
  RDCASSERT(imageState_it != imageStates.end());
  ImageState &imageState = imageState_it->second;

  ImageSubresourceRange range =
      imageState.Range(aspectMask, baseMip, levelCount, baseLayer, layerCount, is2DView);
  for(ImageSubresourceRangeIter res_it = range.begin(); res_it != range.end(); res_it++)
  {
    imageState.At(*res_it).Transition(cmdQueueFamily, oldLayout, newLayout, srcQueueFamily,
                                      dstQueueFamily);
  }
}

void TraceTracker::TransitionAttachmentLayout(uint64_t attachment, VkImageLayout layout)
{
  if(attachment == VK_ATTACHMENT_UNUSED)
  {
    return;
  }
  RDCASSERT(layout != VK_IMAGE_LAYOUT_UNDEFINED && layout != VK_IMAGE_LAYOUT_PREINITIALIZED);
  uint64_t viewID =
      bindingState.framebuffer->FindChild("pAttachments")->GetChild(attachment)->AsUInt64();

  VkImageLayout oldLayout = bindingState.attachmentLayout[attachment];
  RDCASSERT(oldLayout != VK_IMAGE_LAYOUT_MAX_ENUM);

  TransitionImageViewLayout(viewID, oldLayout, layout, VK_QUEUE_FAMILY_IGNORED,
                            VK_QUEUE_FAMILY_IGNORED);
}

void TraceTracker::LoadSubpassAttachment(SDObject *attachmentRef)
{
  uint64_t attachment = attachmentRef->FindChild("attachment")->AsUInt64();
  VkImageLayout layout = (VkImageLayout)attachmentRef->FindChild("layout")->AsUInt64();

  if(attachment == VK_ATTACHMENT_UNUSED)
  {
    return;
  }
  SDObject *att_desc = bindingState.renderPass->GetChild(4)->GetChild(attachment);
  uint64_t view_id = bindingState.framebuffer->GetChild(5)->GetChild(attachment)->AsUInt64();

  if(bindingState.subpassIndex == bindingState.attachmentFirstUse[attachment])
  {
    // This is the first subpass to use the attachment. This triggers the attachment's
    // loadOp/stencilLoadOp

    VkFormat format = (VkFormat)att_desc->FindChild("format")->AsUInt64();
    VkImageLayout initialLayout = (VkImageLayout)att_desc->FindChild("initialLayout")->AsUInt64();

    if(!IsStencilOnlyFormat(format))
    {
      // The attachment has a depth or color component;
      // load behaviour for depth/color component is defined by loadOp
      VkAttachmentLoadOp loadOp = (VkAttachmentLoadOp)att_desc->FindChild("loadOp")->AsUInt64();
      AccessAction action;
      if(loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ||
         (loadOp == VK_ATTACHMENT_LOAD_OP_DONT_CARE &&
          ((optimizations & CODE_GEN_OPT_TREAT_LOAD_OP_DONT_CARE_AS_CLEAR) != 0)))
      {
        if(bindingState.isFullRenderArea)
        {
          action = ACCESS_ACTION_CLEAR;
        }
        else
        {
          action = ACCESS_ACTION_WRITE;
        }
      }
      else
      {
        action = ACCESS_ACTION_READ;
      }
      AccessImageView(view_id, initialLayout, action,
                      VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_COLOR_BIT);
    }
    if(IsStencilFormat(format))
    {
      // The attachment has a stencil component;
      // load behaviour for stencil component is defined by stencilLoadOp
      VkAttachmentLoadOp stencilLoadOp =
          (VkAttachmentLoadOp)att_desc->FindChild("stencilLoadOp")->AsUInt64();
      AccessAction stencilAction;
      if(stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ||
         (stencilLoadOp == VK_ATTACHMENT_LOAD_OP_DONT_CARE &&
          ((optimizations & CODE_GEN_OPT_TREAT_LOAD_OP_DONT_CARE_AS_CLEAR) != 0)))
      {
        if(bindingState.isFullRenderArea)
        {
          stencilAction = ACCESS_ACTION_CLEAR;
        }
        else
        {
          stencilAction = ACCESS_ACTION_WRITE;
        }
      }
      else
      {
        stencilAction = ACCESS_ACTION_READ;
      }
      AccessImageView(view_id, initialLayout, stencilAction, VK_IMAGE_ASPECT_STENCIL_BIT);
    }
  }

  TransitionAttachmentLayout(attachment, layout);
}

void TraceTracker::BeginSubpass()
{
  SDObject *subpasses = bindingState.renderPass->FindChild("pSubpasses");
  RDCASSERT(bindingState.subpassIndex < subpasses->NumChildren());
  SDObject *subpass = subpasses->GetChild(bindingState.subpassIndex);
  SDObject *inputAttachments = subpass->FindChild("pInputAttachments");
  SDObject *colorAttachments = subpass->FindChild("pColorAttachments");
  SDObject *resolveAttachments = subpass->FindChild("pResolveAttachments");
  SDObject *depthStencilAttachment = subpass->FindChild("pDepthStencilAttachment");

  for(uint64_t i = 0; i < inputAttachments->NumChildren(); i++)
  {
    SDObject *attachmentRef = inputAttachments->GetChild(i);
    uint64_t a = attachmentRef->FindChild("attachment")->AsUInt64();
    LoadSubpassAttachment(attachmentRef);
    AccessAttachment(a, ACCESS_ACTION_READ);
  }
  for(uint32_t i = 0; i < colorAttachments->NumChildren(); i++)
  {
    LoadSubpassAttachment(colorAttachments->GetChild(i));
  }
  for(uint32_t i = 0; i < resolveAttachments->NumChildren(); i++)
  {
    LoadSubpassAttachment(resolveAttachments->GetChild(i));
  }
  if(!depthStencilAttachment->IsNULL())
  {
    LoadSubpassAttachment(depthStencilAttachment);
  }
}

void TraceTracker::EndSubpass()
{
}

SDObject *TraceTracker::FindBufferMemBinding(uint64_t buf_id)
{
  ResourceWithViewsMapIter bufCreate_it = ResourceCreateFind(buf_id);
  RDCASSERT(bufCreate_it != ResourceCreateEnd());
  SDObject *result = 0;
  for(SDObjectIDMapIter v_it = bufCreate_it->second.views.begin();
      v_it != bufCreate_it->second.views.end(); v_it++)
  {
    if(v_it->second->name == std::string("vkBindBufferMemory"))
    {
      RDCASSERT(result == 0);    // We should only find one memory binding for the buffer
      result = v_it->second;
    }
  }
  RDCASSERT(result != 0);
  return result;
}

void TraceTracker::BufferImageCopyHelper(uint64_t buf_id, uint64_t img_id, SDObject *regions,
                                         VkImageLayout imageLayout, AccessAction bufferAction,
                                         AccessAction imageAction)
{
  ResourceWithViewsMapIter imgCreate_it = ResourceCreateFind(img_id);

  RDCASSERT(imgCreate_it != ResourceCreateEnd());
  SDObject *image_ci = imgCreate_it->second.sdobj->GetChild(1);
  VkFormat imageFormat = (VkFormat)image_ci->GetChild(4)->AsUInt64();

  for(uint64_t i = 0; i < regions->NumChildren(); i++)
  {
    SDObject *region = regions->GetChild(i);
    SDObject *imageSubresource = region->GetChild(3);
    uint64_t aspectMask = imageSubresource->GetChild(0)->AsUInt64();

    uint64_t layerCount = imageSubresource->GetChild(3)->AsUInt64();
    SDObject *regionOffset = region->GetChild(4);
    SDObject *regionExtent = region->GetChild(5);
    uint32_t regionWidth = (uint32_t)regionExtent->GetChild(0)->AsUInt64();
    uint32_t regionHeight = (uint32_t)regionExtent->GetChild(1)->AsUInt64();
    uint32_t regionDepth = (uint32_t)regionExtent->GetChild(2)->AsUInt64();
    uint64_t bufferOffset = region->GetChild(0)->AsUInt64();

    AccessImage(img_id, imageSubresource, regionOffset, regionExtent, imageLayout, imageAction);

    uint32_t rowLength = (uint32_t)region->GetChild(1)->AsUInt64();
    if(rowLength == 0)
      rowLength = regionWidth;

    uint32_t imageHeight = (uint32_t)region->GetChild(2)->AsUInt64();
    if(imageHeight == 0)
      imageHeight = regionHeight;
    VkFormat regionFormat = imageFormat;
    switch(aspectMask)
    {
      case VK_IMAGE_ASPECT_STENCIL_BIT: regionFormat = VK_FORMAT_S8_UINT; break;
      case VK_IMAGE_ASPECT_DEPTH_BIT:
        switch(imageFormat)
        {
          case VK_FORMAT_D16_UNORM_S8_UINT: regionFormat = VK_FORMAT_D16_UNORM; break;
          case VK_FORMAT_D32_SFLOAT_S8_UINT: regionFormat = VK_FORMAT_D32_SFLOAT; break;
          default: break;
        }
        break;
      default: break;
    }

    // rowSize = # bytes accessed per row
    uint64_t rowSize = GetByteSize(regionWidth, 1, 1, regionFormat, 0);
    // stride_y = # bytes between subsequent rows
    uint64_t stride_y = GetByteSize(rowLength, 1, 1, regionFormat, 0);
    // stride_z = # bytes between subsequent depths
    uint64_t stride_z = GetByteSize(rowLength, imageHeight, 1, regionFormat, 0);
    // stride_layer = # bytes between subsequent layers
    uint64_t stride_layer = GetByteSize(rowLength, imageHeight, regionDepth, regionFormat, 0);
    // numRows = # rows of texels accessed
    uint64_t numRows = GetByteSize(rowLength, regionHeight, 1, regionFormat, 0) / stride_y;

    // Loop over all layers, depths, and rows, marking the region of memory for that row as read or
    // written
    for(uint64_t lr = 0; lr < layerCount; lr++)
    {
      for(uint64_t z = 0; z < regionDepth; z++)
      {
        for(uint64_t y = 0; y < numRows; y++)
        {
          uint64_t rowStart = bufferOffset + lr * stride_layer + z * stride_z + y * stride_y;
          AccessBufferMemory(buf_id, rowStart, rowSize, bufferAction);
        }
      }
    }
  }
}
}    // namespace vk_cpp_codec

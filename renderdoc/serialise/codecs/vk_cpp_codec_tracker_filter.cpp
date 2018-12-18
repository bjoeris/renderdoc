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
// --------------------------------------------------------------------------
// Vulkan API specific tracking functions
// --------------------------------------------------------------------------
void TraceTracker::CreateResource(SDObject *o)
{
  if(InitResourceFind(o->GetChild(3)->AsUInt64()) != InitResourceEnd())
  {
    SDObject *ci = o->GetChild(1);
    // FindChild is used here because buffers and images have different CreateInfo structures
    SDObject *usage = ci->FindChild("usage");
    usage->UInt64() |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    usage->data.str =
        usage->AsString() + std::string("| /*rdoc:init*/ VK_IMAGE_USAGE_TRANSFER_DST_BIT");
  }
}

bool TraceTracker::CreateFramebuffer(SDObject *o)
{
  SDObject *ci = o->GetChild(1);
  SDObject *renderpass = ci->GetChild(3);
  SDObject *attachments = ci->GetChild(5);
  SDObject *framebuffer = o->GetChild(3);

  bool attachment_is_swapchain_image = false;
  for(uint64_t i = 0; i < attachments->NumChildren(); i++)
  {
    if(IsPresentationResource(attachments->GetChild(i)->AsUInt64()))
    {
      attachment_is_swapchain_image = true;
      break;
    }
  }

  bool renderpass_presents = IsPresentationResource(renderpass->AsUInt64());
  if(attachment_is_swapchain_image || renderpass_presents)
  {
    presentResources.insert(SDObjectIDMapPair(framebuffer->AsUInt64(), o));
    std::string name = code->MakeVarName(Type(framebuffer), framebuffer->AsUInt64());
    std::string acquired = name + std::string("[acquired_frame]");
    TrackVarInMap(resources, Type(framebuffer), acquired.c_str(),
                  framebuffer->AsUInt64() + PRESENT_VARIABLE_OFFSET);
    return true;
  }
  return false;
}

bool TraceTracker::CreateImageView(SDObject *o)
{
  SDObject *ci = o->GetChild(1);
  SDObject *view = o->GetChild(3);
  SDObject *image = ci->GetChild(3);

  if(IsPresentationResource(image->AsUInt64()))
  {
    // Mark these view are presentation.
    presentResources.insert(SDObjectIDMapPair(view->AsUInt64(), o));

    std::string name = code->MakeVarName(Type(view), view->AsUInt64());
    // For each view there is a 'VkImageView_<id>[acquired_frame]' which is used in the render loop.
    std::string acquired = name + std::string("[acquired_frame]");
    TrackVarInMap(resources, Type(view), acquired.c_str(), view->AsUInt64() + PRESENT_VARIABLE_OFFSET);
    return true;
  }

  return false;
}

// The purpose of this function is to track what's happening on queue submit.
// We are interested in a few things here:
// 1. If the queue submitting any command buffer that has transfered an image
// to a presentation layout. If yes, use this queue as a present queue.
// 2. Accumulate semaphore from p[Wait|Signal]Semaphores arrays. We need to\
// make sure that there are no 'waits' that are never signalled and also to
// make sure Present() waits on all signalled semaphores later.
// 3. Any Queue that submits anything needs to do a WaitIdle at the end of the
// frame in order to avoid synchronization problems. This can be optimized later.
void TraceTracker::QueueSubmit(SDObject *o)
{
  SDObject *queue = o->GetChild(0);
  // Multiple submissions can happen at the same time in Vulkan
  SDObject *submits = o->GetChild(2);
  for(uint32_t s = 0; s < submits->NumChildren(); s++)
  {
    // Multiple command buffers can be submitted at the same time
    SDObject *cmd_buffers = submits->GetChild(s)->GetChild(6);

    // Check if a command buffer is transferring an image for Presentation.
    // If it does, remember this queue as a Present Queue.
    bool is_presenting = false;
    for(uint32_t b = 0; b < cmd_buffers->NumChildren(); b++)
    {
      if(IsPresentationResource(cmd_buffers->GetChild(b)->AsUInt64()))
      {
        presentResources.insert(SDObjectIDMapPair(queue->AsUInt64(), o));
        presentQueueID = queue->AsUInt64();
        is_presenting = true;
      }
    }

    SDObject *wait = submits->GetChild(s)->GetChild(3);
    SDObject *wait_dst_stage = submits->GetChild(s)->GetChild(4);
    SDObject *signal = submits->GetChild(s)->GetChild(8);
    SDObject *wait_count = submits->GetChild(s)->GetChild(2);

    // If presenting, add a dependency on acquire_semaphore.
    if(is_presenting && signalSemaphoreIDs.find(ACQUIRE_SEMAPHORE_VAR_ID) == signalSemaphoreIDs.end())
    {
      RDCASSERT(wait_dst_stage->NumChildren() == wait->NumChildren());

      wait->data.children.push_back(acquireSemaphore);

      wait_dst_stage->data.children.push_back(makeSDEnum("$el", VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                                              ->SetCustomString("VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT")
                                              ->SetTypeName("VkPipelineStageFlagBits"));
      wait_count->UInt64()++;

      signalSemaphoreIDs.insert(U64MapPair(ACQUIRE_SEMAPHORE_VAR_ID, 1));
    }

    // Check that 'wait' semaphores have been signaled, otherwise, there will
    // be a deadlock during the execution. This includes removing acquire_semaphore
    // if for some reason it was added a second time.
    std::vector<SDObject *> remove_list;
    for(uint32_t i = 0; i < wait->NumChildren(); i++)
    {
      SDObject *semaphore = wait->GetChild(i);
      SDObject *dst_stage = wait_dst_stage->GetChild(i);
      U64MapIter find = signalSemaphoreIDs.find(semaphore->AsUInt64());

      if(find == signalSemaphoreIDs.end() || find->second == 0)
      {
        // We are waiting on this semaphore, but it was never signaled.
        remove_list.push_back(semaphore);
        remove_list.push_back(dst_stage);
        wait_count->UInt64()--;
      }
      else
      {
        find->second--;
      }
    }

    while(remove_list.size() > 0)
    {
      bool early_continue = false;
      SDObjectVec::iterator rem = remove_list.begin();
      for(uint32_t i = 0; i < wait->NumChildren(); i++)
      {
        if(wait->GetChild(i) == *rem)
        {
          remove_list.erase(rem);
          wait->data.children.erase(i);
          early_continue = true;
          break;
        }
      }
      if(early_continue)
        continue;
      for(uint32_t i = 0; i < wait_dst_stage->NumChildren(); i++)
      {
        if(wait_dst_stage->GetChild(i) == *rem)
        {
          remove_list.erase(rem);
          wait_dst_stage->data.children.erase(i);
          break;
        }
      }
    }

    // Accumulate semaphore signals to correctly evaluate 'wait' semaphores
    // will work correctly later.
    for(uint32_t i = 0; i < signal->NumChildren(); i++)
    {
      SDObject *semaphore = signal->GetChild(i);
      U64MapIter find = signalSemaphoreIDs.find(semaphore->AsUInt64());
      if(find == signalSemaphoreIDs.end())
      {
        signalSemaphoreIDs.insert(U64MapPair(semaphore->AsUInt64(), 1));
      }
      else
      {
        find->second++;
      }
    }
  }

  // Add queue to a list of submitted queues;
  submittedQueues.insert(U64MapPair(queue->AsUInt64(), queue->AsUInt64()));
}

void TraceTracker::BeginCommandBuffer(SDObject *o)
{
  SDObject *inherit = o->GetChild(1)->GetChild(3);
  if(inherit == NULL || inherit->NumChildren() == 0)
    return;

  SDObject *renderpass = inherit->GetChild(2);
  SDObject *framebuffer = inherit->GetChild(4);

  bool is_presenting_rp = IsPresentationResource(renderpass->AsUInt64());
  bool is_presenting_fb = IsPresentationResource(framebuffer->AsUInt64());
  SDObject *cmd = o->GetChild(0);

  if(is_presenting_rp || is_presenting_fb)
    presentResources.insert(SDObjectIDMapPair(cmd->AsUInt64(), o));

  if(is_presenting_fb)
  {
    framebuffer->UInt64() += PRESENT_VARIABLE_OFFSET;
  }
}

bool TraceTracker::CmdPipelineBarrierFilter(SDChunk *o)
{
  SDObject *cmd = o->GetChild(0);
  SDObject *memory_count = o->GetChild(4);
  SDObject *memory = o->GetChild(5);
  memory_count->UInt64() = memory->NumChildren();

  SDObject *buffer_count = o->GetChild(6);
  SDObject *buffer = o->GetChild(7);
  for(uint64_t i = 0; i < buffer->NumChildren();)
  {
    SDObject *resource = buffer->GetChild(i)->GetChild(6);
    if(!IsValidNonNullResouce(resource->AsUInt64()))
    {
      buffer->data.children.removeOne(buffer->GetChild(i));
    }
    else
    {
      i++;
    }
  }
  buffer_count->UInt64() = buffer->NumChildren();

  SDObject *image_count = o->GetChild(8);
  SDObject *image = o->GetChild(9);
  for(uint64_t i = 0; i < image->NumChildren();)
  {
    SDObject *resource = image->GetChild(i)->GetChild(8);
    if(IsPresentationResource(resource->AsUInt64()))
    {
      resource->UInt64() = PRESENT_IMAGE_OFFSET;
      presentResources.insert(SDObjectIDMapPair(cmd->AsUInt64(), o));
      i++;
    }
    else if(!IsValidNonNullResouce(resource->AsUInt64()))
    {
      image->data.children.removeOne(image->GetChild(i));
    }
    else
    {
      i++;
    }
  }
  image_count->UInt64() = image->NumChildren();

  return (memory->NumChildren() != 0 || buffer->NumChildren() != 0 || image->NumChildren() != 0);
}

bool TraceTracker::EventFuncFilter(SDChunk *o) {
  return !IsValidNonNullResouce(o->GetChild(1)->AsUInt64());
}

bool TraceTracker::CmdWaitEvents(SDObject *o)
{
  SDObject *event_count = o->GetChild(1);
  SDObject *events = o->GetChild(2);
  for(uint64_t i = 0; i < events->NumChildren();)
  {
    if(!IsValidNonNullResouce(events->GetChild(i)->AsUInt64()))
    {
      events->data.children.removeOne(events->GetChild(i));
    }
    else
    {
      i++;
    }
  }
  event_count->UInt64() = events->NumChildren();

  SDObject *memory_count = o->GetChild(5);
  SDObject *memory = o->GetChild(6);
  memory_count->UInt64() = memory->NumChildren();

  SDObject *buffer_count = o->GetChild(7);
  SDObject *buffer = o->GetChild(8);
  for(uint64_t i = 0; i < buffer->NumChildren();)
  {
    SDObject *resource = buffer->GetChild(i)->GetChild(6);
    if(!IsValidNonNullResouce(resource->AsUInt64()))
    {
      buffer->data.children.removeOne(buffer->GetChild(i));
    }
    else
    {
      i++;
    }
  }
  buffer_count->UInt64() = buffer->NumChildren();

  SDObject *image_count = o->GetChild(9);
  SDObject *image = o->GetChild(10);
  for(uint64_t i = 0; i < image->NumChildren();)
  {
    SDObject *resource = image->GetChild(i)->GetChild(8);
    if(!IsValidNonNullResouce(resource->AsUInt64()))
    {
      image->data.children.removeOne(image->GetChild(i));
    }
    else
    {
      i++;
    }
  }
  image_count->UInt64() = image->NumChildren();

  return (events->NumChildren() != 0 || memory->NumChildren() != 0 || buffer->NumChildren() != 0 || image->NumChildren() != 0);
}

// The purpose of this function is to do several things:
// 1. Keep track of command buffers that transfer a resource into a Present state
// 2. Figure out which image and which image view is transfered into Present state
// 3. For the image view that is transfered into Present state, find it's index
// in the swapchain
void TraceTracker::CmdBeginRenderPass(SDObject *o)
{
  SDObject *renderpass_bi = o->GetChild(1);
  SDObject *renderpass = renderpass_bi->GetChild(2);
  SDObject *framebuffer = renderpass_bi->GetChild(3);
  if(IsPresentationResource(renderpass->AsUInt64()) || IsPresentationResource(framebuffer->AsUInt64()))
  {
    // If the renderpass shows up in presentResources, framebuffer
    // must be there too.
    RDCASSERT(IsPresentationResource(framebuffer->AsUInt64()));
    framebuffer->UInt64() += PRESENT_VARIABLE_OFFSET;

    // Save the current command buffer to the list of presentation resources.
    SDObject *cmd = o->GetChild(0);
    presentResources.insert(SDObjectIDMapPair(cmd->AsUInt64(), o));
  }
}

void TraceTracker::CmdCopyImageToBufferFilter(SDChunk *o)
{
  SDObject *cmd = o->GetChild(0);
  if(IsPresentationResource(o->GetChild(1)->AsUInt64()))
  {
    presentResources.insert(SDObjectIDMapPair(cmd->AsUInt64(), o));
    o->GetChild(3)->UInt64() = PRESENT_IMAGE_OFFSET;
  }
}

void TraceTracker::CmdCopyImageFilter(SDChunk *o)
{
  SDObject *cmd = o->GetChild(0);
  if(IsPresentationResource(o->GetChild(1)->AsUInt64()) || IsPresentationResource(o->GetChild(3)->AsUInt64()))
  {
    presentResources.insert(SDObjectIDMapPair(cmd->AsUInt64(), o));
    uint32_t idx = IsPresentationResource(o->GetChild(1)->AsUInt64()) ? 1 : 3;
    o->GetChild(idx)->UInt64() = PRESENT_IMAGE_OFFSET;
  }
}

void TraceTracker::CmdBlitImageFilter(SDChunk *o)
{
  SDObject *cmd = o->GetChild(0);
  if(IsPresentationResource(o->GetChild(1)->AsUInt64()) || IsPresentationResource(o->GetChild(3)->AsUInt64()))
  {
    presentResources.insert(SDObjectIDMapPair(cmd->AsUInt64(), o));
    uint32_t idx = IsPresentationResource(o->GetChild(1)->AsUInt64()) ? 1 : 3;
    o->GetChild(idx)->UInt64() = PRESENT_IMAGE_OFFSET;
  }
}

void TraceTracker::CmdResolveImageFilter(SDChunk *o)
{
  SDObject *cmd = o->GetChild(0);
  if(IsPresentationResource(o->GetChild(1)->AsUInt64()) || IsPresentationResource(o->GetChild(3)->AsUInt64()))
  {
    presentResources.insert(SDObjectIDMapPair(cmd->AsUInt64(), o));
    uint32_t idx = IsPresentationResource(o->GetChild(1)->AsUInt64()) ? 1 : 3;
    o->GetChild(idx)->UInt64() = PRESENT_IMAGE_OFFSET;
  }
}

void TraceTracker::CreateDeviceFilter(SDChunk *o)
{
  SDObject *ci = o->GetChild(1);
  uint64_t queueCreateInfoCount = ci->FindChild("queueCreateInfoCount")->AsUInt64();
  SDObject *queueCreateInfos = ci->FindChild("pQueueCreateInfos");
  RDCASSERT(queueCreateInfoCount <= queueUsed[PhysDevID()].size());
  RDCASSERT(queueCreateInfoCount <= queueCreateInfos->NumChildren());
  for(size_t i = 0; i < queueCreateInfoCount; i++)
  {
    SDObject *queueCreateInfo = queueCreateInfos->GetChild(i);
    uint64_t &queueCount = queueCreateInfo->FindChild("queueCount")->UInt64();
    uint64_t lastUsedQueue = 0;
    for(size_t j = 0; j < queueUsed[PhysDevID()][i].size(); j++)
    {
      if(queueUsed[PhysDevID()][i][j])
      {
        lastUsedQueue = j;
      }
    }
    queueCount = lastUsedQueue + 1;
  }
}

bool TraceTracker::UpdateDescriptorSetWithTemplateFilter(SDChunk *o)
{
  SDObject *writeDescriptorSets = o->GetChild(3);
  writeDescriptorSets->name = "VkWriteDescriptorSets";
  for(uint64_t i = 0; i < writeDescriptorSets->NumChildren(); i++)
  {
    SDObject *wds = writeDescriptorSets->GetChild(i);
    wds->GetChild(2)->UInt64() =o->GetChild(1)->AsUInt64();
    if(!WriteDescriptorSetFilter(wds))
    {
      writeDescriptorSets->data.children.removeOne(wds);
      i--;
    }
  }
  return (writeDescriptorSets->NumChildren() > 0);
}

bool TraceTracker::CreateGraphicsPipelinesFilter(SDChunk *o)
{
  RDCASSERT(o->GetChild(2)->AsUInt64() == 1);    // only one pipeline gets created at a time.
  SDObject *ci = o->GetChild(3);
  SDObject *ms = ci->GetChild(10);
  if(!ms->IsNULL())
  {
    SDObject *sample_mask = ms->GetChild(6);
    if(!sample_mask->IsNULL())
    {
      SDObject *sample_mask_el = sample_mask->Duplicate();
      sample_mask->type.basetype = SDBasic::Array;
      sample_mask->data.children.push_back(sample_mask_el);
    }
  }

  // For some reason VkSpecializationMapEntry objects have 'constantID' field duplicated.
  // This removes the duplicate fields.
  SDObject *stages = ci->GetChild(4);
  for(uint64_t i = 0; i < stages->NumChildren(); i++)
  {
    SDObject *specializationInfo = stages->GetChild(i)->GetChild(6);
    if(specializationInfo->IsNULL())
    {
      continue;
    }
    SDObject *mapEntries = specializationInfo->GetChild(1);
    for(uint64_t j = 0; j < mapEntries->NumChildren(); j++)
    {
      SDObject *mapEntry = mapEntries->GetChild(j);
      if(mapEntry->NumChildren() != 3)
      {
        RDCASSERT(mapEntry->NumChildren() == 4);
        RDCASSERT(std::string(mapEntry->GetChild(0)->Name()) == "constantID");
        RDCASSERT(std::string(mapEntry->GetChild(2)->Name()) == "constantID");
        mapEntry->data.children.erase(2);
      }
    }
  }
  return true;
}

bool TraceTracker::CreateComputePipelinesFilter(SDChunk *o)
{
  RDCASSERT(o->GetChild(2)->AsUInt64() == 1);    // only one pipeline gets created at a time.
  SDObject *ci = o->GetChild(3);

  // For some reason VkSpecializationMapEntry objects have 'constantID' field duplicated.
  // This removes the duplicate fields.
  SDObject *stage = ci->GetChild(3);
  SDObject *specializationInfo = stage->GetChild(6);
  if(!specializationInfo->IsNULL())
  {
    SDObject *mapEntries = specializationInfo->GetChild(1);
    for(uint64_t j = 0; j < mapEntries->NumChildren(); j++)
    {
      SDObject *mapEntry = mapEntries->GetChild(j);
      if(mapEntry->NumChildren() != 3)
      {
        RDCASSERT(mapEntry->NumChildren() == 4);
        RDCASSERT(std::string(mapEntry->GetChild(0)->Name()) == "constantID");
        RDCASSERT(std::string(mapEntry->GetChild(2)->Name()) == "constantID");
        mapEntry->data.children.erase(2);
      }
    }
  }
  return true;
}

bool TraceTracker::CreateResourceViewFilter(SDChunk *o) {
  SDObject *ci = o->FindChild("CreateInfo");
  SDObject *pNextType = ci->FindChild("pNextType");
  if (pNextType != NULL) {
    // Since we are removing it, it's worth checking it didn't
    // have any usefull data. If it does,this code needs to be
    // improved to handle it.
    RDCASSERT(pNextType->NumChildren() == 0);
    ci->data.children.removeOne(pNextType);
  }
  return o->FindChild("View")->AsUInt64() != 0;
}

bool TraceTracker::CreateImageFilter(SDChunk *o) {
  SDObject *ci = o->FindChild("CreateInfo");
  SDObject *pNextType = ci->FindChild("pNextType");
  if(pNextType != NULL)
  {
    // Since we are removing it, it's worth checking it didn't
    // have any usefull data. If it does,this code needs to be
    // improved to handle it.
    RDCASSERT(pNextType->NumChildren() == 0);
    ci->data.children.removeOne(pNextType);
  }
  return o->FindChild("Image")->AsUInt64() != 0;
}

bool TraceTracker::ImageInfoDescSetFilter(uint64_t type, uint64_t image_id, uint64_t sampler_id,
                                          uint64_t immut_sampler_id, SDObject *layout,
                                          SDObject *descImageInfo)
{
  bool is_sampler = type == VK_DESCRIPTOR_TYPE_SAMPLER;
  bool has_sampler = is_sampler || type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bool is_sampled_image = type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bool is_presented = presentResources.find(image_id) != presentResources.end();
  if(is_presented)
    descImageInfo->GetChild(1)->UInt64() += PRESENT_VARIABLE_OFFSET;
  if(has_sampler)
  {    // If descriptorType has_sampler, it may come from immutable samples in desc set layout.
    if(!IsValidNonNullResouce(sampler_id) && !IsValidNonNullResouce(immut_sampler_id))
    {
      return false;
    }
    if(is_sampled_image && (!IsValidNonNullResouce(image_id)))
    {
      return false;
    }
  }
  else
  {
    if(!IsValidNonNullResouce(image_id))
    {
      return false;
    }
  }

  if(is_sampler)
  {
    layout->data.str = "VK_IMAGE_LAYOUT_UNDEFINED";
  }
  else
  {
    // TODO(akharlamov) I think this is not needed anymore.
    // Find and replace VkImageLayout<> entries.
    std::string result = layout->AsString();
    size_t open_bracket = result.find("<");
    size_t close_bracket = result.find(">");
    if(open_bracket != std::string::npos && close_bracket != std::string::npos)
    {
      result.replace(open_bracket, 1, "(");
      result.replace(close_bracket, 1, ")");
    }
    layout->data.str = result;
  }

  return true;
}

bool TraceTracker::BufferInfoDescSetFilter(uint64_t buffer_id, uint64_t offset, SDObject *range)
{
  if(!IsValidNonNullResouce(buffer_id))
  {
    return false;
  }
  SDObject *buffer_ci = ResourceCreateFind(buffer_id)->second.sdobj->GetChild(1);
  SDObject *buffer_size = buffer_ci->GetChild(3);
  if(range->AsUInt64() > buffer_size->AsUInt64() - offset && range->AsUInt64() != VK_WHOLE_SIZE)
  {
    RDCWARN(
        "Buffer %llu has size (%llu) and is bound with (range %llu, offset %llu). "
        "Replacing with ~0ULL",    // should I replace it with (range - offset) instead?
        buffer_id,
        buffer_size->AsUInt64(), range->AsUInt64(), offset);
    range->UInt64() =VK_WHOLE_SIZE;
    // Force it to be unsigned int type
    range->type.basetype = SDBasic::UnsignedInteger;
  }
  return true;
}

bool TraceTracker::TexelBufferViewDescSetFilter(uint64_t texelview_id)
{
  return IsValidNonNullResouce(texelview_id);
}

bool TraceTracker::WriteDescriptorSetFilter(SDObject *wds)
{
  uint64_t descriptorSet_id = wds->GetChild(2)->AsUInt64();
  if(!IsValidNonNullResouce(descriptorSet_id))
  {
    return false;
  }
  // Descriptor Set Layout Create Info aka dsLayoutCI.
  SDObject *dsLayoutCI = DescSetInfosFindLayout(descriptorSet_id)->GetChild(1);
  SDObject *dsLayoutBindings = dsLayoutCI->GetChild(4);

  SDObject *dsBinding = wds->GetChild(3);
  SDObject *dsArrayElement = wds->GetChild(4);
  SDObject *dsType = wds->GetChild(6);

  // TODO(akharlamov) is it legal to a binding # that's larger than layout binding size?
  SDObject *dsLayoutBinding = NULL;
  for(uint64_t i = 0; i < dsLayoutBindings->NumChildren(); i++)
  {
    SDObject *layoutBinding = dsLayoutBindings->GetChild(i);
    if(layoutBinding->GetChild(0)->AsUInt64() == dsBinding->AsUInt64())
    {
      dsLayoutBinding = layoutBinding;
      if(dsType->AsUInt64() != layoutBinding->GetChild(1)->AsUInt64())
      {
        RDCWARN(
            "Descriptor set binding type %s at %llu doesn't match descriptor set layout bindings "
            "type %s at %llu",
          ValueStr(dsType).c_str(), dsBinding->AsUInt64(), ValueStr(layoutBinding->GetChild(1)).c_str(),
            dsBinding->AsUInt64());
        RDCASSERT(0);    // THIS SHOULD REALLY NEVER HAPPEN!
      }
      break;
    }
  }

  if(dsLayoutBinding == NULL)
  {
    RDCWARN(
        "Descriptor set layout with binding # == %llu is not found in "
        "VkDescriptorSetLayoutCreateInfo.CreateInfo.pBindings[%llu]",
        dsBinding->AsUInt64(), dsLayoutBindings->NumChildren());
    RDCASSERT(0);    // THIS SHOULD REALLY NEVER HAPPEN!
    return false;
  }
  SDObject *dsImmutSamplers = dsLayoutBinding->GetChild(4);
  // Either there were no Immutable Samplers OR there is an immutable sampler for each element.
  RDCASSERT(dsImmutSamplers->NumChildren() == 0 || dsImmutSamplers->NumChildren() == dsLayoutBinding->GetChild(2)->AsUInt64());
  if(dsImmutSamplers->NumChildren() > 0)
    dsImmutSamplers = dsImmutSamplers->GetChild(dsArrayElement->AsUInt64());

  switch(dsType->AsUInt64())
  {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
    {    // use image info
      SDObject *images = wds->GetChild(7);
      for(uint64_t i = 0; i < images->NumChildren();)
      {
        SDObject *image = images->GetChild(i);
        if(!ImageInfoDescSetFilter(dsType->AsUInt64(), image->GetChild(1)->AsUInt64(), image->GetChild(0)->AsUInt64(),
                                   dsImmutSamplers->AsUInt64(), image->GetChild(2), image))
          images->data.children.removeOne(image);
        else
          i++;
      }
      if(images->NumChildren() == 0)
        return false;
    }
    break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
    {    // use buffer info
      SDObject *buffers = wds->GetChild(8);
      for(uint64_t i = 0; i < buffers->NumChildren();)
      {
        SDObject *buffer = buffers->GetChild(i);
        if(!BufferInfoDescSetFilter(buffer->GetChild(0)->AsUInt64(), buffer->GetChild(1)->AsUInt64(), buffer->GetChild(2)))
        {
          buffers->data.children.removeOne(buffer);
        }
        else
          i++;
      }
      if(buffers->NumChildren() == 0)
        return false;
    }
    break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    {    // use texel view info
      SDObject *texelviews = wds->GetChild(9);
      for(uint64_t i = 0; i < texelviews->NumChildren();)
      {
        SDObject *texelview = texelviews->GetChild(i);
        if(!TexelBufferViewDescSetFilter(texelview->AsUInt64()))
        {
          texelviews->data.children.removeOne(texelview);
        }
        else
          i++;
      }
      if(texelviews->NumChildren() == 0)
        return false;
    }
    break;
  }
  return true;
}

bool TraceTracker::UpdateDescriptorSetsFilter(SDChunk *o)
{
  SDObject *dsWrite = o->GetChild(2);
  for(uint64_t i = 0; i < dsWrite->NumChildren(); i++)
  {
    SDObject *wds = dsWrite->GetChild(i);
    if(!WriteDescriptorSetFilter(wds))
    {
      dsWrite->data.children.removeOne(wds);
      i--;
    }
  }

  SDObject *dsWrite_count = o->GetChild(1);
  dsWrite_count->UInt64() =dsWrite->NumChildren();

  SDObject *dsCopy = o->GetChild(4);
  for(uint64_t i = 0; i < dsCopy->NumChildren(); i++)
  {
    SDObject *cds = dsCopy->GetChild(i);
    SDObject *src_desc_set = cds->GetChild(2);
    SDObject *dst_desc_set = cds->GetChild(5);
    if(!IsValidNonNullResouce(src_desc_set->AsUInt64()) || !IsValidNonNullResouce(dst_desc_set->AsUInt64()))
    {
      dsCopy->data.children.removeOne(cds);
      i--;
    }
  }

  SDObject *dsCopy_count = o->GetChild(3);
  dsCopy_count->UInt64() =dsCopy->NumChildren();
  return (dsCopy->NumChildren() > 0) || (dsWrite->NumChildren() > 0);
}

bool TraceTracker::InitDescSetFilter(SDChunk *o)
{
  if(o->GetChild(0)->AsUInt64() != VkResourceType::eResDescriptorSet)
  {    // FilterInitDescSet only filters out invalid descriptor set updates.
    return true;
  }

  uint64_t descriptorSet_id = o->GetChild(1)->AsUInt64();
  SDObject *initBindings = o->GetChild(2);
  SDObject *dsLayoutCI = DescSetInfosFindLayout(descriptorSet_id)->GetChild(1);
  SDObject *dsLayoutBindings = dsLayoutCI->GetChild(4);

  if(initBindings->NumChildren() == 0)
    return false;

  std::vector<uint64_t> initBindingsSizes;
  for(uint64_t i = 0; i < initBindings->NumChildren(); i++)
  {
    initBindingsSizes.push_back(initBindings->GetChild(i)->NumChildren());
    RDCASSERT(initBindingsSizes[i] == 3);
  }

  struct BindingInfo
  {
    uint64_t binding;
    uint64_t type;
    uint64_t count;
    std::string typeStr;
    uint64_t index;
    bool operator<(const BindingInfo &r) const { return binding < r.binding; }
  };

  std::vector<BindingInfo> bindingInfo;
  for(uint64_t i = 0; i < dsLayoutBindings->NumChildren(); i++)
  {
    BindingInfo dsInfo;
    dsInfo.binding = dsLayoutBindings->GetChild(i)->GetChild(0)->AsUInt64();
    dsInfo.type = dsLayoutBindings->GetChild(i)->GetChild(1)->AsUInt64();
    dsInfo.count = dsLayoutBindings->GetChild(i)->GetChild(2)->AsUInt64();
    dsInfo.typeStr = dsLayoutBindings->GetChild(i)->GetChild(1)->AsString();
    dsInfo.index = i;
    bindingInfo.push_back(dsInfo);
  }
  std::sort(bindingInfo.begin(), bindingInfo.end());

  uint64_t initBindings_index = bindingInfo[0].binding;
  int lastLayoutBinding = static_cast<int>(initBindings_index);

  for(size_t i = 0; i < bindingInfo.size(); i++)
  {
    uint64_t layoutIndex = bindingInfo[i].index;
    int layoutBinding = static_cast<int>(bindingInfo[i].binding);
    RDCASSERT(layoutBinding >= lastLayoutBinding);
    // descriptor set layouts can be sparse, such that only three bindings exist
    // but they are at 0, 5 and 10. If descriptor set bindings are sparse, for
    // example 5, followed by a 10, skip '10-5' descriptor bindingInfo.
    initBindings_index += std::max(layoutBinding - lastLayoutBinding - 1, 0);
    lastLayoutBinding = layoutBinding;

    SDObject *dsLayoutBinding = dsLayoutBindings->GetChild(layoutIndex);
    for(uint64_t j = 0; j < bindingInfo[i].count; j++, initBindings_index++)
    {
      SDObject *dsImmutSamplers = dsLayoutBinding->GetChild(4);
      // Either there were no Immutable Samplers OR there is an immutable sampler for each element.
      RDCASSERT(dsImmutSamplers->NumChildren() == 0 || dsImmutSamplers->NumChildren() == bindingInfo[i].count);
      if(dsImmutSamplers->NumChildren() > 0)
        dsImmutSamplers = dsImmutSamplers->GetChild(j);

      switch(bindingInfo[i].type)
      {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        {    // use buffer info
          SDObject *buffer = initBindings->GetChild(initBindings_index)->GetChild(0);
          if(!BufferInfoDescSetFilter(buffer->GetChild(0)->AsUInt64(), buffer->GetChild(1)->AsUInt64(), buffer->GetChild(2)))
          {
            continue;
          }
        }
        break;
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        {    // use image info
          SDObject *image = initBindings->GetChild(initBindings_index)->GetChild(1);
          if(!ImageInfoDescSetFilter(bindingInfo[i].type, image->GetChild(1)->AsUInt64(), image->GetChild(0)->AsUInt64(),
                                     dsImmutSamplers->AsUInt64(), image->GetChild(2), image))
          {
            continue;
          }
        }
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        {    // use texel view info
          SDObject *texelview = initBindings->GetChild(initBindings_index)->GetChild(2);
          if(!TexelBufferViewDescSetFilter(texelview->AsUInt64()))
          {
            continue;
          }
        }
        break;
      }

      // If desc set initialization data is invalid, the loop execution won't
      // get to this point.
      SDObject *extBinding = (SDObject *)makeSDObject("binding", bindingInfo[i].binding);
      SDObject *extType = makeSDEnum("type", (uint32_t)bindingInfo[i].type)
        ->SetCustomString(bindingInfo[i].typeStr.c_str())
        ->SetTypeName("VkDescriptorType");
      SDObject *extArrayElement = makeSDObject("arrayElement", j);
      initBindings->GetChild(initBindings_index)->data.children.push_back(extBinding);
      initBindings->GetChild(initBindings_index)->data.children.push_back(extType);
      initBindings->GetChild(initBindings_index)->data.children.push_back(extArrayElement);
    }
  }

  RDCASSERT(initBindings_index == initBindings->NumChildren());

  // Now remove all elements from initBindings elements that haven't changed
  // in size as they are not used by this descriptor set.
  for(uint64_t i = 0; i < initBindings->NumChildren();)
  {
    if(initBindings->GetChild(i)->NumChildren() == initBindingsSizes[i])
    {
      initBindings->data.children.removeOne(initBindings->GetChild(i));
    }
    else
    {
      i++;
    }
  }

  return initBindings->NumChildren() > 0;
}

}    // namespace vk_cpp_codec
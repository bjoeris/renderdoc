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
void TraceTracker::EnumeratePhysicalDevicesInternal(SDChunk *o)
{
  RDCASSERT(o->NumChildren() == 9);
  uint64_t physDeviceID = o->FindChild("PhysicalDevice")->AsUInt64();
  uint64_t physDeviceIdx = o->FindChild("PhysicalDeviceIndex")->AsUInt64();
  // assert this phys device hasn't been queried yet
  RDCASSERT(indexedPhysicalDeviceID.find(physDeviceID) == indexedPhysicalDeviceID.end());
  indexedPhysicalDeviceID.emplace(physDeviceID, physDeviceIdx);

  U64UMapIter queueFamilyCountIter = physDeviceQueueFamilies.emplace(physDeviceID, o->FindChild("queueCount")->AsUInt64()).first;
  uint64_t queueFamilyCount = queueFamilyCountIter->second;

  SDObject *queueFamilyProps = o->FindChild("queueProps");
  queueUsed[physDeviceID].resize(queueFamilyCount);
  RDCASSERT(queueFamilyCount <= queueFamilyProps->NumChildren());
  for(size_t i = 0; i < queueFamilyCount; i++)
  {
    uint64_t queueCount = queueFamilyProps->GetChild(i)->FindChild("queueCount")->AsUInt64();
    queueUsed[physDeviceID][i].resize((size_t)queueCount);
  }

  queueFamilyPropertiesStr[physDeviceID] = code->MakeVarName("VkQueueFamilyProperties", physDeviceID);
}

void TraceTracker::CreateDeviceInternal(SDChunk *o)
{
  // Only allow this once.
  RDCASSERT(PhysDevID() == 0 && DeviceID() == 0);
  PhysDevID(o->GetChild(0)->AsUInt64());
  DeviceID(o->GetChild(3)->AsUInt64());
  SDObject *ci = o->GetChild(1);
  SDObject *extensions = ci->GetChild(8);
  // Reorder extensions and move VK_EXT_debug_marker to the end if needed.
  std::string debug_marker(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
  SDObject *debugMarker = NULL;
  for(uint64_t i = 0; i < extensions->NumChildren();)
  {
    if(debug_marker == extensions->GetChild(i)->AsString())
    {
      if (debugMarker == NULL)
        debugMarker = extensions->GetChild(i)->Duplicate();
      extensions->data.children.erase(i); // continue, there can be more than one!
    } else {
      i++;
    }
  }
  if(debugMarker)
    extensions->data.children.push_back(debugMarker);
  SDObject *extensionCount = ci->GetChild(7);
  extensionCount->UInt64() = extensions->NumChildren();

  acquireSemaphore = makeSDResourceId("aux.semaphore", ResourceIDGen::GetNewUniqueID())->SetTypeName("VkSemaphore");
  TrackVarInMap(resources, "VkSemaphore", "aux.semaphore", acquireSemaphore->AsUInt64());
}

void TraceTracker::GetDeviceQueueInternal(SDChunk *o)
{
  uint64_t queueFamilyIndex = o->FindChild("queueFamilyIndex")->AsUInt64();
  uint64_t queueIndex = o->FindChild("queueIndex")->AsUInt64();
  RDCASSERT(queueFamilyIndex < queueUsed[PhysDevID()].size());
  RDCASSERT(queueIndex < queueUsed[PhysDevID()][queueFamilyIndex].size());
  queueUsed[PhysDevID()][queueFamilyIndex][queueIndex] = true;
  uint64_t queue = o->FindChild("Queue")->AsUInt64();
  bool inserted = deviceQueues.insert(std::make_pair(queue, o)).second;
  RDCASSERT(inserted);
}

void TraceTracker::AllocateMemoryInternal(SDChunk *o)
{
  MemoryAllocationWithBoundResources mawbr(o);
  MemAllocAdd(o->GetChild(3)->AsUInt64(), mawbr);
}

ResourceWithViewsMapIter TraceTracker::GenericCreateResourceInternal(SDChunk *o)
{
  // Using At(3) here because many Vulkan functions that create resources have the same
  // signature, where a Vulkan resource is the 4th argument in the function call.
  RDCASSERT(o->NumChildren() >= 4 && o->GetChild(3)->IsResource());
  uint64_t resource_id = o->GetChild(3)->AsUInt64();
  ResourceWithViews rwv = {o};
  ResourceWithViewsMapIter it =
      createdResources.insert(ResourceWithViewsMapPair(resource_id, rwv)).first;

  if(o->metadata.chunkID == (uint32_t)VulkanChunk::vkCreateImage)
  {
    imageStates.insert(
        ImageStateMapPair(resource_id, ImageState(resource_id, o->FindChild("CreateInfo"))));
  }
  return it;
}

void TraceTracker::CreateResourceInternal(SDChunk *o)
{
  GenericCreateResourceInternal(o);
  SDObject *mem_reqs = o->GetChild(4);
  mem_reqs->name = code->MakeVarName("VkMemoryRequirements", o->GetChild(3)->AsUInt64());
}

void TraceTracker::CreateResourceViewInternal(SDChunk *o)
{
  SDObject *ci = o->GetChild(1);
  SDObject *resource = ci->GetChild(3);
  SDObject *view = o->GetChild(3);
  ResourceWithViewsMapIter res_it = ResourceCreateFind(resource->AsUInt64());
  if(res_it != ResourceCreateEnd())
  {    // this can fail, for example for swapchain images
    res_it->second.views.insert(SDObjectIDMapPair(view->AsUInt64(), o));
    GenericCreateResourceInternal(o);
    return;
  }
  SDObjectIDMapIter present_it = presentResources.find(resource->AsUInt64());
  if(present_it != presentResources.end())
  {
    GenericCreateResourceInternal(o);
    presentResources.insert(SDObjectIDMapPair(view->AsUInt64(), o));
    presentResources.insert(SDObjectIDMapPair(view->AsUInt64() + PRESENT_VARIABLE_OFFSET, o));
    return;
  }

  {
    RDCWARN("View %llu requires resource %llu that wasn't found in createdResource or presentResources", view->AsUInt64(), resource->AsUInt64());
  }
}

void TraceTracker::BindResourceMemoryHelper(SDChunk *o)
{
  MemAllocWithResourcesMapIter mem_it = MemAllocFind(o->GetChild(2)->AsUInt64());
  ResourceWithViewsMapIter create_it = ResourceCreateFind(o->GetChild(1)->AsUInt64());
  RDCASSERT(create_it != ResourceCreateEnd());
  BoundResource br = {/* create call SDObject    */ create_it->second.sdobj,
                      /* bind call SDObject      */ o,
                      /* serialized resource ID  */ o->GetChild(1),
                      /* serialized requirements */ ResourceCreateFindMemReqs(o->GetChild(1)->AsUInt64()),
                      /* serialized offset       */ o->GetChild(3)};
  mem_it->second.Add(br);    // Add buffer or image to the list of bound resources
}

void TraceTracker::BindBufferMemoryInternal(SDChunk *o)
{
  BindResourceMemoryHelper(o);
  uint64_t buf_id = o->GetChild(1)->AsUInt64();
  uint64_t mem_id = o->GetChild(2)->AsUInt64();
  ResourceWithViewsMapIter r_it = ResourceCreateFind(buf_id);
  if(r_it != ResourceCreateEnd())
  {
    r_it->second.views.insert(SDObjectIDMapPair(mem_id, o));
  }
}

void TraceTracker::BindImageMemoryInternal(SDChunk *o)
{
  BindResourceMemoryHelper(o);
}

void TraceTracker::CreateRenderPassInternal(SDChunk *o)
{
  GenericCreateResourceInternal(o);
  // Is this render pass presenting?
  SDObject *attachments = o->GetChild(1)->GetChild(4);
  for(uint32_t a = 0; a < attachments->NumChildren(); a++)
  {
    if(attachments->GetChild(a)->GetChild(8)->AsUInt64() == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
      SDObject *renderpass = o->GetChild(3);
      presentResources.insert(SDObjectIDMapPair(renderpass->AsUInt64(), o));
      break;
    }
  }
}

void TraceTracker::CreatePipelineLayoutInternal(SDChunk *o)
{
  GenericCreateResourceInternal(o);
}

void TraceTracker::CreateGraphicsPipelinesInternal(SDChunk *o)
{
  uint64_t createInfoCount = o->GetChild(2)->AsUInt64();
  SDObject *pipeline = o->GetChild(5);
  RDCASSERT(createInfoCount == 1);    // `createInfo` and `pipeline` are not serialized as arrays;
                                      // if this fails, figure out how renderdoc is handling that
                                      // case.
  createdPipelines.insert(SDChunkIDMapPair(pipeline->AsUInt64(), o));
}

void TraceTracker::CreateComputePipelinesInternal(SDChunk *o)
{
  uint64_t createInfoCount = o->GetChild(2)->AsUInt64();
  SDObject *pipeline = o->GetChild(5);
  RDCASSERT(createInfoCount == 1);    // `createInfo` and `pipeline` are not serialized as arrays;
                                      // if this fails, figure out how renderdoc is handling that
                                      // case.
  createdPipelines.insert(SDChunkIDMapPair(pipeline->AsUInt64(), o));
}

void TraceTracker::CreateFramebufferInternal(SDChunk *o)
{
  SDObject *ci = o->GetChild(1);
  // Add create framebuffer call to createResource map.
  ResourceWithViewsMapIter fb_it = GenericCreateResourceInternal(o);

  // add renderpass here to link framebuffer with renderpass.
  SDObject *renderpass = ci->GetChild(3);
  ResourceWithViewsMapIter rp_it = ResourceCreateFind(renderpass->AsUInt64());
  RDCASSERT(rp_it != ResourceCreateEnd());
  fb_it->second.views.insert(SDObjectIDMapPair(renderpass->AsUInt64(), rp_it->second.sdobj));

  // look at all the attachments, find view IDs, and link fb with image views
  for(uint64_t i = 0; i < ci->GetChild(5)->NumChildren(); i++)
  {
    SDObject *attach = ci->GetChild(5)->GetChild(i);
    ResourceWithViewsMapIter view_it = ResourceCreateFind(attach->AsUInt64());
    if(view_it != ResourceCreateEnd())
    {    // this can fail, for example swapchain image view isn't in the map
         // add image views that are used as attachments.
      fb_it->second.views.insert(SDObjectIDMapPair(attach->AsUInt64(), view_it->second.sdobj));
    }
  }
}

void TraceTracker::CreateSamplerInternal(SDChunk *o)
{
  GenericCreateResourceInternal(o);
}

void TraceTracker::CreateEventInternal(SDChunk *o) {
  GenericCreateResourceInternal(o);
}

void TraceTracker::CreateShaderModuleInternal(SDChunk *o)
{
  SDObject *ci = o->FindChild("CreateInfo");
  SDObject *buffer = ci->FindChild("pCode");
  buffer->data.str = GetVarFromMap(dataBlobs, "std::vector<uint8_t>", "shader", buffer->AsUInt64());
  if(dataBlobCount.find(buffer->AsUInt64()) == dataBlobCount.end())
    dataBlobCount[buffer->AsUInt64()] = 1;
  else
    dataBlobCount[buffer->AsUInt64()]++;
}

void TraceTracker::CreateSwapchainKHRInternal(SDChunk *o)
{
  RDCASSERT(swapchainID == 0);    // this should only happen once.
  SDObject *ci = o->FindChild("CreateInfo");
  swapchainCreateInfo = ci;
  SDObject *swapchain = o->FindChild("SwapChain");
  swapchainID = swapchain->AsUInt64();
  swapchainCount = ci->FindChild("minImageCount")->AsUInt64();
  SDObject *extent = ci->FindChild("imageExtent");
  swapchainWidth = extent->FindChild("width")->AsUInt64();
  swapchainHeight = extent->FindChild("height")->AsUInt64();
  presentImageIndex.resize(swapchainCount);
  swapchainCountStr = std::string("PresentImageCount_") + std::to_string(swapchain->AsUInt64());
  presentImagesStr = std::string("PresentImages_") + std::to_string(swapchain->AsUInt64());

  TrackVarInMap(resources, "VkImage", (presentImagesStr + std::string("[acquired_frame]")).c_str(),
                PRESENT_IMAGE_OFFSET);
}

void TraceTracker::GetSwapchainImagesKHRInternal(SDChunk *o)
{
  static uint32_t count = 0;
  SDObject *swapchainIdx = o->FindChild("SwapchainImageIndex");
  SDObject *image = o->FindChild("SwapchainImage");
  uint64_t imageID = image->AsUInt64();

  RDCASSERT(swapchainCount > 0 && count < swapchainCount && swapchainIdx->AsUInt64() < swapchainCount);
  // Keep track that this image with ID is at swapchain_index location
  presentImageIndex[swapchainIdx->AsUInt64()] = image;

  imageStates.insert(ImageStateMapPair(imageID, ImageState(imageID, swapchainCreateInfo)));

  // Add the image to the list of swapchain images, We'll be looking for these resources
  // in framebuffer attachments so that we can figure out which one needs to be presented.
  presentResources.insert(SDObjectIDMapPair(imageID, o));
  count++;
}

void TraceTracker::CreatePipelineCacheInternal(SDChunk *o)
{
  SDObject *ci = o->GetChild(1);
  SDObject *buffer = ci->GetChild(4);
  buffer->data.str =
      GetVarFromMap(dataBlobs, "std::vector<uint8_t>", "pipeline_cache", buffer->AsUInt64());
  if(dataBlobCount.find(buffer->AsUInt64()) == dataBlobCount.end())
    dataBlobCount[buffer->AsUInt64()] = 1;
  else
    dataBlobCount[buffer->AsUInt64()]++;
}

void TraceTracker::FlushMappedMemoryRangesInternal(SDChunk *o)
{
  fg.updates.memory.push_back(o);
  uint64_t buffer = o->GetChild(3)->AsUInt64();
  if(dataBlobCount.find(buffer) == dataBlobCount.end())
    dataBlobCount[buffer] = 2;
  else
    dataBlobCount[buffer] += 2;
}

void TraceTracker::UpdateDescriptorSetWithTemplateInternal(SDChunk *o)
{
  fg.updates.descset.push_back(o);
}

void TraceTracker::UpdateDescriptorSetsInternal(SDChunk *o)
{
  fg.updates.descset.push_back(o);
}

void TraceTracker::InitDescriptorSetInternal(SDChunk *o)
{
  uint64_t descriptorSet_id = o->GetChild(1)->AsUInt64();
  SDChunkIDMapIter it = InitResourceFind(descriptorSet_id);

  DescriptorSetInfoMapIter descriptorSet_it = descriptorSetInfos.find(descriptorSet_id);
  RDCASSERT(descriptorSet_it != descriptorSetInfos.end());
  DescriptorSetInfo &descriptorSet = descriptorSet_it->second;

  SDObject *initBindings = o->GetChild(2);
  if(initBindings->NumChildren() == 0)
    return;

  for(uint32_t i = 0; i < initBindings->NumChildren(); i++)
  {
    SDObject *initBinding = initBindings->GetChild(i);
    RDCASSERT(initBinding->NumChildren() == 6);
    uint64_t binding = initBinding->GetChild(3)->AsUInt64();
    uint64_t type = initBinding->GetChild(4)->AsUInt64();
    uint64_t element = initBinding->GetChild(5)->AsUInt64();

    switch(type)
    {
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        descriptorSet.bindings[binding].SetBindingObj(element, initBinding->GetChild(0), true);
        break;

      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        if(IsPresentationResource(initBinding->GetChild(1)->GetChild(1)->AsUInt64()))
        {
          // Desc sets that include presentation resources always have to be reset
          // because they rely on correctly setting an '[acquired_frame]' imageview.
          descriptorSet.bindings[binding].updated[element] = true;
        }
        descriptorSet.bindings[binding].SetBindingObj(element, initBinding->GetChild(1), true);
        break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        descriptorSet.bindings[binding].SetBindingObj(element, initBinding->GetChild(2), true);
        break;

      default: RDCASSERT(0);
    }
  }
}

void TraceTracker::InitialLayoutsInternal(SDChunk *o)
{
  RDCASSERT(o->metadata.chunkID == (uint32_t)SystemChunk::CaptureBegin);
  RDCASSERT(o->GetChild(0)->AsUInt64() > 0);
  for(uint64_t i = 0; i < o->FindChild("NumImages")->AsUInt64(); i++)
  {
    SaveInitialLayout(o->GetChild(i * 2 + 1), o->GetChild(i * 2 + 2));
  }
}

void TraceTracker::WriteDescriptorSetInternal(SDObject *wds)
{
  uint64_t descSet = wds->GetChild(2)->AsUInt64();
  uint64_t descSetBinding = wds->GetChild(3)->AsUInt64();

  DescriptorSetInfoMapIter descriptorSet_it = descriptorSetInfos.find(descSet);
  RDCASSERT(descriptorSet_it != descriptorSetInfos.end());
  DescriptorSetInfo &descriptorSet = descriptorSet_it->second;
  DescriptorBindingMapIter binding_it = descriptorSet.bindings.find(descSetBinding);
  uint64_t dstArrayElement = wds->GetChild(4)->AsUInt64();
  uint64_t srcArrayElement = 0;
  uint64_t descriptorCount = wds->GetChild(5)->AsUInt64();

  SDObject *srcObjs = NULL;

  switch(binding_it->second.type)
  {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: srcObjs = wds->GetChild(8); break;

    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: srcObjs = wds->GetChild(7); break;

    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: srcObjs = wds->GetChild(9); break;
    default: RDCASSERT(0);
  }
  RDCASSERT(srcObjs->NumChildren() == descriptorCount);

  while(srcArrayElement < descriptorCount)
  {
    RDCASSERT(binding_it != descriptorSet.bindings.end());
    RDCASSERT(binding_it->second.type == (VkDescriptorType)wds->GetChild(6)->AsUInt64());
    for(; srcArrayElement < descriptorCount && dstArrayElement < binding_it->second.Size();
        srcArrayElement++, dstArrayElement++)
    {
      binding_it->second.SetBindingObj(dstArrayElement, srcObjs->GetChild(srcArrayElement), false);
    }
    dstArrayElement = 0;
    binding_it++;
  }
}

void TraceTracker::CopyDescriptorSetInternal(SDObject *cds)
{
  uint64_t srcSetID = cds->GetChild(2)->AsUInt64();
  uint64_t srcBinding = cds->GetChild(3)->AsUInt64();
  uint64_t dstSetID = cds->GetChild(5)->AsUInt64();
  uint64_t dstBinding = cds->GetChild(6)->AsUInt64();

  DescriptorSetInfoMapIter srcSet_it = descriptorSetInfos.find(srcSetID);
  RDCASSERT(srcSet_it != descriptorSetInfos.end());
  DescriptorSetInfo &srcSet = srcSet_it->second;
  DescriptorBindingMapIter srcBinding_it = srcSet.bindings.find(srcBinding);

  DescriptorSetInfoMapIter dstSet_it = descriptorSetInfos.find(dstSetID);
  RDCASSERT(dstSet_it != descriptorSetInfos.end());
  DescriptorSetInfo &dstSet = dstSet_it->second;
  DescriptorBindingMapIter dstBinding_it = dstSet.bindings.find(dstBinding);
  uint64_t srcArrayElement = cds->GetChild(4)->AsUInt64();
  uint64_t dstArrayElement = cds->GetChild(7)->AsUInt64();
  uint64_t descriptorCount = cds->GetChild(8)->AsUInt64();
  while(descriptorCount > 0)
  {
    RDCASSERT(srcBinding_it != srcSet.bindings.end());
    RDCASSERT(dstBinding_it != dstSet.bindings.end());
    RDCASSERT(srcBinding_it->second.type == dstBinding_it->second.type);
    uint64_t srcSize = srcBinding_it->second.Size();
    uint64_t dstSize = dstBinding_it->second.Size();
    for(; srcArrayElement < srcSize && dstArrayElement < dstSize;
        srcArrayElement++, dstArrayElement++, descriptorCount--)
    {
      dstBinding_it->second.CopyBinding(dstArrayElement, srcBinding_it->second, srcArrayElement);
    }
    if(srcArrayElement == srcSize)
    {
      srcArrayElement = 0;
      srcBinding_it++;
    }
    if(dstArrayElement == dstSize)
    {
      dstArrayElement = 0;
      dstBinding_it++;
    }
  }
}

void TraceTracker::BeginCommandBufferInternal(SDChunk *o)
{
  CmdBufferRecord cbr = {o, o->GetChild(0)};
  fg.records.push_back(cbr);
}

void TraceTracker::QueueSubmitInternal(SDChunk *o)
{
  // On queue submit:
  // - look at all the command buffers that in this submit info.
  // - look at all the memory updates and descset that have been done
  // - add a new element to the fg.submits list:
  // -- pointers to all command buffer records from fg.records
  // -- memory_updates and descset_updates
  // -- which queue the submit happened on
  SDObject *si = o->GetChild(2);
  for(uint64_t j = 0; j < si->NumChildren(); j++)
  {
    SDObject *cb = si->GetChild(j)->GetChild(6);
    vk_cpp_codec::QueueSubmit qs = {o, o->GetChild(0)};
    uint64_t mem_updates = fg.updates.memory.size();
    uint64_t ds_updates = fg.updates.descset.size();
    for(uint64_t i = 0; i < cb->NumChildren(); i++)
    {
      qs.memory_updates = mem_updates;
      qs.descset_updates = ds_updates;
    }
    fg.AddUnorderedSubmit(qs);
  }
}

void TraceTracker::CreateDescriptorPoolInternal(SDChunk *o)
{
  GenericCreateResourceInternal(o);
}

void TraceTracker::CreateDescriptorUpdateTemplateInternal(SDChunk *o)
{
  GenericCreateResourceInternal(o);
}

void TraceTracker::CreateDescriptorSetLayoutInternal(SDChunk *o)
{
  GenericCreateResourceInternal(o);
  RDCASSERT(descSetLayouts.find(o->GetChild(3)->AsUInt64()) == descSetLayouts.end());
  descSetLayouts.insert(SDObjectIDMapPair(o->GetChild(3)->AsUInt64(), o));
}

void TraceTracker::AllocateDescriptorSetsInternal(SDChunk *o)
{
  RDCASSERT(o->NumChildren() == 3);
  SDObject *ai = o->GetChild(1);
  SDObject *ds = o->GetChild(2);
  // DescriptorSetAllocateInfo.descriptorSetCount must always be equal to '1'.
  // Descriptor set allocation can allocate multiple descriptor sets at the
  // same time, but RenderDoc splits these calls into multiple calls, one per
  // each descriptor set object that is still alive at the time of capture.
  RDCASSERT(ai->GetChild(3)->AsUInt64() == 1);
  uint64_t layout_id = ai->GetChild(4)->GetChild(0)->AsUInt64();
  SDObjectIDMapIter layout_it = descSetLayouts.find(layout_id);
  RDCASSERT(layout_it != descSetLayouts.end());
  SDObject *layout_ci = layout_it->second->GetChild(1);

  DescriptorSetInfo info = {layout_id};
  uint64_t bindingCount = layout_ci->GetChild(3)->AsUInt64();
  SDObject *bindings = layout_ci->GetChild(4);
  for(uint64_t i = 0; i < bindingCount; i++)
  {
    SDObject *bindingLayout = bindings->GetChild(i);
    uint64_t bindingNum = bindingLayout->GetChild(0)->AsUInt64();
    VkDescriptorType type = (VkDescriptorType)bindingLayout->GetChild(1)->AsUInt64();
    uint64_t descriptorCount = bindingLayout->GetChild(2)->AsUInt64();
    DescriptorBinding binding(type, descriptorCount);

    info.bindings[bindingNum] = binding;
  }
  RDCASSERT(descriptorSetInfos.insert(DescriptorSetInfoMapPair(ds->AsUInt64(), info)).second);

  ResourceWithViews rwv = {o};
  createdResources.insert(ResourceWithViewsMapPair(ds->AsUInt64(), rwv));
}

void TraceTracker::CreateCommandPoolInternal(SDChunk *o)
{
  GenericCreateResourceInternal(o);
}

void TraceTracker::AllocateCommandBuffersInternal(SDChunk *o)
{
  uint64_t& commandBufferCount = o->FindChild("AllocateInfo")->FindChild("commandBufferCount")->UInt64();
  if (commandBufferCount != 1) {
    RDCWARN("%s has AllocateInfo.commandBufferCount equal to '%llu', expected '1', setting to '1'", o->Name(),
      commandBufferCount);
    commandBufferCount = 1;
  }
  uint64_t cmdBufferPoolID = o->FindChild("AllocateInfo")->FindChild("commandPool")->AsUInt64();
  uint64_t cmdBufferID = o->FindChild("CommandBuffer")->AsUInt64();
  ResourceWithViewsMapIter cmdBufferPool = ResourceCreateFind(cmdBufferPoolID);
  RDCASSERT(cmdBufferPool != ResourceCreateEnd());
  ResourceCreateAdd(cmdBufferID, o);
  ResourceCreateAddAssociation(cmdBufferPoolID, cmdBufferID, o);
  ResourceCreateAddAssociation(cmdBufferID, cmdBufferPoolID, o);
}

void TraceTracker::InitialContentsInternal(SDChunk *o)
{
  InitResourceAdd(o->GetChild(1)->AsUInt64(), o);

  if(o->GetChild(0)->AsUInt64() == VkResourceType::eResDescriptorSet)
  {
    InitDescriptorSetInternal(o);
  }
  else
  {
    uint64_t buffer = o->FindChild("Contents")->AsUInt64();
    if(dataBlobCount.find(buffer) == dataBlobCount.end())
      dataBlobCount[buffer] = 1;
    else
      dataBlobCount[buffer]++;
  }
}

}    // namespace vk_cpp_codec

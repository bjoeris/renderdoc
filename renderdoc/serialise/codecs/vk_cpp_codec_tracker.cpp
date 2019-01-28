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

#include <unordered_map>
namespace vk_cpp_codec
{
std::unordered_map<uint64_t, std::string> VkImageLayoutStrings = {
    {(uint64_t)VK_IMAGE_LAYOUT_UNDEFINED, "VK_IMAGE_LAYOUT_UNDEFINED"},
    {(uint64_t)VK_IMAGE_LAYOUT_GENERAL, "VK_IMAGE_LAYOUT_GENERAL"},
    {(uint64_t)VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
     "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL"},
    {(uint64_t)VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL"},
    {(uint64_t)VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
     "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL"},
    {(uint64_t)VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
     "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL"},
    {(uint64_t)VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL"},
    {(uint64_t)VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL"},
    {(uint64_t)VK_IMAGE_LAYOUT_PREINITIALIZED, "VK_IMAGE_LAYOUT_PREINITIALIZED"},
    {(uint64_t)VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL"},
    {(uint64_t)VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL"},
    {(uint64_t)VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR"},
    {(uint64_t)VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR"},
    {(uint64_t)VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR,
     "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR"},
    {(uint64_t)VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR,
     "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR"},
};

// ----------------------------------------------------------------------------
// The family of functions below manages the variable maps in various way.
// ----------------------------------------------------------------------------

#ifdef TT_VK_CALL_ANALYZE_SWITCH
#undef TT_VK_CALL_ANALYZE_SWITCH
#endif
#define TT_VK_CALL_ANALYZE_SWITCH(call, arg) \
  case(uint32_t)VulkanChunk::vk##call: call##Analyze(arg); break;

std::string TraceTracker::GetVkDebugObjectFromString(const char *type)
{
  if(strstr(type, "VkInstance") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT";
  if(strstr(type, "VkPhysicalDevice") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT";
  if(strstr(type, "VkDevice") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT";
  if(strstr(type, "VkQueue") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT";
  if(strstr(type, "VkSemaphore") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT";
  if(strstr(type, "VkCommandBuffer") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT";
  if(strstr(type, "VkFence") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT";
  if(strstr(type, "VkDeviceMemory") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT";
  // VkBufferView and VkImageView are checked ahead of VkBuffer and VkImage.
  if(strstr(type, "VkBufferView") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT";
  if(strstr(type, "VkImageView") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT";
  if(strstr(type, "VkBuffer") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT";
  if(strstr(type, "VkImage") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT";
  if(strstr(type, "VkEvent") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT";
  if(strstr(type, "VkQueryPool") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT";
  if(strstr(type, "VkShaderModule") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT";
  if(strstr(type, "VkPipelineCache") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT";
  if(strstr(type, "VkPipelineLayout") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT";
  if(strstr(type, "VkRenderPass") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT";
  if(strstr(type, "VkPipeline") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT";
  if(strstr(type, "VkDescriptorSetLayout") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT";
  if(strstr(type, "VkSampler") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT";
  if(strstr(type, "VkDescriptorPool") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT";
  if(strstr(type, "VkDescriptorSet") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT";
  if(strstr(type, "VkFramebuffer") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT";
  if(strstr(type, "VkCommandPool") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT";
  if(strstr(type, "VkSurface") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT";
  if(strstr(type, "VkSwapchainKHR") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT";
  if(strstr(type, "VkDebugCallbackEXT") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT";
  if(strstr(type, "VkDescriptorUpdateTemplate") != NULL)
    return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT";

  return "VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT";
}

const char *TraceTracker::GetVarFromMap(VariableIDMap &m, uint64_t id, const char *type,
                                        const char *name, std::string dt)
{
  VariableIDMapIter it = m.find(id);
  if(it != m.end())
    return it->second.name.c_str();
  // If resource id isn't found that means it wasn't declared in variable file
  // either, so print that variable out.
  it = m.insert(VariableIDMapPair(id, Variable(type, name, dt))).first;
  code->AddNamedVar(type, name);
  return it->second.name.c_str();
}

const char *TraceTracker::GetVarFromMap(VariableIDMap &m, const char *type, const char *name,
                                        uint64_t id)
{
  std::string dt = GetVkDebugObjectFromString(type);
  std::string full_name = std::string(name) + std::string("_") + std::to_string(id);
  return GetVarFromMap(m, id, type, full_name.c_str(), dt);
}

const char *TraceTracker::GetVarFromMap(VariableIDMap &m, uint64_t id, std::string map_name)
{
  VariableIDMapIter it = m.find(id);
  if(it != m.end())
    return it->second.name.c_str();
  else
  {
    if(id == 0)
      return "NULL";    // This is reasonable, a resource can be NULL sometimes.
    // The serialized frame references this resource, but it was never created.
    RDCLOG("%llu is not found in %s map", id, map_name.c_str());
    // Return 'nullptr' specifically to differentiate between valid NULL
    // resource and a missing resources.
    return "nullptr";
  }
}

void TraceTracker::TrackVarInMap(VariableIDMap &m, const char *type, const char *name, uint64_t id)
{
  RDCASSERT(m.find(id) == m.end());
  m.insert(VariableIDMapPair(id, Variable(type, name)));
}

VariableIDMapIter TraceTracker::GetResourceVarIt(uint64_t id)
{
  return resources.find(id);
}

// Get a resource name from a resource map using the ID.
const char *TraceTracker::GetResourceVar(uint64_t id)
{
  return GetVarFromMap(resources, id, "resource variables");
}

// Get a resource name from the resources Map using the ID if it was already added.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: type name;
const char *TraceTracker::GetResourceVar(uint64_t id, const char *type, const char *name)
{
  return GetVarFromMap(resources, type, name, id);
}

// Get a resource name from a resources Map using the ID if it was already added.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: type name_id;
const char *TraceTracker::GetResourceVar(const char *type, const char *name, uint64_t id)
{
  return GetVarFromMap(resources, type, name, id);
}

// Get a resource name from a resources Map using the ID if it was already added.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: type type_id;
const char *TraceTracker::GetResourceVar(const char *type, uint64_t id)
{
  return GetVarFromMap(resources, type, type, id);
}

// Get a VkMemoryAllocateInfo variable name from a memAllocInfoMap using the ID.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: VkMemoryAllocateInfo VkMemoryAllocateInfo_id;
const char *TraceTracker::GetMemAllocInfoVar(uint64_t id, bool create)
{
  if(create)
    return GetVarFromMap(memAllocInfos, "VkMemoryAllocateInfo", "VkMemoryAllocateInfo", id);
  else
    return GetVarFromMap(memAllocInfos, id, "memory allocations");
}

const char *TraceTracker::GetDataBlobVar(uint64_t id)
{
  return GetVarFromMap(dataBlobs, "std::vector<uint8_t>", "buffer", id);
}

// Get a MemoryRemapVec variable name from a remapMap using the ID.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: MemoryRemapVec Remap_id;
const char *TraceTracker::GetMemRemapVar(uint64_t id)
{
  return GetVarFromMap(remapMap, "MemoryRemapVec", "Remap", id);
}

// Get a VkDeviceSize variable name from a resetSizeMap using the ID.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: VkDeviceSize ResetSize_id;
const char *TraceTracker::GetMemResetSizeVar(uint64_t id)
{
  return GetVarFromMap(resetSizeMap, "VkDeviceSize", "ResetSize", id);
}

// Get a VkDeviceSize variable name from a resetSizeMap using the ID.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: VkDeviceSize InitSize_id;
const char *TraceTracker::GetMemInitSizeVar(uint64_t id)
{
  return GetVarFromMap(initSizeMap, "VkDeviceSize", "InitSize", id);
}

// Get a ReplayedMemoryBindOffset variable name from a replayMemBindOffsetMap using the ID.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: VkDeviceSize ReplayedMemoryBindOffset_id;
const char *TraceTracker::GetReplayBindOffsetVar(uint64_t id)
{
  return GetVarFromMap(replayMemBindOffsets, "VkDeviceSize", "ReplayedMemoryBindOffset", id);
}

// Get a CapturedMemoryBindOffset variable name from a captureMemBindOffsetMap using the ID.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: VkDeviceSize CapturedMemoryBindOffset_id;
const char *TraceTracker::GetCaptureBindOffsetVar(uint64_t id)
{
  return GetVarFromMap(captureMemBindOffsets, "VkDeviceSize", "CapturedMemoryBindOffset", id);
}

// Get a VkMemoryRequirements variable name from a memRequirementsMap using the ID.
// If it's a new variable, add it to the map and also print it to VAR files.
// The resulting variable will look like this: VkMemoryRequirements VkMemoryRequirements_id;
const char *TraceTracker::GetMemReqsVar(uint64_t id)
{
  return GetVarFromMap(memRequirements, "VkMemoryRequirements", "VkMemoryRequirements", id);
}

SDObject *TraceTracker::DescSetInfosFindLayout(uint64_t descSet_id)
{
  DescriptorSetInfoMapIter it = descriptorSetInfos.find(descSet_id);
  RDCASSERT(it != descriptorSetInfos.end());
  ResourceWithViewsMapIter res_it = ResourceCreateFind(it->second.layout);
  RDCASSERT(res_it != ResourceCreateEnd());
  return res_it->second.sdobj;
}

// SDObject of an array type will have elements that all have the same name "$el".
// This is not informative for the code generation and also C/C++ doesn't allow
// names to start with $. To fix this, I create a duplicate and replace the name,
// with the parent's name + array index and I serialize the duplicate instead.
// The duplicates are stored in a 'copies' array and have to be manually cleaned up.
SDObject *TraceTracker::CopiesAdd(SDObject *o, uint64_t i, uint64_t j, std::string &suffix)
{
  SDObject *node = o->GetChild(i);
  if(node->name == "$el")
  {
    node = o->GetChild(i)->Duplicate();
    suffix = std::string("_") + std::to_string(j);
    node->name = o->Name();    // +suffix;
    copies.push_back(node);
  }
  return node;
}
void TraceTracker::CopiesClear()
{
  for(uint32_t i = 0; i < copies.size(); i++)
    delete copies[i];
  copies.clear();
}

bool TraceTracker::IsValidNonNullResouce(uint64_t id)
{
  bool variable_found = resources.find(id) != resources.end();
  bool resource_created = createdResources.find(id) != createdResources.end();
  return (id != 0 && (variable_found || resource_created));
}

bool TraceTracker::IsPresentationResource(uint64_t id)
{
  return presentResources.find(id) != presentResources.end();
}

SDObject *TraceTracker::FramebufferPresentView(SDObject *o)
{
  SDObject *ci = o->GetChild(1);
  for(uint64_t i = 0; i < ci->GetChild(5)->NumChildren(); i++)
  {
    if(IsPresentationResource(ci->GetChild(5)->GetChild(i)->AsUInt64()))
    {
      return ci->GetChild(5)->GetChild(i);
    }
  }
  return NULL;
}

// --------------------------------------------------------------------------
// Vulkan API specific tracking functions called on ScanSerializedInfo to
// track resource state across the frame
// --------------------------------------------------------------------------
void TraceTracker::ApplyMemoryUpdate(SDChunk *ext)
{
  RDCASSERT(ext->metadata.chunkID == (uint32_t)VulkanChunk::vkFlushMappedMemoryRanges);

  SDObject *range = ext->FindChild("MemRange");
  SDObject *memory = range->FindChild("memory");
  uint64_t offset = range->FindChild("offset")->AsUInt64();
  uint64_t size = range->FindChild("size")->AsUInt64();

  MemAllocWithResourcesMapIter it = MemAllocFind(memory->AsUInt64());
  it->second.Access(VK_QUEUE_FAMILY_IGNORED, VK_SHARING_MODE_CONCURRENT, ACCESS_ACTION_CLEAR,
                    offset, size);
}

void TraceTracker::ApplyDescSetUpdate(SDChunk *ext)
{
  SDObject *descriptorWrites = NULL;
  if(ext->metadata.chunkID == (uint32_t)VulkanChunk::vkUpdateDescriptorSets)
    descriptorWrites = ext->GetChild(2);
  if(ext->metadata.chunkID == (uint32_t)VulkanChunk::vkUpdateDescriptorSetWithTemplate)
    descriptorWrites = ext->GetChild(3);
  RDCASSERT(descriptorWrites != NULL);
  for(uint64_t i = 0; i < descriptorWrites->NumChildren(); i++)
  {
    WriteDescriptorSetInternal(descriptorWrites->GetChild(i));
  }

  if(ext->metadata.chunkID == (uint32_t)VulkanChunk::vkUpdateDescriptorSets)
  {
    SDObject *descriptorCopies = ext->GetChild(4);
    for(uint64_t i = 0; i < descriptorCopies->NumChildren(); i++)
    {
      CopyDescriptorSetInternal(descriptorCopies->GetChild(i));
    }
  }
}

void TraceTracker::AddCommandBufferToFrameGraph(SDChunk *o)
{
  uint32_t i = fg.FindCmdBufferIndex(o->GetChild(0));
  fg.records[i].cmds.push_back(o);
}

void TraceTracker::AnalyzeCmd(SDChunk *chunk)
{
  switch(chunk->metadata.chunkID)
  {
    // Image & buffer related functions
    TT_VK_CALL_ANALYZE_SWITCH(CmdBeginRenderPass, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdNextSubpass, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdExecuteCommands, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdEndRenderPass, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdCopyImage, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdBlitImage, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdResolveImage, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdClearColorImage, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdClearDepthStencilImage, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdClearAttachments, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdCopyBufferToImage, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdCopyImageToBuffer, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdPipelineBarrier, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdWaitEvents, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdBindDescriptorSets, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdBindIndexBuffer, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdBindVertexBuffers, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdCopyBuffer, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdUpdateBuffer, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdFillBuffer, chunk);
    // Draw & dispatch related functions
    TT_VK_CALL_ANALYZE_SWITCH(CmdDrawIndirect, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdDrawIndexedIndirect, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdDraw, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdDrawIndexed, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdBindPipeline, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdDispatch, chunk);
    TT_VK_CALL_ANALYZE_SWITCH(CmdDispatchIndirect, chunk);

    default:
      // Make sure we are actually handling all the commands that get sent here by
      // `AddCommandBufferToFrameGraph`.
      RDCERR("Unrecognized chunk (name, ID) (%s %" PRIu64 ")", chunk->Name(),
             chunk->metadata.chunkID);
  }
}

void TraceTracker::SaveInitialLayout(SDObject *image, SDObject *layouts)
{
  RDCASSERT(image != NULL && layouts != NULL);
  uint64_t imageID = image->AsUInt64();

  ImageStateMapIter imageState_it = imageStates.find(imageID);
  if(imageState_it == imageStates.end())
  {
    // Apparently, RenderDoc's "Beginning of Capture" chunk can include images
    // that don't have corresponding vkCreateImage or vkGetSwapchainImages chunks.
    return;
  }
  ImageState &imageState = imageState_it->second;

  SDObject *subresources = layouts->FindChild("subresourceStates");
  uint64_t queueFamily = layouts->FindChild("queueFamilyIndex") != NULL
                             ? layouts->FindChild("queueFamilyIndex")->AsUInt64()
                             : VK_QUEUE_FAMILY_IGNORED;

  for(uint64_t i = 0; i < subresources->NumChildren(); i++)
  {
    SDObject *subres = subresources->GetChild(i);
    SDObject *range = subres->FindChild("subresourceRange");
    uint64_t baseMip = range->FindChild("baseMipLevel")->AsUInt64();
    uint64_t levelCount = range->FindChild("levelCount")->AsUInt64();
    uint64_t baseLayer = range->FindChild("baseArrayLayer")->AsUInt64();
    uint64_t layerCount = range->FindChild("layerCount")->AsUInt64();
    VkImageAspectFlags aspectMask = (VkImageAspectFlags)range->FindChild("aspectMask")->AsUInt64();
    VkImageLayout layout = (VkImageLayout)subres->FindChild("newLayout")->AsUInt64();
    uint64_t dstQueueFamily =
        subres->FindChild("dstQueueFamilyIndex") != NULL
            ? (VkImageLayout)subres->FindChild("dstQueueFamilyIndex")->AsUInt64()
            : VK_QUEUE_FAMILY_IGNORED;
    ImageSubresourceRange imageRange =
        imageState.Range(aspectMask, baseMip, levelCount, baseLayer, layerCount);
    for(ImageSubresourceRangeIter res_it = imageRange.begin(); res_it != imageRange.end(); res_it++)
    {
      if(dstQueueFamily != VK_QUEUE_FAMILY_IGNORED)
      {
        // There are queue family indexes stored in both `layouts` (for the whole image) and in each
        // subresource.
        // So far, the queue family for subresources is always VK_QUEUE_FAMILY_IGNORED. If this is
        // ever not true, we need to understand what is happening.
        RDCWARN(
            "BeginCapture includes an image subresource with a dstQueueFamilyIndex. This is "
            "completely untested. Please let us know what breaks "
            "(with a capture that reproduces it, if possible).");
      }
      imageState.At(*res_it).Initialize(layout, queueFamily);
    }
  }
}

bool TraceTracker::ResourceNeedsReset(uint64_t resourceID, bool forInit, bool forReset)
{
  if(!(forInit || forReset))
  {
    return false;
  }
  SDChunkIDMapIter init_res_it = InitResourceFind(resourceID);
  if(init_res_it == InitResourceEnd())
  {
    // Nothing to reset the resource to. Assume we don't need to reset.
    return false;
  }

  switch(init_res_it->second->GetChild(0)->AsUInt64())
  {
    case VkResourceType::eResDeviceMemory:
    {
      MemAllocWithResourcesMapIter mem_it = MemAllocFind(resourceID);
      return (forInit && mem_it->second.NeedsInit()) || (forReset && mem_it->second.NeedsReset());
    }
    case VkResourceType::eResImage:
    {
      if(forInit && ((optimizations & CODE_GEN_OPT_IMAGE_INIT_BIT) == 0))
      {
        return true;
      }
      if(forReset && ((optimizations & CODE_GEN_OPT_IMAGE_RESET_BIT) == 0))
      {
        return true;
      }
      ResourceWithViewsMapIter create_it = ResourceCreateFind(resourceID);
      if(create_it == ResourceCreateEnd())
      {
        RDCASSERT(0);    // TODO: should this be happening?
        return true;
      }
      bool needsReset = false;
      bool needsInit = false;

      ImageStateMapIter imageState_it = imageStates.find(resourceID);
      RDCASSERT(imageState_it != imageStates.end());
      ImageState &imageState = imageState_it->second;

      for(ImageSubresourceStateMapIter it = imageState.begin(); it != imageState.end(); it++)
      {
        switch(it->second.AccessState())
        {
          case ACCESS_STATE_READ:
            // some part of the initial value could be read, so initialization is required
            needsInit = true;
            break;
          case ACCESS_STATE_RESET:
            // some part of the initial value could be read, and then written, so reset is required
            needsReset = true;
            break;
          default: break;
        }
      }

      // If the image is reset, it is redundant to also initialize
      needsInit = needsInit && (!needsReset);

      return ((forInit && needsInit) || (forReset && needsReset));
    }
    default: RDCASSERT(0); return true;
  }
}

}    // namespace vk_cpp_codec

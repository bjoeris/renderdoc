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
#pragma once

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "common/common.h"
#include "core/core.h"
#if defined(WIN32)
#define VK_USE_PLATFORM_WIN32_KHR 1
#elif defined(__ggp__)
#define VK_USE_PLATFORM_GGP 1
#elif defined(__yeti__)
#define VK_USE_PLATFORM_YETI_GOOGLE 1
#else
#define VK_USE_PLATFORM_XLIB_KHR 1
#endif
#include "driver/vulkan/vk_common.h"
#include "driver/vulkan/vk_resources.h"
#if defined(WIN32)
#undef VK_USE_PLATFORM_WIN32_KHR
#elif defined(__ggp__)
#undef VK_USE_PLATFORM_GGP
#elif defined(__yeti__)
#undef VK_USE_PLATFORM_YETI_GOOGLE
#else
#undef VK_USE_PLATFORM_XLIB_KHR
#endif
#include "serialise/rdcfile.h"
#include "vk_cpp_codec_common.h"
#include "vk_cpp_codec_file.h"
#include "vk_cpp_codec_state.h"
#include "vk_cpp_codec_tracker.h"

#pragma push_macro("GenericEvent")
#ifdef GenericEvent
#undef GenericEvent
#endif

namespace vk_cpp_codec
{
class TraceTracker;

struct TemplateFileDesc
{
  const char *filename;
  size_t size;
  const char *contents;
};

class CodeWriter
{
  friend TraceTracker;

  static TemplateFileDesc TemplateFiles[];

public:
  enum IDs
  {
    ID_MAIN,
    ID_VAR,
    ID_RENDER,
    ID_CREATE,
    ID_RELEASE,
    ID_INIT,
    ID_PRERESET,
    ID_POSTRESET,
    ID_PREDIFF,
    ID_DIFF,

    ID_COUNT,
  };
  const char *shimPrefix = "";

protected:
  typedef std::array<std::string, ID_COUNT> func_array;
  func_array funcs = {{"main", "variables", "render", "create", "release", "init", "prereset",
                       "postreset", "prediff", "diff"}};

  std::string rootDirectory;
  typedef std::array<CodeFile *, ID_COUNT> file_array;
  file_array files;

  TraceTracker *tracker = NULL;

  // Code project doesn't allow multiple calls to 'Open'.
  // Once you create a code project you get all the files you need.
  void Open();

  void WriteTemplateFile(const char *file, const char *contents, size_t size);

  const uint32_t kLinearizeMemory = 0;

  void RemapMemAlloc(uint32_t pass, MemAllocWithResourcesMapIter alloc_it);

  void EarlyCreateResource(uint32_t pass);
  void EarlyAllocateMemory(uint32_t pass);
  void EarlyBindResourceMemory(uint32_t pass);

  // This vulkan functions are not allowed to be called directly by
  // codec.cpp.
  void CreateImage(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateBuffer(SDObject *o, uint32_t pass, bool global_ci = false);
  void AllocateMemory(SDObject *o, uint32_t pass);
  void BindResourceMemory(SDObject *o, uint32_t pass);

  void GenericCreatePipelines(SDObject *o, uint32_t pass, bool global_ci = false);
  void GenericEvent(SDObject *o, uint32_t pass);
  void GenericVkCreate(SDObject *o, uint32_t pass, bool global_ci = false);
  void GenericWaitIdle(SDObject *o, uint32_t pass);
  void GenericCmdSetRectTest(SDObject *o, uint32_t pass);
  void GenericCmdSetStencilParam(SDObject *o, uint32_t pass);
  void GenericCmdEvent(SDObject *o, uint32_t pass);
  void GenericCmdDrawIndirect(SDObject *o, uint32_t pass);

  void CreateAuxResources(SDObject *o, uint32_t pass, bool global_ci = false);
  void HandleMemoryAllocationAndResourceCreation(uint32_t pass);
  void BufferOrImageMemoryReqs(SDObject *o, const char *get_mem_req_func, uint32_t pass);

  void InlineVariable(SDObject *o, uint32_t pass);
  void AssignUnion(std::string path, SDObject *o, bool comment, uint32_t pass);
  void LocalVariable(SDObject *o, std::string suffix, uint32_t pass);

  void InitSrcBuffer(SDObject *o, uint32_t pass);
  void InitDstBuffer(SDObject *o, uint32_t pass);
  void InitDescSet(SDObject *o);
  void ImagePreDiff(SDObject *o, uint32_t pass);
  void ImageDiff(SDObject *o, uint32_t pass);
  void CopyResetImage(SDObject *o, uint32_t pass);
  void CopyResetBuffer(SDObject *o, uint32_t pass);
  void ImageLayoutTransition(uint64_t image_id, SDObject *subres, const char *old_layout,
                             uint32_t pass);
  void ClearBufferData();

public:
  CodeWriter(std::string path) : rootDirectory{path}
  {
    if(strcmp(RENDERDOC_GetConfigSetting("shim"), "true") == 0)
    {
      shimPrefix = "shim_";
    }
    Open();
  }
  ~CodeWriter() { Close(); }
  // Closing the code project also closes all of the code files. In
  // case of a 'MAIN' code files, it get's fully created at the very
  // end of code generation.
  void Close();

  void Set(TraceTracker *ptr)
  {
    tracker = ptr;
    RDCASSERT(tracker != NULL);
    tracker->Set(this);
  }

  void MultiPartSplit()
  {
    for(uint32_t i = ID_RENDER; i < ID_COUNT; i++)
    {
      static_cast<MultiPartCodeFile *>(files[i])->MultiPartSplit();
    }
  }

  std::string MakeVarName(const char *name1, const char *name2)
  {
    std::string full_name = std::string(name1) + std::string("_") + std::string(name2);
    return full_name;
  }

  std::string MakeVarName(const char *name, uint64_t id)
  {
    std::string full_name = std::string(name) + std::string("_") + std::to_string(id);
    return full_name;
  }

  // Add a global variable of a given type into the VAR files. Just use the
  // provided name as the full name for the variable.
  std::string AddNamedVar(const char *type, const char *name)
  {
    files[ID_VAR]->PrintLnH("extern %s %s;", type, name).PrintLn("%s %s;", type, name);
    return std::string(name);
  }

  // Add a global variable of a given type into the VAR files. Concatenate the 'type'
  // and the 'name' to get a 'full' variable name. For example 'VkDevice' and
  // 'captured' will produce 'VkDevice_captured'.
  std::string AddVar(const char *type, const char *name)
  {
    std::string full_name = MakeVarName(type, name);
    return AddNamedVar(type, full_name.c_str());
  }

  // Add a global variable of a given type into the VAR files.
  // This call is used for more complicated variable declarations, such as
  // std::vector<VkDevice> VkDevice_1;
  std::string AddVar(const char *type, const char *name, uint64_t id)
  {
    std::string full_name = MakeVarName(name, id);
    return AddNamedVar(type, full_name.c_str());
  }

  // Add a global variable of a given type into the VAR files.
  // For simple variable declarations, such as VkDevice VkDevice_1;
  std::string AddVar(const char *type, uint64_t id) { return AddVar(type, type, id); }
  // --------------------------------------------------------------------------
  void Resolution(uint32_t pass);

  void EnumeratePhysicalDevices(SDObject *o, uint32_t pass);
  void GetDeviceQueue(SDObject *o, uint32_t pass);
  void GetSwapchainImagesKHR(SDObject *o, uint32_t pass);

  void CreateInstance(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreatePresentFramebuffer(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreatePresentImageView(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateDescriptorPool(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateCommandPool(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateFramebuffer(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateRenderPass(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateSemaphore(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateFence(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateEvent(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateQueryPool(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateDescriptorSetLayout(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateDescriptorUpdateTemplate(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateImageView(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateSampler(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateShaderModule(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreatePipelineLayout(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreatePipelineCache(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateBufferView(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateSwapchainKHR(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateGraphicsPipelines(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateComputePipelines(SDObject *o, uint32_t pass, bool global_ci = false);
  void CreateDevice(SDObject *o, uint32_t pass, bool global_ci = false);

  void AllocateCommandBuffers(SDObject *o, uint32_t pass);
  void AllocateDescriptorSets(SDObject *o, uint32_t pass);

  void FlushMappedMemoryRanges(SDObject *o, uint32_t pass);
  void UnmapMemory(SDObject *o, uint32_t pass);
  void AcquireNextImage(SDObject *o, uint32_t pass);
  void BeginCommandBuffer(SDObject *o, uint32_t pass);
  void EndCommandBuffer(SDObject *o, uint32_t pass);
  void WaitForFences(SDObject *o, uint32_t pass);
  void GetFenceStatus(SDObject *o, uint32_t pass);
  void ResetFences(SDObject *o, uint32_t pass);
  void GetEventStatus(SDObject *o, uint32_t pass);
  void SetEvent(SDObject *o, uint32_t pass);
  void ResetEvent(SDObject *o, uint32_t pass);
  void QueueSubmit(SDObject *o, uint32_t pass);
  void QueueWaitIdle(SDObject *o, uint32_t pass);
  void DeviceWaitIdle(SDObject *o, uint32_t pass);
  void UpdateDescriptorSets(SDObject *o, uint32_t pass);
  void UpdateDescriptorSetWithTemplate(SDObject *o, uint32_t pass);

  // Command recording API calls
  void CmdBeginRenderPass(SDObject *o, uint32_t pass);
  void CmdNextSubpass(SDObject *o, uint32_t pass);
  void CmdExecuteCommands(SDObject *o, uint32_t pass);
  void CmdEndRenderPass(SDObject *o, uint32_t pass);
  void CmdSetViewport(SDObject *o, uint32_t pass);
  void CmdSetScissor(SDObject *o, uint32_t pass);
  void CmdBindDescriptorSets(SDObject *o, uint32_t pass);
  void CmdBindPipeline(SDObject *o, uint32_t pass);
  void CmdBindVertexBuffers(SDObject *o, uint32_t pass);
  void CmdBindIndexBuffer(SDObject *o, uint32_t pass);
  void CmdDraw(SDObject *o, uint32_t pass);
  void CmdDrawIndirect(SDObject *o, uint32_t pass);
  void CmdDrawIndexed(SDObject *o, uint32_t pass);
  void CmdDrawIndexedIndirect(SDObject *o, uint32_t pass);
  void CmdDispatch(SDObject *o, uint32_t pass);
  void CmdDispatchIndirect(SDObject *o, uint32_t pass);
  void CmdSetEvent(SDObject *o, uint32_t pass);
  void CmdResetEvent(SDObject *o, uint32_t pass);
  void CmdWaitEvents(SDObject *o, uint32_t pass);
  void CmdPipelineBarrier(SDObject *o, uint32_t pass);
  void CmdPushConstants(SDObject *o, uint32_t pass);
  void CmdSetDepthBias(SDObject *o, uint32_t pass);
  void CmdSetDepthBounds(SDObject *o, uint32_t pass);
  void CmdSetStencilCompareMask(SDObject *o, uint32_t pass);
  void CmdSetStencilWriteMask(SDObject *o, uint32_t pass);
  void CmdSetStencilReference(SDObject *o, uint32_t pass);
  void CmdSetLineWidth(SDObject *o, uint32_t pass);
  void CmdCopyBuffer(SDObject *o, uint32_t pass);
  void CmdUpdateBuffer(SDObject *o, uint32_t pass);
  void CmdFillBuffer(SDObject *o, uint32_t pass);
  void CmdCopyImage(SDObject *o, uint32_t pass);
  void CmdBlitImage(SDObject *o, uint32_t pass);
  void CmdResolveImage(SDObject *o, uint32_t pass);
  void CmdSetBlendConstants(SDObject *o, uint32_t pass);
  void CmdCopyBufferToImage(SDObject *o, uint32_t pass);
  void CmdCopyImageToBuffer(SDObject *o, uint32_t pass);
  void CmdClearAttachments(SDObject *o, uint32_t pass);
  void CmdClearDepthStencilImage(SDObject *o, uint32_t pass);
  void CmdClearColorImage(SDObject *o, uint32_t pass);
  void CmdDebugMarkerBeginEXT(SDObject *o, uint32_t pass);
  void CmdDebugMarkerInsertEXT(SDObject *o, uint32_t pass);
  void CmdDebugMarkerEndEXT(SDObject *o, uint32_t pass);
  void DebugMarkerSetObjectNameEXT(SDObject *o, uint32_t pass);
  void EndFramePresent(SDObject *o, uint32_t pass);
  void EndFrameWaitIdle(SDObject *o, uint32_t pass);
  void InitialContents(SDObject *o);
  void InitialLayouts(SDChunk *o, uint32_t pass);

  void PrintReadBuffers(StructuredBufferList &buffers);
  void ReleaseResources();
};

}    // namespace vk_cpp_codec

#pragma pop_macro("GenericEvent")

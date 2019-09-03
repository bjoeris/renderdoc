/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Google LLC
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

#include "cpp_codec_transformer.h"

namespace vk_cpp_codec
{
using namespace cpp_codec;

class VkChunkTransformer : public ChunkTransformerNode
{
public:
  VkChunkTransformer(CodeWriter *writer) : writer(writer) {}
protected:
  CodeWriter *writer;
  virtual void input(SDChunk *chunk)
  {
    switch(chunk->metadata.chunkID)
    {
      case(uint32_t)SystemChunk::DriverInit: DriverInit(chunk); break;
      case(uint32_t)VulkanChunk::vkEnumeratePhysicalDevices:
        vkEnumeratePhysicalDevices(chunk);
        break;
      case(uint32_t)VulkanChunk::vkCreateShaderModule: vkCreateShaderModule(chunk); break;
      case(uint32_t)VulkanChunk::vkCreateSwapchainKHR:
        vkCreateSwapchainKHR(chunk);
        break;
      // case(uint32_t)VulkanChunk::vkGetDeviceQueue: vkGetDeviceQueue(chunk); break;
      // case(uint32_t)VulkanChunk::vkGetSwapchainImagesKHR: vkGetSwapchainImagesKHR(chunk); break;
      default:
        // pass through all other chunks
        output(chunk);
        break;
    }
  }
  void DriverInit(SDChunk *chunk)
  {
    // unpack the DriverInit chunk
    SDObject *init_params = chunk->GetChild(0);
    rdcstr appName = init_params->FindChild("AppName")->AsString();
    rdcstr engineName = init_params->FindChild("EngineName")->AsString();
    uint32_t appVersion = init_params->FindChild("AppVersion")->AsUInt32();
    uint32_t engineVersion = init_params->FindChild("EngineVersion")->AsUInt32();
    uint32_t apiVersion = init_params->FindChild("APIVersion")->AsUInt32();
    SDObject *layers = init_params->FindChild("Layers")->Duplicate();
    layers->name = "ppEnabledLayerNames";
    SDObject *extensions = init_params->FindChild("Extensions")->Duplicate();
    extensions->name = "ppEnabledExtensionNames";
    SDObject *instance = init_params->FindChild("InstanceID");

    // build the vkCreateInstance chunk
    SDObject *appInfo =
        StructPtr("pApplicationInfo", "VkApplicationInfo",
                  {Enum("sType", "VkStructureType", "VK_STRUCTURE_TYPE_APPLICATION_INFO"),
                   NullPtr("pNext", "void"), String("pApplicationName", appName),
                   UInt32("applicationVersion", appVersion), String("pEngineName", engineName),
                   UInt32("engineVersion", engineVersion), UInt32("apiVersion", apiVersion)});

    SDObject *createInfo = StructPtr(
        "pCreateInfo", "VkInstanceCreateInfo",
        {Enum("sType", "VkStructureType", "VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO"),
         NullPtr("pNext", "void"), UInt32("flags", 0), appInfo,
         UInt32("enabledLayerCount", (uint32_t)layers->NumChildren()), layers,
         UInt32("enabledExtensionCount", (uint32_t)extensions->NumChildren()), extensions});

    SDObject *pAllocator = NullPtr("pAllocator", "VkAllocationCallbacks");

    output(Chunk((uint32_t)CodeGenChunk::Custom, "vkCreateInstance",
                 {createInfo, pAllocator, instance}));
  }
  void vkEnumeratePhysicalDevices(SDChunk *chunk)
  {
    SDObject *instance = graph->Duplicate(chunk->FindChild("instance"));
    SDObject *pPhysicalDeviceCount = UInt32Ptr("pPhysicalDeviceCount", 0);
    // SDObject *physicalDeviceCount = LocalVar("physicalDeviceCount", "uint32_t");
    // SDObject *physicalDevices = LocalVar("physicalDevices", "std::vector<VkPhysicalDevice>");
    SDChunk *block = CodeBlock(
        "vkEnumeratePhysicalDevices",
        {
            Chunk(
                (uint32_t)VulkanChunk::vkEnumeratePhysicalDevices, "vkEnumeratePhysicalDevices",
                {instance, pPhysicalDeviceCount, NullPtr("pPhysicalDevices", "VkPhysicalDevice")}),
            Stmt("std::vector<VkPhysicalDevice> physicalDevices;"),
            Stmt("physicalDevices.resize(*pPhysicalDeviceCount);"),
            Chunk((uint32_t)VulkanChunk::vkEnumeratePhysicalDevices, "vkEnumeratePhysicalDevices",
                  {instance, pPhysicalDeviceCount, Code("physicalDevices.data()")}),
        });
    graph->FreeChunk(chunk);
    output(block);
  }
  void vkCreateShaderModule(SDChunk *chunk)
  {
    SDObject *createInfo = chunk->FindChild("CreateInfo");
    SDObject *buffer = createInfo->FindChild("pCode");
    rdcstr name = writer->GetDataBlobVar("shader", buffer->AsUInt64());
    buffer->SetCustomString("(const uint32_t*)" + name + ".data()");
    output(chunk);
  }
  // void vkGetDeviceQueue(SDChunk *chunk)
  //{
  //  SDObject *pQueue = chunk->FindChild("Queue");
  //  pQueue->type.flags |= SDTypeFlags::Nullable;
  //  output(chunk);
  //}
  // void vkGetSwapchainImagesKHR(SDChunk *chunk)
  //{
  //  SDObject *pImageCount = chunk->FindChild("ImageIndex");
  //  if(pImageCount->AsUInt32() != 0)
  //    pImageCount->name = "pSwapchainImageCount";
  //  pImageCount->for(size_t i = 0; i < chunk->data.children.size(); i++)
  //  {
  //    SDObject if(chunk->data.children[i]->name == "ImageIndex")
  //    {
  //      if(chunk->data.children[i]->AsUInt32() != 0)
  //      {
  //        graph->FreeChunk(chunk);
  //        return;
  //      }
  //      chunk->data.children[i]->name = "pImageCount";
  //      chunk->data.children[i]->type.flags |= SDTypeFlags::Nullable;
  //      chunk->data.children
  //    }
  //  }
  //  SDObject *index = chunk->FindChild("ImageIndex");
  //  SDObject *block = CodeBlock("vkGetSwapchainImagesKHR", {

  //                                                         });
  //}
  void vkCreateSwapchainKHR(SDChunk *chunk)
  {
    chunk->FindChild("NumImages")->type.flags |= SDTypeFlags::Hidden;
    output(chunk);
  }
};

class VkGetSwapchainImagesKHRTransformer : public ChunkTransformerNode
{
  SDChunk *call = NULL;
  rdcarray<SDObject *> images;

protected:
  void outputBlock()
  {
    SDObject *pImageCount = call->FindChild("ImageIndex");
    pImageCount->name = "pSwapchainImageCount";
    pImageCount->type.flags |= SDTypeFlags::Nullable;
    pImageCount->data.basic.u = images.size();

    SDObject *pImages = call->FindChild("Image");
    pImages->name = "pSwapchainImages";
    pImageCount->type.flags |= SDTypeFlags::Nullable;

    SDChunk *call0 = graph->Duplicate(call);
    call0->FindChild("pSwapchainImages")->type.basetype = SDBasic::Null;

    pImages->SetCustomString("swapchainImages.data()");

    rdcarray<SDChunk *> stmts({
        call0, Stmt("std::vector<VkImage> swapchainImages(*pSwapchainImageCount);"), call,
    });

    for(size_t i = 0; i < images.size(); i++)
    {
      SDObject *index = UInt32("index", (uint32_t)i);
      stmts.push_back(Stmt("if (*pSwapchainImageCount > %s) { %s = swapchainImages[%s]; }", index,
                           images[i], graph->Duplicate(index)));
    }

    output(CodeBlock("vkGetSwapchainImagesKHR", stmts.begin(), stmts.end()));
    call = NULL;
  }
  virtual void input(SDChunk *chunk)
  {
    if(chunk->metadata.chunkID == (uint32_t)VulkanChunk::vkGetSwapchainImagesKHR)
    {
      bool freeChunk = true;
      if(call == NULL)
      {
        call = chunk;
        freeChunk = false;
      }
      else if(call->FindChild("swapchain")->AsUInt64() != chunk->FindChild("swapchain")->AsUInt64())
      {
        outputBlock();
        call = chunk;
        freeChunk = false;
      }
      images.push_back(graph->Duplicate(chunk->FindChild("Image")));
      if(freeChunk)
        graph->FreeChunk(chunk);
    }
    else if(call != NULL)
    {
      outputBlock();
    }
  }
};

class VkCreatePtrTransformer : public ChunkTransformerNode
{
protected:
  virtual void input(SDChunk *chunk)
  {
    if(chunk->name.substr(0, 8) == "vkCreate" || chunk->name.substr(0, 10) == "vkAllocate")
    {
      SDObject *pCreateInfo = NULL;
      SDObject *pHandle = NULL;
      for(size_t i = 0; i < chunk->NumChildren(); ++i)
      {
        SDObject *child = chunk->GetChild(i);
        if(child->IsHidden())
          continue;
        if(pCreateInfo == NULL && child->IsStruct())
          pCreateInfo = child;
        if(child->IsResource())
          pHandle = child;
      }
      pCreateInfo->type.flags |= SDTypeFlags::Nullable;
      pHandle->type.flags |= SDTypeFlags::Nullable;
    }
    else if(chunk->name.substr(0, 5) == "vkGet")
    {
      SDObject *pHandle = NULL;
      for(size_t i = 0; i < chunk->NumChildren(); ++i)
      {
        SDObject *child = chunk->GetChild(i);
        if(child->IsHidden())
          continue;
        if(child->IsResource())
          pHandle = child;
      }
      pHandle->type.flags |= SDTypeFlags::Nullable;
    }
    output(chunk);
  }
};

class VkCheckSuccessTransformer : public ChunkTransformerNode
{
protected:
  virtual void input(SDChunk *chunk)
  {
    switch(chunk->metadata.chunkID)
    {
      case(uint32_t)VulkanChunk::vkCreateDevice:
        chunk->metadata.flags |= SDChunkFlags::CheckSuccess;
        break;
      default: break;
    }
    output(chunk);
  }
};

class VkNullHandleTransformer : public ObjTransformerNode
{
protected:
  virtual void inputObj(SDObject *obj)
  {
    if(obj->type.basetype == SDBasic::Resource && obj->AsUInt64() == 0 && !obj->HasCustomString())
    {
      if(obj->type.name == "VkInstance" || obj->type.name == "VkPhysicalDevice" ||
         obj->type.name == "VkDevice" || obj->type.name == "VkQueue" ||
         obj->type.name == "VkCommandBuffer")
        // dispatchable handle
        obj->SetCustomString("NULL");
      else
        // non-dispatchable handle
        obj->SetCustomString("VK_NULL_HANDLE");
    }
    recurse(obj);
  }
};

// class VkDriverInitTransformer : public ChunkTransformer
//{
//  // public:
//  //  VkDriverInitTransformer(ChunkTransformer *input) : ChunkTransformer(input) {}
// protected:
//  virtual void ProcessChunk(SDChunk *chunk)
//  {
//    if(chunk->metadata.chunkID != (uint32_t)SystemChunk::DriverInit)
//    {
//      // pass through all chunks that aren't DriverInit
//      yield(chunk);
//      return;
//    }
//
//    // unpack the DriverInit chunk
//    SDObject *init_params = chunk->GetChild(0);
//    rdcstr appName = init_params->FindChild("AppName")->AsString();
//    rdcstr engineName = init_params->FindChild("EngineName")->AsString();
//    uint32_t appVersion = init_params->FindChild("AppVersion")->AsUInt32();
//    uint32_t engineVersion = init_params->FindChild("EngineVersion")->AsUInt32();
//    uint32_t apiVersion = init_params->FindChild("APIVersion")->AsUInt32();
//    SDObject *layers = init_params->FindChild("Layers")->Duplicate();
//    layers->name = "ppEnabledLayerNames";
//    SDObject *extensions = init_params->FindChild("Extensions")->Duplicate();
//    extensions->name = "ppEnabledExtensionNames";
//    SDObject *instance = init_params->FindChild("InstanceID");
//
//    // build the vkCreateInstance chunk
//    SDObject *appInfo =
//        StructPtr("pApplicationInfo", "VkApplicationInfo",
//                  {Enum("sType", "VkStructureType", "VK_STRUCTURE_TYPE_APPLICATION_INFO"),
//                   NullPtr("pNext", "void"), String("pApplicationName", appName),
//                   UInt32("applicationVersion", appVersion), String("pEngineName", engineName),
//                   UInt32("engineVersion", engineVersion), UInt32("apiVersion", apiVersion)});
//
//    SDObject *createInfo = StructPtr(
//        "pCreateInfo", "VkCreateInfo",
//        {Enum("sType", "VkStructureType", "VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO"),
//         NullPtr("pNext", "void"), UInt32("flags", 0), appInfo,
//         UInt32("enabledLayerCount", (uint32_t)layers->NumChildren()), layers,
//         UInt32("enabledExtensionCount", (uint32_t)extensions->NumChildren()), extensions});
//
//    SDObject *pAllocator = NullPtr("pAllocator", "VkAllocationCallbacks");
//
//    yield(Chunk("vkCreateInstance", {createInfo, pAllocator, instance}));
//  }
//};
}

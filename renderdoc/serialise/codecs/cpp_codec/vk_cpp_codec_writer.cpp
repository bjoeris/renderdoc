/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2019 Google LLC
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


#include "vk_cpp_codec_writer.h"

namespace vk_cpp_codec
{
using namespace cpp_codec;
using namespace templates;

void VkCodeWriter::WriteDriverInit(const SDChunk *chunk) {}

class VkInitDescriptorSetWritesFunc : public DynFunc
{
  Value DoCall(State *state, const templates::Value &dot, rdcarray<templates::Value> args)
  {
    auto init = args[0].GetSDObject();
    auto initBindings = init.FindChild("Bindings");
    auto layoutCI = args[1].GetSDObject();
    auto writes = Value::WrappedSDObject::NewSDArray("pDescriptorWrites", "VkWriteDescriptorSet");
    auto layoutBindings = layoutCI.FindChild("pBindings").GetSDObject();
    uint64_t initIndex = layoutBindings.GetChild(0).Field("binding").ToUInt().GetUInt();
    uint64_t lastBindingIndex = initIndex;
    for(auto layoutBindingIt = layoutBindings.begin(); layoutBindingIt != layoutBindings.end();
        ++layoutBindingIt)
    {
      Value layoutBinding = layoutBindingIt->second;
      uint64_t bindingIndex = layoutBinding.Field("binding").ToUInt().GetUInt();
      if(bindingIndex > lastBindingIndex)
        initIndex += bindingIndex - lastBindingIndex - 1;
      lastBindingIndex = bindingIndex;
      uint64_t bindingDescriptorCount = layoutBinding.Field("descriptorCount").ToUInt().GetUInt();
      uint64_t arrayIndex = 0;
      uint64_t descriptorCount = 0;

      auto pImageInfo = Value::WrappedSDObject::NewSDArray("pImageInfo", "VkDescriptorImageInfo");
      auto pBufferInfo =
          Value::WrappedSDObject::NewSDArray("pBufferInfo", "VkDescriptorBufferInfo");
      auto pTexelBufferView = Value::WrappedSDObject::NewSDArray("pBufferInfo", "VkBufferview");

      VkDescriptorType descType =
          (VkDescriptorType)layoutBinding.Field("descriptorType").ToUInt().GetUInt();
      while(arrayIndex <= bindingDescriptorCount && initIndex <= initBindings.size())
      {
        bool valid = false;
        if(arrayIndex < bindingDescriptorCount && initIndex < initBindings.size())
        {
          Value initBinding = initBindings.Index(initIndex);
          switch(descType)
          {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            {
              Value imageInfo = initBinding.Field("imageInfo");
              if(descType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                 descType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
              {
                valid |= imageInfo.Field("sampler").ToUInt().GetUInt() != 0;
              }
              if(descType != VK_DESCRIPTOR_TYPE_SAMPLER)
              {
                valid |= imageInfo.Field("imageView").ToUInt().GetUInt() != 0;
              }
              if(valid)
              {
                pImageInfo.AddChild("$el", imageInfo);
              }
              break;
            }
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            {
              Value texelBufferView = initBinding.Field("texelBufferView");
              valid |= texelBufferView.ToUInt().GetUInt() != 0;
              if(valid)
              {
                pTexelBufferView.AddChild("$el", texelBufferView);
              }
              break;
            }
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            {
              Value bufferInfo = initBinding.Field("bufferInfo");
              valid |= bufferInfo.Field("buffer").ToUInt().GetUInt() != 0;
              if(valid)
              {
                pBufferInfo.AddChild("$el", bufferInfo);
              }
              break;
            }
          }
        }
        if(valid)
        {
          ++descriptorCount;
        }
        else if(descriptorCount > 0)
        {
          auto write = Value::WrappedSDObject::NewSDStruct(
              "$el", "VkWriteDescriptorSet",
              {
                  {"sType", "VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET"},
                  {"pNext", "NULL"},
                  {"dstSet", args[0].Field("id")},
                  {"dstBinding", bindingIndex},
                  {"dstArrayElement", arrayIndex - descriptorCount},
                  {"descriptorCount", descriptorCount},
                  {"descriptorType", layoutBinding.Field("descriptorType")},
                  {"pImageInfo", pImageInfo},
                  {"pBufferInfo", pBufferInfo},
                  {"pTexelBufferView", pTexelBufferView},
              });
          writes.AddChild("$el", Value(std::move(write)));
          pImageInfo.ClearChildren();
          pBufferInfo.ClearChildren();
          pTexelBufferView.ClearChildren();
          descriptorCount = 0;
        }
        ++arrayIndex;
        ++initIndex;
      }
    }
    return std::move(writes);
  }

public:
  VkInitDescriptorSetWritesFunc()
      : DynFunc("VkInitDescriptorSetWrites", {ValueType::SDObject, ValueType::SDObject})
  {
  }
};

void VkCodeWriter::SetupTemplates(cpp_codec::templates::Template &t)
{
  CodeWriter::SetupTemplates(t);
  t.AddFunc(new VkInitDescriptorSetWritesFunc());
  // templates::parse::ParseError err = t.Parse("VULKAN_TEMPLATES", VULKAN_TEMPLATES);
  t.Load("templates/VkTemplates.tmpl");
}
rdcarray<cpp_codec::ResourceFileDesc> VkCodeWriter::GetResourceFiles()
{
  const rdcarray<cpp_codec::ResourceFileDesc> &allResources = GetResourceDescs();
  rdcarray<cpp_codec::ResourceFileDesc> vulkanResources;
  rdcstr prefix = "vulkan/"_lit;
  for(auto it = allResources.begin(); it != allResources.end(); ++it)
  {
    cpp_codec::ResourceFileDesc desc = *it;
    if(desc.filename.substrEq(0, prefix))
    {
      desc.filename = desc.filename.substr(prefix.size());
      vulkanResources.push_back(desc);
    }
  }
  return vulkanResources;
}
}    // namespace vk_cpp_codec

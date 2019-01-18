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
#include "serialise/rdcfile.h"
#include "vk_cpp_codec_common.h"
#include "vk_cpp_codec_state.h"
#include "vk_cpp_codec_tracker.h"
#include "vk_cpp_codec_writer.h"

#pragma push_macro("GenericEvent")
#ifdef GenericEvent
#undef GenericEvent
#endif

namespace vk_cpp_codec
{
void CodeWriter::InlineVariable(SDObject *o, uint32_t pass)
{
  files[pass]->PrintLn("{ /* %s = */", o->Name());
  for(uint64_t i = 0, j = 0; i < o->NumChildren(); i++)
  {
    if (o->GetChild(i)->IsHidden())
      continue;
    std::string add_suffix;
    SDObject *node = tracker->CopiesAdd(o, i, j, add_suffix);
    if(!node->IsSimpleType())
    {
      InlineVariable(node, pass);
    }
    else if(node->IsResource())
    {
      files[pass]->PrintLn("/* %s = */ %s,", node->Name(), tracker->GetResourceVar(node->AsUInt64()));
    }
    else
    {
      files[pass]->PrintLn("/* %s = */ %s,", node->Name(), ValueStr(node).c_str());
    }
    j++;
  }
  files[pass]->PrintLn("},");
}

void CodeWriter::AssignUnion(std::string path, SDObject *o, bool comment, uint32_t pass)
{
  if(o->IsStruct() || o->IsArray() || o->IsUnion())
  {
    for(uint64_t i = 0; i < o->NumChildren(); i++)
    {
      SDObject *child = o->GetChild(i);
      std::string childPath(path);
      bool childComment = false;
      if(o->IsArray())
      {
        childPath += "[" + std::to_string(i) + "]";
      }
      else if(o->IsStruct())
      {
        if(o->IsPointer())
        {
          childPath += "->";
        }
        else
        {
          childPath += ".";
        }
        childPath += std::string(child->Name());
        if(o->IsUnion())
        {
          if(i != CanonicalUnionBranch(o))
          {
            childComment = true;
          }
        }
      }
      AssignUnion(childPath, child, comment | childComment, pass);
    }
    return;
  }

  if(o->IsPointer() && !o->IsNULL())
  {
    // This would be a non-null, non-array pointer to non-struct;
    // This should never happen.
    RDCASSERT(0);
  }

  std::string commentStr, value;
  if(comment)
  {
    commentStr = "// ";
  }

  if(o->IsResource())
  {
    value = tracker->GetResourceVar(o->AsUInt64());
  }
  else
  {
    value = ValueStr(o).c_str();
  }
  files[pass]->PrintLn("%s%s = %s;", commentStr.c_str(), path.c_str(), value.c_str());
}

void CodeWriter::LocalVariable(SDObject *o, std::string suffix, uint32_t pass)
{
  // Unions have multiple elements in them, which is why they need to be
  // handled first, ahead of structs and arrays.
  if (o->IsHidden())
    return;

  if(o->IsUnion())
  {
    std::string name(o->Name());
    name += suffix;
    files[pass]->PrintLn("%s %s;", Type(o), name.c_str());
    AssignUnion(name, o, false, pass);
    return;
  }
  else
  {
    uint64_t size = o->NumChildren();
    uint64_t hidden_count = 0;
    // Go through all the children and look for complex structures or variable-
    // size arrays. For each of those, declare and initialize them separately.
    for(uint64_t i = 0, j = 0; i < size; i++)
    {
      if (o->GetChild(i)->IsHidden()) {
        hidden_count++;
        continue;
      }
      // Handle cases when the member is a complex data type, such as a complex
      // structure or a variable sized array.
      if(!o->GetChild(i)->IsInlineable())
      {
        std::string add_suffix;
        SDObject *node = tracker->CopiesAdd(o, i, j, add_suffix);
        LocalVariable(node, suffix + add_suffix, pass);
      }
      j++;
    }

    // Now, declare and initialize the data type. Simple members get inlined.
    // Complex structures or variable arrays get referenced by name.
    if(o->IsNULL() || size - hidden_count == 0)
    {
      files[pass]->PrintLn("%s* %s%s = NULL;", Type(o), o->Name(), suffix.c_str());
    }
    else if(o->IsStruct() && !o->IsPointer())
    {
      files[pass]->PrintLn("%s %s%s = {", Type(o), o->Name(), suffix.c_str());
    }
    else if(o->IsStruct() && o->IsPointer())
    {
      files[pass]->PrintLn("%s %s%s[1] = {", Type(o), o->Name(), suffix.c_str());
    }
    else if(o->IsArray())
    {
      files[pass]->PrintLn("%s %s%s[%llu] = {", Type(o), o->Name(), suffix.c_str(), size - hidden_count);
    }
    
    for(uint64_t i = 0, j = 0; i < size; i++)
    {
      if (o->GetChild(i)->IsHidden()) {
        continue;
      }

      std::string add_suffix;
      SDObject *node = tracker->CopiesAdd(o, i, j, add_suffix);
      if(!node->IsInlineable())
      {
        files[pass]->PrintLn("/* %s = */ %s%s,", node->Name(), node->Name(),
                             (suffix + add_suffix).c_str());
      }
      else if(!node->IsSimpleType() && node->IsInlineable())
      {
        InlineVariable(node, pass);
      }
      else if(node->IsResource())
      {
        files[pass]->PrintLn("/* %s = */ %s,", node->Name(), tracker->GetResourceVar(node->AsUInt64()));
      }
      else
      {
        files[pass]->PrintLn("/* %s = */ %s,", node->Name(), ValueStr(node).c_str());
      }

      j++;
    }

    if(!o->IsNULL())
      files[pass]->PrintLn("};");
  }
}

void CodeWriter::GenericVkCreate(SDObject *o, uint32_t pass, bool global_ci)
{
  SDObject *device = o->GetChild(0);
  SDObject *ci = o->GetChild(1);
  SDObject *vk_res = o->GetChild(3);

  const char *device_name = tracker->GetResourceVar(device->AsUInt64());
  const char *res_name = tracker->GetResourceVar(Type(vk_res), vk_res->AsUInt64());

  files[pass]->PrintLn("{");
  LocalVariable(ci, "", pass);

  if(global_ci)
  {
    std::string ci_name = AddVar(Type(ci), vk_res->AsUInt64());
    files[pass]->PrintLn("%s = %s;", ci_name.c_str(), ci->Name());
  }
  std::string resource_name_str;
  if(*shimPrefix)
    resource_name_str.append(", \"").append(res_name).append("\"");
  files[pass]
      ->PrintLn("VkResult result = %s(%s, &%s, NULL, &%s%s);", o->Name(), device_name, ci->Name(),
                res_name, resource_name_str.c_str())
      .PrintLn("assert(result == VK_SUCCESS);")
      .PrintLn("}");
}

void CodeWriter::GenericCreatePipelines(SDObject *o, uint32_t pass, bool global_ci)
{
  SDObject *device = o->GetChild(0);
  SDObject *cache = o->GetChild(1);
  SDObject *ci_count = o->GetChild(2);
  SDObject *ci = o->GetChild(3);
  SDObject *pipe = o->GetChild(5);

  // CreateInfoCount must always be equal to '1'.
  // Create[Graphics|Compute]Pipelines can create multiple pipelines at the
  // same time, but RenderDoc splits these calls into multiple calls, one per
  // each pipeline object that is still alive at the time of capture.
  RDCASSERT(ci_count->AsUInt64() == 1);

  const char *device_name = tracker->GetResourceVar(device->AsUInt64());
  const char *cache_name = tracker->GetResourceVar(cache->AsUInt64());
  const char *pipe_name = tracker->GetResourceVar(Type(pipe), pipe->AsUInt64());

  files[pass]->PrintLn("{");
  LocalVariable(ci, "", pass);
  std::string resource_name_str;
  if(*shimPrefix)
    resource_name_str.append(", \"").append(pipe_name).append("\"");
  files[pass]
      ->PrintLn("VkResult result = %s(%s, %s, 1, &%s, NULL, &%s%s);", o->Name(), device_name,
                cache_name, ci->Name(), pipe_name, resource_name_str.c_str())
      .PrintLn("assert(result == VK_SUCCESS);")
      .PrintLn("}");
}

void CodeWriter::GenericEvent(SDObject *o, uint32_t pass)
{
  files[pass]->PrintLn("{");
  files[pass]
      ->PrintLn("%s(%s, %s);", o->Name(), tracker->GetResourceVar(o->GetChild(0)->AsUInt64()),
                tracker->GetResourceVar(o->GetChild(1)->AsUInt64()))
      .PrintLn("}");
}

void CodeWriter::GenericWaitIdle(SDObject *o, uint32_t pass)
{
  files[pass]->PrintLn("%s(%s);", o->Name(), tracker->GetResourceVar(o->GetChild(0)->AsUInt64()));
}

void CodeWriter::GenericCmdSetRectTest(SDObject *o, uint32_t pass)
{
  files[pass]->PrintLn("{");
  LocalVariable(o->GetChild(3), "", pass);
  files[pass]
      ->PrintLn("%s(%s, %llu, %llu, %s);", o->Name(), tracker->GetResourceVar(o->GetChild(0)->AsUInt64()),
                o->GetChild(1)->AsUInt64(), o->GetChild(2)->AsUInt64(), o->GetChild(3)->Name())
      .PrintLn("}");
}
void CodeWriter::GenericCmdSetStencilParam(SDObject *o, uint32_t pass)
{
  files[pass]->PrintLn("%s(%s, %s, %llu);", o->Name(), tracker->GetResourceVar(o->GetChild(0)->AsUInt64()),
    ValueStr(o->GetChild(1)).c_str(), o->GetChild(2)->AsUInt64());
}

void CodeWriter::GenericCmdEvent(SDObject *o, uint32_t pass)
{
  files[pass]->PrintLn("{");
  files[pass]
      ->PrintLn("%s(%s, %s, %s);", o->Name(), tracker->GetResourceVar(o->GetChild(0)->AsUInt64()),
                tracker->GetResourceVar(o->GetChild(1)->AsUInt64()), ValueStr(o->GetChild(2)).c_str())
      .PrintLn("}");
}
void CodeWriter::GenericCmdDrawIndirect(SDObject *o, uint32_t pass)
{
  files[pass]->PrintLn(
      "%s(%s, %s, %llu, %llu, %llu);", o->Name(), tracker->GetResourceVar(o->GetChild(0)->AsUInt64()),
      tracker->GetResourceVar(o->GetChild(1)->AsUInt64()), o->GetChild(2)->AsUInt64(), o->GetChild(3)->AsUInt64(), o->GetChild(4)->AsUInt64());
}
}

#pragma pop_macro("GenericEvent")

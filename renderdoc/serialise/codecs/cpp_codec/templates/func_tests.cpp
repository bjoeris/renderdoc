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

#include "common/globalconfig.h"
#include "cpp_templates.h"
#include "exec.h"

#if ENABLED(ENABLE_UNIT_TESTS)

#include "3rdparty/catch/catch.hpp"

using namespace cpp_codec::templates;
using namespace cpp_codec::templates::parse;

void checkCodeGen(const rdcstr &tmplSrc, Value dot, const rdcstr &expected)
{
  Template tmpl;
  AddCommonFuncs(tmpl);
  tmpl.Load("templates/CPPTemplates.tmpl");
  // AddCommonTemplates(tmpl);
  CHECK(tmpl.Parse("", tmplSrc).IsOk());

  StringWriter out;
  CHECK(tmpl.Exec(out, "", dot).IsOk());
  rdcstr str = out.str();
  CHECK(str == expected);
}

TEST_CASE("Test CPP Codec Template Language Functions", "[templates]")
{
  SDAllocator alloc;
  SECTION("Struct LocalVariable")
  {
    SDObject *appInfo =
        alloc.NewStructPtr("pApplicationInfo", "VkApplicationInfo",
                           {
                               alloc.NewEnum("sType", "VkStructureTypeApplicationInfo", 0,
                                             "VK_STRUCTURE_TYPE_APPLICATION_INFO"),
                               alloc.NewNull("pNext"),
                               alloc.NewString("pApplicationName", "RenderDoc Capturing App"),
                               alloc.NewUInt32("applicationVersion", 0),
                               alloc.NewString("pEngineName", "RenderDoc"),
                               alloc.NewUInt32("engineVersion", 0),
                               alloc.NewUInt32("apiVersion", 4198400),
                           });
    SDObject *ci = alloc.NewStructPtr(
        "pInstanceCreateInfo", "VkInstanceCreaetInfo",
        {
            alloc.NewEnum("sType", "VkStructureTypeApplicationInfo", 1,
                          "VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO"),
            alloc.NewNull("pNext"),
            alloc.NewUInt32("flags", 0),
            appInfo,
            alloc.NewUInt32("enabledLayerCount", 1),
            alloc.NewArray(
                "ppEnabledLayerNames", SDType::String(),
                {alloc.NewString("Validation Layer", "VK_LAYER_LUNARG_standard_validation")}),
            alloc.NewUInt32("enabledExtensionCount", 4),
            alloc.NewArray("ppEnabledExtensionNames", SDType::String(),
                           {
                               alloc.NewString("$el", "VK_KHR_surface"),
                               alloc.NewString("$el", "VK_KHR_win32_surface"),
                               alloc.NewString("$el", "VK_EXT_debug_utils"),
                               alloc.NewString("Debug Report Extension", "VK_EXT_debug_report"),
                           }),
        });
    rdcstr tmpl = R"({{template "CPP.LocalVariable" Dict "obj" $ "name" (Name $) "suffix" ""}})";
    rdcstr expected = R"(VkApplicationInfo pApplicationInfo[1] = {
  /* sType = */ VK_STRUCTURE_TYPE_APPLICATION_INFO,
  /* pNext = */ NULL,
  /* pApplicationName = */ "RenderDoc Capturing App",
  /* applicationVersion = */ 0,
  /* pEngineName = */ "RenderDoc",
  /* engineVersion = */ 0,
  /* apiVersion = */ 4198400,
};
const char *ppEnabledLayerNames[1] = {
  /* ppEnabledLayerNames_0 = */ "VK_LAYER_LUNARG_standard_validation",
};
const char *ppEnabledExtensionNames[1] = {
  /* ppEnabledExtensionNames_0 = */ "VK_KHR_surface",
  /* ppEnabledExtensionNames_1 = */ "VK_KHR_win32_surface",
  /* ppEnabledExtensionNames_2 = */ "VK_EXT_debug_utils",
  /* ppEnabledExtensionNames_3 = */ "VK_EXT_debug_report",
};
VkInstanceCreaetInfo pInstanceCreateInfo[1] = {
  /* sType = */ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
  /* pNext = */ NULL,
  /* flags = */ 0,
  /* pApplicationInfo = */ pApplicationInfo,
  /* enabledLayerCount = */ 1,
  /* ppEnabledLayerNames = */ ppEnabledLayerNames,
  /* enabledExtensionCount = */ 4,
  /* ppEnabledExtensionNames = */ ppEnabledExtensionNames,
};
)";
    checkCodeGen(tmpl, Value(ci), expected);
  }
  SECTION("Union LocalVariable")
  {
    SDObject *pClearValues = alloc.NewArray(
        "pClearValues", "VkClearValue",
        {
            alloc.NewUnion(
                "$el", "VkClearValue",
                {
                    alloc.NewUnion("color", "VkClearColorValue",
                                   {
                                       alloc.NewArray("float32", "float",
                                                      {
                                                          alloc.NewFloat("$el", 1.0),
                                                          alloc.NewFloat("$el", 0.0),
                                                          alloc.NewFloat("$el", 0.0),
                                                          alloc.NewFloat("$el", 1.0),
                                                      }),
                                       alloc.NewArray("int32", "int32_t",
                                                      {
                                                          alloc.NewInt32("$el", 1065353216),
                                                          alloc.NewInt32("$el", 0),
                                                          alloc.NewInt32("$el", 0),
                                                          alloc.NewInt32("$el", 1065353216),
                                                      }),
                                       alloc.NewArray("uint32", "uint32_t",
                                                      {
                                                          alloc.NewUInt32("$el", 1065353216u),
                                                          alloc.NewUInt32("$el", 0u),
                                                          alloc.NewUInt32("$el", 0u),
                                                          alloc.NewUInt32("$el", 1065353216u),
                                                      }),
                                   }),
                    alloc.NewStruct("depthStencil", "VkClearDepthStencilValue",
                                    {
                                        alloc.NewFloat("depth", 1.0),
                                        alloc.NewUInt32("stencil", 1u),
                                    }),
                }),
            alloc.NewUnion(
                "$el", "VkClearValue",
                {
                    alloc.NewUnion("color", "VkClearColorValue",
                                   {
                                       alloc.NewArray("float32", "float",
                                                      {
                                                          alloc.NewFloat("$el", 0.0),
                                                          alloc.NewFloat("$el", 0.0),
                                                          alloc.NewFloat("$el", 0.0),
                                                          alloc.NewFloat("$el", 0.0),
                                                      }),
                                       alloc.NewArray("int32", "int32_t",
                                                      {
                                                          alloc.NewInt32("$el", 0),
                                                          alloc.NewInt32("$el", 0),
                                                          alloc.NewInt32("$el", 219566096),
                                                          alloc.NewInt32("$el", 0),
                                                      }),
                                       alloc.NewArray("uint32", "uint32_t",
                                                      {
                                                          alloc.NewUInt32("$el", 0u),
                                                          alloc.NewUInt32("$el", 0u),
                                                          alloc.NewUInt32("$el", 219566096u),
                                                          alloc.NewUInt32("$el", 0u),
                                                      }),
                                   }),
                    alloc.NewStruct("depthStencil", "VkClearDepthStencilValue",
                                    {
                                        alloc.NewFloat("depth", 0.0),
                                        alloc.NewUInt32("stencil", 0u),
                                    }),
                }),
        });
    rdcstr tmpl = R"({{template "CPP.LocalVariable" Dict "obj" $ "name" (Name $) "suffix" ""}})";
    rdcstr expected = R"(VkClearValue pClearValues_0;
pClearValues_0.color.float32[0] = 1.0000;
pClearValues_0.color.float32[1] = 0.0000;
pClearValues_0.color.float32[2] = 0.0000;
pClearValues_0.color.float32[3] = 1.0000;
// pClearValues_0.color.int32[0] = 1065353216;
// pClearValues_0.color.int32[1] = 0;
// pClearValues_0.color.int32[2] = 0;
// pClearValues_0.color.int32[3] = 1065353216;
// pClearValues_0.color.uint32[0] = 1065353216;
// pClearValues_0.color.uint32[1] = 0;
// pClearValues_0.color.uint32[2] = 0;
// pClearValues_0.color.uint32[3] = 1065353216;
// pClearValues_0.depthStencil.depth = 1.0000;
// pClearValues_0.depthStencil.stencil = 1;
VkClearValue pClearValues_1;
pClearValues_1.color.float32[0] = 0.0000;
pClearValues_1.color.float32[1] = 0.0000;
pClearValues_1.color.float32[2] = 0.0000;
pClearValues_1.color.float32[3] = 0.0000;
// pClearValues_1.color.int32[0] = 0;
// pClearValues_1.color.int32[1] = 0;
// pClearValues_1.color.int32[2] = 219566096;
// pClearValues_1.color.int32[3] = 0;
// pClearValues_1.color.uint32[0] = 0;
// pClearValues_1.color.uint32[1] = 0;
// pClearValues_1.color.uint32[2] = 219566096;
// pClearValues_1.color.uint32[3] = 0;
// pClearValues_1.depthStencil.depth = 0.0000;
// pClearValues_1.depthStencil.stencil = 0;
VkClearValue pClearValues[2] = {
  /* pClearValues_0 = */ pClearValues_0,
  /* pClearValues_1 = */ pClearValues_1,
};
)";
    checkCodeGen(tmpl, Value(pClearValues), expected);
  }
  SECTION("Call")
  {
    SDObject *appInfo =
        alloc.NewStructPtr("pApplicationInfo", "VkApplicationInfo",
                           {
                               alloc.NewEnum("sType", "VkStructureTypeApplicationInfo", 0,
                                             "VK_STRUCTURE_TYPE_APPLICATION_INFO"),
                               alloc.NewNull("pNext"),
                               alloc.NewString("pApplicationName", "RenderDoc Capturing App"),
                               alloc.NewUInt32("applicationVersion", 0),
                               alloc.NewString("pEngineName", "RenderDoc"),
                               alloc.NewUInt32("engineVersion", 0),
                               alloc.NewUInt32("apiVersion", 4198400),
                           });
    SDObject *ci = alloc.NewStructPtr(
        "pInstanceCreateInfo", "VkInstanceCreaetInfo",
        {
            alloc.NewEnum("sType", "VkStructureTypeApplicationInfo", 1,
                          "VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO"),
            alloc.NewNull("pNext"),
            alloc.NewUInt32("flags", 0),
            appInfo,
            alloc.NewUInt32("enabledLayerCount", 1),
            alloc.NewArray(
                "ppEnabledLayerNames", SDType::String(),
                {alloc.NewString("Validation Layer", "VK_LAYER_LUNARG_standard_validation")}),
            alloc.NewUInt32("enabledExtensionCount", 4),
            alloc.NewArray("ppEnabledExtensionNames", SDType::String(),
                           {
                               alloc.NewString("$el", "VK_KHR_surface"),
                               alloc.NewString("$el", "VK_KHR_win32_surface"),
                               alloc.NewString("$el", "VK_EXT_debug_utils"),
                               alloc.NewString("Debug Report Extension", "VK_EXT_debug_report"),
                           }),
        });
    SDChunk *vkCreateInstance = alloc.NewChunk(
        "vkCreateInstance", 0,
        {ci, alloc.NewNull("pAllocator"), alloc.NewResourcePtr("pInstance", "VkInstance", 3)});
    rdcstr tmpl = R"({{template "CPP.Chunk" $}})";
    rdcstr expected = R"(VkApplicationInfo pApplicationInfo[1] = {
  /* sType = */ VK_STRUCTURE_TYPE_APPLICATION_INFO,
  /* pNext = */ NULL,
  /* pApplicationName = */ "RenderDoc Capturing App",
  /* applicationVersion = */ 0,
  /* pEngineName = */ "RenderDoc",
  /* engineVersion = */ 0,
  /* apiVersion = */ 4198400,
};
const char *ppEnabledLayerNames[1] = {
  /* ppEnabledLayerNames_0 = */ "VK_LAYER_LUNARG_standard_validation",
};
const char *ppEnabledExtensionNames[1] = {
  /* ppEnabledExtensionNames_0 = */ "VK_KHR_surface",
  /* ppEnabledExtensionNames_1 = */ "VK_KHR_win32_surface",
  /* ppEnabledExtensionNames_2 = */ "VK_EXT_debug_utils",
  /* ppEnabledExtensionNames_3 = */ "VK_EXT_debug_report",
};
VkInstanceCreaetInfo pInstanceCreateInfo[1] = {
  /* sType = */ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
  /* pNext = */ NULL,
  /* flags = */ 0,
  /* pApplicationInfo = */ pApplicationInfo,
  /* enabledLayerCount = */ 1,
  /* ppEnabledLayerNames = */ ppEnabledLayerNames,
  /* enabledExtensionCount = */ 4,
  /* ppEnabledExtensionNames = */ ppEnabledExtensionNames,
};
vkCreateInstance(pInstanceCreateInfo, NULL, &VkInstance_3);
)";
    checkCodeGen(tmpl, Value(vkCreateInstance), expected);
  }
}

#endif    // ENABLED(ENABLE_UNIT_TESTS)

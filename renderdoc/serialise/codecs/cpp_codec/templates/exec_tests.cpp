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
#include "exec.h"

#if ENABLED(ENABLE_UNIT_TESTS)

#include "3rdparty/catch/catch.hpp"

using namespace cpp_codec::templates;
using namespace cpp_codec::templates::parse;

void checkExec(const rdcstr &src, const SDObject *dot, const rdcstr &expected)
{
  Template tmpl;
  CHECK(tmpl.Parse("", src).IsOk());
  StringWriter out;
  CHECK(tmpl.Exec(out, "", Value(dot)).IsOk());
  CHECK(out.str() == expected);
}

TEST_CASE("Test CPP Codec Template Language", "[templates]")
{
  SDAllocator alloc;
  SECTION("empty") { checkExec("", NULL, ""); }
  SECTION("text") { checkExec("abc def", NULL, "abc def"); }
  SECTION("dot")
  {
    SDObject obj("obj", SDType::UInt32());
    obj.data.basic.i = 42;
    checkExec("abc {{.}} def", &obj, "abc 42 def");
  }
  SECTION("field")
  {
    StringWriter out;
    SDObject obj("obj", "objType");
    obj.AddChild(makeSDUInt32("val", 42));
    checkExec("abc {{.val}} def", &obj, "abc 42 def");
  }
  SECTION("var")
  {
    SDObject obj("obj", "objType");
    obj.AddChild(makeSDUInt32("val", 42));
    checkExec("abc {{$x := .val}}{{$x}} def", &obj, "abc 42 def");
  }
  SECTION("var scope")
  {
    SDObject *obj = alloc.NewStruct("Obj", {
                                               alloc.NewBool("cond", true),
                                               alloc.NewUInt32("a", 42),
                                               alloc.NewUInt32("b", 17),
                                           });
    checkExec("abc {{$x := .a}}{{if .cond}}{{$x := .b}}{{end}}{{$x}} def", obj, "abc 42 def");
  }
  SECTION("var assign")
  {
    SDObject *obj = alloc.NewStruct("Obj", {
                                               alloc.NewUInt32("a", 42),
                                               alloc.NewUInt32("b", 17),
                                           });
    checkExec("abc {{$x := .a}}{{$x = .b}}{{$x}} def", obj, "abc 17 def");
  }
  SECTION("var assign scope")
  {
    SDObject *obj = alloc.NewStruct("Obj", {
                                               alloc.NewBool("cond", true),
                                               alloc.NewUInt32("a", 42),
                                               alloc.NewUInt32("b", 17),
                                           });
    checkExec("abc {{$x := .a}}{{if .cond}}{{$x = .b}}{{end}}{{$x}} def", obj, "abc 17 def");
  }
  SECTION("range")
  {
    SDObject obj("obj", "objType");
    obj.AddChild(makeSDString("val", "abc"));
    obj.AddChild(makeSDString("val", "def"));
    checkExec("{{range .}}{{.}}{{end}}", &obj, "abcdef");
  }
  SECTION("range item")
  {
    SDObject obj("obj", "objType");
    obj.AddChild(makeSDString("val", "abc"));
    obj.AddChild(makeSDString("val", "def"));
    checkExec("{{range $x := .}}{{$x}}{{end}}", &obj, "abcdef");
  }
  SECTION("range index")
  {
    SDObject obj("obj", "objType");
    obj.AddChild(makeSDString("val", "abc"));
    obj.AddChild(makeSDString("val", "def"));
    checkExec("{{range $i, $x := .}}{{$i}}:{{$x}}\n{{end}}", &obj, "0:abc\n1:def\n");
  }
  SECTION("prefix")
  {
    SDObject obj("obj", "objType");
    obj.AddChild(makeSDString("val", "//"));
    checkExec(R"(
abc
{{prefix .val}}def
ghi
{{end}}
jkl
)",
              &obj,
              R"(abc
//def
//ghi
jkl
)");
  }
}
//  SECTION("Common")
//  {
//    Template tmpl;
//    CommonTemplates(tmpl);
//    StringWriter out;
//    SDObject *appInfo = new SDObject("pApplicationInfo", "VkApplicationInfo");
//    appInfo->type.flags |= SDTypeFlags::Nullable;
//    SDObject *sType = makeSDEnum("VkStructureTypeApplicationInfo", 1);
//    sType->SetCustomString("VK_STRUCTURE_TYPE_APPLICATION_INFO");
//    appInfo->AddChild(sType);
//    SDObject *pNext = makeSDStruct("pNext", "void");
//    pNext->type.basetype = SDBasic::Null;
//    appInfo->AddChild(pNext);
//    CHECK(tmpl.Exec(out, "CPP.Assign", appInfo));
//    rdcstr str = out.str();
//    CHECK(str == R"(pApplicationInfo->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
// pApplicationInfo->pNext = NULL;
//)");
//}

#endif    // ENABLED(ENABLE_UNIT_TESTS)

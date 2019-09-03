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
#else
#define VK_USE_PLATFORM_XLIB_KHR 1
#endif
#include "driver/vulkan/vk_common.h"
#include "driver/vulkan/vk_resources.h"
#if defined(WIN32)
#undef VK_USE_PLATFORM_WIN32_KHR
#else
#undef VK_USE_PLATFORM_XLIB_KHR
#endif
#include "serialise/rdcfile.h"
#include "templates/cpp_templates.h"

namespace cpp_codec
{
class CodeFile
{
protected:
  // FILE *mCpp = NULL;
  // FILE *mHeader = NULL;
  templates::FileWriter mCppFileWriter, mHeaderFileWriter;
  templates::ReflowWriter mCppWriter, mHeaderWriter;
  // int cpp_lines = 0;
  // int header_lines = 0;
  std::string cpp_name = "";
  std::string header_name = "";
  // int bracket_count = 0;
  std::string spaces = "";
  std::string func_name = "";
  std::string directoryPath = "";
  // bool atLineBegin = true;

public:
  CodeFile(const std::string &dirPath, const std::string &file_name)
      : directoryPath(dirPath),
        func_name(file_name),
        mCppWriter(&mCppFileWriter),
        mHeaderWriter(&mHeaderFileWriter)
  {
  }

  inline templates::ReflowWriter &cppWriter() { return mCppWriter; }
  inline templates::ReflowWriter &headerWriter() { return mHeaderWriter; }
  virtual ~CodeFile() { CloseAll(); }
  // const char *FormatLine(const char *fmt, bool new_line)
  //{
  //  size_t length = strlen(fmt);
  //  // Assume there is always either one '{' or pairs of '{' and '}'.
  //  // If one '{' is present this will add two space indention.
  //  // If a pair is present, nothing will happen.
  //  // Pairs reflect structured objects like vectors {.x, .y, .z}
  //  int bracket = 0;
  //  for(size_t i = 0; i < length; i++)
  //  {
  //    if(fmt[i] == '{')
  //      bracket += 2;
  //    else if(fmt[i] == '}')
  //      bracket -= 2;
  //  }
  //  // Assert that the above assumption is correct (either braces are balanced, or there is one
  //  // extra opening or closing one).
  //  RDCASSERT(bracket == 0 || bracket == 2 || bracket == -2);
  //  if(bracket < 0)
  //    bracket_count += bracket;
  //  if(bracket_count < spaces.length())
  //  {
  //    spaces = spaces.substr(0, bracket_count);
  //  }
  //  else
  //  {
  //    if(bracket_count > spaces.length())
  //    {
  //      length = spaces.length();
  //      for(int i = (int)length; i < bracket_count; i += 2)
  //        spaces += "  ";
  //    }
  //  }
  //  static std::string format;
  //  if(atLineBegin)
  //    format = spaces + std::string(fmt);
  //  else
  //    format = std::string(fmt);
  //  if(new_line)
  //  {
  //    format += std::string("\n");
  //    atLineBegin = true;
  //  }
  //  else
  //  {
  //    atLineBegin = false;
  //  }
  //  cpp_lines += new_line ? 1 : 0;
  //  if(bracket > 0)
  //    bracket_count += bracket;
  //  return format.c_str();
  //}

  // This only accepts `%s` string fields
  template <typename Iter>
  CodeFile &VPrintf(FILE *file, const char *fmt, bool new_line, Iter begin, Iter end)
  {
    fmt = FormatLine(fmt, new_line);
    Iter iter = begin;
    for(const char *p = fmt; *p; ++p)
    {
      if(*p == '%')
      {
        ++p;
        if(*p == '%')
        {
          fwrite(fmt, sizeof(char), p - fmt, file);
        }
        else if(*p == 's')
        {
          assert(iter != end);
          fwrite(fmt, sizeof(char), p - fmt - 1, file);
          const char *v = *iter;
          fwrite(fmt, sizeof(char), strlen(v), file);
        }
        else
        {
          assert(false);
        }
      }
    }
  }

  // This only accepts `%s` string fields
  template <typename Iter>
  CodeFile &VPrint(const char *fmt, Iter begin, Iter end)
  {
    VPrintf(cpp, fmt, false, begin, end);
  }

  // This only accepts `%s` string fields
  template <typename Iter>
  CodeFile &VPrintLn(const char *fmt, Iter begin, Iter end)
  {
    VPrintf(cpp, fmt, true, begin, end);
  }

  template <typename... Args>
  CodeFile &Print(const char *fmt, Args... args)
  {
    mCppWriter.Printf(fmt, args...);
    // va_list argptr;
    // va_start(argptr, fmt);
    // vfprintf(cpp, FormatLine(fmt, false), argptr);
    // va_end(argptr);
    return *this;
  }

  template <typename... Args>
  CodeFile &PrintLn(const char *fmt, Args... args)
  {
    mCppWriter.Printf(fmt, args...);
    mCppWriter.NewLine();
    // va_list argptr;
    // va_start(argptr, fmt);
    // vfprintf(cpp, FormatLine(fmt, true), argptr);
    // va_end(argptr);
    return *this;
  }

  template <typename... Args>
  CodeFile &PrintLnH(const char *fmt, Args... args)
  {
    mHeaderWriter.Printf(fmt, args...);
    mHeaderWriter.NewLine();
    // std::string format = std::string(fmt) + std::string("\n");
    // va_list argptr;
    // va_start(argptr, fmt);
    // vfprintf(header, format.c_str(), argptr);
    // va_end(argptr);
    // header_lines++;
    return *this;
  }

  void CloseAll()
  {
    CloseCPP();
    CloseHeader();
  }

  void CloseHeader()
  {
    mHeaderWriter.Close();
    mHeaderFileWriter.Close();

    // fprintf(mHeader, "\n");
    // FileIO::fclose(mHeader);
    // mHeader = NULL;
    // header_lines = 0;
  }

  virtual void CloseCPP()
  {
    mCppWriter.Close();
    mCppFileWriter.Close();
    // if(bracket_count >= 2)
    //{
    //  RDCASSERT(bracket_count == 2);
    //  PrintLn("}");
    //}
    //// a good check for the code gen.
    // RDCASSERT(bracket_count == 0);
    // FileIO::fclose(cpp);
    // cpp = NULL;
    // cpp_lines = 0;
  }

  virtual void Open(const std::string &file_name)
  {
    std::string name = std::string("gen_") + file_name;
    header_name = name + std::string(".h");
    cpp_name = name + std::string(".cpp");
    std::string header_path = directoryPath + "/" + header_name;
    std::string cpp_path = directoryPath + "/" + cpp_name;
    FileIO::CreateParentDirectory(header_path);
    mHeaderFileWriter.Open(header_path.c_str());
    mHeaderWriter.Reset();
    mCppFileWriter.Open(cpp_path.c_str());
    mCppWriter.Reset();

    PrintLnH("#pragma once")
        .PrintLnH("#include \"common.h\"")
        .PrintLn("#include \"%s\"", header_name.c_str());
  }

  virtual void MultiPartSplit() {}
};

class MultiPartCodeFile : public CodeFile
{
protected:
  int index = 0;

public:
  MultiPartCodeFile(const std::string &dir_path, const std::string &file_name)
      : CodeFile(dir_path, file_name)
  {
  }
  virtual ~MultiPartCodeFile() { CloseAll(); }
  virtual void Open(const std::string &file_name)
  {
    std::string name = std::string("gen_") + file_name;
    cpp_name = name + std::string("_") + std::to_string(index) + std::string(".cpp");

    std::string cpp_path = directoryPath + "/" + cpp_name;
    FileIO::CreateParentDirectory(cpp_path);
    mCppFileWriter.Open(cpp_path.c_str());
    mCppWriter.Reset();
    // cpp = FileIO::fopen(cpp_path.c_str(), "wt");
    // RDCASSERT(cpp != NULL);

    if(!mHeaderFileWriter.IsOpen())
    {
      header_name = name + std::string(".h");
      std::string header_path = directoryPath + "/" + header_name;
      mHeaderFileWriter.Open(header_path.c_str());
      PrintLnH("#pragma once")
          .PrintLnH("#include \"common.h\"")
          .PrintLnH("#include \"gen_variables.h\"");
    }
    // if(header == NULL)
    //{
    //  header_name = name + std::string(".h");
    //  std::string header_path = directoryPath + "/" + header_name;
    //  header = FileIO::fopen(header_path.c_str(), "wt");
    //  RDCASSERT(header != NULL);
    //  PrintLnH("#pragma once")
    //      .PrintLnH("#include \"common.h\"")
    //      .PrintLnH("#include \"gen_variables.h\"");
    //}

    PrintLnH("void %s_%d();", func_name.c_str(), index)
        .PrintLn("#include \"%s\"", header_name.c_str())
        .PrintLn("void %s_%d() {", func_name.c_str(), index);
  }

  virtual void CloseCPP()
  {
    PrintLn("}");
    CodeFile::CloseCPP();
  }

  virtual void MultiPartSplit()
  {
    cppWriter().NewLine();
    // if(cpp_lines > 10000)
    if(mCppWriter.lineCount() > 10000)
    {
      CloseCPP();
      index++;
      Open(func_name);
    }
  }

  int GetIndex() { return index; }
};

}    // namespace vk_cpp_codec

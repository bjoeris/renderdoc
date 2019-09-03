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

#include "value.h"

namespace cpp_codec
{
namespace templates
{
class Writer
{
  virtual bool DoWrite(const char *str, size_t length) = 0;

public:
  inline bool Write(const rdcstr &str) { return DoWrite(str.c_str(), str.size()); }
  inline bool Write(const char *str, size_t length) { return DoWrite(str, length); }
  inline bool Write(const char c) { return DoWrite(&c, 1); }
  bool Print(Value v);
  template <typename... Args>
  bool Printf(const char *fmt, Args... args)
  {
    rdcstr str;
    str.sprintf(fmt, args...);
    return Write(str);
  }
  inline bool NewLine() { return DoWrite("\n", 1); }
};

class FileWriter : public Writer
{
  FILE *mFile;
  virtual bool DoWrite(const char *str, size_t length)
  {
    if(mFile == NULL)
      return false;
    return fwrite(str, sizeof(char), length, mFile) == length;
  }

public:
  FileWriter(FILE *f) : mFile(f) {}
  FileWriter() : mFile(NULL) {}
  inline void Open(const char *path)
  {
    if(IsOpen())
      Close();
    mFile = FileIO::fopen(path, "wt");
  }
  inline void Close()
  {
    if(IsOpen())
    {
      FileIO::fclose(mFile);
      mFile = NULL;
    }
  }
  inline bool IsOpen() const { return mFile != NULL; }
  inline FILE *file() { return mFile; }
};

class StreamWriter : public Writer
{
  std::ostream &ostr;
  virtual bool DoWrite(const char *str, size_t length)
  {
    ostr.write(str, length);
    return ostr.good();
  }

public:
  StreamWriter(std::ostream &ostr) : ostr(ostr) {}
};

class StringWriter : public Writer
{
  std::ostringstream ostr;
  virtual bool DoWrite(const char *str, size_t length)
  {
    ostr.write(str, length);
    return ostr.good();
  }

public:
  inline rdcstr str() const { return ostr.str(); }
};

class ReflowWriter : public Writer
{
  Writer *mInner;
  rdcarray<rdcstr> prefixes;
  rdcstr space;
  bool atLineStart = true;
  bool suppressWhitespace = true;
  size_t mLineCount = 0;
  size_t mByteCount = 0;

  virtual bool DoWrite(const char *str, size_t length);

public:
  static const char SUPPRESS_WHITESPACE;
  static const char FORCE_WHITESPACE;

  bool Close();
  void Reset();

  void PushPrefix(const rdcstr &prefix);
  bool PopPrefix();
  void Indent();
  bool Unindent();
  size_t lineCount() const { return mLineCount; }
  size_t byteCount() const { return mByteCount; }
  ReflowWriter(Writer *inner) : mInner(inner) {}
  inline Writer *inner() { return mInner; }
};
}    // namespace templates
}    // namespace cpp_codec

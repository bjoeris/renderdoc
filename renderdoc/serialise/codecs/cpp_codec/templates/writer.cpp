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

#include "writer.h"
#include "exec.h"
#include "lex.h"

namespace cpp_codec
{
namespace templates
{
const char ReflowWriter::SUPPRESS_WHITESPACE = '\xA7';    // section sign
const char ReflowWriter::FORCE_WHITESPACE = '\xB7';       // center dot

bool ReflowWriter::Close()
{
  if(atLineStart)
  {
    // Pending newline at eof.
    // Write the newline, but no prefix.
    mInner->Write(space);
    space.clear();
  }
  return true;
}
void ReflowWriter::Reset()
{
  prefixes.clear();
  space = "";
  atLineStart = true;
  suppressWhitespace = true;
  mLineCount = 0;
  mByteCount = 0;
}
void ReflowWriter::PushPrefix(const rdcstr &prefix)
{
  prefixes.push_back(prefix);
}
bool ReflowWriter::PopPrefix()
{
  if(prefixes.empty())
    return false;
  prefixes.pop_back();
  return true;
}
void ReflowWriter::Indent()
{
  prefixes.push_back("  ");
}
bool ReflowWriter::Unindent()
{
  if(prefixes.empty() || prefixes.back() != "  ")
    return false;
  prefixes.pop_back();
  return true;
}
bool ReflowWriter::DoWrite(const char *str, size_t len)
{
  size_t i = 0;
  while(i < len)
  {
    if(lex::isSpace(str[i]))
    {
      size_t j = i;
      while(j < len && lex::isSpace(str[j]))
        ++j;
      if(!suppressWhitespace)
        space.append(str + i, j - i);
      i = j;
      continue;
    }
    if(lex::isNewline(str[i]))
    {
      // found the end of the line.

      size_t eolLength = 1;
      if(i + 1 < len && str[i] == '\r' && str[i + 1] == '\n')
        // windows newline
        eolLength = 2;

      if(!suppressWhitespace)
      {
        ++mLineCount;
        atLineStart = true;
        suppressWhitespace = true;
        space.clear();
        // space.append(&str[i], eolLength);
        space.append("\n", 1);
        // inner->Write(&str[i], eolLength);
      }

      i += eolLength;

      // Restart writing the next char
      continue;
    }
    if(str[i] == FORCE_WHITESPACE)
    {
      space.push_back(' ');
    }
    if(str[i] == SUPPRESS_WHITESPACE)
    {
      suppressWhitespace = true;
      ++i;
      continue;
    }
    if(std::isgraph(str[i]))
    {
      mByteCount += space.length();
      if(!mInner->Write(space))
        return false;
      space.clear();
      size_t j = i;
      while(j < len && str[j] == '}')
      {
        Unindent();
        ++j;
      }
      if(atLineStart)
      {
        for(auto pi = prefixes.begin(); pi != prefixes.end(); ++pi)
        {
          mByteCount += pi->length();
          if(!mInner->Write(*pi))
            return false;
        }
      }
      atLineStart = suppressWhitespace = false;
      while(j < len && std::isgraph(str[j]))
      {
        if(str[j] == '{')
          Indent();
        else if(str[j] == '}')
          Unindent();
        ++j;
      }
      mByteCount += j - i;
      if(!mInner->Write(str + i, j - i))
        return false;
      i = j;
      continue;
    }
    // If we reach here, str[i] is not whitespace, not a graphic character, and not a meaningful
    // special character.
    return false;
  }
  return true;
}

bool Writer::Print(Value v)
{
//#define PRINT(T)            \
//  {                         \
//    T x = v.Cast<T>(err);   \
//    if(err.IsError())       \
//      return false;         \
//    return Write(ToStr(x)); \
//  }
#define PRINT(x) return Write(ToStr(x));
  switch(v.Type())
  {
    case ValueType::Null: return true;
    case ValueType::String: PRINT(v.GetString());
    case ValueType::Bool: PRINT(v.GetBool());
    case ValueType::Int: PRINT(v.GetInt());
    case ValueType::UInt: PRINT(v.GetUInt());
    case ValueType::Float: PRINT(v.GetFloat());
    case ValueType::Char: PRINT(v.GetChar());
    case ValueType::Dict: return false;
    case ValueType::SDObject:
    {
      const Value::WrappedSDObject &obj = v.GetSDObject();
      if(obj.HasCustomString())
      {
        return Write(obj.data().str);
      }
      switch(obj.type().basetype)
      {
        case SDBasic::Null: return true;
        case SDBasic::String: return Write(obj.data().str);
        case SDBasic::Boolean: return Write(ToStr(obj.data().basic.b));
        case SDBasic::SignedInteger: return Write(ToStr(obj.data().basic.i));
        case SDBasic::UnsignedInteger: return Write(ToStr(obj.data().basic.u));
        case SDBasic::Float: return Write(ToStr(obj.data().basic.d));
        case SDBasic::Character: return Write(ToStr(obj.data().basic.c));
        case SDBasic::Buffer: return Write(ToStr(obj.data().basic.u));
        case SDBasic::Enum: return Write(ToStr(obj.data().basic.i));
        case SDBasic::Resource: return Write(ToStr(obj.data().basic.u));
        default: return false;
      }
    }
    case ValueType::Error: return false;
    default: return false;
  }
}
}    // namespace templates
}    // namespace cpp_codec

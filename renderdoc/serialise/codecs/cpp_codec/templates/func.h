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

#include <cassert>
#include <cctype>
#include <sstream>
#include "common/common.h"

#include "value.h"

namespace cpp_codec
{
namespace templates
{
enum class ArgSpecFlags : uint32_t
{
  Any = 0x1,
  Convertible = 0x2,
  Optional = 0x4,
  VarArgs = 0x8,
  AllowError = 0x10,
};

struct ArgSpec
{
  ValueType type = ValueType::Null;
  uint32_t flags = 0;
  // Value *default = nullptr;
  inline ArgSpec() = default;
  inline ArgSpec(ValueType type) : type(type) {}
  inline ArgSpec(ValueType type, uint32_t flags) : type(type), flags(flags) {}
  inline static ArgSpec Any(bool allowError = false)
  {
    ArgSpec spec = ArgSpec(ValueType::Null, (uint32_t)ArgSpecFlags::Any);
    if(allowError)
      spec.flags |= (uint32_t)ArgSpecFlags::AllowError;
    return spec;
  }
  inline static ArgSpec Convertible(ValueType type)
  {
    return ArgSpec(type, (uint32_t)ArgSpecFlags::Convertible);
  }
  inline static ArgSpec VarArgs(ArgSpec spec)
  {
    spec.flags |= (uint32_t)ArgSpecFlags::VarArgs;
    return spec;
  }
  inline static ArgSpec Optional(ArgSpec base /*, const Value &default*/)
  {
    base.flags |= (uint32_t)ArgSpecFlags::Optional;
    // base.default = &default;
    return base;
  }
  bool IsAny() const { return (flags & (uint32_t)ArgSpecFlags::Any) != 0; }
  bool IsConvertible() const { return (flags & (uint32_t)ArgSpecFlags::Convertible) != 0; }
  bool IsOptional() const { return (flags & (uint32_t)ArgSpecFlags::Optional) != 0; }
  bool IsVarArgs() const { return (flags & (uint32_t)ArgSpecFlags::VarArgs) != 0; }
  bool IsErrorAllowed() const { return (flags & (uint32_t)ArgSpecFlags::AllowError) != 0; }
};

Value ArgError(const rdcstr &funcName, const rdcarray<ArgSpec> &spec, const rdcarray<Value> &args);

// template <typename SpecIter>
// Value CheckArgs(const rdcstr &name, SpecIter specBegin, SpecIter specEnd, rdcarray<Value> &args)
//{
//  auto specIt = specBegin;
//  auto argIt = args.begin();
//  for(; specIt != specEnd && argIt != args.end(); ++specIt, ++argIt)
//  {
//    if(argIt->IsError())
//      return *argIt;
//    if((argIt->Type() != specIt->type) && !specIt->IsAny())
//    {
//      if(specIt->IsConvertible())
//      {
//        Value conv = argIt->To(specIt->type);
//        if(conv.IsError())
//          return conv;
//        *argIt = conv;
//      }
//      else
//      {
//        return ArgError(name, spec, args);
//      }
//    }
//  }
//  if(argIt != args.end())
//    return ArgError(name, spec, args);
//  for(; specIt != specEnd; ++specIt)
//  {
//    if(specIt->IsOptional())
//      args.push_back(*specIt->default);
//    else
//      return ArgError(name, spec, args);
//  }
//  return Value();
//}
// inline Value CheckArgs(const rdcstr &name, const rdcarray<ArgSpec> &spec, rdcarray<Value> &args)
//{
//  return CheckArgs(name, spec.begin(), spec.end(), args);
//}

class State;

class DynFunc
{
  rdcstr mName;
  rdcarray<ArgSpec> mArgSpec;

protected:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args) = 0;
  inline Value CheckArgs(rdcarray<Value> &args);
  DynFunc(const rdcstr &name, const rdcarray<ArgSpec> &argSpec) : mName(name), mArgSpec(argSpec) {}

public:
  inline const rdcstr &Name() const { return mName; }
  Value Call(State *state, const Value &dot, rdcarray<Value> args);
};
}    // namespace templates
}    // namespace cpp_codec

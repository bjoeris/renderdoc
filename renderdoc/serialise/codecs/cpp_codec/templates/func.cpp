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

#include "func.h"
#include "exec.h"

namespace cpp_codec
{
namespace templates
{
// Value cpp_codec::templates::ArgError(const rdcstr &funcName, const rdcarray<ArgSpec> &spec,
//                                     const rdcarray<Value> &args)
//{
//  std::ostringstream ostr;
//  ostr << "In function \"" << funcName << "\": Received arguments types: (";
//  bool first = true;
//  for(auto it = args.begin(); it != args.end(); ++it)
//  {
//    if(!first)
//      ostr << ", ";
//    first = false;
//    ostr << ToStr(it->Type());
//  }
//  ostr << ") ";
//  ostr << "Expected types: (";
//  first = true;
//  for(auto it = spec.begin(); it != spec.end(); ++it)
//  {
//    if(!first)
//      ostr << ", ";
//    first = false;
//    ostr << ToStr(it->type);
//  }
//  ostr << ")";
//  return Value::Error(ostr.str().c_str());
//}

Value DynFunc::Call(State *state, const Value &dot, rdcarray<Value> args)
{
  Value err = CheckArgs(args);
  if(err.IsError())
    return err;
  return DoCall(state, dot, args);
}

Value DynFunc::CheckArgs(rdcarray<Value> &args)
{
  auto specIt = mArgSpec.begin();
  auto argIt = args.begin();
  for(; specIt != mArgSpec.end() && argIt != args.end(); ++argIt)
  {
    if(argIt->IsError() && !specIt->IsErrorAllowed())
      return *argIt;
    if((argIt->Type() != specIt->type) && !specIt->IsAny())
    {
      if(specIt->IsConvertible())
      {
        Value conv = argIt->To(specIt->type);
        if(conv.IsError())
          return conv;
        *argIt = conv;
      }
      else
      {
        return ArgError(mName, mArgSpec, args);
      }
    }
    if(!specIt->IsVarArgs())
      ++specIt;
  }
  if(argIt != args.end())
    return ArgError(mName, mArgSpec, args);
  for(; specIt != mArgSpec.end(); ++specIt)
  {
    if((!specIt->IsOptional()) && (!specIt->IsVarArgs()))
      return ArgError(mName, mArgSpec, args);
  }
  return Value();
}

Value ArgError(const rdcstr &funcName, const rdcarray<ArgSpec> &spec, const rdcarray<Value> &args)
{
  std::ostringstream ostr;
  ostr << "In function \"" << funcName << "\": Received arguments types: (";
  bool first = true;
  for(auto it = args.begin(); it != args.end(); ++it)
  {
    if(!first)
      ostr << ", ";
    first = false;
    ostr << ToStr(it->Type());
  }
  ostr << ") ";
  ostr << "Expected types: (";
  first = true;
  for(auto it = spec.begin(); it != spec.end(); ++it)
  {
    if(!first)
      ostr << ", ";
    first = false;
    if(it->IsConvertible())
      ostr << "ConvertibleTo(" << ToStr(it->type) << ")";
    else if(it->IsAny())
      ostr << "Any";
    else
      ostr << ToStr(it->type);
    if(it->IsOptional())
      ostr << "?";
    if(it->IsVarArgs())
      ostr << "*";
  }
  ostr << ")";
  return Value::Error(ostr.str().c_str());
}

// Value cpp_codec::templates::CheckArgs(const rdcstr &name, const rdcarray<ArgSpec> &spec,
//                                      rdcarray<Value> &args)
//{
//  auto specIt = spec.begin();
//  auto argIt = args.begin();
//  for(; specIt != spec.end() && argIt != args.end(); ++specIt, ++argIt)
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
//  for(; specIt != spec.end(); ++specIt)
//  {
//    if(specIt->IsOptional())
//      args.push_back(*specIt->default);
//    else
//      return ArgError(name, spec, args);
//  }
//  return Value();
//}
}    // namespace templates
}    // namespace cpp_codec

template <>
rdcstr DoStringise(const cpp_codec::templates::ArgSpec &el)
{
  if(el.flags & (uint32_t)cpp_codec::templates::ArgSpecFlags::Any)
    return "Any";
  else if(el.flags & (uint32_t)cpp_codec::templates::ArgSpecFlags::Convertible)
    return "ConvertableTo<" + ToStr(el.type) + ">";
  else
    return ToStr(el.type);
}

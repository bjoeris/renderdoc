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

#include "cpp_templates.h"
#include "exec.h"

#include <set>

namespace cpp_codec
{
namespace templates
{
class SetGlobalFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    state->Template().SetGlobal(args[0].GetString(), args[1]);
    return Value();
  }

public:
  SetGlobalFunc() : DynFunc("SetGlobal", {ValueType::String, ArgSpec::Any()}) {}
};
class GetGlobalFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return state->Template().GetGlobal(args[0].GetString());
  }

public:
  GetGlobalFunc() : DynFunc("GetGlobal", {ValueType::String}) {}
};

class NotFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return Value(!args[0].GetBool());
  }

public:
  NotFunc() : DynFunc("Not", {ArgSpec::Convertible(ValueType::Bool)}) {}
};
class OrFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.empty())
      return Value(false);
    for(auto it = args.begin(); it != args.end(); ++it)
    {
      Value b = it->ToBool();
      if(b.IsError())
        return b;
      if(b.GetBool())
        return *it;
    }
    return args.back();
  }

public:
  OrFunc() : DynFunc("Or", {ArgSpec::VarArgs(ArgSpec::Any(true))}) {}
};
class AndFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.empty())
      return Value(true);
    Value err;
    for(auto it = args.begin(); it != args.end(); ++it)
    {
      Value b = it->ToBool();
      if(b.IsError())
        return b;
      if(!b.GetBool())
        return *it;
    }
    return args.back();
  }

public:
  AndFunc() : DynFunc("And", {ArgSpec::VarArgs(ArgSpec::Any(true))}) {}
};
class EqFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.size() < 2)
      return Value(true);
    Value arg0 = args.front();
    auto it = args.begin();
    for(++it; it != args.end(); ++it)
      if(arg0 != *it)
        return Value(false);
    return Value(true);
  }

public:
  EqFunc() : DynFunc("Eq", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};
class NeFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return Value(args[0] != args[1]);
  }

public:
  NeFunc() : DynFunc("Ne", {ArgSpec::Any(), ArgSpec::Any()}) {}
};
class LeFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.size() < 2)
      return Value(true);
    Value arg = args.front();
    auto it = args.begin();
    for(++it; it != args.end(); ++it)
    {
      if(!(arg <= *it))
        return Value(false);
      arg = *it;
    }
    return Value(true);
  }

public:
  LeFunc() : DynFunc("Le", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};
class GeFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.size() < 2)
      return Value(true);
    Value arg = args.front();
    auto it = args.begin();
    for(++it; it != args.end(); ++it)
    {
      if(!(arg >= *it))
        return Value(false);
      arg = *it;
    }
    return Value(true);
  }

public:
  GeFunc() : DynFunc("Ge", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};
class LtFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.size() < 2)
      return Value(true);
    Value arg = args.front();
    auto it = args.begin();
    for(++it; it != args.end(); ++it)
    {
      if(!(arg < *it))
        return Value(false);
      arg = *it;
    }
    return Value(true);
  }

public:
  LtFunc() : DynFunc("Lt", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};
class GtFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.size() < 2)
      return Value(true);
    Value arg = args.front();
    auto it = args.begin();
    for(++it; it != args.end(); ++it)
    {
      if(!(arg > *it))
        return Value(false);
      arg = *it;
    }
    return Value(true);
  }

public:
  GtFunc() : DynFunc("Gt", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};

class StartsWithFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.size() < 2)
      return Value(true);
    Value arg0 = args.front().ToString();
    if(arg0.IsError())
      return arg0;
    const rdcstr &str0 = arg0.GetString();
    auto it = args.begin();
    for(++it; it != args.end(); ++it)
    {
      Value arg = it->ToString();
      if(arg.IsError())
        return arg;
      const rdcstr &str = arg.GetString();
      if(!str.substrEq(0, str0))
        return Value(false);
      arg = *it;
    }
    return Value(true);
  }

public:
  StartsWithFunc()
      : DynFunc("StartsWith", {ArgSpec::Convertible(ValueType::String),
                               ArgSpec::VarArgs(ArgSpec::Convertible(ValueType::String))})
  {
  }
};

class EndsWithFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.size() < 2)
      return Value(true);
    Value arg0 = args.front().ToString();
    if(arg0.IsError())
      return arg0;
    const rdcstr &str0 = arg0.GetString();
    auto it = args.begin();
    for(++it; it != args.end(); ++it)
    {
      Value arg = it->ToString();
      if(arg.IsError())
        return arg;
      const rdcstr &str = arg.GetString();
      if(!str.substrEq(str.size() - str0.size(), str0))
        return Value(false);
      arg = *it;
    }
    return Value(true);
  }

public:
  EndsWithFunc()
      : DynFunc("EndsWith", {ArgSpec::Convertible(ValueType::String),
                             ArgSpec::VarArgs(ArgSpec::Convertible(ValueType::String))})
  {
  }
};

class FindSubstringFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    const rdcstr &needle = args[0].GetString();
    const rdcstr &haystack = args[1].GetString();
    return Value((int64_t)haystack.find(needle));
  }

public:
  FindSubstringFunc()
      : DynFunc("FindSubstring",
                {ArgSpec::Convertible(ValueType::String), ArgSpec::Convertible(ValueType::String)})
  {
  }
};

class SubstringFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    uint64_t offs = args[0].GetUInt();
    uint64_t length = args[1].GetUInt();
    const rdcstr &str = args[2].GetString();

    return Value(str.substr(offs, length));
  }

public:
  SubstringFunc()
      : DynFunc("Substring",
                {ArgSpec::Convertible(ValueType::UInt), ArgSpec::Convertible(ValueType::UInt),
                 ArgSpec::Convertible(ValueType::String)})
  {
  }
};

class AddFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    uint64_t res = 0;
    for(auto it = args.begin(); it != args.end(); ++it)
    {
      Value arg = it->ToInt();
      if(arg.IsError())
        return arg;
      res += arg.GetInt();
    }
    return Value(res);
  }

public:
  AddFunc() : DynFunc("Add", {ArgSpec::VarArgs(ArgSpec::Convertible(ValueType::Int))}) {}
};

class SubtractFunc : public DynFunc
{
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    // Value err = CheckArgs("Substring", {ArgSpec::Convertible(ValueType::Int),
    // ArgSpec::VarArgs(ArgSpec::Convertible(ValueType::Int))}, args); if(err.IsError())
    //   return err;
    if(args.size() < 1)
      return Value::Error("Expected at least 1 argument, received %z", args.size());
    Value arg0 = args[0].ToInt();
    if(arg0.IsError())
      return arg0;
    int64_t res = arg0.GetInt();
    auto it = args.begin();
    for(++it; it != args.end(); ++it)
    {
      Value arg = it->ToInt();
      if(arg.IsError())
        return arg;
      res -= arg.GetInt();
    }
    return Value(res);
  }

public:
  SubtractFunc()
      : DynFunc("Subtract", {ArgSpec::Convertible(ValueType::Int),
                             ArgSpec::VarArgs(ArgSpec::Convertible(ValueType::Int))})
  {
  }
};

class ToUIntFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args) { return args[0]; }

public:
  ToUIntFunc() : DynFunc("ToUInt", {ArgSpec::Convertible(ValueType::UInt)}) {}
};

class ToStringFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args) { return args[0]; }

public:
  ToStringFunc() : DynFunc("ToString", {ArgSpec::Convertible(ValueType::String)}) {}
};

class PropFunc : public DynFunc
{
  std::function<bool(const Value &)> predicate;
  // virtual bool CheckProp(Value val) = 0;

  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    for(auto it = args.begin(); it != args.end(); ++it)
    {
      if(it->IsError())
        return *it;
      else if(!predicate(*it))
        return Value(false);
    }
    return Value(true);
  }

public:
  PropFunc(const rdcstr &name, std::function<bool(const Value &)> pred)
      : DynFunc(name, {ArgSpec::VarArgs(ArgSpec::Any())}), predicate(pred)
  {
  }
};

// class IsSDObjectFunc : public PropFunc
//{
//  virtual bool CheckProp(Value val) { return val.IsSDObject(); }
//
// public:
//  IsSDObjectFunc() : PropFunc("IsSDObject") {}
//};
//
// class IsSDUnionFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsUnion()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDUnionFunc() : DynFunc("IsSDUnion") {}
//};
//
// class IsSDStructFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsStruct()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDStructFunc() : DynFunc("IsSDStruct") {}
//};
//
// class IsSDArrayFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsArray()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDArrayFunc() : DynFunc("IsSDArray") {}
//};
//
// class IsSDPointerFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsPointer()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDPointerFunc() : DynFunc("IsSDPointer") {}
//};
//
// class IsSDResourceFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsResource()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDResourceFunc() : DynFunc("IsSDResource") {}
//};
//
// class IsSDBufferFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsBuffer()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDBufferFunc() : DynFunc("IsSDBuffer") {}
//};
//
// class IsInlineableFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsInlineable()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsInlineableFunc() : DynFunc("IsInlineable") {}
//};
// class IsHiddenFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsHidden()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsHiddenFunc() : DynFunc("IsHidden") {}
//};
//
// class IsSDNullFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsNULL()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDNullFunc() : DynFunc("IsSDNull") {}
//};
//
// class IsSDEnumFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsEnum()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDEnumFunc() : DynFunc("IsSDEnum") {}
//};
//
// class IsSDStringFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsString()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDStringFunc() : DynFunc("IsSDString") {}
//};
//
// class IsSDIntFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsInt()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDIntFunc() : DynFunc("IsSDInt") {}
//};
//
// class IsSDInt64Func : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsInt64()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDInt64Func() : DynFunc("IsSDInt64") {}
//};
//
// class IsSDUIntFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsUInt()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDUIntFunc() : DynFunc("IsSDUInt") {}
//};
//
// class IsSDUInt64Func : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsUInt64()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDUInt64Func() : DynFunc("IsSDUInt64") {}
//};
//
// class IsSDFloat32Func : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().IsFloat32()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  IsSDFloat32Func() : DynFunc("IsSDFloat32") {}
//};
//
// class HasCustomStringFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
//  {
//    for(auto it = args.begin(); it != args.end(); ++it)
//    {
//      if(it->IsError())
//        return *it;
//      else if((!it->IsSDObject()) || (!it->GetSDObject().HasCustomString()))
//        return Value(false);
//    }
//    return Value(true);
//  }
//
// public:
//  HasCustomStringFunc() : DynFunc("HasCustomString") {}
//};

class ModiySDObjectFunc : public DynFunc
{
  virtual void DoModify(Value::WrappedSDObject &obj, const rdcarray<Value> &args) = 0;
  Value Modify(const Value &obj, const rdcarray<rdcstr> &path, size_t index,
               const rdcarray<Value> &args)
  {
    if(index >= path.size())
    {
      if(!obj.IsSDObject())
        return Value::Error("In function \"%s\": Expected SDObject, found %s", Name(), obj.Type());
      Value::WrappedSDObject newObj(obj.GetSDObject());
      DoModify(newObj, args);
      return Value(std::move(newObj));
    }
    else
    {
      Value fieldVal = obj.Field(path[index]);
      if(fieldVal.IsNull())
      {
        std::ostringstream ostr;
        ostr << "In function \"" << Name() << "\": Missing field ";
        size_t i = 0;
        for(; i != index; ++i)
          ostr << path[i] << '.';
        ostr << path[index];
        return Value::Error(ostr.str().c_str());
      }
      Value newFieldVal = Modify(fieldVal, path, index + 1, args);
      if(newFieldVal.IsError())
        return newFieldVal;
      return obj.ReplaceField(path[index], newFieldVal);
    }
  }

  static const rdcarray<ArgSpec> BuildArgSpec(const std::initializer_list<ArgSpec> &additionalArgs)
  {
    rdcarray<ArgSpec> res = {ArgSpec::Any(), ValueType::String};
    for(auto it = additionalArgs.begin(); it != additionalArgs.end(); ++it)
      res.push_back(*it);
    return res;
  }

protected:
  ModiySDObjectFunc(const rdcstr &name, const std::initializer_list<ArgSpec> &additionalArgs)
      : DynFunc(name, BuildArgSpec(additionalArgs))
  {
  }
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    rdcarray<rdcstr> path = args[1].GetString().split('.');
    return Modify(args[0], path, 0, args);
  }
};

class MakePointerFunc : public ModiySDObjectFunc
{
  virtual void DoModify(Value::WrappedSDObject &obj, const rdcarray<Value> &args)
  {
    obj.type().flags |= SDTypeFlags::Nullable;
  }

public:
  MakePointerFunc() : ModiySDObjectFunc("MakePointer", {}) {}
};

class SetCustomStringFunc : public ModiySDObjectFunc
{
  virtual void DoModify(Value::WrappedSDObject &obj, const rdcarray<Value> &args)
  {
    obj.type().flags |= SDTypeFlags::HasCustomString;
    obj.data().str = args[2].GetString();
  }

public:
  SetCustomStringFunc()
      : ModiySDObjectFunc("SetCustomString", {ArgSpec::Convertible(ValueType::String)})
  {
  }
};

class RangeFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    Value::Array arr;
    uint64_t count = args[0].GetUInt();
    for(uint64_t i = 0; i < count; ++i)
      arr.push_back(i);
    return arr;
  }

public:
  RangeFunc() : DynFunc("Range", {ArgSpec::Convertible(ValueType::UInt)}) {}
};

class SizeFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return Value(args[0].size());
  }

public:
  SizeFunc() : DynFunc("Size", {ArgSpec::Any()}) {}
};

class EscapeFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    Value err;
    std::ostringstream ostr;
    for(auto it = args.begin(); it != args.end(); ++it)
    {
      Value s = it->ToString();
      if(s.IsError())
        return s;
      ostr << parse::escape(s.GetString());
    }
    return Value(ostr.str());
  }

public:
  EscapeFunc() : DynFunc("Escape", {ArgSpec::VarArgs(ArgSpec::Convertible(ValueType::String))}) {}
};

class CaninonicalUnionBranchFunc : public DynFunc    // Func<uint64_t, SDObject *>
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return Value((uint64_t)0);
  }
  // virtual uint64_t Call(SDObject *obj) { return 0; }
public:
  CaninonicalUnionBranchFunc() : DynFunc("CaninonicalUnionBranch", {ValueType::SDObject}) {}
};

class RemoveHiddenFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    const Value::WrappedSDObject &oldObj = args[0].GetSDObject();
    Value::WrappedSDObject newObj(oldObj);
    newObj.ClearChildren();
    for(auto it = oldObj.begin(); it != oldObj.end(); ++it)
    {
      if(it->second.IsSDObject() && it->second.GetSDObject().IsHidden())
        continue;
      newObj.AddChild(it->first, it->second);
    }
    return Value(newObj);
  }

public:
  RemoveHiddenFunc() : DynFunc("RemoveHidden", {ValueType::SDObject}) {}
};

class RemoveDuplicatesFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    const Value::WrappedSDObject &oldObj = args[0].GetSDObject();
    if(!oldObj.IsStruct())
      return args[0];
    Value::WrappedSDObject newObj(oldObj);
    newObj.ClearChildren();
    std::set<rdcstr> seenNames;
    for(auto it = oldObj.begin(); it != oldObj.end(); ++it)
    {
      if(it->second.IsSDObject())
      {
        rdcstr name = it->second.GetSDObject().name();
        if(seenNames.find(name) != seenNames.end())
          continue;
        else
          seenNames.insert(name);
      }
      newObj.AddChild(it->first, it->second);
    }
    return Value(newObj);
  }

public:
  RemoveDuplicatesFunc() : DynFunc("RemoveDuplicates", {ValueType::SDObject}) {}
};

class PrintFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    StringWriter sw;
    for(auto it = args.begin(); it != args.end(); ++it)
      sw.Print(*it);
    return Value(sw.str());
  }

public:
  PrintFunc() : DynFunc("Print", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};

class LogFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    StringWriter sw;
    for(auto it = args.begin(); it != args.end(); ++it)
      sw.Print(*it);
    RDCLOG(sw.str().c_str());
    return Value();
  }

public:
  LogFunc() : DynFunc("Log", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};

class NameFunc : public DynFunc    // Func<rdcstr, SDObject *>
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return Value(args[0].GetSDObject().name());
  }
  // virtual rdcstr Call(SDObject *obj) { return obj->name; }
public:
  NameFunc() : DynFunc("Name", {ValueType::SDObject}) {}
};    // namespace templates

class TypeFunc : public DynFunc    // Func<rdcstr, SDObject *>
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return Value(args[0].GetSDObject().type().name);
  }
  // virtual rdcstr Call(SDObject *obj) { return obj->type.name; }
public:
  TypeFunc() : DynFunc("Type", {ValueType::SDObject}) {}
};

class MacroFunc : public DynFunc    // Func<rdcstr, SDObject *>
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    rdcstr name = args[0].GetString();
    Value newDot;
    if(args.size() >= 2)
      newDot = args[1];
    else
      newDot = dot;
    return state->Macro(name, newDot);
  }
  // virtual rdcstr Call(SDObject *obj) { return obj->type.name; }
public:
  MacroFunc() : DynFunc("Macro", {ValueType::String, ArgSpec::Optional(ArgSpec::Any())}) {}
};

class DictFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    if(args.size() % 2 != 0)
      return Value::Error(
          "Dict expects an even number of arguments, alternating between name strings "
          "and "
          "values");
    Value::Dict dict;
    Value err;
    for(auto it = args.begin(); it != args.end(); ++it)
    {
      Value name = it->ToString();
      if(name.IsError())
        return name;
      ++it;
      dict[name.GetString()] = *it;
    }
    return Value(std::move(dict));
  }

public:
  DictFunc() : DynFunc("Dict", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};

class ArrayFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    Value::Array array;
    Value err;
    for(auto it = args.begin(); it != args.end(); ++it)
    {
      if(it->IsError())
        return *it;
      array.push_back(*it);
    }
    return Value(std::move(array));
  }

public:
  ArrayFunc() : DynFunc("Array", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};

class IndexFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return args[1].Index(args[0].GetUInt());
  }

public:
  IndexFunc() : DynFunc("Index", {ArgSpec::Convertible(ValueType::UInt), ArgSpec::Any()}) {}
};

// class OverlayFunc : public DynFunc
//{
// public:
//  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args) { return args[0]; }
//
// public:
//  OverlayFunc() : DynFunc("Overlay") {}
//};

class PushPrefixFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    StringWriter prefixWriter;
    for(auto it = args.begin(); it != args.end(); ++it)
      prefixWriter.Print(*it);
    state->writer().PushPrefix(prefixWriter.str());
    return Value();
  }

public:
  PushPrefixFunc() : DynFunc("PushPrefix", {ArgSpec::VarArgs(ArgSpec::Any())}) {}
};

class PopPrefixFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    state->writer().PopPrefix();
    return Value();
  }

public:
  PopPrefixFunc() : DynFunc("PopPrefix", {}) {}
};

class SuppressSpaceFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return Value(rdcstr(&ReflowWriter::SUPPRESS_WHITESPACE, 1));
  }

public:
  SuppressSpaceFunc() : DynFunc("S", {}) {}
};

class ForceSpaceFunc : public DynFunc
{
public:
  virtual Value DoCall(State *state, const Value &dot, rdcarray<Value> args)
  {
    return Value(rdcstr(&ReflowWriter::FORCE_WHITESPACE, 1));
  }

public:
  ForceSpaceFunc() : DynFunc("_", {}) {}
};

class ExecTemplateFunc : public DynFunc
{
  Value DoCall(State *state, const templates::Value &dot, rdcarray<templates::Value> args)
  {
    rdcstr fileName = args[0].GetString();
    rdcstr templateName = args[1].GetString();
    Value newDot;
    if(args.size() >= 3)
      newDot = args[2];
    else
      newDot = dot;
    ReflowWriter *newOut = state->Template().GetFile(fileName);
    if(newOut == NULL)
      return Value::Error("Could not find file %s", fileName);
    return state->ExecTemplate(*newOut, templateName, newDot);
  }

public:
  ExecTemplateFunc()
      : DynFunc("ExecTemplate", {templates::ArgSpec::Convertible(templates::ValueType::String),
                                 templates::ArgSpec::Convertible(templates::ValueType::String),
                                 templates::ArgSpec::Optional(templates::ArgSpec::Any())})
  {
  }
};

class NewScratchFunc : public DynFunc
{
  Value DoCall(State *state, const templates::Value &dot, rdcarray<templates::Value> args)
  {
    std::ostringstream name;
    name << "SCRATCH:" << nextId;
    ++nextId;
    StringWriter *sw = new StringWriter();
    ReflowWriter *rw = new ReflowWriter(sw);
    state->Template().SetFile(name.str(), rw);
    return Value();
  }

  size_t nextId = 1000;

public:
  NewScratchFunc() : DynFunc("NewScratch", {}) {}
};

class DumpScratchFunc : public DynFunc
{
  Value DoCall(State *state, const templates::Value &dot, rdcarray<templates::Value> args)
  {
    const rdcstr &name = args[0].GetString();
    ReflowWriter *rw = state->Template().GetFile(name);
    state->Template().SetFile(name, NULL);
    rw->Close();
    StringWriter *sw = (StringWriter *)rw->inner();
    rdcstr str = sw->str();
    delete rw;
    delete sw;
    return str;
  }

  size_t nextId = 1000;

public:
  DumpScratchFunc() : DynFunc("DumpScratch", {ValueType::String}) {}
};

void AddCommonFuncs(Template &tmpl)
{
  tmpl.AddFunc(new SetGlobalFunc());
  tmpl.AddFunc(new GetGlobalFunc());
  tmpl.AddFunc(new NotFunc());
  tmpl.AddFunc(new OrFunc());
  tmpl.AddFunc(new AndFunc());
  tmpl.AddFunc(new EqFunc());
  tmpl.AddFunc(new NeFunc());
  tmpl.AddFunc(new GeFunc());
  tmpl.AddFunc(new LeFunc());
  tmpl.AddFunc(new GtFunc());
  tmpl.AddFunc(new LtFunc());
  tmpl.AddFunc(new StartsWithFunc());
  tmpl.AddFunc(new EndsWithFunc());
  tmpl.AddFunc(new FindSubstringFunc());
  tmpl.AddFunc(new SubstringFunc());
  tmpl.AddFunc(new AddFunc());
  tmpl.AddFunc(new SubtractFunc());
  tmpl.AddFunc(new ToUIntFunc());
  tmpl.AddFunc(new ToStringFunc());
  tmpl.AddFunc(new SetCustomStringFunc());
  tmpl.AddFunc(new MakePointerFunc());
  tmpl.AddFunc(new RangeFunc());
  tmpl.AddFunc(new SizeFunc());
  tmpl.AddFunc(new EscapeFunc());
  tmpl.AddFunc(new CaninonicalUnionBranchFunc());
  tmpl.AddFunc(new RemoveHiddenFunc());
  tmpl.AddFunc(new RemoveDuplicatesFunc());
  tmpl.AddFunc(new PrintFunc());
  tmpl.AddFunc(new DictFunc());
  tmpl.AddFunc(new ArrayFunc());
  tmpl.AddFunc(new IndexFunc());
  // tmpl.AddFunc(new OverlayFunc());
  tmpl.AddFunc(new NameFunc());
  tmpl.AddFunc(new TypeFunc());
  tmpl.AddFunc(new MacroFunc());
  tmpl.AddFunc(new PushPrefixFunc());
  tmpl.AddFunc(new PopPrefixFunc());
  tmpl.AddFunc(new SuppressSpaceFunc());
  tmpl.AddFunc(new ForceSpaceFunc());
  tmpl.AddFunc(new LogFunc());
  tmpl.AddFunc(new ExecTemplateFunc());
  tmpl.AddFunc(new NewScratchFunc());
  tmpl.AddFunc(new DumpScratchFunc());
  tmpl.AddFunc(new PropFunc("IsSDObject", [](const Value &v) -> bool { return v.IsSDObject(); }));
  tmpl.AddFunc(new PropFunc("IsSDUnion", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsUnion();
  }));
  tmpl.AddFunc(new PropFunc("IsSDUnion", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsUnion();
  }));
  tmpl.AddFunc(new PropFunc("IsSDStruct", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsStruct();
  }));
  tmpl.AddFunc(new PropFunc("IsSDArray", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsArray();
  }));
  tmpl.AddFunc(new PropFunc("IsSDPointer", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsPointer();
  }));
  tmpl.AddFunc(new PropFunc("IsSDResource", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsResource();
  }));
  tmpl.AddFunc(new PropFunc("IsSDBuffer", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsBuffer();
  }));
  tmpl.AddFunc(new PropFunc("IsInlineable", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsInlineable();
  }));
  tmpl.AddFunc(new PropFunc("IsHidden", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsHidden();
  }));
  tmpl.AddFunc(new PropFunc("IsSDNull", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsNULL();
  }));
  tmpl.AddFunc(new PropFunc("IsSDEnum", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsEnum();
  }));
  tmpl.AddFunc(new PropFunc("IsSDString", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsString();
  }));
  tmpl.AddFunc(new PropFunc(
      "IsSDInt", [](const Value &v) -> bool { return v.IsSDObject() && v.GetSDObject().IsInt(); }));
  tmpl.AddFunc(new PropFunc("IsSDInt64", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsInt64();
  }));
  tmpl.AddFunc(new PropFunc("IsSDUInt", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsUInt();
  }));
  tmpl.AddFunc(new PropFunc("IsSDUInt64", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsUInt64();
  }));
  tmpl.AddFunc(new PropFunc("IsSDFloat32", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().IsFloat32();
  }));
  tmpl.AddFunc(new PropFunc("HasCustomString", [](const Value &v) -> bool {
    return v.IsSDObject() && v.GetSDObject().HasCustomString();
  }));
}
}    // namespace templates
}    // namespace cpp_codec

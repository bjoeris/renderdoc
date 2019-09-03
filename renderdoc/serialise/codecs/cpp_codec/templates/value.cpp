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

#include "value.h"
#include "exec.h"

namespace cpp_codec
{
namespace templates
{
Value Value::Field(const rdcstr &name) const
{
  if(IsDict())
  {
    const Dict &dict = GetDict();
    auto it = dict.find(name);
    if(it == dict.end())
      return Value();
    // return Value::Error("Unknown field '%s' in dict", name);
    return it->second;
  }
  else if(IsSDObject())
  {
    return GetSDObject().FindChild(name);
  }
  else if(IsOverlay())
  {
    return Error("Overlay Field not implemented");
  }
  return Error("Attempting to access field '%s' on value of type %s", name, ToStr(Type()));
}
Value Value::ReplaceField(const rdcstr &name, const Value &val) const
{
  if(IsDict())
  {
    Dict dict(GetDict());
    auto it = dict.find(name);
    if(it == dict.end())
      dict[name] = val;
    else
      it->second = val;
    return Value(std::move(dict));
  }
  else if(IsSDObject())
  {
    Value::WrappedSDObject obj(GetSDObject());
    auto it = obj.find(name);
    if(it == obj.end())
      obj.AddChild(name, val);
    else
      it->second = val;
    return Value(std::move(obj));
  }
  return Error("Attempting to assign field '%s' on value of type %s", name, ToStr(Type()));
}
Value Value::Index(size_t index) const
{
  if(IsSDObject())
  {
    return GetSDObject().GetChild(index);
  }
  else if(IsArray())
  {
    const Value::Array &arr = GetArray();
    if(index >= arr.size())
      return Value();
    return arr[index];
  }
  return Error("Attempting to index on value of type %s", ToStr(Type()));
}
Value Value::Visit(std::function<Value(Value, Value, Value)> callback) const
{
  if(IsSDObject())
  {
    const WrappedSDObject &obj = GetSDObject();
    size_t index = 0;
    for(auto it = obj.begin(); it != obj.end(); ++it, ++index)
    {
      Value err = callback(Value(index), Value(it->first), Value(it->second));
      if(err.IsError())
        return err;
    }
    return Value();
  }
  else if(IsDict())
  {
    const Value::Dict &dict = GetDict();
    size_t index = 0;
    for(auto it = dict.begin(); it != dict.end(); ++it, ++index)
    {
      Value err = callback(Value(index), Value(it->first), Value(it->second));
      if(err.IsError())
        return err;
    }
    return Value();
  }
  else if(IsArray())
  {
    const Value::Array &array = GetArray();
    size_t index = 0;
    for(auto it = array.begin(); it != array.end(); ++it, ++index)
    {
      Value err = callback(Value(index), Value(index), Value(*it));
      if(err.IsError())
        return err;
    }
    return Value();
  }
  else if(IsOverlay())
  {
    return Error("Overlay Visit not implemented");
  }
  return Value();
}
size_t Value::size() const
{
  if(IsSDObject())
    return GetSDObject().NumChildren();
  else if(IsArray())
    return GetArray().size();
  else if(IsDict())
    return GetArray().size();
  else
    return 0;
}

// Iter begin() const
//{
//  if(IsSDObject())
//    return Iter(data.obj->begin());
//  else if(IsArray())
//    return Iter((*data.arr)->begin());
//  // else if(IsDict())
//  //  return Iter(data.dict->begin());
//  else
//    return Iter();
//};
// Iter end() const
//{
//  if(IsSDObject())
//    return Iter(data.obj->end());
//  else if(IsArray())
//    return Iter((*data.arr)->end());
//  // else if(IsDict())
//  //  return Iter(data.dict->end());
//  else
//    return Iter();
//};

void Value::AddRef()
{
  if(IsSDObject())
    data.obj->AddRef();
  else if(IsArray())
    data.arr->AddRef();
  else if(IsDict())
    data.dict->AddRef();
  else if(IsOverlay())
    data.overlay->AddRef();
  else if(IsString())
    data.str->AddRef();
  else if(IsError())
    data.err->AddRef();
}
Value::Value(const SDObject *obj)
{
  SetType(ValueType::SDObject);
  data.obj = new Shared<WrappedSDObject>(obj);
}
Value::Value(const WrappedSDObject &obj)
{
  SetType(ValueType::SDObject);
  data.obj = new Shared<WrappedSDObject>(obj);
}
Value::Value(WrappedSDObject &&obj)
{
  SetType(ValueType::SDObject);
  data.obj = new Shared<WrappedSDObject>(obj);
}
Value::Value(const Dict &dict)
{
  SetType(ValueType::Dict);
  data.dict = new Shared<Dict>(dict);
}
Value::Value(Dict &&dict)
{
  SetType(ValueType::Dict);
  // SetOwnership(Ownership::Owned);
  data.dict = new Shared<Dict>(dict);
}
Value::Value(const Array &arr)
{
  SetType(ValueType::Array);
  data.arr = new Shared<Array>(arr);
}
Value::Value(Array &&arr)
{
  SetType(ValueType::Array);
  data.arr = new Shared<Array>(arr);
}
Value::Value(const Overlay &overlay)
{
  SetType(ValueType::Overlay);
  data.overlay = new Shared<Overlay>(overlay);
}
Value::Value(const ExecError &err)
{
  SetType(ValueType::Error);
  data.err = new Shared<ExecError>(err);
}
Value::Value(ExecError &&err)
{
  SetType(ValueType::Error);
  data.err = new Shared<ExecError>(err);
}
Value::Value(const char *str)
{
  SetType(ValueType::String);
  data.str = new Shared<rdcstr>(str);
}
Value::Value(const rdcstr &str)
{
  SetType(ValueType::String);
  data.str = new Shared<rdcstr>(str);
}
Value::Value(rdcstr &&str)
{
  SetType(ValueType::String);
  data.str = new Shared<rdcstr>(str);
}
// void Value::DuplicateOwned()
//{
//  if(IsOwned())
//  {
//    if(IsSDObject())
//      data.obj = data.obj->Duplicate();
//    else if(IsArray())
//      data.arr = new Array(*data.arr);
//    else if(IsDict())
//      data.dict = new Dict(*data.dict);
//    else if(IsString())
//      data.str = new rdcstr(*data.str);
//    else if(IsError())
//      data.err = new ExecError(*data.err);
//  }
//}

// template <typename... Args>
// inline static Value Error(const rdcarray<Frame> stack, const char *fmt, Args... args)
//{
//  rdcstr msg;
//  msg.sprintf(fmt, args...);
//  Value v(std::move(msg));
//  v.SetType(ValueType::Error);
//  return v;
//}

inline void Value::Clear()
{
  if(IsSDObject())
    data.obj->RemoveRef();
  else if(IsArray())
    data.arr->RemoveRef();
  else if(IsDict())
    data.dict->RemoveRef();
  else if(IsOverlay())
    data.overlay->RemoveRef();
  else if(IsString())
    data.str->RemoveRef();
  else if(IsError())
    data.err->RemoveRef();
  memset(&data, 0, sizeof(data));
  // if(IsOwned())
  //{
  //  if(IsSDObject() && data.obj)
  //  {
  //    delete data.obj;
  //    data.obj = NULL;
  //  }
  //  else if(IsArray())
  //  {
  //    delete data.arr;
  //    data.arr = NULL;
  //  }
  //  else if(IsDict())
  //  {
  //    delete data.dict;
  //    data.dict = NULL;
  //  }
  //  else if(IsString())
  //  {
  //    delete data.str;
  //    data.str = NULL;
  //  }
  //  else if(IsError())
  //  {
  //    delete data.err;
  //    data.err = NULL;
  //  }
  //}
  mType = ValueType::Null;
}
inline Value Value::UnpackSDObject() const
{
  if(IsSDObject())
  {
    const WrappedSDObject &obj = GetSDObject();
    switch(obj.type().basetype)
    {
      // case SDBasic::Chunk:
      case SDBasic::Struct: return ToDict();
      case SDBasic::Array: return ToArray();
      case SDBasic::Null: return Value();
      case SDBasic::Buffer: return Value(obj.data().basic.u);
      case SDBasic::String: return Value(obj.data().str);
      case SDBasic::Enum: return Value(obj.data().basic.u);
      case SDBasic::UnsignedInteger: return Value(obj.data().basic.u);
      case SDBasic::SignedInteger: return Value(obj.data().basic.i);
      case SDBasic::Float: return Value(obj.data().basic.d);
      case SDBasic::Boolean: return Value(obj.data().basic.b);
      case SDBasic::Character: return Value(obj.data().basic.c);
      case SDBasic::Resource: return Value(obj.data().basic.u);
      default: return Error("Cannot unpack SDObject of type '%s'.", ToStr(obj.type().basetype));
    }
  }
  return *this;
}
void Value::compare(const Value &other, bool &lt, bool &eq, bool &gt) const
{
  lt = eq = gt = false;

  if(IsError() || other.IsError())
    return;
  if(IsSDObject())
  {
    Value a = UnpackSDObject();
    if(a.IsError())
      return;
    return a.compare(other, lt, eq, gt);
  }
  if(other.IsSDObject())
  {
    Value b = other.UnpackSDObject();
    if(b.IsError())
      // cannot unpack SDObject in `other`, and we know `this` is not an SDObject.
      return;
    return compare(b, lt, eq, gt);
  }

  switch(Type())
  {
    case ValueType::String:
      if(other.IsString())
      {
        eq = GetString() == other.GetString();
        lt = GetString() < other.GetString();
        gt = GetString() > other.GetString();
      }
      break;
    case ValueType::Bool: eq = other.IsBool() && GetBool() == other.GetBool(); break;
    case ValueType::Int:
      if(other.IsInt())
      {
        eq = GetInt() == other.GetInt();
        lt = GetInt() < other.GetInt();
        gt = GetInt() > other.GetInt();
      }
      else if(other.IsUInt())
      {
        eq = GetInt() >= 0 && (uint64_t)GetInt() == other.GetUInt();
        lt = GetInt() < 0 || (uint64_t)GetInt() < other.GetUInt();
        gt = GetInt() >= 0 && (uint64_t)GetInt() > other.GetUInt();
      }
      else if(other.IsChar())
      {
        eq = GetInt() == (int64_t)other.GetChar();
        lt = GetInt() < (int64_t)other.GetChar();
        gt = GetInt() > (int64_t)other.GetChar();
      }
      break;
    case ValueType::UInt:
      if(other.IsUInt())
      {
        eq = GetUInt() == other.GetUInt();
        lt = GetUInt() < other.GetUInt();
        gt = GetUInt() > other.GetUInt();
      }
      else if(other.IsInt())
      {
        eq = other.GetInt() >= 0 && GetUInt() == (uint64_t)other.GetInt();
        lt = other.GetInt() >= 0 && GetUInt() < (uint64_t)other.GetInt();
        gt = other.GetInt() < 0 || GetUInt() > (uint64_t)other.GetInt();
      }
      else if(other.IsChar())
      {
        eq = other.GetChar() >= 0 && GetUInt() == (uint64_t)other.GetChar();
        lt = other.GetChar() >= 0 && GetUInt() < (uint64_t)other.GetChar();
        gt = other.GetChar() < 0 || GetUInt() > (uint64_t)other.GetChar();
      }
      break;
    case ValueType::Float:
      if(other.IsFloat())
      {
        eq = GetFloat() == other.GetFloat();
        lt = GetFloat() < other.GetFloat();
        gt = GetFloat() > other.GetFloat();
      }
      break;
    case ValueType::Char:
      if(other.IsUInt())
      {
        eq = GetChar() >= 0 && (uint64_t)GetChar() == other.GetUInt();
        lt = GetChar() < 0 || (uint64_t)GetChar() < other.GetUInt();
        gt = GetChar() >= 0 && (uint64_t)GetChar() > other.GetUInt();
      }
      if(other.IsInt())
      {
        eq = (int64_t)GetChar() == other.GetInt();
        lt = (int64_t)GetChar() < other.GetInt();
        gt = (int64_t)GetChar() > other.GetInt();
      }
      else if(other.IsChar())
      {
        eq = GetChar() == other.GetChar();
        lt = GetChar() < other.GetChar();
        gt = GetChar() > other.GetChar();
      }
      break;
    default: break;
  }
}
const Value::WrappedSDObject &Value::GetSDObject() const
{
  return **data.obj;
}
const Value::Dict &Value::GetDict() const
{
  return **data.dict;
}
// Value Value::ToSDObject(const rdcstr &name, bool rename) const
//{
//  switch(Type())
//  {
//    case ValueType::Null:
//      return Value(new SDObject(name, SDType::NullPointer("void")), Ownership::Owned);
//    case ValueType::SDObject:
//      if((!rename) || name == data.obj->name)
//        return *this;
//      else
//      {
//        SDObject *obj = data.obj->Duplicate();
//        obj->name = name;
//        return Value(obj, Ownership::Owned);
//      }
//    case ValueType::String:
//    {
//      SDObject *obj = new SDObject(name, SDType::String());
//      obj->data.str = std::move(GetString());
//      Value(obj, Ownership::Owned);
//    }
//    case ValueType::Bool:
//    {
//      SDObject *obj = new SDObject(name, SDType::Bool());
//      obj->data.basic.b = GetBool();
//      Value(obj, Ownership::Owned);
//    }
//    case ValueType::Int:
//    {
//      SDObject *obj = new SDObject(name, SDType::Int64());
//      obj->data.basic.i = GetInt();
//      Value(obj, Ownership::Owned);
//    }
//    case ValueType::UInt:
//    {
//      SDObject *obj = new SDObject(name, SDType::UInt64());
//      obj->data.basic.u = GetUInt();
//      Value(obj, Ownership::Owned);
//    }
//    case ValueType::Float:
//    {
//      SDObject *obj = new SDObject(name, SDType::Double());
//      obj->data.basic.d = GetFloat();
//      Value(obj, Ownership::Owned);
//    }
//    case ValueType::Char:
//    {
//      SDObject *obj = new SDObject(name, SDType::Char());
//      obj->data.basic.d = GetFloat();
//      Value(obj, Ownership::Owned);
//    }
//    case ValueType::Dict:
//    {
//      SDObject *obj = new SDObject(name, "dict");
//      const Dict &dict = GetDict();
//      for(auto it = dict.begin(); it != dict.end(); ++it)
//      {
//        SDObject *child = NULL;
//        ;
//        if(it->second.IsSDObject())
//        {
//          child = it->second.GetSDObject();
//          child = child->Duplicate();
//          child->name = it->first;
//        }
//        else
//        {
//          Value childVal = it->second.ToSDObject(it->first);
//          child = childVal.GetSDObject();
//
//          // ownership of `child` is transfered from the temporary value `childVal` to the new
//          // parent `obj`.
//          childVal.SetOwnership(Ownership::Borrowed);
//        }
//        obj->data.children.push_back(child);
//      }
//      Value(obj, Ownership::Owned);
//    }
//    case ValueType::Array:
//    {
//      SDObject *obj = new SDObject(name, "array");
//      obj->type.basetype = SDBasic::Array;
//      const Array &arr = GetArray();
//      for(auto it = arr.begin(); it != arr.end(); ++it)
//      {
//        SDObject *child = NULL;
//        ;
//        if(it->IsSDObject())
//        {
//          child = it->GetSDObject();
//          child = child->Duplicate();
//          child->name = "$el";
//        }
//        else
//        {
//          Value childVal = it->ToSDObject("$el");
//          child = childVal.GetSDObject();
//
//          // ownership of `child` is transfered from the temporary value `childVal` to the new
//          // parent `obj`.
//          childVal.SetOwnership(Ownership::Borrowed);
//        }
//        obj->data.children.push_back(child);
//      }
//      Value(obj, Ownership::Owned);
//    }
//    case ValueType::Error: return *this;
//    default: return Error("Unknown ValueType %s", ToStr(Type()));
//  }
//}
Value Value::ToDict() const
{
  switch(Type())
  {
    case ValueType::Dict: return *this;
    case ValueType::Null: return Value(Dict());
    case ValueType::SDObject:
    {
      const WrappedSDObject &obj = GetSDObject();
      Dict dict;
      for(auto it = obj.begin(); it != obj.end(); ++it)
      {
        dict[it->first] = it->second;
      }
      return Value(std::move(dict));
    }
    // case ValueType::String:
    // case ValueType::Bool:
    // case ValueType::Int:
    // case ValueType::UInt:
    // case ValueType::Float:
    // case ValueType::Char:
    // case ValueType::Array:
    // case ValueType::Error:
    default: return Error("Cannot convert value of type %s to type dict", ToStr(Type()));
  }
}
const Value::Array &Value::GetArray() const
{
  return **data.arr;
}
Value Value::ToArray() const
{
  switch(Type())
  {
    case ValueType::Array: return *this;
    case ValueType::Null: return Value(Array());
    case ValueType::SDObject:
    {
      const WrappedSDObject &obj = GetSDObject();
      Array arr;
      for(auto it = obj.begin(); it != obj.end(); ++it)
      {
        arr.push_back(Value(it->second));
      }
      return Value(std::move(arr));
    }
    // case ValueType::String:
    // case ValueType::Bool:
    // case ValueType::Int:
    // case ValueType::UInt:
    // case ValueType::Float:
    // case ValueType::Char:
    // case ValueType::Dict:
    // case ValueType::Error:
    default: return Error("Cannot convert value of type %s to type Array", ToStr(Type()));
  }
}
const Value::Overlay &Value::GetOverlay() const
{
  return **data.overlay;
}
const rdcstr &Value::GetString() const
{
  return **data.str;
}
Value Value::ToString() const
{
  switch(Type())
  {
    case ValueType::String: return *this;
    case ValueType::Null: return Value("");
    case ValueType::SDObject:
      if(GetSDObject().HasCustomString())
        return GetSDObject().data().str;
      else
        return UnpackSDObject().ToString();
    case ValueType::Bool: return ToStr(GetBool());
    case ValueType::Int: return ToStr(GetInt());
    case ValueType::UInt: return ToStr(GetUInt());
    case ValueType::Float: return ToStr(GetFloat());
    case ValueType::Char: return ToStr(GetChar());
    // case ValueType::Dict: return ToStr(GetDict());
    // case ValueType::Array: return ToStr(GetArray());
    case ValueType::Error: return *this;
    default: return Error("Cannot convert value of type %s to type String", ToStr(Type()));
  }
}
Value Value::ToBool() const
{
  switch(Type())
  {
    case ValueType::String:
    {
      rdcstr str = GetString();
      return Value(!(str.empty() || str == "false"));
    }
    case ValueType::Null: return Value(false);
    case ValueType::SDObject: return UnpackSDObject().ToBool();
    case ValueType::Bool: return *this;
    case ValueType::Int: return Value(GetInt() != 0);
    case ValueType::UInt: return Value(GetUInt() != 0);
    case ValueType::Float: return Value(GetFloat() != 0);
    case ValueType::Char: return Value(GetChar() != 0);
    case ValueType::Dict: return Value(!GetDict().empty());
    case ValueType::Array: return Value(!GetArray().empty());
    case ValueType::Error: return *this;
    default: return Error("Cannot convert value of type %s to type Bool", ToStr(Type()));
  }
}
Value Value::ToInt() const
{
  switch(Type())
  {
    case ValueType::String:
    {
      rdcstr str = GetString();
      std::istringstream istr(str);
      int64_t i;
      istr >> i;
      if(istr.fail())
        return Error("Failed to parse \"%s\" as a signed integer", str);
      if(!istr.eof())
        return Error("Failed to parse \"%s\" as a signed integer (trailing characters)", str);
      return Value(i);
    }
    case ValueType::Null: return Value((int64_t)0);
    case ValueType::SDObject: return UnpackSDObject().ToInt();
    case ValueType::Bool:
      if(GetBool())
        return Value((int64_t)1);
      else
        return Value((int64_t)0);
    case ValueType::Int: return *this;
    case ValueType::UInt: return Value((int64_t)GetUInt());
    case ValueType::Float: return Value((int64_t)GetFloat());
    case ValueType::Char: return Value((int64_t)GetChar());
    // case ValueType::Dict:
    // case ValueType::Array:
    case ValueType::Error: return *this;
    default: return Error("Cannot convert value of type %s to type Int", ToStr(Type()));
  }
}
Value Value::ToUInt() const
{
  switch(Type())
  {
    case ValueType::String:
    {
      rdcstr str = GetString();
      std::istringstream istr(str);
      uint64_t i;
      istr >> i;
      if(istr.fail())
        return Error("Failed to parse \"%s\" as a signed integer", str);
      if(!istr.eof())
        return Error("Failed to parse \"%s\" as a signed integer (trailing characters)", str);
      return Value(i);
    }
    case ValueType::Null: return Value((uint64_t)0);
    case ValueType::SDObject: return UnpackSDObject().ToUInt();
    case ValueType::Bool:
      if(GetBool())
        return Value((uint64_t)1);
      else
        return Value((uint64_t)0);
    case ValueType::Int: return Value((uint64_t)GetInt());
    case ValueType::UInt: return *this;
    case ValueType::Float: return Value((uint64_t)GetFloat());
    case ValueType::Char: return Value((uint64_t)GetChar());
    // case ValueType::Dict:
    // case ValueType::Array:
    case ValueType::Error: return *this;
    default: return Error("Cannot convert value of type %s to type UInt", ToStr(Type()));
  }
}
Value Value::ToFloat() const
{
  switch(Type())
  {
    case ValueType::String:
    {
      rdcstr str = GetString();
      std::istringstream istr(str);
      double x;
      istr >> x;
      if(istr.fail())
        return Error("Failed to parse \"%s\" as a signed integer", str);
      if(!istr.eof())
        return Error("Failed to parse \"%s\" as a signed integer (trailing characters)", str);
      return Value(x);
    }
    case ValueType::Null: return Value((double)0);
    case ValueType::SDObject: return UnpackSDObject().ToFloat();
    case ValueType::Bool:
      if(GetBool())
        return Value((double)1);
      else
        return Value((double)0);
    case ValueType::Int: return Value((double)GetInt());
    case ValueType::UInt: return Value((double)GetUInt());
    case ValueType::Float: return *this;
    case ValueType::Char: return Value((double)GetChar());
    // case ValueType::Dict:
    // case ValueType::Array:
    case ValueType::Error: return *this;
    default: return Error("Cannot convert value of type %s to type Float", ToStr(Type()));
  }
}
Value Value::ToChar() const
{
  switch(Type())
  {
    // case ValueType::String:
    case ValueType::Null: return Value('\0');
    case ValueType::SDObject: return UnpackSDObject().ToChar();
    case ValueType::Bool:
      if(GetBool())
        return Value('\1');
      else
        return Value('\0');
    case ValueType::Int: return Value((char)GetInt());
    case ValueType::UInt: return Value((char)GetUInt());
    // case ValueType::Float: return *this;
    case ValueType::Char: return *this;
    // case ValueType::Dict:
    // case ValueType::Array:
    case ValueType::Error: return *this;
    default: return Error("Cannot convert value of type %s to type Char", ToStr(Type()));
  }
}
const ExecError &Value::GetError() const
{
  return **data.err;
}
ExecError &Value::GetError()
{
  return **data.err;
}
Value Value::To(ValueType type) const
{
  if(type == Type())
    return *this;
  switch(type)
  {
    case ValueType::Null: return Value();
    case ValueType::SDObject: return Error("Cannot convert to SDObject");
    case ValueType::String: return ToString();
    case ValueType::Bool: return ToBool();
    case ValueType::Int: return ToInt();
    case ValueType::UInt: return ToUInt();
    case ValueType::Float: return ToFloat();
    case ValueType::Char: return ToChar();
    case ValueType::Dict: return ToDict();
    case ValueType::Array: return ToArray();
    case ValueType::Error: return Error("Connot convert to Error");
  }
  return Error("Unknown type %s", ToStr(type));
}
inline Value::Overlay::OverlayTree::~OverlayTree()
{
  for(auto it = children.begin(); it != children.end(); ++it)
    it->second->RemoveRef();
  children.clear();
}
}    // namespace templates
}    // namespace cpp_codec

template <>
rdcstr DoStringise(const cpp_codec::templates::ValueType &el)
{
  BEGIN_ENUM_STRINGISE(cpp_codec::templates::ValueType);
  {
    STRINGISE_ENUM_CLASS(Null);
    STRINGISE_ENUM_CLASS(SDObject);
    STRINGISE_ENUM_CLASS(String);
    STRINGISE_ENUM_CLASS(Bool);
    STRINGISE_ENUM_CLASS(Int);
    STRINGISE_ENUM_CLASS(UInt);
    STRINGISE_ENUM_CLASS(Float);
    STRINGISE_ENUM_CLASS(Char);
    STRINGISE_ENUM_CLASS(Dict);
    STRINGISE_ENUM_CLASS(Array);
    STRINGISE_ENUM_CLASS(Overlay);
    STRINGISE_ENUM_CLASS(Error);
  }
  END_ENUM_STRINGISE();
}

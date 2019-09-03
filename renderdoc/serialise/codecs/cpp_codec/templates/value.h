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

namespace cpp_codec
{
namespace templates
{
enum class Ownership : uint32_t
{
  Owned,
  Borrowed,
};
enum class ValueType : uint32_t
{
  Null,
  SDObject,
  String,
  Bool,
  Int,
  UInt,
  Float,
  Char,
  Dict,
  Array,
  Overlay,
  Error,
};

// enum class ValueFlags : uint32_t
//{
//  IsOwned = 0x20,
//};

struct Frame;
struct ExecError;

class Value
{
  template <typename T>
  class Shared;

public:
  using Dict = std::map<rdcstr, Value>;
  using Array = rdcarray<Value>;
  class Overlay;
  class WrappedSDObject;

protected:
  ValueType mType;
  // uint32_t mFlags;

  union
  {
    Shared<WrappedSDObject> *obj;    // `SDObject`s are NOT owned by `Value`s
    Shared<Dict> *dict;              // `Dict`s are shared among `Value`s using reference counting
    Shared<Array> *arr;              // `Array`s are shared among `Value`s using reference counting
    Shared<Overlay> *overlay;
    Shared<rdcstr> *str;
    Shared<ExecError> *err;
    bool b;
    uint64_t u;
    int64_t i;
    double d;
    char c;
  } data;

  // inline bool IsOwned() const { return (mFlags & (uint32_t)ValueFlags::IsOwned) != 0; }
  // inline void SetOwned(bool owned = true)
  //{
  //  if(owned)
  //    mFlags |= (uint32_t)ValueFlags::IsOwned;
  //  else
  //    mFlags &= ~((uint32_t)ValueFlags::IsOwned);
  //}
  // inline Ownership GetOwnership() const
  //{
  //  if(IsOwned())
  //    return Ownership::Owned;
  //  else
  //    return Ownership::Borrowed;
  //}
  // inline void SetOwnership(Ownership o) { SetOwned(o == Ownership::Owned); }
  inline void SetType(ValueType ty) { mType = ty; }
  void AddRef();
  // void DuplicateOwned();

public:
  inline Value() { memset(this, 0, sizeof(Value)); }
  inline Value(const Value &other)
      : mType(other.mType) /* , mFlags(other.mFlags)*/, data(other.data)
  {
    // DuplicateOwned();
    AddRef();
  }
  inline Value(Value &&other) : mType(other.mType) /*, mFlags(other.mFlags)*/, data(other.data)
  {
    other.mType = ValueType::Null;
    // other.mFlags = 0u;
    memset(&other.data, 0, sizeof(other.data));
  }
  inline Value &operator=(const Value &other)
  {
    Clear();
    mType = other.mType;
    // mFlags = other.mFlags;
    data = other.data;
    // DuplicateOwned();
    AddRef();
    return *this;
  }
  inline Value &operator=(Value &&other)
  {
    Clear();
    mType = other.mType;
    // mFlags = other.mFlags;
    data = other.data;

    other.mType = ValueType::Null;
    // other.mFlags = 0u;
    memset(&other.data, 0, sizeof(other.data));
    return *this;
  }
  Value(const SDObject *obj);
  Value(const WrappedSDObject &obj);
  Value(WrappedSDObject &&obj);
  Value(const Dict &dict);
  Value(Dict &&dict);
  Value(const Array &arr);
  Value(Array &&arr);
  Value(const Overlay &overlay);
  Value(const ExecError &err);
  Value(ExecError &&err);
  Value(const char *str);
  Value(const rdcstr &str);
  Value(rdcstr &&str);
  inline explicit Value(bool b)
  {
    SetType(ValueType::Bool);
    data.b = b;
  }
  inline Value(int64_t i)
  {
    SetType(ValueType::Int);
    data.i = i;
  }
  inline Value(uint64_t u)
  {
    SetType(ValueType::UInt);
    data.u = u;
  }
  inline Value(double d)
  {
    SetType(ValueType::Float);
    data.d = d;
  }
  inline Value(char c)
  {
    SetType(ValueType::Char);
    data.c = c;
  }
  template <typename... Args>
  inline static Value Error(const rdcarray<Frame> &stack, const char *fmt, Args... args);
  template <typename... Args>
  inline static Value Error(const char *fmt, Args... args);
  // template <typename... Args>
  // inline static Value Error(const rdcarray<Frame> stack, const char *fmt, Args... args)
  //{
  //  rdcstr msg;
  //  msg.sprintf(fmt, args...);
  //  Value v(std::move(msg));
  //  v.SetType(ValueType::Error);
  //  return v;
  //}
  inline void Clear();
  inline ~Value() { Clear(); }
  inline ValueType Type() const { return mType; }
  Value UnpackSDObject() const;
  bool operator==(const Value &other) const
  {
    bool lt, eq, gt;
    compare(other, lt, eq, gt);
    return eq;
  }
  bool operator!=(const Value &other) const { return !(*this == other); }
  bool operator<(const Value &other) const
  {
    bool lt, eq, gt;
    compare(other, lt, eq, gt);
    return lt;
  }
  bool operator<=(const Value &other) const
  {
    bool lt, eq, gt;
    compare(other, lt, eq, gt);
    return lt || eq;
  }
  bool operator>(const Value &other) const
  {
    bool lt, eq, gt;
    compare(other, lt, eq, gt);
    return gt;
  }
  bool operator>=(const Value &other) const
  {
    bool lt, eq, gt;
    compare(other, lt, eq, gt);
    return gt || eq;
  }
  void compare(const Value &other, bool &lt, bool &eq, bool &gt) const;
  inline bool IsSDObject() const { return Type() == ValueType::SDObject; }
  const WrappedSDObject &GetSDObject() const;
  // inline Value ToSDObject() const { return ToSDObject("", false); }
  // Value ToSDObject(const rdcstr &name, bool rename = true) const;
  inline bool IsDict() const { return Type() == ValueType::Dict; }
  const Dict &GetDict() const;
  Value ToDict() const;
  inline bool IsArray() const { return Type() == ValueType::Array; }
  const Array &GetArray() const;
  Value ToArray() const;
  inline bool IsOverlay() const { return Type() == ValueType::Array; }
  const Overlay &GetOverlay() const;
  inline bool IsString() const { return Type() == ValueType::String; }
  const rdcstr &GetString() const;
  Value ToString() const;
  inline bool IsBool() const { return Type() == ValueType::Bool; }
  inline bool GetBool() const { return data.b; }
  Value ToBool() const;
  inline bool IsInt() const { return Type() == ValueType::Int; }
  inline int64_t GetInt() const { return data.i; }
  Value ToInt() const;
  inline bool IsUInt() const { return Type() == ValueType::UInt; }
  inline uint64_t GetUInt() const { return data.u; }
  Value ToUInt() const;
  inline bool IsFloat() const { return Type() == ValueType::Float; }
  inline double GetFloat() const { return data.d; }
  Value ToFloat() const;
  inline bool IsChar() const { return Type() == ValueType::Char; }
  inline char GetChar() const { return data.c; }
  Value ToChar() const;
  inline bool IsNull() const { return Type() == ValueType::Null; }
  inline bool IsError() const { return Type() == ValueType::Error; }
  const ExecError &GetError() const;
  ExecError &GetError();

  Value To(ValueType type) const;
  Value Field(const rdcstr &name) const;
  Value ReplaceField(const rdcstr &name, const Value &val) const;
  Value Index(size_t index) const;
  Value Visit(std::function<Value(Value, Value, Value)> callback) const;

  struct Iter
  {
    ValueType t;
    class Data
    {
    public:
      virtual bool Eq(const Data *other) const = 0;
      virtual void Incr() = 0;
      virtual Value Deref() const = 0;
      virtual Data *Duplicate() const = 0;
    };
    template <typename Inner>
    class DataT : public Data
    {
      Inner inner;

    public:
      DataT(Inner inner) : inner(inner) {}
      virtual bool Eq(const Data *other) const
      {
        return inner == ((const DataT<Inner> *)other)->inner;
      }
      virtual void Incr() { ++inner; }
      virtual Value Deref() const { return *inner; }
      virtual Data *Duplicate() const { return new DataT<Inner>(inner); }
    };
    Data *data = NULL;
    Iter() : t(ValueType::Error) {}
    Iter(const Iter &other) : t(other.t) { data = other.data->Duplicate(); }
    Iter(Iter &&other) : t(other.t), data(other.data) { other.data = NULL; }
    Iter(SDObject *const *sdobj) : t(ValueType::SDObject)
    {
      data = new DataT<SDObject *const *>(sdobj);
    }
    Iter(const Value *arr) : t(ValueType::Array) { data = new DataT<const Value *>(arr); }
    // Iter(Dict::const_iterator dict) : t(ValueType::Dict)
    //{
    //  data = new DataT<Dict::const_iterator>(dict);
    //}
    ~Iter()
    {
      if(data)
      {
        delete data;
        data = NULL;
      }
    }
    bool operator==(const Iter &other) const
    {
      if(t != other.t)
        return false;
      if(data == NULL || other.data == NULL)
        return (data == other.data);
      return data->Eq(other.data);
    }
    bool operator!=(const Iter &other) const { return !(*this == other); }
    Iter &operator++()
    {
      data->Incr();
      return *this;
    }
    Value operator*() const { return data->Deref(); }
  };
  size_t size() const;
};

class Value::WrappedSDObject
{
  friend class Value;
  SDObject *sdobj = NULL;
  rdcarray<rdcpair<rdcstr, Value>> children;
  inline WrappedSDObject() {}

public:
  using iterator = rdcpair<rdcstr, Value> *;
  using const_iterator = const rdcpair<rdcstr, Value> *;
  inline WrappedSDObject(const SDObject *raw)
  {
    sdobj = new SDObject(raw->name, raw->type);
    sdobj->data.basic = raw->data.basic;
    sdobj->data.str = raw->data.str;

    children.resize(raw->data.children.size());
    for(size_t i = 0; i < raw->data.children.size(); i++)
    {
      const SDObject *child = raw->data.children[i];
      Value childVal = Value(child);
      children[i] = make_rdcpair(child->name, childVal);
    }
  }
  inline WrappedSDObject(const WrappedSDObject &other)
      : sdobj(other.sdobj->Duplicate()), children(other.children)
  {
  }
  inline WrappedSDObject(WrappedSDObject &&other)
      : sdobj(other.sdobj), children(std::move(other.children))
  {
    other.sdobj = NULL;
  }
  inline ~WrappedSDObject()
  {
    delete sdobj;
    sdobj = NULL;
  }
  inline const rdcstr &name() const { return sdobj->name; }
  inline SDType &type() { return sdobj->type; }
  inline const SDType &type() const { return sdobj->type; }
  inline SDObjectData &data() { return sdobj->data; }
  inline const SDObjectData &data() const { return sdobj->data; }
  inline iterator begin() { return children.begin(); }
  inline const_iterator begin() const { return children.begin(); }
  inline iterator end() { return children.end(); }
  inline const_iterator end() const { return children.end(); }
  inline const_iterator find(const rdcstr &name) const
  {
    for(auto it = begin(); it != end(); ++it)
    {
      if(it->first == name)
        return it;
    }
    return end();
  }
  inline iterator find(const rdcstr &name)
  {
    for(auto it = begin(); it != end(); ++it)
    {
      if(it->first == name)
        return it;
    }
    return end();
  }
  inline Value FindChild(const rdcstr &name) const
  {
    for(auto it = children.begin(); it != children.end(); ++it)
    {
      if(it->first == name)
        return it->second;
    }
    return Value();
  }
  inline Value GetChild(size_t index) const
  {
    if(index < children.size())
      return children[index].second;
    return Value();
  }
  inline size_t NumChildren() const { return children.size(); }
  // inline const SDObject &raw() const { return *sdobj; }
  inline bool IsUnion() const
  {
    return type().basetype == SDBasic::Struct && (type().flags & SDTypeFlags::Union);
  }
  inline bool IsStruct() const { return type().basetype == SDBasic::Struct; }
  inline bool IsArray() const { return type().basetype == SDBasic::Array; }
  inline bool IsPointer() const { return (bool)(type().flags & SDTypeFlags::Nullable); }
  inline bool IsResource() const { return type().basetype == SDBasic::Resource; }
  inline bool IsBuffer() const { return type().basetype == SDBasic::Buffer; }
  inline bool IsEnum() const { return type().basetype == SDBasic::Enum; }
  inline bool IsString() const { return type().basetype == SDBasic::String; }
  inline bool IsInt() const { return type().basetype == SDBasic::SignedInteger; }
  inline bool IsUInt() const { return type().basetype == SDBasic::UnsignedInteger; }
  inline bool IsFloat() const { return type().basetype == SDBasic::Float; }
  inline bool IsInt64() const { return IsInt() && type().byteSize == 8; }
  inline bool IsUInt64() const { return IsUInt() && type().byteSize == 8; }
  inline bool IsFloat32() const { return IsFloat() && type().byteSize == 4; }
  inline bool IsSimpleType() const
  {
    return IsNULL() || (!IsStruct() && !IsArray() && !IsPointer() && !IsUnion());
  }
  inline bool IsInlineable() const
  {
    if(IsSimpleType() || IsFixedArray() || HasCustomString())
      return true;
    // if it has elements that are not inlineable, return false.
    for(size_t i = 0; i < NumChildren(); i++)
    {
      Value child = GetChild(i);
      if(child.IsSDObject() && !child.GetSDObject().IsInlineable())
        return false;
    }
    if((IsPointer() || IsVariableArray()) && !IsNULL())
      return false;
    if(IsUnion())
      return false;
    return true;
  }
  inline bool IsHidden() const { return (bool)(type().flags & SDTypeFlags::Hidden); }
  inline bool IsNULL() const
  {
    return type().basetype == SDBasic::Null || (IsArray() && NumChildren() == 0) ||
           (IsString() && (type().flags & SDTypeFlags::NullString));
  }
  inline bool IsVariableArray() const
  {
    return IsArray() && !(type().flags & SDTypeFlags::FixedArray);
  }
  inline bool IsFixedArray() const { return IsArray() && (type().flags & SDTypeFlags::FixedArray); }
  inline bool HasCustomString() const
  {
    return (bool)(type().flags & SDTypeFlags::HasCustomString);
  }
  inline void ClearChildren() { children.clear(); }
  inline void AddChild(const rdcstr &name, const Value &v)
  {
    children.push_back(make_rdcpair(name, v));
  }
  inline static WrappedSDObject NewSDArray(const rdcstr &name, const rdcstr &type)
  {
    SDObject *sdobj = makeSDArray(name.c_str());
    sdobj->type.name = type;
    WrappedSDObject w;
    w.sdobj = sdobj;
    return w;
  }
  inline static WrappedSDObject NewSDStruct(const rdcstr &name, const rdcstr &type,
                                            const std::initializer_list<rdcpair<rdcstr, Value>> &fields)
  {
    SDObject *sdobj = makeSDStruct(name.c_str(), type.c_str());
    WrappedSDObject w;
    w.sdobj = sdobj;
    for(auto it = fields.begin(); it != fields.end(); ++it)
      w.children.push_back(*it);
    return std::move(w);
  }
};

class Value::Overlay
{
  struct OverlayTree
  {
    std::map<rdcstr, Shared<OverlayTree> *> children;
    ~OverlayTree();
  };
  Shared<OverlayTree> *tree;
  Value inner;
};

template <typename T>
class Value::Shared
{
  T mValue;
  int32_t mRefCount = 1;

public:
  inline Shared(const T &value) : mValue(value) {}
  inline Shared(T &&value) : mValue(value) {}
  inline const T &operator*() const { return mValue; }
  inline T &operator*() { return mValue; }
  inline const T *operator->() const { return &mValue; }
  inline T *operator->() { return &mValue; }
  inline void AddRef() { ++mRefCount; }
  inline void RemoveRef()
  {
    --mRefCount;
    if(mRefCount <= 0)
      delete this;
  }
};

static_assert(sizeof(Value) == 16, "Value must have size 16 for unboxed");
}    // namespace templates
}    // namespace cpp_codec

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

#include "func.h"
#include "parse.h"
#include "value.h"

namespace cpp_codec
{
namespace templates
{
class Writer;

struct TemplateSource
{
  rdcstr name;
  rdcstr source;
};

struct Frame
{
  const rdcstr name;
  const parse::Node *node = NULL;
  Frame() = default;
  Frame(const rdcstr &name, const parse::Node *node) : name(name), node(node) {}
};

struct ExecError
{
  rdcstr msg;
  bool needsStack = true;
  rdcarray<Frame> stack;
  inline bool IsOk() const { return msg.empty(); }
};

template <typename... Args>
inline static Value Value::Error(const rdcarray<Frame> &stack, const char *fmt, Args... args)
{
  ExecError *err = new ExecError();
  err->msg.sprintf(fmt, args...);
  err->stack = stack;
  return Value(err, Ownership::Owned);
}

template <typename... Args>
inline static Value Value::Error(const char *fmt, Args... args)
{
  ExecError err;
  err.msg.sprintf(fmt, args...);
  return Value(std::move(err));
}

class Template
{
  friend class State;

protected:
  rdcarray<TemplateSource> sources;
  std::map<rdcstr, parse::ListNode *> roots;
  std::map<rdcstr, DynFunc *> funcs;
  std::map<rdcstr, Value> globals;
  std::map<rdcstr, ReflowWriter *> files;
  parse::NodeAllocator alloc;
  Value execError;

  void PrintFrame(Writer &w, const Frame &frame)
  {
    if(frame.node == NULL)
    {
      w.Printf("  UNKNOWN_SOURCE: %s\n", frame.name);
      return;
    }
    parse::SourceRange range = frame.node->source();
    if(range.sourceId >= sources.size())
    {
      w.Printf("  UNKNOWN_SOURCE(offset %z-%z): %s\n", range.offset, range.offset + range.size,
               frame.name);
      return;
    }
    TemplateSource &src = sources[range.sourceId];
    int lineNo = parse::LineNumber(src.source, (int)range.offset);
    ReflowWriter r(&w);
    r.Indent();
    r.Printf("%s(%d): %s\n", src.name, lineNo, frame.name);
    r.Indent();
    parse::PrintSourceRange(r, src.source, range.offset, 0, 0, false);
    r.NewLine();
    r.Close();
  }

public:
  void AddFunc(DynFunc *func);
  parse::ParseError Parse(const rdcstr &name, const rdcstr &src);
  void Load(const rdcstr &resourceName);
  ExecError Exec(Writer &out, const rdcstr &name, Value v);
  size_t NumTemplates() const { return roots.size(); }
  size_t NumFuncs() const { return funcs.size(); }
  void PrintParseError(Writer &w, parse::ParseError err)
  {
    if(err.sourceId < sources.size())
    {
      TemplateSource &src = sources[err.sourceId];
      err.Print(w, src.name, src.source);
    }
    else
    {
      err.Print(w, "UNKNOWN", "");
    }
  }
  void PrintExecError(Writer &w, ExecError err)
  {
    w.Printf("Error executing template: %s\n", err.msg);
    w.Printf("Template Stack:\n");
    for(auto it = err.stack.begin(); it != err.stack.end(); ++it)
    {
      PrintFrame(w, *it);
    }
  }
  void SetGlobal(const rdcstr &name, Value val) { globals[name] = val; }
  Value GetGlobal(const rdcstr &name)
  {
    auto it = globals.find(name);
    if(it != globals.end())
      return it->second;
    return Value();
  }
  void SetFile(const rdcstr &name, ReflowWriter *w) { files[name] = w; }
  ReflowWriter *GetFile(const rdcstr &name) { return files[name]; }
};

class SDAllocator
{
  rdcarray<SDObject *> mObjs;
  rdcarray<SDChunk *> mChunks;
  SDObject *nullObj = NULL;
  SDObject *trueObj = NULL;
  SDObject *falseObj = NULL;

public:
  ~SDAllocator() { Clear(); }
  void Clear()
  {
    for(auto it = mObjs.begin(); it != mObjs.end(); ++it)
    {
      SDObject *obj = *it;

      // We must clear the children so that they are not also deleted.
      // If the children are owned, they will be deleted separately.
      obj->data.children.clear();

      delete obj;
    }
    for(auto it = mChunks.begin(); it != mChunks.end(); ++it)
    {
      SDChunk *obj = *it;

      // We must clear the children so that they are not also deleted.
      // If the children are owned, they will be deleted separately.
      obj->data.children.clear();

      delete obj;
    }
  }
  SDObject *Register(SDObject *obj)
  {
    mObjs.push_back(obj);
    return obj;
  }
  SDObject *RegisterRecursive(SDObject *obj)
  {
    Register(obj);
    for(auto it = obj->begin(); it != obj->end(); ++it)
      RegisterRecursive(*it);
    return obj;
  }
  SDChunk *Register(SDChunk *chunk)
  {
    mChunks.push_back(chunk);
    return chunk;
  }
  SDChunk *RegisterRecursive(SDChunk *chunk)
  {
    Register(chunk);
    for(auto it = chunk->begin(); it != chunk->end(); ++it)
      RegisterRecursive(*it);
    return chunk;
  }
  SDObject *DeepClone(const SDObject *obj) { return RegisterRecursive(obj->Duplicate()); }
  SDObject *EmptyClone(const SDObject *obj)
  {
    SDObject *ret = Register(new SDObject(obj->name, obj->type));
    ret->data.basic = obj->data.basic;
    ret->data.str = obj->data.str;
    return ret;
  }
  SDObject *ShallowClone(const SDObject *obj)
  {
    SDObject *ret = EmptyClone(obj);
    for(auto it = obj->begin(); it != obj->end(); ++it)
      ret->data.children.push_back(*it);
    return ret;
  }
  SDChunk *DeepClone(const SDChunk *chunk) { return RegisterRecursive(chunk->Duplicate()); }
  SDChunk *EmptyClone(const SDChunk *chunk)
  {
    SDChunk *ret = Register(new SDChunk(chunk->name.c_str()));
    ret->metadata = chunk->metadata;
    ret->type = chunk->type;
    ret->data.basic = chunk->data.basic;
    ret->data.str = chunk->data.str;
    return ret;
  }
  SDChunk *ShallowClone(const SDChunk *chunk)
  {
    SDChunk *ret = EmptyClone(chunk);
    for(auto it = chunk->begin(); it != chunk->end(); ++it)
      ret->data.children.push_back(*it);
    return ret;
  }
  SDObject *GetNull()
  {
    if(nullObj == NULL)
      nullObj = Register(new SDObject("value", SDType::NullPointer("void")));
    return nullObj;
  }
  SDObject *NewNull(const rdcstr &name)
  {
    SDObject *obj = ShallowClone(GetNull());
    obj->name = name;
    return obj;
  }
  SDObject *GetBool(bool value)
  {
    if(value)
    {
      if(trueObj == NULL)
        trueObj = Register(makeSDBool("value", true));
      return trueObj;
    }
    else
    {
      if(falseObj == NULL)
        falseObj = Register(makeSDBool("value", false));
      return falseObj;
    }
  }
  SDObject *NewBool(const rdcstr &name, bool value)
  {
    return Register(makeSDBool(name.c_str(), value));
  }

  SDObject *NewString(const rdcstr &value) { return NewString("value", value); }
  SDObject *NewString(const rdcstr &name, const rdcstr &value)
  {
    return Register(makeSDString(name.c_str(), value.c_str()));
  }

  SDObject *NewChar(char value) { return NewChar("value", value); }
  SDObject *NewChar(const rdcstr &name, char value)
  {
    SDObject *obj = Register(new SDObject("value", "char"));
    obj->type.basetype = SDBasic::Character;
    obj->type.byteSize = sizeof(char);
    obj->data.basic.c = value;
    return obj;
  }

  SDObject *NewUInt32(uint32_t value) { return NewUInt32("value", value); }
  SDObject *NewUInt32(const rdcstr &name, uint32_t value)
  {
    return Register(makeSDUInt32(name.c_str(), value));
  }
  SDObject *NewInt32(int32_t value) { return NewInt32("value", value); }
  SDObject *NewInt32(const rdcstr &name, int32_t value)
  {
    return Register(makeSDInt32(name.c_str(), value));
  }

  SDObject *NewUInt64(uint64_t value) { return NewUInt64("value", value); }
  SDObject *NewUInt64(const rdcstr &name, uint64_t value)
  {
    return Register(makeSDUInt64(name.c_str(), value));
  }
  SDObject *NewInt64(int64_t value) { return NewInt64("value", value); }
  SDObject *NewInt64(const rdcstr &name, int64_t value)
  {
    return Register(makeSDInt64(name.c_str(), value));
  }
  SDObject *NewFloat(double value) { return NewFloat("value", value); }
  SDObject *NewFloat(const rdcstr &name, double value)
  {
    return Register(makeSDFloat(name.c_str(), value));
  }
  SDObject *NewStruct(const rdcstr &type, const std::initializer_list<SDObject *> &children)
  {
    return NewStruct("value", type, children);
  }
  SDObject *NewStruct(const rdcstr &name, const rdcstr &type,
                      const std::initializer_list<SDObject *> &children)
  {
    SDObject *obj = Register(new SDObject(name, type));
    for(auto it = children.begin(); it != children.end(); ++it)
      obj->data.children.push_back(*it);
    return obj;
  }
  SDObject *NewStructPtr(const rdcstr &type, const std::initializer_list<SDObject *> &children)
  {
    return NewStructPtr("value", type, children);
  }
  SDObject *NewStructPtr(const rdcstr &name, const rdcstr &type,
                         const std::initializer_list<SDObject *> &children)
  {
    SDObject *obj = Register(new SDObject(name, type));
    obj->type.flags |= SDTypeFlags::Nullable;
    for(auto it = children.begin(); it != children.end(); ++it)
      obj->data.children.push_back(*it);
    return obj;
  }
  SDObject *NewUnion(const rdcstr &name, const rdcstr &type,
                     const std::initializer_list<SDObject *> &children)
  {
    SDObject *obj = NewStruct(name, type, children);
    obj->type.flags |= SDTypeFlags::Union;
    return obj;
  }
  SDObject *NewEnum(const rdcstr &name, const rdcstr &type, uint32_t value, const rdcstr &valueStr)
  {
    SDObject *ret = NewEnum(name, type, value);
    ret->SetCustomString(valueStr);
    return ret;
  }
  SDObject *NewEnum(const rdcstr &name, const rdcstr &type, uint32_t value)
  {
    SDObject *ret = Register(new SDObject(name, type));
    ret->type.basetype = SDBasic::Enum;
    ret->type.byteSize = sizeof(uint32_t);
    ret->data.basic.u = value;
    return ret;
  }
  SDObject *NewResourcePtr(const rdcstr &name, const rdcstr &type, uint64_t id)
  {
    SDObject *ret = Register(new SDObject(name, type));
    ret->type.basetype = SDBasic::Resource;
    ret->type.byteSize = sizeof(uint64_t);
    ret->type.flags |= SDTypeFlags::Nullable;
    ret->data.basic.u = id;
    return ret;
  }
  SDObject *NewArray(const rdcstr &name, SDType type,
                     const std::initializer_list<SDObject *> &children)
  {
    SDObject *ret = Register(new SDObject(name, type));
    ret->type.basetype = SDBasic::Array;
    for(auto it = children.begin(); it != children.end(); ++it)
      ret->data.children.push_back(*it);
    return ret;
  }
  SDObject *NewArray(const rdcstr &name, const rdcstr &type,
                     const std::initializer_list<SDObject *> &children)
  {
    return NewArray(name, SDType(type), children);
  }
  SDChunk *NewChunk(const rdcstr &name, uint32_t chunkID,
                    const std::initializer_list<SDObject *> &children)
  {
    SDChunk *chunk = Register(new SDChunk(name.c_str()));
    chunk->metadata.chunkID = chunkID;
    for(auto it = children.begin(); it != children.end(); ++it)
      chunk->data.children.push_back(*it);
    return chunk;
  }
};

class State
{
protected:
  Template &tmpl;
  ReflowWriter &out;
  rdcarray<rdcpair<rdcstr, Value>> vars;
  rdcarray<Frame> mStack;
  rdcstr templateName;
  // bool mError = false;
  // rdcstr mErrorMsg;

  // template <typename... Args>
  // void errorf(const char *fmt, const Args &... args)
  //{
  //  mError = true;
  //  mErrorMsg.clear();
  //  mErrorMsg.sprintf(fmt, args...);
  //}
  template <typename... Args>
  Value errorf(const parse::Node *node, const char *fmt, const Args &... args) const
  {
    ExecError err;
    err.msg.sprintf(fmt, args...);
    err.stack = mStack;
    if(node)
      err.stack.push_back(Frame(templateName, node));
    err.needsStack = err.stack.empty();
    return Value(std::move(err));
  }

  struct StackMark
  {
    rdcarray<rdcpair<rdcstr, Value>> *stack;
    size_t markedSize;
    StackMark(rdcarray<rdcpair<rdcstr, Value>> &stack) : stack(&stack), markedSize(stack.size()) {}
    StackMark(const StackMark &) = delete;
    StackMark(StackMark &&other)
    {
      stack = other.stack;
      other.stack = NULL;
      markedSize = other.markedSize;
    }
    ~StackMark()
    {
      if(stack != NULL)
        stack->resize(RDCMIN(stack->size(), markedSize));
    }
  };

  inline StackMark MarkVarStack() { return StackMark(vars); }
  void SetTopVar(size_t index, Value value) { vars[vars.size() - index].second = value; }
  bool SetVar(const rdcstr &name, Value value);

  // void WriteLines(Writer &w, const rdcstr &str);
  void Print(Value obj);
  // void Print(Writer &w, Value obj);
  Value EvalPipeline(Value dot, const parse::PipeNode *pipe);
  Value EvalCommand(Value dot, const parse::CommandNode *cmd,
                    const std::initializer_list<Value> &finalArgs);
  Value EvalCall(Value dot, const parse::FuncNode *func, const rdcarray<Value> args);
  Value EvalArg(Value dot, const parse::Node *cmd);

  Value TagError(const parse::Node *n, Value v)
  {
    if(v.IsError() && v.GetError().needsStack)
    {
      for(auto it = mStack.begin(); it != mStack.end(); ++it)
        v.GetError().stack.push_back(*it);
      if(n != NULL)
        v.GetError().stack.push_back(Frame(templateName, n));
      v.GetError().needsStack = false;
    }
    return v;
  }

public:
  State(Template &tmpl, const rdcstr &templateName, ReflowWriter &out)
      : tmpl(tmpl), templateName(templateName), out(out)
  {
  }
  Value Walk(Value dot, const parse::Node *node);
  Value Macro(const rdcstr &name, Value newDot);
  Value ExecTemplate(ReflowWriter &newOut, const rdcstr &name, Value newDot);
  inline ReflowWriter &writer() { return out; }
  Value GetVar(const rdcstr &name) const;
  inline void PushVar(const rdcstr &name, Value value)
  {
    vars.push_back(make_rdcpair(name, value));
  }
  inline const rdcarray<Frame> &Stack() const { return mStack; }
  inline Template &Template() { return tmpl; }
};

// bool IsTrue(Value obj);
}    // namespace templates
}    // namespace cpp_codec

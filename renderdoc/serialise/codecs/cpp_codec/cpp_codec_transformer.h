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

#include <initializer_list>
#include "common/common.h"
#include "core/core.h"
#include "serialise/rdcfile.h"

namespace cpp_codec
{
enum class CodeGenChunk : uint32_t
{
  CodeBlock = 0x10000,
  Statement,
  Custom,
};

class ChunkTransformerGraph;

class ChunkTransformerNode
{
  friend class ChunkTransformerGraph;

protected:
  ChunkTransformerGraph *graph;
  rdcarray<ChunkTransformerNode *> outputs;
  void output(SDChunk *chunk) const;
  virtual void input(SDChunk *chunk) { output(chunk); }
  // helpers for constructing SDObjects
  SDChunk *Chunk(uint32_t chunkID, const char *name, std::initializer_list<SDObject *> args);
  SDObject *StructPtr(const rdcstr &name, const rdcstr &type,
                      std::initializer_list<SDObject *> members);
  SDObject *Enum(const rdcstr &name, const rdcstr &type, const rdcstr &str);
  SDObject *NullPtr(const rdcstr &name, const rdcstr &type);
  SDObject *String(const rdcstr &name, const rdcstr &str);
  SDObject *UInt32(const rdcstr &name, uint32_t val);
  SDObject *UInt32Ptr(const rdcstr &name, uint32_t val);
  // SDObject *Ptr(const SDObject *val)
  //{
  //  SDObject *ptr = graph->Duplicate(val);
  //  ptr->type.flags |= SDTypeFlags::Nullable;
  //  return ptr;
  //}
  template <typename... Args>
  SDObject *Code(const rdcstr &fmt, Args... args)
  {
    SDType t("");
    t.basetype = SDBasic::Code;
    t.flags |= SDTypeFlags::HasCustomString;
    SDObject *obj = graph->AllocObj("CODE", t);
    obj->data.str = fmt;
    return obj;
  }
  template <typename... Args>
  SDChunk *Stmt(const rdcstr &code, Args... args)
  {
    SDChunk *stmt = Chunk((uint32_t)CodeGenChunk::Statement, "Statement", {});
    stmt->data.children.push_back(Code(code, args...));
    return stmt;
  }
  // SDObject *Code(const rdcstr &code);
  // SDChunk *Stmt(const rdcstr &code);
  template <typename Iter>
  SDChunk *CodeBlock(const rdcstr &name, Iter begin, Iter end)
  {
    SDChunk *codeBlock = Chunk((uint32_t)CodeGenChunk::CodeBlock, name.c_str(), {});
    for(auto it = begin; it != end; ++it)
    {
      SDChunk *stmt = *it;
      codeBlock->data.children.push_back(graph->Duplicate(stmt));
    }
    return codeBlock;
  }
  SDChunk *CodeBlock(const rdcstr &name, std::initializer_list<SDChunk *> statements);
};

class ObjTransformerNode : public ChunkTransformerNode
{
protected:
  rdcarray<SDObject *> stack;
  virtual void input(SDChunk *chunk) { inputObj(chunk); }
  void outputObj(SDObject *obj)
  {
    if(stack.empty())
      output((SDChunk *)obj);
    else
      stack.back()->data.children.push_back(obj);
  }
  void recurse(SDObject *obj)
  {
    if(!obj->data.children.empty())
    {
      stack.push_back(obj);
      rdcarray<SDObject *> children;
      obj->data.children.swap(children);
      obj->data.children.reserve(children.size());
      for(auto it = children.begin(); it != children.end(); ++it)
        inputObj(*it);
      stack.pop_back();
    }
    outputObj(obj);
  }
  virtual void inputObj(SDObject *obj) { recurse(obj); }
};

class StripCustomStringsTransformer : public ObjTransformerNode
{
protected:
  virtual void inputObj(SDObject *obj)
  {
    obj->type.flags &= ~SDTypeFlags::HasCustomString;
    recurse(obj);
  }
};

class TypeTransformer : public ObjTransformerNode
{
protected:
  virtual void inputObj(SDObject *obj)
  {
    obj->type.name = typeName(obj->type);
    recurse(obj);
  }
  rdcstr typeName(const SDType &t)
  {
    switch(t.basetype)
    {
      case SDBasic::Boolean: return "bool";
      case SDBasic::Character: return "char";
      case SDBasic::Float:
        switch(t.byteSize)
        {
          case 4: return "float";
          case 8: return "double";
          default: RDCASSERT(false);
        }
        break;
      case SDBasic::SignedInteger:
        switch(t.byteSize)
        {
          case 1: return "int8_t";
          case 2: return "int16_t";
          case 4: return "int32_t";
          case 8: return "int64_t";
          default: RDCASSERT(false);
        }
        break;
      case SDBasic::UnsignedInteger:
        switch(t.byteSize)
        {
          case 1: return "uint8_t";
          case 2: return "uint16_t";
          case 4: return "uint32_t";
          case 8: return "uint64_t";
          default: RDCASSERT(false);
        }
        break;
      case SDBasic::String: return "const char *";
      default: break;
    }
    if(t.name == "string")
      return "const char* ";
    else
      return t.name;
  }
};

class CodeWriter;

class BufferTransformer : public ObjTransformerNode
{
public:
  BufferTransformer(CodeWriter *writer) : writer(writer) {}
protected:
  CodeWriter *writer;
  virtual void inputObj(SDObject *obj);
};

// class BufferTransformer : public ObjTransformerNode
//{
// protected:
//  virtual void inputObj(SDObject *obj)
//  {
//    if (obj->IsBuffer()) {
//      obj->type.flags |= SDTypeFlags::HasCustomString;
//      obj->data.str =
//    obj->type.name = typeName(obj->type);
//    recurse(obj);
//  }
//};

class ChunkTransformerGraph
{
  friend class ChunkTransformerNode;

protected:
  ChunkTransformerNode *source;
  ChunkTransformerNode *sink;
  rdcarray<ChunkTransformerNode *> nodes;

public:
  ChunkTransformerGraph() = default;
  ChunkTransformerGraph(rdcarray<ChunkTransformerNode *> &&n)
  {
    nodes.swap(n);
    if(nodes.empty())
      nodes.push_back(new ChunkTransformerNode());
    for(auto it = nodes.begin(), prev = nodes.end(); it != nodes.end(); prev = it, ++it)
    {
      if(prev != nodes.end())
        (*prev)->outputs.push_back(*it);
      (*it)->graph = this;
    }
    source = nodes.front();
    sink = nodes.back();
  }
  ChunkTransformerGraph(const ChunkTransformerGraph &) = delete;
  ~ChunkTransformerGraph()
  {
    for(auto it = nodes.begin(); it != nodes.end(); ++it)
      delete *it;
  }
  void Run(const rdcarray<SDChunk *> &chunks)
  {
    for(auto it = chunks.begin(); it != chunks.end(); ++it)
      source->input(Duplicate(*it));
  }
  ChunkTransformerGraph &append(ChunkTransformerNode *node)
  {
    node->graph = this;
    nodes.push_back(node);
    sink->outputs.push_back(node);
    sink = node;
    return *this;
  }
  ChunkTransformerGraph &prepend(ChunkTransformerNode *node)
  {
    node->graph = this;
    nodes.push_back(node);
    nodes.back()->outputs.push_back(source);
    source = node;
    return *this;
  }
  // SDChunk and SDObject alloc/free.
  // All ChunkTransformerNode implementations should use these functions to alloc/free SDChunks and
  // SDObjects. These are currently just thin wrappers, but leave open the possibility of using a
  // more efficient allocator.
  inline SDChunk *Duplicate(const SDChunk *chunk) { return chunk->Duplicate(); }
  inline SDObject *Duplicate(const SDObject *obj)
  {
    if(obj->type.basetype == SDBasic::Chunk)
      return Duplicate((SDChunk *)obj);
    else
      return obj->Duplicate();
  }
  inline void FreeChunk(SDChunk *chunk) { delete chunk; }
  inline void FreeObject(SDObject *obj) { delete obj; }
  inline SDChunk *AllocChunk(const char *name) { return new SDChunk(name); }
  inline SDObject *AllocObj(const rdcstr &name, const SDType &type)
  {
    return new SDObject(name, type);
  }
};

// class ChunkTransformer
//{
// protected:
//  // outputs process each output chunk from this transformer
//  rdcarray<ChunkTransformer *> outputs;
//
//  // children are destroyed when this transformer is destroyed.
//  rdcarray<ChunkTransformer *> children;
//
//  void yield(SDChunk *chunk)
//  {
//    if(outputs.empty())
//      FreeChunk(chunk);
//    auto it = outputs.begin();
//    while(it != outputs.end())
//    {
//      ChunkTransformer *output = *it;
//      ++it;
//      SDChunk *dup = chunk;
//      if(it != outputs.end())
//        // we will need another copy of the chunk for at least 1 more output
//        dup = chunk->Duplicate();
//      output->ProcessChunk(chunk);
//      chunk = dup;
//    }
//  }
//  SDChunk *Duplicate(const SDChunk *chunk) { return chunk->Duplicate(); }
//  SDObject *Duplicate(const SDObject *obj) { return obj->Duplicate(); }
//  void FreeChunk(SDChunk *chunk) { delete chunk; }
//  void FreeObject(SDObject *obj) { delete obj; }
//  SDChunk *Chunk(const char *name, std::initializer_list<SDObject *> args)
//  {
//    SDChunk *chunk = new SDChunk(name);
//    for(auto it = args.begin(); it != args.end(); ++it)
//      chunk->AddChild(*it);
//    return chunk;
//  }
//  SDObject *AllocObj(const rdcstr &name, const SDType &type) { return new SDObject(name, type); }
//  inline SDObject *StructPtr(const rdcstr &name, const rdcstr &type,
//                             std::initializer_list<SDObject *> members)
//  {
//    SDType t(type);
//    t.flags |= SDTypeFlags::Nullable;
//    SDObject *obj = AllocObj(name, t);
//    for(auto it = members.begin(); it != members.end(); ++it)
//      obj->AddChild(*it);
//    return obj;
//  }
//
//  inline SDObject *Enum(const rdcstr &name, const rdcstr &type, const rdcstr &str)
//  {
//    SDType t(type);
//    t.basetype = SDBasic::Enum;
//    SDObject *obj = AllocObj(name, t);
//    obj->data.str = str;
//    return obj;
//  }
//
//  inline SDObject *NullPtr(const rdcstr &name, const rdcstr &type)
//  {
//    SDType t(type);
//    t.basetype = SDBasic::Null;
//    t.flags |= SDTypeFlags::Nullable;
//    return AllocObj(name, t);
//  }
//
//  inline SDObject *String(const rdcstr &name, const rdcstr &str)
//  {
//    SDType t("");
//    t.basetype = SDBasic::String;
//    SDObject *obj = AllocObj(name, t);
//    obj->data.str = str;
//    return obj;
//  }
//
//  inline SDObject *UInt32(const rdcstr &name, uint32_t val)
//  {
//    SDType t("");
//    t.basetype = SDBasic::UnsignedInteger;
//    t.byteSize = 4;
//    SDObject *obj = AllocObj(name, t);
//    obj->data.basic.u = val;
//    return obj;
//  }
//
// public:
//  virtual void ProcessChunk(SDChunk *chunk) { yield(chunk); }
//  virtual ~ChunkTransformer()
//  {
//    for(auto it = children.begin(); it != children.end(); ++it)
//    {
//      delete *it;
//    }
//  }
//  ChunkTransformer() {}
//  // ChunkTransformer(ChunkTransformer *input)
//  //{
//  //  if(input != NULL)
//  //  {
//  //    input->outputs.push_back(this);
//  //    input->children.push_back(this);
//  //  }
//  //}
//  void AddOutput(ChunkTransformer *output) { outputs.push_back(output); }
//  void AddChild(ChunkTransformer *child) { children.push_back(child); }
//};
//
// class ChunkSource : public ChunkTransformer
//{
// public:
//  virtual void ProcessChunk(SDChunk *) {}
//  void Run(const rdcarray<SDChunk *> &chunks)
//  {
//    for(auto it = chunks.begin(); it != chunks.end(); ++it)
//      yield(Duplicate(*it));
//  }
//};
//
// template <typename OutputIterator>
// class ChunkSink : public ChunkTransformer
//{
// protected:
//  OutputIterator outputIter;
//
// public:
//  virtual void ProcessChunk(SDChunk *chunk)
//  {
//    *outputIter = chunk;
//    ++outputIter;
//  }
//  ChunkSink(OutputIterator outputIter) : ChunkTransformer(), outputIter(outputIter) {}
//};
//
// ChunkTransformer *Compose(std::initializer_list<ChunkTransformer *> transformers);
}

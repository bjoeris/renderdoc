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

#include "cpp_codec_transformer.h"
#include "cpp_codec_writer.h"

namespace cpp_codec
{
void ChunkTransformerNode::output(SDChunk *chunk) const
{
  for(auto it = outputs.begin(); it != outputs.end();)
  {
    ChunkTransformerNode &output = **it;
    ++it;
    SDChunk *dup = NULL;
    if(it != outputs.end())
      // we will need another copy of the chunk for at least 1 more output
      dup = graph->Duplicate(chunk);
    output.input(chunk);
    chunk = dup;
  }
  if(chunk != NULL)
    graph->FreeChunk(chunk);
}

// helpers for constructing SDObjects

SDChunk *ChunkTransformerNode::Chunk(uint32_t chunkID, const char *name,
                                     std::initializer_list<SDObject *> args)
{
  SDChunk *chunk = graph->AllocChunk(name);
  chunk->metadata.chunkID = chunkID;
  for(auto it = args.begin(); it != args.end(); ++it)
    chunk->data.children.push_back(graph->Duplicate(*it));
  return chunk;
}
SDObject *ChunkTransformerNode::StructPtr(const rdcstr &name, const rdcstr &type,
                                          std::initializer_list<SDObject *> members)
{
  SDType t(type);
  t.flags |= SDTypeFlags::Nullable;
  SDObject *obj = graph->AllocObj(name, t);
  for(auto it = members.begin(); it != members.end(); ++it)
    obj->data.children.push_back(graph->Duplicate(*it));
  return obj;
}
SDObject *ChunkTransformerNode::Enum(const rdcstr &name, const rdcstr &type, const rdcstr &str)
{
  SDType t(type);
  t.basetype = SDBasic::Enum;
  SDObject *obj = graph->AllocObj(name, t);
  obj->data.str = str;
  return obj;
}
SDObject *ChunkTransformerNode::NullPtr(const rdcstr &name, const rdcstr &type)
{
  SDType t(type);
  t.basetype = SDBasic::Null;
  t.flags |= SDTypeFlags::Nullable;
  return graph->AllocObj(name, t);
}
SDObject *ChunkTransformerNode::String(const rdcstr &name, const rdcstr &str)
{
  SDType t("");
  t.basetype = SDBasic::String;
  SDObject *obj = graph->AllocObj(name, t);
  obj->data.str = str;
  return obj;
}
SDObject *ChunkTransformerNode::UInt32(const rdcstr &name, uint32_t val)
{
  SDType t("uint32_t");
  t.basetype = SDBasic::UnsignedInteger;
  t.byteSize = 4;
  SDObject *obj = graph->AllocObj(name, t);
  obj->data.basic.u = val;
  return obj;
}
SDObject *ChunkTransformerNode::UInt32Ptr(const rdcstr &name, uint32_t val)
{
  SDType t("uint32_t");
  t.basetype = SDBasic::UnsignedInteger;
  t.byteSize = 4;
  t.flags |= SDTypeFlags::Nullable;
  SDObject *obj = graph->AllocObj(name, t);
  obj->data.basic.u = val;
  return obj;
}
// SDObject *ChunkTransformerNode::Code(const rdcstr &code)
//{
//  SDType t("");
//  t.basetype = SDBasic::Code;
//  t.flags |= SDTypeFlags::HasCustomString;
//  SDObject *obj = graph->AllocObj("CODE", t);
//  obj->data.str = code;
//  return obj;
//}
// SDChunk *ChunkTransformerNode::Stmt(const rdcstr &code)
//{
//  SDChunk *stmt = Chunk((uint32_t)CodeGenChunk::Statement, "Statement", {});
//  stmt->data.children.push_back(Code(code));
//  return stmt;
//}
SDChunk *ChunkTransformerNode::CodeBlock(const rdcstr &name,
                                         std::initializer_list<SDChunk *> statements)
{
  SDChunk *codeBlock = Chunk((uint32_t)CodeGenChunk::CodeBlock, name.c_str(), {});
  for(auto it = statements.begin(); it != statements.end(); ++it)
    codeBlock->data.children.push_back(graph->Duplicate(*it));
  return codeBlock;
}
void BufferTransformer::inputObj(SDObject *obj)
{
  if(obj->type.basetype == SDBasic::Buffer && !obj->HasCustomString())
  {
    rdcstr bufName = writer->GetDataBlobVar(obj->AsUInt64());
    obj->SetCustomString(bufName + ".data()");
  }
  recurse(obj);
}
// ChunkTransformer *Compose(std::initializer_list<ChunkTransformer *> transformers)
//{
//  if(transformers.size() == 0)
//    return new ChunkTransformer();
//  auto it = transformers.begin();
//  auto prev = it;
//  ++it;
//  for(; it != transformers.end(); ++it)
//  {
//    (*prev)->AddOutput(*it);
//    (*prev)->AddChild(*it);
//    prev = it;
//  }
//  return *prev;
//}
}

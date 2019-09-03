/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2019 Google LLC
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

#include "templates/cpp_templates.h"
#include "cpp_codec_file.h"
#include "cpp_codec_transformer.h"

#include "serialise/rdcfile.h"
#include "cpp_codec_resource.h"

#include <set>
#include <unordered_map>

namespace cpp_codec
{
enum FileID
{
  FILE_MAIN,
  FILE_VAR,
  FILE_RENDER,
  FILE_EARLY_CREATE,
  FILE_CREATE,
  FILE_RELEASE,
  FILE_INIT,
  FILE_RESET,

  FILE_COUNT,
};

extern const char *FILE_NAMES[FILE_COUNT];

struct TemplateFileDesc
{
  const char *filename;
  size_t size;
  const char *contents;
};

class CodeWriter
{
protected:
  templates::Template mTemplates;
  rdcarray<CodeFile *> files;
  rdcstr rootDirectory;
  FileID currentFile = FILE_CREATE;

  typedef std::unordered_map<uint64_t, rdcstr> VarMap;
  VarMap resourceVars;
  VarMap dataBlobVars;
  int blockDepth = 0;
  typedef std::set<rdcstr> VarSet;
  VarSet localVars;

  void Open();
  void OpenFile(const rdcstr &name, uint32_t pass, bool isMultiPart);
  void CloseFiles();
  void ReadBuffers(const StructuredBufferList &buffers);
  void WriteBufferData(const StructuredBufferList &buffers);
  void MultiPartSplit();
  void WriteTemplateFile(const rdcstr &file, const char *contents, size_t size);
  void WriteGenericChunk(const SDChunk *chunk);
  rdcarray<rdcstr> ChunkArgs(const SDChunk *chunk);
  void ChunkCall(const SDChunk *chunk, const rdcarray<rdcstr> &args);
  void LocalVariable(const rdcstr &name, const SDObject *o, rdcstr suffix);
  void InlineValue(const rdcstr &name, const SDObject *o);
  void AssignUnion(const rdcstr &path, const SDObject *obj, bool comment);
  void AddNamedVar(const char *type, const char *name);
  void AddNamedVar(const char *type, const char *name, const char *value);

  void ProcessChunk(size_t chunkId, SDChunk *chunk)
  {
    FileID nextFile = currentFile;
    switch(chunk->metadata.chunkID)
    {
      case(uint32_t)SystemChunk::InitialContentsList:    // fallthrough
      case(uint32_t)SystemChunk::InitialContents: currentFile = nextFile = FILE_RESET; break;
      case(uint32_t)SystemChunk::CaptureBegin: nextFile = FILE_RENDER; break;
    }
    MultiPartSplit();
    WriteChunk(chunkId, chunk);
    currentFile = nextFile;
  }
  virtual uint64_t CanonicalUnionBranch(const SDObject *obj) { return 0; }
  virtual const char *CheckSuccessFunc() const = 0;
  virtual void WriteChunk(size_t chunkId, SDChunk *chunk);
  virtual void WritePreamble(FileID id, CodeFile *file) = 0;
  virtual void WritePostamble(FileID id, CodeFile *file) = 0;
  virtual void WriteDriverInit(const SDChunk *chunk) = 0;
  virtual void WriteInitialContentsList(const SDChunk *chunk) = 0;
  virtual void WriteInitialContents(const SDChunk *chunk) = 0;
  virtual void WriteCaptureBegin(const SDChunk *chunk) = 0;
  virtual void WriteCaptureScope(const SDChunk *chunk) = 0;
  virtual void WriteCaptureEnd(const SDChunk *chunk) = 0;
  virtual ChunkTransformerGraph *GetTransformer() = 0;

  templates::ReflowWriter *findFile(const rdcstr &name);

  class ExecTemplateFunc : public templates::DynFunc
  {
    std::set<rdcstr> globalVars;
    CodeWriter &cw;

  public:
    virtual templates::Value DoCall(templates::State *state, const templates::Value &dot,
                                    rdcarray<templates::Value> args);
    ExecTemplateFunc(CodeWriter &cw)
        : cw(cw),
          DynFunc("ExecTemplate", {templates::ArgSpec::Convertible(templates::ValueType::String),
                                   templates::ArgSpec::Convertible(templates::ValueType::String),
                                   templates::ArgSpec::Optional(templates::ArgSpec::Any())})
    {
    }
  };

  class RegisterGlobalVarFunc : public templates::DynFunc
  {
    std::set<rdcstr> globalVars;
    CodeWriter &cw;

  public:
    virtual templates::Value DoCall(templates::State *state, const templates::Value &dot,
                                    rdcarray<templates::Value> args);
    RegisterGlobalVarFunc(CodeWriter &cw)
        : cw(cw),
          DynFunc("RegisterGlobalVar",
                  {templates::ArgSpec::Convertible(templates::ValueType::String)})
    {
    }
  };

  class GlobalConstFunc : public templates::DynFunc
  {
    std::set<rdcstr> globalVars;
    CodeWriter &cw;

  public:
    virtual templates::Value DoCall(templates::State *state, const templates::Value &dot,
                                    rdcarray<templates::Value> args);
    GlobalConstFunc(CodeWriter &cw)
        : cw(cw),
          DynFunc("GlobalConst", {
                                     templates::ArgSpec::Convertible(templates::ValueType::String),
                                     templates::ArgSpec::Convertible(templates::ValueType::String),
                                     templates::ArgSpec::Convertible(templates::ValueType::String),
                                 })
    {
    }
  };

  class GlobalVarFunc : public templates::DynFunc
  {
    std::set<rdcstr> globalVars;
    CodeWriter &cw;

  public:
    virtual templates::Value DoCall(templates::State *state, const templates::Value &dot,
                                    rdcarray<templates::Value> args);
    GlobalVarFunc(CodeWriter &cw)
        : cw(cw),
          DynFunc("GlobalConst", {
                                     templates::ArgSpec::Any(),
                                     templates::ArgSpec::Optional(templates::ArgSpec::Any()),
                                     templates::ArgSpec::Optional(templates::ArgSpec::Any()),
                                 })
    {
    }
  };
  class BufferVarFunc : public templates::DynFunc
  {
    std::set<rdcstr> globalVars;
    CodeWriter &cw;

  public:
    virtual templates::Value DoCall(templates::State *state, const templates::Value &dot,
                                    rdcarray<templates::Value> args);
    BufferVarFunc(CodeWriter &cw) : cw(cw), DynFunc("BufferVar", {templates::ValueType::SDObject})
    {
    }
  };
  virtual void SetupTemplates(cpp_codec::templates::Template &t)
  {
    templates::AddCommonFuncs(t);
    // templates::AddCommonTemplates(t);
    t.AddFunc(new ExecTemplateFunc(*this));
    t.AddFunc(new RegisterGlobalVarFunc(*this));
    t.AddFunc(new GlobalConstFunc(*this));
    t.AddFunc(new GlobalVarFunc(*this));
    t.AddFunc(new BufferVarFunc(*this));
    t.Load("templates/CPPTemplates.tmpl");
  }
  virtual rdcarray<ResourceFileDesc> GetResourceFiles() { return rdcarray<ResourceFileDesc>(); }
  struct ChunkWriter : public ChunkTransformerNode
  {
    size_t chunkId = 0;
    CodeWriter *parent;
    ChunkWriter(CodeWriter *parent) : parent(parent) {}
    virtual void input(SDChunk *chunk)
    {
      parent->ProcessChunk(chunkId, chunk);
      graph->FreeChunk(chunk);
      ++chunkId;
    }
  };
  inline templates::Template &getTemplates()
  {
    if(mTemplates.NumTemplates() == 0)
      SetupTemplates(mTemplates);
    return mTemplates;
  }

public:
  inline CodeWriter(const rdcstr &path) : rootDirectory(path) {}
  void Close();
  ReplayStatus Structured2Code(const RDCFile &rdc, uint64_t version,
                               const StructuredChunkList &chunks, const StructuredBufferList &buffers,
                               RENDERDOC_ProgressCallback progress);

  const char *GetVar(VarMap &vars, const char *type, const char *name, uint64_t id);
  inline const char *GetResourceVar(const char *type, uint64_t id)
  {
    if(id == 0)
      return "VK_NULL";
    return GetVar(resourceVars, type, type, id);
  }
  inline const char *GetResourceVar(const SDObject *obj)
  {
    return GetResourceVar(obj->Type(), obj->AsUInt64());
  }
  inline const char *GetDataBlobVar(uint64_t id) { return GetDataBlobVar("buffer", id); }
  inline const char *GetDataBlobVar(const char *category, uint64_t id)
  {
    return GetVar(dataBlobVars, "std::vector<uint8_t>", category, id);
  }
};
}    // namespace cpp_codec

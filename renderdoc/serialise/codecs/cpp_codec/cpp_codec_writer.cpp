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

#include <cinttypes>

#include "cpp_codec_common.h"
#include "cpp_codec_writer.h"

namespace cpp_codec
{
const char *FILE_NAMES[FILE_COUNT] = {"main",   "variables", "render", "early_create",
                                      "create", "release",   "init",   "reset"};

ReplayStatus CodeWriter::Structured2Code(const RDCFile &rdc, uint64_t version,
                                         const StructuredChunkList &chunks,
                                         const StructuredBufferList &buffers,
                                         RENDERDOC_ProgressCallback progress)
{
  Open();

  ReadBuffers(buffers);

  ChunkTransformerGraph *transformer = GetTransformer();
  transformer->append(new ChunkWriter(this));
  transformer->Run(chunks);
  delete transformer;

  WriteBufferData(buffers);

  return ReplayStatus::Succeeded;
}

void CodeWriter::Open()
{
  for(uint32_t id = 0; id < FILE_COUNT; ++id)
    OpenFile(FILE_NAMES[id], id, !(id == FILE_MAIN || id == FILE_VAR));

  // 'main' generated file contains functions per each stage of a project.
  // For example, there is a main_render() function that will call all
  // indexed render_i() functions that a trace produces. This applies to
  // render, [pre|post]reset, create, release generated functions. It
  // serves as the glue between the template application, which calls into
  // main_render() or main_create() functions and the core of the generated
  // code.
  for(int i = FILE_RENDER; i < FILE_COUNT; i++)
  {
    files[FILE_MAIN]->PrintLnH("#include \"gen_%s.h\"", FILE_NAMES[i]);
  }
  for(int i = FILE_RENDER; i < FILE_COUNT; i++)
  {
    files[FILE_MAIN]->PrintLnH("void %s_%s();", FILE_NAMES[FILE_MAIN], FILE_NAMES[i]);
  }
  files[FILE_MAIN]->PrintLnH("bool main_should_quit_now();");
  files[FILE_MAIN]->PrintLnH("bool main_parse_command_line_flags(int argc, char ** argv, int* i);");

  rdcarray<ResourceFileDesc> resourceFiles = GetResourceFiles();
  for(auto it = resourceFiles.begin(); it != resourceFiles.end(); ++it)
    WriteTemplateFile(it->filename, it->contents, it->size);
}

void CodeWriter::Close()
{
  for(int i = FILE_RENDER; i < FILE_COUNT; i++)
  {
    if(files[i] == NULL)
      continue;

    uint64_t fileIndex = static_cast<cpp_codec::MultiPartCodeFile *>(files[i])->GetIndex();
    templates::ExecError err = getTemplates().Exec(
        files[FILE_MAIN]->cppWriter(), "CPP.PhaseMain",
        templates::Value::Dict({{"phase", FILE_NAMES[i]}, {"count", fileIndex + 1}}));
    if(!err.IsOk())
    {
      templates::StringWriter w;
      getTemplates().PrintExecError(w, err);
      RDCERR(w.str().c_str());
    }

    //  files[FILE_MAIN]->PrintLn("void %s_%s() {", FILE_NAMES[FILE_MAIN], FILE_NAMES[i]);

    //  WritePreamble((FileID)i, files[FILE_MAIN]);

    //  int fileCount = static_cast<cpp_codec::MultiPartCodeFile *>(files[i])->GetIndex();
    //  for(int j = 0; j <= fileCount; j++)
    //  {
    //    const char *stage = NULL;
    //    if(i == FILE_INIT)
    //      stage = "Initializing Resources";
    //    if(i == FILE_CREATE)
    //      stage = "Creating Resources";

    //    if(i == FILE_INIT || i == FILE_CREATE)
    //      files[FILE_MAIN]->PrintLn("PostStageProgress(\"%s\", %d, %d);", stage, j, fileCount);

    //    files[FILE_MAIN]->PrintLn("%s_%d();", FILE_NAMES[i], j);
    //  }

    //  WritePostamble((FileID)i, files[FILE_MAIN]);

    //  files[FILE_MAIN]->PrintLn("}");
  }

  if(files[FILE_MAIN])
  {
    files[FILE_MAIN]->PrintLn("bool main_should_quit_now() {");
    //// If we are using shims, call into the shim to check if the main message loop should quit
    /// now.
    // if(*shimPrefix)
    //{
    //  files[FILE_MAIN]->PrintLn("return ShimShouldQuitNow();");
    //}
    // else
    //{
    files[FILE_MAIN]->PrintLn("return false;");
    //}
    files[FILE_MAIN]->PrintLn("}");

    files[FILE_MAIN]->PrintLn(
        "bool main_parse_command_line_flags(int argc, char ** argv, int* i) {");
    // if(*shimPrefix)
    //{
    //  files[FILE_MAIN]->PrintLn("return ShimParseCommandLineFlags(argc, argv, i);");
    //}
    // else
    //{
    files[FILE_MAIN]->PrintLn("return false;");
    //}

    files[FILE_MAIN]->PrintLn("}");
  }

  CloseFiles();
}

void CodeWriter::OpenFile(const rdcstr &name, uint32_t pass, bool isMultiPart)
{
  if(files.size() <= pass)
  {
    files.resize(pass + 1);
  }
  if(isMultiPart)
    files[pass] = new MultiPartCodeFile(rootDirectory + "/sample_cpp_trace", name);
  else
    files[pass] = new CodeFile(rootDirectory + "/sample_cpp_trace", name);
  files[pass]->Open(name);
}

void CodeWriter::CloseFiles()
{
  for(auto it = files.begin(); it != files.end(); ++it)
  {
    if(*it)
    {
      delete *it;
      *it = NULL;
    }
  }
}

void CodeWriter::ReadBuffers(const StructuredBufferList &buffers)
{
  CodeFile &file = *files[FILE_CREATE];
  file.PrintLn("{");
  for(size_t i = 0; i < buffers.size(); i++)
  {
    if(buffers[i]->size() == 0)
      continue;

    const char *name = GetDataBlobVar(i);
    files[FILE_CREATE]->PrintLn("ReadBuffer(\"%s\", %s);", name, name);
  }
  file.PrintLn("}");
}

void CodeWriter::WriteBufferData(const StructuredBufferList &buffers)
{
  for(size_t i = 0; i < buffers.size(); ++i)
  {
    if(buffers[i]->size() == 0)
      continue;

    rdcstr name = "buffer_" + ToStr(i);
    rdcstr path = rootDirectory + "/sample_cpp_trace/" + name;
    FILE *f = FileIO::fopen(path.c_str(), "wb");
    assert(f != NULL);
    FileIO::fwrite(buffers[i]->data(), 1, buffers[i]->size(), f);
    FileIO::fclose(f);
  }
}    // namespace cpp_codec

void CodeWriter::MultiPartSplit()
{
  for(auto it = files.begin(); it != files.end(); ++it)
    (*it)->MultiPartSplit();
}

void CodeWriter::WriteTemplateFile(const rdcstr &file, const char *contents, size_t size)
{
  rdcstr path = rootDirectory;
  path.append("/");
  path.append(file);
  FileIO::CreateParentDirectory(path);

  FILE *templateFile = FileIO::fopen(path.c_str(), "wb");
  FileIO::fwrite(contents, 1, size, templateFile);
  fclose(templateFile);
}

void CodeWriter::WriteChunk(size_t chunkId, SDChunk *chunk)
{
  CodeFile &file = *files[currentFile];
  using namespace templates;
  Value::Dict dict;
  dict["chunkId"] = chunkId;
  dict["chunk"] = Value(chunk);
  ExecError err = getTemplates().Exec(file.cppWriter(), "CPP.Chunk", std::move(dict));
  if(!err.IsOk())
  {
    StringWriter w;
    getTemplates().PrintExecError(w, err);
    RDCERR(w.str().c_str());
  }

  // bool implicitBlock = (blockDepth == 0);
  // if(implicitBlock)
  //{
  //  // implicitly place each chunk in a code block

  //  if(!chunk->name.empty())
  //    file.PrintLn("/* %s */", chunk->name.c_str());
  //  file.PrintLn("{");
  //  ++blockDepth;
  //}
  // switch(chunk->metadata.chunkID)
  //{
  //  case(uint32_t)CodeGenChunk::CodeBlock:
  //    if(!implicitBlock)
  //    {
  //      file.PrintLn("{");
  //      ++blockDepth;
  //      VarSet savedLocals(localVars);

  //      for(auto it = chunk->begin(); it != chunk->end(); ++it)
  //        WriteChunk(chunk);

  //      localVars.swap(savedLocals);
  //      --blockDepth;
  //      file.PrintLn("}");
  //    }
  //    else
  //    {
  //      for(auto it = chunk->begin(); it != chunk->end(); ++it)
  //        WriteChunk((SDChunk *)*it);
  //    }
  //    break;
  //  case(uint32_t)CodeGenChunk::Statement:
  //    for(auto it = chunk->begin(); it != chunk->end(); ++it)
  //      file.PrintLn((*it)->data.str.c_str());
  //    break;
  //  // case(uint32_t)SystemChunk::DriverInit:
  //  //  file.PrintLn("/* Driver Initialisation */");
  //  //  WriteDriverInit(chunk);
  //  //  break;
  //  // case(uint32_t)SystemChunk::InitialContentsList:
  //  //  file.PrintLn("/* Initial Contents List */");
  //  //  WriteInitialContentsList(chunk);
  //  //  break;
  //  // case(uint32_t)SystemChunk::InitialContents:
  //  //  file.PrintLn("/* Initial Contents */");
  //  //  if(currentFile == FILE_CREATE)
  //  //    currentFile = FILE_INIT;
  //  //  WriteInitialContents(chunk);
  //  //  break;
  //  // case(uint32_t)SystemChunk::CaptureBegin:
  //  //  currentFile = FILE_RENDER;
  //  //  file.PrintLn("/* Capture Begin */");
  //  //  WriteCaptureBegin(chunk);
  //  //  break;
  //  // case(uint32_t)SystemChunk::CaptureScope:
  //  //  file.PrintLn("/* Capture Scope */");
  //  //  WriteCaptureScope(chunk);
  //  //  break;
  //  // case(uint32_t)SystemChunk::CaptureEnd:
  //  //  file.PrintLn("/* Capture End */");
  //  //  WriteCaptureEnd(chunk);
  //  //  break;
  //  default: WriteGenericChunk(chunk); break;
  //}
  // if(implicitBlock)
  //{
  //  file.PrintLn("}");
  //  localVars.clear();
  //  --blockDepth;
  //}
}

rdcarray<rdcstr> CodeWriter::ChunkArgs(const SDChunk *chunk)
{
  rdcarray<rdcstr> args;
  for(auto it = chunk->begin(); it != chunk->end(); ++it)
  {
    const SDObject *child = *it;
    if(child->IsHidden())
      continue;
    rdcstr arg;
    if(child->type.basetype == SDBasic::Code)
    {
    }
    if(child->type.flags & SDTypeFlags::HasCustomString)
    {
      arg = child->data.str;
    }
    else if(child->IsResource())
    {
      arg = GetResourceVar(child->Type(), child->AsUInt64());
      if(child->type.flags & SDTypeFlags::Nullable)
        arg = "&" + arg;
    }
    else if(!child->IsSimpleType())
    {
      arg = child->name;
      LocalVariable(child->name, child, "");
    }
    else
    {
      arg = ValueStr(child);
    }
    args.push_back(arg);
  }
  return args;
}

void CodeWriter::ChunkCall(const SDChunk *chunk, const rdcarray<rdcstr> &args)
{
  cpp_codec::CodeFile &file = *files[currentFile];
  file.Print("%s(", chunk->Name());
  for(auto it = args.begin(); it != args.end(); ++it)
  {
    if(it != args.begin())
      file.Print(", ");
    file.Print("%s", it->c_str());
  }
  file.Print(")");
}

void CodeWriter::WriteGenericChunk(const SDChunk *chunk)
{
  cpp_codec::CodeFile &file = *files[currentFile];

  rdcarray<rdcstr> args = ChunkArgs(chunk);
  if(chunk->metadata.flags & SDChunkFlags::CheckSuccess)
    file.Print("%s(", CheckSuccessFunc());
  ChunkCall(chunk, args);
  if(chunk->metadata.flags & SDChunkFlags::CheckSuccess)
    file.Print(")");
  file.PrintLn(";");
}

void CodeWriter::LocalVariable(const rdcstr &name, const SDObject *obj, rdcstr suffix)
{
  if(localVars.find(name) != localVars.end())
    return;
  localVars.insert(name);

  cpp_codec::CodeFile &file = *files[currentFile];

  if(obj->IsUnion())
  {
    file.PrintLn("%s %s;", Type(obj), name.c_str());
    AssignUnion(name, obj, false);
  }
  else if(obj->IsNULL())
  {
    file.PrintLn("%s* %s = NULL;", Type(obj), name.c_str());
  }
  else if(obj->IsStruct() || obj->IsArray())
  {
    uint64_t size = obj->NumChildren();
    uint64_t hidden_count = 0;
    // Go through all the children and look for complex structures or variable-
    // size arrays. For each of those, declare and initialize them separately.
    uint32_t index = 0;
    for(auto it = obj->begin(); it != obj->end(); ++it, ++index)
    {
      const SDObject *child = *it;
      if(child->IsHidden())
      {
        hidden_count++;
        continue;
      }
      // Handle cases when the member is a complex data type, such as a complex
      // structure or a variable sized array.
      if(!child->IsInlineable())
      {
        rdcstr childSuffix(suffix);
        rdcstr childName = ChildName(name, child, index, childSuffix);
        LocalVariable(childName, child, childSuffix);
      }
    }

    // Now, declare and initialize the data type. Simple members get inlined.
    // Complex structures or variable arrays get referenced by name.
    if(obj->IsStruct() && !obj->IsPointer())
    {
      file.PrintLn("%s %s = {", Type(obj), name.c_str());
    }
    else if(obj->IsStruct() && obj->IsPointer())
    {
      file.PrintLn("%s %s[1] = {", Type(obj), name.c_str());
    }
    else if(obj->IsArray())
    {
      file.PrintLn("%s %s[%llu] = {", Type(obj), name.c_str(), size - hidden_count);
    }

    index = 0;
    for(auto it = obj->begin(); it != obj->end(); ++it, ++index)
    {
      const SDObject *child = *it;
      if(child->IsHidden())
        continue;

      rdcstr childSuffix = suffix;
      rdcstr childName = cpp_codec::ChildName(name, child, index, childSuffix);
      if(child->IsInlineable())
        InlineValue(childName, child);
      else
        file.PrintLn("/* %s = */ %s,", childName.c_str(), childName.c_str());
    }

    file.PrintLn("};");
  }
  else
  {
    RDCASSERT(obj->IsPointer());
    file.PrintLn("%s %s[1] = { %s };", Type(obj), name.c_str(), ValueStr(obj).c_str());
  }
}

void CodeWriter::InlineValue(const rdcstr &name, const SDObject *obj)
{
  cpp_codec::CodeFile &file = *files[currentFile];
  if(obj->HasCustomString())
  {
    file.PrintLn("/* %s = */ %s,", name.c_str(), obj->data.str.c_str());
  }
  else if(!obj->IsSimpleType())
  {
    file.PrintLn("/* %s = */ {", name.c_str());
    uint32_t index = 0;
    for(auto it = obj->begin(); it != obj->end(); ++it, ++index)
    {
      const SDObject *child = *it;
      if(child->IsHidden())
        continue;
      rdcstr childSuffix;
      rdcstr childName = ChildName(name, child, index, childSuffix);
      InlineValue(childName, child);
      index++;
    }
    file.PrintLn("},");
  }
  else if(obj->IsResource())
  {
    const char *ref = "";
    if(obj->type.flags & SDTypeFlags::Nullable)
      ref = "&";
    file.PrintLn("/* %s = */ %s%s,", name.c_str(), ref, GetResourceVar(obj));
  }
  else
  {
    file.PrintLn("/* %s = */ %s,", name.c_str(), ValueStr(obj).c_str());
  }
}

void CodeWriter::AssignUnion(const rdcstr &path, const SDObject *obj, bool comment)
{
  if(obj->IsStruct() || obj->IsArray() || obj->IsUnion())
  {
    for(uint64_t i = 0; i < obj->NumChildren(); i++)
    {
      SDObject *child = obj->GetChild(i);
      std::string childPath(path);
      bool childComment = false;
      if(obj->IsArray())
      {
        childPath += "[" + std::to_string(i) + "]";
      }
      else if(obj->IsStruct())
      {
        if(obj->IsPointer())
        {
          childPath += "->";
        }
        else
        {
          childPath += ".";
        }
        childPath += std::string(child->Name());
        if(obj->IsUnion())
        {
          if(i != CanonicalUnionBranch(obj))
          {
            childComment = true;
          }
        }
      }
      AssignUnion(childPath, child, comment | childComment);
    }
    return;
  }

  if(obj->IsPointer() && !obj->IsNULL())
  {
    // This would be a non-null, non-array pointer to non-struct;
    // This should never happen.
    RDCASSERT(0);
  }

  std::string commentStr, value;
  if(comment)
  {
    commentStr = "// ";
  }

  if(obj->HasCustomString())
  {
    value = obj->data.str;
  }
  else if(obj->IsResource())
  {
    value = GetResourceVar(obj->Type(), obj->AsUInt64());
  }
  else
  {
    value = ValueStr(obj).c_str();
  }
  files[currentFile]->PrintLn("%s%s = %s;", commentStr.c_str(), path.c_str(), value.c_str());
}

void CodeWriter::AddNamedVar(const char *type, const char *name)
{
  files[FILE_VAR]->PrintLnH("extern %s %s;", type, name).PrintLn("%s %s;", type, name);
}

void CodeWriter::AddNamedVar(const char *type, const char *name, const char *value)
{
  files[FILE_VAR]->PrintLnH("extern %s %s;", type, name).PrintLn("%s %s = %s;", type, name, value);
}

const char *CodeWriter::GetVar(VarMap &vars, const char *type, const char *name, uint64_t id)
{
  auto it = vars.find(id);
  if(it == vars.end())
  {
    rdcstr full_name = rdcstr(name) + "_" + ToStr(id);
    it = vars.insert(std::make_pair(id, full_name)).first;
    AddNamedVar(type, it->second.c_str());
  }
  return it->second.c_str();
}
using namespace templates;

Value CodeWriter::ExecTemplateFunc::DoCall(State *state, const templates::Value &dot,
                                           rdcarray<templates::Value> args)
{
  rdcstr fileName = args[0].GetString();
  rdcstr templateName = args[1].GetString();
  Value newDot;
  if(args.size() >= 3)
    newDot = args[2];
  else
    newDot = dot;
  ReflowWriter *newOut = cw.findFile(fileName);
  if(newOut == NULL)
    return Value::Error("Could not find file %s", fileName);
  return state->ExecTemplate(*newOut, templateName, newDot);
}

templates::Value CodeWriter::GlobalConstFunc::DoCall(templates::State *state,
                                                     const templates::Value &dot,
                                                     rdcarray<templates::Value> args)
{
  rdcstr type = args[0].GetString();
  rdcstr constType = "const " + type;

  rdcstr name = args[1].GetString();

  rdcstr value = args[2].GetString();

  if(globalVars.find(name) == globalVars.end())
  {
    globalVars.insert(name);
    cw.AddNamedVar(constType.c_str(), name.c_str(), value.c_str());
  }
  return Value(name);
}
templates::Value CodeWriter::GlobalVarFunc::DoCall(templates::State *state,
                                                   const templates::Value &dot,
                                                   rdcarray<templates::Value> args)
{
  rdcstr type;
  rdcstr name;
  uint64_t id;
  if(args.size() == 1)
  {
    Value arg = args[0];
    if(!arg.IsSDObject())
      return Value::Error("Expected SDObject, received", ToStr(arg.Type()));
    const Value::WrappedSDObject &obj = arg.GetSDObject();
    type = obj.type().name;
    name = type;
    id = arg.ToUInt().GetUInt();
  }
  else if(args.size() == 2)
  {
    Value tyVal = args[0].ToString();
    if(tyVal.IsError())
      return tyVal;
    type = tyVal.GetString();
    name = type;

    Value idVal = args[1].ToUInt();
    if(idVal.IsError())
      return idVal;
    id = idVal.GetUInt();
  }
  else if(args.size() == 3)
  {
    Value tyVal = args[0].ToString();
    if(tyVal.IsError())
      return tyVal;
    type = tyVal.GetString();

    Value nameVal = args[1].ToString();
    if(nameVal.IsError())
      return nameVal;
    name = nameVal.GetString();

    Value idVal = args[2].ToUInt();
    if(idVal.IsError())
      return idVal;
    id = idVal.GetUInt();
  }
  else
  {
    return Value::Error("Usage: GlobalVar [type [name]] id");
  }
  rdcstr varName;
  varName.sprintf("%s_%" PRIu64, name, id);
  if(globalVars.find(varName) == globalVars.end())
  {
    globalVars.insert(varName);
    cw.AddNamedVar(type.c_str(), varName.c_str());
  }
  return Value(varName);
}
templates::Value CodeWriter::BufferVarFunc::DoCall(templates::State *state,
                                                   const templates::Value &dot,
                                                   rdcarray<templates::Value> args)
{
  const Value::WrappedSDObject &sdobj = args[0].GetSDObject();
  if(!sdobj.IsBuffer())
    return Value::Error("Expected Byte Buffer SDObject, received", ToStr(sdobj.type().basetype));
  uint64_t id = sdobj.data().basic.u;
  return Value(rdcstr(cw.GetDataBlobVar(id)));
}
ReflowWriter *CodeWriter::findFile(const rdcstr &name)
{
  int32_t i = name.indexOf('.');
  if(i < 0)
    i = (int32_t)name.size();
  rdcstr ext = name.substr(i);
  rdcstr baseName = name.substr(0, i);
  for(int id = 0; id < FILE_COUNT; ++id)
  {
    if(FILE_NAMES[id] == baseName)
    {
      if(ext == "" || ext == ".cpp")
        return &files[id]->cppWriter();
      else if(ext == ".h")
        return &files[id]->headerWriter();
    }
  }
  return NULL;
}
templates::Value CodeWriter::RegisterGlobalVarFunc::DoCall(templates::State *state,
                                                           const templates::Value &dot,
                                                           rdcarray<templates::Value> args)
{
  rdcstr name = args[0].GetString();
  if(globalVars.find(name) == globalVars.end())
  {
    globalVars.insert(name);
    return Value(true);
  }
  return Value(false);
}
}    // namespace cpp_codec

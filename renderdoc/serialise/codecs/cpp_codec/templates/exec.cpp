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

#include "exec.h"
#include "../cpp_codec_resource.h"
#include "lex.h"

namespace cpp_codec
{
namespace templates
{
using namespace parse;
void Template::AddFunc(DynFunc *func)
{
  funcs[func->Name()] = func;
}
parse::ParseError Template::Parse(const rdcstr &name, const rdcstr &src)
{
  size_t sourceId = sources.size();
  sources.push_back({name, src});
  Parser parser(name, src, sourceId, roots, alloc);
  parser.Parse(name);
  return parser.Error();
}
void Template::Load(const rdcstr &resourceName)
{
  const ResourceFileDesc *desc = FindResourceDesc(resourceName);
  if(desc == NULL)
  {
    RDCERR("Could not find template %s", resourceName.c_str());
    return;
  }
  templates::parse::ParseError err = Parse(resourceName, rdcstr(desc->contents, desc->size));
  if(!err.IsOk())
  {
    StringWriter w;
    PrintParseError(w, err);
    RDCERR(w.str().c_str());
  }
}

ExecError Template::Exec(Writer &out, const rdcstr &name, Value dot)
{
  ReflowWriter r(&out);
  auto rootIt = roots.find(name);
  if(rootIt == roots.end())
  {
    ExecError err;
    err.msg.sprintf("Could not find template \"%s\"", name);
    return err;
  }
  State st(*this, name, r);
  st.PushVar("", dot);
  Value result = st.Walk(dot, rootIt->second);
  r.Close();
  if(result.IsError())
    return result.GetError();
  else
    return ExecError();
}

bool State::SetVar(const rdcstr &name, Value value)
{
  for(int i = (int)vars.size() - 1; i >= 0; --i)
  {
    if(vars[i].first == name)
    {
      vars[i].second = value;
      return true;
    }
  }
  return false;
}

Value State::GetVar(const rdcstr &name) const
{
  for(int i = (int)vars.size() - 1; i >= 0; --i)
  {
    if(vars[i].first == name)
      return vars[i].second;
  }
  return errorf(NULL, "Unknown variable $%s", name);
}

// void State::WriteLines(Writer &w, const rdcstr &str)
//{
//  const char *lineBegin = str.begin();
//  for(const char *it = str.begin(); it != str.end(); ++it)
//  {
//    const char *lineEnd = NULL;
//    if(*it == '\n')
//      lineEnd = it;
//    if(*it == '\r' && *(it + 1) == '\n')
//      lineEnd = it + 1;
//    if(lineEnd)
//    {
//      if(atNewLine && !prefix.empty())
//        w.Write(prefix.c_str(), prefix.size());
//      w.Write(lineBegin, lineEnd - lineBegin + 1);
//      atNewLine = true;
//      it = lineEnd;
//      lineBegin = lineEnd + 1;
//    }
//  }
//  if(lineBegin != str.end())
//  {
//    if(atNewLine && !prefix.empty())
//      w.Write(prefix.c_str(), prefix.size());
//    w.Write(lineBegin, str.end() - lineBegin);
//    atNewLine = false;
//  }
//}

// bool printSDObject(Writer &w, Value obj)
//{
//  if(obj == NULL)
//    return true;
//  if(obj->HasCustomString())
//  {
//    w.Write(obj->data.str);
//    return true;
//  }
//  switch(obj->type.basetype)
//  {
//    case SDBasic::Boolean: w.Write(ToStr(obj->data.basic.b)); return true;
//    case SDBasic::Buffer: w.Write(ToStr(obj->data.basic.u)); return true;
//    case SDBasic::Character: w.Write(ToStr(obj->data.basic.c)); return true;
//    case SDBasic::Enum: w.Write(ToStr(obj->data.basic.i)); return true;
//    case SDBasic::Float: w.Write(ToStr(obj->data.basic.d)); return true;
//    case SDBasic::Null: return true;
//    case SDBasic::Resource: w.Write(ToStr(obj->data.basic.u)); return true;
//    case SDBasic::SignedInteger: w.Write(ToStr(obj->data.basic.i)); return true;
//    case SDBasic::String: w.Write(obj->data.str); return true;
//    case SDBasic::UnsignedInteger: w.Write(ToStr(obj->data.basic.u)); return true;
//    default: return false;
//  }
//}

void State::Print(Value v)
{
  out.Print(v);
}
// void State::Print(Writer &w, Value obj)
//{
//  if(obj->HasCustomString())
//  {
//    w.Write(obj->data.str);
//    return;P
//  }
//  switch(obj->type.basetype)
//  {
//    case SDBasic::Boolean: w.Write(ToStr(obj->data.basic.b)); break;
//    case SDBasic::Buffer: w.Write(ToStr(obj->data.basic.u)); break;
//    case SDBasic::Character: w.Write(ToStr(obj->data.basic.c)); break;
//    case SDBasic::Enum: w.Write(ToStr(obj->data.basic.i)); break;
//    case SDBasic::Float: w.Write(ToStr(obj->data.basic.d)); break;
//    case SDBasic::Null: break;
//    case SDBasic::Resource: w.Write(ToStr(obj->data.basic.u)); break;
//    case SDBasic::SignedInteger: w.Write(ToStr(obj->data.basic.i)); break;
//    case SDBasic::String: w.Write(obj->data.str); break;
//    case SDBasic::UnsignedInteger: w.Write(ToStr(obj->data.basic.u)); break;
//    default: errorf("Could not print object type %s", ToStr(obj->type.basetype)); break;
//  }
//}

Value State::EvalPipeline(Value dot, const PipeNode *pipe)
{
  if(pipe == NULL || pipe->numCmds() == 0)
    return errorf(pipe, "Evaluating empty pipe");
  // return Value::Error("Evaluating empty pipe");
  auto cmdIter = pipe->beginCmds();
  Value value = EvalCommand(dot, *cmdIter, {});
  if(value.IsError())
    return TagError(*cmdIter, value);
  for(++cmdIter; cmdIter != pipe->endCmds(); ++cmdIter)
  {
    value = EvalCommand(dot, *cmdIter, {value});
    if(value.IsError())
      return TagError(*cmdIter, value);
  }
  for(auto declIter = pipe->beginDecls(); declIter != pipe->endDecls(); ++declIter)
  {
    if(pipe->isAssign())
    {
      if(!SetVar(*declIter, value))
        return errorf(pipe, "Assignment to unknown variable `%s`", *declIter);
    }
    else
    {
      PushVar(*declIter, value);
    }
  }
  return value;
}

Value State::EvalCommand(Value dot, const CommandNode *cmd,
                         const std::initializer_list<Value> &finalArgs)
{
  auto argIter = cmd->begin();
  const Node *arg0 = *argIter;
  if(arg0->type() == NodeType::Func)
  {
    rdcarray<Value> args;
    size_t argCount = cmd->size() + finalArgs.size() - 1;
    args.reserve(argCount);
    for(++argIter; argIter != cmd->end(); ++argIter)
    {
      Value arg = EvalArg(dot, *argIter);
      if(arg.IsError())
        arg = TagError(*argIter, arg);
      args.push_back(arg);
    }
    for(auto it = finalArgs.begin(); it != finalArgs.end(); ++it)
      args.push_back(*it);
    return EvalCall(dot, (const FuncNode *)arg0, args);
  }
  if(cmd->size() > 1 || finalArgs.size() > 0)
    return errorf(cmd, "Attempted to apply arguments to non-function");
  return EvalArg(dot, arg0);
}
Value State::EvalCall(Value dot, const FuncNode *funcNode, const rdcarray<Value> args)
{
  auto funcIter = tmpl.funcs.find(funcNode->name());
  if(funcIter == tmpl.funcs.end())
    return errorf(funcNode, "Unknown function '%s'", funcNode->name());
  DynFunc *func = funcIter->second;
  return func->Call(this, dot, args);
}

Value State::EvalArg(Value dot, const Node *arg)
{
  switch(arg->type())
  {
    case NodeType::Literal:
    {
      switch(((const LiteralNode *)arg)->literalType())
      {
        case LiteralType::Null: return Value();
        case LiteralType::Bool: return Value(((const BoolNode *)arg)->value());
        case LiteralType::Int: return Value(((const IntNode *)arg)->value());
        case LiteralType::UInt: return Value(((const UIntNode *)arg)->value());
        case LiteralType::Float: return Value(((const FloatNode *)arg)->value());
        case LiteralType::String: return Value(((const StringNode *)arg)->value());
        case LiteralType::Char: return Value(((const CharNode *)arg)->value());
        default:
          return errorf(arg, "Unknown literal type: %s",
                        ToStr(((const LiteralNode *)arg)->literalType()));
      }
    }
    case NodeType::Func:
      // call with no args
      return EvalCall(dot, (const FuncNode *)arg, {});
    case NodeType::Identifier:
    {
      const IdentifierNode *ident = (const IdentifierNode *)arg;
      if(ident->identType() == IdentType::Dot)
        return dot;
      else
        return GetVar(ident->name());
    }
    case NodeType::Chain:
    {
      const ChainNode *chain = (const ChainNode *)arg;
      Value obj = EvalArg(dot, chain->object());
      if(obj.IsError())
        return TagError(chain->object(), obj);
      for(auto fieldIter = chain->begin(); fieldIter != chain->end(); ++fieldIter)
      {
        obj = obj.Field(fieldIter->c_str());
        if(obj.IsError())
          return TagError(chain, obj);
      }
      return obj;
    }
    case NodeType::Pipe: return EvalPipeline(dot, (const PipeNode *)arg);
    default: return errorf(arg, "Unknown node type '%s' for arg", ToStr(arg->type()));
  }
}
//
// bool IsTrue(Value v)
//{
//  Value err;
//  switch(v.Type())
//  {
//    case ValueType::Null: return false;
//    case ValueType::String: return v.strSize() > 0;
//    case ValueType::Bool: return v.Cast<bool>(err) && !err.IsError();
//    case ValueType::Int: return v.Cast<int64_t>(err) && !err.IsError();
//    case ValueType::UInt: return v.Cast<uint64_t>(err) && !err.IsError();
//    case ValueType::Float: return v.Cast<double>(err) && !err.IsError();
//    case ValueType::Char: return v.Cast<char>(err) && !err.IsError();
//    case ValueType::Dict: return v.Cast<const Value::Dict *>(err)->size() && !err.IsError();
//    case ValueType::Error: return false;
//    case ValueType::SDObject:
//    {
//      SDObject *obj = v.Cast<SDObject *>(err);
//      if(err.IsError())
//        return false;
//      if(obj->IsNULL())
//        return false;
//      switch(obj->type.basetype)
//      {
//        case SDBasic::Boolean: return obj->data.basic.b;
//        case SDBasic::Character: return obj->data.basic.c != 0;
//        case SDBasic::Enum: return obj->data.basic.i != 0;
//        case SDBasic::Float: return obj->data.basic.d != 0;
//        case SDBasic::SignedInteger: return obj->data.basic.i != 0;
//        case SDBasic::String: return !obj->data.str.isEmpty();
//        case SDBasic::UnsignedInteger: return obj->data.basic.u != 0;
//        default: return true;
//      }
//    }
//  }
//  return false;
//}

Value State::Macro(const rdcstr &name, Value newDot)
{
  auto tmplIter = tmpl.roots.find(name);
  if(tmplIter == tmpl.roots.end())
    return errorf(NULL, "Unknown template '%s'", name);
  ListNode *newRoot = tmplIter->second;
  StringWriter sw;
  ReflowWriter rw(&sw);
  State newState(tmpl, name, rw);
  newState.PushVar("", newDot);
  Value res = newState.Walk(newDot, newRoot);
  if(res.IsError())
  {
    // the error probably already has a stack trace, but we want to add the higher stack trace of
    // the `Macro` call.
    res.GetError().needsStack = true;
    return res;
  }
  return Value(sw.str());
}

Value State::ExecTemplate(ReflowWriter &newOut, const rdcstr &name, Value newDot)
{
  auto tmplIter = tmpl.roots.find(name);
  if(tmplIter == tmpl.roots.end())
    return errorf(NULL, "Unknown template '%s'", name);
  ListNode *newRoot = tmplIter->second;
  State newState(tmpl, name, newOut);
  newState.PushVar("", newDot);
  Value res = newState.Walk(newDot, newRoot);
  if(res.IsError())
  {
    // the error probably already has a stack trace, but we want to add the higher stack trace of
    // the `Macro` call.
    res.GetError().needsStack = true;
  }
  return res;
}

Value State::Walk(Value dot, const Node *n)
{
  switch(n->type())
  {
    case NodeType::Action:
    {
      const ActionNode *action = (const ActionNode *)n;
      Value val = EvalPipeline(dot, action->pipe());
      if(val.IsError())
        return TagError(action->pipe(), val);
      if(action->pipe()->numDecls() == 0)
        Print(val);
      break;
    }
    case NodeType::If:
    {
      const IfNode *ifNode = (const IfNode *)n;
      StackMark mark = MarkVarStack();
      Value val = EvalPipeline(dot, ifNode->pipe());
      if(val.IsError())
        return TagError(ifNode->pipe(), val);
      Value bVal = val.ToBool();
      if(bVal.IsError())
        return TagError(ifNode->pipe(), bVal);
      if(bVal.GetBool())
      {
        Value err = Walk(dot, ifNode->body());
        if(err.IsError())
          return TagError(ifNode->body(), err);
      }
      else
      {
        Value err = Walk(dot, ifNode->alt());
        if(err.IsError())
          return TagError(ifNode->alt(), err);
      }
      break;
    }
    case NodeType::With:
    {
      const WithNode *withNode = (const WithNode *)n;
      StackMark mark = MarkVarStack();
      Value val = EvalPipeline(dot, withNode->pipe());
      if(val.IsError())
        return TagError(withNode->pipe(), val);
      Value bVal = val.ToBool();
      if(bVal.IsError())
        return TagError(withNode->pipe(), bVal);
      if(bVal.GetBool())
      {
        Value err = Walk(val, withNode->body());
        if(err.IsError())
          return TagError(withNode->body(), err);
      }
      else
      {
        Value err = Walk(dot, withNode->alt());
        if(err.IsError())
          return TagError(withNode->alt(), err);
      }
      break;
    }
    case NodeType::Prefix:
    {
      const PrefixNode *prefixNode = (const PrefixNode *)n;
      StackMark mark = MarkVarStack();
      Value prefixObj = EvalPipeline(dot, prefixNode->pipe());
      StringWriter prefixSW;
      ReflowWriter prefixRW(&prefixSW);
      prefixRW.Print(prefixObj);
      rdcstr prefixStr = prefixSW.str();
      out.PushPrefix(prefixStr);
      Value err = Walk(dot, prefixNode->body());
      if(err.IsError())
        return TagError(prefixNode->body(), err);
      out.PopPrefix();
      break;
    }
    case NodeType::Range:
    {
      const RangeNode *rangeNode = (const RangeNode *)n;
      StackMark outerMark = MarkVarStack();
      Value val = EvalPipeline(dot, rangeNode->pipe());
      if(val.IsError())
        return TagError(rangeNode->pipe(), val);

      bool empty = true;
      Value err = val.Visit([this, rangeNode, &empty](Value index, Value key, Value elem) -> Value {
        empty = false;
        if(rangeNode->pipe()->numDecls() > 2)
          SetTopVar(3, index);
        if(rangeNode->pipe()->numDecls() > 1)
          SetTopVar(2, key);
        if(rangeNode->pipe()->numDecls() > 0)
          SetTopVar(1, elem);
        StackMark innerMark = MarkVarStack();
        return Walk(elem, rangeNode->body());
      });
      if(err.IsError())
        return TagError(rangeNode->body(), err);
      // for(auto it = val.begin(); it != val.end(); ++it, ++index)
      //{
      //  if(rangeNode->pipe()->numDecls() > 1)
      //    SetTopVar(2, Value(index));
      //  StackMark innerMark = MarkVarStack();
      //  Value elem = *it;
      //  if(rangeNode->pipe()->numDecls() > 0)
      //    SetTopVar(1, elem);
      //  Value err = Walk(elem, rangeNode->body());
      //  if(err.IsError())
      //    return TagError(rangeNode->body(), err);
      //}
      if(empty)
        return Walk(dot, rangeNode->alt());
      break;
    }
    case NodeType::List:
    {
      const ListNode *listNode = (const ListNode *)n;
      for(auto it = listNode->begin(); it != listNode->end(); ++it)
      {
        Value err = Walk(dot, *it);
        if(err.IsError())
          return TagError(*it, err);
      }
      break;
    }
    case NodeType::Template:
    {
      const TemplateNode *tmplNode = (const TemplateNode *)n;
      auto tmplIter = tmpl.roots.find(tmplNode->name());
      if(tmplIter == tmpl.roots.end())
        return errorf(tmplNode, "Unknown template '%s'", tmplNode->name());
      ListNode *newRoot = tmplIter->second;
      Value newDot = EvalPipeline(dot, tmplNode->pipe());
      out.Write(ReflowWriter::SUPPRESS_WHITESPACE);
      State newState(tmpl, tmplNode->name(), out);
      newState.PushVar("", newDot);
      Value res = newState.Walk(newDot, newRoot);
      if(res.IsError())
      {
        res.GetError().needsStack = true;
        return TagError(tmplNode, res);
      }
      break;
    }
    case NodeType::Text: out.Write(((const TextNode *)n)->text()); break;
    default: return errorf(n, "unknown node type: %s", ToStr(n->type())); break;
  }
  return Value();
}

}    // namespace templates
}    // namespace cpp_codec

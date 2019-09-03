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

#include "lex.h"
#include "writer.h"

namespace cpp_codec
{
namespace templates
{
namespace parse
{
rdcstr escape(const rdcstr &str, bool isChar = false);
bool unescape(const rdcstr &str, rdcstr &out);

class Parser;

enum class NodeType
{
  Literal,
  Text,          // Plain text.
  Command,       // An element of a pipeline.
  Pipe,          // A pipeline of commands.
  Action,        // A non-control action such as a field evaluation.
  Chain,         // A sequence of field accesses.
  Identifier,    // An identifier; always a function name.
  Func,
  List,        // A list of NodeTypes.
  If,          // An if action.
  Range,       // A range action.
  Else,        // An else action. Not added to tree.
  End,         // An end action. Not added to tree.
  Template,    // A template invocation action.
  With,        // A with action.
  Prefix,      // A line-prefix action.
};

enum class LiteralType
{
  Null,
  Bool,
  Int,
  UInt,
  Float,
  String,
  Char,
};

enum class IdentType
{
  Dot,
  Var,
};

class Parser;

struct SourceRange
{
  size_t sourceId = 0;
  size_t offset = 0;
  size_t size = 0;
  inline SourceRange(size_t sourceId, size_t offset, size_t size)
      : sourceId(sourceId), offset(offset), size(size)
  {
  }
  inline SourceRange(size_t sourceId, const lex::Item &item)
      : sourceId(sourceId), offset(item.offset), size(item.size)
  {
  }
};

// find the offset of the first non-newline character in this line
int BeginningOfLine(const rdcstr &source, int offset);

// find the offset of the newline character that ends this line (or the string size, if this is the
// last line)
int EndOfLine(const rdcstr &source, int offset);

int LineNumber(const rdcstr &source, int offset);

bool PrintSourceRange(Writer &w, const rdcstr &source, size_t offset, size_t size, size_t context,
                      bool printColumnIndicator);

class Node
{
  friend class Parser;

protected:
  NodeType mType;
  SourceRange mSource;

public:
  virtual ~Node() = default;
  Node(NodeType type, const SourceRange &source) : mType(type), mSource(source) {}
  inline NodeType type() const { return mType; }
  inline const SourceRange &source() const { return mSource; }
};

class LiteralNode : public Node
{
  LiteralType mLiteralType;

public:
  LiteralNode(SourceRange source, LiteralType literalType)
      : Node(NodeType::Literal, source), mLiteralType(literalType)
  {
  }
  LiteralType literalType() const { return mLiteralType; }
};

class NullNode : public LiteralNode
{
public:
  NullNode(SourceRange source) : LiteralNode(source, LiteralType::Null) {}
};

class BoolNode : public LiteralNode
{
  bool mValue;

public:
  BoolNode(SourceRange source, bool value) : LiteralNode(source, LiteralType::Bool), mValue(value)
  {
  }
  bool value() const { return mValue; }
};

class IntNode : public LiteralNode
{
  int64_t mValue;

public:
  IntNode(SourceRange source, int64_t value) : LiteralNode(source, LiteralType::Int), mValue(value)
  {
  }
  int64_t value() const { return mValue; }
};

class UIntNode : public LiteralNode
{
  uint64_t mValue;

public:
  UIntNode(SourceRange source, uint64_t value)
      : LiteralNode(source, LiteralType::UInt), mValue(value)
  {
  }
  uint64_t value() const { return mValue; }
};

class FloatNode : public LiteralNode
{
  double mValue;

public:
  FloatNode(SourceRange source, double value)
      : LiteralNode(source, LiteralType::Float), mValue(value)
  {
  }
  double value() const { return mValue; }
};

class StringNode : public LiteralNode
{
  rdcstr mValue;

public:
  StringNode(SourceRange source, const rdcstr &value)
      : LiteralNode(source, LiteralType::String), mValue(value)
  {
  }
  const rdcstr &value() const { return mValue; }
};

class CharNode : public LiteralNode
{
  char mValue;

public:
  CharNode(SourceRange source, char value) : LiteralNode(source, LiteralType::Char), mValue(value)
  {
  }
  char value() const { return mValue; }
};

class TextNode : public Node
{
  rdcstr mText;

public:
  TextNode(SourceRange source, const rdcstr &text) : Node(NodeType::Text, source), mText(text) {}
  const rdcstr &text() const { return mText; }
};

class CommandNode : public Node
{
  friend class Parser;
  rdcarray<Node *> mOperands;

public:
  size_t size() const { return mOperands.size(); }
  const Node *const *begin() const { return mOperands.begin(); }
  const Node *const *end() const { return mOperands.end(); }
  CommandNode(SourceRange source) : Node(NodeType::Command, source) {}
  CommandNode(SourceRange source, const std::initializer_list<Node *> &operands)
      : Node(NodeType::Command, source), mOperands(operands)
  {
  }
};

class IdentifierNode : public Node
{
  IdentType mIdentType;
  rdcstr mName;

public:
  IdentType identType() const { return mIdentType; }
  const rdcstr &name() const { return mName; }
  IdentifierNode(SourceRange source, IdentType identType, const rdcstr &ident)
      : Node(NodeType::Identifier, source), mIdentType(identType), mName(ident)
  {
  }
};

class FuncNode : public Node
{
  rdcstr mName;

public:
  const rdcstr &name() const { return mName; }
  FuncNode(SourceRange source, const rdcstr &ident) : Node(NodeType::Func, source), mName(ident) {}
};

class PipeNode : public Node
{
  friend class Parser;
  bool mIsAssign = false;
  rdcarray<rdcstr> mDeclarations;
  rdcarray<CommandNode *> mCommands;

public:
  bool isAssign() const { return mIsAssign; }
  size_t numDecls() const { return mDeclarations.size(); }
  const rdcstr *beginDecls() const { return mDeclarations.begin(); }
  const rdcstr *endDecls() const { return mDeclarations.end(); }
  size_t numCmds() const { return mCommands.size(); }
  const CommandNode *const *beginCmds() const { return mCommands.begin(); }
  const CommandNode *const *endCmds() const { return mCommands.end(); }
  PipeNode(SourceRange source) : Node(NodeType::Pipe, source) {}
  PipeNode(SourceRange source, const std::initializer_list<CommandNode *> &commands)
      : Node(NodeType::Pipe, source), mCommands(commands)
  {
  }
  PipeNode(SourceRange source, bool isAssign, const std::initializer_list<rdcstr> &decls,
           const std::initializer_list<CommandNode *> &commands)
      : Node(NodeType::Pipe, source), mIsAssign(isAssign), mDeclarations(decls), mCommands(commands)
  {
  }
};

class ActionNode : public Node
{
  PipeNode *mPipe;

public:
  ActionNode(SourceRange source, PipeNode *pipe) : Node(NodeType::Action, source), mPipe(pipe) {}
  inline const PipeNode *pipe() const { return mPipe; }
};

class ChainNode : public Node
{
  friend class Parser;
  Node *mObj;
  rdcarray<rdcstr> mFields;

public:
  size_t size() const { return mFields.size(); }
  const Node *object() const { return mObj; }
  const rdcstr *begin() const { return mFields.begin(); }
  const rdcstr *end() const { return mFields.end(); }
  ChainNode(SourceRange source, Node *obj) : Node(NodeType::Chain, source), mObj(obj) {}
  ChainNode(SourceRange source, Node *obj, const std::initializer_list<rdcstr> &fields)
      : Node(NodeType::Chain, source), mObj(obj), mFields(fields)
  {
  }
};

class ListNode : public Node
{
  friend class Parser;
  rdcarray<Node *> mChildren;

public:
  ListNode(SourceRange source) : Node(NodeType::List, source) {}
  ListNode(SourceRange source, std::initializer_list<Node *> children)
      : Node(NodeType::List, source), mChildren(children)
  {
  }
  size_t size() const { return mChildren.size(); }
  const Node *const *begin() const { return mChildren.begin(); }
  const Node *const *end() const { return mChildren.end(); }
};

class BranchNode : public Node
{
  friend class Parser;

  PipeNode *mPipe;
  ListNode *mBody;
  ListNode *mAlt;

public:
  BranchNode(NodeType type, SourceRange source, PipeNode *pipe, ListNode *body, ListNode *alt)
      : Node(type, source), mPipe(pipe), mBody(body), mAlt(alt)
  {
  }
  const PipeNode *pipe() const { return mPipe; }
  const ListNode *body() const { return mBody; }
  const ListNode *alt() const { return mAlt; }
};

class IfNode : public BranchNode
{
  friend class Parser;

public:
  IfNode(SourceRange source, PipeNode *pipe, ListNode *body, ListNode *alt)
      : BranchNode(NodeType::If, source, pipe, body, alt)
  {
  }
};

class RangeNode : public BranchNode
{
  friend class Parser;

public:
  RangeNode(SourceRange source, PipeNode *pipe, ListNode *body, ListNode *alt)
      : BranchNode(NodeType::Range, source, pipe, body, alt)
  {
  }
};

class WithNode : public BranchNode
{
  friend class Parser;

public:
  WithNode(SourceRange source, PipeNode *pipe, ListNode *body, ListNode *alt)
      : BranchNode(NodeType::With, source, pipe, body, alt)
  {
  }
};

class PrefixNode : public BranchNode
{
  friend class Parser;

public:
  PrefixNode(SourceRange source, PipeNode *pipe, ListNode *body)
      : BranchNode(NodeType::Prefix, source, pipe, body, NULL)
  {
  }
};

class ElseNode : public Node
{
public:
  ElseNode(SourceRange source) : Node(NodeType::Else, source) {}
};

class EndNode : public Node
{
public:
  EndNode(SourceRange source) : Node(NodeType::End, source) {}
};

class TemplateNode : public Node
{
  rdcstr mName;
  PipeNode *mPipe;

public:
  TemplateNode(SourceRange source, const rdcstr &name, PipeNode *pipe)
      : Node(NodeType::Template, source), mName(name), mPipe(pipe)
  {
  }
  inline const rdcstr &name() const { return mName; }
  inline const PipeNode *pipe() const { return mPipe; }
};

class NodeAllocator
{
protected:
  rdcarray<Node *> nodes;

public:
  ~NodeAllocator();
  template <typename TNode, typename... Args>
  TNode *newNode(SourceRange source, Args... args)
  {
    TNode *node = new TNode(source, args...);
    nodes.push_back(node);
    return node;
  }
  NullNode *newNull(SourceRange source) { return newNode<NullNode>(source); }
  BoolNode *newBool(SourceRange source, bool value) { return newNode<BoolNode>(source, value); }
  IntNode *newInt(SourceRange source, int64_t value) { return newNode<IntNode>(source, value); }
  UIntNode *newUInt(SourceRange source, uint64_t value) { return newNode<UIntNode>(source, value); }
  FloatNode *newFloat(SourceRange source, double value)
  {
    return newNode<FloatNode>(source, value);
  }
  StringNode *newString(SourceRange source, const rdcstr &value)
  {
    return newNode<StringNode>(source, value);
  }
  CharNode *newChar(SourceRange source, char value) { return newNode<CharNode>(source, value); }
  TextNode *newText(SourceRange source, const rdcstr &text)
  {
    return newNode<TextNode>(source, text);
  }
  ActionNode *newAction(SourceRange source, PipeNode *pipe)
  {
    return newNode<ActionNode>(source, pipe);
  }
  ChainNode *newChain(SourceRange source, Node *obj) { return newNode<ChainNode>(source, obj); }
  ChainNode *newChain(SourceRange source, Node *obj, const std::initializer_list<rdcstr> &operands)
  {
    return newNode<ChainNode>(source, obj, operands);
  }
  CommandNode *newCommand(SourceRange source) { return newNode<CommandNode>(source); }
  CommandNode *newCommand(SourceRange source, const std::initializer_list<Node *> &operands)
  {
    return newNode<CommandNode>(source, operands);
  }
  FuncNode *newFunc(SourceRange source, const rdcstr &name)
  {
    return newNode<FuncNode>(source, name);
  }
  IdentifierNode *newDot(SourceRange source)
  {
    return newNode<IdentifierNode>(source, IdentType::Dot, rdcstr("."));
  }
  IdentifierNode *newVar(SourceRange source, const rdcstr &name)
  {
    return newNode<IdentifierNode>(source, IdentType::Var, name);
  }
  ListNode *newList(SourceRange source) { return newNode<ListNode>(source); }
  ListNode *newList(SourceRange source, const std::initializer_list<Node *> &children)
  {
    return newNode<ListNode>(source, children);
  }
  PipeNode *newPipe(SourceRange source) { return newNode<PipeNode>(source); }
  PipeNode *newPipe(SourceRange source, const std::initializer_list<CommandNode *> &commands)
  {
    return newNode<PipeNode>(source, commands);
  }
  PipeNode *newPipe(SourceRange source, bool isAssign, const std::initializer_list<rdcstr> &decls,
                    const std::initializer_list<CommandNode *> &commands)
  {
    return newNode<PipeNode>(source, isAssign, decls, commands);
  }
  IfNode *newIf(SourceRange source, PipeNode *pipe, ListNode *body, ListNode *alt)
  {
    return newNode<IfNode>(source, pipe, body, alt);
  }
  IfNode *newIf(SourceRange source, PipeNode *pipe, ListNode *body)
  {
    return newNode<IfNode>(source, pipe, body, newList(source));
  }
  RangeNode *newRange(SourceRange source, PipeNode *pipe, ListNode *body, ListNode *alt)
  {
    return newNode<RangeNode>(source, pipe, body, alt);
  }
  RangeNode *newRange(SourceRange source, PipeNode *pipe, ListNode *body)
  {
    return newNode<RangeNode>(source, pipe, body, newList(source));
  }
  WithNode *newWith(SourceRange source, PipeNode *pipe, ListNode *body, ListNode *alt)
  {
    return newNode<WithNode>(source, pipe, body, alt);
  }
  WithNode *newWith(SourceRange source, PipeNode *pipe, ListNode *body)
  {
    return newNode<WithNode>(source, pipe, body, newList(source));
  }
  PrefixNode *newPrefix(SourceRange source, PipeNode *pipe, ListNode *body)
  {
    return newNode<PrefixNode>(source, pipe, body);
  }
  ElseNode *newElse(SourceRange source) { return newNode<ElseNode>(source); }
  EndNode *newEnd(SourceRange source) { return newNode<EndNode>(source); }
  TemplateNode *newTemplate(SourceRange source, const rdcstr &name, PipeNode *pipe)
  {
    return newNode<TemplateNode>(source, name, pipe);
  }
};

struct ParseError
{
  rdcstr msg;
  size_t sourceId = 0u;
  size_t offset = 0u;
  size_t size = 0u;
  inline bool IsOk() const { return msg.empty(); }
  void Print(Writer &w, const rdcstr &name, const rdcstr &source)
  {
    int lineBegin = LineNumber(source, (int)offset);
    int lineEnd = LineNumber(source, (int)(offset + size - 1));
    w.Write(name);
    if(lineBegin == lineEnd)
      w.Printf("(%d): ", lineBegin);
    else
      w.Printf("(%d-%d): ", lineBegin, lineEnd);
    w.Write(msg);
    w.NewLine();
    PrintSourceRange(w, source, offset, size, 2, true);
  }
};

class Parser
{
  const rdcstr &sourceName;
  const rdcstr &source;
  size_t sourceId;
  lex::LexIter itemIter;
  // Template *tmpl;
  NodeAllocator &alloc;
  std::map<rdcstr, ListNode *> &mRoots;
  ParseError mError;
  template <typename... Args>
  void errorf(SourceRange range, const char *fmt, const Args &... args)
  {
    if(!mError.IsOk())
      return;
    mError.msg.clear();
    mError.msg.sprintf(fmt, args...);
    mError.sourceId = sourceId;
    mError.offset = range.offset;
    mError.size = range.size;
  }

  template <typename... Args>
  inline bool check(bool condition, const char *fmt, const Args &... args)
  {
    if(!condition)
      errorf(itemRange(), fmt, args...);
    return condition;
  }

  void skipSpace();
  Node *textOrAction();
  Node *action();
  ListNode *itemList(Node *&end);
  LiteralNode *number();
  Node *term();
  Node *operand();
  CommandNode *command();
  PipeNode *pipeline();
  void definition();

  BranchNode *parseControl(bool allowElse, bool allowElseIf, BranchNode *node);
  Node *ifControl();
  Node *rangeControl();
  Node *withControl();
  Node *prefixControl();
  Node *elseControl();
  Node *endControl();
  Node *templateControl();
  Node *blockControl();

  inline SourceRange itemRange(const lex::Item &item) { return SourceRange(sourceId, item); }
  inline SourceRange itemRange() { return SourceRange(sourceId, *itemIter); }

public:
  Parser(const rdcstr &sourceName, const rdcstr &source, size_t sourceId,
         std::map<rdcstr, ListNode *> &roots, NodeAllocator &allocator)
      : sourceName(sourceName),
        source(source),
        itemIter(source),
        sourceId(sourceId),
        mRoots(roots),
        alloc(allocator)
  {
  }
  bool Parse(const rdcstr &name);
  inline const ParseError &Error() const { return mError; }
};
}    // namespace parse
}    // namespace templates
}    // namespace cpp_codec

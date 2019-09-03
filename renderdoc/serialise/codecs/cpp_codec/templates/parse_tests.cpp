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

#include "common/globalconfig.h"
#include "exec.h"
#include "parse.h"

#if ENABLED(ENABLE_UNIT_TESTS)

#include "3rdparty/catch/catch.hpp"

using namespace cpp_codec::templates;
using namespace cpp_codec::templates::parse;

void checkNodesEqual(const Node *left, const Node *right, bool checkPos = false);

template <typename ILeft, typename IRight>
void checkNodeRangeEqual(ILeft leftBegin, ILeft leftEnd, IRight rightBegin, IRight rightEnd,
                         bool checkPos = false)
{
  auto iLeft = leftBegin;
  auto iRight = rightBegin;
  for(; iLeft != leftEnd && iRight != rightEnd; ++iLeft, ++iRight)
  {
    checkNodesEqual(*iLeft, *iRight, checkPos);
  }
  CHECK(iLeft == leftEnd);
  CHECK(iRight == rightEnd);
}

template <typename NLeft, typename NRight>
void checkNodeSeqEqual(const NLeft *left, const NRight *right, bool checkPos = false)
{
  CHECK(left->size() == right->size());
  auto iLeft = left->begin();
  auto iRight = right->begin();
  for(; iLeft != left->end() && iRight != right->end(); ++iLeft, ++iRight)
  {
    checkNodesEqual(*iLeft, *iRight, checkPos);
  }
  CHECK(iLeft == left->end());
  CHECK(iRight == right->end());
}

void checkNodesEqual(const Node *left, const Node *right, bool checkPos)
{
  if(left == NULL || right == NULL)
  {
    CHECK(left == NULL);
    CHECK(right == NULL);
    return;
  }

  CHECK(left->type() == right->type());
  if(checkPos)
  {
    CHECK(left->source().sourceId == right->source().sourceId);
    CHECK(left->source().offset == right->source().offset);
    CHECK(left->source().size == right->source().size);
  }
  switch(left->type())
  {
    case NodeType::Literal:
    {
      LiteralNode *litLeft = (LiteralNode *)left;
      LiteralNode *litRight = (LiteralNode *)right;
      CHECK(litLeft->literalType() == litRight->literalType());
      switch(litLeft->literalType())
      {
        case LiteralType::Null:
          // nothing to check
          break;
        case LiteralType::Bool:
          CHECK(((BoolNode *)left)->value() == ((BoolNode *)right)->value());
          break;
        case LiteralType::Int:
          CHECK(((IntNode *)left)->value() == ((IntNode *)right)->value());
          break;
        case LiteralType::UInt:
          CHECK(((UIntNode *)left)->value() == ((UIntNode *)right)->value());
          break;
        case LiteralType::Float:
          CHECK(((FloatNode *)left)->value() == ((FloatNode *)right)->value());
          break;
        case LiteralType::String:
          CHECK(((StringNode *)left)->value() == ((StringNode *)right)->value());
          break;
      }
      break;
    }
    case NodeType::Text:    // Plain text.
    {
      TextNode *textLeft = (TextNode *)left;
      TextNode *textRight = (TextNode *)right;
      CHECK(textLeft->text() == textRight->text());
      break;
    }
    case NodeType::Command:    // An element of a pipeline.
    {
      CommandNode *cmdLeft = (CommandNode *)left;
      CommandNode *cmdRight = (CommandNode *)right;
      checkNodeSeqEqual(cmdLeft, cmdRight);
      break;
    }
    case NodeType::Pipe:    // A pipeline of commands.
    {
      PipeNode *pipeLeft = (PipeNode *)left;
      PipeNode *pipeRight = (PipeNode *)right;
      CHECK(pipeLeft->numDecls() == pipeRight->numDecls());
      auto iLeft = pipeLeft->beginDecls(), iRight = pipeRight->beginDecls();
      for(; iLeft != pipeLeft->endDecls() && iRight != pipeRight->endDecls(); ++iLeft, ++iRight)
      {
        CHECK(*iLeft == *iRight);
      }
      CHECK(pipeLeft->numCmds() == pipeRight->numCmds());
      checkNodeRangeEqual(pipeLeft->beginCmds(), pipeLeft->endCmds(), pipeRight->beginCmds(),
                          pipeRight->endCmds(), checkPos);
      break;
    }
    case NodeType::Action:    // A non-control action such as a field evaluation.
    {
      ActionNode *actionLeft = (ActionNode *)left;
      ActionNode *actionRight = (ActionNode *)right;
      checkNodesEqual(actionLeft->pipe(), actionRight->pipe());
      break;
    }
    case NodeType::Chain:    // A sequence of field accesses.
    {
      ChainNode *chainLeft = (ChainNode *)left;
      ChainNode *chainRight = (ChainNode *)right;
      checkNodesEqual(chainLeft->object(), chainRight->object());
      CHECK(chainLeft->size() == chainRight->size());
      auto iLeft = chainLeft->begin(), iRight = chainRight->begin();
      for(; iLeft != chainLeft->end() && iRight != chainRight->end(); ++iLeft, ++iRight)
      {
        CHECK(*iLeft == *iRight);
      }
      CHECK(iLeft == chainLeft->end());
      CHECK(iRight == chainRight->end());
      break;
    }
    case NodeType::Func:    // An identifier; (dot, function, or variable)
    {
      FuncNode *identLeft = (FuncNode *)left;
      FuncNode *identRight = (FuncNode *)right;
      CHECK(identLeft->name() == identRight->name());
      break;
    }
    case NodeType::Identifier:    // An identifier; (dot, function, or variable)
    {
      IdentifierNode *identLeft = (IdentifierNode *)left;
      IdentifierNode *identRight = (IdentifierNode *)right;
      CHECK(identLeft->identType() == identRight->identType());
      CHECK(identLeft->name() == identRight->name());
      break;
    }
    case NodeType::List:    // A list of NodeTypes.
    {
      ListNode *listLeft = (ListNode *)left;
      ListNode *listRight = (ListNode *)right;
      checkNodeSeqEqual(listLeft, listRight);
      break;
    }
    case NodeType::If:
    case NodeType::Range:
    case NodeType::With:
    case NodeType::Prefix:
    {
      BranchNode *bLeft = (BranchNode *)left;
      BranchNode *bRight = (BranchNode *)right;
      checkNodesEqual(bLeft->pipe(), bRight->pipe());
      checkNodesEqual(bLeft->body(), bRight->body());
      checkNodesEqual(bLeft->alt(), bRight->alt());
      break;
    }
    case NodeType::Else:
    case NodeType::End:
      // nothing to check
      break;
    case NodeType::Template:
    {
      TemplateNode *tLeft = (TemplateNode *)left;
      TemplateNode *tRight = (TemplateNode *)right;
      CHECK(tLeft->name() == tRight->name());
      checkNodesEqual(tLeft->pipe(), tRight->pipe());
      break;
    }
    default: INFO("Unknown node type (" << ToStr(left->type()) << ")"); CHECK(false);
  }
}

void checkParse(const rdcstr &src, std::initializer_list<Node *> expected, bool checkPos = false)
{
  std::map<rdcstr, ListNode *> roots;
  NodeAllocator alloc;
  Parser parser("TEST", src, 0, roots, alloc);
  if(!parser.Parse(""))
  {
    INFO("Parser failed with message: " << parser.Error().msg);
    CHECK(false);
  }
  CHECK(roots.size() == 1);
  ListNode *root = roots[""];
  CHECK(root != NULL);
  checkNodeSeqEqual(root, &expected);
}

class TestNodeAllocator : public NodeAllocator
{
public:
  inline TextNode *newText(const rdcstr &text)
  {
    return NodeAllocator::newText(SourceRange(0, 0, 0), text);
  }
  inline ActionNode *newAction(PipeNode *pipe)
  {
    return NodeAllocator::newAction(SourceRange(0, 0, 0), pipe);
  }
  inline PipeNode *newPipe(const std::initializer_list<CommandNode *> &commands)
  {
    return NodeAllocator::newPipe(SourceRange(0, 0, 0), commands);
  }
  inline PipeNode *newPipe(bool isAssign, const std::initializer_list<rdcstr> &decls,
                           const std::initializer_list<CommandNode *> &commands)
  {
    return NodeAllocator::newPipe(SourceRange(0, 0, 0), isAssign, decls, commands);
  }
  inline CommandNode *newCommand(const std::initializer_list<Node *> &operands)
  {
    return NodeAllocator::newCommand(SourceRange(0, 0, 0), operands);
  }
  inline FuncNode *newFunc(const rdcstr &name)
  {
    return NodeAllocator::newFunc(SourceRange(0, 0, 0), name);
  }
  inline ChainNode *newChain(Node *obj, const std::initializer_list<rdcstr> &operands)
  {
    return NodeAllocator::newChain(SourceRange(0, 0, 0), obj, operands);
  }
  inline IdentifierNode *newDot() { return NodeAllocator::newDot(SourceRange(0, 0, 0)); }
  inline IfNode *newIf(PipeNode *pipe, ListNode *body, ListNode *alt)
  {
    return NodeAllocator::newIf(SourceRange(0, 0, 0), pipe, body, alt);
  }
  inline ListNode *newList(const std::initializer_list<Node *> &children)
  {
    return NodeAllocator::newList(SourceRange(0, 0, 0), children);
  }
  inline RangeNode *newRange(PipeNode *pipe, ListNode *body, ListNode *alt)
  {
    return NodeAllocator::newRange(SourceRange(0, 0, 0), pipe, body, alt);
  }
  inline WithNode *newWith(PipeNode *pipe, ListNode *body, ListNode *alt)
  {
    return NodeAllocator::newWith(SourceRange(0, 0, 0), pipe, body, alt);
  }
  inline TemplateNode *newTemplate(const rdcstr &name, PipeNode *pipe)
  {
    return NodeAllocator::newTemplate(SourceRange(0, 0, 0), name, pipe);
  }
  inline IdentifierNode *newVar(const rdcstr &name)
  {
    return NodeAllocator::newVar(SourceRange(0, 0, 0), name);
  }
  inline NullNode *newNull() { return NodeAllocator::newNull(SourceRange(0, 0, 0)); }
  inline BoolNode *newBool(bool value)
  {
    return NodeAllocator::newBool(SourceRange(0, 0, 0), value);
  }
  inline IntNode *newInt(int64_t value)
  {
    return NodeAllocator::newInt(SourceRange(0, 0, 0), value);
  }
  inline UIntNode *newUInt(uint64_t value)
  {
    return NodeAllocator::newUInt(SourceRange(0, 0, 0), value);
  }
  inline FloatNode *newFloat(double value)
  {
    return NodeAllocator::newFloat(SourceRange(0, 0, 0), value);
  }
  inline StringNode *newString(const rdcstr &value)
  {
    return NodeAllocator::newString(SourceRange(0, 0, 0), value);
  }
  inline CharNode *newChar(char value)
  {
    return NodeAllocator::newChar(SourceRange(0, 0, 0), value);
  }
};

TEST_CASE("Test CPP Codec Template Language Parser", "[templates]")
{
  TestNodeAllocator a;
  SECTION("empty") { checkParse("", {}); }
  SECTION("text") { checkParse("abc def", {a.newText("abc def")}); }
  SECTION("function")
  {
    checkParse("abc {{foo}} def",
               {a.newText("abc "), a.newAction(a.newPipe({a.newCommand({a.newFunc("foo")})})),
                a.newText(" def")});
  }
  SECTION("chain")
  {
    checkParse(
        "abc {{foo.bar.baz}} def",
        {a.newText("abc "),
         a.newAction(a.newPipe({a.newCommand({a.newChain(a.newFunc("foo"), {"bar", "baz"})})})),
         a.newText(" def")});
  }
  SECTION("dot")
  {
    checkParse("abc {{.}} def",
               {a.newText("abc "), a.newAction(a.newPipe({a.newCommand({a.newDot()})})),
                a.newText(" def")});
  }
  SECTION("dot chain")
  {
    checkParse(
        "abc {{.foo.bar.baz}} def",
        {a.newText("abc "),
         a.newAction(a.newPipe({a.newCommand({a.newChain(a.newDot(), {"foo", "bar", "baz"})})})),
         a.newText(" def")});
  }
  SECTION("arguments")
  {
    checkParse("abc {{foo bar baz}} def",
               {a.newText("abc "),
                a.newAction(a.newPipe(
                    {a.newCommand({a.newFunc("foo"), a.newFunc("bar"), a.newFunc("baz")})})),
                a.newText(" def")});
  }
  SECTION("dot argument")
  {
    checkParse("abc {{foo bar . baz}} def",
               {a.newText("abc "),
                a.newAction(a.newPipe({a.newCommand(
                    {a.newFunc("foo"), a.newFunc("bar"), a.newDot(), a.newFunc("baz")})})),
                a.newText(" def")});
  }
  SECTION("pipe")
  {
    checkParse(
        "abc {{foo | bar | baz}} def",
        {a.newText("abc "),
         a.newAction(a.newPipe({a.newCommand({a.newFunc("foo")}), a.newCommand({a.newFunc("bar")}),
                                a.newCommand({a.newFunc("baz")})})),
         a.newText(" def")});
  }
  SECTION("pipe with args")
  {
    checkParse(
        "abc {{foo a b | bar x y | baz z}} def",
        {a.newText("abc "),
         a.newAction(a.newPipe({a.newCommand({a.newFunc("foo"), a.newFunc("a"), a.newFunc("b")}),
                                a.newCommand({a.newFunc("bar"), a.newFunc("x"), a.newFunc("y")}),
                                a.newCommand({a.newFunc("baz"), a.newFunc("z")})})),
         a.newText(" def")});
  }
  SECTION("parens")
  {
    checkParse("abc {{(foo fuu) (bar baz)}} def",
               {a.newText("abc "),
                a.newAction(a.newPipe({a.newCommand(
                    {a.newPipe({a.newCommand({a.newFunc("foo"), a.newFunc("fuu")})}),
                     a.newPipe({a.newCommand({a.newFunc("bar"), a.newFunc("baz")})})})})),
                a.newText(" def")});
  }
  SECTION("parens with pipes")
  {
    checkParse(
        "abc {{(foo | fuu) | (bar | baz)}} def",
        {a.newText("abc "),
         a.newAction(a.newPipe({a.newCommand({a.newPipe({a.newCommand({a.newFunc("foo")}),
                                                         a.newCommand({a.newFunc("fuu")})})}),
                                a.newCommand({a.newPipe({a.newCommand({a.newFunc("bar")}),
                                                         a.newCommand({a.newFunc("baz")})})})})),
         a.newText(" def")});
  }
  SECTION("if")
  {
    checkParse("abc {{if foo}} def {{end}} ghi",
               {a.newText("abc "),
                a.newIf(
                    /* condition */ a.newPipe({a.newCommand({a.newFunc("foo")})}),
                    /* true branch */ a.newList({a.newText(" def ")}),
                    /* false branch */ a.newList({})),
                a.newText(" ghi")});
  }
  SECTION("if else")
  {
    checkParse("abc {{if foo}} def {{else}} xyz {{end}} ghi",
               {a.newText("abc "),
                a.newIf(/* condition */ a.newPipe({a.newCommand({a.newFunc("foo")})}),
                        /* true branch */ a.newList({a.newText(" def ")}),
                        /* false branch */ a.newList({a.newText(" xyz ")})),
                a.newText(" ghi")});
  }
  SECTION("else if")
  {
    checkParse(
        "abc {{if foo}} def {{else if bar}} stu {{else}} xyz {{end}} ghi",
        {a.newText("abc "),
         a.newIf(/* condition */ a.newPipe({a.newCommand({a.newFunc("foo")})}),
                 /* true branch */ a.newList({a.newText(" def ")}),
                 /* false branch */
                 a.newList({a.newIf(/* condition */ a.newPipe({a.newCommand({a.newFunc("bar")})}),
                                    /* true branch */ a.newList({a.newText(" stu ")}),
                                    /* false branch */ a.newList({a.newText(" xyz ")}))})),
         a.newText(" ghi")});
  }
  SECTION("range")
  {
    checkParse("abc {{range foo}} def {{end}} ghi",
               {a.newText("abc "),
                a.newRange(/* collection */ a.newPipe({a.newCommand({a.newFunc("foo")})}),
                           /* body */ a.newList({a.newText(" def ")}),
                           /* else */ a.newList({})),
                a.newText(" ghi")});
  }
  SECTION("range else")
  {
    checkParse("abc {{range foo}} def {{else}} xyz {{end}} ghi",
               {a.newText("abc "),
                a.newRange(/* collection */ a.newPipe({a.newCommand({a.newFunc("foo")})}),
                           /* body */ a.newList({a.newText(" def ")}),
                           /* else */ a.newList({a.newText(" xyz ")})),
                a.newText(" ghi")});
  }
  SECTION("range var")
  {
    checkParse(
        "abc {{range $x := foo}} def {{end}} ghi",
        {a.newText("abc "),
         a.newRange(/* collection */ a.newPipe(false, {"x"}, {a.newCommand({a.newFunc("foo")})}),
                    /* body */ a.newList({a.newText(" def ")}),
                    /* else */ a.newList({})),
         a.newText(" ghi")});
  }
  SECTION("range index var")
  {
    checkParse(
        "abc {{range $i, $x := foo}} def {{end}} ghi",
        {a.newText("abc "),
         a.newRange(/* collection */ a.newPipe(false, {"i", "x"}, {a.newCommand({a.newFunc("foo")})}),
                    /* body */ a.newList({a.newText(" def ")}),
                    /* else */ a.newList({})),
         a.newText(" ghi")});
  }
  SECTION("with")
  {
    checkParse("abc {{with foo}} def {{end}} ghi",
               {a.newText("abc "),
                a.newWith(/* value */ a.newPipe({a.newCommand({a.newFunc("foo")})}),
                          /* body */ a.newList({a.newText(" def ")}),
                          /* else */ a.newList({})),
                a.newText(" ghi")});
  }
  SECTION("with else")
  {
    checkParse("abc {{with foo}} def {{else}} xyz {{end}} ghi",
               {a.newText("abc "),
                a.newWith(/* value */ a.newPipe({a.newCommand({a.newFunc("foo")})}),
                          /* body */ a.newList({a.newText(" def ")}),
                          /* else */ a.newList({a.newText(" xyz ")})),
                a.newText(" ghi")});
  }
  SECTION("template")
  {
    checkParse(
        R"(abc {{template "foo" bar}} def)",
        {a.newText("abc "), a.newTemplate("foo", a.newPipe({a.newCommand({a.newFunc("bar")})})),
         a.newText(" def")});
  }
  SECTION("read variable")
  {
    checkParse("abc {{$x}} def",
               {a.newText("abc "), a.newAction(a.newPipe({a.newCommand({a.newVar("x")})})),
                a.newText(" def")});
  }
  SECTION("declare variable")
  {
    checkParse("abc {{$x := foo}} def",
               {a.newText("abc "),
                a.newAction(a.newPipe(false, {"x"}, {a.newCommand({a.newFunc("foo")})})),
                a.newText(" def")});
  }
  SECTION("assign variable")
  {
    checkParse(
        "abc {{$x = foo}} def",
        {a.newText("abc "), a.newAction(a.newPipe(true, {"x"}, {a.newCommand({a.newFunc("foo")})})),
         a.newText(" def")});
  }
  SECTION("const null")
  {
    checkParse("{{null}}", {a.newAction(a.newPipe({a.newCommand({a.newNull()})}))});
  }
  SECTION("const bool")
  {
    checkParse("{{true}}{{false}}", {
                                        a.newAction(a.newPipe({a.newCommand({a.newBool(true)})})),
                                        a.newAction(a.newPipe({a.newCommand({a.newBool(false)})})),
                                    });
  }
  SECTION("const int")
  {
    checkParse("{{0}}{{1}}{{-3}}{{4294967295}}{{9223372036854775807}}{{-9223372036854775808}}",
               {
                   a.newAction(a.newPipe({a.newCommand({a.newInt(0)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newInt(1)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newInt(-3)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newInt(4294967295)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newInt(9223372036854775807LL)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newInt(-9223372036854775807LL - 1)})})),
               });
  }
  SECTION("const uint")
  {
    checkParse(
        "{{0u}}{{1u}}{{3u}}{{4294967295u}}{{18446744073709551615u}}"
        "{{0U}}{{1U}}{{3U}}{{4294967295U}}{{18446744073709551615U}}",
        {
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0u)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(1u)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(3u)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(4294967295u)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(18446744073709551615ULL)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0u)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(1u)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(3u)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(4294967295u)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(18446744073709551615ULL)})})),
        });
  }
  SECTION("const uint hex")
  {
    checkParse(
        "{{0x0}}{{0x1}}{{0xffffffff}}{{0xffffffffffffffff}}"
        "{{0X0}}{{0X1}}{{0XFFFFFFFF}}{{0XFFFFFFFFFFFFFFFF}}",
        {
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0x0)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0x1)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0xffffffff)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0xffffffffffffffffULL)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0x0)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0x1)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0xffffffff)})})),
            a.newAction(a.newPipe({a.newCommand({a.newUInt(0xffffffffffffffffULL)})})),
        });
  }
  SECTION("const float")
  {
    checkParse("{{0.0}}{{1.1}}{{.2}}{{3.}}{{4.5e-6}}{{7.e8}}",
               {
                   a.newAction(a.newPipe({a.newCommand({a.newFloat(0.0)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newFloat(1.1)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newFloat(.2)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newFloat(3.)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newFloat(4.5e-6)})})),
                   a.newAction(a.newPipe({a.newCommand({a.newFloat(7.e8)})})),
               });
  }
  SECTION("const string")
  {
    checkParse(R"({{""}}{{"abc"}}{{"a\n\t b"}})",
               {
                   a.newAction(a.newPipe({a.newCommand({a.newString("")})})),
                   a.newAction(a.newPipe({a.newCommand({a.newString("abc")})})),
                   a.newAction(a.newPipe({a.newCommand({a.newString("a\n\t b")})})),
               });
  }
  SECTION("const raw string")
  {
    checkParse(R"({{``}}{{`abc`}}{{`a\n\t b`}}{{`abc
def`}})",
               {
                   a.newAction(a.newPipe({a.newCommand({a.newString("")})})),
                   a.newAction(a.newPipe({a.newCommand({a.newString("abc")})})),
                   a.newAction(a.newPipe({a.newCommand({a.newString("a\\n\\t b")})})),
                   a.newAction(a.newPipe({a.newCommand({a.newString("abc\ndef")})})),
               });
  }
  SECTION("const char")
  {
    checkParse(R"({{'a'}}{{'\n'}}{{'\0'}}{{'\x22'}})",
               {
                   a.newAction(a.newPipe({a.newCommand({a.newChar('a')})})),
                   a.newAction(a.newPipe({a.newCommand({a.newChar('\n')})})),
                   a.newAction(a.newPipe({a.newCommand({a.newChar('\0')})})),
                   a.newAction(a.newPipe({a.newCommand({a.newChar('\x22')})})),
               });
  }
  // SECTION("const uint") {}
  // SECTION("const float") {}
  // SECTION("const string") {}
  //  SECTION("prefix")
  //  {
  //    checkParse(
  //        R"(
  // abc
  //{{prefix .val}}def
  // ghi
  //{{end}}
  // jkl
  //)",
  //        {a.newText("\nabc\n"),
  //         a.newPrefix(a.newPipe({a.newCommand({a.newChain(a.newDot(), {"val"})})}),
  //                     a.newList({a.newText("def\nghi\n")})),
  //         a.newText("\njkl\n")});
  //  }
}

#endif    // ENABLED(ENABLE_UNIT_TESTS)

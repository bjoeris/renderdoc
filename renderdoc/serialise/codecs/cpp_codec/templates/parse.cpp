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

#include "parse.h"
#include "exec.h"

namespace cpp_codec
{
namespace templates
{
namespace parse
{
using namespace lex;
void Parser::skipSpace()
{
  while(itemIter->type == ItemType::Space)
    ++itemIter;
}
Node *Parser::textOrAction()
{
  Node *node = NULL;
  if(itemIter->type == ItemType::Text)
  {
    node = alloc.newText(itemRange(), itemIter->val);
    ++itemIter;
  }
  // return newNode<TextNode, const rdcstr &>(itemIter->pos, itemIter->val);
  else if(itemIter->type == ItemType::LeftDelim)
  {
    node = action();
  }
  else
  {
    errorf(itemRange(), "Unexpected %s", ToStr(itemIter->type));
  }
  return node;
}
BranchNode *Parser::parseControl(bool allowElse, bool allowElseIf, BranchNode *node)
{
  ++itemIter;    // consume control keyword

  PipeNode *pipe = pipeline();
  Node *next = NULL;
  ListNode *body = itemList(next);
  if(next == NULL || body == NULL)
    return NULL;
  ListNode *alt = NULL;
  switch(next->type())
  {
    case NodeType::End:
    {
      if(allowElse)
      {
        alt = alloc.newList(SourceRange(sourceId, next->mSource.offset, 0));
        if(alt == NULL)
          return NULL;
      }
      break;
    }
    case NodeType::Else:
    {
      if(!allowElse)
      {
        errorf(next->mSource, "Unexpected `{{else}}`");
        return NULL;
      }
      if(allowElseIf && itemIter->type == ItemType::If)
      {
        Node *elseIf = ifControl();
        if(elseIf == NULL)
          return NULL;
        SourceRange range = next->mSource;
        range.size = elseIf->mSource.offset + elseIf->mSource.size - range.offset;
        alt = alloc.newList(range, {elseIf});
        if(alt == NULL)
          return NULL;
        // call to `ifControl` will have already checked and consumed `{{end}}`
      }
      else
      {
        alt = itemList(next);
        if(alt == NULL)
          return NULL;
        if(next->type() != NodeType::End)
        {
          errorf(next->mSource, "Expected `{{end}}`; found %s", ToStr(next->type()));
          return NULL;
        }
      }
      break;
    }
    default:
      errorf(next->mSource, "Expected `{{end}}` or `{{else}}`, found %s", ToStr(next->type()));
      return NULL;
  }
  node->mPipe = pipe;
  node->mBody = body;
  node->mAlt = alt;
  node->mSource.size = itemIter->offset - node->mSource.offset;
  return node;
}
Node *Parser::ifControl()
{
  IfNode *node = alloc.newIf(itemRange(), NULL, NULL, NULL);
  return parseControl(true, true, node);
}
Node *Parser::rangeControl()
{
  RangeNode *node = alloc.newRange(itemRange(), NULL, NULL, NULL);
  return parseControl(true, false, node);
}
Node *Parser::withControl()
{
  WithNode *node = alloc.newWith(itemRange(), NULL, NULL, NULL);
  return parseControl(true, false, node);
}
Node *Parser::prefixControl()
{
  PrefixNode *node = alloc.newPrefix(itemRange(), NULL, NULL);
  if(parseControl(false, false, node) == NULL)
    return NULL;
  return node;
}
Node *Parser::elseControl()
{
  SourceRange range = itemRange();
  ++itemIter;    // consume `else`
  skipSpace();
  if(itemIter->type != ItemType::If)
  {
    if(!check(itemIter->type == ItemType::RightDelim, "Expected right delim, found %s",
              ToStr(itemIter->type)))
      return NULL;
    ++itemIter;    // consume `}}`
  }
  range.size = itemIter->offset - range.offset;
  return alloc.newElse(range);
}
Node *Parser::endControl()
{
  SourceRange range = itemRange();
  ++itemIter;    // consume `end`
  skipSpace();
  if(!check(itemIter->type == ItemType::RightDelim, "Expected right delim, found %s",
            ToStr(itemIter->type)))
    return NULL;
  ++itemIter;    // consume `}}`
  range.size = itemIter->offset - range.offset;
  return alloc.newEnd(range);
}
bool unquote(const rdcstr &quoted, rdcstr &unquoted)
{
  unquoted = quoted.substr(1, quoted.size() - 2);
  if(quoted[0] == '"' || quoted[0] == '\'')
  {
    rdcstr temp;
    if(!unescape(unquoted, temp))
      return false;
    std::swap(unquoted, temp);
    if(quoted[0] == '\'' && unquoted.size() != 1)
      return false;
  }
  return true;
}
Node *Parser::templateControl()
{
  SourceRange range = itemRange();
  ++itemIter;
  skipSpace();
  rdcstr name;
  switch(itemIter->type)
  {
    case ItemType::String:
    case ItemType::RawString:
    {
      if(!unquote(itemIter->val, name))
      {
        errorf(itemRange(), "Invalid quoted template name: %s", itemIter->val);
        return NULL;
      }
      break;
    }
    default: errorf(itemRange(), "Unexpected %s", ToStr(itemIter->type)); return NULL;
  }
  ++itemIter;
  skipSpace();
  PipeNode *pipe = NULL;
  if(itemIter->type == ItemType::RightDelim)
  {
    ++itemIter;
    pipe = alloc.newPipe(range, {alloc.newCommand(range, {alloc.newDot(range)})});
  }
  else
    pipe = pipeline();
  range.size = itemIter->offset - range.offset;
  return alloc.newTemplate(range, name, pipe);
}
Node *Parser::blockControl()
{
  errorf(itemRange(), "blockControl not implemented");
  return NULL;
}
Node *Parser::action()
{
  SourceRange range = itemRange();
  ++itemIter;
  skipSpace();
  Node *node = NULL;
  switch(itemIter->type)
  {
    case ItemType::Block: return blockControl();
    case ItemType::Else: return elseControl();
    case ItemType::End: return endControl();
    case ItemType::If: return ifControl();
    case ItemType::Range: return rangeControl();
    case ItemType::Template: return templateControl();
    case ItemType::With: return withControl();
    case ItemType::Prefix: return prefixControl();
    default:
    {
      PipeNode *pipe = pipeline();
      if(pipe == NULL)
        return NULL;
      range.size = pipe->mSource.offset + pipe->mSource.size - range.offset;
      node = alloc.newAction(range, pipe);
    }
  }

  return node;
}
ListNode *Parser::itemList(Node *&end)
{
  ListNode *list = alloc.newList(itemRange());
  while(true)
  {
    if(itemIter->type == ItemType::EndOfFile)
    {
      errorf(itemRange(), "Unexpected eof");
      return NULL;
    }
    Node *next = textOrAction();
    if(next == NULL)
      return NULL;
    if(next->type() == NodeType::End || next->type() == NodeType::Else)
    {
      end = next;
      list->mSource.size = itemIter->offset - list->mSource.offset;
      return list;
    }
    list->mChildren.push_back(next);
  }
}
LiteralNode *Parser::number()
{
  SourceRange range = itemRange();
  rdcstr str = itemIter->val;
  ++itemIter;
  if(!check(str.size() > 0, "found zero size numeric constant"))
    return NULL;

  const char *front = str.c_str();
  const char *back = str.c_str() + str.size() - 1;

  bool isHex = false;
  bool isUnsigned = false;
  bool isFloat = false;

  if(*front == '0' && (front[1] == 'x' || front[1] == 'X'))
  {
    // skip leading "0x"
    front += 2;
    isHex = true;
    isUnsigned = true;
  }
  else if(*front == '+')
  {
    // skip leading '+'
    ++front;
  }

  if(*back == 'u' || *back == 'U')
  {
    --back;
    isUnsigned = true;
  }

  if(!check(front <= back, "malformed numeric constant: %s", str))
    return NULL;

  for(const char *i = front; i <= back; ++i)
  {
    if(*i == '.' || *i == 'e' || *i == 'E')
    {
      isFloat = true;
      break;
    }
  }

  if(!check(!(isUnsigned && isFloat),
            "floating point constant cannot include the unsigned suffix: %s", str))
    return NULL;

  std::istringstream istr(std::string(front, back + 1));
  LiteralNode *node = NULL;

  if(isUnsigned)
  {
    uint64_t value;
    if(isHex)
      istr >> std::hex >> value;
    else
      istr >> value;
    node = alloc.newUInt(range, value);
  }
  else if(isFloat)
  {
    double value;
    if(isHex)
      istr >> std::hex >> value;
    else
      istr >> value;
    node = alloc.newFloat(range, value);
  }
  else
  {
    int64_t value;
    if(isHex)
      istr >> std::hex >> value;
    else
      istr >> value;
    node = alloc.newInt(range, value);
  }
  if(!check(!istr.fail(), "failed to parse numeric constant: '%s'", str) ||
     !check(istr.eof(), "unexpected trailing characters in numeric constant: '%s'", str))
  {
    return NULL;
  }
  node->mSource.size = itemIter->offset - range.offset;
  return node;
}

rdcstr escape(const rdcstr &str, bool isChar)
{
  std::ostringstream ostr;
  for(auto it = str.begin(); it != str.end(); ++it)
  {
    char c = *it;
    switch(c)
    {
      case '\'':
        if(isChar)
          ostr << "\\'";
        else
          ostr << '\'';
        break;
      case '"':
        if(isChar)
          ostr << '"';
        else
          ostr << "\\\"";
        break;
      case '\\': ostr << "\\\\"; break;
      case '\a': ostr << "\\a"; break;
      case '\b': ostr << "\\b"; break;
      case '\f': ostr << "\\f"; break;
      case '\n': ostr << "\\n"; break;
      case '\r': ostr << "\\r"; break;
      case '\t': ostr << "\\t"; break;
      case '\v': ostr << "\\v"; break;
      default:
        if(std::isspace(c) || std::isprint(c))
          ostr << c;
        else
          ostr << "\\x" << std::hex << (int)c;
    }
  }
  return ostr.str();
}

bool unescape(const rdcstr &str, rdcstr &out)
{
  out.reserve(str.size());
  for(auto it = str.begin(); it != str.end(); ++it)
  {
    if(*it == '\\')
    {
      ++it;
      if(it == str.end())
        return false;
      switch(*it)
      {
        case '\'': out.push_back('\''); break;
        case '\\': out.push_back('\\'); break;
        case '"': out.push_back('\"'); break;
        case '?': out.push_back('\?'); break;
        case 'a': out.push_back('\a'); break;
        case 'b': out.push_back('\b'); break;
        case 'f': out.push_back('\f'); break;
        case 'n': out.push_back('\n'); break;
        case 'r': out.push_back('\r'); break;
        case 't': out.push_back('\t'); break;
        case 'v': out.push_back('\v'); break;
        case 'x':
        {
          ++it;
          int j = 0;
          for(; it + j != str.end() && isxdigit(it[j]); ++j)
          {
          }
          if(j == 0)
            return false;    // "\x" not followed by a hex digit
          std::istringstream istr(std::string(it, it + j));
          int val;
          istr >> std::hex >> val;
          out.push_back((char)val);
          it = it + j - 1;
          break;
        }
        case 'u':
        case 'U': return false;    // unicode character codes are not supported
        default:
          if('0' <= *it && *it <= '7')
          {
            int j = 1;
            for(; it + j != str.end() && '0' <= it[j] && it[j] <= '7'; ++j)
            {
            }
            std::istringstream istr(std::string(it, it + j));
            int val;
            istr >> std::oct >> val;
            out.push_back((char)val);
            it = it + j - 1;
            break;
          }
          else
            return false;    // unknown escape sequence
      }
    }
    else
    {
      out.push_back(*it);
    }
  }
  return true;
}

Node *Parser::term()
{
  skipSpace();
  Node *node = NULL;
  switch(itemIter->type)
  {
    case ItemType::Identifier:
      node = alloc.newFunc(itemRange(), itemIter->val);
      ++itemIter;
      break;
    case ItemType::Variable:
      node = alloc.newVar(itemRange(), itemIter->val.substr(1));
      ++itemIter;
      break;
    case ItemType::Field: node = alloc.newDot(itemRange()); break;
    case ItemType::Dot:
      node = alloc.newDot(itemRange());
      ++itemIter;
      break;
    case ItemType::Null:
      node = alloc.newNull(itemRange());
      ++itemIter;
      break;
    case ItemType::Bool:
      node = alloc.newBool(itemRange(), itemIter->val == "true");
      ++itemIter;
      break;
    case ItemType::String:
    {
      rdcstr u;
      if(!check(unescape(itemIter->val.substr(1, itemIter->val.size() - 2), u),
                "invalid string literal: %s", itemIter->val))
        return NULL;
      node = alloc.newString(itemRange(), u);
      ++itemIter;
      break;
    }
    case ItemType::RawString:
      node = alloc.newString(itemRange(), itemIter->val.substr(1, itemIter->val.size() - 2));
      ++itemIter;
      break;
    case ItemType::Char:
    {
      rdcstr u;
      if(!check(unescape(itemIter->val.substr(1, itemIter->val.size() - 2), u) || u.size() != 1,
                "invalid char literal: %s", itemIter->val))
        return NULL;
      node = alloc.newChar(itemRange(), u[0]);
      ++itemIter;
      break;
    }
    case ItemType::Number: node = number(); break;
    case ItemType::LeftParen:
    {
      ++itemIter;
      node = pipeline();
      if(itemIter->type != ItemType::RightParen)
      {
        errorf(itemRange(), "Expected ')', found '%s' (%s)", itemIter->val, ToStr(itemIter->type));
        return NULL;
      }
      ++itemIter;
      break;
    }
    default: break;    // Do nothing
  }
  if(node)
    node->mSource.size = itemIter->offset - node->mSource.offset;
  return node;
}
Node *Parser::operand()
{
  Node *node = term();
  if(node == NULL)
    return NULL;
  if(itemIter->type == ItemType::Field)
  {
    ChainNode *chain = alloc.newChain(node->mSource, node);
    node = chain;
    do
    {
      chain->mFields.push_back(itemIter->val.substr(1));
      ++itemIter;
    } while(itemIter->type == ItemType::Field);
    chain->mSource.size = itemIter->offset - chain->mSource.offset;
  }
  return node;
}
CommandNode *Parser::command()
{
  CommandNode *cmd = alloc.newCommand(itemRange());
  while(true)
  {
    skipSpace();
    Node *op = operand();
    if(op != NULL)
      cmd->mOperands.push_back(op);
    switch(itemIter->type)
    {
      case ItemType::Space: break;
      case ItemType::Error: errorf(itemRange(), "%s", itemIter->val); return NULL;
      case ItemType::Pipe: ++itemIter;
      // fallthrough
      case ItemType::RightParen:
      case ItemType::RightDelim:
        if(cmd->mOperands.size() == 0)
        {
          errorf(itemRange(), "empty command");
          return NULL;
        }
        return cmd;
      default: errorf(itemRange(), "unexpected %s in operand", ToStr(itemIter->type)); return NULL;
    }
  }
}

PipeNode *Parser::pipeline()
{
  PipeNode *pipe = alloc.newPipe(itemRange());
  while(true)
  {
    skipSpace();
    if(itemIter->type == ItemType::Variable)
    {
      // could be a declaration (e.g. "$x := foo") or function application of the variable (e.g.
      // "$x foo").
      LexIter savedIter = itemIter;
      Item varItem = *itemIter;
      ++itemIter;
      skipSpace();
      ItemType t = itemIter->type;
      if(t == ItemType::Assign || t == ItemType::Declare || t == ItemType::Comma)
      {
        // This is definitely a declaration
        pipe->mDeclarations.push_back(varItem.val.substr(1));
        ++itemIter;
        if(t == ItemType::Assign || t == ItemType::Declare)
        {
          pipe->mIsAssign = t == ItemType::Assign;
          // end of declarations
          break;
        }
      }
      else
      {
        // This is definitely not a declaration, possibly a function application (e.g.
        // "$x foo").
        if(pipe->numDecls() > 0)
        {
          // We saw something like "$x, $y foo", which is an error
          errorf(itemRange(), "Expected `,` or `=` or `:=`, but found `%s`", ToStr(itemIter->type));
          return NULL;
        }
        itemIter = savedIter;
        break;
      }
    }
    else if(pipe->numDecls() > 0)
    {
      // We saw something like "$x, foo", which is an error
      errorf(itemRange(), "Expected variable, %s", ToStr(itemIter->type));
      return NULL;
    }
    else
    {
      // no declarations.
      break;
    }
  }
  while(true)
  {
    skipSpace();
    switch(itemIter->type)
    {
      case ItemType::RightDelim: ++itemIter;
      case ItemType::RightParen:
        pipe->mSource.size = itemIter->offset - pipe->mSource.offset;
        return pipe;
      case ItemType::Dot:
      case ItemType::Identifier:
      case ItemType::Variable:
      case ItemType::Field:
      case ItemType::Null:
      case ItemType::Bool:
      case ItemType::String:
      case ItemType::RawString:
      case ItemType::Char:
      case ItemType::Number:
      case ItemType::LeftParen:
      {
        CommandNode *cmd = command();
        if(cmd == NULL)
          return NULL;
        pipe->mCommands.push_back(cmd);
        break;
      }
      default: errorf(itemRange(), "unexpected %s in pipe", ToStr(itemIter->type)); return NULL;
    }
  }
}

void Parser::definition()
{
  ++itemIter;
  skipSpace();
  if(!check(itemIter->type == ItemType::String || itemIter->type == ItemType::RawString,
            "Expected quoted definition name, found %s", ToStr(itemIter->type)))
    return;
  rdcstr name;
  if(!check(unquote(itemIter->val, name), "Invalid quoted definition name: %s", itemIter->val))
    return;
  ++itemIter;
  skipSpace();
  if(!check(itemIter->type == ItemType::RightDelim, "Expected `{{`, found %s", ToStr(itemIter->type)))
    return;
  ++itemIter;
  Node *end = NULL;
  ListNode *root = itemList(end);
  if(root == NULL || end == NULL)
    return;
  if(!check(end->type() == NodeType::End, "Unexpected %s", ToStr(end->type())))
    return;
  mRoots[name] = root;
}

bool Parser::Parse(const rdcstr &name)
{
  ListNode *root = alloc.newList(itemRange());
  while(itemIter->type != ItemType::EndOfFile)
  {
    if(itemIter->type == ItemType::LeftDelim)
    {
      LexIter savedIter(itemIter);
      ++itemIter;
      skipSpace();
      if(itemIter->type == ItemType::Define)
      {
        definition();
        continue;
      }
      else
      {
        itemIter = savedIter;
      }
    }
    Node *n = textOrAction();
    if(n == NULL)
      return false;
    if(n->type() == NodeType::End || n->type() == NodeType::Else)
    {
      errorf(n->mSource, "Unexpected node type %s", ToStr(n->type()));
      return false;
    }
    root->mChildren.push_back(n);
  }
  root->mSource.size = itemIter->offset - root->mSource.offset;
  mRoots[name] = root;
  return true;
}

NodeAllocator::~NodeAllocator()
{
  for(auto it = nodes.begin(); it != nodes.end(); ++it)
    delete(*it);
}

int BeginningOfLine(const rdcstr &source, int offset)
{
  if(offset >= source.size())
    return -1;
  do
  {
    --offset;
  } while(offset >= 0 && source[offset] != '\n');
  return offset + 1;
}

int EndOfLine(const rdcstr &source, int offset)
{
  if(offset >= source.size())
    return -1;
  while(offset < source.size() && source[offset] != '\n')
    ++offset;
  return offset;
}

int LineNumber(const rdcstr &source, int offset)
{
  if(offset < 0)
    return 0;
  offset = RDCMIN(offset, (int)source.size());
  const char *begin = source.begin();
  const char *end = begin + offset;
  int line = 1;
  for(const char *i = begin; i != end; ++i)
  {
    if(*i == '\n')
      ++line;
  }
  return line;
}

bool PrintSourceRange(Writer &w, const rdcstr &source, size_t offset, size_t size, size_t context,
                      bool printColumnIndicator)
{
  size_t lastOffset = offset + size - 1;
  if(lastOffset >= source.size())
    return false;

  // line number containing `offset`
  int beginLineNo = LineNumber(source, (int)offset);

  // line number following the line containing `offset+size-1`
  int endLineNo = LineNumber(source, (int)(offset + size - 1)) + 1;

  // offset of the first non-newline character in the line
  int lineBegin = BeginningOfLine(source, (int)offset);

  // offset of the first non-newline character in the next line
  int lineEnd = EndOfLine(source, (int)offset) + 1;

  // current line number
  int lineNo = beginLineNo;

  // go back `context` lines
  for(int i = 0; i < context && lineBegin > 0; ++i, --lineNo)
  {
    lineEnd = lineBegin;
    lineBegin = BeginningOfLine(source, lineBegin - 1);
  }
  lineBegin = RDCMAX(lineBegin, 0);

  rdcstr indicator;
  bool ellipsesPrinted = false;
  while(lineNo < endLineNo + context && lineBegin < source.size())
  {
    if(beginLineNo + context < lineNo && lineNo < endLineNo - context - 1)
    {
      if(!ellipsesPrinted)
      {
        w.Write("...\n");
        ellipsesPrinted = true;
      }
    }
    else
    {
      char lineIndicator = ' ';
      if(lineNo == beginLineNo || lineNo == endLineNo - 1)
        lineIndicator = '>';
      else if(beginLineNo < lineNo && lineNo < endLineNo - 1)
        lineIndicator = '|';

      w.Printf("%5d: %c %s", lineNo, lineIndicator, source.substr(lineBegin, lineEnd - lineBegin));

      if(lineBegin <= offset && offset < lineEnd)
      {
        indicator.reserve(lineEnd - lineBegin);
        int i;
        for(i = lineBegin; i < (int)offset; ++i)
          indicator.push_back(' ');
        indicator.push_back('^');
        for(i = (int)offset + 1; i < lineEnd && i < (int)lastOffset; ++i)
          indicator.push_back('-');
        if(i == lastOffset)
          indicator.push_back('^');
      }
      else if(lineBegin <= lastOffset && lastOffset < lineEnd)
      {
        indicator.reserve(lineEnd - lineBegin);
        for(int i = lineBegin; i < lastOffset; ++i)
          indicator.push_back('-');
        indicator.push_back('^');
      }
      if(!indicator.empty())
      {
        if(printColumnIndicator)
          w.Printf("       %s\n", indicator);
        indicator.clear();
      }
    }
    ++lineNo;
    lineBegin = lineEnd;
    lineEnd = EndOfLine(source, lineBegin) + 1;
  }
  return true;
}
}    // namespace parse
}    // namespace templates
}    // namespace cpp_codec

template <>
rdcstr DoStringise(const cpp_codec::templates::parse::NodeType &el)
{
  BEGIN_ENUM_STRINGISE(cpp_codec::templates::parse::NodeType);
  {
    STRINGISE_ENUM_CLASS(Literal);
    STRINGISE_ENUM_CLASS(Text);
    STRINGISE_ENUM_CLASS(Command);
    STRINGISE_ENUM_CLASS(Pipe);
    STRINGISE_ENUM_CLASS(Action);
    STRINGISE_ENUM_CLASS(Chain);
    // STRINGISE_ENUM_CLASS(Identifier);
    STRINGISE_ENUM_CLASS(Func);
    STRINGISE_ENUM_CLASS(List);
    STRINGISE_ENUM_CLASS(If);
    STRINGISE_ENUM_CLASS(Range);
    STRINGISE_ENUM_CLASS(Else);
    STRINGISE_ENUM_CLASS(End);
    STRINGISE_ENUM_CLASS(Template);
    STRINGISE_ENUM_CLASS(With);
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const cpp_codec::templates::parse::IdentType &el)
{
  BEGIN_ENUM_STRINGISE(cpp_codec::templates::parse::IdentType);
  {
    STRINGISE_ENUM_CLASS(Dot);
    STRINGISE_ENUM_CLASS(Var);
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const cpp_codec::templates::parse::LiteralType &el)
{
  BEGIN_ENUM_STRINGISE(cpp_codec::templates::parse::LiteralType);
  {
    STRINGISE_ENUM_CLASS(Null);
    STRINGISE_ENUM_CLASS(Bool);
    STRINGISE_ENUM_CLASS(Int);
    STRINGISE_ENUM_CLASS(UInt);
    STRINGISE_ENUM_CLASS(Float);
    STRINGISE_ENUM_CLASS(String);
    STRINGISE_ENUM_CLASS(Char);
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const cpp_codec::templates::parse::Node &el)
{
  return ToStr(el.type());
}

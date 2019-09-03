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


#include "lex.h"

namespace cpp_codec
{
namespace templates
{
namespace lex
{
const rdcstr LexIter::LEFT_DELIM = "{{";
const rdcstr LexIter::RIGHT_DELIM = "}}";
const rdcstr LexIter::LEFT_TRIM_MARKER = "- ";
const rdcstr LexIter::RIGHT_TRIM_MARKER = " -";
const rdcstr LexIter::LEFT_COMMENT = "/*";
const rdcstr LexIter::RIGHT_COMMENT = "*/";
const rdcstr LexIter::WINDOWS_NEWLINE = "\r\n";

ItemType LexIter::keyword(const rdcstr &word)
{
#define KEYWORD(kw, item) \
  if(word == kw)          \
    return item;
  KEYWORD(".", ItemType::Dot);
  KEYWORD("block", ItemType::Block);
  KEYWORD("define", ItemType::Define);
  KEYWORD("else", ItemType::Else);
  KEYWORD("end", ItemType::End);
  KEYWORD("if", ItemType::If);
  KEYWORD("range", ItemType::Range);
  KEYWORD("null", ItemType::Null);
  KEYWORD("template", ItemType::Template);
  KEYWORD("with", ItemType::With);
  KEYWORD("prefix", ItemType::Prefix);
  KEYWORD("true", ItemType::Bool);
  KEYWORD("false", ItemType::Bool);
#undef KEYWORD
  return ItemType::Error;
}
bool LexIter::emit(ItemType type)
{
  item.offset = start;
  item.size = pos - start;
  item.type = type;
  item.val = input->substr(start, pos - start);
  return true;
}
bool LexIter::emitText()
{
  if(start >= pos)
    return false;
  return emit(ItemType::Text);
}
bool LexIter::lexText()
{
  int32_t i = input->find(LEFT_DELIM, pos);
  if(i >= 0)
  {
    pos = (size_t)i;
    size_t leftDelimPos = pos;
    if(input->substrEq(pos + LEFT_DELIM.size(), LEFT_TRIM_MARKER))
    {
      while(pos > 0)
      {
        --pos;
        if(!isSpace((*input)[pos]))
        {
          ++pos;
          break;
        }
      }
    }
    state = LexState::LeftDelim;
    bool res = emitText();
    pos = leftDelimPos;
    return res;
  }
  pos = input->size();
  state = LexState::EndOfFile;
  return emitText();
}
bool LexIter::lexComment()
{
  pos += LEFT_COMMENT.size();
  int i = input->find(RIGHT_COMMENT, pos);
  if(i < 0)
    return errorf("unclosed comment");
  pos = i + RIGHT_COMMENT.size();
  if(!atRightDelim())
    return errorf("comment ended before closing delimiter");
  bool trimSpace = input->substrEq(pos, RIGHT_TRIM_MARKER);
  pos += RIGHT_DELIM.size();
  if(trimSpace)
  {
    pos += RIGHT_TRIM_MARKER.size();
    while(pos >= 0 && isSpace((*input)[pos]))
      ++pos;
  }
  state = LexState::Text;
  return false;
}
bool LexIter::lexEOF()
{
  return emit(ItemType::EndOfFile);
}
bool LexIter::lexLeftDelim()
{
  pos += LEFT_DELIM.size();
  if(input->substrEq(pos, LEFT_TRIM_MARKER))
    pos += LEFT_TRIM_MARKER.size();
  if(input->substrEq(pos, LEFT_COMMENT))
  {
    return lexComment();
  }
  state = LexState::InsideAction;
  return emit(ItemType::LeftDelim);
}
bool LexIter::lexRightDelim()
{
  bool trimSpace = input->substrEq(pos, RIGHT_TRIM_MARKER);
  pos += RIGHT_DELIM.size();
  if(trimSpace)
    pos += RIGHT_TRIM_MARKER.size();

  state = LexState::Text;
  bool res = emit(ItemType::RightDelim);

  if(trimSpace)
    while(pos >= 0 && isSpace((*input)[pos]))
      ++pos;

  return res;
}
bool LexIter::atRightDelim()
{
  return input->substrEq(pos, RIGHT_DELIM) ||
         (input->substrEq(pos, RIGHT_TRIM_MARKER) &&
          input->substrEq(pos + RIGHT_TRIM_MARKER.size(), RIGHT_DELIM));
  ;
}
bool LexIter::atEndOfLine()
{
  return pos == input->size() || (*input)[pos] == '\n' || input->substrEq(pos, WINDOWS_NEWLINE);
}
bool LexIter::lexSpace()
{
  while(isSpace((*input)[pos]))
    ++pos;
  return emit(ItemType::Space);
}
bool LexIter::atTerminator()
{
  if(atEndOfLine())
    return true;
  char c = (*input)[pos];
  if(isSpace(c) || c == '.' || c == ',' || c == '|' || c == ':' || c == ')' || c == '(')
    return true;
  if(input->substrEq(pos, RIGHT_DELIM))
    return true;
  return false;
}
rdcstr escapeChar(char c)
{
  rdcstr escaped;
  if(isprint(c))
    escaped.push_back(c);
  else
    escaped.sprintf("\\x%02X", (int)c);
  return escaped;
}
bool LexIter::lexVariable()
{
  do
  {
    ++pos;
  } while(pos < input->size() && isAlphaNumeric((*input)[pos]));
  if(!atTerminator())
    return errorf("unrecognized character '%s'", escapeChar((*input)[pos]));
  state = LexState::InsideAction;
  return emit(ItemType::Variable);
}
bool LexIter::lexIdentifier()
{
  while(pos < input->size() && isAlphaNumeric((*input)[pos]))
    ++pos;
  if(!atTerminator())
    return errorf("unrecognized character '%s'", escapeChar((*input)[pos]));

  state = LexState::InsideAction;
  rdcstr word = input->substr(start, pos - start);
  ItemType kwType = keyword(word);
  if(kwType != ItemType::Error)
    return emit(kwType);
  return emit(ItemType::Identifier);
}
bool LexIter::lexField()
{
  ++pos;
  if(atTerminator())
    return emit(ItemType::Dot);

  while(pos < input->size() && isAlphaNumeric((*input)[pos]))
    ++pos;

  if(!atTerminator())
    return errorf("unrecognized character '%s'", escapeChar((*input)[pos]));

  state = LexState::InsideAction;
  return emit(ItemType::Field);
}
bool LexIter::lexInsideAction()
{
  if(atRightDelim())
  {
    if(parenDepth == 0)
      return lexRightDelim();
    return errorf("unclosed left paren");
  }
  if(atEndOfLine())
    return errorf("unclosed action");
  char c = (*input)[pos];
  if(isSpace(c))
    return lexSpace();
  if(c == '|')
  {
    ++pos;
    return emit(ItemType::Pipe);
  }
  if(c == '"')
    return lexQuote();
  if(c == '\'')
    return lexChar();
  if(c == '`')
    return lexRawQuote();
  if(c == '$')
    return lexVariable();
  if(c == '=')
  {
    ++pos;
    return emit(ItemType::Assign);
  }
  if(c == ':')
  {
    ++pos;
    if(atEndOfLine() || (*input)[pos] != '=')
      return errorf("expected `:=`");
    ++pos;
    return emit(ItemType::Declare);
  }
  if(c == '.')
  {
    if(pos + 1 < input->size())
    {
      char d = (*input)[pos + 1];
      if(!isdigit(d))
        return lexField();
    }
    return lexNumber();
  }
  if(c == '+' || c == '-' || isdigit(c))
    return lexNumber();
  if(isAlphaNumeric(c))
    return lexIdentifier();
  if(c == ',')
  {
    ++pos;
    return emit(ItemType::Comma);
  }
  if(c == '(')
  {
    ++pos;
    ++parenDepth;
    return emit(ItemType::LeftParen);
  }
  if(c == ')')
  {
    ++pos;
    --parenDepth;
    if(parenDepth < 0)
      return errorf("unexpected right paren");
    return emit(ItemType::RightParen);
  }
  if(isprint(c))
  {
    ++pos;
    return emit(ItemType::Char);
  }
  return errorf("unrecognized character '%s'", escapeChar(c));
}
bool LexIter::accept(const rdcstr &chars)
{
  if(pos < input->size())
  {
    char c = (*input)[pos];
    for(auto it = chars.begin(); it != chars.end(); ++it)
    {
      if(c == *it)
      {
        ++pos;
        return true;
      }
    }
  }
  return false;
}
void LexIter::scanDigits(bool isHex)
{
  if(isHex)
  {
    while(pos < input->size() && isxdigit((*input)[pos]))
      ++pos;
  }
  else
  {
    while(pos < input->size() && isdigit((*input)[pos]))
      ++pos;
  };
}
bool LexIter::lexNumber()
{
  accept("+-");
  if(accept("0") && accept("xX"))
  {
    // hex
    scanDigits(true);
  }
  else
  {
    // decimal
    scanDigits(false);
    if(accept("."))
      scanDigits(false);
    if(accept("eE"))
    {
      // exponential notation
      accept("+-");
      scanDigits(false);
    }
  }
  accept("uU");
  if(pos < input->size() && isAlphaNumeric((*input)[pos]))
  {
    return errorf("bad number syntax : \"%s\"", input->substr(start, pos - start + 1));
  }
  return emit(ItemType::Number);
}
bool LexIter::lexQuote()
{
  while(true)
  {
    ++pos;
    if(atEndOfLine())
      break;
    char c = (*input)[pos];
    if(c == '"')
    {
      ++pos;
      return emit(ItemType::String);
    }
    if(c == '\\')
    {
      ++pos;
      if(atEndOfLine())
        break;
    }
  }
  return errorf("unterminated quoted string");
}
bool LexIter::lexRawQuote()
{
  for(++pos; pos < input->size(); ++pos)
  {
    if((*input)[pos] == '`')
    {
      ++pos;
      return emit(ItemType::RawString);
    }
  }
  return errorf("unterminated raw quoted string");
}
bool LexIter::lexChar()
{
  while(true)
  {
    ++pos;
    if(atEndOfLine())
      break;
    char c = (*input)[pos];
    if(c == '\'')
    {
      ++pos;
      return emit(ItemType::Char);
    }
    if(c == '\\')
    {
      ++pos;
      if(atEndOfLine())
        break;
    }
  }
  return errorf("unterminated character constant");
}
bool LexIter::update()
{
  start = pos;
  switch(state)
  {
    case LexState::Text: return lexText();
    case LexState::LeftDelim: return lexLeftDelim();
    case LexState::RightDelim: return lexRightDelim();
    case LexState::EndOfFile: return lexEOF();
    case LexState::InsideAction: return lexInsideAction();
    case LexState::Error: return true;
    default: assert(false);
  }
  return true;
}
LexIter::LexIter(const rdcstr &input) : input(&input)
{
  while(!update())
  {
    // empty
  }
}
LexIter &LexIter::operator++()
{
  while(!update())
  {
    // empty
  }
  return *this;
}
}    // namespace lex
}    // namespace templates
}    // namespace cpp_codec

template <>
rdcstr DoStringise(const cpp_codec::templates::lex::ItemType &el)
{
  BEGIN_ENUM_STRINGISE(cpp_codec::templates::lex::ItemType);
  {
    STRINGISE_ENUM_CLASS(Error);
    STRINGISE_ENUM_CLASS(Bool);
    STRINGISE_ENUM_CLASS(Char);
    STRINGISE_ENUM_CLASS(CharConstant);
    STRINGISE_ENUM_CLASS(Complex);
    STRINGISE_ENUM_CLASS(Assign);
    STRINGISE_ENUM_CLASS(Declare);
    STRINGISE_ENUM_CLASS(EndOfFile);
    STRINGISE_ENUM_CLASS(Field);
    STRINGISE_ENUM_CLASS(Identifier);
    STRINGISE_ENUM_CLASS(LeftDelim);
    STRINGISE_ENUM_CLASS(LeftParen);
    STRINGISE_ENUM_CLASS(Number);
    STRINGISE_ENUM_CLASS(Pipe);
    STRINGISE_ENUM_CLASS(RawString);
    STRINGISE_ENUM_CLASS(RightDelim);
    STRINGISE_ENUM_CLASS(RightParen);
    STRINGISE_ENUM_CLASS(Space);
    STRINGISE_ENUM_CLASS(String);
    STRINGISE_ENUM_CLASS(Text);
    STRINGISE_ENUM_CLASS(Variable);
    STRINGISE_ENUM_CLASS(Keyword);
    STRINGISE_ENUM_CLASS(Block);
    STRINGISE_ENUM_CLASS(Dot);
    STRINGISE_ENUM_CLASS(Define);
    STRINGISE_ENUM_CLASS(Else);
    STRINGISE_ENUM_CLASS(End);
    STRINGISE_ENUM_CLASS(If);
    STRINGISE_ENUM_CLASS(Null);
    STRINGISE_ENUM_CLASS(Range);
    STRINGISE_ENUM_CLASS(Template);
    STRINGISE_ENUM_CLASS(With);
    STRINGISE_ENUM_CLASS(Prefix);
  }
  END_ENUM_STRINGISE();
}

template <>
rdcstr DoStringise(const cpp_codec::templates::lex::Item &el)
{
  rdcstr str = ToStr(el.type);
  if(el.val.size() > 0)
  {
    str += "(";
    str += el.val;
    str += ")";
  }
  return str;
}

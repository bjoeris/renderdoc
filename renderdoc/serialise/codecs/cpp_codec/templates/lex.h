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

namespace cpp_codec
{
namespace templates
{
namespace lex
{
inline bool isSpace(char c)
{
  return c == ' ' || c == '\t';
}
inline bool isNewline(char c)
{
  return c == '\r' || c == '\n';
}
inline bool isAlphaNumeric(char c)
{
  return c == '_' || isalnum(c);
}

enum class ItemType
{
  Error,           // error occurred; value is text of error
  Bool,            // boolean constant
  Char,            // printable ASCII character; grab bag for comma etc.
  CharConstant,    // character constant
  Complex,         // complex constant (1+2i); imaginary is just a number
  Assign,          // equals ('=') introducing an assignment
  Declare,         // colon-equals (':=') introducing a declaration
  EndOfFile,
  Field,         // alphanumeric identifier starting with '.'
  Identifier,    // alphanumeric identifier not starting with '.'
  LeftDelim,     // left action delimiter
  LeftParen,     // '(' inside action
  Number,        // simple number, including imaginary
  Pipe,          // pipe symbol
  Comma,         // ',' inside action
  RawString,     // raw quoted string (includes quotes)
  RightDelim,    // right action delimiter
  RightParen,    // ')' inside action
  Space,         // run of spaces separating arguments
  String,        // quoted string (includes quotes)
  Text,          // plain text
  Variable,      // variable starting with '$', such as '$' or  '$1' or '$hello'
  // Keywords appear after all the rest.
  Keyword,     // used only to delimit the keywords
  Block,       // block keyword
  Dot,         // the cursor, spelled '.'
  Define,      // define keyword
  Else,        // else keyword
  End,         // end keyword
  If,          // if keyword
  Null,        // the untyped null constant, easiest to treat as a keyword
  Range,       // range keyword
  Template,    // template keyword
  With,        // with keyword
  Prefix,      // prefix keyword
};

struct Item
{
  ItemType type = ItemType::Error;
  size_t offset = 0;
  size_t size = 0;
  rdcstr val;
};

enum class LexState
{
  Text,
  LeftDelim,
  RightDelim,
  EndOfFile,
  InsideAction,
  Error,
};

class LexIter
{
  const rdcstr *input;
  size_t start = 0;
  size_t pos = 0;
  Item item;
  // std::function<void()> stateFn;
  LexState state = LexState::Text;
  int parenDepth = 0;

  static const rdcstr LEFT_DELIM;
  static const rdcstr RIGHT_DELIM;
  static const rdcstr LEFT_TRIM_MARKER;
  static const rdcstr RIGHT_TRIM_MARKER;
  static const rdcstr LEFT_COMMENT;
  static const rdcstr RIGHT_COMMENT;
  static const rdcstr WINDOWS_NEWLINE;

  static ItemType keyword(const rdcstr &word);
  // static const rdcarray<rdcpair<rdcstr, ItemType>> &keywords();

  template <typename... Args>
  bool errorf(const char *fmt, const Args &... args)
  {
    rdcstr msg;
    msg.sprintf(fmt, args...);

    item.offset = start;
    item.size = pos - start + 1;
    item.type = ItemType::Error;
    item.val = msg;
    state = LexState::Error;
    return true;
  }

  bool emit(ItemType type);
  bool emitText();

  bool lexText();

  bool lexComment();

  bool lexEOF();
  bool lexLeftDelim();
  bool lexRightDelim();

  bool atRightDelim();
  bool atEndOfLine();

  bool lexSpace();
  bool atTerminator();
  bool lexVariable();
  bool lexIdentifier();
  bool lexField();
  bool lexInsideAction();
  bool lexNumber();
  bool lexQuote();
  bool lexRawQuote();
  bool lexChar();

  bool accept(const rdcstr &chars);
  void scanDigits(bool isHex);
  bool update();

public:
  LexIter(const rdcstr &input);
  LexIter(const LexIter &) = default;
  LexIter &operator=(const LexIter &other) = default;
  inline const Item &operator*() const { return item; }
  inline const Item *operator->() const { return &item; }
  LexIter &operator++();
};
}    // namespace lex
}    // namespace templates
}    // namespace cpp_codec

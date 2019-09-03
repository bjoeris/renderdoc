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
#include "lex.h"

#if ENABLED(ENABLE_UNIT_TESTS)

#include "3rdparty/catch/catch.hpp"

using namespace cpp_codec::templates;
using namespace cpp_codec::templates::lex;

void checkLexIter(LexIter itemIter, std::initializer_list<Item> expected, bool checkPos = false)
{
  for(auto exIter = expected.begin(); exIter != expected.end(); ++itemIter, ++exIter)
  {
    CHECK(itemIter->type == exIter->type);
    CHECK(itemIter->val == exIter->val);
    if(checkPos)
    {
      CHECK(itemIter->offset == exIter->offset);
      CHECK(itemIter->size == exIter->size);
    }
  }
}

Item mkItem(ItemType type, const rdcstr &val, size_t offset = 0, size_t size = 0)
{
  Item item;
  item.type = type;
  item.val = val;
  item.offset = offset;
  item.size = size;
  return item;
}

TEST_CASE("Test CPP Codec Template Language Lexer", "[templates]")
{
  Item tEOF = mkItem(ItemType::EndOfFile, "");
  Item tLeft = mkItem(ItemType::LeftDelim, "{{");
  Item tLeftTrim = mkItem(ItemType::LeftDelim, "{{- ");
  Item tRight = mkItem(ItemType::RightDelim, "}}");
  Item tRightTrim = mkItem(ItemType::RightDelim, " -}}");
  Item tLPar = mkItem(ItemType::LeftParen, "(");
  Item tRPar = mkItem(ItemType::RightParen, ")");
  Item tSpace = mkItem(ItemType::Space, " ");
  Item tDot = mkItem(ItemType::Dot, ".");
  Item tPipe = mkItem(ItemType::Pipe, "|");
  Item tIf = mkItem(ItemType::If, "if");
  Item tElse = mkItem(ItemType::Else, "else");
  Item tEnd = mkItem(ItemType::End, "end");

  SECTION("empty") { checkLexIter(LexIter(""), {tEOF}); }
  SECTION("spaces") { checkLexIter(LexIter(" \t\n"), {mkItem(ItemType::Text, " \t\n"), tEOF}); }
  SECTION("text")
  {
    checkLexIter(LexIter("hello 123 testing"), {mkItem(ItemType::Text, "hello 123 testing"), tEOF});
  }
  SECTION("text with comment")
  {
    checkLexIter(LexIter("hello-{{/* comment 123*/}}-world"),
                 {mkItem(ItemType::Text, "hello-"), mkItem(ItemType::Text, "-world"), tEOF});
  }
  SECTION("punctuation")
  {
    checkLexIter(LexIter("{{,@% }}"),
                 {tLeft, mkItem(ItemType::Comma, ","), mkItem(ItemType::Char, "@"),
                  mkItem(ItemType::Char, "%"), tSpace, tRight, tEOF});
  }
  SECTION("parens")
  {
    checkLexIter(LexIter("{{((3))}}"),
                 {tLeft, tLPar, tLPar, mkItem(ItemType::Number, "3"), tRPar, tRPar, tRight, tEOF});
  }
  SECTION("empty action") { checkLexIter(LexIter("{{}}"), {tLeft, tRight, tEOF}); }
  SECTION("identifier")
  {
    checkLexIter(LexIter("{{foo}}"), {tLeft, mkItem(ItemType::Identifier, "foo"), tRight, tEOF});
  }
  SECTION("quote")
  {
    checkLexIter(LexIter(R"({{"abc \n\t\" "}})"),
                 {tLeft, mkItem(ItemType::String, R"("abc \n\t\" ")"), tRight, tEOF});
  }
  SECTION("raw quote")
  {
    checkLexIter(LexIter(R"({{`abc\n\t\" `}})"),
                 {tLeft, mkItem(ItemType::RawString, R"(`abc\n\t\" `)"), tRight, tEOF});
  }
  SECTION("raw quote with newline")
  {
    checkLexIter(LexIter("{{`abc\ndef`}}"),
                 {tLeft, mkItem(ItemType::RawString, "`abc\ndef`"), tRight, tEOF});
  }
  SECTION("numbers")
  {
    checkLexIter(
        LexIter("{{1 02 0x14 -7.2 1e3 +1.2e-4 .7}}"),
        {tLeft, mkItem(ItemType::Number, "1"), tSpace, mkItem(ItemType::Number, "02"), tSpace,
         mkItem(ItemType::Number, "0x14"), tSpace, mkItem(ItemType::Number, "-7.2"), tSpace,
         mkItem(ItemType::Number, "1e3"), tSpace, mkItem(ItemType::Number, "+1.2e-4"), tSpace,
         mkItem(ItemType::Number, ".7"), tRight, tEOF});
  }
  SECTION("characters")
  {
    checkLexIter(
        LexIter(R"({{'a' '\n' '\'' '\\' '\xFF'}})"),
        {tLeft, mkItem(ItemType::Char, "'a'"), tSpace, mkItem(ItemType::Char, R"('\n')"), tSpace,
         mkItem(ItemType::Char, R"('\'')"), tSpace, mkItem(ItemType::Char, R"('\\')"), tSpace,
         mkItem(ItemType::Char, R"('\xFF')"), tRight, tEOF});
  }
  SECTION("bools")
  {
    checkLexIter(LexIter("{{true false}}"), {tLeft, mkItem(ItemType::Bool, "true"), tSpace,
                                             mkItem(ItemType::Bool, "false"), tRight, tEOF});
  }
  SECTION("dot") { checkLexIter(LexIter("{{.}}"), {tLeft, tDot, tRight, tEOF}); }
  SECTION("dots")
  {
    checkLexIter(LexIter("{{.x . .2 .x.y.z}}"),
                 {tLeft, mkItem(ItemType::Field, ".x"), tSpace, tDot, tSpace,
                  mkItem(ItemType::Number, ".2"), tSpace, mkItem(ItemType::Field, ".x"),
                  mkItem(ItemType::Field, ".y"), mkItem(ItemType::Field, ".z"), tRight, tEOF});
  }
  SECTION("keywords")
  {
    checkLexIter(LexIter("{{range if else end with}}"),
                 {tLeft, mkItem(ItemType::Range, "range"), tSpace, mkItem(ItemType::If, "if"),
                  tSpace, mkItem(ItemType::Else, "else"), tSpace, mkItem(ItemType::End, "end"),
                  tSpace, mkItem(ItemType::With, "with"), tRight, tEOF});
  }
  SECTION("pipeline")
  {
    checkLexIter(
        LexIter(R"(intro {{echo hi 1.2 |noargs|args 1 "hi"}} outro)"),
        {mkItem(ItemType::Text, "intro "), tLeft, mkItem(ItemType::Identifier, "echo"), tSpace,
         mkItem(ItemType::Identifier, "hi"), tSpace, mkItem(ItemType::Number, "1.2"), tSpace, tPipe,
         mkItem(ItemType::Identifier, "noargs"), tPipe, mkItem(ItemType::Identifier, "args"),
         tSpace, mkItem(ItemType::Number, "1"), tSpace, mkItem(ItemType::String, R"("hi")"), tRight,
         mkItem(ItemType::Text, " outro"), tEOF});
  }
  SECTION("long pipeline")
  {
    checkLexIter(LexIter("{{|||||}}"), {
                                           tLeft,
                                           tPipe,
                                           tPipe,
                                           tPipe,
                                           tPipe,
                                           tPipe,
                                           tRight,
                                           tEOF,
                                       });
  }
  SECTION("field of parenthesized expression")
  {
    checkLexIter(LexIter("{{(.X).Y}}"), {tLeft, tLPar, mkItem(ItemType::Field, ".X"), tRPar,
                                         mkItem(ItemType::Field, ".Y"), tRight, tEOF});
  }
  SECTION("trimming spaces before and after")
  {
    checkLexIter(LexIter("hello- {{- 3 -}} -world"), {
                                                         mkItem(ItemType::Text, "hello-"),
                                                         tLeftTrim,
                                                         mkItem(ItemType::Number, "3"),
                                                         tRightTrim,
                                                         mkItem(ItemType::Text, "-world"),
                                                         tEOF,
                                                     });
  }
  SECTION("trimming spaces before and after comment")
  {
    checkLexIter(LexIter("hello- {{- /* hello */ -}} -world"), {
                                                                   mkItem(ItemType::Text, "hello-"),
                                                                   mkItem(ItemType::Text, "-world"),
                                                                   tEOF,
                                                               });
  }
  SECTION("if")
  {
    checkLexIter(LexIter("abc {{if foo}} def {{end}} ghi"),
                 {mkItem(ItemType::Text, "abc "), tLeft, tIf, tSpace,
                  mkItem(ItemType::Identifier, "foo"), tRight, mkItem(ItemType::Text, " def "),
                  tLeft, tEnd, tRight, mkItem(ItemType::Text, " ghi")});
  }
  SECTION("errors")
  {
    SECTION("badchar")
    {
      checkLexIter(LexIter("#{{\x01}}"),
                   {
                       mkItem(ItemType::Text, "#"),
                       tLeft,
                       mkItem(ItemType::Error, "unrecognized character '\\x01'"),
                   });
    }
    SECTION("unclosed action")
    {
      checkLexIter(LexIter("{{\n}}"), {
                                          tLeft,
                                          mkItem(ItemType::Error, "unclosed action"),
                                      });
    }
    SECTION("EOF in action")
    {
      checkLexIter(LexIter("{{range"), {
                                           tLeft,
                                           mkItem(ItemType::Range, "range"),
                                           mkItem(ItemType::Error, "unclosed action"),
                                       });
    }
    SECTION("unclosed quote")
    {
      checkLexIter(LexIter("{{\"\n\"}}"), {
                                              tLeft,
                                              mkItem(ItemType::Error, "unterminated quoted string"),
                                          });
    }
    SECTION("unclosed raw quote")
    {
      checkLexIter(LexIter("{{`xx}}"),
                   {
                       tLeft,
                       mkItem(ItemType::Error, "unterminated raw quoted string"),
                   });
    }
    SECTION("unclosed char constant")
    {
      checkLexIter(LexIter("{{'\n}}"),
                   {
                       tLeft,
                       mkItem(ItemType::Error, "unterminated character constant"),
                   });
    }
    SECTION("bad number")
    {
      checkLexIter(LexIter("{{3k}}"), {
                                          tLeft,
                                          mkItem(ItemType::Error, R"(bad number syntax : "3k")"),
                                      });
    }
    SECTION("unclosed paren")
    {
      checkLexIter(LexIter("{{(3}}"), {
                                          tLeft,
                                          tLPar,
                                          mkItem(ItemType::Number, "3"),
                                          mkItem(ItemType::Error, "unclosed left paren"),
                                      });
    }
    SECTION("extra right paren")
    {
      checkLexIter(LexIter("{{3)}}"), {
                                          tLeft,
                                          mkItem(ItemType::Number, "3"),
                                          mkItem(ItemType::Error, "unexpected right paren"),
                                      });
    }
    SECTION("bad comment")
    {
      checkLexIter(LexIter("hello-{{/*/}}-world"), {
                                                       mkItem(ItemType::Text, "hello-"),
                                                       mkItem(ItemType::Error, "unclosed comment"),
                                                   });
    }
  }
}

#endif    // ENABLED(ENABLE_UNIT_TESTS)

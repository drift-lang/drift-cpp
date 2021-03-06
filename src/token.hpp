//
// Copyright (c) 2021 bingxio（丙杺，黄菁）. All rights reserved.
//

// GNU General Public License, more to see file: LICENSE
// https://www.gnu.org/licenses

//          THE DRIFT PROGRAMMING LANGUAGE
//
//          https://github.com/bingxio/drift
//
//          https://www.drift-lang.fun/
//

#ifndef DRIFT_TOKEN_H
#define DRIFT_TOKEN_H

#include <iostream>
#include <map>
#include <sstream>

// tokens
namespace token {
  // total number of token for drift
  constexpr int len = 56;
  // token type
  enum Kind {
    IDENT, // identifier literal
    NUM,   // number     literal
    STR,   // string     literal
    CHAR,  // char       literal
    FLOAT, // float      literal

    ADD, // +
    SUB, // -
    MUL, // *
    DIV, // /

    AS_ADD, // +=
    AS_SUB, // -=
    AS_MUL, // *=
    AS_DIV, // /=

    PLUS,  // ++
    MINUS, // --

    R_ARROW,        // ->
    L_ARROW,        // <-
    L_CURVED_ARROW, // <~

    DOLLAR, // $
    DOT,    // .
    COMMA,  // ,
    COLON,  // :
    EQ,     // =

    GREATER, // >
    LESS,    // <
    GR_EQ,   // >=
    LE_EQ,   // <=

    ADDR,    // &
    OR,      // |
    BANG,    // !
    BANG_EQ, // !=
    EQ_EQ,   // ==
    //    SINGLE_MARK,  // '
    //    DOUBLE_MARk,  // "
    //    OBLIQUE_MARK, // `

    L_BRACE,   // {
    R_BRACE,   // }
    L_PAREN,   // (
    R_PAREN,   // )
    L_BRACKET, // [
    R_BRACKET, // ]

    UNDERLINE, // _

    EFF, // end of file

    // keywords
    USE,
    DEF,
    PUB,
    RET,
    AND,
    END,
    IF,
    EF,
    NF,
    FOR,
    DO,
    OUT,
    TIN,
    NEW,
    MOD,
    AS
  };

  // returns a string of each type
  static std::string kindString[len] = {
      "IDENT",     "NUM",       "STR",
      "CHAR",      "FLOAT",     "ADD",
      "SUB",       "MUL",       "DIV",
      "AS_ADD",    "AS_SUB",    "AS_MUL",
      "AS_DIV",    "PLUS",      "MINUS",
      "R_ARROW",   "L_ARROW",   "L_CURVED_ARROW",
      "DOLLAR",    "DOT",       "COMMA",
      "COLON",     "EQ",        "GREATER",
      "LESS",      "GR_EQ",     "LE_EQ",
      "ADDR",      "OR",        "BANG",
      "BANG_EQ",   "EQ_EQ",     "L_BRACE",
      "R_BRACE",   "L_PAREN",   "R_PAREN",
      "L_BRACKET", "R_BRACKET", "UNDERLINE",
      "EFF",       "USE",       "DEF",
      "PUB",       "RET",       "AND",
      "END",       "IF",        "EF",
      "NF",        "FOR",       "DO",
      "OUT",       "TIN",       "NEW",
      "MOD",       "AS",
  };

  //  token structure
  struct Token {
    // token type
    Kind kind = EFF;
    // token literal
    std::string literal;
    // line of source code
    int line;
  };

  // keywords for drift
  static std::map<std::string, Kind> keyword;

  // 16 keywords
  // initialize it when tokenizer

  // format return token structure
  static std::string toString(const Token &token) {
    std::stringstream str;

    str << "<Token { Kind=";
    str << kindString[token.kind] << " Literal=\"";
    str << token.literal << "\" Line=";
    str << token.line << " }>";

    return str.str();
  }

  // return the corresponding keyword type according to the literal amount
  static Kind getKeyword(const std::string &literal) {
    auto i = keyword.find(literal);
    // search map
    if (i != keyword.end())
      return i->second;
    else
      return IDENT;
  }
}; // namespace token

#endif
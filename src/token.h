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
  constexpr int len = 54;
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
    SUR, // %

    AS_ADD, // +=
    AS_SUB, // -=
    AS_MUL, // *=
    AS_DIV, // /=
    AS_SUR, // %=

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

    L_BRACE,   // {
    R_BRACE,   // }
    L_PAREN,   // (
    R_PAREN,   // )
    L_BRACKET, // [
    R_BRACKET, // ]

    UNDERLINE, // _

    EFF, // end of file

    // keywords
    USE, // 1
    DEF, // 2
    RET, // 3
    LET, // 4
    END, // 5
    IF,  // 6
    EF,  // 7
    NF,  // 8
    FOR, // 9
    DO,  // 10
    OUT, // 11
    TIN, // 12
    NEW, // 13
    MOD  // 14
  };

  // returns a string of each type
  static std::string kindString[len] = {
      "IDENT",     "NUM",       "STR",
      "CHAR",      "FLOAT",     "ADD",
      "SUB",       "MUL",       "DIV",
      "SUR",       "AS_ADD",    "AS_SUB",
      "AS_MUL",    "AS_DIV",    "AS_SUR",
      "R_ARROW",   "L_ARROW",   "L_CURVED_ARROW",
      "DOLLAR",    "DOT",       "COMMA",
      "COLON",     "EQ",        "GREATER",
      "LESS",      "GR_EQ",     "LE_EQ",
      "ADDR",      "OR",        "BANG",
      "BANG_EQ",   "EQ_EQ",     "L_BRACE",
      "R_BRACE",   "L_PAREN",   "R_PAREN",
      "L_BRACKET", "R_BRACKET", "UNDERLINE",
      "EFF",       "USE",       "DEF",
      "RET",       "LET",       "END",
      "IF",        "EF",        "NF",
      "FOR",       "DO",        "OUT",
      "TIN",       "NEW",       "MOD",
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
}; // namespace token

void initializeKeyword(std::map<std::string, token::Kind> *);
token::Kind getKeyword(std::map<std::string, token::Kind>, const std::string &);

#endif
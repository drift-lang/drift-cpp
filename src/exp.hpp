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

#ifndef DRIFT_EXP_H
#define DRIFT_EXP_H

#include <exception>
#include <iostream>
#include <sstream>

// exceptions
namespace exp {
  // total number of exceptions
  constexpr int len = 12;
  // exception type
  enum Kind {
    // LEXER
    UNKNOWN_SYMBOL, // unknown symbol
    CHARACTER_EXP,  // character is empty
    STRING_EXP,     // lost left or right mark
    // PARSER
    UNEXPECTED,     // unexpected
    INVALID_SYNTAX, // invalid syntax
    INCREMENT_OP,   // left value increment operand
    // SEMANTIC
    TYPE_ERROR,    // type error
    DIVISION_ZERO, // div zero
    CANNOT_PUBLIC, // can not to public
    ENUMERATION,   // whole body not definition of enum
    CALL_INHERIT,  // can only be with call expr
    //
    RUNTIME_ERROR,
  };

  //  return a string of exception type
  static std::string kindString[len] = {
      "UNKNOWN_SYMBOL", "CHARACTER_EXP", "STRING_EXP",   "UNEXPECTED",
      "INVALID_SYNTAX", "INCREMENT_OP",  "TYPE_ERROR",   "DIVISION_ZERO",
      "CANNOT_PUBLIC",  "ENUMERATION",   "CALL_INHERIT", "RUNTIME_ERROR",
  };

  // exception structure
  class Exp : public std::exception {
  private:
    // exception kind
    Kind kind;
    // exception message
    std::string message;
    // at exception line of source code
    int line;

  public:
    explicit Exp(Kind kind, std::string message, int line) {
      this->kind = kind;
      this->message = std::move(message);
      this->line = line;
    }

    // return a string of exception structure
    std::string stringer() {
      std::stringstream str;

      str << "<Exception { Kind=";
      str << kindString[this->kind] << " Message=\"";
      str << this->message << "\" Line=";
      str << this->line << " }>";

      return str.str();
    }
  };
}; // namespace exp

#endif
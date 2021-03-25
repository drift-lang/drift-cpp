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

#ifndef DRIFT_OPCODE_H
#define DRIFT_OPCODE_H

#include <iostream>

// bytecode
namespace byte {
  // total number of bytecodes
  constexpr int len = 40;
  // bytecode type
  enum Code {
    CONST,   // O
    ASSIGN,  // ASSIGN
    STORE,   // GLOBAL
    LOAD,    // LOAD
    INDEX,   // INDEX
    REPLACE, // REPLACE
    GET,     // GET
    SET,     // SET
    CALL,    // CALL
    ORIG,    // ORIG
    NAME,    // NAME
    NEW,     // NEW
    FUNC,    // FUNC
    WHOLE,   // WHOLE
    ENUM,    // ENUM

    MOD, // MOD
    USE, // USE

    B_ARR, // ARRAY
    B_TUP, // TUPLE
    B_MAP, // MAP

    // INFIX
    ADD,  // +
    SUB,  // -
    MUL,  // *
    DIV,  // /
    SUR,  // %
    GR,   // >
    LE,   // <
    GR_E, // >=
    LE_E, // <=
    E_E,  // ==
    N_E,  // !=
    AND,  // &
    OR,   // |

    // PREFIX
    BANG, // !
    NOT,  // -

    JUMP,   // JUMP
    F_JUMP, // F_JUMP
    T_JUMP,

    RET_N, // RET_N
    RET,
  };

  // return a string of bytecode
  static std::string codeString[len] = {
      "CONST", "ASSIGN", "STORE", "LOAD",  "INDEX",  "REPLACE", "GET",   "SET",
      "CALL",  "ORIG",   "NAME",  "NEW",   "FUNC",   "WHOLE",   "ENUM",  "MOD",
      "USE",   "B_ARR",  "B_TUP", "B_MAP", "ADD",    "SUB",     "MUL",   "DIV",
      "SUR",   "GR",     "LE",    "GR_E",  "LE_E",   "E_E",     "N_E",   "AND",
      "OR",    "BANG",   "NOT",   "JUMP",  "F_JUMP", "T_JUMP",  "RET_N", "RET",
  };
}; // namespace byte

#endif
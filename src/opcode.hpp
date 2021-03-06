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
  constexpr int len = 50;
  // bytecode type
  enum Code {
    CONST,  // O
    ASSIGN, // ASSIGN
    STORE,  // V
    LOAD,   // V
    INDEX,  // INDEX
    GET,    // GET
    SET,    // SET
    CALL,   // CALL
    CALL_I, // CALL_I
    ORIG,   // ORIG
    NAME,   // NAME
    NEW,    // NEW
    FUNC,   // FUNC
    CHA,    // CHA
    END,    // END
    WHOLE,  // WHOLE
    ENUM,   // ENUM

    PUB, // PUB
    MOD, // MOD
    USE, // USE
    UAS, // UAS

    B_ARR, // ARRAY
    B_TUP, // TUPLE
    B_MAP, // MAP

    INCR,   // DECR
    DECR,   // INCR
    P_INCR, // P_INCR
    P_DECR, // P_DECR

    // INFIX
    ADD,   // +
    SUB,   // -
    MUL,   // *
    DIV,   // /
    A_ADD, // +=
    A_SUB, // -=
    A_MUL, // *=
    A_DIV, // /=
    GR,    // >
    LE,    // <
    GR_E,  // >=
    LE_E,  // <=
    E_E,   // ==
    N_E,   // !=
    AND,   // &
    OR,    // |

    // PREFIX
    BANG, // !
    NOT,  // -

    JUMP,   // JUMP
    F_JUMP, // F_JUMP
    T_JUMP,

    RET,
  };

  // return a string of bytecode
  static std::string codeString[len] = {
      "CONST", "ASSIGN", "STORE", "LOAD", "INDEX", "GET",    "SET",
      "CALL",  "CALL_I", "ORIG",  "NAME", "NEW",   "FUNC",   "CHA",
      "END",   "WHOLE",  "ENUM",  "PUB",  "MOD",   "USE",    "UAS",
      "B_ARR", "B_TUP",  "B_MAP", "INCR", "DECR",  "P_INCR", "P_DECR",
      "ADD",   "SUB",    "MUL",   "DIV",  "A_ADD", "A_SUB",  "A_MUL",
      "A_DIV", "GR",     "LE",    "GR_E", "LE_E",  "E_E",    "N_E",
      "AND",   "OR",     "BANG",  "NOT",  "JUMP",  "F_JUMP", "T_JUMP",
      "RET",
  };
}; // namespace byte

#endif
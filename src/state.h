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

#ifndef DRIFT_STATE_H
#define DRIFT_STATE_H

#include <iostream>
#include <vector>

struct State {
  std::string filePath = "REPL_MODE"; // current read file
  std::string message;                // exception message

  int line = 1; // syntax lineno
  int kind;
};

#endif
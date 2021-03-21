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

#ifndef DRIFT_BUILTIN_H
#define DRIFT_BUILTIN_H

#include <iostream>
#include <vector>

#include "exception.h"
#include "frame.h"

struct builtin {
  std::string name;                     // builtin name
  void (*to)(object::Object*, Frame*);  // to handler function
};

// return it is builtin function name
bool isBuiltinName(std::string);

// if its builtin function to call it
void builtinFuncCall(std::string, object::Object*, Frame*);

#endif
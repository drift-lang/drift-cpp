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

#ifndef DRIFT_MODULE_H
#define DRIFT_MODULE_H

#include <algorithm>
#include <vector>

#include "exp.hpp"
#include "frame.hpp"
#include "object.hpp"
#include "system.hpp"
#include "util.h"

#include "compiler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semantic.hpp"
#include "vm.hpp"

// add new module into global state
bool addModule(std::vector<object::Module *> *, std::string, Frame *,
               std::vector<std::string>);

// get module with name
object::Module *getModule(std::vector<object::Module *> *, std::string);

void loadStdModules(std::vector<object::Module *> *); // load standard modules

#endif
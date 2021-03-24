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

#include <vector>

#include "object.h"
#include "util.h"

// add new module into global state and return error message
std::string *addModule(std::vector<object::Module *> *, std::string, Frame *);

// get module with name
std::vector<object::Module *> getModule(std::vector<object::Module *> *,
                                        std::string);

#endif
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

#include "frame.hpp"
#include "object.hpp"

extern std::vector<object::Module *> mods;

void addModule(std::string, Frame *f, std::vector<std::string>);

object::Module *getModule(std::string);

#endif
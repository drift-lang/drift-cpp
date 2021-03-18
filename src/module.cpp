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

#include "module.hpp"

void addModule(std::string name, Frame *f, std::vector<std::string> pub) {
  mods.push_back(new object::Module(name, f, pub));
}

object::Module *getModule(std::string name) {
  for (auto i : mods) {
    if (i->name == name) {
      return i;
    }
  }
  return nullptr;
}
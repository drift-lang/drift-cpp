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

// add new module into global state
bool addModule(std::vector<object::Module *> mods, std::string name, Frame *f,
               std::vector<std::string> pub) {
  if (getModule(mods, name) != nullptr) { // redefining module
    return false;
  } else {
    mods.push_back(new object::Module(name, f, pub)); // insert new module
  }
  return true; // OK
}

// get module with name
object::Module *getModule(std::vector<object::Module *> mods,
                          std::string name) {
  for (auto i : mods) {
    if (i->name == name) {
      return i;
    }
  }
  return nullptr;
}
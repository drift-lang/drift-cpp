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

// add new module into global state and return error message
std::string *addModule(std::vector<object::Module *> *mods, std::string name,
                       Frame *f, std::vector<std::string> pub) {
  std::vector<object::Module *> m = getModule(mods, name);

  // multiple modules with the same name are alloweds
  for (auto i : m)
    if (sameValue(i->pub, pub)) // same value
      return new std::string("same module defines the same name");

  mods->push_back(new object::Module(name, f, pub)); // insert new module

  return nullptr; // OK
}

// get module with name
std::vector<object::Module *> getModule(std::vector<object::Module *> *mods,
                                        std::string name) {
  std::vector<object::Module *> m; // multiple modules
  for (auto i : *mods)
    if (i->name == name) m.push_back(i);
  return m;
}
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

#include <map>

#ifndef DRIFT_TABLE_H
#define DRIFT_TABLE_H

// symbols table
struct Table {
  std::map<std::string, object::Object *> symbols;

  // lookup a name
  object::Object *lookUp(std::string name) {
    if (symbols.count(name) == 0) return nullptr;
    return symbols.at(name);
  }

  void clear() { symbols.clear(); }
};

#endif
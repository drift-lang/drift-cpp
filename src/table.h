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

#ifndef DRIFT_TABLE_H
#define DRIFT_TABLE_H

#include <map>

#include "object.h"

struct Table {
  std::map<std::string, object::Object *> symbols;

  void remove(std::string n) { symbols.erase(n); }

  void clear() { symbols.clear(); }

  bool empty() { return symbols.empty(); }

  object::Object *lookUp(std::string n) {
    for (auto i : symbols)
      if (i.first == n)
        return i.second;
    return nullptr;
  }

  void emit(std::string n, object::Object *o) { symbols[n] = o; }

  void dissemble() {
    printf("SYMBOL: \n");
    for (auto i : symbols) {
      printf("%20s: %40s %10p\n", i.first.c_str(),
             i.second->rawStringer().c_str(), i.second);
    }
  }
};

#endif
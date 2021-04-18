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

// Symbol table structure
struct Table {
  std::map<std::string, object::Object *> symbols; /* Elements */

  // Remove a name
  void remove(std::string n) { symbols.erase(n); }

  // Clear all elements
  void clear() { symbols.clear(); }

  // Return is empty of elements
  bool empty() { return symbols.empty(); }

  // Lookup a name
  object::Object *lookUp(std::string n) {
    for (auto i : symbols)
      if (i.first == n)
        return i.second;
    return nullptr;
  }

  // To emit a name with its object
  void emit(std::string n, object::Object *o) { symbols[n] = o; }

  // Dissemble symbols in table
  void dissemble() {
    printf("SYMBOL: \n");
    for (auto i : symbols) {
      printf("%20s: %40s %10p\n", i.first.c_str(),
             i.second->rawStringer().c_str(), i.second);
    }
  }
};

#endif
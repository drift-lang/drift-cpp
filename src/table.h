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
  std::map<std::string, object::Object*> symbols;

  void remove(std::string n) { symbols.erase(n); }

  void clear() { symbols.clear(); }

  bool empty() { return symbols.empty(); }
};

#endif
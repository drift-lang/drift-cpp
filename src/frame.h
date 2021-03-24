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

#ifndef DRIFT_FRAME_H
#define DRIFT_FRAME_H

#include "stack.h"
#include "table.h"

// frame structure
struct Frame {
  Entity *entity; // ENTITY

  Table tb; // SYMBOL

  Stack<object::Object *> data;  // DATA
  object::Object *ret = nullptr; // RETURN

  std::string mod; // MODULE NAME

  explicit Frame(Entity *e) : entity(e) {}
};

#endif
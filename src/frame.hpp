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

#include "stack.hpp"
#include "table.hpp"

// frame structure
struct Frame {
    Entity *entity; // ENTITY

    Table local;                  // local names
    Stack<object::Object *> data; // data stack

    explicit Frame(Entity *e) : entity(e) {}
};

#endif
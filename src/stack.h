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

#ifndef DRIFT_STACK_H
#define DRIFT_STACK_H

#include <iostream>

// Type stack structure
template <class T> class Stack {
private:
  int capacity = 4, count = 0; /* Capacity and count */

  // Elements with generic T
  T *elements;

public:
  // First, alloc memory to default capacity 4
  explicit Stack() { this->elements = new T[capacity]; }

  // After program out to free elements
  ~Stack() { delete[] elements; }

  // Push a element
  void push(T t) {
    if (count + 1 > capacity) {
      this->capacity = capacity * 2; /* Renew capacity */
      this->elements =
          (T *)realloc(this->elements, sizeof(T) * capacity); /* Realloc */
    }
    this->elements[count++] = t; /* Push */
  }

  // Return the element of position
  T at(int pos) { return this->elements[pos]; }

  // Pop element
  T pop() { return this->elements[--count]; }

  // Top element
  T top() { return this->elements[count - 1]; }

  // Return length of elements
  int len() { return count; }

  // Return is empty of elements
  bool empty() { return count == 0; }

  // Clear all
  void clear() {
    while (len()) {
      this->pop();
    }
  }

  // Stringer
  std::string stringer() {
    return "<Stack count = " + std::to_string(count) + ">";
  }
};

#endif
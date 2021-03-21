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

template <class T>
class Stack {
 private:
  int capacity = 4, count = 0;

  T* elements;

 public:
  explicit Stack() { this->elements = new T[capacity]; }

  ~Stack() { delete[] elements; }

  void push(T t) {
    if (count + 1 > capacity) {
      this->capacity = capacity * 2;
      this->elements = (T*)realloc(this->elements, sizeof(T) * capacity);
    }
    this->elements[count++] = t;
  }

  T at(int pos) { return this->elements[pos]; }

  T pop() { return this->elements[--count]; }

  T top() { return this->elements[count]; }

  int len() { return count; }

  bool empty() { return count == 0; }

  void clear() {
    while (len()) {
      this->pop();
    }
  }

  std::string stringer() {
    return "<Stack count = " + std::to_string(count) + ">";
  }
};

#endif
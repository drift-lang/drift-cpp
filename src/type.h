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

#ifndef DRIFT_TYPE_H
#define DRIFT_TYPE_H

#include <iostream>

#include "token.h"

// types for drift
enum TypeKind {
  T_INT,   // int
  T_FLOAT, // float
  T_STR,   // str
  T_CHAR,  // char
  T_BOOL,  // bool
  T_ARRAY, // []<T>
  T_MAP,   // <T1, T2>
  T_TUPLE, // (T)
  T_FUNC,  // |A| -> R
  T_USER,  // user
};

// basic type for drift
//
#define S_INT "int"     // 1
#define S_FLOAT "float" // 2
#define S_STR "str"     // 3
#define S_CHAR "char"   // 4
#define S_BOOL "bool"   // 5

// TYPE
class Type {
public:
  // stringer
  virtual std::string stringer() = 0;
  // kind of basic type
  virtual TypeKind kind() = 0;
};

// <int>
class Int : public Type {
public:
  std::string stringer() override { return "<Int>"; }

  TypeKind kind() override { return T_INT; }
};

// float
class Float : public Type {
public:
  std::string stringer() override { return "<Float>"; }

  TypeKind kind() override { return T_FLOAT; }
};

// str
class Str : public Type {
public:
  std::string stringer() override { return "<Str>"; }

  TypeKind kind() override { return T_STR; }
};

// char
class Char : public Type {
public:
  std::string stringer() override { return "<Char>"; }

  TypeKind kind() override { return T_CHAR; }
};

// bool
class Bool : public Type {
public:
  std::string stringer() override { return "<Bool>"; }

  TypeKind kind() override { return T_BOOL; }
};

// array (not keyword, for compiler analysis)
// []<type>
class Array : public Type {
public:
  Type *T; // type for elements

  int count; // defaule built original value

  explicit Array(Type *T, int count) : T(T), count(count) {}

  std::string stringer() override {
    if (count != -1) {
      return "<Array T=" + T->stringer() + " Count=" + std::to_string(count) +
             ">";
    }
    return "<Array T=" + T->stringer() + ">";
  }

  TypeKind kind() override { return T_ARRAY; }
};

// map (not keyword, for compiler analysis)
// <type, type>
class Map : public Type {
public:
  Type *T1; // K
  Type *T2; // V

  explicit Map(Type *T1, Type *T2) : T1(T1), T2(T2) {}

  std::string stringer() override {
    return "<Map T1=" + T1->stringer() + " T2=" + T2->stringer() + ">";
  }

  TypeKind kind() override { return T_MAP; }
};

// tuple (not keyword, for compiler analysis)
// (type)
class Tuple : public Type {
public:
  Type *T; // type for elements

  explicit Tuple(Type *T) : T(T) {}

  std::string stringer() override { return "<Tuple T=" + T->stringer() + ">"; }

  TypeKind kind() override { return T_TUPLE; }
};

// function
class Func : public Type {
public:
  std::vector<Type *> arguments; // function arguments
  Type *ret;                     // function return

  explicit Func(std::vector<Type *> arguments, Type *ret)
      : arguments(arguments), ret(ret) {}

  std::string stringer() override {
    if (ret == nullptr) {
      return "<Func A=" + std::to_string(arguments.size()) + " R=NONE>";
    }
    return "<Func A=" + std::to_string(arguments.size()) +
           " R=" + ret->stringer() + ">";
  }

  TypeKind kind() override { return T_FUNC; }
};

// user definition type
// `type`
class User : public Type {
public:
  token::Token name;

  explicit User(token::Token name) { this->name = std::move(name); }

  std::string stringer() override {
    return "<User Name='" + name.literal + "'>";
  }

  TypeKind kind() override { return T_USER; }
};

#endif
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

#ifndef DRIFT_OBJECT_H
#define DRIFT_OBJECT_H

#include "ast.hpp"

struct Entity;

// object
namespace object {
  // kind
  enum Kind {
    INT,
    FLOAT,
    STR,
    CHAR,
    BOOL,
    ARRAY,
    TUPLE,
    MAP,
    ENUM,
    FUNC,
    WHOLE
  };

  // object abstract
  class Object {
  public:
    // return a string of dis object
    virtual std::string stringer() = 0;
    // return the kind of object
    virtual Kind kind() = 0;
  };

  // INT
  class Int : public Object {
  public:
    int value;

    Int(int v) : value(v) {}

    std::string stringer() override {
      return "<Int " + std::to_string(value) + ">";
    }

    Kind kind() override { return INT; }
  };

  // FLOAT
  class Float : public Object {
  public:
    double value;

    Float(float v) : value(v) {}

    std::string stringer() override {
      return "<Float " + std::to_string(value) + ">";
    }

    Kind kind() override { return FLOAT; }
  };

  // STR
  class Str : public Object {
  public:
    std::string value;
    bool longer = false;

    Str(std::string v) : value(v) {}

    Str(std::string v, bool longer) : value(v), longer(longer) {
      value.pop_back(); // long character judgment end, delete judgment
                        // char
    }

    std::string stringer() override {
      if (longer) {
        return "<Str LONGER>";
      }
      return "<Str \"" + value + "\" >";
    }

    Kind kind() override { return STR; }
  };

  // CHAR
  class Char : public Object {
  public:
    char value;

    Char(char v) : value(v) {}

    std::string stringer() override {
      std::stringstream str;

      str << "<Char '" << value << "'>";

      return str.str();
    }

    Kind kind() override { return CHAR; }
  };

  // BOOL
  class Bool : public Object {
  public:
    bool value;

    Bool(bool v) : value(v) {}

    std::string stringer() override {
      return "<Bool " + std::to_string(value) + ">";
    }

    Kind kind() override { return BOOL; }
  };

  // ARRAY
  class Array : public Object {
  public:
    std::vector<object::Object *> elements;

    std::string stringer() override {
      std::stringstream str;

      str << "<Array [";
      for (auto iter = elements.begin(); iter != elements.end();) {
        str << (*iter)->stringer();
        if (++iter != elements.end()) {
          str << ", ";
        }
      }

      str << "]>";
      return str.str();
    }

    Kind kind() override { return ARRAY; }
  };

  // TUPLE
  class Tuple : public Object {
  public:
    std::vector<object::Object *> elements;

    std::string stringer() override {
      std::stringstream str;

      str << "<Tuple (";
      for (auto iter = elements.begin(); iter != elements.end();) {
        str << (*iter)->stringer();
        if (++iter != elements.end()) {
          str << ", ";
        }
      }

      str << ")>";
      return str.str();
    }

    Kind kind() override { return TUPLE; }
  };

  // MAP
  class Map : public Object {
  public:
    std::map<object::Object *, object::Object *> value;

    std::string stringer() override {
      if (value.empty()) return "<Map {}>";

      std::stringstream str;
      str << "<Map {";
      for (auto iter = value.begin(); iter != value.end();) {
        str << "K: " << iter->first->stringer()
            << " V: " << iter->second->stringer();
        if (++iter != value.end()) {
          str << ", ";
        }
      }

      str << "}>";
      return str.str();
    }

    Kind kind() override { return MAP; }
  };

  // ENUM
  class Enum : public Object {
  public:
    std::string name;
    std::map<int, std::string> elements;

    std::string stringer() override { return "<Enum '" + name + "'>"; }

    Kind kind() override { return ENUM; }
  };

  // FUNC
  class Func : public Object {
  public:
    std::string name; // function name

    ast::Arg arguments; // function args
    ast::Type *ret;     // function return

    Entity *entity; // function entity

    std::string stringer() override { return "<Func '" + name + "'>"; }

    Kind kind() override { return FUNC; }
  };

  // WHOLE
  class Whole : public Object {
  public:
    std::string name; // whole name

    Entity *entity; // whole entity

    // interface definition
    std::vector<std::tuple<std::string, ast::Arg, ast::Type *>> interface;

    // inherit definition
    std::vector<std::string> inherit;

    std::string stringer() override { return "<Whole '" + name + "'>"; }

    Kind kind() override { return WHOLE; }
  };
}; // namespace object

#endif
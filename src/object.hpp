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
struct Frame;

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
    WHOLE,
    MODULE
  };

  // object abstract
  class Object {
  public:
    // return a string of dis object
    virtual std::string stringer() = 0;
    // return a raw string of dis object
    virtual std::string rawStringer() = 0;
    // return the kind of object
    virtual Kind kind() = 0;
  };

  // INT
  class Int : public Object {
  public:
    int value;

    Int(int v) : value(v) {}

    std::string rawStringer() override {
      return "<Int " + std::to_string(value) + ">";
    }

    std::string stringer() override { return std::to_string(value); }

    Kind kind() override { return INT; }
  };

  // FLOAT
  class Float : public Object {
  public:
    double value;

    Float(float v) : value(v) {}

    std::string rawStringer() override {
      return "<Float " + std::to_string(value) + ">";
    }

    std::string stringer() override { return std::to_string(value); }

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

    std::string rawStringer() override {
      if (longer) {
        return "<Str LONGER>";
      }
      return "<Str \"" + value + "\">";
    }

    std::string stringer() override { return value; }

    Kind kind() override { return STR; }
  };

  // CHAR
  class Char : public Object {
  public:
    char value;

    Char(char v) : value(v) {}

    std::string rawStringer() override {
      std::stringstream str;

      str << "<Char '" << value << "'>";

      return str.str();
    }

    std::string stringer() override { return "" + value; }

    Kind kind() override { return CHAR; }
  };

  // BOOL
  class Bool : public Object {
  public:
    bool value;

    Bool(bool v) : value(v) {}

    std::string rawStringer() override {
      return "<Bool " + std::to_string(value) + ">";
    }

    std::string stringer() override { return value ? "T" : "F"; }

    Kind kind() override { return BOOL; }
  };

  // ARRAY
  class Array : public Object {
  public:
    std::vector<object::Object *> elements;

    std::string rawStringer() override {
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

    std::string stringer() override {
      std::stringstream str;

      str << "[";
      for (auto iter = elements.begin(); iter != elements.end();) {
        str << (*iter)->stringer();
        if (++iter != elements.end()) {
          str << ", ";
        }
      }

      str << "]";
      return str.str();
    }

    Kind kind() override { return ARRAY; }
  };

  // TUPLE
  class Tuple : public Object {
  public:
    std::vector<object::Object *> elements;

    std::string rawStringer() override {
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

    std::string stringer() override {
      std::stringstream str;

      str << "(";
      for (auto iter = elements.begin(); iter != elements.end();) {
        str << (*iter)->stringer();
        if (++iter != elements.end()) {
          str << ", ";
        }
      }

      str << ")";
      return str.str();
    }

    Kind kind() override { return TUPLE; }
  };

  // MAP
  class Map : public Object {
  public:
    std::map<object::Object *, object::Object *> elements;

    std::string rawStringer() override {
      std::stringstream str;

      str << "<Map {";
      for (auto iter = elements.begin(); iter != elements.end();) {
        str << "K: " << iter->first->stringer()
            << " V: " << iter->second->stringer();
        if (++iter != elements.end()) {
          str << ", ";
        }
      }

      str << "}>";
      return str.str();
    }

    std::string stringer() override {
      std::stringstream str;

      str << "{";
      for (auto iter = elements.begin(); iter != elements.end();) {
        str << iter->first->stringer() << ": " << iter->second->stringer();
        if (++iter != elements.end()) {
          str << ", ";
        }
      }

      str << "}";
      return str.str();
    }

    Kind kind() override { return MAP; }
  };

  // ENUM
  class Enum : public Object {
  public:
    std::string name;
    std::map<int, std::string> elements;

    std::string rawStringer() override { return "<Enum '" + name + "'>"; }
    std::string stringer() override { return "<Enum '" + name + "'>"; }

    Kind kind() override { return ENUM; }
  };

  // FUNC
  class Func : public Object {
  public:
    std::string name; // function name

    ast::FuncArg arguments; // function args
    ast::Type *ret;         // function return

    Entity *entity; // function entity

    std::string rawStringer() override { return "<Func '" + name + "'>"; }
    std::string stringer() override { return "<Func '" + name + "'>"; }

    Kind kind() override { return FUNC; }
  };

  // WHOLE
  class Whole : public Object {
  public:
    std::string name; // whole name

    Entity *entity; // whole entity

    // interface definition
    std::vector<std::tuple<std::string, ast::FaceArg, ast::Type *>> interface;
    // inherit definition
    std::vector<std::string> inherit;

    // whole fram
    Frame *f;

    std::string rawStringer() override { return "<Whole '" + name + "'>"; }
    std::string stringer() override { return "<Whole '" + name + "'>"; }

    Kind kind() override { return WHOLE; }
  };

  // MODULE
  class Module : public Object {
  public:
    std::string name; // module name

    Frame *f;                     // module evaluated frame
    std::vector<std::string> pub; // module public names

    explicit Module(std::string name, Frame *f, std::vector<std::string> pub) {
      this->name = name;
      this->f = f;
      this->pub = pub;
    }

    std::string rawStringer() override { return "<Module '" + name + "'>"; }
    std::string stringer() override { return "<Module '" + name + "'>"; }

    Kind kind() override { return MODULE; }
  };
}; // namespace object

#endif
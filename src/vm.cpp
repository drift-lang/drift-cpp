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

#include "vm.hpp"

// top frame
Frame *vm::top() { return frames.back(); }

// push object to the current frame
void vm::pushData(object::Object *obj) { top()->data.push(obj); }

// pop the top of data stack
object::Object *vm::popData() { return top()->data.pop(); }

// emit new name of table to the current frame
void vm::emitTable(std::string name, object::Object *obj) {
  top()->local.symbols.insert(std::make_pair(name, obj));
}

// look up a name
object::Object *vm::lookUp(std::string n) { return top()->local.lookUp(n); }

// first to end constant iterator for current frame's entity
object::Object *vm::retConstant() {
  return top()->entity->constants.at(top()->entity->offsets.at(op));
}

// first to end
ast::Type *vm::retType() {
  return top()->entity->types.at(top()->entity->offsets.at(op));
}

// first to end
std::string vm::retName() {
  return top()->entity->names.at(top()->entity->offsets.at(op));
}

// first to end
int vm::retOffset() { return top()->entity->offsets.at(op); }

// throw an exception
void error(std::string message) {
  throw exp::Exp(exp::RUNTIME_ERROR, message, -1);
}

// are the comparison types the same
void vm::typeChecker(ast::Type *x, object::Object *y) {
  // base type error
  if (
      // int
      (x->kind() == ast::T_INT && y->kind() != object::INT) ||
      // float
      (x->kind() == ast::T_FLOAT && y->kind() != object::FLOAT) ||
      // str
      (x->kind() == ast::T_STR && y->kind() != object::STR) ||
      // char
      (x->kind() == ast::T_CHAR && y->kind() != object::CHAR)) {
    error("type error, require: " + x->stringer() +
          ", found: " + y->stringer());
  }
  // list
  if (x->kind() == ast::T_ARRAY) {
    ast::Array *T = static_cast<ast::Array *>(x);
    object::Array *arr = static_cast<object::Array *>(y);

    for (auto i : arr->elements) {
      this->typeChecker(T->T, i);
    }
  }
  // tuple
  if (x->kind() == ast::T_TUPLE) {
    ast::Tuple *T = static_cast<ast::Tuple *>(x);
    object::Tuple *tup = static_cast<object::Tuple *>(y);

    for (auto i : tup->elements) {
      this->typeChecker(T->T, i);
    }
  }
  // map
  if (x->kind() == ast::T_MAP) {
    ast::Map *T = static_cast<ast::Map *>(x);
    object::Map *map = static_cast<object::Map *>(y);

    for (auto &i : map->value) {
      this->typeChecker(T->T1, i.first);  // K
      this->typeChecker(T->T2, i.second); // R
    }
  }
}

void vm::evaluate() {

#define BINARY_OP(T, L, OP, R) this->pushData(new T(L OP R));

  for (int ip = 0; ip < top()->entity->codes.size(); ip++) {

    // bytecode
    byte::Code co = top()->entity->codes.at(ip);

    switch (co) {

      case byte::CONST:
        this->pushData(this->retConstant());
        this->op++;
        break;

      case byte::ADD:
      case byte::A_ADD: {
        object::Object *y = this->popData();
        object::Object *x = this->popData();

        if (x->kind() == object::INT) {
          switch (y->kind()) {
            case object::INT: {
              BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, +,
                        static_cast<object::Int *>(y)->value);
              break;
            }
            case object::FLOAT: {
              BINARY_OP(object::Float, static_cast<object::Int *>(x)->value, +,
                        static_cast<object::Float *>(y)->value);
              break;
            }
          }
        }
        if (x->kind() == object::FLOAT) {
          switch (y->kind()) {
            case object::INT: {
              BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                        +, static_cast<object::Int *>(y)->value);
              break;
            }
            case object::FLOAT: {
              BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                        +, static_cast<object::Float *>(y)->value);
              break;
            }
          }
        }
        if (x->kind() == object::STR && y->kind() == object::STR) {
          object::Str *l = static_cast<object::Str *>(x);
          object::Str *r = static_cast<object::Str *>(y);

          if (l->longer || r->longer) {
            error("cannot plus two long string literal");
          }

          this->pushData(new object::Str(l->value + r->value));
        }
        break;
      }
      case byte::SUB:
      case byte::A_SUB: {
        object::Object *y = this->popData();
        object::Object *x = this->popData();

        if (x->kind() == object::INT) {
          switch (y->kind()) {
            case object::INT: {
              BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, -,
                        static_cast<object::Int *>(y)->value);
              break;
            }
            case object::FLOAT: {
              BINARY_OP(object::Float, static_cast<object::Int *>(x)->value, -,
                        static_cast<object::Float *>(y)->value);
              break;
            }
          }
        }
        if (x->kind() == object::FLOAT) {
          switch (y->kind()) {
            case object::INT: {
              BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                        -, static_cast<object::Int *>(y)->value);
              break;
            }
            case object::FLOAT: {
              BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                        -, static_cast<object::Float *>(y)->value);
              break;
            }
          }
        }
        break;
      }
      case byte::MUL:
      case byte::A_MUL: {
        object::Object *y = this->popData();
        object::Object *x = this->popData();

        if (x->kind() == object::INT) {
          switch (y->kind()) {
            case object::INT: {
              BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, *,
                        static_cast<object::Int *>(y)->value);
              break;
            }
            case object::FLOAT: {
              BINARY_OP(object::Float, static_cast<object::Int *>(x)->value, *,
                        static_cast<object::Float *>(y)->value);
              break;
            }
          }
        }
        if (x->kind() == object::FLOAT) {
          switch (y->kind()) {
            case object::INT: {
              BINARY_OP(object::Float,
                        static_cast<object::Float *>(x)->value, *,
                        static_cast<object::Int *>(y)->value);
              break;
            }
            case object::FLOAT: {
              BINARY_OP(object::Float,
                        static_cast<object::Float *>(x)->value, *,
                        static_cast<object::Float *>(y)->value);
              break;
            }
          }
        }
        break;
      }
      case byte::DIV:
      case byte::A_DIV: {
        object::Object *y = this->popData();
        object::Object *x = this->popData();

        if (x->kind() == object::INT) {
          switch (y->kind()) {
            case object::INT: {
              if (static_cast<object::Int *>(y)->value == 0)
                error("division by zero");
              BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, /,
                        static_cast<object::Int *>(y)->value);
              break;
            }
            case object::FLOAT: {
              if (static_cast<object::Float *>(y)->value == 0)
                error("division by zero");
              BINARY_OP(object::Float, static_cast<object::Int *>(x)->value, /,
                        static_cast<object::Float *>(y)->value);
              break;
            }
          }
        }
        if (x->kind() == object::FLOAT) {
          switch (y->kind()) {
            case object::INT: {
              if (static_cast<object::Int *>(y)->value == 0)
                error("division by zero");
              BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                        /, static_cast<object::Int *>(y)->value);
              break;
            }
            case object::FLOAT: {
              if (static_cast<object::Float *>(y)->value == 0)
                error("division by zero");
              BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                        /, static_cast<object::Float *>(y)->value);
              break;
            }
          }
        }
        break;
      }

      case byte::STORE: {
        object::Object *obj = this->popData(); // OBJECT
        ast::Type *type = this->retType();     // TO TYPE

        std::string name = this->retName(); // TO

        this->typeChecker(type, obj);

        if (top()->local.lookUp(name) != nullptr) {
          error("redefining name '" + name + "'");
        }

        top()->local.symbols[name] = obj; // store to table
        this->op += 2;
        break;
      }

      case byte::LOAD: {
        std::string name = this->retName();              // NAME
        object::Object *obj = top()->local.lookUp(name); // OBJECT

        if (obj == nullptr) error("not defined name '" + name + "'");

        this->pushData(obj);
        this->op++;
        break;
      }

      case byte::B_ARR: {
        int count = this->retOffset(); // COUNT

        object::Array *arr = new object::Array;
        // emit elements
        for (int i = 0; i < count; i++) {
          arr->elements.push_back(this->popData());
        }

        this->pushData(arr);
        this->op++;
        break;
      }

      case byte::B_TUP: {
        int count = this->retOffset(); // COUNT

        object::Tuple *tup = new object::Tuple;
        // emit elements
        for (int i = 0; i < count; i++) {
          tup->elements.push_back(this->popData());
        }

        this->pushData(tup);
        this->op++;
        break;
      }

      case byte::B_MAP: {
        int count = this->retOffset(); // COUNT

        object::Map *map = new object::Map;
        // emit elements
        for (int i = 0; i < count - 2; i++) {
          object::Object *y = this->popData();
          object::Object *x = this->popData();

          map->value.insert(std::make_pair(x, y));
        }

        this->pushData(map);
        this->op++;
        break;
      }

      case byte::RET: {
        // std::cout << top()->data.stringer() << std::endl;
        while (!top()->data.empty()) {
          std::cout << top()->data.pop()->stringer() << std::endl;
        }
      }
    }
  }
#undef BINARY_OP
}
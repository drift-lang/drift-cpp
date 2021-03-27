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

#include "vm.h"

// top frame
Frame *vm::top() { return frames.back(); }

// main frame
Frame *vm::main() { return frames.front(); }

// push object to the current frame
void vm::pushData(object::Object *obj) { top()->data.push(obj); }

// pop the top of data stack
object::Object *vm::popData() { return top()->data.pop(); }

#define PUSH(obj) this->pushData(obj)
#define POP this->popData

// emit new name of table to the current frame
void vm::emitTable(std::string name, object::Object *obj) {
  if (this->lookUp(name) != nullptr) {
    // REPLACE
    top()->tb.remove(name);
  }
  // STORE
  top()->tb.emit(name, obj);
}

// emit some objects in module to the current frame
void vm::emitModule(std::vector<object::Module *> m) {
  for (auto i : m)
    for (auto k : i->f->tb.symbols)
      this->emitTable(k.first, k.second);
}

// look up a name
object::Object *vm::lookUp(std::string n) {
  if (!top()->tb.empty() &&
      // check
      top()->tb.symbols.count(n) != 0) {
    return top()->tb.symbols.at(n); // GET
  }
  return nullptr;
}

// look up a name from main frame
object::Object *vm::lookUpMainFrame(std::string n) {
  if (!main()->tb.empty() &&
      // check
      main()->tb.symbols.count(n) != 0) {
    return main()->tb.symbols.at(n); // GET
  }
  return nullptr;
}

#define GET_OFFSET(ip) top()->entity->offsets.at(ip)

// first to end constant iterator for current frame's entity
object::Object *vm::retConstant() {
  return top()->entity->constants.at(GET_OFFSET(op));
}

// first to end
ast::Type *vm::retType() { return top()->entity->types.at(GET_OFFSET(op)); }

// first to end
std::string vm::retName() { return top()->entity->names.at(GET_OFFSET(op)); }

// first to end
int vm::retOffset() { return GET_OFFSET(op); }

// throw an exception
void vm::error(std::string message) {
  this->state->kind = exp::RUNTIME_ERROR;
  this->state->message = message;
  this->state->line = top()->entity->lineno.at(this->lp); // line no of bytecode

  throw exp::Exp(state);
}

// are the comparison types the same
void vm::typeChecker(ast::Type *x, object::Object *y) {
  switch (x->kind()) {
  // array
  case ast::T_ARRAY: {
    ast::Array *T = static_cast<ast::Array *>(x);
    object::Array *arr = static_cast<object::Array *>(y);

    for (auto i : arr->elements)
      this->typeChecker(T->T, i);
    break;
  }
  // tuple
  case ast::T_TUPLE: {
    ast::Tuple *T = static_cast<ast::Tuple *>(x);
    object::Tuple *tup = static_cast<object::Tuple *>(y);

    for (auto i : tup->elements)
      this->typeChecker(T->T, i);
    break;
  }
  // map
  case ast::T_MAP: {
    ast::Map *T = static_cast<ast::Map *>(x);
    object::Map *map = static_cast<object::Map *>(y);

    for (auto &i : map->elements) {
      this->typeChecker(T->T1, i.first);  // K
      this->typeChecker(T->T2, i.second); // R
    }
    break;
  }
  // other
  default: {
    // base
    if (
        // int
        (x->kind() == ast::T_INT && y->kind() != object::INT) ||
        // float
        (x->kind() == ast::T_FLOAT && y->kind() != object::FLOAT) ||
        // str
        (x->kind() == ast::T_STR && y->kind() != object::STR) ||
        // char
        (x->kind() == ast::T_CHAR && y->kind() != object::CHAR) ||
        // bool
        (x->kind() == ast::T_BOOL && y->kind() != object::INT)) {
      error("type error, require: " + x->stringer() +
            ", found: " + y->rawStringer());
    } else {
      // user
      if (x->kind() == ast::T_USER) {
        std::string name = static_cast<ast::User *>(x)->name.literal;

        switch (y->kind()) {
        // function
        case object::FUNC:
          if (static_cast<object::Func *>(y)->name != name)
            error("type error in store and function");
          break;
        // enum
        case object::ENUM:
          if (static_cast<object::Enum *>(y)->name != name)
            error("type error in store and enumerate");
          break;
        // whole
        case object::WHOLE:
          if (static_cast<object::Whole *>(y)->name != name)
            error("type error in store and whole statement");
          break;
        // other
        default: {
          if (this->lookUp(name) == nullptr) {
            error("not defined type '" + name + "'");
          }
        }
        }
      }
    }
  }
  }
}

// are two values of the same type equal
bool vm::objValueEquation(object::Object *x, object::Object *y) {
  switch (x->kind()) {
  case object::INT:
    return static_cast<object::Int *>(x)->value ==
           static_cast<object::Int *>(y)->value;
  case object::FLOAT:
    return static_cast<object::Float *>(x)->value ==
           static_cast<object::Float *>(y)->value;
  case object::STR:
    return static_cast<object::Str *>(x)->value ==
           static_cast<object::Str *>(y)->value;
  case object::CHAR:
    return static_cast<object::Char *>(x)->value ==
           static_cast<object::Char *>(y)->value;
  case object::BOOL:
    return static_cast<object::Bool *>(x)->value ==
           static_cast<object::Bool *>(y)->value;
  }
  return false;
}

// generate default value
object::Object *vm::setOriginalValue(ast::Type *t) {
  switch (t->kind()) {
  case ast::T_INT:
    return new object::Int(0);
  case ast::T_FLOAT:
    return new object::Float(0.0);
  case ast::T_STR:
    return new object::Str("");
  case ast::T_CHAR:
    return new object::Char(0);
  case ast::T_BOOL:
    return new object::Int(0); // default conversion

  case ast::T_ARRAY:
    return new object::Array();
  case ast::T_TUPLE:
    return new object::Tuple();
  case ast::T_MAP:
    return new object::Map();

  default:
    error("this type cannot generate a default value");
  }
  return nullptr;
}

// add counter for bytecode within jump
void vm::addCounter(int *ip, int to) {
  bool reverse = *ip > to; // condition
  // std::cout << "JUMP: " << *ip << " TO: " << to << std::endl;

  if (reverse)
    (*ip)--; // reverse skip current instruction

  while (
      /*
       * condition
       *
       * reversal needs to be judged by the jump instruction itself,
       * so its greater than or equal to
       *
       * forward, until the judgement instruction itself,
       * do not do its own offset
       *
       * forward, it processes its own offset
       */
      (reverse ? *ip >= to : *ip < to)
      //
  ) {
    switch (top()->entity->codes.at(*ip)) {
      // offset of instruction processing
    case byte::LOAD:   // 1
    case byte::CONST:  // 1
    case byte::ASSIGN: // 1
    case byte::JUMP:   // 1
    case byte::T_JUMP: // 1
    case byte::F_JUMP: // 1
    case byte::CALL:   // 1
    case byte::GET:    // 1
    case byte::SET:    // 1
    case byte::WHOLE:  // 1
    case byte::FUNC:   // 1
    case byte::NAME:   // 1
    case byte::ENUM:   // 1
    case byte::MOD:    // 1
    case byte::USE:    // 1
    {
      if (reverse) {
        // ADD
        this->op -= 1;
      } else {
        // MINUS
        this->op += 1;
      }
      break;
    }
    case byte::STORE: // 2
    case byte::NEW:   // 2
    {
      if (reverse) {
        // ADD TWO
        this->op -= 2;
      } else {
        // MINUS TWO
        this->op += 2;
      }
      break;
    }
    }
    // indexes
    reverse ? (*ip)-- : (*ip)++;
  }

  // std::cout << "OP: " << this->op << " IP: " << *ip << "+1" << std::endl;
  if (!reverse)
    (*ip)--; // for loop update
}

void vm::evaluate() { // EVALUATE

#define BINARY_OP(T, L, OP, R) PUSH(new T(L OP R));

  for (int ip = 0; ip < top()->entity->codes.size(); ip++) { // MAIN LOOP

    this->lp = ip;

    // bytecode
    byte::Code co = top()->entity->codes.at(ip);

    switch (co) {
    case byte::CONST: { // CONST
      PUSH(this->retConstant());
      this->op++;
    } break;

      //
      // BINARY OPERATOR START
      //

    case byte::ADD: { // +
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> + <Int> <Float>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, +,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Float, static_cast<object::Int *>(x)->value, +,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      }
      // <Float> + <Int> <Float>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Float, static_cast<object::Float *>(x)->value, +,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Float, static_cast<object::Float *>(x)->value, +,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      }
      // <Str> + <Str>
      else if (x->kind() == object::STR && y->kind() == object::STR) {
        object::Str *l = static_cast<object::Str *>(x);
        object::Str *r = static_cast<object::Str *>(y);

        if (l->longer || r->longer)
          error("cannot plus long string literal");

        PUSH(new object::Str(l->value + r->value));
      } else {
        // ERROR
        error("unsupport type to + operator");
      }
    } break;

    case byte::SUB: { // -
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> - <Int> <Float>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, -,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Float, static_cast<object::Int *>(x)->value, -,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      }
      // <Float> - <Int> <Float>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Float, static_cast<object::Float *>(x)->value, -,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Float, static_cast<object::Float *>(x)->value, -,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      } else {
        // ERROR
        error("unsupport type to - operator");
      }
    } break;

    case byte::MUL: { // *
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> * <Int> <Float>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, *,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Float, static_cast<object::Int *>(x)->value, *,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      }
      // <Float> * <Int> <Float>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Float, static_cast<object::Float *>(x)->value, *,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Float, static_cast<object::Float *>(x)->value, *,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      } else {
        // ERROR
        error("unsupport type to * operator");
      }
    } break;

    case byte::DIV: { // /
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> / <Int> <Float>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT: {
          if (static_cast<object::Int *>(y)->value == 0)
            error("division by zero");
          BINARY_OP(object::Float, static_cast<object::Int *>(x)->value, /,
                    static_cast<object::Int *>(y)->value);
        } break;
        //
        case object::FLOAT: {
          if (static_cast<object::Float *>(y)->value == 0)
            error("division by zero");
          BINARY_OP(object::Float, static_cast<object::Int *>(x)->value, /,
                    static_cast<object::Float *>(y)->value);
        } break;
        }
      }
      // <Float> / <Int> <Float>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT: {
          if (static_cast<object::Int *>(y)->value == 0)
            error("division by zero");
          BINARY_OP(object::Float, static_cast<object::Float *>(x)->value, /,
                    static_cast<object::Int *>(y)->value);
        } break;
        //
        case object::FLOAT: {
          if (static_cast<object::Float *>(y)->value == 0)
            error("division by zero");
          BINARY_OP(object::Float, static_cast<object::Float *>(x)->value, /,
                    static_cast<object::Float *>(y)->value);
        } break;
        }
      } else {
        // ERROR
        error("unsupport type to / operator");
      }
    } break;

    case byte::SUR: { // %
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> % <Int>
      if (x->kind() == object::INT && y->kind() == object::INT) {
        BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, %,
                  static_cast<object::Int *>(y)->value);
      } else {
        // ERROR
        error("unsupport type to % operator");
      }
    } break;

    case byte::GR: { // >
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> > <Int> <Float>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, >,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, >,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      }
      // <Float> > <Int> <Float>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, >,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, >,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      } else {
        // ERROR
        error("unsupport type to > operator");
      }
    } break;

    case byte::GR_E: { // >=
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> >= <Int> <Float>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, >=,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, >=,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      }
      // <Float> >= <Int> <Float>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, >=,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, >=,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      } else {
        // ERROR
        error("unsupport type to >= operator");
      }
    } break;

    case byte::LE: { // <
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> < <Int> <Float>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, <,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, <,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      }
      // <Float> < <Int> <Float>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, <,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, <,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      } else {
        // ERROR
        error("unsupport type to < operator");
      }
    } break;

    case byte::LE_E: { // <=
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> <= <Int> <Float>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, <=,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, <=,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      }
      // <Float> <= <Int> <Float>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, <=,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, <=,
                    static_cast<object::Float *>(y)->value);
          break;
        }
      } else {
        // ERROR
        error("unsupport type to <= operator");
      }
    } break;

    case byte::E_E: { // ==
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> == <Int> <Float> <Bool>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, ==,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, ==,
                    static_cast<object::Float *>(y)->value);
          break;
        //
        case object::BOOL: {
          int l = static_cast<object::Int *>(x)->value;
          bool r = static_cast<object::Bool *>(y)->value;

          if (l > 0 && r)
            PUSH(new object::Bool(true));
          else if (l < 0 && !r)
            PUSH(new object::Bool(true));
          else
            PUSH(new object::Bool(false));
        } break;
        }
      }
      // <Float> == <Int> <Float> <Bool>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, ==,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, ==,
                    static_cast<object::Float *>(y)->value);
          break;
        //
        case object::BOOL: {
          float l = static_cast<object::Float *>(x)->value;
          bool r = static_cast<object::Bool *>(y)->value;

          if (l > 0 && r)
            PUSH(new object::Bool(true));
          else if (l < 0 && !r)
            PUSH(new object::Bool(true));
          else
            PUSH(new object::Bool(false));
        } break;
        }
      }
      // <Bool> == <Int> <Float> <Bool>
      else if (x->kind() == object::BOOL) {
        switch (y->kind()) {
        case object::INT:
          if (static_cast<object::Int *>(y)->value > 0 &&
              static_cast<object::Bool *>(x)->value)
            PUSH(new object::Bool(true));
          else
            PUSH(new object::Bool(false));
          break;
        //
        case object::FLOAT:
          if (static_cast<object::Float *>(y)->value > 0 &&
              static_cast<object::Bool *>(x)->value)
            PUSH(new object::Bool(true));
          else
            PUSH(new object::Bool(false));
          break;
        //
        case object::BOOL:
          bool p = static_cast<object::Bool *>(x)->value;

          if (static_cast<object::Bool *>(y)->value) // RIGHT IS T
            PUSH(new object::Bool(p == true));
          else
            PUSH(new object::Bool(p == false));
          break;
        }
      }
      // <Str> == <Str>
      else if (x->kind() == object::STR && y->kind() == object::STR) {
        PUSH(new object::Bool(static_cast<object::Str *>(x)->value ==
                              static_cast<object::Str *>(y)->value));
      }
      // <Char> == <Char>
      else if (x->kind() == object::CHAR && y->kind() == object::CHAR) {
        PUSH(new object::Bool(static_cast<object::Char *>(x)->value ==
                              static_cast<object::Char *>(y)->value));
      } else {
        // ERROR
        error("unsupport type to == operator");
      }
    } break;

    case byte::N_E: { // !=
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> != <Int> <Float> <Bool>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, !=,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Int *>(x)->value, !=,
                    static_cast<object::Float *>(y)->value);
          break;
        //
        case object::BOOL: {
          int l = static_cast<object::Int *>(x)->value;
          bool r = static_cast<object::Bool *>(y)->value;

          if (l > 0 && r)
            PUSH(new object::Bool(false));
          else if (l < 0 && !r)
            PUSH(new object::Bool(false));
          else
            PUSH(new object::Bool(true));
        } break;
        }
      }
      // <Float> != <Int> <Float> <Bool>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, !=,
                    static_cast<object::Int *>(y)->value);
          break;
        //
        case object::FLOAT:
          BINARY_OP(object::Bool, static_cast<object::Float *>(x)->value, !=,
                    static_cast<object::Float *>(y)->value);
          break;
        //
        case object::BOOL: {
          float l = static_cast<object::Float *>(x)->value;
          bool r = static_cast<object::Bool *>(y)->value;

          if (l > 0 && r)
            PUSH(new object::Bool(false));
          else if (l < 0 && !r)
            PUSH(new object::Bool(false));
          else
            PUSH(new object::Bool(true));
        } break;
        }
      }
      // <Bool> != <Int> <Float> <Bool>
      else if (x->kind() == object::BOOL) {
        switch (y->kind()) {
        case object::INT:
          if (static_cast<object::Int *>(y)->value > 0 &&
              static_cast<object::Bool *>(x)->value) {
            PUSH(new object::Bool(false));
          } else {
            PUSH(new object::Bool(true));
          }
          break;
        //
        case object::FLOAT:
          if (static_cast<object::Float *>(y)->value > 0 &&
              static_cast<object::Bool *>(x)->value) {
            PUSH(new object::Bool(false));
          } else {
            PUSH(new object::Bool(true));
          }
          break;
        //
        case object::BOOL:
          if (static_cast<object::Bool *>(y)->value &&
              static_cast<object::Bool *>(x)->value) {
            PUSH(new object::Bool(false));
          } else {
            PUSH(new object::Bool(true));
          }
          break;
        }
      }
      // <Str> != <Str>
      else if (x->kind() == object::STR && y->kind() == object::STR) {
        PUSH(new object::Bool(static_cast<object::Str *>(x)->value !=
                              static_cast<object::Str *>(y)->value));
      }
      // <Char> != <Char>
      else if (x->kind() == object::CHAR && y->kind() == object::CHAR) {
        PUSH(new object::Bool(static_cast<object::Char *>(x)->value !=
                              static_cast<object::Char *>(y)->value));
      } else {
        // ERROR
        error("unsupport type to == operator");
      }
    } break;

    case byte::AND: { // &
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> & <Int> <Float> <Bool>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          if (static_cast<object::Int *>(x)->value > 0 &&
              static_cast<object::Int *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::FLOAT:
          if (static_cast<object::Int *>(x)->value > 0 &&
              static_cast<object::Float *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::BOOL:
          if (static_cast<object::Int *>(x)->value > 0 &&
              static_cast<object::Bool *>(y)->value) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        }
      }
      // <Float> & <Int> <Float> <Bool>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          if (static_cast<object::Float *>(x)->value > 0 &&
              static_cast<object::Int *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::FLOAT:
          if (static_cast<object::Float *>(x)->value > 0 &&
              static_cast<object::Float *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::BOOL:
          if (static_cast<object::Float *>(x)->value > 0 &&
              static_cast<object::Bool *>(y)->value) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        }
      }
      // <Bool> & <Int> <Float> <Bool>
      else if (x->kind() == object::BOOL) {
        switch (y->kind()) {
        case object::INT:
          if (static_cast<object::Bool *>(x)->value &&
              static_cast<object::Int *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::FLOAT:
          if (static_cast<object::Bool *>(x)->value &&
              static_cast<object::Float *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::BOOL:
          if (static_cast<object::Bool *>(x)->value &&
              static_cast<object::Bool *>(y)->value) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        }
      }
      // ERROR
      else {
        error("only number and boolean type to & operator");
      }
    } break;

    case byte::OR: { // |
      object::Object *y = POP();
      object::Object *x = POP();

      // <Int> & <Int> <Float> <Bool>
      if (x->kind() == object::INT) {
        switch (y->kind()) {
        case object::INT:
          if (static_cast<object::Int *>(x)->value > 0 ||
              static_cast<object::Int *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::FLOAT:
          if (static_cast<object::Int *>(x)->value > 0 ||
              static_cast<object::Float *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::BOOL:
          if (static_cast<object::Int *>(x)->value > 0 ||
              static_cast<object::Bool *>(y)->value) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        }
      }
      // <Float> & <Int> <Float> <Bool>
      else if (x->kind() == object::FLOAT) {
        switch (y->kind()) {
        case object::INT:
          if (static_cast<object::Float *>(x)->value > 0 ||
              static_cast<object::Int *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::FLOAT:
          if (static_cast<object::Float *>(x)->value > 0 ||
              static_cast<object::Float *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::BOOL:
          if (static_cast<object::Float *>(x)->value > 0 ||
              static_cast<object::Bool *>(y)->value) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        }
      }
      // <Bool> & <Int> <Float> <Bool>
      else if (x->kind() == object::BOOL) {
        switch (y->kind()) {
        case object::INT:
          if (static_cast<object::Bool *>(x)->value ||
              static_cast<object::Int *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::FLOAT:
          if (static_cast<object::Bool *>(x)->value ||
              static_cast<object::Float *>(y)->value > 0) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        //
        case object::BOOL:
          if (static_cast<object::Bool *>(x)->value ||
              static_cast<object::Bool *>(y)->value) {
            PUSH(new object::Bool(true));
          } else {
            PUSH(new object::Bool(false));
          }
          break;
        }
      }
      // ERROR
      else {
        error("only number and boolean type to | operator");
      }
    } break;

      //
      // BINARY OPERATOR END
      //

    case byte::BANG: { // !
      object::Object *obj = POP();

      // !<Int> <Float> <Bool>
      if (obj->kind() == object::INT) {
        PUSH(static_cast<object::Int *>(obj)->value ? new object::Bool(false)
                                                    : new object::Bool(true));
        //
      } else if (obj->kind() == object::FLOAT) {
        PUSH(static_cast<object::Float *>(obj)->value ? new object::Bool(false)
                                                      : new object::Bool(true));
        //
      } else if (obj->kind() == object::BOOL) {
        PUSH(static_cast<object::Bool *>(obj)->value ? new object::Bool(false)
                                                     : new object::Bool(true));
        //
      } else {
        error("only number and boolean type to bang operator");
      }
    } break;

    case byte::NOT: { // -
      object::Object *obj = POP();

      if (obj->kind() != object::INT && obj->kind() != object::FLOAT)
        error("only number type to unary operator");

      if (obj->kind() == object::FLOAT) {
        PUSH(new object::Float(-static_cast<object::Float *>(obj)->value));
        break;
      }

      PUSH(new object::Int(-static_cast<object::Int *>(obj)->value));
    } break;

    case byte::STORE: { // STORE
      object::Object *obj;
      std::string name = this->retName(); // TO NAME

      this->op++;
      ast::Type *type = this->retType(); // TO TYPE

      if (top()->entity->codes.at(ip - 1) == byte::ORIG) { // ORIGINAL
        obj = this->setOriginalValue(type);                // VALUE
      } else {
        obj = POP(); // OBJECT
      }

      if (type->kind() == ast::T_BOOL) {
        // transfrom integer to boolean
        obj = new object::Bool(static_cast<object::Int *>(obj)->value);
      } else {
        this->typeChecker(type, obj); // TYPE CHECKER
      }

      this->emitTable(name, obj); // STORE
      this->op++;
    } break;

    case byte::LOAD: {
      std::string name = this->retName(); // NAME
      // std::cout << "LOAD: " << name << std::endl;

      // LOAD BUILTIN
      if (isBuiltinName(name)) {
        object::Func *f = new object::Func;
        f->name = name;

        PUSH(f);
        this->op++;
        break;
      }

      object::Object *obj = this->lookUp(name); // OBJECT
      // std::cout << "L: " << obj << " NAME: " << name << std::endl;

      if (obj == nullptr) {
        // LOAD SUBCLASS FUNCTION
        if (this->callWholeMethod && this->callWhole != nullptr) {
          for (auto i : this->callWhole->inherit) {
            object::Whole *w =
                static_cast<object::Whole *>(this->lookUpMainFrame(i));

            obj = w->f->tb.lookUp(name);

            if (obj == nullptr)
              error("not defined name '" + name + "'");
            if (obj->kind() != object::FUNC)
              error("only parent class methods can be called");
          }
        } else
          error("not defined name '" + name + "'");
      }

      PUSH(obj);
      this->op++;
    } break;

    case byte::B_ARR: {
      int count = this->retOffset(); // COUNT

      object::Array *arr = new object::Array;
      // emit elements
      for (int i = 0; i < count; i++) {
        arr->elements.push_back(POP());
      }

      PUSH(arr);
      this->op++;
    } break;

    case byte::B_TUP: {
      int count = this->retOffset(); // COUNT

      object::Tuple *tup = new object::Tuple;
      // emit elements
      for (int i = 0; i < count; i++) {
        tup->elements.push_back(POP());
      }

      PUSH(tup);
      this->op++;
    } break;

    case byte::B_MAP: {
      int count = this->retOffset(); // COUNT

      object::Map *map = new object::Map;
      // emit elements
      for (int i = 0; i < count - 2; i++) {
        object::Object *y = POP();
        object::Object *x = POP();

        map->elements.insert(std::make_pair(x, y));
      }

      PUSH(map);
      this->op++;
    } break;

    case byte::ASSIGN: {
      std::string name = this->retName(); // NAME
      object::Object *obj = POP();        // OBJ

      // std::cout << "ASS: " << name << " OBJ: " << obj->stringer()
      //           << std::endl;
      if (this->lookUp(name) == nullptr)
        error("not defined name '" + name + "'");

      this->emitTable(name, obj); // STORE
      this->op++;
    } break;

    case byte::JUMP: // JUMP

    case byte::F_JUMP:
    case byte::T_JUMP: {
      int off = this->retOffset(); // TO

      if (co == byte::JUMP && this->loopWasRet && off < ip) {
        this->loopWasRet = false;
        this->op++;
        break;
      }

      // JUMP
      if (co == byte::JUMP) {
        this->addCounter(&ip, off);
        //
      } else {
        // T
        if (static_cast<object::Bool *>(POP())->value) {
          if (co == byte::T_JUMP) {
            //
            this->addCounter(&ip, off); // T_JUMP
          } else {
            this->op++; // NO
          }
        } else {
          // F
          if (co == byte::F_JUMP) {
            //
            this->addCounter(&ip, off); // F_JUMP
          } else {
            this->op++; // NO
          }
        }
      }
    } break;

    case byte::FUNC: { // FUNCTION
      object::Func *f =
          static_cast<object::Func *>(this->retConstant()); // OBJECT
      this->emitTable(f->name, f);                          // STORE
      this->op++;
    } break;

    case byte::CALL: { // CALL FUNCTION
      int args = this->retOffset();

      Stack<object::Object *> arguments;
      while (args-- > 0 && top()->data.len() != 1) { // TOP IS FUNC OBJ
        arguments.push(POP());                       // ARGUMENT
      }

      // std::cout << "ARGS: " << arguments.len() << std::endl;

      object::Func *f = static_cast<object::Func *>(POP()); // FUNCTION
      // std::cout << "CALL: " << f->name << std::endl;

      if (isBuiltinName(f->name)) {
        while (arguments.len()) {
          f->builtin.push_back(arguments.pop()); // BUILTIN ARGUMENTS
        }
        builtinFuncCall(f->name, f, top()); // TO BUILTIN CALL

        this->op++;
        break;
      }

      /* if (disMode)
        f->entity->dissemble(); */

      Frame *fra = new Frame(f->entity); // FRAME

      if (f->arguments.size() != arguments.len())
        error("wrong number of parameters");

      // SET TABLE SYMBOL
      if (this->callWholeMethod)
        // std::cout << "CALL " << (this->callWhole->name) << std::endl;

        // CALL WHOLE
        fra->tb = this->callWhole->f->tb;
      else
        // GLOBAL
        fra->tb.symbols = top()->tb.symbols;

      // std::cout << "CALL: " << f->name << " " <<
      // fra->tb.symbols.size()
      //           << std::endl;
      // std::cout << "CALL ARGS: " << arguments.len() << std::endl;

      // ARGUMENT
      for (std::map<token::Token *, ast::Type *>::reverse_iterator iter =
               f->arguments.rbegin();
           //  REVERSE EMIT
           iter != f->arguments.rend(); iter++) {
        object::Object *val = arguments.pop(); // OBJECT

        this->typeChecker(iter->second, val);    // TYPE CHECKER
        fra->tb.emit(iter->first->literal, val); // STORE
      }

      int t = this->op; // TEMP OFFSET

      this->op = 0;
      this->frames.push_back(fra); // NEW FRAME
      this->evaluate();

      this->frames.pop_back(); // POP

      if (f->ret != nullptr) {
        // RETURN
        if (top()->ret == nullptr)
          error("missing return value");
        // TYPE CHECKER
        this->typeChecker(f->ret, top()->ret);
        PUSH(top()->ret); // PUSH

        top()->ret = nullptr;
      }

      if (f->ret == nullptr && top()->ret != nullptr) {
        error("function does not define return value, but it has "
              "return value");
      }

      // CALL WHOLE METHOD
      if (this->callWholeMethod) {
        // CLEAR
        this->callWholeMethod = false;
        this->callWhole = nullptr;
      }

      this->op = ++t; // NEXT
    } break;

    case byte::INDEX: { // INDEX
      object::Object *obj = POP();
      object::Object *idx = POP();

      // GET
      switch (obj->kind()) {
      case object::ARRAY: {
        if (idx->kind() != object::INT) {
          error("array subscript index can only be an integer");
        }

        auto x = static_cast<object::Int *>(idx);   // INDEX
        auto y = static_cast<object::Array *>(obj); // TO

        if (y->elements.empty())
          error("empty element of array");
        if (x->value >= y->elements.size()) {
          error("array out of bounds, index: " + std::to_string(x->value) +
                " max: " + std::to_string(y->elements.size() - 1));
        }
        PUSH(y->elements.at(x->value)); // PUSH
      } break;
      //
      case object::MAP: {
        object::Map *m = static_cast<object::Map *>(obj);

        if (m->elements.empty())
          error("empty element of map");
        if (m->elements.begin()->first->kind() != idx->kind()) {
          error("wrong key index");
        }

        std::map<object::Object *, object::Object *>::iterator iter; // ITERATOR
        for (iter = m->elements.begin();
             // K TO IDX
             iter != m->elements.end() &&
             !this->objValueEquation(iter->first, idx);
             *iter++)
          ;
        if (iter == m->elements.end()) { // ERROR
          error("map does not have this key: " + idx->stringer());
        }

        PUSH(iter->second); // PUSH
      } break;
      //
      case object::STR: {
        if (idx->kind() != object::INT) {
          error("string subscript index can only be an integer");
        }

        object::Str *s = static_cast<object::Str *>(obj);
        object::Int *i = static_cast<object::Int *>(idx);

        // INDEX OUT
        if (i->value >= s->value.size() || s->value.empty()) {
          error("string out of bounds, index: " + std::to_string(i->value) +
                " max: " + std::to_string(s->value.size() - 1));
        }

        PUSH(new object::Char(s->value.at(i->value)));
      } break;
      }
    } break;

    case byte::REPLACE: { // REPLACE
      object::Object *obj = POP();
      object::Object *idx = POP();
      object::Object *val = POP();

      // SET
      switch (obj->kind()) {
      case object::ARRAY: {
        if (idx->kind() != object::INT) {
          error("array subscript index can only be an integer");
        }

        object::Array *a = static_cast<object::Array *>(obj);
        int i = static_cast<object::Int *>(idx)->value;

        if (i >= a->elements.size()) {
          error("array out of bounds, index: " + std::to_string(i) +
                " max: " + std::to_string(a->elements.size() - 1));
        }

        // REPLACE
        std::replace(std::begin(a->elements), std::end(a->elements),
                     a->elements.at(i), val);

        // RESTORE
        if (top()->entity->codes.at(ip) == byte::LOAD) {
          this->emitTable(
              // NAME
              top()->entity->names.back(),
              // VALUE
              a);
        }
      } break;
      //
      case object::MAP: {
        object::Map *m = static_cast<object::Map *>(obj);

        std::map<object::Object *, object::Object *>::iterator iter; // ITERATOR
        for (iter = m->elements.begin();
             // K TO IDX
             iter != m->elements.end() &&
             !this->objValueEquation(iter->first, idx);
             *iter++)
          ;
        if (iter == m->elements.end()) { // NOT FOUND
          // INSERT
          m->elements.insert(std::make_pair(idx, val));
        } else {
          // REPLACE
          iter->second = val;
        }

        // RESTORE
        if (top()->entity->codes.at(ip) == byte::LOAD) {
          this->emitTable(
              // NAME
              top()->entity->names.back(),
              // VALUE
              m);
        }
      } break;
        //
      }
    } break;

    case byte::GET: { // GET
      std::string name = this->retName();
      object::Object *obj = POP();

      switch (obj->kind()) {
      case object::TUPLE: {
        object::Tuple *t = static_cast<object::Tuple *>(obj);

        int i;

        if (isNumberStr(name)) { // DIGITAL INDEX
          i = atoi(name.c_str());
        } else {
          // VAR
          object::Object *o = this->lookUp(name);

          if (o->kind() != object::INT) {
            error("index can only be of integer type");
          }
          i = static_cast<object::Int *>(o)->value;
        }

        if (i >= t->elements.size()) {
          error("tuple out of bounds, index: " + std::to_string(i) +
                " max: " + std::to_string(t->elements.size() - 1));
        }
        PUSH(t->elements.at(i));
      } break;
      //
      case object::ENUM: {
        object::Enum *e = static_cast<object::Enum *>(obj);

        std::map<int, std::string>::iterator iter =
            e->elements.begin(); // ITERATOR
        for (; iter != e->elements.end() && iter->second != name; *iter++)
          ;
        if (iter == e->elements.end()) {
          error("nonexistent member '" + name + "'");
        }

        PUSH(new object::Int(iter->first));
      } break;
      //
      case object::WHOLE: {
        object::Whole *w = static_cast<object::Whole *>(obj);
        // std::cout << "G: " << w << " NAME: " << name << std::endl;

        if (!w->newOut)
          error("should new one first");

        // GET TO
        object::Object *op = w->f->tb.lookUp(name);

        if (op == nullptr)
          error("nonexistent member '" + name + "'");

        if (op->kind() == object::FUNC) {
          this->callWholeMethod = true;
          this->callWhole = w; // CALL WHOLE METHOD
        }

        PUSH(op);
      } break;
      default:
        error("nonexistent member '" + name + "'");
      }
      this->op++;
    } break;

    case byte::SET: { // SET
      object::Object *w = POP();
      std::string name = this->retName(); // NAME

      if (w->kind() != object::WHOLE)
        error("the value type is not whole object");

      object::Whole *n = static_cast<object::Whole *>(w);
      // std::cout << "S: " << n << " NAME: " << name << std::endl;

      if (!n->newOut)
        error("should new one first");
      if (n->f->tb.lookUp(name) == nullptr)
        error("no member '" + name + "' to set");

      n->f->tb.emit(name, POP()); // SET

      this->op++;
    } break;

    case byte::ENUM: { // ENUM
      object::Enum *e =
          static_cast<object::Enum *>(this->retConstant()); // OBJECT
      this->emitTable(e->name, e);                          // STORE
      this->op++;
    } break;

    case byte::WHOLE: { // WHOLE
      object::Whole *w =
          static_cast<object::Whole *>(this->retConstant()); // OBJECT

      /* if (this->disMode)
        w->entity->dissemble(); */

      this->emitTable(w->name, w); // STORE
      this->op++;
    } break;

    case byte::NAME: { // NAME
      PUSH(new object::Str(this->retName()));
      this->op++;
    } break;

    case byte::NEW: { // NEW
      std::string name = this->retName();

      this->op++;
      int count = this->retOffset(); // COUNT

      object::Object *obj = this->lookUp(name); // OBJECT
      if (obj == nullptr || obj->kind() != object::WHOLE)
        error("not defined whole of '" + name + "'");

      int size = sizeof(object::Whole); // 112
                                        // std::cout << size << std::endl;

      object::Whole *r = static_cast<object::Whole *>(obj);
      object::Whole *w = (object::Whole *)malloc(size);

      memcpy(w, r, size); // MEM COPY
      // std::cout << "N: " << w << " <- " << r << std::endl;

      // EVALUATE IT
      w->f = new Frame(w->entity);

      int t = this->op; // TEMP OFFSET

      this->op = 0;
      this->frames.push_back(w->f); // GO
      this->evaluate();

      this->frames.pop_back(); // POP

      // SET CONSTRUCTOR
      while (count > 0) {
        object::Object *v = POP();
        object::Object *k = POP();
        // STORE
        w->f->tb.emit(static_cast<object::Str *>(k)->value, v);
        count -= 2;
      }

      // INHERIT
      if (!w->inherit.empty()) {
        for (auto i : w->inherit) {

          object::Object *obj = this->lookUp(i);

          if (obj == nullptr)
            error("inheritance '" + i + "' dose not exist");
          if (obj->kind() != object::WHOLE)
            error("only whole object can be inherited");

          // INTERFACE
          object::Whole *it = static_cast<object::Whole *>(obj);

          for (std::tuple<std::string, ast::FaceArg, ast::Type *> i :
               it->interface) {
            // std::cout << std::get<0>(i) << std::endl;

            // TO TABLE SYMBOL
            std::map<std::string, object::Object *>::iterator iter;

            // std::cout << w->f->tb.symbols.size() << std::endl;

            for (iter = w->f->tb.symbols.begin();
                 iter != w->f->tb.symbols.end(); iter++) {
              //  NAME
              if (iter->first == std::get<0>(i)) {
                // std::cout << "NAME: " << iter->first <<
                // std::endl;
                //
                if (iter->second->kind() != object::FUNC) {
                  error("subclass inheritance is not "
                        "function");
                }

                object::Func *f = static_cast<object::Func *>(iter->second);

                // RETURN
                if (f->ret->kind() != std::get<2>(i)->kind()) {
                  error("bad return type for subclass "
                        "inheritance");
                }
                // ARGUMENT
                if (std::get<1>(i).size() != f->arguments.size()) {
                  error("inconsistent arguments for "
                        "subclass inheritance");
                }
                // TYPE
                auto x = std::get<1>(i).begin(); // T
                auto y = f->arguments.begin();   // K, T

                while (y != f->arguments.end()) {
                  //
                  if ((*x)->kind() != y->second->kind()) {
                    error("subclass inheritance "
                          "parameter " +
                          y->second->stringer() + " no " + (*x)->stringer());
                  }
                  *x++; // NEXT
                  *y++; // NEXT
                }
                break;
              };
            }

            // NOT FOUND
            if (iter == w->f->tb.symbols.end()) {
              error("not inherited method '" + std::get<0>(i) +
                    "' of subclass");
            }
          }
        }
      }

      w->newOut = true; // TO NEW

      PUSH(w);        // PUSH
      this->op = ++t; // NEXT
    } break;

    case byte::MOD: { // MOD
      top()->mod = this->retName();
      this->op++;
    } break;

    case byte::USE: { // USE
      std::string name = this->retName();
      std::vector<object::Module *> m = getModule(this->mods, name);

      if (m.empty())
        error("not defined module '" + name + "'");

      // STORE
      this->emitModule(m);
      this->op++;
    } break;

    case byte::RET_N: // RET NONE

    case byte::RET: { // RETURN
      if (top()->entity->title != "main") {
        // TO PREVIOUS FRAME
        if (co != byte::RET_N)
          this->frames.at(this->frames.size() - 2)->ret = POP(); // VALUE
        ip = top()->entity->codes.size();                        // CATCH
      }

      // loop exit and no return value return
      if (co == byte::RET_N)
        this->loopWasRet = true;

      // MODULE
      if (!top()->mod.empty()) {
        std::string *errs = addModule(this->mods, top()->mod, top());
        if (errs != nullptr) {
          error(*errs); // if have some errors to throw
        }
      }

      if (this->replMode) {
        for (int i = 0; i < top()->data.len(); i++) {
          std::cout << top()->data.at(i)->stringer() << std::endl;
        }
        top()->data.clear();
      }
    } break;
    }
  }
#undef BINARY_OP
}
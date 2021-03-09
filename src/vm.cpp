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
        (x->kind() == ast::T_CHAR && y->kind() != object::CHAR) ||
        // bool
        (x->kind() == ast::T_BOOL && y->kind() != object::INT)) {
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

                //
                // BINARY OPERATOR START
                //

            case byte::ADD: { // +
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> + <Int> <Float>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Int,
                                      static_cast<object::Int *>(x)->value, +,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Float,
                                      static_cast<object::Int *>(x)->value, +,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                }
                // <Float> + <Int> <Float>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Float,
                                      static_cast<object::Float *>(x)->value, +,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Float,
                                      static_cast<object::Float *>(x)->value, +,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                }
                // <Str> + <Str>
                else if (x->kind() == object::STR && y->kind() == object::STR) {
                    object::Str *l = static_cast<object::Str *>(x);
                    object::Str *r = static_cast<object::Str *>(y);

                    if (l->longer || r->longer) {
                        error("cannot plus long string literal");
                    }

                    this->pushData(new object::Str(l->value + r->value));
                } else {
                    // ERROR
                    error("unsupport type to + operator");
                }
                break;
            }
            case byte::SUB: { // -
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> - <Int> <Float>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Int,
                                      static_cast<object::Int *>(x)->value, -,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Float,
                                      static_cast<object::Int *>(x)->value, -,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                }
                // <Float> - <Int> <Float>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Float,
                                      static_cast<object::Float *>(x)->value, -,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Float,
                                      static_cast<object::Float *>(x)->value, -,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                } else {
                    // ERROR
                    error("unsupport type to - operator");
                }
                break;
            }
            case byte::MUL: { // *
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> * <Int> <Float>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Int,
                                      static_cast<object::Int *>(x)->value, *,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Float,
                                      static_cast<object::Int *>(x)->value, *,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                }
                // <Float> * <Int> <Float>
                else if (x->kind() == object::FLOAT) {
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
                } else {
                    // ERROR
                    error("unsupport type to * operator");
                }
                break;
            }
            case byte::DIV: { // /
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> / <Int> <Float>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Int *>(y)->value == 0)
                                error("division by zero");
                            BINARY_OP(object::Float,
                                      static_cast<object::Int *>(x)->value, /,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Float *>(y)->value == 0)
                                error("division by zero");
                            BINARY_OP(object::Float,
                                      static_cast<object::Int *>(x)->value, /,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                }
                // <Float> / <Int> <Float>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Int *>(y)->value == 0)
                                error("division by zero");
                            BINARY_OP(object::Float,
                                      static_cast<object::Float *>(x)->value, /,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Float *>(y)->value == 0)
                                error("division by zero");
                            BINARY_OP(object::Float,
                                      static_cast<object::Float *>(x)->value, /,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                } else {
                    // ERROR
                    error("unsupport type to / operator");
                }
                break;
            }

            case byte::GR: { // >
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> > <Int> <Float>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, >,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, >,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                }
                // <Float> > <Int> <Float>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value, >,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value, >,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                } else {
                    // ERROR
                    error("unsupport type to > operator");
                }
                break;
            }

            case byte::GR_E: { // >=
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> >= <Int> <Float>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, >=,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, >=,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                }
                // <Float> >= <Int> <Float>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value,
                                      >=, static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value, >=
                                      , static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                } else {
                    // ERROR
                    error("unsupport type to >= operator");
                }
                break;
            }

            case byte::LE: { // <
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> < <Int> <Float>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, <,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, <,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                }
                // <Float> < <Int> <Float>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value, <,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value, <,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                } else {
                    // ERROR
                    error("unsupport type to < operator");
                }
                break;
            }

            case byte::LE_E: { // <=
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> <= <Int> <Float>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, <=,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, <=,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                }
                // <Float> <= <Int> <Float>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value,
                                      <=, static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value, <=
                                      , static_cast<object::Float *>(y)->value);
                            break;
                        }
                    }
                } else {
                    // ERROR
                    error("unsupport type to <= operator");
                }
                break;
            }

            case byte::E_E: { // ==
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> == <Int> <Float> <Bool>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, ==,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, ==,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                        case object::BOOL: {
                            int l = static_cast<object::Int *>(x)->value;
                            bool r = static_cast<object::Bool *>(y)->value;

                            if (l > 0 && r) {
                                this->pushData(new object::Bool(true));
                            } else if (l < 0 && !r) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                    }
                }
                // <Float> == <Int> <Float> <Bool>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value,
                                      ==, static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value, ==
                                      , static_cast<object::Float *>(y)->value);
                            break;
                        }
                        case object::BOOL: {
                            float l = static_cast<object::Float *>(x)->value;
                            bool r = static_cast<object::Bool *>(y)->value;

                            if (l > 0 && r) {
                                this->pushData(new object::Bool(true));
                            } else if (l < 0 && !r) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                    }
                }
                // <Bool> == <Int> <Float> <Bool>
                else if (x->kind() == object::BOOL) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Int *>(y)->value > 0 &&
                                static_cast<object::Bool *>(x)->value) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Float *>(y)->value > 0 &&
                                static_cast<object::Bool *>(x)->value) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::BOOL: {
                            if (static_cast<object::Bool *>(y)->value &&
                                static_cast<object::Bool *>(x)->value) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                    }
                }
                // <Str> == <Str>
                else if (x->kind() == object::STR && y->kind() == object::STR) {
                    this->pushData(
                        new object::Bool(static_cast<object::Str *>(x)->value ==
                                         static_cast<object::Str *>(y)->value));
                }
                // <Char> == <Char>
                else if (x->kind() == object::CHAR &&
                         y->kind() == object::CHAR) {
                    this->pushData(new object::Bool(
                        static_cast<object::Char *>(x)->value ==
                        static_cast<object::Char *>(y)->value));
                } else {
                    // ERROR
                    error("unsupport type to == operator");
                }
                break;
            }

            case byte::N_E: { // !=
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> != <Int> <Float> <Bool>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, !=,
                                      static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Int *>(x)->value, !=,
                                      static_cast<object::Float *>(y)->value);
                            break;
                        }
                        case object::BOOL: {
                            int l = static_cast<object::Int *>(x)->value;
                            bool r = static_cast<object::Bool *>(y)->value;

                            if (l > 0 && r) {
                                this->pushData(new object::Bool(false));
                            } else if (l < 0 && !r) {
                                this->pushData(new object::Bool(false));
                            } else {
                                this->pushData(new object::Bool(true));
                            }
                            break;
                        }
                    }
                }
                // <Float> != <Int> <Float> <Bool>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value,
                                      !=, static_cast<object::Int *>(y)->value);
                            break;
                        }
                        case object::FLOAT: {
                            BINARY_OP(object::Bool,
                                      static_cast<object::Float *>(x)->value, !=
                                      , static_cast<object::Float *>(y)->value);
                            break;
                        }
                        case object::BOOL: {
                            float l = static_cast<object::Float *>(x)->value;
                            bool r = static_cast<object::Bool *>(y)->value;

                            if (l > 0 && r) {
                                this->pushData(new object::Bool(false));
                            } else if (l < 0 && !r) {
                                this->pushData(new object::Bool(false));
                            } else {
                                this->pushData(new object::Bool(true));
                            }
                            break;
                        }
                    }
                }
                // <Bool> != <Int> <Float> <Bool>
                else if (x->kind() == object::BOOL) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Int *>(y)->value > 0 &&
                                static_cast<object::Bool *>(x)->value) {
                                this->pushData(new object::Bool(false));
                            } else {
                                this->pushData(new object::Bool(true));
                            }
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Float *>(y)->value > 0 &&
                                static_cast<object::Bool *>(x)->value) {
                                this->pushData(new object::Bool(false));
                            } else {
                                this->pushData(new object::Bool(true));
                            }
                            break;
                        }
                        case object::BOOL: {
                            if (static_cast<object::Bool *>(y)->value &&
                                static_cast<object::Bool *>(x)->value) {
                                this->pushData(new object::Bool(false));
                            } else {
                                this->pushData(new object::Bool(true));
                            }
                            break;
                        }
                    }
                }
                // <Str> != <Str>
                else if (x->kind() == object::STR && y->kind() == object::STR) {
                    this->pushData(
                        new object::Bool(static_cast<object::Str *>(x)->value !=
                                         static_cast<object::Str *>(y)->value));
                }
                // <Char> != <Char>
                else if (x->kind() == object::CHAR &&
                         y->kind() == object::CHAR) {
                    this->pushData(new object::Bool(
                        static_cast<object::Char *>(x)->value !=
                        static_cast<object::Char *>(y)->value));
                } else {
                    // ERROR
                    error("unsupport type to == operator");
                }
                break;
            }

            case byte::AND: { // &
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> & <Int> <Float> <Bool>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Int *>(x)->value > 0 &&
                                static_cast<object::Int *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Int *>(x)->value > 0 &&
                                static_cast<object::Float *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::BOOL: {
                            if (static_cast<object::Int *>(x)->value > 0 &&
                                static_cast<object::Bool *>(y)->value) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                    }
                }
                // <Float> & <Int> <Float> <Bool>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Float *>(x)->value > 0 &&
                                static_cast<object::Int *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Float *>(x)->value > 0 &&
                                static_cast<object::Float *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::BOOL: {
                            if (static_cast<object::Float *>(x)->value > 0 &&
                                static_cast<object::Bool *>(y)->value) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                    }
                }
                // <Bool> & <Int> <Float> <Bool>
                else if (x->kind() == object::BOOL) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Bool *>(x)->value &&
                                static_cast<object::Int *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Bool *>(x)->value &&
                                static_cast<object::Float *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::BOOL: {
                            if (static_cast<object::Bool *>(x)->value &&
                                static_cast<object::Bool *>(y)->value) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                    }
                }
                // ERROR
                else {
                    error("only number and boolean type to & operator");
                }
                break;
            }

            case byte::OR: { // |
                object::Object *y = this->popData();
                object::Object *x = this->popData();

                // <Int> & <Int> <Float> <Bool>
                if (x->kind() == object::INT) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Int *>(x)->value > 0 ||
                                static_cast<object::Int *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Int *>(x)->value > 0 ||
                                static_cast<object::Float *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::BOOL: {
                            if (static_cast<object::Int *>(x)->value > 0 ||
                                static_cast<object::Bool *>(y)->value) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                    }
                }
                // <Float> & <Int> <Float> <Bool>
                else if (x->kind() == object::FLOAT) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Float *>(x)->value > 0 ||
                                static_cast<object::Int *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Float *>(x)->value > 0 ||
                                static_cast<object::Float *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::BOOL: {
                            if (static_cast<object::Float *>(x)->value > 0 ||
                                static_cast<object::Bool *>(y)->value) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                    }
                }
                // <Bool> & <Int> <Float> <Bool>
                else if (x->kind() == object::BOOL) {
                    switch (y->kind()) {
                        case object::INT: {
                            if (static_cast<object::Bool *>(x)->value ||
                                static_cast<object::Int *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::FLOAT: {
                            if (static_cast<object::Bool *>(x)->value ||
                                static_cast<object::Float *>(y)->value > 0) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                        case object::BOOL: {
                            if (static_cast<object::Bool *>(x)->value ||
                                static_cast<object::Bool *>(y)->value) {
                                this->pushData(new object::Bool(true));
                            } else {
                                this->pushData(new object::Bool(false));
                            }
                            break;
                        }
                    }
                }
                // ERROR
                else {
                    error("only number and boolean type to | operator");
                }
                break;
            }

                //
                // BINARY OPERATOR END
                //

            case byte::BANG: { // !
                object::Object *obj = this->popData();

                // !<Int> <Float> <Bool>
                if (obj->kind() == object::INT) {
                    this->pushData(static_cast<object::Int *>(obj)->value
                                       ? new object::Bool(false)
                                       : new object::Bool(true));
                    //
                } else if (obj->kind() == object::FLOAT) {
                    this->pushData(static_cast<object::Float *>(obj)->value
                                       ? new object::Bool(false)
                                       : new object::Bool(true));
                    //
                } else if (obj->kind() == object::BOOL) {
                    this->pushData(static_cast<object::Bool *>(obj)->value
                                       ? new object::Bool(false)
                                       : new object::Bool(true));
                    //
                } else {
                    error("only number and boolean type to bang operator");
                }

                this->op++;
                break;
            }

            case byte::NOT: { // -
                object::Object *obj = this->popData();

                if (obj->kind() != object::INT &&
                    obj->kind() != object::FLOAT) {
                    error("only number type to unary operator");
                }

                this->pushData(
                    new object::Int(-static_cast<object::Int *>(obj)->value));
                break;
            }

            case byte::STORE: {
                object::Object *obj = this->popData(); // OBJECT
                ast::Type *type = this->retType();     // TO TYPE

                std::string name = this->retName(); // TO NAME

                this->typeChecker(type, obj);

                if (top()->local.lookUp(name) != nullptr) {
                    error("redefining name '" + name + "'");
                }

                if (type->kind() == ast::T_BOOL) {
                    // transfrom integer to boolean
                    obj = new object::Bool(
                        static_cast<object::Int *>(obj)->value);
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

            case byte::ASSIGN: {
                std::string name = this->retName();
                object::Object *obj = this->popData();

                top()->local.symbols[name] = obj;
                this->op++;
                break;
            }

            case byte::RET: {
                if (!top()->data.empty()) {
                    for (int i = 0; i < top()->data.len(); i++) {
                        std::cout << top()->data.at(i)->stringer() << std::endl;
                    }
                }
                top()->data.clear();
            }
        }
    }
#undef BINARY_OP
}
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

#ifndef DRIFT_AST_H
#define DRIFT_AST_H

#include <vector>

#include "token.h"

// abstract syntax tree
namespace ast {
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

  explicit Array(Type *T) : T(T) {}

  std::string stringer() override { return "<Array T=" + T->stringer() + " >"; }

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
    return "<Map T1=" + T1->stringer() + " T2=" + T2->stringer() + " >";
  }

  TypeKind kind() override { return T_MAP; }
};

// tuple (not keyword, for compiler analysis)
// (type)
class Tuple : public Type {
public:
  Type *T; // type for elements

  explicit Tuple(Type *T) : T(T) {}

  std::string stringer() override { return "<Tuple T=" + T->stringer() + " >"; }

  TypeKind kind() override { return T_TUPLE; }
};

// user definition type
// `type`
class User : public Type {
public:
  token::Token name;

  explicit User(token::Token name) { this->name = std::move(name); }

  std::string stringer() override {
    return "<User Name='" + name.literal + "' >";
  }

  TypeKind kind() override { return T_USER; }
};

// ast types
enum Kind {
  // expression
  EXPR_LITERAL, // literal
  EXPR_BINARY,  // T1 <OP> T2
  EXPR_GROUP,   // (EXPR)
  EXPR_UNARY,   // <OP>EXPR
  EXPR_NAME,    // IDENT
  EXPR_CALL,    // EXPR(<EXPR>..)
  EXPR_GET,     // EXPR.NAME
  EXPR_SET,     // EXPR.NAME = EXPR
  EXPR_ASSIGN,  // EXPR = EXPR
  EXPR_ARRAY,   // [<EXPR>..]
  EXPR_MAP,     // {K1: V1, K2: V2}
  EXPR_TUPLE,   // (<EXPR>..)
  EXPR_INDEX,   // EXPR[EXPR]
  EXPR_NEW,
  // statement
  STMT_EXPR,     // EXPR
  STMT_VAR,      // VAR
  STMT_BLOCK,    // BLOCK
  STMT_IF,       // IF
  STMT_FOR,      // FOR
  STMT_DO,       // DO
  STMT_OUT,      // OUT
  STMT_TIN,      // TIN
  STMT_FUNC,     // FUNC
  STMT_WHOLE,    // CLASS | ENUM
  STMT_AND,      // AND
  STMT_MOD,      // MOD
  STMT_USE,      // USE
  STMT_RET,      // RET
  STMT_ENUM,     // ENUM
  STMT_INHERIT,  // <- <name> + <name>..
  STMT_INTERFACE // INTERFACE
};

// K1: V1 | K1 + K2: V2
using FuncArg = std::map<token::Token *, Type *>;
// T1, T2..
using FaceArg = std::vector<Type *>;

// abstract expr
class Expr {
public:
  // return string of expr
  virtual std::string stringer() = 0;
  // return kind of expr
  virtual Kind kind() = 0;
};

// number | string | char
class LiteralExpr : public Expr {
public:
  // literal
  token::Token token;

  explicit LiteralExpr(token::Token tok) { this->token = std::move(tok); }

  std::string stringer() override {
    return "<LiteralExpr { Token='" + token.literal + "' }>";
  }

  Kind kind() override { return EXPR_LITERAL; }
};

// T1 <OP> T2
// + | - | * | / | += | -= | *= | /= | > | >= | < | <= | != | == | & | |
class BinaryExpr : public Expr {
public:
  Expr *left;      // left
  token::Token op; // operator
  Expr *right;     // right

  explicit BinaryExpr(Expr *l, token::Token op, Expr *r) : left(l), right(r) {
    this->op = std::move(op);
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<BinaryExpr { Left=" << left->stringer() << " Operator='"
        << op.literal << "' Right=" << right->stringer() << " }>";

    return str.str();
  }

  Kind kind() override { return EXPR_BINARY; }
};

//(EXPR)
class GroupExpr : public Expr {
public:
  Expr *expr;

  explicit GroupExpr(Expr *expr) : expr(expr) {}

  std::string stringer() override {
    return "<GroupExpr { Expr=" + this->expr->stringer() + " }>";
  }

  Kind kind() override { return EXPR_GROUP; }
};

//<OP>EXPR
class UnaryExpr : public Expr {
public:
  token::Token token;
  Expr *expr;

  explicit UnaryExpr(token::Token tok, Expr *expr) : expr(expr) {
    this->token = std::move(tok);
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<UnaryExpr { Token='" << token.literal
        << "' Expr=" << expr->stringer() << " }>";

    return str.str();
  }

  Kind kind() override { return EXPR_UNARY; }
};

// IDENT
class NameExpr : public Expr {
public:
  token::Token token;

  explicit NameExpr(token::Token tok) { this->token = std::move(tok); }

  std::string stringer() override {
    return "<NameExpr { Token='" + token.literal + "' }>";
  }

  Kind kind() override { return EXPR_NAME; }
};

// EXPR(<EXPR>..)
class CallExpr : public Expr {
public:
  Expr *callee;
  std::vector<Expr *> arguments;

  explicit CallExpr(Expr *expr, std::vector<Expr *> args) : callee(expr) {
    this->arguments = std::move(args);
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<CallExpr { Callee=" << callee->stringer();

    if (!arguments.empty()) {
      str << " Args=(";

      for (auto &i : arguments)
        str << i->stringer() << " ";
      str << ")";
    } else
      str << " Args=()";

    str << " }>";
    return str.str();
  }

  Kind kind() override { return EXPR_CALL; }
};

// EXPR.NAME
class GetExpr : public Expr {
public:
  token::Token name;

  Expr *expr;

  explicit GetExpr(Expr *expr, token::Token name) : expr(expr) {
    this->name = std::move(name);
  }

  std::string stringer() override {
    return "<GetExpr { Expr=" + expr->stringer() + " Get='" + name.literal +
           "' }>";
  }

  Kind kind() override { return EXPR_GET; }
};

// EXPR.NAME = EXPR
class SetExpr : public Expr {
public:
  Expr *expr;
  token::Token name;
  Expr *value;

  explicit SetExpr(Expr *e, token::Token name, Expr *v) : expr(e), value(v) {
    this->name = std::move(name);
  }

  std::string stringer() override {
    return "<SetExpr { Expr='" + expr->stringer() + " Name='" + name.literal +
           "' Value=" + value->stringer() + " }>";
  }

  Kind kind() override { return EXPR_SET; }
};

// EXPR = EXPR
class AssignExpr : public Expr {
public:
  Expr *expr;
  Expr *value;

  explicit AssignExpr(Expr *e, Expr *v) : expr(e), value(v) {}

  std::string stringer() override {
    return "<AssignExpr { Expr='" + expr->stringer() +
           "' Value=" + value->stringer() + " }>";
  }

  Kind kind() override { return EXPR_ASSIGN; }
};

//[<EXPR>..]
class ArrayExpr : public Expr {
public:
  std::vector<Expr *> elements;

  explicit ArrayExpr(std::vector<Expr *> e) { this->elements = std::move(e); }

  std::string stringer() override {
    std::stringstream str;

    str << "<ArrayExpr { Elements=[";
    if (!elements.empty()) {
      for (auto &i : elements)
        str << i->stringer() << " ";
    }

    str << "] }>";
    return str.str();
  }

  Kind kind() override { return EXPR_ARRAY; }
};

//{K1: V1, K2: V2}
class MapExpr : public Expr {
public:
  std::map<Expr *, Expr *> elements;

  explicit MapExpr(std::map<Expr *, Expr *> e) {
    this->elements = std::move(e);
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<MapExpr { Elements={";
    if (!elements.empty()) {
      for (auto &i : elements)
        str << "K : " << i.first->stringer() << ", V : " << i.second->stringer()
            << " ";
    }

    str << "} }>";
    return str.str();
  }

  Kind kind() override { return EXPR_MAP; }
};

//(<EXPR>..)
class TupleExpr : public Expr {
public:
  std::vector<Expr *> elements;

  explicit TupleExpr(std::vector<Expr *> e) { this->elements = std::move(e); }

  std::string stringer() override {
    std::stringstream str;

    str << "<TupleExpr { Elements=(";
    if (!elements.empty()) {
      for (auto &i : elements)
        str << i->stringer() << " ";
    }

    str << ") }>";
    return str.str();
  }

  Kind kind() override { return EXPR_TUPLE; }
};

// EXPR[EXPR]
class IndexExpr : public Expr {
public:
  Expr *left;
  Expr *right;

  explicit IndexExpr(Expr *l, Expr *r) : left(l), right(r) {}

  std::string stringer() override {
    return "<IndexExpr { Left=" + left->stringer() +
           " Right=" + right->stringer() + " }>";
  }

  Kind kind() override { return EXPR_INDEX; }
};

// new <name>{K1: V1, K2: V2}
class NewExpr : public Expr {
public:
  token::Token name;
  std::map<token::Token *, ast::Expr *> builder;

  explicit NewExpr(token::Token name,
                   std::map<token::Token *, ast::Expr *> builder) {
    this->name = std::move(name);
    this->builder = builder;
  }

  std::string stringer() override {
    if (builder.empty()) {
      return "<NewExpr { Name='" + name.literal + "' Builder=NONE }>";
    }
    std::stringstream str;

    str << "<NewExpr { Name='" << name.literal << "'";
    if (!builder.empty()) {
      str << " Builder=(";

      for (auto i : builder) {
        str << "K : '" << i.first->literal << "' V : " << i.second->stringer();
      }
      str << ")";
    }
    return str.str();
  }

  Kind kind() override { return EXPR_NEW; }
};

// abstract stmt
class Stmt {
public:
  // return string of stmt
  virtual std::string stringer() = 0;
  // return kind of stmt
  virtual Kind kind() = 0;
};

// <expr>
class ExprStmt : public Stmt {
public:
  Expr *expr;

  explicit ExprStmt(Expr *expr) : expr(expr) {}

  std::string stringer() override {
    return "<ExprStmt { Expr=" + expr->stringer() + " }>";
  }

  Kind kind() override { return STMT_EXPR; }
};

// def <name>: <type> = <expr>
class VarStmt : public Stmt {
public:
  token::Token name;

  // type define
  Type *T;

  // default is not init
  Expr *expr = nullptr;

  bool local = false;

  // has expr
  explicit VarStmt(token::Token name, Type *T, Expr *e, bool local)
      : T(T), expr(e) {
    this->name = std::move(name);
    this->local = local;
  }

  // not init expr
  explicit VarStmt(token::Token name, Type *T, bool local) : T(T) {
    this->name = std::move(name);
    this->local = local;
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<VarStmt { Name='" << name.literal << "' Type=" << T->stringer()
        << " Local=" << (local ? "T" : "F");

    if (expr != nullptr)
      str << " Expr=" << expr->stringer() << " }>";
    else
      str << " }>";

    return str.str();
  }

  Kind kind() override { return STMT_VAR; }
};

// <expr>.. end
class BlockStmt : public Stmt {
public:
  std::vector<Stmt *> block;

  explicit BlockStmt(std::vector<Stmt *> block) : block(block) {}

  std::string stringer() override {
    std::stringstream str;

    str << "<BlockStmt { ";
    if (!block.empty()) {
      str << "Block=";

      for (auto &i : block)
        str << i->stringer() << " ";
    }

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_BLOCK; }
};

/**
 * if <expr>
 *     <block>
 * ef <expr>
 *     <block>
 * ef <expr>
 *     <block>
 * nf
 *     <block>
 */
class IfStmt : public Stmt {
public:
  Expr *condition;     // main condition
  BlockStmt *ifBranch; // main condition branch

  std::map<Expr *, BlockStmt *> efBranch; // ef cond and branch

  BlockStmt *nfBranch; // nf branch;

  explicit IfStmt(Expr *cond, BlockStmt *then, std::map<Expr *, BlockStmt *> ef,
                  BlockStmt *nf) {
    this->condition = cond;
    this->ifBranch = then;
    this->efBranch = std::move(ef);
    this->nfBranch = nf;
  }

  explicit IfStmt(Expr *cond, BlockStmt *then) {
    this->condition = cond;
    this->ifBranch = then;
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<IfStmt { Condition=" << condition->stringer();
    if (ifBranch != nullptr)
      str << " IfBranch=" << ifBranch->stringer();
    if (!efBranch.empty()) {
      str << " EfBranch=";

      for (auto &i : efBranch)
        str << "K : " << i.first->stringer() << ", V : " << i.second->stringer()
            << " ";
    }
    if (nfBranch != nullptr)
      str << " NfBranch=" << nfBranch->stringer();

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_IF; }
};

/**
 * for <expr>
 *     <block>
 * end
 */
class ForStmt : public Stmt {
public:
  Expr *condition;  // cond
  BlockStmt *block; // stmt

  explicit ForStmt(Expr *cond, BlockStmt *block) {
    this->condition = cond;
    this->block = block;
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<ForStmt { Condition=";
    if (condition == nullptr)
      str << "DEAD Block=";
    else
      str << condition->stringer() << " Block=";
    str << block->stringer();

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_FOR; }
};

/**
 * do
 *     <block>
 * for <expr>
 *     <block>
 * end
 */
class DoStmt : public Stmt {
public:
  BlockStmt *block; // first do block
  Stmt *stmt;       // for statement

  explicit DoStmt(BlockStmt *block, Stmt *stmt) {
    this->block = block;
    this->stmt = stmt;
  }

  std::string stringer() override {
    return "<DoStmt { Block=" + block->stringer() +
           " Stmt=" + stmt->stringer() + " }>";
  }

  Kind kind() override { return STMT_DO; }
};

// out <expr>
class OutStmt : public Stmt {
public:
  Expr *expr;

  OutStmt() { this->expr = nullptr; }

  explicit OutStmt(Expr *e) : expr(e) {}

  std::string stringer() override {
    if (expr == nullptr) {
      return "<OutStmt {}>";
    }
    return "<OutStmt { Expr=" + expr->stringer() + " }>";
  }

  Kind kind() override { return STMT_OUT; }
};

// tin <expr>
class TinStmt : public Stmt {
public:
  Expr *expr;

  TinStmt() { this->expr = nullptr; }

  explicit TinStmt(Expr *e) : expr(e) {}

  std::string stringer() override {
    if (expr == nullptr) {
      return "<TinStmt {}>";
    }
    return "<TinStmt { Expr=" + expr->stringer() + " }>";
  }

  Kind kind() override { return STMT_TIN; }
};

// mod <name>
class ModStmt : public Stmt {
public:
  token::Token name;

  explicit ModStmt(token::Token name) { this->name = std::move(name); }

  std::string stringer() override {
    return "<ModStmt { Name='" + name.literal + "' }>";
  }

  Kind kind() override { return STMT_MOD; }
};

// use <name> | as <name>
class UseStmt : public Stmt {
public:
  token::Token name;

  // use <name>
  explicit UseStmt(token::Token name) { this->name = std::move(name); }

  std::string stringer() override {
    return "<UseStmt { Name='" + name.literal + "' }>";
  }

  Kind kind() override { return STMT_USE; }
};

// ret <stmt>
// ret ->
class RetStmt : public Stmt {
public:
  Stmt *stmt = nullptr;

  explicit RetStmt() {}

  explicit RetStmt(Stmt *s) : stmt(s) {}

  std::string stringer() override {
    if (stmt == nullptr) {
      return "<RetStmt { Stmt=NONE }>";
    }
    return "<RetStmt { Stmt=" + stmt->stringer() + " }>";
  }

  Kind kind() override { return STMT_RET; }
};

// def (<param>..) <name> -> <ret>
//     <block>
// end
class FuncStmt : public Stmt {
public:
  FuncArg arguments; // args

  token::Token name; // name
  Type *ret;         // return
  BlockStmt *block;  // body

  explicit FuncStmt(FuncArg args, token::Token name, Type *ret,
                    BlockStmt *block) {
    this->arguments = args;
    this->name = std::move(name);
    this->ret = ret;
    this->block = block;
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<FuncStmt { Name='" << name.literal;
    str << "' Args=";
    // args
    if (!arguments.empty()) {
      str << "(";
      for (auto &i : arguments) {
        str << "K : '" << i.first->literal << "' T : " << i.second->stringer()
            << " ";
      }
      str << ")";
    } else
      str << "()";

    if (ret == nullptr)
      str << " Ret=NONE";
    else
      str << " Ret=" << ret->stringer();

    str << " Block=" << block->stringer();

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_FUNC; }
};

// <- <name> + <name>..
class InheritStmt : public Stmt {
public:
  std::vector<token::Token *> names;

  explicit InheritStmt(std::vector<token::Token *> names) {
    this->names = std::move(names);
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<InheritStmt Names=";
    if (!names.empty()) {
      str << "(";
      for (auto &i : names) {
        str << "'" << i->literal << "' ";
      }
      str << ")";
    } else
      str << "()";

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_INHERIT; }
};

// def (<param>..) *<name> -> <ret>
class InterfaceStmt : public Stmt {
public:
  FaceArg arguments; // arguments

  token::Token name; // name
  Type *ret;         // return

  explicit InterfaceStmt(FaceArg args, token::Token name, Type *ret) {
    this->arguments = args;
    this->name = std::move(name);
    this->ret = ret;
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<InterfaceStmt { Name='" << name.literal;
    str << "' Args=";
    // args
    if (!arguments.empty()) {
      str << "(";
      for (auto &i : arguments)
        str << "T : '" << i->stringer() << " ";
      str << ")";
    } else
      str << "()";

    if (ret == nullptr)
      str << " Ret=NONE";
    else
      str << " Ret=" << ret->stringer();

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_INTERFACE; }
};

// class
class WholeStmt : public Stmt {
public:
  Stmt *inherit = nullptr; // inherit within class
  BlockStmt *body;

  token::Token name;

  explicit WholeStmt(token::Token name, Stmt *inherit, BlockStmt *body)
      : body(body), inherit(inherit) {
    this->name = std::move(name);
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<WholeStmt { Name='" + name.literal + "' Inherit=";

    if (inherit != nullptr)
      str << inherit->stringer() << " Body=";
    else
      str << "NONE Body=";

    str << body->stringer();

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_WHOLE; }
};

// enum
class EnumStmt : public Stmt {
public:
  std::vector<token::Token *> field;

  token::Token name;

  explicit EnumStmt(token::Token name, std::vector<token::Token *> f)
      : field(f) {
    this->name = std::move(name);
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<EnumStmt { Name='" + name.literal + "' Fields=";

    if (field.empty())
      str << "()";
    else {
      str << "(";
      for (auto iter = field.begin(); iter != field.end();) {
        str << (*iter)->literal;
        if (++iter != field.end()) {
          str << " ";
        }
      }
      str << ")";
    }

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_ENUM; }
};
}; // namespace ast

#endif
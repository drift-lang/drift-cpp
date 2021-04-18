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
#include "type.h"

// abstract syntax tree
namespace ast {
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
  STMT_EXPR,      // EXPR
  STMT_VAR,       // VAR
  STMT_BLOCK,     // BLOCK
  STMT_IF,        // IF
  STMT_FOR,       // FOR
  STMT_AOP,       // AOP
  STMT_OUT,       // OUT
  STMT_GO,        // GO
  STMT_FUNC,      // FUNC
  STMT_WHOLE,     // CLASS | ENUM
  STMT_AND,       // AND
  STMT_MOD,       // MOD
  STMT_USE,       // USE
  STMT_RET,       // RET
  STMT_ENUM,      // ENUM
  STMT_INHERIT,   // <- <name> + <name>..
  STMT_INTERFACE, // INTERFACE
  STMT_DEL        // DEL
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

  std::string stringer() override { return token.literal; }

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

    str << "<BE L=" << left->stringer() << " P='" << op.literal
        << "' R=" << right->stringer() << ">";

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
    return "<GE E=" + this->expr->stringer() + ">";
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

    str << "<UE T='" << token.literal << "' E=" << expr->stringer() << ">";

    return str.str();
  }

  Kind kind() override { return EXPR_UNARY; }
};

// IDENT
class NameExpr : public Expr {
public:
  token::Token token;

  explicit NameExpr(token::Token tok) { this->token = std::move(tok); }

  std::string stringer() override { return "<NE T='" + token.literal + "'>"; }

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

    str << "<CE C=" << callee->stringer();

    if (!arguments.empty()) {
      str << " A=(";

      for (auto &i : arguments)
        str << i->stringer() << " ";
      str << ")";
    } else
      str << " A=()";

    str << ">";
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
    return "<GE E=" + expr->stringer() + " G='" + name.literal + "'>";
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
    return "<SE E='" + expr->stringer() + " N='" + name.literal +
           "' V=" + value->stringer() + ">";
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
    return "<AE E='" + expr->stringer() + "' V=" + value->stringer() + ">";
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

    str << "<AE ES=[";
    if (!elements.empty()) {
      for (auto &i : elements)
        str << i->stringer() << " ";
    }

    str << "]>";
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

    str << "<ME ES={";
    if (!elements.empty()) {
      for (auto &i : elements)
        str << "K : " << i.first->stringer() << ", V : " << i.second->stringer()
            << " ";
    }

    str << "}>";
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

    str << "<TE ES=(";
    if (!elements.empty()) {
      for (auto &i : elements)
        str << i->stringer() << " ";
    }

    str << ")>";
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
    return "<IE L=" + left->stringer() + " R=" + right->stringer() + ">";
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
      return "<NE N='" + name.literal + "' B=NONE>";
    }
    std::stringstream str;

    str << "<NE N='" << name.literal << "'";
    if (!builder.empty()) {
      str << " B=(";

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

  // has expr
  explicit VarStmt(token::Token name, Type *T, Expr *e) : T(T), expr(e) {
    this->name = std::move(name);
  }

  // not init expr
  explicit VarStmt(token::Token name, Type *T) : T(T) {
    this->name = std::move(name);
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<VarStmt { Name='" << name.literal << "' Type=" << T->stringer();

    if (expr != nullptr)
      str << " Expr=" << expr->stringer() << " }>";
    else
      str << " }>";

    return str.str();
  }

  Kind kind() override { return STMT_VAR; }
};

// <expr>.. end | ?
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
 * for <init>; <cond>; <update>
 *     <block>
 * end
 */
class ForStmt : public Stmt {
public:
  Stmt *init; // initializer
  Stmt *cond; // condition
  Stmt *more; // update

  BlockStmt *block; // block

  explicit ForStmt(Stmt *init, Stmt *cond, Stmt *more, BlockStmt *block) {
    this->init = init;
    this->cond = cond;
    this->more = more;
    this->block = block;
  }

  std::string stringer() override {
    return "<ForStmt {}>"; // TODO
  }

  Kind kind() override { return STMT_FOR; }
};

/**
 * aop <expr> | ->
 *  <block>
 * end
 */
class AopStmt : public Stmt {
public:
  Expr *expr;       // expr
  BlockStmt *block; // block

  explicit AopStmt(Expr *expr, BlockStmt *block) {
    this->expr = expr;
    this->block = block;
  }

  std::string stringer() override {
    if (expr == nullptr) {
      return "<AopStmt { Expr=DEAD Block=" + block->stringer() + "}>";
    }
    return "<AopStmt { Expr=" + expr->stringer() +
           " Block=" + block->stringer() + " }>";
  }

  Kind kind() override { return STMT_AOP; }
};

// out <expr> | ->
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

// go <expr> | ->
class GoStmt : public Stmt {
public:
  Expr *expr;

  GoStmt() { this->expr = nullptr; }

  explicit GoStmt(Expr *e) : expr(e) {}

  std::string stringer() override {
    if (expr == nullptr) {
      return "<GoStmt {}>";
    }
    return "<GoStmt { Expr=" + expr->stringer() + " }>";
  }

  Kind kind() override { return STMT_GO; }
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

// del
class DelStmt : public Stmt {
public:
  token::Token name;

  explicit DelStmt(token::Token n) : name(std::move(n)) {}

  std::string stringer() override {
    return "<DelStmt { Name='" + name.literal + "' }>";
  }

  Kind kind() override { return STMT_DEL; }
};
}; // namespace ast

#endif
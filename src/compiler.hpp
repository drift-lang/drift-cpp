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

#ifndef DRIFT_COMPILER_H
#define DRIFT_COMPILER_H

#include <algorithm>

#include "ast.hpp"
#include "entity.hpp"
#include "object.hpp"
#include "opcode.hpp"

// compiler structure
class Compiler {
 private:
  int position = 0;

  // after semantic analysis
  std::vector<ast::Stmt*> statements;

  // return the current statement
  ast::Stmt* look();

  // offset of constant, offset of name, offset of type
  int icf = 0, inf = 0, itf = 0;

  void emitCode(byte::Code);           // push bytecode to entity
  void emitOffset(int);                // push offset to entity
  void emitConstant(object::Object*);  // push constant to entity
  void emitName(std::string);          // push name to entity
  void emitType(ast::Type*);           // push names type to entity

  // insert position with current counts of bytecode
  void insertPosOffset(int);
  void insertPosOffset(int, int);  // with custom value

  void expr(ast::Expr*);  // expression
  void stmt(ast::Stmt*);  // statements

 public:
  Compiler(std::vector<ast::Stmt*> statements) : statements(statements) {}

  // entities of compiled
  std::vector<Entity*> entities = {new Entity("main")};

  // compile statements to entities
  void compile();

  // currently compile entity
  Entity* now = entities.at(0);
};

#endif
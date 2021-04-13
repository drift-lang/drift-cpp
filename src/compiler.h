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

#include "ast.h"
#include "entity.h"
#include "object.h"
#include "opcode.h"
#include "type.h"

// compiler structure
class Compiler {
private:
  int position = 0;

  std::vector<int> lineno; // lineno
  int line = 0;            // line of current stmt

  // after semantic analysis
  std::vector<ast::Stmt *> statements;

  // return the current statement
  ast::Stmt *look();

  // offset of constant, offset of name, offset of type
  int icf = 0, inf = 0, itf = 0;

  void emitCode(byte::Code);           // push bytecode to entity
  void emitOffset(int);                // push offset to entity
  void emitConstant(object::Object *); // push constant to entity
  void emitName(std::string);          // push name to entity
  void emitType(Type *);               // push names type to entity

  void emitJumpOffset(int);

  // insert position with current counts of bytecode
  void insertPosOffset(int);
  void insertPosOffset(int, int); // with custom value

  void expr(ast::Expr *); // expression
  void stmt(ast::Stmt *); // statements

  void replaceHolder(int original); // replace placeHolder

public:
  Compiler(std::vector<ast::Stmt *> statements, std::vector<int> lineno)
      : statements(statements), lineno(lineno) {
    this->line = lineno.front();
  }

  // entities of compiled
  std::vector<Entity *> entities = {new Entity("main")};

  // compile statements to entities
  void compile();

  // currently compile entity
  Entity *now = entities.at(0);
};

#endif
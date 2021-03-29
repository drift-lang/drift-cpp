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

#ifndef DRIFT_PARSER_H
#define DRIFT_PARSER_H

#include <vector>

#include "ast.h"
#include "exception.h"
#include "state.h"
#include "type.h"

// parser structure
class Parser {
private:
  // current token
  int position = 0;

  // token list
  std::vector<token::Token> tokens;

  // return is end of token
  // end of file
  inline bool isEnd();

  // return the address of the token
  inline token::Token *look(bool);

  // look current token, if equal to peek next
  bool look(token::Kind);

  // look current token, do nothing
  inline token::Token look();

  // look the appoint position of tokens
  token::Token look(int);

  // look previous token
  inline token::Token previous();

  // parsing expressions
  ast::Expr *expr();
  ast::Expr *assignment();
  ast::Expr *logicalOr();
  ast::Expr *logicalAnd();
  ast::Expr *equality();
  ast::Expr *comparison();
  ast::Expr *addition();
  ast::Expr *multiplication();
  ast::Expr *unary();
  ast::Expr *call();
  ast::Expr *primary();

  // parsing statements
  ast::Stmt *stmt();

  // determine where to stop the analysis
  ast::BlockStmt *block(token::Kind x, token::Kind y = token::EFF,
                        token::Kind z = token::EFF);

  Type *type(bool previous = false); // parse types

  // throw an exception
  inline void error(exp::Kind, std::string);

  State *state;

public:
  // parser constructor
  explicit Parser(std::vector<token::Token> tokens, State *state) {
    // tokens
    this->tokens = std::move(tokens);
    this->state = state;
  }

  // final stmts list
  std::vector<ast::Stmt *> statements;

  // final stmts line
  std::vector<int> lineno;

  // do parsing
  void parse();

  // final to dissemble statement list
  void dissembleStmts();
};

#endif
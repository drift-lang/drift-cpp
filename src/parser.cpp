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

#include "parser.hpp"

// do parsing
void Parser::parse() {
  while (!this->isEnd()) {
    // push to final list
    this->statements.push_back(this->stmt());
  }
}

// final to dissemble statement list
void Parser::dissembleStmts() {
  if (this->statements.empty()) {
    std::cout << "Empty Statements" << std::endl << std::endl;
    return;
  } else {
    std::cout << "STATEMENT: " << std::endl << std::endl;
  }
  int i = 1;
  for (auto stmt : this->statements)
    std::cout << i++ << " " + stmt->stringer() << std::endl;
}

// if kind of current token is EFF, its end of file and end of tokens
inline bool Parser::isEnd() {
  return look().kind == token::EFF || this->position >= this->tokens.size();
}

// return the address of the token
inline token::Token *Parser::look(bool previous) {
  if (previous) {
    return &this->tokens.at(this->position - 1);
  }
  return &this->tokens.at(this->position);
}

// return the token of the current location
inline token::Token Parser::look() { return this->tokens.at(this->position); }

// look the appoint position of tokens
token::Token Parser::look(int i) {
  if (this->position + i >= this->tokens.size())
    return token::Token
        // EFF token
        {token::EFF, "EFF", -1};
  else
    return this->tokens.at(this->position + i);
}

// if argument is equal to current token
bool Parser::look(token::Kind kind) {
  if (this->look().kind == kind) {
    this->position++;
    //
    return true;
  }
  return false;
}

// return the previous of tokens
inline token::Token Parser::previous() {
  return this->tokens.at(this->position - 1);
}

/**
 * expression
 *
 * assignment -> logicalOr -> logicalAnd -> equality  |
 *                                                    v
 * | unary <- multiplication <- addition <-  comparison
 * v
 * primary -> call
 *
 * - top down operation precedence grammar analysis -
 */
ast::Expr *Parser::expr() { return assignment(); }

// EXPR.NAME = EXPR : SET
// EXPR = EXPR      : ASSIGN
ast::Expr *Parser::assignment() {
  ast::Expr *expr = logicalOr();

  if (look(token::EQ)) {
    ast::Expr *value = assignment();

    // EXPR = EXPR
    if (expr->kind() == ast::EXPR_NAME || expr->kind() == ast::EXPR_INDEX) {
      return new ast::AssignExpr(expr, value);
    }
    // EXPR.NAME = EXPR
    if (expr->kind() == ast::EXPR_GET) {
      ast::GetExpr *get = static_cast<ast::GetExpr *>(expr);
      return new ast::SetExpr(get->expr, get->name, value);
    }
    error(exp::INVALID_SYNTAX, "cannot assign value");
  }
  return expr;
}

// |
ast::Expr *Parser::logicalOr() {
  ast::Expr *expr = logicalAnd();

  while (look(token::OR)) {
    token::Token op = this->previous();
    ast::Expr *right = logicalAnd();
    //
    expr = new ast::BinaryExpr(expr, op, right);
  }
  return expr;
}

// &
ast::Expr *Parser::logicalAnd() {
  ast::Expr *expr = equality();

  while (look(token::ADDR)) {
    token::Token op = this->previous();
    ast::Expr *right = equality();
    //
    expr = new ast::BinaryExpr(expr, op, right);
  }
  return expr;
}

// == | !=
ast::Expr *Parser::equality() {
  ast::Expr *expr = comparison();

  while (look(token::EQ_EQ) || look(token::BANG_EQ)) {
    token::Token op = this->previous();
    ast::Expr *right = comparison();
    //
    expr = new ast::BinaryExpr(expr, op, right);
  }
  return expr;
}

// > | >= | < | <=
ast::Expr *Parser::comparison() {
  ast::Expr *expr = addition();

  while (look(token::GREATER) || look(token::GR_EQ) || look(token::LESS) ||
         look(token::LE_EQ)) {
    token::Token op = this->previous();
    ast::Expr *right = addition();
    //
    expr = new ast::BinaryExpr(expr, op, right);
  }
  return expr;
}

// + | - | += | -=
ast::Expr *Parser::addition() {
  ast::Expr *expr = multiplication();

  while (look(token::ADD) || look(token::SUB) || look(token::AS_ADD) ||
         look(token::AS_SUB)) {
    token::Token op = this->previous();
    ast::Expr *right = multiplication();
    //
    expr = new ast::BinaryExpr(expr, op, right);
  }
  return expr;
}

// * | / | *= | /=
ast::Expr *Parser::multiplication() {
  ast::Expr *expr = unary();

  while (look(token::MUL) || look(token::DIV) || look(token::AS_MUL) ||
         look(token::AS_DIV) || look(token::SUR) || look(token::AS_SUR)) {
    token::Token op = this->previous();
    ast::Expr *right = unary();
    //
    expr = new ast::BinaryExpr(expr, op, right);
  }
  return expr;
}

// ! | -
ast::Expr *Parser::unary() {
  while (look(token::BANG) || look(token::SUB)) {
    token::Token op = previous();
    ast::Expr *expr = unary();
    //
    return new ast::UnaryExpr(op, expr);
  }
  return call();
}

// expr(<expr>..) | expr.name | expr[expr]
ast::Expr *Parser::call() {
  ast::Expr *expr = primary();
  // stack up the expression!!
  //
  // LIKE: bar(foo(1, 2, 3)[x + 4])
  //
  while (true) {
    // call
    if (look(token::L_PAREN)) {
      // arguments
      auto args = std::vector<ast::Expr *>();
      // no argument
      if (look(token::R_PAREN)) {
        expr = new ast::CallExpr(expr, args);
        // to next loop
        continue;
        // have arguments
      } else {
        do {
          args.push_back(this->expr());
          //
        } while (look(token::COMMA));
      }
      if (!look(token::R_PAREN))
        error(exp::UNEXPECTED, "expect ')' after arguments");
      expr = new ast::CallExpr(expr, args);
      // get
    } else if (look(token::DOT)) {
      token::Token name = look();

      if (isEnd()) error(exp::UNEXPECTED, "missing name and found EFF");
      this->position++; // skip name token
      expr = new ast::GetExpr(expr, name);
      // index for array
    } else if (look(token::L_BRACKET)) {
      // empty index
      if (look(token::R_BRACKET)) error(exp::UNEXPECTED, "null index");
      // index
      auto index = this->expr();

      if (!look(token::R_BRACKET))
        error(exp::UNEXPECTED, "expect ']' after index of array");
      expr = new ast::IndexExpr(expr, index);
    } else {
      break;
    }
  }
  return expr;
}

// primary
ast::Expr *Parser::primary() {
  // literal expr
  // number | float | string | char
  if (look(token::NUM) || look(token::FLOAT) || look(token::STR) ||
      look(token::CHAR))
    return new ast::LiteralExpr(this->previous());
  // name expr
  if (look(token::IDENT)) return new ast::NameExpr(previous());
  // group expr
  if (look(token::L_PAREN)) {
    // vector for tuple and group expression
    std::vector<ast::Expr *> elem;
    // empty tuple expr
    if (look(token::R_PAREN)) return new ast::TupleExpr(elem);
    // tuple or group ?
    elem.push_back(this->expr());

    // tuple expr
    if (look(token::COMMA)) {
      do {
        elem.push_back(this->expr());
        //
      } while (look(token::COMMA));
      //
      if (!look(token::R_PAREN))
        error(exp::UNEXPECTED, "expect ')' after tuple expression");
      return new ast::TupleExpr(elem);
    }

    if (look(token::R_PAREN) == false)
      error(exp::UNEXPECTED, "expect ')' after group expression");
    //
    return new ast::GroupExpr(elem.at(0));
  }
  // array expr
  if (look(token::L_BRACKET)) {
    auto elem = std::vector<ast::Expr *>();

    if (look(token::R_BRACKET))
      return new ast::ArrayExpr(elem);
    else {
      do {
        elem.push_back(this->expr());
        //
      } while (look(token::COMMA));
    }
    if (!look(token::R_BRACKET))
      error(exp::UNEXPECTED, "expect ']' after elements");
    return new ast::ArrayExpr(elem);
  }
  // map expr
  if (look(token::L_BRACE)) {
    std::map<ast::Expr *, ast::Expr *> elem;
    // empty map expr
    if (look(token::R_BRACE)) return new ast::MapExpr(elem);

    while (true) {
      ast::Expr *K = this->expr();

      if (!look(token::COLON)) {
        error(exp::UNEXPECTED, "expect ':' after key in map");
      }
      ast::Expr *V = this->expr();

      // push to map
      elem.insert(std::make_pair(K, V));

      if (look(token::COMMA)) {
        continue;
      }
      if (look(token::R_BRACE)) {
        break;
      }
      error(exp::UNEXPECTED, "expect ',' or '}' after value in map");
    }
    return new ast::MapExpr(elem);
  }
  // new expr
  if (look(token::NEW)) {
    if (!look(token::IDENT)) {
      error(exp::INVALID_SYNTAX, "name of new must be an identifier");
    }
    token::Token name = previous(); // name of new

    std::map<token::Token *, ast::Expr *> builder; // fields

    if (!look(token::L_BRACE)) return new ast::NewExpr(name, builder);

    while (true) {
      if (!look(token::IDENT)) {
        error(exp::INVALID_SYNTAX,
              "key of name for new statement must be an identifier");
      }
      int tempPos = this->position - 1;

      if (!look(token::COLON)) {
        error(exp::INVALID_SYNTAX, "expect ':' after key");
      }
      ast::Expr *V = this->expr(); // expr V

      builder.insert(std::make_pair(&this->tokens.at(tempPos), V));

      if (look(token::COMMA)) {
        continue;
      }
      if (look(token::R_BRACE)) {
        break;
      }
    }
    return new ast::NewExpr(name, builder);
  }
  // end
  error(exp::INVALID_SYNTAX, "invalid expression: " + look().literal);
  return nullptr;
}

//  statement
ast::Stmt *Parser::stmt() {
  switch (this->look().kind) {
    // definition statement
    case token::DEF:
      this->position++;
      // variable
      if (look(token::IDENT) && look().kind == token::COLON) {
        token::Token name = previous();
        this->position++; // skip colon symbol

        ast::Type *T = this->type();

        // value of variable
        if (look(token::EQ))
          // there is an initial value
          return new ast::VarStmt(name, T, this->expr());
        else
          return new ast::VarStmt(name, T);
      }
      // function or interface
      else if (look(token::L_PAREN)) {
        // arguments
        //
        // <[ [token] ], Expr>
        ast::FuncArg funcArgs;
        // name
        token::Token name;
        // return
        ast::Type *ret = nullptr;
        // cache multiple parameters
        std::vector<token::Token *> temp;

        //
        bool interfaceStmt = false;
        ast::FaceArg faceArgs; // T1, T2..

        while (!look(token::R_PAREN)) {
          if (!look(token::IDENT) && !interfaceStmt) {
            error(exp::UNEXPECTED, "argument name muse be an identifier");
          }
          // K
          token::Token *K = look(true); // address of token

          // handle multiparameter
          while (look(token::ADD)) {
            if (!look(token::IDENT)) {
              error(exp::UNEXPECTED, "argument name muse be an identifier");
            } else {
              temp.push_back(K); // previous,
              // left of the
              // plus sign
              temp.push_back(look(true)); // address
                                          // of token
            }
          }

          // interface
          if (look().kind == token::COMMA || look().kind == token::R_PAREN ||
              interfaceStmt) {
            //
            faceArgs.push_back(
                this->type(true)); // parse previous token to type
            interfaceStmt = true;  // set

            if (look().kind == token::COMMA) this->position++;
            if (look().kind == token::R_PAREN || look().kind == token::MUL) {
              break; // end
            } else {
              continue; // go
            }
          }

          if (!look(token::COLON)) {
            error(exp::UNEXPECTED, "expect ':' after parameter name");
          }
          // handle multiparameter
          if (temp.empty()) {
            // no
            funcArgs.insert(std::make_pair(K, this->type()));
          } else {
            // multip
            ast::Type *T = this->type();

            for (auto i : temp) {
              funcArgs.insert(std::make_pair(i, T));
            }
          }
          if (look(token::COMMA)) {
            continue;
          }
        }
        // function
        if (look(token::IDENT)) {
          name = previous();
        }
        // interface
        else if (look(token::MUL)) {
          name = look(); // name of interface
          //
          this->position++; // skip name of
          // interface
          //
          // current parsing interface statement
          interfaceStmt = true;
        }
        // error
        else {
          error(exp::UNEXPECTED,
                "expect '*' to interface or identifier to function");
        }
        // return value
        if (look(token::R_ARROW)) {
          ret = this->type();
        }

        if (interfaceStmt) {
          return new ast::InterfaceStmt(faceArgs, name, ret);
        }
        // function
        return new ast::FuncStmt(funcArgs, name, ret, this->block(token::END));
        //
        break;
        // whole
      } else {
        ast::Stmt *inherit = nullptr;
        token::Token name = previous(); // name

        // inherit
        if (look().kind == token::L_ARROW) {
          inherit = this->stmt();
        }
        return new ast::WholeStmt(name, inherit, this->block(token::END));
      }
      break;
      // if
    case token::IF: {
      this->position++;
      // if condition
      ast::Expr *condition = this->expr();
      // if then branch
      ast::BlockStmt *thenBranch =
          this->block(token::EF, token::END, token::NF);

      std::map<ast::Expr *, ast::BlockStmt *> elem;

      while (previous().kind == token::EF) {
        ast::Expr *efCondition = this->expr();
        ast::BlockStmt *efBranch =
            this->block(token::EF, token::END, token::NF);
        //
        elem.insert(std::make_pair(efCondition, efBranch));
      }

      ast::BlockStmt *nfBranch = nullptr;

      if (previous().kind == token::NF) {
        nfBranch = this->block(token::END);
      }
      return new ast::IfStmt(condition, thenBranch, elem, nfBranch);
    } break;
      // loop
    case token::FOR: {
      this->position++;
      // dead loop
      if (look(token::R_ARROW))
        return new ast::ForStmt(nullptr, this->block(token::END));
      // for condition
      ast::Expr *condition = this->expr();
      ast::BlockStmt *block = this->block(token::END);
      //
      return new ast::ForStmt(condition, block);
    } break;
      // do loop
    case token::DO: {
      this->position++;
      // block
      ast::BlockStmt *block = this->block(token::FOR);
      // go back to the position of the `for` keyword
      this->position--;
      ast::Stmt *stmt = this->stmt();
      //
      return new ast::DoStmt(block, stmt);
    } break;
      // out in loop
      // out <expr>
    case token::OUT:
      this->position++;
      //
      if (look(token::R_ARROW)) {
        // no condition
        return new ast::OutStmt();
      }
      return new ast::OutStmt(this->expr());
      break;
      // tin in loop
      // tin <expr>
    case token::TIN:
      this->position++;
      //
      if (look(token::R_ARROW)) {
        // no condition
        return new ast::TinStmt();
      }
      return new ast::TinStmt(this->expr());
      // and
    case token::AND: {
      this->position++;

      if (!look(token::R_ARROW)) {
        error(exp::UNEXPECTED, "expect '->' after and keyword");
      }
      if (!look(token::IDENT)) {
        error(exp::UNEXPECTED, "alias must be an identifier");
      }
      // name
      token::Token alias = previous();
      // block
      ast::BlockStmt *stmt = this->block(token::END);
      //
      return new ast::AndStmt(alias, stmt);
    } break;
      // mod
    case token::MOD:
      this->position++;
      //
      if (!look(token::IDENT)) {
        error(exp::UNEXPECTED, "module name must be an identifier");
      }
      return new ast::ModStmt(previous());
      break;
      // use
    case token::USE: {
      this->position++;

      if (!look(token::IDENT)) {
        error(exp::UNEXPECTED, "use of module name must be an identifier");
      }
      // name
      token::Token name = previous();
      // no alias
      if (!look(token::AS)) {
        return new ast::UseStmt(name);
      }
      if (!look(token::IDENT)) {
        error(exp::UNEXPECTED, "alias of module name must be an identifier");
      }
      int previous = this->position - 1;
      //
      // [Q]: why can't variables on the stack be referenced
      //
      return new ast::UseStmt(name, &this->tokens.at(previous));
    } break;
      // return
      // ret <expr>
      // ret ->
    case token::RET:
      this->position++;
      //
      if (look(token::R_ARROW)) {
        // no return value
        return new ast::RetStmt();
      }
      return new ast::RetStmt(this->stmt());
      break;
      // inherit for class
    case token::L_ARROW: {
      this->position++;

      if (!look(token::IDENT)) {
        error(exp::UNEXPECTED, "inheritance name must be an indentifier");
      }
      std::vector<token::Token *> names;
      // single
      names.push_back(look(true)); // address of token

      while (look(token::ADD)) {
        if (!look(token::IDENT)) {
          error(exp::UNEXPECTED, "inheritance name must be an indentifier");
        }
        names.push_back(look(true)); // address
      }

      return new ast::InheritStmt(names);
    } break;
      // pub
    case token::PUB:
      this->position++;
      //
      return new ast::PubStmt(look().line, this->stmt());
      break;
    default:
      // expression statement
      return new ast::ExprStmt(this->expr());
  }
  // end
  error(exp::INVALID_SYNTAX, "invalid statement");
  return nullptr;
}

/**
 * parse block statement
 *
 * the x parameter is required, and y and z have default value
 * determine where to stop the analysis
 */
ast::BlockStmt *Parser::block(token::Kind x, token::Kind y, token::Kind z) {
  std::vector<ast::Stmt *> body;
  // until end token
  while (true) {
    if (look(x)) {
      break;
    }
    // it is not the default value and holds
    if (y != token::EFF && look(y)) {
      break;
    }
    // it is not the default value and holds
    if (z != token::EFF && look(z)) {
      break;
    }
    body.push_back(this->stmt());
  }
  return new ast::BlockStmt(body);
}

// throw an exception
inline void Parser::error(exp::Kind kind, std::string message) {
  throw exp::Exp(kind, std::move(message), look().line);
}

//  type analysis
ast::Type *Parser::type(bool previous) {
  token::Token now = *this->look(previous);
  // type
  if (now.kind == token::IDENT) {
    // skip type ident
    this->position++;
    // T1
    if (now.literal == S_INT) return new ast::Int();
    // T2
    if (now.literal == S_FLOAT) return new ast::Float();
    // T3
    if (now.literal == S_STR) return new ast::Str;
    // T4
    if (now.literal == S_CHAR) return new ast::Char();
    // T5
    if (now.literal == S_BOOL) return new ast::Bool;
    // user define type
    return new ast::User(now);
  }
  // T6
  if (now.kind == token::L_BRACKET) {
    this->position++; // skip left [ symbol

    if (!look(token::R_BRACKET)) {
      error(exp::UNEXPECTED, "expect ']' after left square bracket");
    }
    return new ast::Array(this->type());
  }
  // T7
  if (now.kind == token::LESS) {
    this->position++; // skip left < symbol
    // key
    ast::Type *T1 = this->type();

    if (!look(token::COMMA)) {
      error(exp::UNEXPECTED, "expect ',' after key of map");
    }
    ast::Type *T2 = this->type();

    if (!look(token::GREATER)) {
      error(exp::UNEXPECTED, "expect '>' after value of map");
    }
    return new ast::Map(T1, T2);
  }
  // T8
  if (now.kind == token::L_PAREN) {
    this->position++; // skip left ( symbol

    ast::Type *T = this->type();

    if (!look(token::R_PAREN)) {
      error(exp::UNEXPECTED, "expect ')' after tuple define");
    }
    return new ast::Tuple(T);
  }
  error(exp::INVALID_SYNTAX, "invalid type");
  //
  return nullptr;
}
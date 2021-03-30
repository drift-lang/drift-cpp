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

#include "semantic.h"

// statement
void Analysis::analysisStmt(ast::Stmt *stmt) {
  switch (stmt->kind()) {
  case ast::STMT_EXPR: {
    ast::ExprStmt *e = static_cast<ast::ExprStmt *>(stmt);
    ast::Expr *expr = static_cast<ast::Expr *>(e->expr);
    // expression
    this->analysisExpr(expr);
  } break;
  //
  case ast::STMT_WHOLE: {
    ast::WholeStmt *w = static_cast<ast::WholeStmt *>(stmt);

    if (w->body->block.empty()) break;

    ast::Stmt *f = w->body->block.at(0); // first statement

    // just a ident of expression statement
    //
    // enumeration
    //
    if (f->kind() == ast::STMT_EXPR) {
      //
      if (w->inherit != nullptr) {
        error(exp::ENUMERATION, "enumeration type cannot be inherited",
              w->name.line);
      }

      ast::ExprStmt *expr = static_cast<ast::ExprStmt *>(f);
      std::vector<token::Token *> fields;

      for (auto &i : w->body->block) {
        //
        if (i->kind() != ast::STMT_EXPR) {
          error(exp::ENUMERATION, "whole is an enumeration type", w->name.line);
        }
        ast::ExprStmt *pStmt = static_cast<ast::ExprStmt *>(i);
        if (pStmt->expr->kind() != ast::EXPR_NAME) {
          error(exp::ENUMERATION, "whole is an enumeration type", w->name.line);
        }

        ast::NameExpr *name = static_cast<ast::NameExpr *>(pStmt->expr);
        // push to enumeration
        // structure
        fields.push_back(&name->token);
      }
      // replace new statement into
      ast::Stmt *n = new ast::EnumStmt(w->name, fields);
      std::replace(std::begin(*statements), std::end(*statements), now(), n);
      // std::cout << "\033[33m[Semantic analysis replace " << position + 1
      //           << "]\033[0m: WholeStmt -> " << n->stringer() << std::endl;
    }
    // normal whole statement if hinder of statements include name expr
    // to throw an error
    else {
      for (auto &i : w->body->block) {
        if (i->kind() == ast::STMT_EXPR) {
          error(exp::ENUMERATION,
                "it an whole statement but contains some other "
                "value",
                w->name.line);
        }
      }
    }
  } break;
  //
  default: break;
  }
}

// expression
void Analysis::analysisExpr(ast::Expr *expr) {
  using namespace token;

  switch (expr->kind()) {
  case ast::EXPR_BINARY: {
    ast::BinaryExpr *binary = static_cast<ast::BinaryExpr *>(expr);

    if (binary->left->kind() != ast::EXPR_LITERAL) {
      this->analysisExpr(binary->left);
      break;
    }
    if (binary->right->kind() != ast::EXPR_LITERAL) {
      this->analysisExpr(binary->right);
      break;
    }

    Token l = (static_cast<ast::LiteralExpr *>(binary->left))->token;
    Token r = (static_cast<ast::LiteralExpr *>(binary->right))->token;

    switch (binary->op.kind) {
    case ADD: // +
    case SUB: // -
      if (l.kind == NUM) {
        //
        if (r.kind == STR || r.kind == CHAR) {
          error(exp::TYPE_ERROR, "unsupported operand", l.line);
        }
      }
      if (l.kind == STR || l.kind == CHAR) {
        //
        if (r.kind == NUM) {
          error(exp::TYPE_ERROR, "unsupported operand", l.line);
        }
      }
      break;
    case AS_ADD: // +=
    case AS_SUB: // -=
    case AS_MUL: // *=
    case AS_DIV: // /=
      if (binary->left->kind() != ast::EXPR_NAME) {
        error(exp::TYPE_ERROR, "unsupported operand", l.line);
      }
      break;
    case DIV: // /
      if (l.kind == STR || l.kind == CHAR || r.kind == STR || r.kind == CHAR) {
        error(exp::TYPE_ERROR, "unsupported operand", l.line);
      }
      if (r.kind == NUM) {
        // convert, keep floating point
        // numbers
        if (std::stof(r.literal) == 0) {
          error(exp::DIVISION_ZERO, "division by zero", l.line);
        }
      }
      // array
      if (binary->left->kind() == ast::EXPR_ARRAY ||
          binary->right->kind() == ast::EXPR_ARRAY) {
        error(exp::TYPE_ERROR, "unsupported operand", l.line);
      }
      break;
    case MUL: // *
      if ((l.kind == CHAR || l.kind == STR) &&
          (r.kind == CHAR || r.kind == STR)) {
        error(exp::TYPE_ERROR, "unsupported operand", l.line);
      }
      break;
    case GR_EQ:   // >=
    case LE_EQ:   // <=
    case GREATER: // >
    case LESS: {  // <
      if (l.kind == STR || r.kind == STR) {
        error(exp::TYPE_ERROR, "unsupported operand", l.line);
      }
    } break;
    default: break;
    }
  } break;
  case ast::EXPR_GROUP: {
    ast::GroupExpr *group = static_cast<ast::GroupExpr *>(expr);
    this->analysisExpr(group->expr);
  } break;
    //
  case ast::EXPR_UNARY: {
    ast::UnaryExpr *unary = static_cast<ast::UnaryExpr *>(expr);
    this->analysisExpr(unary->expr);
  } break;
    //
  case ast::EXPR_CALL: {
    ast::CallExpr *call = static_cast<ast::CallExpr *>(expr);

    for (auto i : call->arguments) {
      this->analysisExpr(i);
    }
  } break;
    //
  case ast::EXPR_GET: {
    ast::GetExpr *get = static_cast<ast::GetExpr *>(expr);
    this->analysisExpr(get->expr);
  } break;
    //
  case ast::EXPR_SET: {
    ast::SetExpr *set = static_cast<ast::SetExpr *>(expr);
    this->analysisExpr(set->expr);
    this->analysisExpr(set->value);
  } break;
    //
  case ast::EXPR_ASSIGN: {
    ast::AssignExpr *assign = static_cast<ast::AssignExpr *>(expr);
    this->analysisExpr(assign->expr);
    this->analysisExpr(assign->value);
  } break;
  //
  default: break;
  }
}
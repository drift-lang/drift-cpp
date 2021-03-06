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

#include "compiler.hpp"

// return the current statement
ast::Stmt *Compiler::look() { return this->statements.at(this->position); }

// compile statements to entities
void Compiler::compile() {
  while (this->position < this->statements.size()) {
    this->stmt(look()); // START
    this->position++;
  }
  this->emitCode(byte::RET); // RET
}

// push bytecode to entity
void Compiler::emitCode(byte::Code co) { this->now->codes.push_back(co); }

// push offset to entity
void Compiler::emitOffset(int off) { this->now->offsets.push_back(off); }

// push constant to entity
void Compiler::emitConstant(object::Object *obj) {
  this->now->constants.push_back(obj);
  this->emitOffset(this->icf++);
}

// push name to entity
void Compiler::emitName(std::string v) {
  std::vector<std::string>::iterator iter =
      std::find(now->names.begin(), now->names.end(), v);
  if (iter != now->names.end()) {
    // found
    this->emitOffset(
        std::distance(now->names.begin(), iter)); // only push offset
  } else {
    // not found
    this->now->names.push_back(v); // push new name
    this->emitOffset(this->inf++); // push new offset
  }
}

// push names type to entity
void Compiler::emitType(ast::Type *t) {
  this->now->types.push_back(t);
  this->emitOffset(this->itf++);
}

// insert position with current counts of bytecode
void Compiler::insertPosOffset(int pos) {
  this->now->offsets.insert(now->offsets.begin() + pos, now->codes.size());
}

// with custom value
void Compiler::insertPosOffset(int pos, int val) {
  this->now->offsets.insert(now->offsets.begin() + pos, val);
}

// expression
void Compiler::expr(ast::Expr *expr) {
  switch (expr->kind()) {
    case ast::EXPR_LITERAL: {
      ast::LiteralExpr *l = static_cast<ast::LiteralExpr *>(expr);
      token::Token tok = l->token;

      if (tok.kind == token::NUM) {
        this->emitConstant(new object::Int(std::stoi(tok.literal)));
      }
      if (tok.kind == token::FLOAT) {
        this->emitConstant(new object::Float(std::stof(tok.literal)));
      }
      if (tok.kind == token::STR) {
        this->emitConstant(
            // judge long characters at here
            new object::Str(tok.literal, tok.literal.back() == '`'));
      }
      if (tok.kind == token::CHAR) {
        this->emitConstant(new object::Char(tok.literal.at(0)));
      }
      this->emitCode(byte::CONST);
    } break;
    case ast::EXPR_BINARY: {
      ast::BinaryExpr *b = static_cast<ast::BinaryExpr *>(expr);

      this->expr(b->left);
      this->expr(b->right);

      switch (b->op.kind) {
        case token::ADD: this->emitCode(byte::ADD); break;
        case token::SUB: this->emitCode(byte::SUB); break;
        case token::MUL: this->emitCode(byte::MUL); break;
        case token::DIV: this->emitCode(byte::DIV); break;

        case token::AS_ADD: this->emitCode(byte::A_ADD); break;
        case token::AS_SUB: this->emitCode(byte::A_SUB); break;
        case token::AS_MUL: this->emitCode(byte::A_MUL); break;
        case token::AS_DIV: this->emitCode(byte::A_DIV); break;

        case token::GREATER: this->emitCode(byte::GR); break;
        case token::LESS: this->emitCode(byte::LE); break;
        case token::GR_EQ: this->emitCode(byte::GR_E); break;
        case token::LE_EQ: this->emitCode(byte::LE_E); break;
        case token::EQ_EQ: this->emitCode(byte::E_E); break;
        case token::BANG_EQ: this->emitCode(byte::N_E); break;
        case token::ADDR: this->emitCode(byte::AND); break;
        case token::OR: this->emitCode(byte::OR); break;
      }

      if (b->op.kind == token::AS_ADD || b->op.kind == token::AS_SUB ||
          b->op.kind == token::AS_MUL || b->op.kind == token::AS_DIV) {
        ast::NameExpr *n = static_cast<ast::NameExpr *>(b->left);

        this->emitName(n->token.literal);
        this->emitCode(byte::ASSIGN);
      }
    } break;
    //
    case ast::EXPR_GROUP: {
      ast::GroupExpr *g = static_cast<ast::GroupExpr *>(expr);

      this->expr(g->expr);
    } break;
    //
    case ast::EXPR_UNARY: {
      ast::UnaryExpr *u = static_cast<ast::UnaryExpr *>(expr);

      this->expr(u->expr);

      if (u->token.kind == token::BANG) {
        this->emitCode(byte::BANG);
      }
      if (u->token.kind == token::SUB) {
        this->emitCode(byte::NOT);
      }
    } break;
    //
    case ast::EXPR_NAME: {
      ast::NameExpr *n = static_cast<ast::NameExpr *>(expr);

      this->emitCode(byte::LOAD);
      this->emitName(n->token.literal); // new name

      // increment and prefix
      if (n->selfIncrement && n->prefix) this->emitCode(byte::P_INCR);
      if (n->selfIncrement) this->emitCode(byte::INCR); // suffix

      // decrement and prefix
      if (n->selfDecrement && n->prefix) this->emitCode(byte::P_DECR);
      if (n->selfDecrement) this->emitCode(byte::DECR); // suffix
    } break;
    //
    case ast::EXPR_CALL: {
      ast::CallExpr *c = static_cast<ast::CallExpr *>(expr);

      this->expr(c->callee);

      for (int i = c->arguments.size(); i > 0; i--)
        this->expr(c->arguments.at(i - 1)); // arguments

      this->emitCode(byte::CALL);
      this->emitOffset(c->arguments.size());
    } break;
    //
    case ast::EXPR_GET: {
      ast::GetExpr *g = static_cast<ast::GetExpr *>(expr);

      this->expr(g->expr);

      this->emitCode(byte::GET);
      this->emitName(g->name.literal); // name
    } break;
    //
    case ast::EXPR_SET: {
      ast::SetExpr *s = static_cast<ast::SetExpr *>(expr);

      this->expr(s->value); // right expression
      this->expr(s->expr);  // left expression

      this->emitCode(byte::SET);
      this->emitName(s->name.literal); // name
    } break;
    //
    case ast::EXPR_ASSIGN: {
      ast::AssignExpr *a = static_cast<ast::AssignExpr *>(expr);

      this->expr(a->value); // right expression

      this->emitCode(byte::ASSIGN);
      this->emitName(static_cast<ast::NameExpr *>(a->expr)->token.literal);
    } break;
    //
    case ast::EXPR_ARRAY: {
      ast::ArrayExpr *a = static_cast<ast::ArrayExpr *>(expr);

      // push elements from right to left
      for (int i = a->elements.size(); i > 0; i--)
        this->expr(a->elements.at(i - 1));

      this->emitCode(byte::B_ARR);
      this->emitOffset(a->elements.size()); // length
    } break;
    //
    case ast::EXPR_TUPLE: {
      ast::TupleExpr *t = static_cast<ast::TupleExpr *>(expr);

      for (int i = t->elements.size(); i > 0; i--)
        this->expr(t->elements.at(i - 1));

      this->emitCode(byte::B_TUP);
      this->emitOffset(t->elements.size()); // length
    } break;
    //
    case ast::EXPR_MAP: {
      ast::MapExpr *m = static_cast<ast::MapExpr *>(expr);

      for (std::map<ast::Expr *, ast::Expr *>::reverse_iterator iter =
               m->elements.rbegin();
           iter != m->elements.rend(); iter++) {
        //  from right to left by iterator
        this->expr(iter->first);
        this->expr(iter->second);
      }

      this->emitCode(byte::B_MAP);
      this->emitOffset(m->elements.size() * 2); // length
    } break;
    //
    case ast::EXPR_INDEX: {
      ast::IndexExpr *i = static_cast<ast::IndexExpr *>(expr);

      this->expr(i->right);
      this->expr(i->left);

      this->emitCode(byte::INDEX);
    } break;
    //
    case ast::EXPR_NEW: {
      ast::NewExpr *n = static_cast<ast::NewExpr *>(expr);

      for (auto i : n->builder) {
        this->emitCode(byte::NAME);
        this->emitName(i.first->literal); // K

        this->expr(i.second); // V
      }

      this->emitCode(byte::NEW);
      this->emitName(n->name.literal);         // name
      this->emitOffset(n->builder.size() * 2); // fields
    } break;
  }
}

// statements
void Compiler::stmt(ast::Stmt *stmt) {
  switch (stmt->kind()) {
    case ast::STMT_EXPR:
      this->expr(static_cast<ast::ExprStmt *>(stmt)->expr); // expression
      break;
    //
    case ast::STMT_VAR: {
      ast::VarStmt *v = static_cast<ast::VarStmt *>(stmt);

      if (v->expr != nullptr)
        this->expr(v->expr); // initial value
      else
        this->emitCode(byte::ORIG); // original value

      this->emitCode(byte::STORE);
      this->emitName(v->name.literal);

      this->emitType(v->T); // type
    } break;
    //
    case ast::STMT_BLOCK: {
      ast::BlockStmt *b = static_cast<ast::BlockStmt *>(stmt);

      for (auto i : b->block) this->stmt(i);
    } break;
    //
    case ast::STMT_IF: {
      ast::IfStmt *i = static_cast<ast::IfStmt *>(stmt);
      /**
       * Moisture regain algorithm
       */
      this->expr(i->condition);
      this->emitCode(byte::F_JUMP);
      int ifPos = now->offsets.size();

      this->stmt(i->ifBranch);
      this->emitCode(byte::JUMP); // jump out after

      int ifOff = now->offsets.size(); // jump after execution if branch
      std::vector<int> tempEfOffs;     // ef condition offsets

      // ef branch
      if (!i->efBranch.empty()) {
        bool firstStmt = true;

        for (auto i : i->efBranch) {
          // if jump to the first ef
          if (firstStmt) {
            this->insertPosOffset(ifPos); // TO: if (F_JUMP)
            firstStmt = false;
          }

          this->expr(i.first); // condition
          this->emitCode(byte::F_JUMP);
          int efPos = now->offsets.size();

          this->stmt(i.second); // block
          this->insertPosOffset(efPos,
                                now->codes.size() + 1); // TO: ef (F_JUMP)

          this->emitCode(byte::JUMP); // jump out after
          tempEfOffs.push_back(now->offsets.size());
        }
        // nf branch
        if (i->nfBranch != nullptr) this->stmt(i->nfBranch);
      }
      // nf branch
      else {
        if (i->nfBranch != nullptr) {
          this->insertPosOffset(ifPos); // TO: if (F_JUMP)
          this->stmt(i->nfBranch);
        } else {
          // no ef and nf statement
          this->insertPosOffset(ifPos); // TO: if (F_JUMP)
        }
      }

      // for (auto i : tempEfOffs) std::cout << i << std::endl;
      for (int i = 0; i < tempEfOffs.size(); i++) {
        // insertion increment successively
        this->insertPosOffset(tempEfOffs.at(i) + i);
      }

      this->insertPosOffset(ifOff + 1); // TO: if (JUMP)
    } break;
    //
    case ast::STMT_FOR: {
      ast::ForStmt *f = static_cast<ast::ForStmt *>(stmt);

      int original = now->codes.size(); // original state: for callback loops

      // DEAD LOOP
      if (f->condition == nullptr) this->stmt(f->block);
      // condition and block
      else {
        this->expr(f->condition);
        this->emitCode(byte::F_JUMP);
        int ePos = now->offsets.size(); // skip loop for false

        this->stmt(f->block); // block

        // jump to next bytecode
        this->insertPosOffset(ePos,
                              now->codes.size() + 1); // TO: (F_JUMP)
      }
      this->emitCode(byte::JUMP); // back to original state
      this->emitOffset(original);
      // replace placeholder
      for (std::vector<int>::iterator iter = now->offsets.begin();
           iter != now->offsets.end(); iter++) {
        // out statement
        if (*iter == -1) {
          *iter = now->codes.size();
        }
        // tin statement
        if (*iter == -2) {
          *iter = original;
        }
      }
    } break;
    //
    case ast::STMT_DO: {
      ast::DoStmt *d = static_cast<ast::DoStmt *>(stmt);

      this->stmt(d->block); // execute the do block first
      this->stmt(d->stmt);  // then execute loop
    } break;
    //
    case ast::STMT_OUT: {
      ast::OutStmt *o = static_cast<ast::OutStmt *>(stmt);

      if (o->expr != nullptr) this->expr(o->expr);

      // jump straight out
      this->emitCode(o->expr == nullptr ? byte::JUMP : byte::T_JUMP);
      // place holder
      this->emitOffset(-1);
    } break;
    //
    case ast::STMT_TIN: {
      ast::TinStmt *t = static_cast<ast::TinStmt *>(stmt);

      if (t->expr != nullptr) this->expr(t->expr);

      // jump straight out
      this->emitCode(t->expr == nullptr ? byte::JUMP : byte::T_JUMP);
      // place holder
      this->emitOffset(-2);
    } break;
    //
    case ast::STMT_FUNC: {
      ast::FuncStmt *f = static_cast<ast::FuncStmt *>(stmt);

      int entitiesSize = this->entities.size() - 1; // original

      this->entities.push_back(
          new Entity(f->name.literal)); // new entity for function statement
      this->now = this->entities.back();

      object::Func *obj = new object::Func;

      obj->name = f->name.literal;   // function name
      obj->arguments = f->arguments; // function arguments
      obj->ret = f->ret;             // function return

      int x = this->icf;
      int y = this->inf;
      int z = this->itf;

      this->icf = 0; // x
      this->inf = 0; // y
      this->itf = 0; // z

      this->stmt(f->block);

      this->icf = x;
      this->inf = y;
      this->itf = z;

      obj->entity = this->now; // function entity

      // if more than one it points to the last one
      this->now = this->entities.at(entitiesSize); // restore to main entity

      // TO main ENTITY
      this->emitCode(byte::FUNC);
      this->emitConstant(obj); // push to constant object
    } break;
    //
    case ast::STMT_WHOLE: {
      ast::WholeStmt *w = static_cast<ast::WholeStmt *>(stmt);

      int entitiesSize = this->entities.size() - 1; // original

      this->entities.push_back(
          new Entity(w->name.literal)); // new entity for whole statement
      this->now = this->entities.back();

      object::Whole *obj = new object::Whole;

      obj->name = w->name.literal; // whole name

      // whole inherit
      if (w->inherit != nullptr) {
        ast::InheritStmt *i = static_cast<ast::InheritStmt *>(w->inherit);
        for (auto iter : i->names) {
          obj->inherit.push_back(iter->literal);
        }
      }

      int x = this->icf;
      int y = this->inf;
      int z = this->itf;

      this->icf = 0; // x
      this->inf = 0; // y
      this->itf = 0; // z

      // block statement
      for (auto i : w->body->block) {
        // interface definition
        if (i->kind() == ast::STMT_INTERFACE) {
          ast::InterfaceStmt *inter = static_cast<ast::InterfaceStmt *>(i);
          obj->interface.push_back(std::make_tuple(
              inter->name.literal, inter->arguments, inter->ret));
          continue;
        }
        this->stmt(i);
      }

      this->icf = x;
      this->inf = y;
      this->itf = z;

      obj->entity = this->now; // whole entity

      // if more than one it points to the last one
      this->now = this->entities.at(entitiesSize); // restore to main entity

      // TO main ENTITY
      this->emitCode(byte::WHOLE);
      this->emitConstant(obj); // push to constant object
    } break;
    //
    case ast::STMT_AND: {
      ast::AndStmt *a = static_cast<ast::AndStmt *>(stmt);

      this->emitCode(byte::CHA);
      this->emitName(a->name.literal); // STORE

      this->stmt(a->block);

      this->emitCode(byte::END);
      this->emitName(a->name.literal); // END
    } break;
    //
    case ast::STMT_MOD: {
      ast::ModStmt *m = static_cast<ast::ModStmt *>(stmt);

      this->emitCode(byte::MOD);
      this->emitName(m->name.literal);
    } break;
    //
    case ast::STMT_USE: {
      ast::UseStmt *u = static_cast<ast::UseStmt *>(stmt);

      if (u->as != nullptr) {
        this->emitCode(byte::UAS);

        this->emitName(u->name.literal); // name
        this->emitName(u->as->literal);  // alias
      } else {
        this->emitCode(byte::USE);

        this->emitName(u->name.literal);
      }
    } break;
    //
    case ast::STMT_RET: {
      ast::RetStmt *r = static_cast<ast::RetStmt *>(stmt);

      if (r->stmt != nullptr) this->stmt(r->stmt);

      this->emitCode(byte::RET);
    } break;
    //
    case ast::STMT_ENUM: {
      ast::EnumStmt *e = static_cast<ast::EnumStmt *>(stmt);

      object::Enum *obj = new object::Enum;
      obj->name = e->name.literal;

      for (int i = 0; i < e->field.size(); i++) {
        obj->elements.insert(std::make_pair(i, e->field.at(i)->literal));
      }

      this->emitCode(byte::ENUM);
      this->emitConstant(obj); // push to constant object
    } break;
    //
    case ast::STMT_CALLINHERIT: {
      ast::CallInheritStmt *c = static_cast<ast::CallInheritStmt *>(stmt);
      ast::CallExpr *e = static_cast<ast::CallExpr *>(c->expr);

      this->expr(e->callee);

      for (int i = e->arguments.size(); i > 0; i--)
        this->expr(e->arguments.at(i - 1));

      this->emitCode(byte::CALL_I);
      this->emitOffset(e->arguments.size()); // length
    } break;
    //
    case ast::STMT_PUB: {
      ast::PubStmt *p = static_cast<ast::PubStmt *>(stmt);

      this->stmt(p->stmt);
      this->emitCode(byte::PUB);
    } break;
    //
    default: break;
  }
}
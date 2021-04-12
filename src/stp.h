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

/**
 * The parser implementation of STP,
 * more to see my paper: https://bingxio.fun/STP.pdf
 */

#ifndef DRIFT_STP_H
#define DRIFT_STP_H

#include <vector>

#include "ast.h"
#include "token.h"

class Stp {
private:
  int p = 0;                        /* Position */
  std::vector<token::Token> tokens; /* Tokens */

public:
  explicit Stp(std::vector<token::Token> tokens)
      : tokens(tokens) {} /* Tokens parse to statements */

  void parse(); /* Go to parse */
  /* Result data */
  std::vector<ast::Stmt *> statements;
};

#endif
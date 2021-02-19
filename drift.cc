//
// Copyright (c) 2021 bingxio（丙杺，黄菁）. All rights reserved.
//

// GNU General Public License, more to see file: LICENSE
// https://www.gnu.org/licenses

// The Drift Programming Language.
//
//          https://github.com/bingxio/drift
//

#include <iostream>
#include <vector>
#include <sstream>
#include <exception>
#include <fstream>
#include <map>

// tokens
namespace token {
//  total number of token for drift
  constexpr int len = 54;
//  token type
  enum Kind {
    IDENT,          // identifier literal
    NUM,            // number     literal
    STR,            // string     literal
    CHAR,           // char       literal

    ADD,            // +
    SUB,            // -
    MUL,            // *
    DIV,            // /

    AS_ADD,         // +=
    AS_SUB,         // -=
    AS_MUL,         // *=
    AS_DIV,         // /=

    PLUS,           // ++
    MINUS,          // --

    R_ARROW,        // ->
    L_ARROW,        // <-

    DOLLAR,         // $
    DOT,            // .
    COMMA,          // ,
    COLON,          // :
    EQ,             // =

    GREATER,        // >
    LESS,           // <
    GR_EQ,          // >=
    LE_EQ,          // <=

    ADDR,           // &
    OR,             // |
    BANG,           // !
    BANG_EQ,        // !=
    EQ_EQ,          // ==
//    SINGLE_MARK,  // '
//    DOUBLE_MARk,  // "
//    OBLIQUE_MARK, // `

    L_BRACE,        // {
    R_BRACE,        // }
    L_PAREN,        // (
    R_PAREN,        // )
    L_BRACKET,      // [
    R_BRACKET,      // ]

    UNDERLINE,      // _

    EFF,            // end of file

//    keywords
    USE,
    DEF,
    PUB,
    RET,
    AND,
    END,
    IF,
    EIF,
    ELS,
    FOR,
    DO,
    OUT,
    TIN,
    NEW,
    MOD,
    AS
  };

//  returns a string of each type
  std::string kindString[len] = { // NOLINT(cert-err58-cpp)
    "IDENT", "NUM", "STR", "CHAR",
    "ADD", "SUB", "MUL", "DIV",
    "AS_ADD", "AS_SUB", "AS_MUL", "AS_DIV",
    "PLUS", "MINUS",
    "R_ARROW", "L_ARROW",
    "DOLLAR", "DOT", "COMMA", "COLON", "EQ",
    "GREATER", "LESS", "GR_EQ", "LE_EQ",
    "ADDR", "OR", "BANG", "BANG_EQ", "EQ_EQ",
//    "SINGLE_MARK", "DOUBLE_MARK", "OBLIQUE_MARK",
    "L_BRACE", "R_BRACE", "L_PAREN", "R_PAREN", "L_BRACKET", "R_BRACKET",
    "UNDERLINE",
//    end of file
    "EFF",
//    keywords
    "USE", "DEF", "PUB", "RET",
    "AND", "END", "IF", "EIF",
    "ELS", "FOR", "DO", "OUT",
    "TIN", "NEW", "MOD", "AS"
  };

//  token structure
  struct Token {
//    token type
    Kind kind = EFF;
//    token literal
    std::string literal;
//    line of source code
    int line = 0;
  };

//  format return token structure
  std::string toString(const Token &token) {
    std::stringstream str;

    str << "<Token { Kind=";
    str << kindString[token.kind] << " Literal=\"";
    str << token.literal << "\" Line=";
    str << token.line << " }>";

    return str.str();
  }

//  keywords for drift
  static std::map<std::string, Kind> keyword;

//  16 keywords
//  initialize it when tokenizer
  void initializeKeywords() {
    keyword["use"] = USE; // 1
    keyword["def"] = DEF; // 2
    keyword["pub"] = PUB; // 3
    keyword["ret"] = RET; // 4
    keyword["and"] = AND; // 5
    keyword["end"] = END; // 6
    keyword["if"] = IF;   // 7
    keyword["eif"] = EIF; // 8
    keyword["els"] = ELS; // 9
    keyword["for"] = FOR; // 10
    keyword["do"] = DO;   // 11
    keyword["out"] = OUT; // 12
    keyword["tin"] = TIN; // 13
    keyword["new"] = NEW; // 14
    keyword["mod"] = MOD; // 15
    keyword["as"] = AS;   // 16
  }

//  return the corresponding keyword type according to the literal amount
  Kind getKeyword(const std::string &literal) {
    auto i = keyword.find(literal);
//    search map
    if (i != keyword.end())
      return i->second;
    else
      return IDENT;
  }
}

// exceptions
namespace exp {
//  total number of exceptions
  constexpr int len = 6;
//  exception type
  enum Kind {
//    lexer
    UNKNOWN_SYMBOL,           // unknown symbol
    CHARACTER_EXP,            // character is empty
    STRING_EXP,               // lost left or right mark
//    parser
    UNEXPECTED,               // unexpected
    INVALID_SYNTAX,           // invalid syntax
    INCREMENT_OP,             // left value increment operand
  };

//  return a string of exception type
  std::string kindString[len] = { // NOLINT(cert-err58-cpp)
//    lexer
    "UNKNOWN_SYMBOL",
    "CHARACTER_EXP",
    "STRING_EXP",
//    parser
    "UNEXPECTED",
    "INVALID_SYNTAX",
    "INCREMENT_OP"
  }; // NOLINT(cert-err58-cpp)

//  exception structure
  class Exp : public std::exception {
    private:
//      exception kind
      Kind kind;
//      exception message
      std::string message;
//      at exception line of source code
      int line;
    public:
      explicit Exp(Kind kind, std::string message, int line) {
        this->kind = kind;
        this->message = std::move(message);
        this->line = line;
      }

//      return a string of exception structure
      std::string toString();
  };

//  return a string of exception structure
  std::string Exp::toString() {
    std::stringstream str;

    str << "<Exception { Kind=";
    str << kindString[this->kind] << " Message=\"";
    str << this->message << "\" Line=";
    str << this->line << " }>";

    return str.str();
  }
}

// lexer
namespace lexer {
//  lexer structure
  class Lexer {
    private:
//      current character
      int position = 0;
//      current line
      int line = 1;
//      source code
      std::string source;

//      resolve identifier
      void lexIdent();
//      resolve digit
      void lexDigit();
//      resolve "xxx" string literal
      void lexString(bool longStr);
//      resolve 'x' character literal
      void lexChar();
//      resolve other symbol
      void lexSymbol();
//      return current char of resolve
      inline char now();
//      return next char of resolve
      char peek();
//      judge the current character and process the token
      bool peekEmit(token::Token *t,
                    char c,              // current char
                    token::Kind k,       // equal token kind
                    const std::string &l // equal token literal
      );
//      return resolve is end
      inline bool isEnd();
//      return current char is identifier
      inline bool isIdent();
//      return current char is digit
      inline bool isDigit();
//      return current char is whitespace
      inline bool isSpace();
//      resolve to skip whitespace
      inline void skipWhitespace();
//      resolve to skip line comment
      inline void skipLineComment();

    public:
      explicit Lexer(std::string source) : source(std::move(source)) {
//        initializer keywords map here
        token::initializeKeywords();
      }

//      final token list
      std::vector<token::Token> tokens;
//      start
      void tokenizer();
//      final to dissemble tokens list
      void dissembleTokens();
  };

//  start
  void Lexer::tokenizer() {
    while (!this->isEnd()) {
//      first to skip whitespace
      if (isSpace()) skipWhitespace();
//      identifier
      else if (isIdent())
        this->lexIdent();
//      digit
      else if (isDigit())
        this->lexDigit();
//      string
      else if (now() == '"')
        this->lexString(false);
//      long strings
      else if (now() == '`')
        this->lexString(true);
//      character
      else if (now() == '\'')
        this->lexChar();
//      symbol
      else
        this->lexSymbol();
    }
    this->tokens.push_back(
//      resolve end insert EFF for end of file
      token::Token{token::EFF, "EFF", ++this->line}
    );
  }

//  final to dissemble tokens list
  void Lexer::dissembleTokens() {
    int i = 1;
    for (const auto &token : this->tokens)
      std::cout << i++ <<
                " " + token::toString(token) << std::endl;
  }

//  return resolve is end
  inline bool Lexer::isEnd() { return this->position >= this->source.length(); }

//  resolve to skip whitespace
  inline void Lexer::skipWhitespace() {
    while (!isEnd() && this->isSpace()) {
      if (now() == '\n')
        this->line++;
      this->position++;
    }
  }

//  resolve to skip line comment
  inline void Lexer::skipLineComment() {
    while (!isEnd() && now() != '\n')
      this->position++;
  }

//  return current char is identifier
  inline bool Lexer::isIdent() {
    return now() >= 'a' && now() <= 'z' ||
           now() >= 'A' && now() <= 'Z' ||
           now() == '_';
  }

//  return current char is digit
  inline bool Lexer::isDigit() {
    return now() >= '0' && now() <= '9';
  }

//  return current char is whitespace
  inline bool Lexer::isSpace() {
    if (now() == ' ' ||
        now() == '\r' ||
        now() == '\t' ||
        now() == '\n') { return true; }
    return false;
  }

//  return current char of resolve
  inline char Lexer::now() { return this->source.at(this->position); }

//  resolve identifier
  void Lexer::lexIdent() {
    std::stringstream literal;

    while (!isEnd()) {
      if (isIdent())
        literal << now();
      else
        break;
      this->position++;
    }

    this->tokens.push_back(
      token::Token{
//        keyword or IDENT
        token::getKeyword(literal.str()),
        literal.str(),
        this->line
      }
    );
  }

//  resolve digit
  void Lexer::lexDigit() {
    std::stringstream literal;

    while (!isEnd()) {
      if (isDigit() || now() == '.')
        literal << now();
      else
        break;
      this->position++;
    }

    this->tokens.push_back(
//      number
      token::Token{token::NUM, literal.str(), this->line});
  }

//  resolve string literal
  void Lexer::lexString(bool longStr) {
    char cond = '"';
//    longer string
    if (longStr) cond = '`';

    std::stringstream literal;
    bool isEndFile = false;

//    skip left double quotation mark
    this->position++;

    while (!isEnd()) {
      if (now() == cond) {
//        end string
        this->position++;
        isEndFile = true;
        break;
      }
      if (now() == '\n' && !longStr) {
        throw exp::Exp(exp::STRING_EXP,
//                       long strings
                       "for long strings use the ` operator", this->line);
        break;
      }
      literal << now();
      this->position++;
    }

//    missing closing symbol
    if (!isEndFile)
      throw exp::Exp(exp::STRING_EXP,
                     "missing closing symbol", this->line);

    this->tokens.push_back(
//      string
      token::Token{token::STR, literal.str(), this->line});
  }

//  resolve character
  void Lexer::lexChar() {
    std::stringstream literal;

//    skip left single quotation mark
    this->position++;

    literal << now();

    if (peek() != '\'')
//      this character is empty
      throw exp::Exp(exp::CHARACTER_EXP,
                     "this character is empty", this->line);
    else
//      skip value and right single quotation mark
      this->position += 2;

    this->tokens.push_back(
//      character
      token::Token{token::CHAR, literal.str(), this->line});
  }

//  resolve symbols
  void Lexer::lexSymbol() {
    token::Token tok;

    tok.literal = now();
    tok.line = this->line;

    switch (now()) {
      case '(':
        tok.kind = token::L_PAREN;
        break;
      case ')':
        tok.kind = token::R_PAREN;
        break;
      case '{':
        tok.kind = token::L_BRACE;
        break;
      case '}':
        tok.kind = token::R_BRACE;
        break;
      case '[':
        tok.kind = token::L_BRACKET;
        break;
      case ']':
        tok.kind = token::R_BRACKET;
        break;
      case ':':
        tok.kind = token::COLON;
        break;
      case '+':
        if (peekEmit(&tok, '=', token::AS_ADD, "+="))
          break;
        if (peekEmit(&tok, '+', token::PLUS, "++"))
          break;
        else
          tok.kind = token::ADD;
        break;
      case '-':
        if (peekEmit(&tok, '>', token::R_ARROW, "->"))
          break;
        if (peekEmit(&tok, '-', token::MINUS, "--"))
          break;
        if (peekEmit(&tok, '=', token::AS_SUB, "-="))
          break;
        else
          tok.kind = token::SUB;
        break;
      case '*':
        if (peekEmit(&tok, '=', token::AS_MUL, "*="))
          break;
        else
          tok.kind = token::MUL;
        break;
      case '/':
        if (peekEmit(&tok, '=', token::AS_DIV, "/="))
          break;
//        to resolve skip comment
        else if (peek() == '/') {
          this->skipLineComment();
//          continue
          return;
        } else
          tok.kind = token::DIV;
        break;
      case '$':
        tok.kind = token::DOLLAR;
        break;
      case '.':
        tok.kind = token::DOT;
        break;
      case ',':
        tok.kind = token::COMMA;
        break;
      case '>':
        if (peekEmit(&tok, '=', token::GR_EQ, ">="))
          break;
        else
          tok.kind = token::GREATER;
        break;
      case '<':
        if (peekEmit(&tok, '=', token::LE_EQ, "<="))
          break;
        if (peekEmit(&tok, '-', token::L_ARROW, "<-"))
          break;
        else
          tok.kind = token::LESS;
        break;
      case '&':
        tok.kind = token::ADDR;
        break;
      case '|':
        tok.kind = token::OR;
        break;
      case '!':
        if (peekEmit(&tok, '=', token::BANG_EQ, "!="))
          break;
        else
          tok.kind = token::BANG;
        break;
      case '=':
        if (peekEmit(&tok, '=', token::EQ_EQ, "=="))
          break;
        else
          tok.kind = token::EQ;
        break;
      case '_':
        tok.kind = token::UNDERLINE;
        break;
      default:
//        what
        throw exp::Exp(exp::UNKNOWN_SYMBOL,
                       "unknown symbol", this->line);
    }
//    skip current single symbol
    this->position++;
    this->tokens.push_back(tok);
  }

//  return next char of resolve
  char Lexer::peek() {
    if (position + 1 >= source.length())
      return -1;
    else
      return source.at(position + 1);
  }

//  judge the current character and process the token
  bool Lexer::peekEmit(token::Token *t,
                       char c,
                       token::Kind k,
                       const std::string &l) {
    if (peek() == c) {
      t->kind = k;
      t->literal = l;
//      advance
      this->position++;
//
      return true;
    } else
      return false;
  }
}

// abstract syntax tree
namespace ast {
//  ast types
  enum Kind {
//    expression
    EXPR_LITERAL,
    EXPR_BINARY,
    EXPR_GROUP,
    EXPR_UNARY,
    EXPR_NAME,
//    statement
    STMT_EXPR
  };

//  abstract expr
  class Expr {
    public:
//      return string of expr
      virtual std::string toString() = 0;
//      return kind of expr
      virtual Kind kind() = 0;
  };

//  number | string | char
  class LiteralExpr : public Expr {
    public:
//      literal
      token::Token token;

      explicit LiteralExpr(token::Token tok) {
        this->token = std::move(tok);
      }

      std::string toString() override {
        return "<LiteralExpr { Token='" + token.literal + "' }>";
      }

      Kind kind() override { return EXPR_LITERAL; }
  };

//  <expr> <operator> <expr>
//  + | - | * | / | += | -= | *= | /= | > | >= | < | <= | != | == | & | |
  class BinaryExpr : public Expr {
    public:
      Expr *left;       // left
      token::Token op;  // operator
      Expr *right;      // right

      explicit BinaryExpr(Expr *l, token::Token op, Expr *r) : left(l), right(r) {
        this->op = std::move(op);
      }

      std::string toString() override {
        std::stringstream str;

        str << "<BinaryExpr { Left=" <<
            left->toString() << " Operator='" <<
            op.literal << "' Right=" << right->toString() << " }>";

        return str.str();
      }

      Kind kind() override { return EXPR_BINARY; }
  };

//  (expr)
  class GroupExpr : public Expr {
    public:
      Expr *expr;

      explicit GroupExpr(Expr *expr) : expr(expr) {}

      std::string toString() override {
        return "<GroupExpr { Expr=" + this->expr->toString() + " }>";
      }

      Kind kind() override { return EXPR_GROUP; }
  };

//  -expr | !expr
  class UnaryExpr : public Expr {
    public:
      token::Token token;
      Expr *expr;

      explicit UnaryExpr(token::Token tok, Expr *expr) : expr(expr) {
        this->token = std::move(tok);
      }

      std::string toString() override {
        std::stringstream str;

        str << "<UnaryExpr { Token=" <<
            token.literal << " Expr=" <<
            expr->toString() << " }>";

        return str.str();
      }

      Kind kind() override { return EXPR_UNARY; }
  };

//  name expr
  class NameExpr : public Expr {
    public:
      token::Token token;

      bool selfIncrement; // ++
      bool selfDecrement; // --

      bool prefix;        // prefix, calc it first

      explicit NameExpr(token::Token tok,
                        bool x = false,   // increment
                        bool y = false,   // decrement
                        bool z = false    // prefix
      ) {
        this->token = std::move(tok);

        this->selfIncrement = x;
        this->selfDecrement = y;

        this->prefix = z;
      }

      std::string toString() override {
        std::stringstream str;

        str << "<NameExpr { Token='" <<
            token.literal << "' Increment=" <<
            selfIncrement << " Decrement=" <<
            selfDecrement << " Prefix=" <<
            prefix << " }>";

        return str.str();
      }

      Kind kind() override { return EXPR_NAME; }
  };

//  abstract stmt
  class Stmt {
    public:
//      return string of stmt
      virtual std::string toString() = 0;
//      return kind of stmt
      virtual Kind kind() = 0;
  };

//  <expr>
  class ExprStmt : public Stmt {
    public:
      Expr *expr;

      explicit ExprStmt(Expr *expr) : expr(expr) {}

      std::string toString() override {
        return "<ExprStmt { Expr=" + expr->toString() + " }>";
      }

      Kind kind() override { return STMT_EXPR; }
  };
}

// parser
namespace parser {
//  parser structure
  class Parser {
    private:
//      current token
      int position = 0;
//      token list
      std::vector<token::Token> tokens;
//      return is end of token
//      end of file
      inline bool isEnd();
//      look current token, if equal to peek next
      bool look(token::Kind kind);
//      look current token, do nothing
      inline token::Token look();
//      look the appoint position of tokens
      token::Token look(int i);
//      look previous token
      inline token::Token previous();
//      parsing expressions
      ast::Expr *expr();
//      ast::Expr *assignment();
      ast::Expr *logicalOr();
      ast::Expr *logicalAnd();
      ast::Expr *equality();
      ast::Expr *comparison();
      ast::Expr *addition();
      ast::Expr *multiplication();
      ast::Expr *unary();
      ast::Expr *primary();
//      parsing statements
      ast::Stmt *stmt();
//      throw an exception
      inline void error(exp::Kind kind, std::string message);

    public:
//      parser constructor
      explicit Parser(std::vector<token::Token> tokens) {
//      tokens
        this->tokens = std::move(tokens);
      }

//      final stmts list
      std::vector<ast::Stmt *> statements;
//      do parsing
      void parse();
//      final to dissemble statement list
      void dissembleStmts();
  };

//  do parsing
  void Parser::parse() {
    while (!this->isEnd()) {
//      push to final list
      this->statements.push_back(this->stmt());
    }
  }

//  final to dissemble statement list
  void Parser::dissembleStmts() {
    if (this->statements.empty()) {
      std::cout << "Empty Statements" << std::endl;
      return;
    }
    int i = 1;
    for (auto stmt : this->statements)
      std::cout << i++ <<
                " " + stmt->toString() << std::endl;
  }

//  if kind of current token is EFF, its end of file and end of tokens
  inline bool Parser::isEnd() { return look().kind == token::EFF; }

//  return the token of the current location
  inline token::Token Parser::look() { return this->tokens.at(this->position); }

//  look the appoint position of tokens
  token::Token Parser::look(int i) {
    if (this->position + i >= this->tokens.size())
      return token::Token
//      EFF token
        {token::EFF, "EFF", -1};
    else
      return this->tokens.at(this->position + i);
  }

//  if argument is equal to current token
  bool Parser::look(token::Kind kind) {
    bool eq = this->look().kind == kind;
//    peek to next token
    if (eq) {
      this->position++;
    }
    return eq;
  }

//  return the previous of tokens
  inline token::Token Parser::previous() {
    return this->tokens.at(this->position - 1);
  }

//  expression
  ast::Expr *Parser::expr() { return logicalOr(); }

//  drift does not support assigning values within expressions

//  ast::Expr *Parser::assignment() { return logicalOr(); }

//  |
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

//  &
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

//  == | !=
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

//  > | >= | < | <=
  ast::Expr *Parser::comparison() {
    ast::Expr *expr = addition();

    while (look(token::GREATER) ||
           look(token::GR_EQ) ||
           look(token::LESS) ||
           look(token::LE_EQ)) {
      token::Token op = this->previous();
      ast::Expr *right = addition();
//
      expr = new ast::BinaryExpr(expr, op, right);
    }
    return expr;
  }

//  + | - | += | -=
  ast::Expr *Parser::addition() {
    ast::Expr *expr = multiplication();

    while (look(token::ADD) ||
           look(token::SUB) ||
           look(token::AS_ADD) ||
           look(token::AS_SUB)) {
      token::Token op = this->previous();
      ast::Expr *right = multiplication();
//
      expr = new ast::BinaryExpr(expr, op, right);
    }
    return expr;
  }

//  * | / | *= | /=
  ast::Expr *Parser::multiplication() {
    ast::Expr *expr = unary();

    while (look(token::MUL) ||
           look(token::DIV) ||
           look(token::AS_MUL) ||
           look(token::AS_DIV)) {
      token::Token op = this->previous();
      ast::Expr *right = unary();
//
      expr = new ast::BinaryExpr(expr, op, right);
    }
    return expr;
  }

//  ! | -
  ast::Expr *Parser::unary() {
    while (look(token::BANG) || look(token::SUB)) {
      token::Token op = previous();
      ast::Expr *expr = unary();
//
      return new ast::UnaryExpr(op, expr);
    }
    return primary();
  }

//  primary
  ast::Expr *Parser::primary() {
//    literal expr
//    number | string | char
    if (look(token::NUM) ||
        look(token::STR) ||
        look(token::CHAR)
      )
      return new ast::LiteralExpr(this->previous());
//    name expr
    if (look(token::IDENT)) {
      token::Token tok = previous();
//      ++ | --
      if (look(token::PLUS) || look(token::MINUS))
//        self increment
        return new ast::NameExpr(tok,
                                 previous().kind == token::PLUS,
                                 previous().kind == token::MINUS, false);
      else
        return new ast::NameExpr(tok);
    }
//    name expr of ++ or -- operators
    if (look(token::PLUS) || look(token::MINUS)) {
      token::Token op = previous();
//
      if (look(token::IDENT))
        return new ast::NameExpr(previous(),
                                 op.kind == token::PLUS,
                                 op.kind == token::MINUS, true);
      else
        error(exp::INCREMENT_OP,
              "increment operand can only be performed on name");
    }
//    group expr
    if (look(token::L_PAREN)) {
      ast::Expr *expr = this->expr();

      if (look(token::R_PAREN) == false)
        error(exp::UNEXPECTED, "expect ')' after group expression");
//
      return new ast::GroupExpr(expr);
    }
// end
    error(exp::INVALID_SYNTAX, "invalid syntax");
    return nullptr;
  }

//  statement
  ast::Stmt *Parser::stmt() {
    switch (this->look().kind) {
      default:
        return new ast::ExprStmt(this->expr());
    }
  }

//  throw an exception
  inline void Parser::error(exp::Kind kind, std::string message) {
    throw exp::Exp(kind, std::move(message), look().line);
  }
}

int main() {
  std::ifstream stream;
  stream.open("test2.ft");

  if (stream.fail()) {
    std::cout << "failed to open file" << std::endl;
    return -1;
  }

  std::string source((std::istreambuf_iterator<char>(stream)),
                     (std::istreambuf_iterator<char>()));

//    lexer
  auto lex = new lexer::Lexer(source);

  try {
    lex->tokenizer();
  } catch (exp::Exp &e) {
    std::cout << "\033[31m" << e.toString() << "\033[0m" << std::endl;
    return -1;
  }

  lex->dissembleTokens();

//    parser
  auto parser = new parser::Parser(lex->tokens);

  try {
    parser->parse();
  } catch (exp::Exp &e) {
    std::cout << "\033[31m" << e.toString() << "\033[0m" << std::endl;
    return -1;
  }

  parser->dissembleStmts();

  stream.close();
  return 0;
}

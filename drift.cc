//
// Copyright (c) 2021 bingxio（丙杺，黄菁）. All rights reserved.
//

// GNU General Public License, more to see file: LICENSE
// https://www.gnu.org/licenses

// The Drift Programming Language.
//
//          https://github.com/bingxio/drift
//
//          https://www.drift-lang.org/
//

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include <cstring>

// tokens
namespace token {
// total number of token for drift
constexpr int len = 56;
// token type
enum Kind {
  IDENT, // identifier literal
  NUM,   // number     literal
  STR,   // string     literal
  CHAR,  // char       literal
  FLOAT, // float      literal

  ADD, // +
  SUB, // -
  MUL, // *
  DIV, // /

  AS_ADD, // +=
  AS_SUB, // -=
  AS_MUL, // *=
  AS_DIV, // /=

  PLUS,  // ++
  MINUS, // --

  R_ARROW,        // ->
  L_ARROW,        // <-
  L_CURVED_ARROW, // <~

  DOLLAR, // $
  DOT,    // .
  COMMA,  // ,
  COLON,  // :
  EQ,     // =

  GREATER, // >
  LESS,    // <
  GR_EQ,   // >=
  LE_EQ,   // <=

  ADDR,    // &
  OR,      // |
  BANG,    // !
  BANG_EQ, // !=
  EQ_EQ,   // ==
           //    SINGLE_MARK,  // '
           //    DOUBLE_MARk,  // "
           //    OBLIQUE_MARK, // `

  L_BRACE,   // {
  R_BRACE,   // }
  L_PAREN,   // (
  R_PAREN,   // )
  L_BRACKET, // [
  R_BRACKET, // ]

  UNDERLINE, // _

  EFF, // end of file

  // keywords
  USE,
  DEF,
  PUB,
  RET,
  AND,
  END,
  IF,
  EF,
  NF,
  FOR,
  DO,
  OUT,
  TIN,
  NEW,
  MOD,
  AS
};

// returns a string of each type
std::string kindString[len] = {
    // NOLINT(cert-err58-cpp)
    "IDENT",
    "NUM",
    "STR",
    "CHAR",
    "FLOAT",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "AS_ADD",
    "AS_SUB",
    "AS_MUL",
    "AS_DIV",
    "PLUS",
    "MINUS",
    "R_ARROW",
    "L_ARROW",
    "L_CURVED_ARROW",
    "DOLLAR",
    "DOT",
    "COMMA",
    "COLON",
    "EQ",
    "GREATER",
    "LESS",
    "GR_EQ",
    "LE_EQ",
    "ADDR",
    "OR",
    "BANG",
    "BANG_EQ",
    "EQ_EQ",
    // "SINGLE_MARK",
    // "DOUBLE_MARK",
    // "OBLIQUE_MARK",
    "L_BRACE",
    "R_BRACE",
    "L_PAREN",
    "R_PAREN",
    "L_BRACKET",
    "R_BRACKET",
    "UNDERLINE",
    // end of file
    "EFF",
    // keywords
    "USE",
    "DEF",
    "PUB",
    "RET",
    "AND",
    "END",
    "IF",
    "EF",
    "NF",
    "FOR",
    "DO",
    "OUT",
    "TIN",
    "NEW",
    "MOD",
    "AS",
};

//  token structure
struct Token {
  // token type
  Kind kind = EFF;
  // token literal
  std::string literal;
  // line of source code
  int line = 0;
};

// format return token structure
std::string toString(const Token &token) {
  std::stringstream str;

  str << "<Token { Kind=";
  str << kindString[token.kind] << " Literal=\"";
  str << token.literal << "\" Line=";
  str << token.line << " }>";

  return str.str();
}

// keywords for drift
static std::map<std::string, Kind> keyword;

// 16 keywords
// initialize it when tokenizer
void initializeKeywords() {
  keyword["use"] = USE; // 1
  keyword["def"] = DEF; // 2
  keyword["pub"] = PUB; // 3
  keyword["ret"] = RET; // 4
  keyword["and"] = AND; // 5
  keyword["end"] = END; // 6
  keyword["if"] = IF;   // 7
  keyword["ef"] = EF;   // 8
  keyword["nf"] = NF;   // 9
  keyword["for"] = FOR; // 10
  keyword["do"] = DO;   // 11
  keyword["out"] = OUT; // 12
  keyword["tin"] = TIN; // 13
  keyword["new"] = NEW; // 14
  keyword["mod"] = MOD; // 15
  keyword["as"] = AS;   // 16
}

// return the corresponding keyword type according to the literal amount
Kind getKeyword(const std::string &literal) {
  auto i = keyword.find(literal);
  // search map
  if (i != keyword.end())
    return i->second;
  else
    return IDENT;
}
} // namespace token

// exceptions
namespace exp {
// total number of exceptions
constexpr int len = 12;
// exception type
enum Kind {
  // lexer
  UNKNOWN_SYMBOL, // unknown symbol
  CHARACTER_EXP,  // character is empty
  STRING_EXP,     // lost left or right mark
                  // PARSER
  UNEXPECTED,     // unexpected
  INVALID_SYNTAX, // invalid syntax
  INCREMENT_OP,   // left value increment operand
                  // SEMANTIC
  TYPE_ERROR,     // type error
  DIVISION_ZERO,  // div zero
  CANNOT_PUBLIC,  // can not to public
  ENUMERATION,    // whole body not definition of enum
  CALL_INHERIT,   // can only be with call expr
};

//  return a string of exception type
std::string kindString[len] = {
    // lexer
    "UNKNOWN_SYMBOL",
    "CHARACTER_EXP",
    "STRING_EXP",
    // parser
    "UNEXPECTED",
    "INVALID_SYNTAX",
    "INCREMENT_OP",
    // semantic
    "TYPE_ERROR",
    "DIVISION_ZERO",
    "CANNOT_PUBLIC",
    "ENUMERATION",
    "CALL_INHERIT",
}; // NOLINT(cert-err58-cpp)

// exception structure
class Exp : public std::exception {
private:
  // exception kind
  Kind kind;
  // exception message
  std::string message;
  // at exception line of source code
  int line;

public:
  explicit Exp(Kind kind, std::string message, int line) {
    this->kind = kind;
    this->message = std::move(message);
    this->line = line;
  }

  // return a string of exception structure
  std::string stringer();
};

// return a string of exception structure
std::string Exp::stringer() {
  std::stringstream str;

  str << "<Exception { Kind=";
  str << kindString[this->kind] << " Message=\"";
  str << this->message << "\" Line=";
  str << this->line << " }>";

  return str.str();
}
} // namespace exp

// lexer
namespace lexer {
// lexer structure
class Lexer {
private:
  // current character
  int position = 0;
  // current line
  int line = 1;
  // source code
  std::string source;

  // resolve identifier
  void lexIdent();
  // resolve digit
  void lexDigit();
  // resolve "xxx" string literal
  void lexString(bool longStr);
  // resolve 'x' character literal
  void lexChar();
  // resolve other symbol
  void lexSymbol();
  // return current char of resolve
  inline char now();
  // return next char of resolve
  char peek();
  // judge the current character and process the token
  bool peekEmit(token::Token *t,
                char c,              // current char
                token::Kind k,       // equal token kind
                const std::string &l // equal token literal
  );
  // return resolve is end
  inline bool isEnd();
  // return current char is identifier
  inline bool isIdent();
  // return current char is digit
  inline bool isDigit();
  // return current char is whitespace
  inline bool isSpace();
  // resolve to skip whitespace
  inline void skipWhitespace();
  // resolve to skip line comment
  inline void skipLineComment();

public:
  explicit Lexer(std::string source) : source(std::move(source)) {
    // initializer keywords map here
    token::initializeKeywords();
  }

  // final token list
  std::vector<token::Token> tokens;
  // start
  void tokenizer();
  // final to dissemble tokens list
  void dissembleTokens();
};

//  start
void Lexer::tokenizer() {
  while (!this->isEnd()) {
    // first to skip whitespace
    if (isSpace())
      skipWhitespace();
    // identifier
    else if (isIdent())
      this->lexIdent();
    // digit
    else if (isDigit())
      this->lexDigit();
    // string
    else if (now() == '"')
      this->lexString(false);
    // long strings
    else if (now() == '`')
      this->lexString(true);
    // character
    else if (now() == '\'')
      this->lexChar();
    // symbol
    else
      this->lexSymbol();
  }
  this->tokens.push_back(
      // resolve end insert EFF for end of file
      token::Token{token::EFF, "EFF", ++this->line});
}

// final to dissemble tokens list
void Lexer::dissembleTokens() {
  int i = 1;
  for (const auto &token : this->tokens)
    std::cout << i++ << " " + token::toString(token) << std::endl;
}

// return resolve is end
inline bool Lexer::isEnd() { return this->position >= this->source.length(); }

// resolve to skip whitespace
inline void Lexer::skipWhitespace() {
  while (!isEnd() && this->isSpace()) {
    if (now() == '\n')
      this->line++;
    this->position++;
  }
}

// resolve to skip line comment
inline void Lexer::skipLineComment() {
  while (!isEnd() && now() != '\n')
    this->position++;
}

// return current char is identifier
inline bool Lexer::isIdent() {
  return now() >= 'a' && now() <= 'z' || now() >= 'A' && now() <= 'Z' ||
         now() == '_';
}

// return current char is digit
inline bool Lexer::isDigit() { return now() >= '0' && now() <= '9'; }

// return current char is whitespace
inline bool Lexer::isSpace() {
  if (now() == ' ' || now() == '\r' || now() == '\t' || now() == '\n') {
    return true;
  }
  return false;
}

// return current char of resolve
inline char Lexer::now() { return this->source.at(this->position); }

// resolve identifier
void Lexer::lexIdent() {
  std::stringstream literal;

  while (!isEnd()) {
    if (isIdent())
      literal << now();
    else
      break;
    this->position++;
  }

  this->tokens.push_back(token::Token{// keyword or IDENT
                                      token::getKeyword(literal.str()),
                                      literal.str(), this->line});
}

// resolve digit
void Lexer::lexDigit() {
  std::stringstream literal;

  bool floating = false;

  while (!isEnd()) {
    if (isDigit() || now() == '.') {
      literal << now();

      if (now() == '.')
        floating = true;
    } else
      break;
    this->position++;
  }

  this->tokens.push_back(
      // number or float
      token::Token{floating ? token::FLOAT : token::NUM, literal.str(),
                   this->line});
}

// resolve string literal
void Lexer::lexString(bool longStr) {
  char cond = '"';
  // longer string
  if (longStr)
    cond = '`';

  std::stringstream literal;
  bool isEndFile = false;

  // skip left double quotation mark
  this->position++;

  while (!isEnd()) {
    if (now() == cond) {
      // end string
      this->position++;
      isEndFile = true;
      break;
    }
    if (now() == '\n' && !longStr) {
      throw exp::Exp(exp::STRING_EXP,
                     // long strings
                     "for long strings use the ` operator", this->line);
      break;
    }
    literal << now();
    this->position++;
  }

  // missing closing symbol
  if (!isEndFile)
    throw exp::Exp(exp::STRING_EXP, "missing closing symbol", this->line);

  this->tokens.push_back(
      // string
      token::Token{token::STR, literal.str(), this->line});
}

// resolve character
void Lexer::lexChar() {
  std::stringstream literal;

  // skip left single quotation mark
  this->position++;

  literal << now();

  if (peek() != '\'')
    // this character is empty
    throw exp::Exp(exp::CHARACTER_EXP, "wrong character", this->line);
  else
    // skip value and right single quotation mark
    this->position += 2;

  this->tokens.push_back(
      // character
      token::Token{token::CHAR, literal.str(), this->line});
}

// resolve symbols
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
    // to resolve skip comment
    else if (peek() == '/') {
      this->skipLineComment();
      // continue
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
    if (peekEmit(&tok, '~', token::L_CURVED_ARROW, "<~"))
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
    break;
  default:
    // what
    throw exp::Exp(exp::UNKNOWN_SYMBOL, "unknown symbol", this->line);
  }
  // skip current single symbol
  this->position++;
  this->tokens.push_back(tok);
}

// return next char of resolve
char Lexer::peek() {
  if (position + 1 >= source.length())
    return -1;
  else
    return source.at(position + 1);
}

// judge the current character and process the token
bool Lexer::peekEmit(token::Token *t, char c, token::Kind k,
                     const std::string &l) {
  if (peek() == c) {
    t->kind = k;
    t->literal = l;
    // advance
    this->position++;
    //
    return true;
  } else
    return false;
}
} // namespace lexer

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
  EXPR_LITERAL,     // literal
  EXPR_BINARY,      // T1 <OP> T2
  EXPR_GROUP,       // (EXPR)
  EXPR_UNARY,       // <OP>EXPR
  EXPR_NAME,        // IDENT
  EXPR_CALL,        // EXPR(<EXPR>..)
  EXPR_GET,         // EXPR.NAME
  EXPR_SET,         // EXPR.NAME = EXPR
  EXPR_ASSIGN,      // EXPR = EXPR
  EXPR_ARRAY,       // [<EXPR>..]
  EXPR_MAP,         // {K1: V1, K2: V2}
  EXPR_TUPLE,       // (<EXPR>..)
  EXPR_INDEX,       // EXPR[EXPR]
                    // statement
  STMT_EXPR,        // EXPR
  STMT_VAR,         // VAR
  STMT_BLOCK,       // BLOCK
  STMT_IF,          // IF
  STMT_FOR,         // FOR
  STMT_DO,          // DO
  STMT_OUT,         // OUT
  STMT_TIN,         // TIN
  STMT_FUNC,        // FUNC
  STMT_WHOLE,       // CLASS | ENUM
  STMT_NEW,         // NEW
  STMT_AND,         // AND
  STMT_MOD,         // MOD
  STMT_USE,         // USE
  STMT_RET,         // RET
  STMT_ENUM,        // ENUM
  STMT_INHERIT,     // <- <name> + <name>..
  STMT_CALLINHERIT, // <~ expr
  STMT_INTERFACE,   // INTERFACE
  STMT_PUB,         // PUB
};

// K1: V1 | K1 + K2: V2
using Arg = std::map<token::Token *, Type *>;

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

  bool selfIncrement; // ++
  bool selfDecrement; // --

  bool prefix; // prefix, calc it first

  explicit NameExpr(token::Token tok,
                    bool x = false, // increment
                    bool y = false, // decrement
                    bool z = false  // prefix
  ) {
    this->token = std::move(tok);

    this->selfIncrement = x;
    this->selfDecrement = y;

    this->prefix = z;
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<NameExpr { Token='" << token.literal
        << "' Increment=" << selfIncrement << " Decrement=" << selfDecrement
        << " Prefix=" << prefix << " }>";

    return str.str();
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
        str << i->stringer() << ", ";
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
        str << i->stringer() << ", ";
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
            << ", ";
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
        str << i->stringer() << ", ";
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

  explicit IfStmt(Expr *cond, BlockStmt *then,
                  std::map<Expr *, BlockStmt *> eif, BlockStmt *nai) {
    this->condition = cond;
    this->ifBranch = then;
    this->efBranch = std::move(eif);
    this->nfBranch = nai;
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
            << ", ";
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

  explicit OutStmt(Expr *e) : expr(e) {}

  std::string stringer() override {
    return "<OutStmt { Expr=" + expr->stringer() + " }>";
  }

  Kind kind() override { return STMT_OUT; }
};

// tin <expr>
class TinStmt : public Stmt {
public:
  Expr *expr;

  explicit TinStmt(Expr *e) : expr(e) {}

  std::string stringer() override {
    return "<TinStmt { Expr=" + expr->stringer() + " }>";
  }

  Kind kind() override { return STMT_TIN; }
};

// new <name>{ K1: V1, K2: V2 }
class NewStmt : public Stmt {
public:
  token::Token name;
  std::map<token::Token *, Expr *> builder;

  explicit NewStmt(token::Token name, std::map<token::Token *, Expr *> b) {
    this->name = std::move(name);
    this->builder = b;
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<NewStmt { Name='" << name.literal << "' ";
    if (!builder.empty()) {
      str << " Builder=";

      for (auto &i : builder)
        str << "K : " << i.first->literal << ", V : " << i.second->stringer()
            << ", ";
    }

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_NEW; }
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
  token::Token *as = nullptr;

  // use <name>
  explicit UseStmt(token::Token name) { this->name = std::move(name); }

  // use <name> as <name>
  explicit UseStmt(token::Token name, token::Token *as) {
    this->name = std::move(name);
    this->as = as;
  }

  std::string stringer() override {
    std::stringstream str;

    str << "<UseStmt { Name='" << name.literal;
    if (as != nullptr)
      str << "' As='" << as->literal;

    str << "' }>";
    return str.str();
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

// and -> <name> <block> end
class AndStmt : public Stmt {
public:
  token::Token name; // alias name
  BlockStmt *block;  // block

  explicit AndStmt(token::Token name, BlockStmt *block) : block(block) {
    this->name = std::move(name);
  }

  std::string stringer() override {
    return "<AndStmt Name='" + name.literal + "' Block=" + block->stringer() +
           " }>";
  }

  Kind kind() override { return STMT_AND; }
};

// def (<param>..) <name> -> <ret>
//     <block>
// end
class FuncStmt : public Stmt {
public:
  Arg arguments; // args

  token::Token name; // name
  Type *ret;         // return
  BlockStmt *block;  // body

  explicit FuncStmt(Arg args, token::Token name, Type *ret, BlockStmt *block) {
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
        str << "'" << i->literal << "', ";
      }
      str << ")";
    } else
      str << "()";

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_INHERIT; }
};

// <~ expr
class CallInheritStmt : public Stmt {
public:
  Expr *expr;

  int line; // line of call inherit statement

  explicit CallInheritStmt(int line, Expr *e) : expr(e) { this->line = line; }

  std::string stringer() override {
    return "<CallInheritStmt { Line=" + std::to_string(line) +
           " Expr=" + expr->stringer() + " }>";
  }

  Kind kind() override { return STMT_CALLINHERIT; }
};

// def (<param>..) *<name> -> <ret>
class InterfaceStmt : public Stmt {
public:
  Arg arguments; // arguments

  token::Token name; // name
  Type *ret;         // return

  explicit InterfaceStmt(Arg args, token::Token name, Type *ret) {
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
//
// TODO: semantic analysis
//
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
      for (auto &i : field)
        str << i->literal << ", ";
      str << ")";
    }

    str << " }>";
    return str.str();
  }

  Kind kind() override { return STMT_ENUM; }
};

// pub
class PubStmt : public Stmt {
public:
  // line of pub statement
  int line;

  PubStmt(int line) { this->line = line; }

  std::string stringer() override {
    return "<PubStmt { Line=" + std::to_string(line) + " } >";
  }

  Kind kind() override { return STMT_PUB; }
};
} // namespace ast

// parser
namespace parser {
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
  inline token::Token *look(bool previous);
  // look current token, if equal to peek next
  bool look(token::Kind kind);
  // look current token, do nothing
  inline token::Token look();
  // look the appoint position of tokens
  token::Token look(int i);
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
  //
  ast::Type *type();
  // throw an exception
  inline void error(exp::Kind kind, std::string message);

public:
  // parser constructor
  explicit Parser(std::vector<token::Token> tokens) {
    // tokens
    this->tokens = std::move(tokens);
  }

  // final stmts list
  std::vector<ast::Stmt *> statements;
  // do parsing
  void parse();
  // final to dissemble statement list
  void dissembleStmts();
};

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
    std::cout << "Empty Statements" << std::endl;
    return;
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
         look(token::AS_DIV)) {
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

      this->position++; // skip name token
      expr = new ast::GetExpr(expr, name);
      // index for array
    } else if (look(token::L_BRACKET)) {
      // empty index
      if (look(token::R_BRACKET))
        error(exp::UNEXPECTED, "null index");
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
  if (look(token::IDENT)) {
    token::Token tok = previous();
    // ++ | --
    if (look(token::PLUS) || look(token::MINUS))
      // self increment
      return new ast::NameExpr(tok, previous().kind == token::PLUS,
                               previous().kind == token::MINUS, false);
    return new ast::NameExpr(tok);
  }
  // name expr of ++ or -- operators
  if (look(token::PLUS) || look(token::MINUS)) {
    token::Token op = previous();
    //
    if (look(token::IDENT))
      return new ast::NameExpr(previous(), op.kind == token::PLUS,
                               op.kind == token::MINUS, true);
    else
      error(exp::INCREMENT_OP,
            "increment operand can only be performed on name");
  }
  // group expr
  if (look(token::L_PAREN)) {
    // vector for tuple and group expression
    std::vector<ast::Expr *> elem;
    // empty tuple expr
    if (look(token::R_PAREN))
      return new ast::TupleExpr(elem);
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
    if (look(token::R_BRACE))
      return new ast::MapExpr(elem);

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
      ast::Arg args;
      // name
      token::Token name;
      // return
      ast::Type *ret = nullptr;
      // cache multiple parameters
      std::vector<token::Token *> temp;
      //
      bool interfaceStmt = false;

      while (!look(token::R_PAREN)) {
        if (!look(token::IDENT)) {
          error(exp::UNEXPECTED, "argument name muse be an identifier");
        }
        // K
        token::Token *K = look(true); // address of token
        // handle multiparameter
        while (look(token::ADD)) {
          if (!look(token::IDENT)) {
            error(exp::UNEXPECTED, "argument name muse be an identifier");
          } else {
            temp.push_back(K);          // previous, left of the plus sign
            temp.push_back(look(true)); // address of token
          }
        }
        if (!look(token::COLON)) {
          error(exp::UNEXPECTED, "expect ':' after parameter name");
        }
        // handle multiparameter
        if (temp.empty()) {
          // no
          args.insert(std::make_pair(K, this->type()));
        } else {
          // multip
          ast::Type *T = this->type();

          for (auto i : temp) {
            args.insert(std::make_pair(i, T));
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
        this->position++; // skip name of interface
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

      if (interfaceStmt)
        return new ast::InterfaceStmt(args, name, ret);
      else
        return new ast::FuncStmt(args, name, ret, this->block(token::END));
      //
      break;
      // whole
    } else {
      ast::Stmt *inherit = nullptr;

      // inherit
      if (look().kind == token::L_ARROW) {
        inherit = this->stmt();
      }
      return new ast::WholeStmt(previous(), inherit, this->block(token::END));
    }
    break;
    // if
  case token::IF: {
    this->position++;
    // if condition
    ast::Expr *condition = this->expr();
    // if then branch
    ast::BlockStmt *thenBranch = this->block(token::EF, token::END, token::NF);

    std::map<ast::Expr *, ast::BlockStmt *> elem;

    while (previous().kind == token::EF) {
      ast::Expr *eifCondition = this->expr();
      ast::BlockStmt *eifBranch = this->block(token::EF, token::END, token::NF);
      //
      elem.insert(std::make_pair(eifCondition, eifBranch));
    }

    ast::BlockStmt *nafBranch = nullptr;

    if (previous().kind == token::NF) {
      nafBranch = this->block(token::END);
    }
    return new ast::IfStmt(condition, thenBranch, elem, nafBranch);
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
    return new ast::OutStmt(this->expr());
    break;
    // tin in loop
    // tin <expr>
  case token::TIN:
    this->position++;
    //
    return new ast::TinStmt(this->expr());
    break;
    // new <name>{K1: V1, K2, V2}
  case token::NEW: {
    this->position++;

    if (!look(token::IDENT)) {
      error(exp::UNEXPECTED, "name of new muse be an identidier");
    }
    // name of new
    token::Token name = previous();
    //
    if (!look(token::L_BRACE)) {
      error(exp::UNEXPECTED, "expect '{' to new a class with fields");
    }
    // builder fields
    std::map<token::Token *, ast::Expr *> builder;
    // no field
    if (look(token::R_BRACE)) {
      return new ast::NewStmt(name, builder);
    }
    // parsing fields
    while (true) {
      token::Token field = look();
      /**
       * variable on the stack cannot be used within a loop
       *
       * the location where the current token is stored
       */
      int tempPosition = this->position;
      //
      if (field.kind != token::IDENT) {
        error(exp::UNEXPECTED, "field muse be an identifier");
      }
      // skip left name
      this->position++;
      // :
      if (!look(token::COLON)) {
        error(exp::UNEXPECTED, "expect ':' after field");
      }
      // field
      builder.insert(
          // directly based on location
          std::make_pair(&this->tokens.at(tempPosition), this->expr()));

      if (look(token::COMMA)) {
        continue;
      }
      if (look(token::R_BRACE)) {
        break;
      }
      error(exp::UNEXPECTED, "expect ',' of '}' after field of class");
    }
    return new ast::NewStmt(name, builder);
  } break;
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
  // call of super class
  // <~ expr
  case token::L_CURVED_ARROW: // TODO: can only call method
    this->position++;
    //
    return new ast::CallInheritStmt(look().line, this->expr());
    break;
  // pub
  case token::PUB:
    this->position++;
    //
    return new ast::PubStmt(look().line);
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
ast::Type *Parser::type() {
  token::Token now = this->look();
  // type
  if (now.kind == token::IDENT) {
    // skip type ident
    this->position++;
    // T1
    if (now.literal == S_INT)
      return new ast::Int();
    // T2
    if (now.literal == S_FLOAT)
      return new ast::Float();
    // T3
    if (now.literal == S_STR)
      return new ast::Str;
    // T4
    if (now.literal == S_CHAR)
      return new ast::Char();
    // T5
    if (now.literal == S_BOOL)
      return new ast::Bool;
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
} // namespace parser

// semantic
namespace semantic {
// analysis
class Analysis {
private:
  int position = 0;
  // stmts
  std::vector<ast::Stmt *> *statements;

  // return the kind of current statement
  ast::Kind look() { return statements->at(position)->kind(); }

  // return the current statement
  ast::Stmt *now() { return statements->at(position); }

  // peek next statement
  //
  // [Q]: how use lvalue to quote statement
  //
  ast::Stmt *peek() {
    if (position + 1 >= statements->size()) {
      return nullptr;
    } else {
      return statements->at(position + 1);
    }
  }

  // throw semantic analysis exception
  void error(exp::Kind k, std::string message, int line) {
    throw exp::Exp(k, message, line);
  }

public:
  explicit Analysis(std::vector<ast::Stmt *> *stmts) {
    this->statements = stmts;
  }

  // analysis
  void analysisAst() {
    while (position < statements->size()) {
      switch (look()) {
        //
      case ast::STMT_EXPR: {
        ast::ExprStmt *stmt = static_cast<ast::ExprStmt *>(now());
        ast::Expr *expr = static_cast<ast::Expr *>(stmt->expr);
        // expression
        this->analysisExpr(expr);
      } break;
        //           if (l->token.kind == token::FLOAT) {
        //             //
        //             // string -> float + 0.5 -> int
        //             //
        //             int x = ((int)(std::stof(l->token.literal) + 0.5));

        //             l->token.literal = std::to_string(x);
        //             l->token.kind = token::NUM;
        //           }
        //           if (l->token.kind == token::CHAR) {
        //             //
        //             // char -> int
        //             //
        //             int x = l->token.literal.data()[0];

        //             l->token.literal = std::to_string(x);
        //             l->token.kind = token::NUM;
        //           }
      case ast::STMT_PUB: {
        ast::PubStmt *stmt = static_cast<ast::PubStmt *>(now());
        ast::Stmt *next = this->peek();

        if (next != nullptr) {
          //
          switch (next->kind()) {
          case ast::STMT_VAR:
          case ast::STMT_FUNC:
          case ast::STMT_WHOLE:
          case ast::STMT_INTERFACE:
            break;
          default:
            error(exp::CANNOT_PUBLIC, "statement cannot be public", stmt->line);
          }
        }
      } break;
      case ast::STMT_WHOLE: {
        ast::WholeStmt *stmt = static_cast<ast::WholeStmt *>(now());

        if (stmt->body->block.empty()) {
          break;
        }
        ast::Stmt *f = stmt->body->block.at(0); // first statement

        // just a ident of expression statement
        //
        // enumeration
        //
        if (f->kind() == ast::STMT_EXPR) {
          //
          if (stmt->inherit != nullptr) {
            error(exp::ENUMERATION, "enumeration type cannot be inherited",
                  stmt->name.line);
          }

          ast::ExprStmt *expr = static_cast<ast::ExprStmt *>(f);
          std::vector<token::Token *> fields;

          for (auto &i : stmt->body->block) {
            //
            if (i->kind() != ast::STMT_EXPR) {
              error(exp::ENUMERATION, "whole is an enumeration type",
                    stmt->name.line);
            }
            ast::ExprStmt *expr = static_cast<ast::ExprStmt *>(i);
            if (expr->expr->kind() != ast::EXPR_NAME) {
              error(exp::ENUMERATION, "whole is an enumeration type",
                    stmt->name.line);
            }

            ast::NameExpr *name = static_cast<ast::NameExpr *>(expr->expr);
            // push to enumeration structure
            fields.push_back(&name->token);
          }
          // replace new statement into vector
          //
          // [Q]: iterator and std::replace
          //
          ast::Stmt *n = new ast::EnumStmt(stmt->name, fields);
          std::replace(std::begin(*statements), std::end(*statements), now(),
                       n);

          std::cout << "[Semantic analysis replace " << position + 1
                    << "]: WholeStmt -> " << n->stringer() << std::endl;
        }
        // normal whole statement
        // if hinder of statements include name expr to throw an error
        else {
          for (auto &i : stmt->body->block) {
            if (i->kind() == ast::STMT_EXPR) {
              error(exp::ENUMERATION,
                    "it an whole statement but contains some other value",
                    stmt->name.line);
            }
          }
        }
      } break;
      case ast::STMT_CALLINHERIT: {
        ast::CallInheritStmt *stmt = static_cast<ast::CallInheritStmt *>(now());

        if (stmt->expr->kind() != ast::EXPR_CALL) {
          error(exp::CALL_INHERIT,
                "only methods of the parent class can be called", 2);
        }
      } break;
      default:
        break;
      }
      this->position++;
    }
  }

  // expression
  void analysisExpr(ast::Expr *expr) {
    using namespace token;

    switch (expr->kind()) {
    case ast::EXPR_BINARY: {
      ast::BinaryExpr *binary = static_cast<ast::BinaryExpr *>(expr);

      Token l = (static_cast<ast::LiteralExpr *>(binary->left))->token;
      Token r = (static_cast<ast::LiteralExpr *>(binary->right))->token;

      switch (binary->op.kind) {
      case ADD:    // +
      case SUB:    // -
      case AS_ADD: // +=
      case AS_SUB: // -=
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
      case DIV:    // /
      case AS_DIV: // /=
        if (l.kind == STR || l.kind == CHAR || r.kind == STR ||
            r.kind == CHAR) {
          error(exp::TYPE_ERROR, "unsupported operand", l.line);
        }
        if (r.kind == NUM) {
          // convert, keep floating point numbers
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
    }
  }
};
}; // namespace semantic

// run source code
void run(std::string source) {
  try {
    // lexer
    auto lex = new lexer::Lexer(source);

    lex->tokenizer();
    lex->dissembleTokens();

    // parser
    auto parser = new parser::Parser(lex->tokens);

    parser->parse();
    parser->dissembleStmts();

    // semantic
    auto semantic = new semantic::Analysis(&parser->statements);
    semantic->analysisAst();
    //
  } catch (exp::Exp &e) {
    std::cout << "\033[31m" << e.stringer() << "\033[0m" << std::endl;
    return;
  }
}

// FILE mode
void runFile(const char *path) {
  std::ifstream stream;
  stream.open(path);

  if (stream.fail()) {
    std::cout << "failed to open file" << std::endl;
    return;
  }

  std::string source((std::istreambuf_iterator<char>(stream)),
                     (std::istreambuf_iterator<char>()));
  run(source);

  stream.close();
}

// REPL mode
void repl() {
  char *line = (char *)malloc(1024);
  std::cout << "\n\tDrift 0.0.1 (repl mode, Feb 20 2021, 15:42) - Hello!! (:\n"
            << std::endl;
  std::cout << "\t\t     https://www.drift-lang.org/\n" << std::endl;

  while (true) {
    std::cout << "ft >> ";
    std::cin.getline(line, 1024);

    if (strlen(line) == 0) {
      continue;
    }
    run(line);
  }
}

int main(int argc, char **argv) {
  if (argc == 2)
    runFile(argv[1]);
  else
    repl();
  return 0;
}

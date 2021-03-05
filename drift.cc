//
// Copyright (c) 2021 bingxio（丙杺，黄菁）. All rights reserved.
//

// GNU General Public License, more to see file: LICENSE
// https://www.gnu.org/licenses

//          THE DRIFT PROGRAMMING LANGUAGE
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
#include <stack>
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
      "IDENT",     "NUM",       "STR",
      "CHAR",      "FLOAT",     "ADD",
      "SUB",       "MUL",       "DIV",
      "AS_ADD",    "AS_SUB",    "AS_MUL",
      "AS_DIV",    "PLUS",      "MINUS",
      "R_ARROW",   "L_ARROW",   "L_CURVED_ARROW",
      "DOLLAR",    "DOT",       "COMMA",
      "COLON",     "EQ",        "GREATER",
      "LESS",      "GR_EQ",     "LE_EQ",
      "ADDR",      "OR",        "BANG",
      "BANG_EQ",   "EQ_EQ",     "L_BRACE",
      "R_BRACE",   "L_PAREN",   "R_PAREN",
      "L_BRACKET", "R_BRACKET", "UNDERLINE",
      "EFF",       "USE",       "DEF",
      "PUB",       "RET",       "AND",
      "END",       "IF",        "EF",
      "NF",        "FOR",       "DO",
      "OUT",       "TIN",       "NEW",
      "MOD",       "AS",
  };

  //  token structure
  struct Token {
    // token type
    Kind kind = EFF;
    // token literal
    std::string literal;
    // line of source code
    int line;
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
    // LEXER
    UNKNOWN_SYMBOL, // unknown symbol
    CHARACTER_EXP,  // character is empty
    STRING_EXP,     // lost left or right mark
    // PARSER
    UNEXPECTED,     // unexpected
    INVALID_SYNTAX, // invalid syntax
    INCREMENT_OP,   // left value increment operand
    // SEMANTIC
    TYPE_ERROR,    // type error
    DIVISION_ZERO, // div zero
    CANNOT_PUBLIC, // can not to public
    ENUMERATION,   // whole body not definition of enum
    CALL_INHERIT,  // can only be with call expr
    //
    RUNTIME_ERROR,
  };

  //  return a string of exception type
  std::string kindString[len] = {
      "UNKNOWN_SYMBOL", "CHARACTER_EXP", "STRING_EXP",   "UNEXPECTED",
      "INVALID_SYNTAX", "INCREMENT_OP",  "TYPE_ERROR",   "DIVISION_ZERO",
      "CANNOT_PUBLIC",  "ENUMERATION",   "CALL_INHERIT", "RUNTIME_ERROR"};

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
    // resolve to skip block comment
    inline void skipBlockComment();

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
      if (isSpace()) skipWhitespace();
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
      if (now() == '\n') this->line++;
      this->position++;
    }
  }

  // resolve to skip line comment
  inline void Lexer::skipLineComment() {
    while (!isEnd() && now() != '\n') this->position++;
  }

  // resolve to skip block comment
  inline void Lexer::skipBlockComment() {
    while (!isEnd()) {
      if (now() == '*' && peek() == '/') {
        this->position += 2;
        break;
      }
      this->position++;
    }
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

        if (now() == '.') floating = true;
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
    if (longStr) cond = '`';

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

    // add judgment character
    // used to judge long characters at compile time
    literal << cond;

    this->tokens.push_back(
        // string
        token::Token{token::STR, literal.str(), this->line});
  }

  // resolve character
  void Lexer::lexChar() {
    std::stringstream literal;

    // skip left single quotation mark
    this->position++;
    if (isEnd())
      throw exp::Exp(exp::CHARACTER_EXP, "wrong character", this->line);

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
      case '(': tok.kind = token::L_PAREN; break;
      case ')': tok.kind = token::R_PAREN; break;
      case '{': tok.kind = token::L_BRACE; break;
      case '}': tok.kind = token::R_BRACE; break;
      case '[': tok.kind = token::L_BRACKET; break;
      case ']': tok.kind = token::R_BRACKET; break;
      case ':': tok.kind = token::COLON; break;
      case '+':
        if (peekEmit(&tok, '=', token::AS_ADD, "+=")) break;
        if (peekEmit(&tok, '+', token::PLUS, "++"))
          break;
        else
          tok.kind = token::ADD;
        break;
      case '-':
        if (peekEmit(&tok, '>', token::R_ARROW, "->")) break;
        if (peekEmit(&tok, '-', token::MINUS, "--")) break;
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
        if (peekEmit(&tok, '=', token::AS_DIV, "/=")) break;
        // to resolve skip comment
        else if (peek() == '/') {
          this->skipLineComment();
          // continue
          return;
        }
        // block comment
        else if (peek() == '*') {
          this->skipBlockComment();
          return;
        } else
          tok.kind = token::DIV;
        break;
      case '$': tok.kind = token::DOLLAR; break;
      case '.': tok.kind = token::DOT; break;
      case ',': tok.kind = token::COMMA; break;
      case '>':
        if (peekEmit(&tok, '=', token::GR_EQ, ">="))
          break;
        else
          tok.kind = token::GREATER;
        break;
      case '<':
        if (peekEmit(&tok, '=', token::LE_EQ, "<=")) break;
        if (peekEmit(&tok, '-', token::L_ARROW, "<-")) break;
        if (peekEmit(&tok, '~', token::L_CURVED_ARROW, "<~"))
          break;
        else
          tok.kind = token::LESS;
        break;
      case '&': tok.kind = token::ADDR; break;
      case '|': tok.kind = token::OR; break;
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

    std::string stringer() override {
      return "<Array T=" + T->stringer() + " >";
    }

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

    std::string stringer() override {
      return "<Tuple T=" + T->stringer() + " >";
    }

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

        for (auto &i : arguments) str << i->stringer() << " ";
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
        for (auto &i : elements) str << i->stringer() << " ";
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
          str << "K : " << i.first->stringer()
              << ", V : " << i.second->stringer() << " ";
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
        for (auto &i : elements) str << i->stringer() << " ";
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
        return "<NewExpr { Name='" + name.literal + "' Builder=NONE }>";
      }
      std::stringstream str;

      str << "<NewExpr { Name='" << name.literal << "'";
      if (!builder.empty()) {
        str << " Builder=(";

        for (auto i : builder) {
          str << "K : '" << i.first->literal
              << "' V : " << i.second->stringer();
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

        for (auto &i : block) str << i->stringer() << " ";
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
                    std::map<Expr *, BlockStmt *> ef, BlockStmt *nf) {
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
      if (ifBranch != nullptr) str << " IfBranch=" << ifBranch->stringer();
      if (!efBranch.empty()) {
        str << " EfBranch=";

        for (auto &i : efBranch)
          str << "K : " << i.first->stringer()
              << ", V : " << i.second->stringer() << " ";
      }
      if (nfBranch != nullptr) str << " NfBranch=" << nfBranch->stringer();

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

  // tin <expr>
  class TinStmt : public Stmt {
  public:
    Expr *expr;

    TinStmt() { this->expr = nullptr; }

    explicit TinStmt(Expr *e) : expr(e) {}

    std::string stringer() override {
      if (expr == nullptr) {
        return "<TinStmt {}>";
      }
      return "<TinStmt { Expr=" + expr->stringer() + " }>";
    }

    Kind kind() override { return STMT_TIN; }
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
      if (as != nullptr) str << "' As='" << as->literal;

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

    explicit FuncStmt(Arg args, token::Token name, Type *ret,
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

  // pub
  class PubStmt : public Stmt {
  public:
    int line;
    Stmt *stmt;

    PubStmt(int line, Stmt *stmt) {
      this->line = line;
      this->stmt = stmt;
    }

    std::string stringer() override {
      return "<PubStmt { Line=" + std::to_string(line) +
             " Stmt=" + stmt->stringer() + " } >";
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
                temp.push_back(K); // previous,
                // left of the
                // plus sign
                temp.push_back(look(true)); // address
                                            // of token
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

          if (interfaceStmt)
            return new ast::InterfaceStmt(args, name, ret);
          else
            return new ast::FuncStmt(args, name, ret, this->block(token::END));
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
  ast::Type *Parser::type() {
    token::Token now = this->look();
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
    inline ast::Kind look() { return statements->at(position)->kind(); }

    // return the current statement
    inline ast::Stmt *now() { return statements->at(position); }

    // throw semantic analysis exception
    void error(exp::Kind k, std::string message, int line) {
      throw exp::Exp(k, message, line);
    }

  public:
    explicit Analysis(std::vector<ast::Stmt *> *stmts) {
      this->statements = stmts;

      while (position < statements->size()) {
        this->analysisStmt(now());
        this->position++;
      }
    }

    // statement
    void analysisStmt(ast::Stmt *stmt) {
      switch (stmt->kind()) {
        case ast::STMT_EXPR: {
          ast::ExprStmt *e = static_cast<ast::ExprStmt *>(stmt);
          ast::Expr *expr = static_cast<ast::Expr *>(e->expr);
          // expression
          this->analysisExpr(expr);
        } break;
        //
        case ast::STMT_PUB: {
          ast::PubStmt *p = static_cast<ast::PubStmt *>(stmt);

          switch (p->stmt->kind()) {
            case ast::STMT_VAR:       // defintin
            case ast::STMT_FUNC:      // function
            case ast::STMT_WHOLE:     // whole
            case ast::STMT_INTERFACE: // interface
              break;
            default:
              error(exp::CANNOT_PUBLIC, "statement cannot be public", p->line);
          }

          // if its whole statement and must to analysis body
          // for example it contains a new whole statement inside
          if (p->stmt->kind() == ast::STMT_WHOLE) this->analysisStmt(p->stmt);
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
                error(exp::ENUMERATION, "whole is an enumeration type",
                      w->name.line);
              }
              ast::ExprStmt *pStmt = static_cast<ast::ExprStmt *>(i);
              if (pStmt->expr->kind() != ast::EXPR_NAME) {
                error(exp::ENUMERATION, "whole is an enumeration type",
                      w->name.line);
              }

              ast::NameExpr *name = static_cast<ast::NameExpr *>(pStmt->expr);
              // push to enumeration
              // structure
              fields.push_back(&name->token);
            }
            // replace new statement into
            ast::Stmt *n = new ast::EnumStmt(w->name, fields);
            std::replace(std::begin(*statements), std::end(*statements), now(),
                         n);
            std::cout << "\033[33m[Semantic analysis replace " << position + 1
                      << "]\033[0m: WholeStmt -> " << n->stringer()
                      << std::endl;
          }
          // normal whole statement if hinder of statements include name expr
          // to throw an error
          else {
            for (auto &i : w->body->block) {
              if (i->kind() == ast::STMT_EXPR) {
                error(exp::ENUMERATION,
                      "it an whole statement but contains some other value",
                      w->name.line);
              }
            }
          }
        } break;
        //
        case ast::STMT_CALLINHERIT: {
          ast::CallInheritStmt *c = static_cast<ast::CallInheritStmt *>(stmt);

          if (c->expr->kind() != ast::EXPR_CALL) {
            error(exp::CALL_INHERIT,
                  "only methods of the parent class can be called", 2);
          }
        } break;
        //
        default: break;
      }
    }

    // expression
    void analysisExpr(ast::Expr *expr) {
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
              if (l.kind == STR || l.kind == CHAR || r.kind == STR ||
                  r.kind == CHAR) {
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
  }; // namespace semantic
};   // namespace semantic

struct Entity;

// bytecode
namespace byte {
  // total number of bytecodes
  constexpr int len = 50;
  // bytecode type
  enum Code {
    CONST,  // O
    ASSIGN, // ASSIGN
    STORE,  // V
    LOAD,   // V
    INDEX,  // IDEX
    GET,    // GET
    SET,    // SET
    CALL,   // CALL
    CALL_I, // CALL_I
    ORIG,   // ORIG
    NAME,   // NAME
    NEW,    // NEW
    FUNC,   // FUNC
    CHA,    // CHA
    END,    // END
    WHOLE,  // WHOLE
    ENUM,   // ENUM

    PUB, // PUB
    MOD, // MOD
    USE, // USE
    UAS, // UAS

    B_ARR, // ARRAY
    B_TUP, // TUPLE
    B_MAP, // MAP

    INCR,   // DECR
    DECR,   // INCR
    P_INCR, // P_INCR
    P_DECR, // P_DECR

    // INFIX
    ADD,   // +
    SUB,   // -
    MUL,   // *
    DIV,   // /
    A_ADD, // +=
    A_SUB, // -=
    A_MUL, // *=
    A_DIV, // /=
    GR,    // >
    LE,    // <
    GR_E,  // >=
    LE_E,  // <=
    E_E,   // ==
    N_E,   // !=
    AND,   // &
    OR,    // |

    // PREFIX
    BANG, // !
    NOT,  // -

    JUMP,   // JUMP
    F_JUMP, // F_JUMP
    T_JUMP,

    RET,
  };

  // return a string of bytecode
  std::string codeString[len] = {
      "CONST", "ASSIGN", "STORE", "LOAD", "INDEX", "GET",    "SET",
      "CALL",  "CALL_I", "ORIG",  "NAME", "NEW",   "FUNC",   "CHA",
      "END",   "WHOLE",  "ENUM",  "PUB",  "MOD",   "USE",    "UAS",
      "B_ARR", "B_TUP",  "B_MAP", "INCR", "DECR",  "P_INCR", "P_DECR",
      "ADD",   "SUB",    "MUL",   "DIV",  "A_ADD", "A_SUB",  "A_MUL",
      "A_DIV", "GR",     "LE",    "GR_E", "LE_E",  "E_E",    "N_E",
      "AND",   "OR",     "BANG",  "NOT",  "JUMP",  "F_JUMP", "T_JUMP",
      "RET",
  };

  // compare two bytecode
  bool compare(Code a, Code b) { return a == b; }
}; // namespace byte

// object
namespace object {
  // kind
  enum Kind {
    INT,
    FLOAT,
    STR,
    CHAR,
    BOOL,
    ARRAY,
    TUPLE,
    MAP,
    ENUM,
    FUNC,
    WHOLE
  };

  // object abstract
  class Object {
  public:
    // return a string of dis object
    virtual std::string stringer() = 0;
    // return the kind of object
    virtual Kind kind() = 0;
  };

  // INT
  class Int : public Object {
  public:
    int value;

    Int(int v) : value(v) {}

    std::string stringer() override {
      return "<Int " + std::to_string(value) + ">";
    }

    Kind kind() override { return INT; }
  };

  // FLOAT
  class Float : public Object {
  public:
    double value;

    Float(float v) : value(v) {}

    std::string stringer() override {
      return "<Float " + std::to_string(value) + ">";
    }

    Kind kind() override { return FLOAT; }
  };

  // STR
  class Str : public Object {
  public:
    std::string value;
    bool longer = false;

    Str(std::string v) : value(v) {}

    Str(std::string v, bool longer) : value(v), longer(longer) {
      value.pop_back(); // long character judgment end, delete judgment
                        // char
    }

    std::string stringer() override {
      if (longer) {
        return "<Str LONGER>";
      }
      return "<Str \"" + value + "\">";
    }

    Kind kind() override { return STR; }
  };

  // CHAR
  class Char : public Object {
  public:
    char value;

    Char(char v) : value(v) {}

    std::string stringer() override {
      std::stringstream str;

      str << "<Char '" << value << "'>";

      return str.str();
    }

    Kind kind() override { return CHAR; }
  };

  // BOOL
  class Bool : public Object {
  public:
    bool value;

    Bool(bool v) : value(v) {}

    std::string stringer() override { return "<Bool>"; }

    Kind kind() override { return BOOL; }
  };

  // ARRAY
  class Array : public Object {
  public:
    std::vector<object::Object *> elements;

    Array(std::vector<object::Object *> v) : elements(v) {}

    std::string stringer() override { return "<Array>"; }

    Kind kind() override { return ARRAY; }
  };

  // TUPLE
  class Tuple : public Object {
  public:
    std::vector<object::Object *> elements;

    Tuple(std::vector<object::Object *> v) : elements(v) {}

    std::string stringer() override { return "<Tuple>"; }

    Kind kind() override { return TUPLE; }
  };

  // MAP
  class Map : public Object {
  public:
    std::map<std::string, object::Object *> value;

    Map(std::map<std::string, object::Object *> v) : value(v) {}

    std::string stringer() override { return "<Map>"; }

    Kind kind() override { return MAP; }
  };

  // ENUM
  class Enum : public Object {
  public:
    std::string name;
    std::map<int, std::string> elements;

    std::string stringer() override { return "<Enum '" + name + "'>"; }

    Kind kind() override { return ENUM; }
  };

  // FUNC
  class Func : public Object {
  public:
    std::string name; // function name

    ast::Arg arguments; // function args
    ast::Type *ret;     // function return

    Entity *entity; // function entity

    std::string stringer() override { return "<Func '" + name + "'>"; }

    Kind kind() override { return FUNC; }
  };

  // WHOLE
  class Whole : public Object {
  public:
    std::string name; // whole name

    Entity *entity; // whole entity

    // interface definition
    std::vector<std::tuple<std::string, ast::Arg, ast::Type *>> interface;

    // inherit definition
    std::vector<std::string> inherit;

    std::string stringer() override { return "<Whole '" + name + "'>"; }

    Kind kind() override { return WHOLE; }
  };
}; // namespace object

// entity structure
struct Entity {
  std::string title = ""; // TITLE FOR ENTITY

  explicit Entity() {}
  explicit Entity(std::string title) : title(title) {} // TO title

  std::vector<byte::Code> codes;           // bytecodes
  std::vector<int> offsets;                // offset of bytecode
  std::vector<object::Object *> constants; // constant
  std::vector<std::string> names;          // names
  std::vector<ast::Type *> types;          // type of variables

  // output entity data
  void dissemble() {
    std::cout << "ENTITY '" << title << "': " << std::endl;

    for (int ip = 0, op = 0; ip < codes.size(); ip++) {
      byte::Code co = codes.at(ip);

      switch (co) {
        case byte::CONST: {
          printf("%20d: %s %10d %s\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op++),
                 constants.at(offsets.at(op))->stringer().c_str());
        } break;
        case byte::ASSIGN: {
          printf("%20d: %s %9d '%s'\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op++),
                 names.at(offsets.at(op)).c_str());
        } break;
        case byte::STORE: {
          printf("%20d: %s %10d '%s' %d %s\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op),
                 names.at(offsets.at(op)).c_str(), offsets.at(op + 1),
                 types.at(offsets.at(op + 1))->stringer().c_str());
          op += 2;
        } break;
        case byte::LOAD:
        case byte::NAME: {
          printf("%20d: %s %11d '%s'\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op++),
                 names.at(offsets.at(op)).c_str());
        } break;
        case byte::FUNC:
        case byte::ENUM: {
          printf("%20d: %s %11d %s\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op++),
                 constants.at(offsets.at(op))->stringer().c_str());
        } break;
        case byte::WHOLE: {
          printf("%20d: %s %10d %s\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op++),
                 constants.at(offsets.at(op))->stringer().c_str());
        } break;
        case byte::GET:
        case byte::SET:
        case byte::MOD:
        case byte::USE:
        case byte::CHA: {
          printf("%20d: %s %12d '%s'\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op++),
                 names.at(offsets.at(op)).c_str());
        } break;
        case byte::CALL: {
          printf("%20d: %s %11d\n", ip, byte::codeString[codes.at(ip)].c_str(),
                 offsets.at(op++));
        } break;
        case byte::CALL_I: {
          printf("%20d: %s %9d\n", ip, byte::codeString[codes.at(ip)].c_str(),
                 offsets.at(op++));
        } break;
        case byte::B_ARR:
        case byte::B_TUP:
        case byte::B_MAP: {
          printf("%20d: %s %10d\n", ip, byte::codeString[codes.at(ip)].c_str(),
                 offsets.at(op++));
        } break;
        case byte::F_JUMP:
        case byte::T_JUMP: {
          printf("%20d: %s %9d\n", ip, byte::codeString[codes.at(ip)].c_str(),
                 offsets.at(op++));
        } break;
        case byte::JUMP: {
          printf("%20d: %s %11d\n", ip, byte::codeString[codes.at(ip)].c_str(),
                 offsets.at(op++));
        } break;
        case byte::NEW: {
          printf("%20d: %s %12d '%s' %d\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op),
                 names.at(offsets.at(op)).c_str(), offsets.at(op + 1));
          op += 2;
        } break;
        case byte::UAS: {
          printf("%20d: %s %12d '%s' %d '%s'\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op),
                 names.at(offsets.at(op)).c_str(), offsets.at(op + 1),
                 names.at(offsets.at(op + 1)).c_str());
          op += 2;
        } break;
        default:
          printf("%20d: %s\n", ip, byte::codeString[codes.at(ip)].c_str());
          break;
      }
    }

    std::cout << "CONSTANT: " << std::endl;
    if (constants.empty()) {
      printf("%20s\n", "EMPTY");
    } else {
      for (int i = 0; i < constants.size(); i++) {
        printf("%20d: %s\n", i, constants.at(i)->stringer().c_str());
      }
    }

    std::cout << "NAME: " << std::endl;
    if (names.empty()) {
      printf("%20s\n", "EMPTY");
    } else {
      for (int i = 0; i < names.size(); i++) {
        if (i % 4 == 0) {
          printf("%20d: '%s'\t", i, names.at(i).c_str());
        } else {
          printf("%5d: '%s' \t", i, names.at(i).c_str());
        }
        if ((i + 1) % 4 == 0) {
          printf("\n");
        }
      }
      printf("\n");
    }

    std::cout << "OFFSET: " << std::endl;
    if (offsets.empty()) {
      printf("%20s\n", "EMPTY");
    } else {
      for (int i = 0; i < offsets.size(); i++) {
        if (i % 4 == 0) {
          printf("%20d: %d \t", i, offsets.at(i));
        } else {
          printf("%5d: %d  \t", i, offsets.at(i));
        }
        if ((i + 1) % 4 == 0) {
          printf("\n");
        }
      }
      printf("\n");
    }

    std::cout << "TYPE: " << std::endl;
    if (types.empty()) {
      printf("%20s\n", "EMPTY");
    } else {
      for (int i = 0; i < types.size(); i++) {
        printf("%20d: %s\n", i, types.at(i)->stringer().c_str());
      }
    }
  }
}; // namespace entity

// compiler
namespace compiler {
  // compiler structure
  class Compiler {
  private:
    int position = 0;
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
    void emitType(ast::Type *);          // push names type to entity

    // insert position with current counts of bytecode
    void insertPosOffset(int);
    void insertPosOffset(int, int); // with custom value

    void expr(ast::Expr *); // expression
    void stmt(ast::Stmt *); // statements

  public:
    Compiler(std::vector<ast::Stmt *> statements) : statements(statements) {}

    // entities of compiled
    std::vector<Entity *> entities = {new Entity("main")};
    // compile statements to entities
    void compile();

    // currently compile entity
    Entity *now = entities.at(0);
  };

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
}; // namespace compiler

struct Table {
  std::map<std::string, object::Object *> symbols;

  object::Object *lookUp(std::string name) {
    if (symbols.count(name) == 0) return nullptr;
    return symbols.at(name);
  }

  void clear() { symbols.clear(); }
  bool empty() { return symbols.empty(); }
};

template <class T> class Stack {
private:
  int capacity = 4, count = 0;

  T *elements;

public:
  explicit Stack() { this->elements = new T[capacity]; }

  ~Stack() { delete[] elements; }

  void push(T t) {
    if (count + 1 > capacity) {
      this->capacity = capacity * 2;
      this->elements = (T *)realloc(this->elements, sizeof(T) * capacity);
    }
    this->elements[count++] = t;
  }

  T pop() { return this->elements[--count]; }

  T top() { return this->elements[count]; }

  int len() { return count; }

  bool empty() { return count == 0; }

  std::string stringer() {
    return "<Stack count = " + std::to_string(count) + ">";
  }
};

// frame structure
struct Frame {
  Entity *entity; // ENTITY

  Table local;                  // local names
  Stack<object::Object *> data; // data stack

  explicit Frame(Entity *e) : entity(e) {}
};

/**
 * VIRTUAL MACHINE
 *
 * ENTITY:
 *          BYTE, CONSTANT, NAME, TYPE, OFFSET
 *
 * OBJECT:
 *          INT, FLOAT, STR, CHAR, BOOL, ARRAY, TUPLE, MAP, ENUM, FUNC, WHOLE
 *
 * FRAME:
 *          (ENTITY, TABLE(K TO OBJECT), DATA STACK)
 */
namespace vm {
  // structure
  class vm {
  private:
    std::vector<Frame *> frames; // execute frames
    // top frame
    Frame *top();
    // push object to the current frame
    void pushData(object::Object *);
    // pop the top of data stack
    object::Object *popData();
    // emit new name of table to the current frame
    void emitTable(std::string, object::Object *);
    // look up a name
    object::Object *lookUp(std::string);
    // first to end iterator
    object::Object *retConstant();
    // first to end iterator
    ast::Type *retType();
    // first to end iterator
    std::string retName();

    int op = 0; // offset pointer

  public:
    explicit vm(Entity *main) {
      // to main frame as main frame
      this->frames.push_back(new Frame(main));
    }

    void evaluate(); // evaluate the top of frame
  };

  // top frame
  Frame *vm::top() { return frames.back(); }

  // push object to the current frame
  void vm::pushData(object::Object *obj) { top()->data.push(obj); }

  // pop the top of data stack
  object::Object *vm::popData() { return top()->data.pop(); }

  // emit new name of table to the current frame
  void vm::emitTable(std::string name, object::Object *obj) {
    top()->local.symbols.insert(std::make_pair(name, obj));
  }

  // look up a name
  object::Object *vm::lookUp(std::string n) { return top()->local.lookUp(n); }

  // first to end constant iterator for current frame's entity
  object::Object *vm::retConstant() {
    return top()->entity->constants.at(top()->entity->offsets.at(op++));
  }

  // first to end
  ast::Type *vm::retType() {
    return top()->entity->types.at(top()->entity->offsets.at(op++));
  }

  // first to end
  std::string vm::retName() {
    return top()->entity->names.at(top()->entity->offsets.at(op++));
  }

  // throw an exception
  void error(std::string message) {
    throw exp::Exp(exp::RUNTIME_ERROR, message, -1);
  }

  void vm::evaluate() {

#define BINARY_OP(T, L, OP, R) this->pushData(new T(L OP R));

    for (int ip = 0; ip < top()->entity->codes.size(); ip++) {

      // bytecode
      byte::Code co = top()->entity->codes.at(ip);

      switch (co) {

        case byte::CONST:
          this->pushData(this->retConstant());
          if (op != 0) this->op++;
          break;

        case byte::ADD:
        case byte::A_ADD: {
          object::Object *y = this->popData();
          object::Object *x = this->popData();

          if (x->kind() == object::INT) {
            switch (y->kind()) {
              case object::INT: {
                BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, +,
                          static_cast<object::Int *>(y)->value);
                break;
              }
              case object::FLOAT: {
                BINARY_OP(object::Float, static_cast<object::Int *>(x)->value,
                          +, static_cast<object::Float *>(y)->value);
                break;
              }
            }
          }
          if (x->kind() == object::FLOAT) {
            switch (y->kind()) {
              case object::INT: {
                BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                          +, static_cast<object::Int *>(y)->value);
                break;
              }
              case object::FLOAT: {
                BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                          +, static_cast<object::Float *>(y)->value);
                break;
              }
            }
          }
          if (x->kind() == object::STR && y->kind() == object::STR) {
            object::Str *l = static_cast<object::Str *>(x);
            object::Str *r = static_cast<object::Str *>(y);

            if (l->longer || r->longer) {
              error("cannot plus two long string literal");
            }

            this->pushData(new object::Str(l->value + r->value));
          }
          break;
        }
        case byte::SUB:
        case byte::A_SUB: {
          object::Object *y = this->popData();
          object::Object *x = this->popData();

          if (x->kind() == object::INT) {
            switch (y->kind()) {
              case object::INT: {
                BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, -,
                          static_cast<object::Int *>(y)->value);
                break;
              }
              case object::FLOAT: {
                BINARY_OP(object::Float, static_cast<object::Int *>(x)->value,
                          -, static_cast<object::Float *>(y)->value);
                break;
              }
            }
          }
          if (x->kind() == object::FLOAT) {
            switch (y->kind()) {
              case object::INT: {
                BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                          -, static_cast<object::Int *>(y)->value);
                break;
              }
              case object::FLOAT: {
                BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                          -, static_cast<object::Float *>(y)->value);
                break;
              }
            }
          }
          break;
        }
        case byte::MUL:
        case byte::A_MUL: {
          object::Object *y = this->popData();
          object::Object *x = this->popData();

          if (x->kind() == object::INT) {
            switch (y->kind()) {
              case object::INT: {
                BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, *,
                          static_cast<object::Int *>(y)->value);
                break;
              }
              case object::FLOAT: {
                BINARY_OP(object::Float,
                          static_cast<object::Int *>(x)->value, *,
                          static_cast<object::Float *>(y)->value);
                break;
              }
            }
          }
          if (x->kind() == object::FLOAT) {
            switch (y->kind()) {
              case object::INT: {
                BINARY_OP(object::Float,
                          static_cast<object::Float *>(x)->value, *,
                          static_cast<object::Int *>(y)->value);
                break;
              }
              case object::FLOAT: {
                BINARY_OP(object::Float,
                          static_cast<object::Float *>(x)->value, *,
                          static_cast<object::Float *>(y)->value);
                break;
              }
            }
          }
          break;
        }
        case byte::DIV:
        case byte::A_DIV: {
          object::Object *y = this->popData();
          object::Object *x = this->popData();

          if (x->kind() == object::INT) {
            switch (y->kind()) {
              case object::INT: {
                if (static_cast<object::Int *>(y)->value == 0)
                  error("division by zero");
                BINARY_OP(object::Int, static_cast<object::Int *>(x)->value, /,
                          static_cast<object::Int *>(y)->value);
                break;
              }
              case object::FLOAT: {
                if (static_cast<object::Float *>(y)->value == 0)
                  error("division by zero");
                BINARY_OP(object::Float, static_cast<object::Int *>(x)->value,
                          /, static_cast<object::Float *>(y)->value);
                break;
              }
            }
          }
          if (x->kind() == object::FLOAT) {
            switch (y->kind()) {
              case object::INT: {
                if (static_cast<object::Int *>(y)->value == 0)
                  error("division by zero");
                BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                          /, static_cast<object::Int *>(y)->value);
                break;
              }
              case object::FLOAT: {
                if (static_cast<object::Float *>(y)->value == 0)
                  error("division by zero");
                BINARY_OP(object::Float, static_cast<object::Float *>(x)->value,
                          /, static_cast<object::Float *>(y)->value);
                break;
              }
            }
          }
          break;
        }

        case byte::STORE: {
          std::cout << op << std::endl;

          object::Object *obj = this->popData();
          ast::Type *type = this->retType();
          std::string name = this->retName();

          if (type->kind() == ast::T_INT && obj->kind() != object::INT) {
            error("type error");
          }

          top()->local.symbols[name] = obj; // store to table
          break;
        }

        case byte::LOAD: {
          std::string name = this->retName();
          object::Object *obj = top()->local.lookUp(name);

          if (obj == nullptr) error("not defined name '" + name + "'");

          this->pushData(obj);
          break;
        }

        case byte::RET: {
          std::cout << top()->data.stringer() << std::endl;

          while (!top()->data.empty()) {
            std::cout << top()->data.pop()->stringer() << std::endl;
          }
          std::cout << "== END EVALUATE!! ==" << std::endl;
        }
      }
    }
#undef BINARY_OP
  }
} // namespace vm

// DEBUG to output tokens and statements
bool DEBUG = false;

// run source code
void run(std::string source) {
  try {
    // lexer
    auto lex = new lexer::Lexer(source);

    lex->tokenizer();
    if (DEBUG) lex->dissembleTokens();

    // parser
    auto parser = new parser::Parser(lex->tokens);

    parser->parse();
    if (DEBUG) parser->dissembleStmts();

    // semantic
    auto semantic = new semantic::Analysis(&parser->statements);
    // compiler
    auto compiler = new compiler::Compiler(parser->statements);
    compiler->compile();

    for (auto i : compiler->entities) i->dissemble();

    // vm
    (new vm::vm(compiler->entities[0]))->evaluate();
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
  std::cout << "\n"
            << "Drift 0.0.1 (REPL Mode, Feb 18 2021, 15:43:31)"
            << "\n"
            << std::endl;

  while (true) {
    std::cout << "ft >> ";
    std::cin.getline(line, 1024);

    if (strlen(line) == 0) {
      continue;
    }
    run(line);
  }
}

// entry
int main(int argc, char **argv) {
  if (argc == 2) {
    if (strcmp(argv[1], "-d") == 0) {
      DEBUG = true;
      repl();
    } else {
      runFile(argv[1]);
    }
  } else if (argc == 3) {
    if (strcmp("-d", argv[2]) == 0) {
      DEBUG = true;
    }
    runFile(argv[1]);
  } else {
    repl();
  }
  return 0;
}
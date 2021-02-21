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

#include <iostream>
#include <vector>
#include <sstream>
#include <exception>
#include <fstream>
#include <map>

#include <cstring>

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
    NAF,
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
    "NAF", "FOR", "DO", "OUT",
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
    keyword["naf"] = NAF; // 9
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
//  types for drift
  enum TypeKind {
    T_INT,    // int
    T_FLOAT,  // float
    T_STR,    // str
    T_CHAR,   // char
    T_BOOL,   // bool
    T_LIST,   // []<T>
    T_MAP,    // <T1, T2>
    T_TUPLE,  // (T)
    T_USER,   // user
  };

//  basic type for drift
//
#define S_INT   "int"   // 1
#define S_FLOAT "float" // 2
#define S_STR   "str"   // 3
#define S_CHAR  "char"  // 4
#define S_BOOL  "bool"  // 5

//  TYPE
  class Type {
    public:
//      stringer
      virtual std::string stringer() = 0;
//      kind of basic type
      virtual TypeKind kind() = 0;
  };

//  <int>
  class Int : public Type {
    public:
      std::string stringer() override { return "<Int>"; }

      TypeKind kind() override { return T_INT; }
  };

//  float
  class Float : public Type {
    public:
      std::string stringer() override { return "<Float>"; }

      TypeKind kind() override { return T_FLOAT; }
  };

//  str
  class Str : public Type {
    public:
      std::string stringer() override { return "<Str>"; }

      TypeKind kind() override { return T_STR; }
  };

//  char
  class Char : public Type {
    public:
      std::string stringer() override { return "<Char>"; }

      TypeKind kind() override { return T_CHAR; }
  };

//  bool
  class Bool : public Type {
    public:
      std::string stringer() override { return "<Bool>"; }

      TypeKind kind() override { return T_BOOL; }
  };

//  list (not keyword, for compiler analysis)
//  []<type>
  class List : public Type {
    public:
      Type *T;  // type for elements

      explicit List(Type *T) : T(T) {}

      std::string stringer() override {
        return "<List T=" + T->stringer() + " >";
      }

      TypeKind kind() override { return T_LIST; }
  };

//  map (not keyword, for compiler analysis)
//  <type, type>
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

//  tuple (not keyword, for compiler analysis)
//  (type)
  class Tuple : public Type {
    public:
      Type *T;  // type for elements

      explicit Tuple(Type *T) : T(T) {}

      std::string stringer() override {
        return "<Tuple T=" + T->stringer() + " >";
      }

      TypeKind kind() override { return T_TUPLE; }
  };

//  user definition type
//  `type`
  class User : public Type {
    public:
      token::Token name;

      explicit User(token::Token name) {
        this->name = std::move(name);
      }

      std::string stringer() override {
        return "<User Name='" + name.literal + "' >";
      }

      TypeKind kind() override { return T_USER; }
  };

//  ast types
  enum Kind {
//    expression
    EXPR_LITERAL,   // literal
    EXPR_BINARY,    // T1 <OP> T2
    EXPR_GROUP,     // (EXPR)
    EXPR_UNARY,     // <OP>EXPR
    EXPR_NAME,      // IDENT
    EXPR_CALL,      // EXPR(<EXPR>..)
    EXPR_GET,       // EXPR.NAME
    EXPR_LIST,      // [<EXPR>..]
    EXPR_MAP,       // {K1: V1, K2: V2}
    EXPR_TUPLE,     // (<EXPR>..)
    EXPR_INDEX,     // EXPR[EXPR]
//    statement
    STMT_EXPR,      // EXPR
    STMT_VAR,       // def <name>: <type> = <expr>
    STMT_BLOCK,     // BLOCK
    STMT_IF,        // IF
    STMT_FOR,       // FOR
    STMT_DO,        // DO
    STMT_OUT,       // OUT
    STMT_TIN,       // TIN
    STMT_FUNC,      // FUNC
    STMT_WHOLE,     // CLASS | ENUM | INTERFACE
    STMT_NEW,       // NEW
    STMT_AND,       // AND
    STMT_MOD,       // MOD
    STMT_USE,       // USE
    STMT_INHERIT,   // <- <name> + <name>..
    STMT_INTERFACE  // INTERFACE
  };

//  K1: V1 | K1 + K2: V2
  using Arg = std::map<token::Token, Type *>;

//  abstract expr
  class Expr {
    public:
//      return string of expr
      virtual std::string stringer() = 0;
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

      std::string stringer() override {
        return "<LiteralExpr { Token='" + token.literal + "' }>";
      }

      Kind kind() override { return EXPR_LITERAL; }
  };

//  T1 <OP> T2
//  + | - | * | / | += | -= | *= | /= | > | >= | < | <= | != | == | & | |
  class BinaryExpr : public Expr {
    public:
      Expr *left;       // left
      token::Token op;  // operator
      Expr *right;      // right

      explicit BinaryExpr(Expr *l, token::Token op, Expr *r) : left(l), right(r) {
        this->op = std::move(op);
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<BinaryExpr { Left=" <<
            left->stringer() << " Operator='" <<
            op.literal << "' Right=" << right->stringer() << " }>";

        return str.str();
      }

      Kind kind() override { return EXPR_BINARY; }
  };

//  (EXPR)
  class GroupExpr : public Expr {
    public:
      Expr *expr;

      explicit GroupExpr(Expr *expr) : expr(expr) {}

      std::string stringer() override {
        return "<GroupExpr { Expr=" + this->expr->stringer() + " }>";
      }

      Kind kind() override { return EXPR_GROUP; }
  };

//  <OP>EXPR
  class UnaryExpr : public Expr {
    public:
      token::Token token;
      Expr *expr;

      explicit UnaryExpr(token::Token tok, Expr *expr) : expr(expr) {
        this->token = std::move(tok);
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<UnaryExpr { Token=" <<
            token.literal << " Expr=" <<
            expr->stringer() << " }>";

        return str.str();
      }

      Kind kind() override { return EXPR_UNARY; }
  };

//  IDENT
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

      std::string stringer() override {
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

//  EXPR(<EXPR>..)
  class CallExpr : public Expr {
    public:
      Expr *callee;
      std::vector<Expr *> arguments;

      explicit CallExpr(Expr *expr, std::vector<Expr *> args) : callee(expr) {
        this->arguments = std::move(args);
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<CallExpr { Callee=" <<
            callee->stringer();

        if (!arguments.empty()) {
          str << " Args=";

          for (auto &i : arguments)
            str << i->stringer() << " ";
        }

        str << " }>";
        return str.str();
      }

      Kind kind() override { return EXPR_CALL; }
  };

//  EXPR.NAME
  class GetExpr : public Expr {
    public:
      token::Token name;

      Expr *expr;

      explicit GetExpr(Expr *expr, token::Token name) : expr(expr) {
        this->name = std::move(name);
      }

      std::string stringer() override {
        return "<GetExpr { Expr=" +
               expr->stringer() + " Get='" +
               name.literal + "' }>";
      }

      Kind kind() override { return EXPR_GET; }
  };

//  [<EXPR>..]
  class ListExpr : public Expr {
    public:
      std::vector<Expr *> elements;

      explicit ListExpr(std::vector<Expr *> e) {
        this->elements = std::move(e);
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<ListExpr { Elements=[";
        if (!elements.empty()) {
          for (auto &i : elements)
            str << i->stringer() << ", ";
        }

        str << "] }>";
        return str.str();
      }

      Kind kind() override { return EXPR_LIST; }
  };

//  {K1: V1, K2: V2}
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
            str << "K : " << i.first->stringer() << ", V : " <<
                i.second->stringer() << ", ";
        }

        str << "} }>";
        return str.str();
      }

      Kind kind() override { return EXPR_MAP; }
  };

//  (<EXPR>..)
  class TupleExpr : public Expr {
    public:
      std::vector<Expr *> elements;

      explicit TupleExpr(std::vector<Expr *> e) {
        this->elements = std::move(e);
      }

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

//  EXPR[EXPR]
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

//  abstract stmt
  class Stmt {
    public:
//      return string of stmt
      virtual std::string stringer() = 0;
//      return kind of stmt
      virtual Kind kind() = 0;
  };

//  <expr>
  class ExprStmt : public Stmt {
    public:
      Expr *expr;

      explicit ExprStmt(Expr *expr) : expr(expr) {}

      std::string stringer() override {
        return "<ExprStmt { Expr=" + expr->stringer() + " }>";
      }

      Kind kind() override { return STMT_EXPR; }
  };

//  def <name>: <type> = <expr>
  class VarStmt : public Stmt {
    public:
      token::Token name;

//      type define
      Type *T;

//      default is not init
      Expr *expr = nullptr;

//      has expr
      explicit VarStmt(token::Token name, Type *T, Expr *e) : T(T), expr(e) {
        this->name = std::move(name);
      }

//      not init expr
      explicit VarStmt(token::Token name, Type *T) : T(T) {
        this->name = std::move(name);
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<VarStmt { Name='" <<
            name.literal << "' Type=" <<
            T->stringer();

        if (expr != nullptr)
          str << " Expr=" << expr->stringer() << " }>";
        else
          str << " }>";

        return str.str();
      }

      Kind kind() override { return STMT_VAR; }
  };

//  <expr>.. end
  class BlockStmt : public Stmt {
    public:
      std::vector<Stmt *> block;

      explicit BlockStmt(std::vector<Stmt *> block) : block(block) {}

      std::string stringer() override {
        std::stringstream str;

        str << "<BlockStmt { ";
        if (!block.empty()) {
          str << "Block=";

          for (auto &i: block)
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
   * eif <expr>
   *     <block>
   * eif <expr>
   *     <block>
   * nai
   *     <block>
   */
  class IfStmt : public Stmt {
    public:
      Expr *condition;        // main condition
      BlockStmt *thenBranch;   // main condition branch

      std::map<Expr *, BlockStmt> eifBranch;  // eif cond and branch

      BlockStmt *naiBranch;    // nai branch;

      explicit IfStmt(Expr *cond,
                      BlockStmt *then,
                      std::map<Expr *, BlockStmt> eif,
                      BlockStmt *nai) {
        this->condition = cond;
        this->thenBranch = then;
        this->eifBranch = std::move(eif);
        this->naiBranch = nai;
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<IfStmt { Condition=" << condition->stringer();
        if (thenBranch != nullptr)
          str << " ThenBranch=" << thenBranch->stringer();
        if (!eifBranch.empty()) {
          str << " EifBranch=";

          for (auto &i: eifBranch)
            str << "K : " << i.first->stringer() << ", V : " <<
                i.second.stringer() << ", ";
        }

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
      BlockStmt *block;  // stmt

      explicit ForStmt(Expr *cond, BlockStmt *block) {
        this->condition = cond;
        this->block = block;
      }

      std::string stringer() override {
        return "<ForStmt { Condition=" + condition->stringer() +
               " Block=" + block->stringer() + " }>";
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
      BlockStmt *block;   // first do block
      ForStmt *forStmt;   // for statement

      explicit DoStmt(BlockStmt *block, ForStmt *stmt) {
        this->block = block;
        this->forStmt = stmt;
      }

      std::string stringer() override {
        return "<DoStmt { Block=" + block->stringer() +
               " ForStmt=" + forStmt->stringer() + " }>";
      }

      Kind kind() override { return STMT_DO; }
  };

//  out <expr>
  class OutStmt : public Stmt {
    public:
      Expr *expr;

      explicit OutStmt(Expr *e) : expr(e) {}

      std::string stringer() override {
        return "<OutExpr { Expr=" + expr->stringer() + " }>";
      }

      Kind kind() override { return STMT_OUT; }
  };

//  tin <expr>
  class TinStmt : public Stmt {
    public:
      Expr *expr;

      explicit TinStmt(Expr *e) : expr(e) {}

      std::string stringer() override {
        return "<TinExpr { Expr=" + expr->stringer() + " }>";
      }

      Kind kind() override { return STMT_TIN; }
  };

//  new <expr>
  class NewStmt : public Stmt {
    public:
      Expr *expr;

      explicit NewStmt(Expr *e) : expr(e) {}

      std::string stringer() override {
        return "<NewExpr { Expr=" + expr->stringer() + " }>";
      }

      Kind kind() override { return STMT_NEW; }
  };

//  mod <name>
  class ModStmt : public Stmt {
    public:
      token::Token name;

      explicit ModStmt(token::Token name) {
        this->name = std::move(name);
      }

      std::string stringer() override {
        return "<ModExpr { Name=" + name.literal + " }>";
      }

      Kind kind() override { return STMT_MOD; }
  };

//  use <name> | as <name>
  class UseStmt : public Stmt {
    public:
      token::Token name;
      token::Token *as = nullptr;

//      use <name>
      explicit UseStmt(token::Token name) {
        this->name = std::move(name);
      }

//      use <name> as <name>
      explicit UseStmt(token::Token name, token::Token *as) {
        this->name = std::move(name);
        this->as = as;
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<UseStmt { Name=" << name.literal;
        if (as != nullptr)
          str << " As=" << as->literal;

        str << " }>";
        return str.str();
      }

      Kind kind() override { return STMT_USE; }
  };

//  and <block> end
  class AndStmt : public Stmt {
    public:
      BlockStmt *block;

      explicit AndStmt(BlockStmt *block) : block(block) {}

      std::string stringer() override {
        return "<AndStmt Block=" + block->stringer() + " }>";
      }

      Kind kind() override { return STMT_AND; }
  };

//  def (<param>..) <name> -> <ret>
//      <block>
//  end
  class FuncStmt : public Stmt {
    public:
      Arg arguments;        // args

      token::Token name;    // name
      Type *ret;            // return
      BlockStmt *block;     // body

      explicit FuncStmt(Arg args,
                        token::Token name,
                        Type *ret,
                        BlockStmt *block) {
        this->arguments = args;
        this->name = std::move(name);
        this->ret = ret;
        this->block = block;
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<FuncStmt { Name=" << name.literal;
        str << " Args=";
//        args
        if (!arguments.empty()) {
          str << "(";
          for (auto &i : arguments) {
            str << "K : " << i.first.literal << ", V : " <<
                i.second->stringer() << ", ";
          }
          str << ")";
        } else
          str << "()";

        str << " Ret=" << ret->stringer();
        str << " Block=" << block->stringer();

        str << " }>";
        return str.str();
      }

      Kind kind() override { return STMT_FUNC; }
  };

//  <- <name> + <name>..
  class InheritStmt : public Stmt {
    public:
      std::vector<token::Token> names;

      explicit InheritStmt(std::vector<token::Token> names) {
        this->names = std::move(names);
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<InheritStmt Names=";
        if (!names.empty()) {
          str << "(";
          for (auto &i:names) {
            str << i.literal << ", ";
          }
          str << ")";
        } else
          str << "()";

        str << " }>";
        return str.str();
      }

      Kind kind() override { return STMT_INHERIT; }
  };

//  def (<param>..) <name> -> <ret>
  class InterfaceStmt : public Stmt {
    public:
      Arg arguments;      // arguments

      token::Token name;  // name
      Type *ret;          // return

      explicit InterfaceStmt(Arg args, token::Token name, Type *ret) {
        this->arguments = args;
        this->name = std::move(name);
        this->ret = ret;
      }

      std::string stringer() override {
        std::stringstream str;

        str << "<InterfaceStmt { Name=" << name.literal;
        str << " Args=";
//        args
        if (!arguments.empty()) {
          str << "(";
          for (auto &i : arguments) {
            str << "K : " << i.first.literal << ", V : " <<
                i.second->stringer() << ", ";
          }
          str << ")";
        } else
          str << "()";

        str << " Ret=" << ret->stringer();

        str << " }>";
        return str.str();
      }

      Kind kind() override { return STMT_INTERFACE; }
  };

//  class | enum | interface
  class WholeStmt : public Stmt {
    public:
      std::vector<Stmt *> body;

      explicit WholeStmt(std::vector<Stmt *> body) : body(body) {}

      std::string stringer() override {
        std::stringstream str;

        str << "<WholeStmt { Body=";
        if (!body.empty()) {
          str << "(";
          for (auto &i:body) {
            str << i->stringer() << ", ";
          }
          str << ")";
        } else
          str << "()";

        str << " }>";
        return str.str();
      }

      Kind kind() override { return STMT_WHOLE; }
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
      ast::Expr *call();
      ast::Expr *primary();
//      parsing statements
      ast::Stmt *stmt();
      ast::Stmt *block();
//
      ast::Type *type();
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
                " " + stmt->stringer() << std::endl;
  }

//  if kind of current token is EFF, its end of file and end of tokens
  inline bool Parser::isEnd() {
    return look().kind == token::EFF || this->position >= this->tokens.size();
  }

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
    if (this->look().kind == kind) {
      this->position++;
//
      return true;
    }
    return false;
  }

//  return the previous of tokens
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
    return call();
  }

//  expr(<expr>..) | expr.name | expr[expr]
  ast::Expr *Parser::call() {
    ast::Expr *expr = primary();
//    stack up the expression!!
//
//    LIKE: bar(foo(1, 2, 3)[x + 4])
//
    while (true) {
//      call
      if (look(token::L_PAREN)) {
//        arguments
        auto args = std::vector<ast::Expr *>();
//        no argument
        if (look(token::R_PAREN)) {
          expr = new ast::CallExpr(expr, args);
//          to next loop
          continue;
//          have arguments
        } else {
          do {
            args.push_back(this->expr());
//
          } while (look(token::COMMA));
        }
        if (!look(token::R_PAREN))
          error(exp::UNEXPECTED, "expect ')' after arguments");
        expr = new ast::CallExpr(expr, args);
//        get
      } else if (look(token::DOT)) {
        token::Token name = look();

        this->position++; // skip name token
        expr = new ast::GetExpr(expr, name);
//        index for list
      } else if (look(token::L_BRACKET)) {
//        empty index
        if (look(token::R_BRACKET))
          error(exp::UNEXPECTED, "null index");
//        index
        auto index = this->expr();

        if (!look(token::R_BRACKET))
          error(exp::UNEXPECTED, "expect ']' after index of list");
        expr = new ast::IndexExpr(expr, index);
      } else {
        break;
      }
    }
    return expr;
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
//      vector for tuple and group expression
      std::vector<ast::Expr *> elem;
//      empty tuple expr
      if (look(token::R_PAREN))
        return new ast::TupleExpr(elem);
//      tuple or group ?
      elem.push_back(this->expr());

//      tuple expr
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
//    list expr
    if (look(token::L_BRACKET)) {
      auto elem = std::vector<ast::Expr *>();

      if (look(token::R_BRACKET))
        return new ast::ListExpr(elem);
      else {
        do {
          elem.push_back(this->expr());
//
        } while (look(token::COMMA));
      }
      if (!look(token::R_BRACKET))
        error(exp::UNEXPECTED, "expect ']' after elements");
      return new ast::ListExpr(elem);
    }
//    map expr
    if (look(token::L_BRACE)) {
      std::map<ast::Expr *, ast::Expr *> elem;
//      empty map expr
      if (look(token::R_BRACE))
        return new ast::MapExpr(elem);

      while (true) {
        ast::Expr *K = this->expr();

        if (!look(token::COLON)) {
          error(exp::UNEXPECTED, "expect ':' after key in map");
        }
        ast::Expr *V = this->expr();

//        push to map
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
//    end
    error(exp::INVALID_SYNTAX, "invalid expression");
    return nullptr;
  }

//  statement
  ast::Stmt *Parser::stmt() {
    switch (this->look().kind) {
//      definition statement
      case token::DEF:
        this->position++;
//        variable | enum | class | interface | function
        if (look(token::IDENT)) {
          token::Token name = previous();

//          variable define
          if (look(token::COLON)) {
            ast::Type *T = this->type();
            this->position++;

//            value of variable
            if (look(token::EQ))
//              there is an initial value
              return new ast::VarStmt(name, T, this->expr());
            else
              return new ast::VarStmt(name, T);
          }
        }
//        function
        else if (look(token::L_PAREN)) {
          break;
//        whole
        } else {

        }
        break;
//      if
      case token::IF:
        break;
//      loop
      case token::FOR:
        break;
//      do loop
      case token::DO:
        break;
//      out in loop
      case token::OUT:
        break;
//      tin in loop
      case token::TIN:
        break;
//      new
      case token::NEW:
        break;
//      and
      case token::AND:
        break;
//      mod
      case token::MOD:
        break;
//      use
      case token::USE:
        break;
//      inherit for class
      case token::L_ARROW:
        break;
      default:
//        expression statement
        return new ast::ExprStmt(this->expr());
    }
//    end
    error(exp::INVALID_SYNTAX, "invalid statement");
    return nullptr;
  }

//  block
  ast::Stmt *Parser::block() {
    std::vector<ast::Stmt *> body;
//    until end token
    while (!look(token::END)) {
      body.push_back(this->stmt());
    }
    return new ast::BlockStmt(body);
  }

//  throw an exception
  inline void Parser::error(exp::Kind kind, std::string message) {
    throw exp::Exp(kind, std::move(message), look().line);
  }

//  type analysis
  ast::Type *Parser::type() {
    token::Token now = this->look();
//    type
    if (now.kind == token::IDENT) {
//      T1
      if (now.literal == S_INT) return new ast::Int();
//      T2
      if (now.literal == S_FLOAT) return new ast::Float();
//      T3
      if (now.literal == S_STR) return new ast::Str;
//      T4
      if (now.literal == S_CHAR) return new ast::Char();
//      T5
      if (now.literal == S_BOOL) return new ast::Bool;
//      user define type
      return new ast::User(now);
    }
//    T6
    if (now.kind == token::L_BRACKET) {
      this->position++; // skip left [ symbol

      if (!look(token::R_BRACKET)) {
        error(exp::UNEXPECTED, "expect ']' after left square bracket");
      }
      return new ast::List(this->type());
    }
//    T7
    if (now.kind == token::LESS) {
      this->position++; // skip left < symbol
//      key
      ast::Type *T1 = this->type();
      this->position++;  // skip left key token

      if (!look(token::COMMA)) {
        error(exp::UNEXPECTED, "expect ',' after key of map");
      }
      ast::Type *T2 = this->type();
      this->position++;  // skip right value token

      if (look().kind != token::GREATER) {
        error(exp::UNEXPECTED, "expect '>' after value of map");
      }
      return new ast::Map(T1, T2);
    }
//    T8
    if (now.kind == token::L_PAREN) {
      this->position++; // skip left ( symbol

      ast::Type *T = this->type();
      this->position++;

      if (look().kind != token::R_PAREN) {
        error(exp::UNEXPECTED, "expect ')' after tuple define");
      }
      return new ast::Tuple(T);
    }
    error(exp::INVALID_SYNTAX, "invalid type");
//
    return nullptr;
  }
}

// run source code
void run(std::string source) {
  try {
//    lexer
    auto lex = new lexer::Lexer(source);

    lex->tokenizer();
    lex->dissembleTokens();

//    parser
    auto parser = new parser::Parser(lex->tokens);

    parser->parse();
    parser->dissembleStmts();
//
  } catch (exp::Exp &e) {
    std::cout << "\033[31m" << e.toString() << "\033[0m" << std::endl;
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
  char *line = (char *) malloc(1024);
  std::cout <<
            "\n\tDrift 0.0.1 (repl mode, Feb 20 2021, 15:42) - Hello!! (:\n" << std::endl;
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

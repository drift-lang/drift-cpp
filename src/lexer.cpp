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

#include "lexer.hpp"

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

    this->tokens.push_back(token::Token{
        // keyword or IDENT
        getKeyword(this->keyword, literal.str()), literal.str(), this->line});
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
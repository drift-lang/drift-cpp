//
// Copyright (c) 2021 bingxio（丙杺，黄菁）. All rights reserved.
//

// GNU General Public License
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
//    total number of tokens for drift
    constexpr int len = 54;
//    token type
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
//        SINGLE_MARK,    // '
//        DOUBLE_MARk,    // "
//        OBLIQUE_MARK,   // `

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

//    returns a string of each type
    std::string kindString[len] = { // NOLINT(cert-err58-cpp)
        "IDENT", "NUM", "STR", "CHAR",
        "ADD", "SUB", "MUL", "DIV",
        "AS_ADD", "AS_SUB", "AS_MUL", "AS_DIV",
        "PLUS", "MINUS",
        "R_ARROW", "L_ARROW",
        "DOLLAR", "DOT", "COMMA", "COLON", "EQ",
        "GREATER", "LESS", "GR_EQ", "LE_EQ",
        "ADDR", "OR", "BANG", "BANG_EQ", "EQ_EQ",
//        "SINGLE_MARK", "DOUBLE_MARK", "OBLIQUE_MARK",
        "L_BRACE", "R_BRACE", "L_PAREN", "R_PAREN", "L_BRACKET", "R_BRACKET",
        "UNDERLINE",
//        end of file
        "EFF",
//        keywords
        "USE", "DEF", "PUB", "RET",
        "AND", "END", "IF", "EIF",
        "ELS", "FOR", "DO", "OUT",
        "TIN", "NEW", "MOD", "AS"
    };

//    token structure
    struct Token {
//        token type
        Kind kind = EFF;
//        token literal
        std::string literal;
//        line of source code
        int line = 0;
    };

//    format return token structure
    std::string toString(const Token &token) {
        std::stringstream str;

        str << "<Token { Kind=";
        str << kindString[token.kind] << " Literal=\"";
        str << token.literal << "\" Line=";
        str << token.line << " }>";

        return str.str();
    }

//    keywords for drift
    static std::map<std::string, Kind> keyword;

//    16 keywords
//    initialize it when tokenizer
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

//    return the corresponding keyword type according to the literal amount
    Kind getKeyword(const std::string &literal) {
        auto i = keyword.find(literal);
//        search map
        if (i != keyword.end())
            return i->second;
        else
            return IDENT;
    }
}

// exceptions
namespace exp {
//    total number of exceptions
    constexpr int len = 1;
//    exception type
    enum Kind {
        INVALID_SYNTAX, // invalid syntax
    };

//    return a string of exception type
    std::string kindString[len] = {"INVALID_SYNTAX"}; // NOLINT(cert-err58-cpp)

//    exception structure
    class Exp : public std::exception {
        private:
//            exception kind
            Kind kind;
//            exception message
            std::string message;
//            at exception line of source code
            int line;
        public:
            explicit Exp(Kind kind, std::string message, int line) {
                this->kind = kind;
                this->message = std::move(message);
                this->line = line;
            }

//            return a string of exception structure
            std::string toString();
    };

//    return a string of exception structure
    std::string Exp::toString() {
        std::stringstream str;

        str << "<Exp { Kind=";
        str << kindString[this->kind] << " Message=\"";
        str << this->message << "\" Line=";
        str << this->line << " }>";

        return str.str();
    }
}

// lexer
namespace lexer {
//    lexer structure
    class Lexer {
        private:
//            current character
            int position = 0;
//            current line
            int line = 1;
//            source code
            std::string source;
//            final token list
            std::vector<token::Token> tokens;
        public:
            explicit Lexer(std::string source) : source(std::move(source)) {
//                initializer keywords map here
                token::initializeKeywords();
            }

//            start
            void tokenizer();

//            resolve identifier
            void lexIdent();

//            resolve digit
            void lexDigit();

//            resolve "xxx" string literal
            void lexString();

//            resolve 'x' character literal
            void lexChar();

//            resolve other symbol
            void lexSymbol();

//            return current char of resolve
            char now();

//            return next char of resolve
            char peek();

//            return resolve is end
            bool isEnd();

//            return current char is identifier
            bool isIdent();

//            return current char is digit
            bool isDigit();

//            return current char is whitespace
            bool isSpace();

//            resolve to skip whitespace
            void skipWhitespace();

//            resolve to skip line comment
            void skipLineComment();

//            final to dissemble tokens list
            void dissembleTokens();
    };

//    start
    void Lexer::tokenizer() {
        while (!this->isEnd()) {
//            first to skip whitespace
            if (isSpace()) skipWhitespace();
//            identifier
            if (isIdent())
                this->lexIdent();
//            digit
            else if (isDigit())
                this->lexDigit();
//            string
            else if (now() == '"')
                this->lexString();
//            character
            else if (now() == '\'')
                this->lexChar();
//            symbol
            else
                this->lexSymbol();
        }
        this->tokens.push_back(
//            resolve end insert EFF for end of file
            token::Token{token::EFF, "EFF", ++this->line}
        );
    }

//    final to dissemble tokens list
    void Lexer::dissembleTokens() {
        for (const auto &token : this->tokens) {
            std::cout <<
                      token::toString(token) << std::endl;
        }
    }

//    return resolve is end
    bool Lexer::isEnd() { return this->position >= this->source.length(); }

//    resolve to skip whitespace
    void Lexer::skipWhitespace() {
        while (this->isSpace()) {
            if (now() == '\n') {
                this->line++;
            }
            this->position++;
        }
    }

//    resolve to skip line comment
    void Lexer::skipLineComment() {
        while (!isEnd() && now() != '\n') {
            this->position++;
        }
    }

//    return current char is identifier
    bool Lexer::isIdent() {
        return now() >= 'a' && now() <= 'z' ||
               now() >= 'A' && now() <= 'Z' ||
               now() == '_';
    }

//    return current char is digit
    bool Lexer::isDigit() {
        return now() >= '0' && now() <= '9';
    }

//    return current char is whitespace
    bool Lexer::isSpace() {
        if (now() == ' ' ||
            now() == '\r' ||
            now() == '\t' ||
            now() == '\n') { return true; }
        return false;
    }

//    return current char of resolve
    char Lexer::now() { return this->source.at(this->position); }

//    resolve identifier
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
//                keyword or IDENT
                token::getKeyword(literal.str()),
                literal.str(),
                this->line
            }
        );
    }

//    resolve digit
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
//            number
            token::Token{token::NUM, literal.str(), this->line});
    }

//    resolve string literal
    void Lexer::lexString() {
        std::stringstream literal;
        bool isEndFile = false;

//        skip left double quotation mark
        this->position++;

        while (!isEnd()) {
            if (now() == '"') {
//                end string
                this->position++;
                isEndFile = true;
                break;
            }
            literal << now();
            this->position++;
        }

//        missing closing double quote
        if (!isEndFile)
            throw exp::Exp(exp::INVALID_SYNTAX,
                           "missing closing double quote", this->line);

        this->tokens.push_back(
//            string
            token::Token{token::STR, literal.str(), this->line});
    }

//    resolve character
    void Lexer::lexChar() {
        std::stringstream literal;

//        skip left single quotation mark
        this->position++;

        literal << now();

        if (peek() != '\'')
//            this character is empty
            throw exp::Exp(exp::INVALID_SYNTAX, "", this->line);
        else
//            skip value and right single quotation mark
            this->position += 2;

        this->tokens.push_back(
//            character
            token::Token{token::CHAR, literal.str(), this->line});
    }

//    resolve symbols
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
//                +=
                if (peek() == '=') {
                    tok.kind = token::AS_ADD;
                    tok.literal = "+=";

                    this->position++;
//                    ++
                } else if (peek() == '+') {
                    tok.kind = token::PLUS;
                    tok.literal = "++";

                    this->position++;
                } else
//                    +
                    tok.kind = token::ADD;
                break;
            case '-':
//                ->
                if (peek() == '>') {
                    tok.kind = token::R_ARROW;
                    tok.literal = "->";

                    this->position++;
//                    --
                } else if (peek() == '-') {
                    tok.kind = token::MINUS;
                    tok.literal = "--";

                    this->position++;
//                    -=
                } else if (peek() == '=') {
                    tok.kind = token::AS_SUB;
                    tok.literal = "-=";

                    this->position++;
                } else
//                    -
                    tok.kind = token::SUB;
                break;
            case '*':
//                *=
                if (peek() == '=') {
                    tok.kind = token::AS_MUL;
                    tok.literal = "*=";

                    this->position++;
                } else
//                    *
                    tok.kind = token::MUL;
                break;
            case '/':
//                /=
                if (peek() == '=') {
                    tok.kind = token::AS_DIV;
                    tok.literal = "/=";

                    this->position++;
//                    to resolve skip comment
                } else if (peek() == '/') {
                    this->skipLineComment();

                    return;
                } else
//                    /
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
//                >=
                if (peek() == '=') {
                    tok.kind = token::GR_EQ;
                    tok.literal = ">=";

                    this->position++;
                } else
//                    >
                    tok.kind = token::GREATER;
                break;
            case '<':
//                <=
                if (peek() == '=') {
                    tok.kind = token::LE_EQ;
                    tok.literal = "<=";

                    this->position++;
//                    <-
                } else if (peek() == '-') {
                    tok.kind = token::L_ARROW;
                    tok.literal = "<-";

                    this->position++;
                } else
//                    <
                    tok.kind = token::LESS;
                break;
            case '&':
                tok.kind = token::ADDR;
                break;
            case '|':
                tok.kind = token::OR;
                break;
            case '!':
//                !=
                if (peek() == '=') {
                    tok.kind = token::BANG_EQ;
                    tok.literal = "!=";

                    this->position++;
                } else
//                    !
                    tok.kind = token::BANG;
                break;
            case '=':
//                ==
                if (peek() == '=') {
                    tok.kind = token::EQ_EQ;
                    tok.literal = "==";

                    this->position++;
                } else
//                    =
                    tok.kind = token::EQ;
                break;
            case '_':
                tok.kind = token::UNDERLINE;
                break;
        }

        this->position++;
        this->tokens.push_back(tok);
    }

//    return next char of resolve
    char Lexer::peek() {
        if (position + 1 >= source.length())
            return -1;
        else
            return source.at(position + 1);
    }
}

int main() {
    std::ifstream stream;
    stream.open("test1.ft");

    if (stream.fail()) {
        std::cout << "failed to open file" << std::endl;
        return -1;
    }

    std::string source((std::istreambuf_iterator<char>(stream)),
                       (std::istreambuf_iterator<char>()));
//    std::cout << source << std::endl;

    auto lex = new lexer::Lexer(source);

    try {
        lex->tokenizer();
    } catch (exp::Exp &e) {
        std::cout << e.toString() << std::endl;
        return -1;
    }

    lex->dissembleTokens();

    stream.close();
    return 0;
}

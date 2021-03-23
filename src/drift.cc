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

#include <cstring>
#include <filesystem>
#include <fstream>

#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "version.h"
#include "vm.h"

#include "system.h"

// DEBUG to output tokens and statements
bool DEBUG = false;
// repl mode
bool REPL = false;
// dis mode
bool DIS = false;

static std::vector<object::Module*> mods;  // global modules

vm* mac;  // local

// run source code
void run(std::string source) {
  try {
    // lexer
    auto lex = new Lexer(source);

    lex->tokenizer();
    if (DEBUG)
      lex->dissembleTokens();

    // parser
    auto parser = new Parser(lex->tokens);

    parser->parse();
    if (DEBUG)
      parser->dissembleStmts();

    // semantic
    auto semantic = new Analysis(&parser->statements);
    // compiler
    auto compiler = new Compiler(parser->statements);
    compiler->compile();

    if (DIS)
      for (auto i : compiler->entities)
        i->dissemble();

    // vm
    if (REPL && mac != nullptr) {
      // save the current symbol table
      mac->top()->entity = compiler->entities[0];
      mac->clean();
    } else {
      // new virtual machine
      mac = new vm(compiler->entities[0], &mods);
    }
    mac->evaluate();
    //
  } catch (exp::Exp& e) {
    std::cout << "\033[31m" << e.stringer() << "\033[0m" << std::endl;
    return;
  }
}

// FILE mode
void runFile(const char* path) {
  std::string s;
  fileString(path, &s);

  run(s);
}

// REPL mode
void repl() {
  REPL = true;

  char* line = (char*)malloc(1024);
  std::cout << VERS << std::endl;

  while (true) {
    std::cout << "ft >> ";
    std::cin.getline(line, 1024);

    if (strlen(line) == 0) {
      continue;
    }
    run(line);
  }
}

// load standard modules
bool loadStdModules() {
  std::vector<std::string>* fs =
      getAllFileWithPath(std::filesystem::current_path().string() + "/std");

  for (auto i : *fs) {
    std::string s;
    fileString(i.c_str(), &s);

    try {
      // lexer
      auto lex = new Lexer(s);
      lex->tokenizer();

      // parser
      auto parser = new Parser(lex->tokens);
      parser->parse();

      // semantic
      auto semantic = new Analysis(&parser->statements);
      // compiler
      auto compiler = new Compiler(parser->statements);
      compiler->compile();

      // vm
      (new vm(compiler->entities[0], &mods))->evaluate();
      //
    } catch (exp::Exp& e) {
      std::cout << "\033[31m" << e.stringer() << "\033[0m" << std::endl;
      return false;
    }
  }
  return true;  // OK
}

// VER
void version() {
  std::cout << VERS << std::endl;
}

// USAGE
void usage() {
  std::cout << "\n\t\tTHE DRIFT PROGRAMMING LANGUAGE\n" << std::endl;
  std::cout << "\t\tCOPYRIGHT C 2021 BINGXIO (黄菁) | GPL 3.0 LICENSE | ALL "
               "RIGHTS RESERVED\n"
            << std::endl;
  std::cout << "LINKS: " << std::endl;
  std::cout << "\t\t- https://drift-lang.fun/" << std::endl;
  std::cout << "\t\t- https://github.com/bingxio/drift\n" << std::endl;
  std::cout << "USAGE: " << std::endl;
  std::cout << "\t\t> drift                   # REPL MODE" << std::endl;
  std::cout << "\t\t> drift <ft file>         # FILE MODE" << std::endl
            << std::endl;
  std::cout << "\t\t> drift -v                # DIS VERSION" << std::endl
            << std::endl;
  std::cout << "\t\t> drift -d                # REPL AND DEBUG MODE"
            << std::endl;
  std::cout << "\t\t> drift -d <ft file>      # FILE AND DEBUG MODE"
            << std::endl
            << std::endl;
  std::cout << "\t\t> drift -b                # REPL AND DIS MODE" << std::endl;
  std::cout << "\t\t> drift <ft file> -b      # FILE AND DIS MODE" << std::endl
            << std::endl;
  std::cout << "CONTACT: " << std::endl;
  std::cout << "\t\t+ 1171840237@qq.com" << std::endl;
  std::cout << "\t\t+ https://bingxio.fun/" << std::endl << std::endl;
}

// entry
int main(int argc, char** argv) {
  if (argc == 2 && strcmp(argv[1], "-v") == 0) {
    version();
    return 0;
  }

  if (argc == 2 && strcmp(argv[1], "-u") == 0) {
    usage();
    return 0;
  }

  if (!loadStdModules())
    return 1;  // load standard modules

  if (argc == 2) {
    // D
    if (strcmp(argv[1], "-d") == 0) {
      DEBUG = true;
      repl();
    }
    // B
    else if (strcmp(argv[1], "-b") == 0) {
      DIS = true;
      repl();
    }
    // O
    else {
      runFile(argv[1]);
    }
  } else if (argc == 3) {
    if (strcmp("-d", argv[2]) == 0) {
      DEBUG = true;
    }
    if (strcmp("-b", argv[2]) == 0) {
      DIS = true;
    }
    runFile(argv[1]);
  } else {
    repl();
  }
  return 0;
}
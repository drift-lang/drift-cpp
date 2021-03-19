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

#include "module.hpp"

// add new module into global state
bool addModule(std::vector<object::Module *> *mods, std::string name, Frame *f,
               std::vector<std::string> pub) {
  if (getModule(mods, name) != nullptr) { // redefinition module
    return false;
  } else {
    mods->push_back(new object::Module(name, f, pub)); // insert new module
  }
  return true; // OK
}

// get module with name
object::Module *getModule(std::vector<object::Module *> *mods,
                          std::string name) {
  for (auto i : *mods) {
    if (i->name == name) {
      return i;
    }
  }
  return nullptr;
}

#define STD_PATH "/Users/turaiiao/Desktop/drift/std"

// load standard modules
void loadStdModules(std::vector<object::Module *> *mods) {
  std::vector<std::string> *fs = getAllFileWithPath(STD_PATH);

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
      auto mac = new vm(compiler->entities[0], mods);
      mac->evaluate();
      //
    } catch (exp::Exp &e) {
      std::cout << "\033[31m" << e.stringer() << "\033[0m" << std::endl;
    }
  }
}
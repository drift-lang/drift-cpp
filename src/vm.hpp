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

#ifndef DRIFT_VM_H
#define DRIFT_VM_H

#include <algorithm>

#include "exp.hpp"
#include "frame.hpp"
#include "object.hpp"
#include "opcode.hpp"

#include "entity.hpp"

#include "module.hpp"
#include "util.hpp"

// structure
class vm {
private:
  std::vector<Frame *> frames; // execute frames

  // push object to the current frame
  void pushData(object::Object *);

  // pop the top of data stack
  object::Object *popData();

  // emit new name of table to the current frame
  void emitTable(std::string, object::Object *);

  // emit the public of name in current frame
  void emitPub(std::string);

  // look up a name from current top frame
  object::Object *lookUp(std::string);

  // look up a name from main frame
  object::Object *lookUpMainFrame(std::string);

  // first to end iterator
  object::Object *retConstant();

  // first to end iterator
  ast::Type *retType();

  // first to end iterator
  std::string retName();

  // first to end iterator
  int retOffset();

  // are the comparison types the same
  void typeChecker(ast::Type *, object::Object *);

  // are two values of same type equal
  bool objValueEquation(object::Object *, object::Object *);

  // generate default values
  object::Object *setOriginalValue(ast::Type *);

  int op = 0; // offset pointer

  void addCounter(int *, int, int); // add counter for bytecode within jump

  bool callWholeMethod = false;          // is current calling whole
  std::string wholeName = "";            // name of current calling whole
  std::vector<std::string> wholeInherit; // inherits of current whole

  // loop exit and no return value return
  bool loopWasRet = false;

  std::vector<object::Module *> mods; // to global modules of program

public:
  explicit vm(Entity *main, std::vector<object::Module *> mods) {
    // to main frame as main
    this->frames.push_back(new Frame(main));
    this->mods = mods; // set global modules
  }

  // top frame
  Frame *top();

  // main frame
  Frame *main();

  // repl mode to clean pointer for offset
  inline void clean() { this->op = 0; }

  void evaluate(); // evaluate the top of frame
};

#endif
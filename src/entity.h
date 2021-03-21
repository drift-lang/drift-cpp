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

#ifndef DRIFT_ENTITY_H
#define DRIFT_ENTITY_H

#include <iostream>
#include <vector>

#include "ast.h"
#include "object.h"
#include "opcode.h"

// entity structure
struct Entity {
  std::string title = "";  // TITLE FOR ENTITY

  explicit Entity() {}
  explicit Entity(std::string title) : title(title) {}  // TO title

  std::vector<byte::Code> codes;           // bytecodes
  std::vector<int> offsets;                // offset of bytecode
  std::vector<object::Object*> constants;  // constant
  std::vector<std::string> names;          // names
  std::vector<ast::Type*> types;           // type of variables

  // output entity data
  void dissemble() {
    std::cout << "ENTITY '" << title << "': " << std::endl;

    for (int ip = 0, op = 0; ip < codes.size(); ip++) {
      byte::Code co = codes.at(ip);

      switch (co) {
        case byte::CONST: {
          printf("%20d: %s %10d %s\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op++),
                 constants.at(offsets.at(op))->rawStringer().c_str());
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
                 constants.at(offsets.at(op))->rawStringer().c_str());
        } break;
        case byte::WHOLE: {
          printf("%20d: %s %10d %s\n", ip,
                 byte::codeString[codes.at(ip)].c_str(), offsets.at(op++),
                 constants.at(offsets.at(op))->rawStringer().c_str());
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
        printf("%20d: %s\n", i, constants.at(i)->rawStringer().c_str());
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

    // std::cout << "OFFSET: " << std::endl;
    // if (offsets.empty()) {
    //     printf("%20s\n", "EMPTY");
    // } else {
    //     for (int i = 0; i < offsets.size(); i++) {
    //         if (i % 4 == 0) {
    //             printf("%20d: %d \t", i, offsets.at(i));
    //         } else {
    //             printf("%5d: %d  \t", i, offsets.at(i));
    //         }
    //         if ((i + 1) % 4 == 0) {
    //             printf("\n");
    //         }
    //     }
    //     printf("\n");
    // }

    std::cout << "TYPE: " << std::endl;
    if (types.empty()) {
      printf("%20s\n", "EMPTY");
    } else {
      for (int i = 0; i < types.size(); i++) {
        printf("%20d: %s\n", i, types.at(i)->stringer().c_str());
      }
    }
  }
};

#endif
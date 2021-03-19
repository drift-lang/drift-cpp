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

#ifndef DRIFT_UTIL_H
#define DRIFT_UTIL_H

#include <fstream>
#include <iostream>

// returns whether the string is all numbers
bool isNumberStr(const std::string);

// read the string buffer into string
void fileString(const char *, std::string *);

#endif
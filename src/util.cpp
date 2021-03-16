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

#include "util.hpp"

// returns whether the string is all numbers
bool isNumberStr(const std::string str) {
    for (int i = 0; i < str.size(); i++) {
        if (!std::isdigit(str.at(i))) {
            return false;
        }
    }
    return true;
}
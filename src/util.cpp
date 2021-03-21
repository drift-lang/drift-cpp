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

// return whether the string is all numbers
bool isNumberStr(const std::string str) {
  for (int i = 0; i < str.size(); i++)
    if (!std::isdigit(str.at(i)))
      return false;
  return true;
}

// read the string buffer into string
void fileString(const char* path, std::string* to) {
  std::ifstream stream;
  stream.open(path);

  if (stream.fail()) {
    std::cout << "<Failed To Open File>" << std::endl;
    return;
  }

  std::string source((std::istreambuf_iterator<char>(stream)),
                     (std::istreambuf_iterator<char>()));
  *to = source;
  stream.close();
}

// return the whether a value is the same
bool sameValue(std::vector<std::string>& a, std::vector<std::string>& b) {
  for (auto i : a)
    for (auto k : b)
      if (i == k)
        return true;
  return false;
}
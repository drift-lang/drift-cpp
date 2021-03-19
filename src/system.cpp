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

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "system.hpp"

std::vector<std::string> *getAllFileWithPath(std::string path) {
  DIR *dir;
  struct dirent *p;

  std::vector<std::string> *files;

  if ((dir = opendir(path.c_str())) == NULL) return nullptr;

  // std::cout << "GET: " << path << std::endl;
  // std::cout << (files == nullptr) << std::endl;

  while ((p = readdir(dir)) != NULL) {
    if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0) {
      continue;
    } else if (p->d_type == 8) { // file
      files->push_back(path + "/" + p->d_name);
    } else {
      continue;
    }
  }

  return files;
}
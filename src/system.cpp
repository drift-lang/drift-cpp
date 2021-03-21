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

// return all the files in path
std::vector<std::string>* getAllFileWithPath(std::string path) {
  DIR* dir;
  struct dirent* p;

  std::vector<std::string>* files = new std::vector<std::string>();
  // std::cout << "GET: " << path << std::endl;

  if ((dir = opendir(path.c_str())) == NULL)
    return nullptr;

  while ((p = readdir(dir)) != NULL) {
    // folder
    if (p->d_type == 4 && strcmp(p->d_name, ".") != 0 &&
        strcmp(p->d_name, "..") != 0) {
      std::vector<std::string>* fo = getAllFileWithPath(path + "/" + p->d_name);
      for (auto i : *fo) {
        files->push_back(i);
      }
    }
    // file
    if (p->d_type == 8) {
      files->push_back(path + "/" + p->d_name);
    }
  }
  return files;
}
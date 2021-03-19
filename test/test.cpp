#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <vector>

std::vector<std::string> *getAllFileWithPath(std::string path) {
  DIR *dir;
  struct dirent *p;

  std::vector<std::string> *files = new std::vector<std::string>();

  if ((dir = opendir(path.c_str())) == NULL) return nullptr;

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

int main() {
  std::vector<std::string> *fs =
      getAllFileWithPath("/Users/turaiiao/Desktop/drift/std");
  for (auto i : *fs) {
    std::cout << i << std::endl;
  }
  return 0;
}
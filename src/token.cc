#include "token.h"

void initializeKeyword(std::map<std::string, token::Kind> *map) {
  using namespace token;
  // initializer keywords map here

  map->insert(std::make_pair("use", USE)); // 1
  map->insert(std::make_pair("def", DEF)); // 2
  map->insert(std::make_pair("ret", RET)); // 3
  map->insert(std::make_pair("end", END)); // 4
  map->insert(std::make_pair("if", IF));   // 5
  map->insert(std::make_pair("ef", EF));   // 6
  map->insert(std::make_pair("nf", NF));   // 7
  map->insert(std::make_pair("for", FOR)); // 8
  map->insert(std::make_pair("do", DO));   // 9
  map->insert(std::make_pair("out", OUT)); // 10
  map->insert(std::make_pair("tin", TIN)); // 11
  map->insert(std::make_pair("new", NEW)); // 12
  map->insert(std::make_pair("mod", MOD)); // 13
  map->insert(std::make_pair("del", DEL)); // 14
}

token::Kind getKeyword(std::map<std::string, token::Kind> m,
                       const std::string &literal) {
  auto i = m.find(literal);
  if (i == m.end()) {
    return token::IDENT;
  } else {
    return i->second;
  }
}
#include "token.h"

void initializeKeyword(std::map<std::string, token::Kind> *map) {
  using namespace token;
  // initializer keywords map here

  map->insert(std::make_pair("use", USE)); // 1
  map->insert(std::make_pair("def", DEF)); // 2
  map->insert(std::make_pair("ret", RET)); // 3
  map->insert(std::make_pair("and", AND)); // 4
  map->insert(std::make_pair("end", END)); // 5
  map->insert(std::make_pair("if", IF));   // 6
  map->insert(std::make_pair("ef", EF));   // 7
  map->insert(std::make_pair("nf", NF));   // 8
  map->insert(std::make_pair("for", FOR)); // 9
  map->insert(std::make_pair("do", DO));   // 10
  map->insert(std::make_pair("out", OUT)); // 11
  map->insert(std::make_pair("tin", TIN)); // 12
  map->insert(std::make_pair("new", NEW)); // 13
  map->insert(std::make_pair("mod", MOD)); // 14
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
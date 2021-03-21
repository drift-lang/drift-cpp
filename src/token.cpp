#include "token.hpp"

void initializeKeyword(std::map<std::string, token::Kind>* map) {
  using namespace token;
  // initializer keywords map here

  map->insert(std::make_pair("use", USE));  // 1
  map->insert(std::make_pair("def", DEF));  // 2
  map->insert(std::make_pair("pub", PUB));  // 3
  map->insert(std::make_pair("ret", RET));  // 4
  map->insert(std::make_pair("and", AND));  // 5
  map->insert(std::make_pair("end", END));  // 6
  map->insert(std::make_pair("if", IF));    // 7
  map->insert(std::make_pair("ef", EF));    // 8
  map->insert(std::make_pair("nf", NF));    // 9
  map->insert(std::make_pair("for", FOR));  // 10
  map->insert(std::make_pair("do", DO));    // 11
  map->insert(std::make_pair("out", OUT));  // 12
  map->insert(std::make_pair("tin", TIN));  // 13
  map->insert(std::make_pair("new", NEW));  // 14
  map->insert(std::make_pair("mod", MOD));  // 15
  map->insert(std::make_pair("as", AS));    // 16
}

token::Kind getKeyword(std::map<std::string, token::Kind> m,
                       const std::string& literal) {
  auto i = m.find(literal);
  if (i == m.end()) {
    return token::IDENT;
  } else {
    return i->second;
  }
}
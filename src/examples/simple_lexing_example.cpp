#include "lexer.hpp"
#include <iostream>
int main() {
  
  for (auto tok : solve::lexer::tokens("Foo[1, 2, Bar[Baz[3.14, 1.41]]]"))
    std::cout << std::get<0>(tok) << "\n";
  return 0;
}

#include "../solve.hpp"
#include <iostream>
int main() { 
  for (auto tok: solve::lexer::tokens("Foo[1, 2, Bar [Baz  [3.14, 1.41]]]"))
    std::cout << tok << "\n";
  return 0;
}
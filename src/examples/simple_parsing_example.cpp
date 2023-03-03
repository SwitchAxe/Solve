#include "lexer.hpp"
#include "parser.hpp"

int main() {
  solve::parser::print_tree(solve::parser::parse(
      solve::lexer::tokens("Foo[1 / 2 * 2, 3 + 4 - 5, Bar[1, 2, 3]]")));
}

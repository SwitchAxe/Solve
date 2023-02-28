#include "lexer.hpp"
#include "parser.hpp"

int main() {
  solve::parser::print_tree(
      solve::parser::parse(solve::lexer::tokens("Foo[1, Bar[4, 5, 6], 3]")));
}

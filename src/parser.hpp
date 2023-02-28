#pragma once
#include "lexer.hpp"
#include "types.hpp"
#include <charconv>
#include <map>
#include <stack>
namespace solve {
namespace parser {

// helper functions to deduce the correct number type
template <class T> types::Number<T> make_number(T t) {
  return types::Number<T>(t);
}
using State = lexer::State;
using Pair = std::tuple<std::string, State>;
types::ast parse_dispatch(Pair tk);
types::ast parse_dispatch_lparen(Pair tk);
types::ast parse_dispatch_rparen(Pair tk);
types::ast parse_dispatch_ident(Pair tk);
types::ast parse_dispatch_number(Pair tk);
types::ast parse_dispatch_comma(Pair tk);
types::ast parse_dispatch_dot(Pair tk);
std::variant<types::Real, types::Integer, types::Natural>
convert_to_number(std::string num);

types::ast parse(std::vector<Pair> tokens);
void print_tree(types::ast tree);
} // namespace parser
} // namespace solve

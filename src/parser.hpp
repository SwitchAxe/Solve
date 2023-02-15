#pragma once
#include "lexer.hpp"
#include "types.hpp"

namespace solve {
namespace parser {
// Create an AST of which the nodes are the function call operators, and the
// leaves are identifiers and literals.
// T is the type of the number stored in the resulting node. This makes sense
// since this is a recursive parser, and thus we can evaluate each child node
// with a different type depending on the situation.
template <class T> types::Type<T> parse(std::vector<std::string> tokens) {}
} // namespace parser
} // namespace solve

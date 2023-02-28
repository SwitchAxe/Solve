#include "parser.hpp"
#include <iostream>
using namespace solve;

std::variant<types::Real, types::Integer, types::Natural>
parser::convert_to_number(std::string num) {
  types::Real x;
  types::Integer n;
  types::Natural k;
  if (auto [ptr, ec] = std::from_chars(num.data(), num.data() + num.size(), x);
      ec == std::errc()) {
    return x;
  } else if (auto [ptr, ec] =
                 std::from_chars(num.data(), num.data() + num.size(), k);
             ec == std::errc()) {
    return k;
  } else if (auto [ptr, ec] =
                 std::from_chars(num.data(), num.data() + num.size(), n);
             ec == std::errc()) {
    return n;
  }
  throw std::logic_error{"Invalid number in token stream!\n"};
}

// stack to keep track of where we are in the tree so we can
// rollback to a previoous position instead of appending to the
// same branch everytime
std::stack<types::ast> ast_stk;

// Number stack so we can abstract away the "state" of the current expression.
// e.g. "are we in a lone numeric literal?", "are we in an infix operator call?"
// this stack answers these questions.
std::stack<types::ast> number_stk;

types::ast parser::parse_dispatch_ident(Pair tk) {
  if (number_stk.empty())
    ast_stk.push({{std::get<0>(tk)}, {}});
  else if (number_stk.size() > 1)
    throw std::logic_error{"Too many arguments to the left of an infix "
                           "operator! Expected 1, got " +
                           std::to_string(number_stk.size()) + ".\n"};
  else {
    ast_stk.push({{std::get<0>(tk)}, {number_stk.top()}});
    number_stk.pop();
  }
  return {{std::get<0>(tk)}, {}};
}

types::ast parser::parse_dispatch_number(Pair tk) {
  if (ast_stk.empty()) {
    throw std::logic_error{"Invalid token in the parser: Numeric literal "
                           "outside of a function call!\n"};
  }
  std::string num;
  if (!number_stk.empty()) {
    num = std::get<std::string>(number_stk.top().node.sym);
    number_stk.pop();
    num += std::get<0>(tk);
  } else {
    num = std::get<0>(tk);
  }
  number_stk.push({{num}, {}});
  return {{""}, {}}; // this will get filtered out later in the parser itself
}

types::ast parser::parse_dispatch_lparen(Pair tk) {
  if (ast_stk.empty()) {
    throw std::logic_error{"Invalid token in the parser: Left parenthesis "
                           "without an identifier!\n"};
  }
  return {};
}

types::ast parser::parse_dispatch_rparen(Pair tk) {
  if (ast_stk.empty()) {
    throw std::logic_error{"Invalid token in the parser: Right parenthesis "
                           "without a function call!\n"};
  }
  if (ast_stk.size() == 1) {
    auto node = ast_stk.top();
    ast_stk.pop();
    if (!number_stk.empty()) {
      node.childs.push_back(number_stk.top());
      number_stk.pop();
    }
    return node;
  } else {
    auto last_closed = ast_stk.top();
    ast_stk.pop();
    if (!number_stk.empty()) {
      last_closed.childs.push_back(number_stk.top());
      number_stk.pop();
    }
    auto node = ast_stk.top();
    ast_stk.pop();
    node.childs.push_back(last_closed);
    ast_stk.push(node);
  }
  return {};
}

types::ast parser::parse_dispatch_comma(Pair tk) {
  if (ast_stk.empty()) {
    throw std::logic_error{
        "Misplaced comma: The function call stack is empty!\n"};
  }
  if (!number_stk.empty()) {
    types::ast last_id = ast_stk.top();
    ast_stk.pop();
    last_id.childs.push_back(number_stk.top());
    number_stk.pop();
    ast_stk.push(last_id);
  }
  return {};
}

types::ast parser::parse_dispatch(Pair tk) {
  switch (std::get<1>(tk)) {
  case State::Lparen:
    return parser::parse_dispatch_lparen(tk);
  case State::End:
  case State::Rparen:
    return parser::parse_dispatch_rparen(tk);
  case State::Comma:
    return parser::parse_dispatch_comma(tk);
  case State::Integer:
  case State::Real:
    return parser::parse_dispatch_number(tk);
  case State::Identifier:
    return parser::parse_dispatch_ident(tk);
  case State::Argument:
  case State::Start:
  case State::Dot:
    return {};
  case State::Error:
    throw std::logic_error{"Error in the parser in the '" + std::get<0>(tk) =
                               "' token!\n"};
  }
  return {};
}

types::ast parser::parse(std::vector<Pair> tokens) {
  std::vector<types::ast> v = tokens |
                              std::views::transform([](Pair p) -> types::ast {
                                return parser::parse_dispatch(p);
                              }) |
                              utils::to<std::vector<types::ast>>();
  return v.back();
}

void parser::print_tree(types::ast tree) {
  static int depth = 0;
  std::visit(
      []<class T>(T v) -> void {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
          for (int i = 0; i < depth; ++i) {
            std::cout << " ";
          }
          std::cout << "|";
          std::cout << v;
          std::cout << "\n";
        } else if constexpr (std::is_same_v<std::decay_t<T>,
                                            std::list<types::ast>>) {
          for (auto e : v) {
            print_tree(e);
          }
        } else if constexpr (std::is_same_v<std::decay_t<T>,
                                            types::Number<types::Integer>> ||
                             std::is_same_v<std::decay_t<T>,
                                            types::Number<types::Natural>> ||
                             std::is_same_v<std::decay_t<T>,
                                            types::Number<types::Real>>) {
          for (int i = 0; i < depth; ++i) {
            std::cout << " ";
          }
          std::cout << "|";
          std::cout << v << "\n";
        } else {
          for (int i = 0; i < depth; ++i) {
            std::cout << " ";
          }
          std::cout << "|"
                    << "functor\n";
        }
      },
      tree.node.sym);
  if (tree.childs.empty()) {
    return;
  }
  depth++;
  for (auto e : tree.childs) {
    print_tree(e);
  }
  depth--;
}

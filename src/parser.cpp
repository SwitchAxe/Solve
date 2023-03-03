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
// this tells us if the top of the ast_stack is an infix operator
bool maybe_infix = false;
std::string num;
types::ast parser::parse_dispatch_ident(Pair tk) {
  ast_stk.push({{std::get<0>(tk)}, {}});
  if (!number_stk.empty()) {
    maybe_infix = true;
  }
  if (!num.empty()) {
    number_stk.push({{num}, {}});
    num = "";
  }
  return {{std::get<0>(tk)}, {}};
}

types::ast parser::parse_dispatch_number(Pair tk) {
  if (ast_stk.empty()) {
    throw std::logic_error{"Invalid token in the parser: Numeric literal "
                           "outside of a function call!\n"};
  }
  num += std::get<0>(tk);
  return {{""}, {}}; // this will get filtered out later in the parser itself
}

types::ast parser::parse_dispatch_lparen(Pair tk) {
  if (ast_stk.empty()) {
    throw std::logic_error{"Parser error: expected an identifier before an "
                           "opening parenthesis!\n"};
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
    if (!num.empty()) {
      node.childs.push_back({{num}, {}});
    }
    return node;
  } else {
    auto last_closed = ast_stk.top();
    ast_stk.pop();
    if (!number_stk.empty()) {
      last_closed.childs.push_back(number_stk.top());
      number_stk.pop();
    }
    if (!num.empty()) {
      last_closed.childs.push_back({{num}, {}});
      num = "";
    }
    types::ast node = ast_stk.top();
    ast_stk.pop();
    node.childs.push_back(last_closed);
    ast_stk.push(node);
  }
  return {};
}

types::ast parser::parse_dispatch_comma(Pair tk) {
  if (ast_stk.empty() && number_stk.empty()) {
    throw std::logic_error{
        "Misplaced comma: Expected a function call or a literal!\n"};
  }
  // if, at this point, maybe_infix still contains a value, then we're
  // parsing an infix operator.
  if (!num.empty()) {
    number_stk.push({{num}, {}});
    num = "";
  }
  if (maybe_infix) {
    while (number_stk.size() > 1) {
      types::ast lhs = number_stk.top();
      number_stk.pop();
      types::ast rhs = number_stk.top();
      number_stk.pop();
      types::ast op = ast_stk.top();
      ast_stk.pop();
      op.childs.push_front(lhs);
      op.childs.push_front(rhs);
      number_stk.push(op); // ONLY here, to iteratively traverse the expression.
    }
    types::ast last_op = number_stk.top();
    number_stk.pop();
    types::ast func = ast_stk.top();
    ast_stk.pop();
    func.childs.push_back(last_op);
    ast_stk.push(func);
    maybe_infix = false;
  } else {
    types::ast op = ast_stk.top();
    ast_stk.pop();
    op.childs.push_back(number_stk.top());
    number_stk.pop();
    ast_stk.push(op);
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

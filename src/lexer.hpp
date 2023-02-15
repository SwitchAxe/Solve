#pragma once
#include "utils.hpp"
#include <algorithm>
#include <cctype>
#include <ranges>
#include <tuple>
#include <vector>
namespace solve {
namespace lexer {

bool ignore_whitespace(char ch);

bool is_comma(char ch);

bool is_rdelim(char ch);
bool is_ldelim(char ch);

enum State {
  Identifier,
  Integer,
  Real,
  Argument,
  End,
  Start,
  Error,
  Comma,
  Dot
};

State dispatch_right_delimiter(State st);

State dispatch_left_delimiter(State st);

State dispatch_whitespace(State st);

State dispatch_alpha(State st);

State dispatch_comma(State st);

State dispatch_dot(State st);

State dispatch_numeric(State st);

State dispatch(char ch, State st = State::Start);

std::vector<std::string> tokens(std::string input); 
} // namespace lexer
} // namespace solve

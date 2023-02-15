#include "lexer.hpp"
#include "bits/ranges_base.h"

bool solve::lexer::ignore_whitespace(char ch) {
  if (!isgraph(ch))
    return true;
  return false;
}

bool solve::lexer::is_comma(char ch) {
  if (ch == ',')
    return true;
  return false;
}

bool solve::lexer::is_rdelim(char ch) {
  static std::vector<char> delims = {']', ')'};
  if (std::find(delims.begin(), delims.end(), ch) != delims.end())
    return true;
  return false;
}

bool solve::lexer::is_ldelim(char ch) {
  static std::vector<char> delims = {'[', '('};
  if (std::find(delims.begin(), delims.end(), ch) != delims.end())
    return true;
  return false;
}

// i don't want to write solve::lexer::State each fucking time
using State = solve::lexer::State;

State solve::lexer::dispatch_right_delimiter(State st) {
  if ((st == State::Argument) || (st == State::Real) ||
      (st == State::Integer) || (st == State::End)) {
    return State::End;
  }
  return State::Error;
}

State solve::lexer::dispatch_left_delimiter(State st) {
  if ((st == State::Identifier) || (st == State::Comma) ||
      (st == State::Start) || (st == State::Argument)) {
    return State::Argument;
  }
  return State::Error;
}

State solve::lexer::dispatch_whitespace(State st) {
  if ((st == State::Comma) || (st == State::Argument) || (st == State::Real) ||
      (st == State::Integer)) {
    return State::Argument;
  }
  if (st == State::Start)
    return State::Start;
  if (st == State::Identifier)
    return State::Argument;
  return State::Error;
}

State solve::lexer::dispatch_alpha(State st) {
  if ((st == State::Comma) || (st == State::Argument)) {
    return State::Identifier;
  }
  if (st == State::Identifier)
    return State::Identifier;
  if (st == State::Start)
    return State::Identifier;
  if (st == State::Integer)
    return State::Argument;
  return State::Error;
}

State solve::lexer::dispatch_comma(State st) {
  if (st == State::Integer)
    return State::Comma;
  if (st == State::Real)
    return State::Comma;
  if (st == State::Argument)
    return State::Comma;
  return State::Error;
}

State solve::lexer::dispatch_dot(State st) {
  if (st == State::Integer)
    return State::Real;
  return State::Error;
}

State solve::lexer::dispatch_numeric(State st) {
  if (st == State::Dot)
    return State::Real;
  if (st == State::Integer)
    return State::Integer;
  if (st == State::Identifier)
    return State::Identifier;
  if (st == State::Comma)
    return State::Integer;
  if (st == State::Real)
    return State::Real;
  if (st == State::Argument)
    return State::Integer;
  return State::Error;
}

State solve::lexer::dispatch(char ch, State st) {
  if (solve::lexer::ignore_whitespace(ch))
    return dispatch_whitespace(st);
  if (isdigit(ch))
    return dispatch_numeric(st);
  if (solve::lexer::is_rdelim(ch))
    return dispatch_right_delimiter(st);
  if (solve::lexer::is_ldelim(ch))
    return dispatch_left_delimiter(st);
  if (ch == ',')
    return dispatch_comma(st);
  if (ch == '.')
    return dispatch_dot(st);
  if (isgraph(ch))
    return dispatch_alpha(st);
  return State::Error;
}

std::vector<std::tuple<std::string, State>>
solve::lexer::tokens(std::string input) {
  // string -> view (range) -> transform into <tokens, states> (range) ->
  // accumulate based on the state component -> filter empty vectors ->
  // accumulate into a vector
  static const std::string enum_values[] = {"Identifier", "Integer", "Real",
                                            "Argument",   "End",     "Start",
                                            "Error",      "Comma",   "Dot"};
  State cur = State::Start;
  auto tps =
      input |
      std::views::transform([&](const char ch) -> std::tuple<char, State> {
        auto tp = std::make_tuple(ch, cur = dispatch(ch, cur));
        return tp;
      }) |
      utils::to<std::vector<std::tuple<char, State>>>();

  auto tpstovec =
      tps |
      std::views::transform([](std::tuple<char, State> tp)
                                -> std::vector<std::tuple<char, State>> {
        static State reduced = State::Start;
        static std::vector<std::tuple<char, State>> accum;
        if (std::get<0>(tp) == ' ') {
          reduced = std::get<1>(tp);
          auto returned = accum;
          accum = {};
          return returned;
        }
        if (std::get<1>(tp) == reduced) {
          accum.push_back(tp);
          return {};
        }
        // special case for the last token of an expression,
        // marked with State::End.
        // we want to collect that too:
        if (std::get<1>(tp) == State::End) {
          auto returned = std::vector<std::tuple<char, State>>{tp};
          accum = {};
          reduced = std::get<1>(tp);
          return returned;
        }
        reduced = std::get<1>(tp);
        auto returned = accum;
        accum = {tp};
        return returned;
      }) |
      utils::to<std::vector<std::vector<std::tuple<char, State>>>>();
  auto filteredvec =
      tpstovec |
      std::views::filter([](std::vector<std::tuple<char, State>> v) -> bool {
        return !v.empty();
      });
  auto stringvec =
      filteredvec |
      std::views::transform([](std::vector<std::tuple<char, State>> v)
                                -> std::tuple<std::string, State> {
        auto chars =
            v | std::views::transform([](std::tuple<char, State> tp) -> char {
              return std::get<0>(tp);
            }) |
            utils::to<std::vector<char>>();
        return {std::string(chars.begin(), chars.end()), std::get<1>(v[0])};
      });
  return (stringvec | utils::to<std::vector<std::tuple<std::string, State>>>());
}

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <iostream>
#include <ranges>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
namespace solve {
namespace types {
using Real = long double;
using Natural = long long unsigned int;
using Integer = long long signed int;
template <class T> struct Functor;
template <class T> struct Number;
template <> struct Number<Real>;
template <> struct Number<Integer>;
template <> struct Number<Natural>;
struct List;
template <class T> using Type = std::variant<Functor<T>, Number<T>, List>;

template <class T> struct Functor {
  std::function<Number<T>(Number<T>)> f;
};

template <> struct Number<Real> {
  Number() = default;
  Number(Real x) { num = x; }
  Real num;
  static const bool is_real = true;
  static const bool is_integer = false;
  bool is_positive = false;
};

template <> struct Number<Integer> {
  Number() = default;
  Number(Integer n) { num = n; }
  Integer num;
  static const bool is_integer = true;
  static const bool is_real = false;
  bool is_positive = false;
};

template <> struct Number<Natural> {
  Number() = default;
  Number(Natural n) { num = n; }
  Natural num;
  static const bool is_integer = true;
  static const bool is_real = false;
  static const bool is_positive = true;
};

// standard composable functions
template <class T> T quot(T t, T u) { return t / u; }
template <class T> T prod(T t, T u) { return t * u; }
template <class T> T add(T t, T u) { return t + u; }
template <class T> T sub(T t, T u) { return t - u; }

} // namespace types
namespace utils {
template <class C> struct to_helper {};
template <typename Container, std::ranges::range R>
  requires std::convertible_to<std::ranges::range_value_t<R>,
                               typename Container::value_type>
Container operator|(R &&r, to_helper<Container>) {
  auto cmn = r | std::views::common;
  return Container(cmn.begin(), cmn.end());
}
template <std::ranges::range Container>
  requires(!std::ranges::view<Container>)
auto to() {
  return to_helper<Container>{};
}
} // namespace utils
namespace lexer {

bool ignore_whitespace(char ch) {
  if (!isgraph(ch))
    return true;
  return false;
}

bool is_comma(char ch) {
  if (ch == ',')
    return true;
  return false;
}

bool is_rdelim(char ch) {
  static std::vector<char> delims = {']', ')'};
  if (std::find(delims.begin(), delims.end(), ch) != delims.end())
    return true;
  return false;
}
bool is_ldelim(char ch) {
  static std::vector<char> delims = {'[', '('};
  if (std::find(delims.begin(), delims.end(), ch) != delims.end())
    return true;
  return false;
}
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

State dispatch_right_delimiter(State st) {
  if ((st == State::Argument) || (st == State::Real) ||
      (st == State::Integer) || (st == State::End)) {
    return State::End;
  }
  return State::Error;
}

State dispatch_left_delimiter(State st) {
  if ((st == State::Identifier) || (st == State::Comma) ||
      (st == State::Start) || (st == State::Argument)) {
    return State::Argument;
  }
  return State::Error;
}

State dispatch_whitespace(State st) {
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

State dispatch_alpha(State st) {
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

State dispatch_comma(State st) {
  if (st == State::Integer)
    return State::Comma;
  if (st == State::Real)
    return State::Comma;
  if (st == State::Argument)
    return State::Comma;
  return State::Error;
}

State dispatch_dot(State st) {
  if (st == State::Integer)
    return State::Real;
  return State::Error;
}

State dispatch_numeric(State st) {
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

State dispatch(char ch, State st = State::Start) {
  if (ignore_whitespace(ch))
    return dispatch_whitespace(st);
  if (isdigit(ch))
    return dispatch_numeric(st);
  if (is_rdelim(ch))
    return dispatch_right_delimiter(st);
  if (is_ldelim(ch))
    return dispatch_left_delimiter(st);
  if (ch == ',')
    return dispatch_comma(st);
  if (ch == '.')
    return dispatch_dot(st);
  if (isgraph(ch))
    return dispatch_alpha(st);
  return State::Error;
}

std::vector<std::string> tokens(std::string input) {
  // string -> view (range) -> transform into tokens (range) ->
  // take-while based on a lexer function -> accumulate into a vector
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

  auto tpstovec = tps |
                  std::views::transform(
                      [](std::tuple<char, State> tp) -> std::vector<char> {
                        static State reduced = State::Start;
                        static std::vector<char> accum;
                        if (std::get<0>(tp) == ' ') {
                          reduced = std::get<1>(tp);
                          auto returned = accum;
                          accum = {};
                          return returned;
                        }
                        if (std::get<1>(tp) == reduced) {
                          accum.push_back(std::get<0>(tp));
                          return {};
                        }
                        // special case for the last token of an expression,
                        // marked with State::End.
                        // we want to collect that too:
                        if (std::get<1>(tp) == State::End) {
                          auto returned = std::vector<char>{std::get<0>(tp)};
                          accum = {};
                          reduced = std::get<1>(tp);
                          return returned;
                        }
                        reduced = std::get<1>(tp);
                        auto returned = accum;
                        accum = {std::get<0>(tp)};
                        return returned;
                      }) |
                  utils::to<std::vector<std::vector<char>>>();
  auto filteredvec =
      tpstovec | std::views::filter(
                     [](std::vector<char> v) -> bool { return !v.empty(); });
  auto stringvec = filteredvec | std::views::transform(
                                     [](std::vector<char> v) -> std::string {
                                       return std::string(v.begin(), v.end());
                                     });
  return (stringvec | utils::to<std::vector<std::string>>());
}
} // namespace lexer
} // namespace solve

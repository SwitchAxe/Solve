#pragma once
#include <functional>
#include <list>
#include <string>
#include <variant>
namespace solve {
namespace types {
using Real = long double;
using Natural = long long unsigned int;
using Integer = long long signed int;
template <class T, class U> struct Functor;
template <class... Ts> struct Number;
template <> struct Number<Real>;
template <> struct Number<Integer>;
template <> struct Number<Natural>;
template <class T, class U> struct Functor {
  std::function<Number<U>(Number<T>)> f;
};

template <> struct Number<> {
  Number() = default;
  Number(std::variant<Real, Integer, Natural> x) { num = x; }
  std::variant<Real, Integer, Natural> num;
  bool is_real = false;
  bool is_positive = false;
  bool is_integer = false;
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

struct Type {
  using nums = std::variant<Number<Real>, Number<Integer>, Number<Natural>>;
  using fn = Functor<nums, nums>;
  using _T = std::variant<nums, fn, std::list<Type>, std::string>;
  _T sym;
};

struct ast {
  Type node;
  std::list<ast> childs;
};

/*
template <> struct Type<Real> {
  std::variant<Functor<Real>, Number<Real>, std::list<Type>, std::string> sym;
};

template <> struct Type<Integer> {
  std::variant<Functor<Integer>, Number<Integer>, std::list<Type>, std::string>
      sym;
};

template <> struct Type<Natural> {
  std::variant<Functor<Natural>, Number<Natural>, std::list<Type>, std::string>
      sym;
};
*/

// standard composable functions
template <class T> T quot(T t, T u) { return t / u; }
template <class T> T prod(T t, T u) { return t * u; }
template <class T> T add(T t, T u) { return t + u; }
template <class T> T sub(T t, T u) { return t - u; }

} // namespace types
} // namespace solve

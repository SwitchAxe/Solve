#ifndef TYPESHPP
#define TYPESHPP
#include <functional>
#include <variant>
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
} // namespace solve
#endif

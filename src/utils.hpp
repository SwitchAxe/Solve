#include <ranges>
#ifndef UTILSHPP
#define UTILSHPP
namespace solve {
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
} // namespace solve
#endif

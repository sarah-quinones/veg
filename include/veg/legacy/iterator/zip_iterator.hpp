#ifndef __VEG_ZIP_ITERATOR_HPP_NK26HCX0S
#define __VEG_ZIP_ITERATOR_HPP_NK26HCX0S

#include "veg/internal/type_traits.hpp"
#include "veg/legacy/iterator/begin_end.hpp"
#include "veg/tuple.hpp"

#include <iterator>

namespace veg {
namespace legacy {

template <typename... Iters>
struct zip_iterator;
template <typename... Iters>
struct zip_proxy;

} // namespace legacy
} // namespace veg

template <typename... Iters>
struct std::iterator_traits<veg::legacy::zip_iterator<Iters...>> {
  using difference_type =
      common_type_t<typename iterator_traits<Iters>::difference_type...>;
  using value_type = tuple<typename iterator_traits<Iters>::value_type...>;
  using pointer = value_type*;
  using reference = veg::legacy::zip_proxy<Iters...>;

  using iterator_category = conditional_t<
      veg::meta::all_of({std::is_base_of<
          input_iterator_tag,
          typename iterator_traits<Iters>::iterator_category>::value...}),
      input_iterator_tag,
      veg::meta::conditional_t<
          veg::meta::all_of({std::is_base_of<
              output_iterator_tag,
              typename iterator_traits<Iters>::iterator_category>::value...}),
          output_iterator_tag,
          void>>;
};

namespace veg {
namespace legacy {

template <typename... Iters>
struct zip_proxy {
  tuple<decltype(*__VEG_DECLVAL(Iters))...> m_proxies;

public:
  VEG_TEMPLATE(
      (usize I, typename T),
      requires(
          I < sizeof...(Iters) &&
          meta::is_same<meta::remove_cvref_t<T>, zip_proxy>::value),
      friend constexpr auto get,
      (p, T&&))
  noexcept -> decltype(auto) {
    return ::veg::internal::tuple::adl_get<I>((VEG_FWD(p).m_proxies));
  }

  // clang-format off
  template <i64 I> auto operator[](fix<I> arg) & noexcept -> decltype(m_proxies[arg]) { return m_proxies[arg]; }
  template <i64 I> auto operator[](fix<I> arg) const& noexcept -> decltype(m_proxies[arg]) { return m_proxies[arg]; }
  template <i64 I> auto operator[](fix<I> arg) && noexcept
    -> decltype(VEG_MOV(m_proxies)[arg]) { return VEG_MOV(m_proxies)[arg]; }
  template <i64 I> auto operator[](fix<I> arg) const&& noexcept
    -> decltype(VEG_MOV(m_proxies)[arg]) { return VEG_MOV(m_proxies)[arg]; }
  // clang-format on
};

template <typename... Iters>
struct zip_iterator {
  using traits_type = std::iterator_traits<zip_iterator>;
  zip_iterator() = default;

private:
  explicit zip_iterator(tuple<Iters...> iters) : m_iters{iters} {}
  tuple<Iters...> m_iters;

  template <i64... Is>
  constexpr auto incr(meta::integer_sequence<i64, Is...> /*tag*/)
      -> zip_iterator& {
    int dummy[] = {(void(++m_iters[fix<Is>{}]), 0)...};
    (void)dummy;
    return *this;
  }
  template <i64... Is>
  constexpr auto decr(meta::integer_sequence<i64, Is...> /*tag*/)
      -> zip_iterator& {
    int dummy[] = {(void(--m_iters[fix<Is>{}]), 0)...};
    (void)dummy;
    return *this;
  }

  template <i64... Is>
  constexpr auto deref(meta::integer_sequence<i64, Is...> /*tag*/) const ->
      typename traits_type::reference {
    return {tuple<decltype(*m_iters)>{*m_iters[fix<Is>{}]...}};
  }

  template <i64... Is>
  constexpr auto cmp_eq(
      zip_iterator const& other,
      meta::integer_sequence<i64, Is...> /*tag*/) const -> bool {
    return meta::any_of({m_iters[fix<Is>{}] == other.m_iters[fix<Is>{}]...});
  }

  using seq = meta::make_integer_sequence<i64, i64(sizeof...(Iters))>;

public:
  auto operator++() -> auto& { return incr(seq{}); }
  auto operator++(int) {
    auto clone = *this;
    incr(seq{});
    return clone;
  }
  auto operator--() -> auto& { return decr(seq{}); }
  auto operator--(int) {
    auto clone = *this;
    decr(seq{});
    return clone;
  }
  auto operator*() const { return deref(seq{}); }
  auto operator==(zip_iterator const& other) const {
    return cmp_eq(other, seq{});
  }

  VEG_TEMPLATE(
      (usize I, typename T),
      requires(
          I < sizeof...(Iters) &&
          meta::is_same<meta::remove_cvref_t<T>, zip_iterator>::value),
      friend constexpr auto get,
      (p, T&&),
      (/*tag*/ = {}, tag_t<zip_iterator>))
  noexcept -> decltype(auto) {
    return ::veg::internal::tuple::adl_get<I>((VEG_FWD(p).m_iters));
  }

  // clang-format off
  template <i64 I> auto operator[](fix<I> arg) & noexcept -> decltype(m_iters[arg]) { return m_iters[arg]; }
  template <i64 I> auto operator[](fix<I> arg) const& noexcept -> decltype(m_iters[arg]) { return m_iters[arg]; }
  template <i64 I> auto operator[](fix<I> arg) && noexcept
    -> decltype(VEG_MOV(m_iters)[arg]) { return VEG_MOV(m_iters)[arg]; }
  template <i64 I> auto operator[](fix<I> arg) const&& noexcept
    -> decltype(VEG_MOV(m_iters)[arg]) { return VEG_MOV(m_iters)[arg]; }
  // clang-format on
};

template <typename... Rngs>
struct zip_exact {
  tuple<Rngs...> m_rngs;
  using iterator = zip_iterator<typename Rngs::iterator...>;
  using const_iterator = zip_iterator<typename Rngs::const_iterator...>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  template <i64... Is>
  constexpr auto _begin_mut(meta::integer_sequence<i64, Is...> /*tag*/)
      -> iterator {
    return {legacy::begin(m_rngs[fix<Is>{}])...};
  }

  template <i64... Is>
  constexpr auto _end_mut(meta::integer_sequence<i64, Is...> /*tag*/)
      -> iterator {
    return {legacy::end(m_rngs[fix<Is>{}])...};
  }

  template <i64... Is>
  constexpr auto _begin_const(meta::integer_sequence<i64, Is...> /*tag*/) const
      -> const_iterator {
    return {legacy::begin(m_rngs[fix<Is>{}])...};
  }

  template <i64... Is>
  constexpr auto _end_const(meta::integer_sequence<i64, Is...> /*tag*/) const
      -> const_iterator {
    return {legacy::end(m_rngs[fix<Is>{}])...};
  }

  using seq = meta::make_integer_sequence<i64, i64(sizeof...(Rngs))>;

public:
  template <i64... Is>
  constexpr auto begin() -> iterator {
    return iterator{_begin_mut(seq{})};
  }

  template <i64... Is>
  constexpr auto end(meta::integer_sequence<i64, Is...> /*tag*/) -> iterator {
    return iterator{_end_mut(seq{})};
  }

  template <i64... Is>
  constexpr auto begin(meta::integer_sequence<i64, Is...> /*tag*/) const
      -> const_iterator {
    return const_iterator{_begin_const(seq{})};
  }

  template <i64... Is>
  constexpr auto end(meta::integer_sequence<i64, Is...> /*tag*/) const
      -> const_iterator {
    return const_iterator{_end_const(seq{})};
  }

  template <i64... Is>
  constexpr auto rbegin(meta::integer_sequence<i64, Is...> /*tag*/)
      -> reverse_iterator {
    return reverse_iterator{end()};
  }

  template <i64... Is>
  constexpr auto rend(meta::integer_sequence<i64, Is...> /*tag*/)
      -> reverse_iterator {
    return reverse_iterator{begin()};
  }

  template <i64... Is>
  constexpr auto rbegin(meta::integer_sequence<i64, Is...> /*tag*/) const
      -> const_reverse_iterator {
    return const_reverse_iterator{end()};
  }

  template <i64... Is>
  constexpr auto rend(meta::integer_sequence<i64, Is...> /*tag*/) const
      -> const_reverse_iterator {
    return const_reverse_iterator{begin()};
  }

  VEG_TEMPLATE(
      (usize I, typename T),
      requires(
          I < sizeof...(Rngs) &&
          meta::is_same<meta::remove_cvref_t<T>, zip_exact>::value),
      friend constexpr auto get,
      (p, T&&))
  noexcept -> decltype(auto) {
    return ::veg::internal::tuple::adl_get<I>((VEG_FWD(p).m_rngs));
  }

  // clang-format off
  template <i64 I> auto operator[](fix<I> arg) & noexcept -> decltype(m_rngs[arg]) { return m_rngs[arg]; }
  template <i64 I> auto operator[](fix<I> arg) const& noexcept -> decltype(m_rngs[arg]) { return m_rngs[arg]; }
  template <i64 I> auto operator[](fix<I> arg) && noexcept
    -> decltype(VEG_MOV(m_rngs)[arg]) { return VEG_MOV(m_rngs)[arg]; }
  template <i64 I> auto operator[](fix<I> arg) const&& noexcept
    -> decltype(VEG_MOV(m_rngs)[arg]) { return VEG_MOV(m_rngs)[arg]; }
  // clang-format on
};

} // namespace legacy
} // namespace veg

template <veg::usize I, typename... Ts>
struct std::tuple_element<I, veg::legacy::zip_proxy<Ts...>>
    : std::tuple_element<I, veg::tuple<Ts...>> {};
template <typename... Ts>
struct std::tuple_size<veg::legacy::zip_proxy<Ts...>>
    : integral_constant<veg::usize, sizeof...(Ts)> {};

template <veg::usize I, typename... Ts>
struct std::tuple_element<I, veg::legacy::zip_iterator<Ts...>>
    : std::tuple_element<I, veg::tuple<Ts...>> {};
template <typename... Ts>
struct std::tuple_size<veg::legacy::zip_iterator<Ts...>>
    : integral_constant<veg::usize, sizeof...(Ts)> {};

#endif /* end of include guard __VEG_ZIP_ITERATOR_HPP_NK26HCX0S */

#ifndef VEG_BEGIN_END_HPP_LH1IYIZBS
#define VEG_BEGIN_END_HPP_LH1IYIZBS

#include "veg/internal/type_traits.hpp"

namespace veg {
namespace legacy {

namespace internal {
namespace begin_end {

template <typename T, typename Enable = void>
struct has_member_fn_begin_end : std::false_type {};

template <typename T>
struct has_member_fn_begin_end<
    T,
    meta::void_t<
        decltype(std::declval<T>().begin()),
        decltype(std::declval<T>().end())>> : std::true_type {};

template <typename T, typename Enable = void>
struct has_adl_begin_end : std::false_type {};

template <typename T>
struct has_adl_begin_end<
    T,
    meta::void_t<
        decltype(begin(std::declval<T>())),
        decltype(end(std::declval<T>()))>> : std::true_type {};

template <typename T>
auto range_end(T&& rng) -> VEG_REQUIRES_RET(
    !has_member_fn_begin_end<T&&>::value,

    decltype(end(VEG_FWD(rng)))) {
  return end(VEG_FWD(rng));
}

template <
    typename T,
    i64 which = meta::first_true<                        //
        meta::is_bounded_array<meta::remove_cvref_t<T>>, //
        has_member_fn_begin_end<T>,                      //
        has_adl_begin_end<T>>::value                     //
    >
struct has_begin_end;

template <typename T>
struct has_begin_end<T, meta::not_found> : std::false_type {
  // none
  static void begin(T&& /*arr*/) {}
  static void end(T&& /*arr*/) {}
};

template <typename T>
struct has_begin_end<T, 0> : std::true_type {
  // array
  constexpr static auto begin(T&& arr) -> decltype(+arr) { return +arr; }
  constexpr static auto end(T&& arr) -> decltype(+arr) {
    return +arr + sizeof(arr) / sizeof(arr[0]);
  }
};

template <typename T>
struct has_begin_end<T, 1> : std::true_type {
  // member
  constexpr static auto begin(T&& rng) VEG_DECLTYPE_RET(VEG_FWD(rng).begin());
  constexpr static auto end(T&& rng) VEG_DECLTYPE_RET(VEG_FWD(rng).end());
};

template <typename T>
struct has_begin_end<T, 2> : std::true_type {
  // adl
  constexpr static auto begin(T&& rng) VEG_DECLTYPE_RET(begin(VEG_FWD(rng)));
  constexpr static auto end(T&& rng) VEG_DECLTYPE_RET(end(VEG_FWD(rng)));
};
} // namespace begin_end
} // namespace internal

namespace tt {
template <typename T>
struct has_begin
    : meta::bool_constant<internal::begin_end::has_begin_end<T&&>::value> {};
template <typename T>
struct has_end : has_begin<T> {};
} // namespace tt

namespace fn {
struct begin_fn {
  template <typename T>
  constexpr auto operator()(T&& rng) const noexcept -> VEG_REQUIRES_RET(
      tt::has_begin<T>::value,
      decltype(internal::begin_end::has_begin_end<T&&>::begin(VEG_FWD(rng)))) {
    return internal::begin_end::has_begin_end<T&&>::begin(VEG_FWD(rng));
  }
};
struct end_fn {
  template <typename T>
  constexpr auto operator()(T&& rng) const noexcept -> VEG_REQUIRES_RET(
      tt::has_end<T>::value,
      decltype(internal::begin_end::has_begin_end<T&&>::end(VEG_FWD(rng)))) {
    return internal::begin_end::has_begin_end<T&&>::end(VEG_FWD(rng));
  }
};
} // namespace fn
VEG_ODR_VAR(begin, fn::begin_fn);
VEG_ODR_VAR(end, fn::end_fn);

} // namespace legacy
} // namespace veg

#endif /* end of include guard VEG_BEGIN_END_HPP_LH1IYIZBS */

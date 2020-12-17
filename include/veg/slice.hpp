#ifndef VEG_SPAN_HPP_CBT4079WS
#define VEG_SPAN_HPP_CBT4079WS

#include "veg/assert.hpp"

namespace veg {
namespace internal {

template <typename T, typename Enable = void>
struct has_member_fn_data : std::false_type {};

template <typename T>
struct has_member_fn_data<
    T,
    meta::enable_if_t<
        meta::is_pointer_v<decltype(VEG_DECLVAL(T&).data())> &&
        meta::is_constructible_v<decltype(VEG_DECLVAL(T const&).size())>>>
    : std::true_type {};

template <
    typename T,
    i64 which = meta::first_true<
        meta::is_bounded_array<meta::remove_cvref_t<T>>,
        has_member_fn_data<T>>::value>
struct has_data;

template <typename T>
struct has_data<T, meta::not_found> : std::true_type {
  static void data(T& /*rng*/) {}
  static void size(T& /*rng*/) {}
};

template <typename T>
struct has_data<T, 0> : std::true_type {
  // array
  constexpr static auto data(T& rng) noexcept { return +rng; };
  constexpr static auto size(T& rng) noexcept {
    return sizeof(rng) / sizeof(rng[0]);
  }
};

template <typename T>
struct has_data<T, 1> : std::true_type {
  // mem fn
  constexpr static auto data(T& rng) noexcept { return rng.data(); }
  constexpr static auto size(T& rng) noexcept { return rng.size(); }
};

template <typename Rng>
using data_t =
    decltype(has_data<meta::remove_cvref_t<Rng>>::data(VEG_DECLVAL(Rng&)));
template <typename Rng>
using size_t_ =
    decltype(has_data<meta::remove_cvref_t<Rng>>::size(VEG_DECLVAL(Rng&)));

// prevents forwarding ctor from hiding copy/move ctors
// disables implicitly generated deduction guides
template <typename T>
struct slice_ctor_common {
  slice_ctor_common() noexcept = default;

  constexpr slice_ctor_common(T* data, i64 count) noexcept
      : m_begin{data}, m_count{count} {
    VEG_DEBUG_ASSERT(count >= 0);
    if (data == nullptr) {
      VEG_DEBUG_ASSERT(count == 0);
    }
  }

  // COMPAT: check if slice_ctor_common is a base of Rng
  template <typename Rng>
  constexpr slice_ctor_common /* NOLINT(hicpp-explicit-conversions,
                         bugprone-forwarding-reference-overload) */
      (Rng&& rng,
       VEG_REQUIRES_CTOR((                                 //
           meta::is_convertible_v<                         //
               meta::remove_pointer_t<data_t<Rng&>> (*)[], //
               T (*)[]> &&                                 //
           meta::is_constructible_v<i64, size_t_<Rng&>>    //
           ))) noexcept
      : slice_ctor_common(
            has_data<meta::remove_cvref_t<Rng>>::data(rng),
            i64(has_data<meta::remove_cvref_t<Rng>>::size(rng))) {}

  T* m_begin = nullptr;
  i64 m_count = 0;
};

template <typename T>
struct slice_ctor : slice_ctor_common<T> {
  using slice_ctor_common<T>::slice_ctor_common;
};

template <typename T>
struct slice_ctor<T const> : slice_ctor_common<T const> {
  using slice_ctor_common<T const>::slice_ctor_common;

  constexpr slice_ctor(
      init_list_t /*tag*/, std::initializer_list<T> lst) noexcept
      : slice_ctor_common<T const>{lst.begin(), static_cast<i64>(lst.size())} {}
};
} // namespace internal

template <typename T>
struct slice : private internal::slice_ctor<T> {
  using internal::slice_ctor<T>::slice_ctor;
  VEG_NODISCARD constexpr auto data() const noexcept -> T* {
    return internal::slice_ctor_common<T>::m_begin;
  }
  VEG_NODISCARD constexpr auto size() const noexcept -> i64 {
    return internal::slice_ctor_common<T>::m_count;
  }
};

template <>
struct slice<void> : slice<unsigned char> {
  using slice<unsigned char>::slice;
  template <typename T>
  slice /* NOLINT(hicpp-explicit-conversions) */ (
      slice<T> s,
      VEG_REQUIRES_CTOR(
          !meta::is_const<T>::value && //
          meta::is_trivially_copyable<T>::value)) noexcept
      : slice(s.data(), s.size() * static_cast<i64>(sizeof(T))) {}
};
template <>
struct slice<void const> : slice<unsigned char const> {
  using slice<unsigned char const>::slice;
  template <typename T>
  slice /* NOLINT(hicpp-explicit-conversions) */ (
      slice<T> s,
      VEG_REQUIRES_CTOR(std::is_trivially_copyable<T>::value)) noexcept
      : slice{s.data(), s.size() * static_cast<i64>(sizeof(T))} {}
};

namespace make {
namespace fn {
struct slice_fn {
  template <typename Rng>
  auto operator()(Rng&& rng) const noexcept -> VEG_REQUIRES_RET(
      (std::is_constructible< //
          slice<meta::remove_pointer_t<decltype(
              internal::has_data<meta::remove_cvref_t<Rng>>::data(rng))>>, //
          Rng&&                                                            //
          >::value),
      slice<meta::remove_pointer_t<decltype(
          internal::has_data<meta::remove_cvref_t<Rng>>::data(rng))>>) {
    return {VEG_FWD(rng)};
  }
};
} // namespace fn
VEG_ODR_VAR(slice, fn::slice_fn);
} // namespace make

} // namespace veg

#endif /* end of include guard VEG_SPAN_HPP_CBT4079WS */

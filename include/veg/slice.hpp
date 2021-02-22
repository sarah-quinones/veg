#ifndef VEG_SPAN_HPP_CBT4079WS
#define VEG_SPAN_HPP_CBT4079WS

#include "veg/assert.hpp"
#include "veg/option.hpp"

namespace veg {

template <typename T, i64 N>
struct array {
  static_assert(N > 0, "");
  T self[static_cast<usize>(N)];

  VEG_NODISCARD auto data() noexcept -> T* { return self; }
  VEG_NODISCARD auto data() const noexcept -> T const* { return self; }
  VEG_NODISCARD auto size() const noexcept -> i64 { return N; }

  VEG_NODISCARD auto operator[](i64 i) && noexcept -> T&& = delete;
  VEG_NODISCARD auto operator[](i64 i) & noexcept -> T& {
    VEG_ASSERT_ALL_OF(i >= 0, i < N);
    return self[i];
  }
  VEG_NODISCARD auto operator[](i64 i) const& noexcept -> T const& {
    VEG_ASSERT_ALL_OF(i >= 0, i < N);
    return self[i];
  }

  VEG_NODISCARD auto at(i64 i) && noexcept -> option<T&&> = delete;
  VEG_NODISCARD auto at(i64 i) & noexcept -> option<T&> {
    if (i >= 0 && i < N) {
      return {some, self[i]};
    }
    return none;
  }
  VEG_NODISCARD auto at(i64 i) const& noexcept -> option<T const&> {
    if (i >= 0 && i < N) {
      return {some, self[i]};
    }
    return none;
  }
};

namespace internal {

struct member_fn_data_size {
  template <typename R, typename T>
  using dtype_r = decltype(void(static_cast<R* const*>(
      static_cast<decltype(VEG_DECLVAL(T&).data()) const*>(nullptr))));

  template <typename T>
  using dtype = decltype(void(VEG_DECLVAL(T&).data()));

  template <typename T>
  using stype = decltype(void(static_cast<i64>(VEG_DECLVAL(T&).size())));

  template <typename T>
  static constexpr auto d(T& arg) noexcept -> decltype(arg.data()) {
    return arg.data();
  }
  template <typename T>
  static constexpr auto s(T& arg) noexcept -> i64 {
    return i64(arg.size());
  }
};
template <typename R, typename T>
struct has_members_r
    : meta::conjunction<
          meta::is_detected<member_fn_data_size::dtype_r, R, T&>,
          meta::is_detected<member_fn_data_size::stype, T&>>,
      member_fn_data_size {};
template <typename T>
struct has_members : meta::conjunction<
                         meta::is_detected<member_fn_data_size::dtype, T&>,
                         meta::is_detected<member_fn_data_size::stype, T&>>,
                     member_fn_data_size {};

struct array_data_size {
  template <typename T>
  static constexpr auto d(T& arg) noexcept -> decltype(+arg) {
    return +arg;
  }
  template <typename T, usize N>
  static constexpr auto s(T (&/*arg*/)[N]) noexcept -> i64 {
    return i64(N);
  }
};

template <typename R, typename T>
struct has_array_data2
    : meta::is_convertible<
          T&,
          R (&)[sizeof(T) / sizeof(decltype(VEG_DECLVAL(T)[0]))]> {};

template <typename R, typename T>
struct has_array_data_r
    : meta::conjunction<meta::bounded_array<T>, has_array_data2<R, T>>,
      array_data_size {};

template <typename T>
struct has_array_data : meta::bounded_array<T>, array_data_size {};

template <typename R, typename T>
struct has_data_r
    : meta::disjunction<has_members_r<R, T&>, has_array_data_r<R, T>> {};
template <typename T>
struct has_data : meta::disjunction<has_members<T&>, has_array_data<T>> {};

// prevents forwarding ctor from hiding copy/move ctors
// disables implicitly generated deduction guides
template <typename T>
struct slice_ctor_common {
  slice_ctor_common() noexcept = default;

  constexpr slice_ctor_common(T* data, i64 count) noexcept
      : m_begin{(
            VEG_DEBUG_ASSERT(count >= 0),
            (count > 0 ? VEG_DEBUG_ASSERT(data) : void(0)),
            data)},
        m_count{count} {}

  // COMPAT: check if slice_ctor_common is a base of Rng
  VEG_TEMPLATE(
      (typename Rng),
      requires(internal::has_data_r<T, meta::remove_cvref_t<Rng>>::value),
      constexpr slice_ctor_common, /* NOLINT(hicpp-explicit-conversions,
                             bugprone-forwarding-reference-overload) */
      (rng, Rng&&))
  noexcept
      : slice_ctor_common(
            static_cast<T*>(
                internal::has_data_r<T, meta::remove_cvref_t<Rng>>::d(rng)),
            static_cast<i64>(
                has_data_r<T, meta::remove_cvref_t<Rng>>::s(rng))) {}

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

  constexpr slice_ctor(std::initializer_list<T> lst) noexcept
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
  VEG_NODISCARD constexpr auto operator[](i64 i) const noexcept -> T& {
    return VEG_ASSERT_ALL_OF( //
               (i >= 0),
               (i < size())),
           *(data() + i);
  }
  VEG_NODISCARD VEG_CPP14(constexpr) auto at(i64 i) const noexcept
      -> option<T&> {
    if (i > 0 || i <= size()) {
      return {some, *(data() + i)};
    }
    return none;
  }
};

template <>
struct slice<void> : slice<unsigned char> {
  using slice<unsigned char>::slice;

  VEG_TEMPLATE(
      (typename T),
      requires !meta::const_<T>::value && meta::trivially_copyable<T>::value,
      slice, /* NOLINT(hicpp-explicit-conversions) */
      (s, slice<T>))
  noexcept
      : slice(
            reinterpret_cast<unsigned char*>(s.data()),
            s.size() * static_cast<i64>(sizeof(T))) {}
};
template <>
struct slice<void const> : slice<unsigned char const> {
  using slice<unsigned char const>::slice;
  VEG_TEMPLATE(
      (typename T),
      requires meta::trivially_copyable<T>::value,
      slice, /* NOLINT(hicpp-explicit-conversions) */
      (s, slice<T>))
  noexcept
      : slice{
            reinterpret_cast<unsigned char const*>(s.data()),
            s.size() * static_cast<i64>(sizeof(T))} {}
};

namespace make {
namespace fn {
struct slice_fn {
  VEG_TEMPLATE(
      (typename Rng),
      requires(meta::constructible< //
               slice<meta::remove_pointer_t<
                   decltype(internal::has_data<meta::remove_cvref_t<Rng>>::d(
                       VEG_DECLVAL(Rng&)))>>,
               Rng&&>::value),
      auto
      operator(),
      (rng, Rng&&))
  const noexcept->slice<meta::remove_pointer_t<decltype(
      internal::has_data<meta::remove_cvref_t<Rng>>::d(rng))>> {
    return {VEG_FWD(rng)};
  }
};
} // namespace fn
VEG_ODR_VAR(slice, fn::slice_fn);
} // namespace make

namespace meta {
template <typename T>
struct mostly_trivial<slice<T>> : bool_constant<true> {};
} // namespace meta
} // namespace veg

#endif /* end of include guard VEG_SPAN_HPP_CBT4079WS */

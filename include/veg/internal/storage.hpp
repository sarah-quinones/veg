#ifndef __VEG_STORAGE_HPP_X0B4XDKES
#define __VEG_STORAGE_HPP_X0B4XDKES

#include "veg/internal/type_traits.hpp"
#include "veg/internal/memory.hpp"

namespace veg {
namespace internal {
namespace storage {

template <typename T, typename Arg>
struct arg_ctor_to_fn {
  Arg&& arg;
  HEDLEY_ALWAYS_INLINE constexpr auto operator()() const
      noexcept(__VEG_CONCEPT(meta::nothrow_constructible<T, Arg&&>)) -> T {
    return T{VEG_FWD(arg)};
  }
};
template <typename T>
using copy_ctor_fn = arg_ctor_to_fn<T, T const&>;
template <typename T>
using move_ctor_fn = arg_ctor_to_fn<T, T&&>;

struct hidden_tag1 {};
struct hidden_tag2 {};

template <typename T, bool Rebind = true>
struct storage {
  T inner;

  storage() = default;

  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE constexpr storage(
      hidden_tag1 /*tag*/,
      Fn&& fn,
      Args&&... args) noexcept(__VEG_CONCEPT(meta::
                                                 nothrow_invocable<
                                                     Fn&&,
                                                     Args&&...>))
      : inner(invoke(VEG_FWD(fn), VEG_FWD(args)...)) {}

  template <typename U>
  HEDLEY_ALWAYS_INLINE explicit constexpr storage(
      hidden_tag2 /*tag*/,
      U&& arg) noexcept(__VEG_CONCEPT(meta::nothrow_constructible<T, U&&>))
      : inner(static_cast<T>(VEG_FWD(arg))) {}

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto _get() const noexcept
      -> T const& {
    return inner;
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mut() noexcept -> T& {
    return inner;
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mov_ref() && noexcept
      -> T&& {
    return static_cast<T&&>(inner);
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mov() && noexcept(
      __VEG_CONCEPT(meta::nothrow_move_constructible<T>)) -> T {
    return static_cast<T&&>(inner);
  }
};

template <typename T>
struct ref_base {
  T&& inner;

  HEDLEY_ALWAYS_INLINE explicit constexpr ref_base(
      hidden_tag2 /*tag*/, T&& arg) noexcept
      : inner{VEG_FWD(arg)} {}
  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE constexpr ref_base(
      hidden_tag1 /*tag*/,
      Fn&& fn,
      Args&&... args) noexcept(__VEG_CONCEPT(meta::
                                                 nothrow_invocable<
                                                     Fn&&,
                                                     Args&&...>))
      : inner(invoke(VEG_FWD(fn), VEG_FWD(args)...)) {}

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto _get() const noexcept -> T& {
    return inner;
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mut() noexcept -> T& {
    return inner;
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mov_ref() && noexcept
      -> T&& {
    return VEG_FWD(inner);
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mov() && noexcept
      -> T&& {
    return VEG_FWD(inner);
  }
};

template <typename T>
struct storage<T&, false> : ref_base<T&> {
  using ref_base<T&>::ref_base;
};

template <typename T>
struct storage<T&&, false> : ref_base<T&&> {
  using ref_base<T&&>::ref_base;
};

template <typename T>
HEDLEY_ALWAYS_INLINE constexpr auto as_lvalue(T&& arg) noexcept -> T& {
  return arg;
}
struct null_key {};

template <typename T>
struct storage<T&, true> {
  T* inner_ptr = nullptr;

  HEDLEY_ALWAYS_INLINE explicit constexpr storage(
      hidden_tag2 /*tag*/, T& arg) noexcept
      : inner_ptr{addressof(arg)} {}
  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE constexpr storage(
      hidden_tag1 /*tag*/,
      Fn&& fn,
      Args&&... args) noexcept(__VEG_CONCEPT(meta::
                                                 nothrow_invocable<
                                                     Fn&&,
                                                     Args&&...>))
      : inner_ptr(addressof(invoke(VEG_FWD(fn), VEG_FWD(args)...))) {}

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto _get() const noexcept -> T& {
    return *inner_ptr;
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mut() noexcept -> T& {
    return *inner_ptr;
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mov_ref() && noexcept
      -> T& {
    return *inner_ptr;
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mov() && noexcept -> T& {
    return *inner_ptr;
  }

private:
  HEDLEY_ALWAYS_INLINE explicit constexpr storage(
      null_key /*unused*/) noexcept {}
  HEDLEY_ALWAYS_INLINE static constexpr auto null() noexcept -> storage {
    return storage{null_key{}};
  }
  template <typename U>
  friend struct meta::value_sentinel_for;
};

template <typename T>
struct storage<T&&, true> : meta::internal::delete_copy {
  T* inner_ptr = nullptr;

  storage() = default;
  HEDLEY_ALWAYS_INLINE explicit constexpr storage(
      hidden_tag2 /*tag*/, T&& arg) noexcept
      : inner_ptr{addressof(arg)} {}
  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE constexpr storage(
      hidden_tag1 /*tag*/,
      Fn&& fn,
      Args&&... args) noexcept(__VEG_CONCEPT(meta::
                                                 nothrow_invocable<
                                                     Fn&&,
                                                     Args&&...>))
      : inner_ptr(addressof(internal::storage::as_lvalue(
            invoke(VEG_FWD(fn), VEG_FWD(args)...)))) {}

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto _get() const noexcept -> T& {
    return *inner_ptr;
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mut() noexcept -> T& {
    return *inner_ptr;
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mov_ref() && noexcept
      -> T&& {
    return static_cast<T&&>(*inner_ptr);
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto get_mov() && noexcept
      -> T&& {
    return static_cast<T&&>(*inner_ptr);
  }

private:
  HEDLEY_ALWAYS_INLINE explicit constexpr storage(
      null_key /*unused*/) noexcept {}
  HEDLEY_ALWAYS_INLINE static constexpr auto null() noexcept -> storage {
    return storage{null_key{}};
  }
  template <typename U>
  friend struct meta::value_sentinel_for;
};

// intentionally unsupported. please don't
template <typename T>
struct storage<T const&&, true>;
template <typename T>
struct storage<T const&&, false>;

} // namespace storage
} // namespace internal

template <typename T>
struct meta::value_sentinel_for<veg::internal::storage::storage<T&>>
    : std::integral_constant<i64, 1> {
  HEDLEY_ALWAYS_INLINE static constexpr auto invalid(i64 i) noexcept
      -> ::veg::internal::storage::storage<T&> {
    return ((i == 0) ? (void)0 : terminate()),
           ::veg::internal::storage::storage<T&>::null();
  }
  HEDLEY_ALWAYS_INLINE static constexpr auto
  id(::veg::internal::storage::storage<T&> const& arg) noexcept -> i64 {
    return arg.inner_ptr == nullptr ? 0 : -1;
  }
};

template <typename T>
struct meta::value_sentinel_for<veg::internal::storage::storage<T&&>>
    : std::integral_constant<i64, 1> {
  HEDLEY_ALWAYS_INLINE static constexpr auto invalid(i64 i) noexcept
      -> ::veg::internal::storage::storage<T&&> {
    return ((i == 0) ? (void)0 : terminate()),
           ::veg::internal::storage::storage<T&&>::null();
  }
  HEDLEY_ALWAYS_INLINE static constexpr auto
  id(::veg::internal::storage::storage<T&&> const& arg) noexcept -> i64 {
    return arg.inner_ptr == nullptr ? 0 : -1;
  }
};

template <typename T>
struct meta::value_sentinel_for<veg::internal::storage::storage<T>>
    : std::integral_constant<i64, meta::value_sentinel_for<T>::value> {
  HEDLEY_ALWAYS_INLINE static constexpr auto invalid(i64 i) noexcept
      -> ::veg::internal::storage::storage<T> {
    return ::veg::internal::storage::storage<T>{
        veg::internal::storage::hidden_tag2{},
        meta::value_sentinel_for<T>::invalid(i)};
  }
  HEDLEY_ALWAYS_INLINE static constexpr auto
  id(::veg::internal::storage::storage<T> const& arg) noexcept -> i64 {
    return meta::value_sentinel_for<T>::id(arg.inner);
  }
};

} // namespace veg

#endif /* end of include guard __VEG_STORAGE_HPP_X0B4XDKES */

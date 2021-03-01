#ifndef __VEG_NEW_HPP_43XG2FSKS
#define __VEG_NEW_HPP_43XG2FSKS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/std.hpp"

#if __VEG_HAS_BUILTIN(__builtin_launder) || __GNUC__ >= 7
#define VEG_LAUNDER(p) __builtin_launder(p)
#elif __cplusplus >= 201703L
#define VEG_LAUNDER(p) ::std::launder(p)
#else
#define VEG_LAUNDER(p) (+p)
#endif

namespace veg {
namespace internal {
auto opaque_memmove(void* dest, void const* src, usize n) noexcept -> void*;
} // namespace internal
namespace mem {

#if !(__VEG_HAS_BUILTIN(__builtin_addressof) || __cplusplus >= 201703L)

namespace internal {
namespace _addr {
struct member_addr {
  template <typename T>
  using type = decltype(void(__VEG_DECLVAL(T&).operator&()));

  template <typename T>
  HEDLEY_ALWAYS_INLINE static auto apply(T& var) noexcept -> T* {
    using char_ref = char&;
    return static_cast<T*>(static_cast<void*>(&char_ref(var)));
  }
};
struct adl_addr : member_addr {
  template <typename T>
  using type = decltype(void(operator&(__VEG_DECLVAL(T&))));
};
struct builtin_addr : std::true_type {
  template <typename T>
  HEDLEY_ALWAYS_INLINE static constexpr auto apply(T& var) noexcept -> T* {
    return &var;
  }
};
template <typename T>
struct has_member_addr : meta::is_detected<member_addr::type, T&>,
                         member_addr {};
template <typename T>
struct has_adl_addr : meta::is_detected<adl_addr::type, T&>, adl_addr {};

template <typename T>
struct addr_impl
    : meta::disjunction<has_member_addr<T>, has_adl_addr<T>, builtin_addr> {};

} // namespace _addr
} // namespace internal

#endif

namespace internal {
namespace _ctor_at {

struct uniform_init_ctor {
  template <typename T, typename... Args>
  HEDLEY_ALWAYS_INLINE static auto apply(T* mem, Args&&... args) noexcept(
      noexcept(new (mem) T{VEG_FWD(args)...})) -> T* {
    return new (mem) T{VEG_FWD(args)...};
  }
};
struct ctor {
  template <typename T, typename... Args>
  HEDLEY_ALWAYS_INLINE static __VEG_CPP20(constexpr) auto apply(
      T* mem,
      Args&&... args) noexcept(meta::nothrow_constructible<T, Args&&...>::value)
      -> T* {
#if __cplusplus >= 202002L
    return ::std::construct_at(mem, VEG_FWD(args)...);
#else
    return new (mem) T(VEG_FWD(args)...);
#endif
  }
};

template <typename T, typename... Args>
struct uniform_constructible : meta::uniform_init_constructible<T, Args&&...>,
                               uniform_init_ctor {};
template <typename T, typename... Args>
struct constructible : meta::constructible<T, Args&&...>, ctor {};

template <typename T, typename... Args>
struct ctor_at_impl : meta::disjunction<
                          constructible<T, Args&&...>,
                          uniform_constructible<T, Args&&...>> {};

template <typename Fn>
struct fn_to_convertible {
  Fn&& fn;
  HEDLEY_ALWAYS_INLINE constexpr explicit
  operator meta::invoke_result_t<Fn&&>() const
      noexcept(meta::nothrow_invocable<Fn&&>::value) {
    return VEG_FWD(fn)();
  }
};
} // namespace _ctor_at
} // namespace internal

namespace fn {
struct construct_at {
  VEG_TEMPLATE(
      (typename T, typename... Args),
      requires(
          !meta::const_<T>::value &&
          meta::disjunction<
              meta::constructible<T, Args&&...>,
              meta::uniform_init_constructible<T, Args&&...>>::value),
      HEDLEY_ALWAYS_INLINE __VEG_CPP20(constexpr) auto
      operator(),
      (mem, T*),
      (... args, Args&&))
  const noexcept(meta::nothrow_constructible<T, Args&&...>::value)->T* {
    return internal::_ctor_at::ctor_at_impl<T, Args&&...>::apply(
        mem, VEG_FWD(args)...);
  }
};

struct construct_with {
  VEG_TEMPLATE(
      (typename T, typename Fn),
      requires !meta::const_<T>::value &&
          __VEG_SAME_AS(T, (meta::detected_t<meta::invoke_result_t, Fn&&>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP20(constexpr) auto
      operator(),
      (mem, T*),
      (fn, Fn&&))
  const noexcept(meta::nothrow_invocable<Fn&&>::value)->T* {
#if __cplusplus >= 202002L
    return ::std::construct_at(
        mem, internal::_ctor_at::fn_to_convertible<Fn&&>{VEG_FWD(fn)});
#else
    return new (mem) T(VEG_FWD(fn)());
#endif
  }
};

struct destroy_at {
  VEG_TEMPLATE(
      (typename T, typename... Args),
      requires !meta::const_<T>::value,
      HEDLEY_ALWAYS_INLINE __VEG_CPP20(constexpr) void
      operator(),
      (mem, T*))
  const noexcept { mem->~T(); }
};

struct addressof {
  template <typename T>
  constexpr void operator()(T const&& var) const noexcept = delete;

  template <typename T>
  HEDLEY_ALWAYS_INLINE constexpr auto operator()(T& var) const noexcept -> T* {
#if __VEG_HAS_BUILTIN(__builtin_addressof)

    return __builtin_addressof(var);

#elif __cplusplus >= 201703L

    return ::std::addressof(var);

#else

    return internal::_addr::addr_impl<T>::apply(var);

#endif
  }
};

namespace internal {
namespace _reloc {

enum struct which {
  trivial,
  nothrow_move,
  copy,
  throw_move,
};

template <which Which>
struct reloc_impl;

template <>
struct reloc_impl<which::nothrow_move> {
  template <typename T>
  static __VEG_CPP20(constexpr) void apply(T* dest, T* src, i64 n) noexcept {
    T* end = dest + n;
    for (; dest < end; ++dest, ++src) {
      fn::construct_at{}(dest, static_cast<T&&>(*src));
      fn::destroy_at{}(*src);
    }
  }
};

template <>
struct reloc_impl<which::trivial> {
  VEG_TEMPLATE(
      typename T,
      requires sizeof(T) >= 1,
      static __VEG_CPP20(constexpr) void apply,
      (dest, T*),
      (src, T*),
      (n, i64))

  noexcept {
    static_assert(
        meta::nothrow_move_constructible<T>::value,
        "is T really trivially relocatable?");

    __VEG_CPP20(

        if (std::is_constant_evaluated()) {
          reloc_impl<which::nothrow_move>::apply(dest, src, n);
        } else

    )

    {
      veg::internal::opaque_memmove(
          dest, src, static_cast<usize>(n) * sizeof(T));
    }
  }
};

template <typename T>
struct destroy_range_fn {
  bool const& success;
  T* const& cleanup_ptr;
  i64 const& size;

  __VEG_CPP20(constexpr)
  void operator()() const noexcept {
    if (!success) {
      for (i64 i = 0; i < size; ++i) {
        fn::destroy_at{}(cleanup_ptr + i);
      }
    }
  }
};

template <typename Cast, typename T>
static __VEG_CPP20(constexpr) void reloc_fallible(
    T* dest,
    T* src,
    i64 n) noexcept(meta::nothrow_constructible<T, T const&>::value) {

  bool success = false;
  T* cleanup_ptr = dest;
  i64 i = 0;
  auto&& fn = make::defer(destroy_range_fn<T>{});

  for (; i < n; ++i) {
    fn::construct_at{}(dest + i, static_cast<Cast>(src[i]));
  }

  success = true;
  cleanup_ptr = src;
}

template <>
struct reloc_impl<which::copy> {
  template <typename T>
  static __VEG_CPP20(constexpr) void apply(T* dest, T* src, i64 n) noexcept(
      meta::nothrow_constructible<T, T const&>::value) {
    _reloc::reloc_fallible<T const&>(dest, src, n);
  }
};

template <>
struct reloc_impl<which::throw_move> {
  template <typename T>
  HEDLEY_ALWAYS_INLINE static __VEG_CPP20(constexpr) void apply(
      T* dest, T* src, i64 n) //
      noexcept(false) {
    _reloc::reloc_fallible<T&&>(dest, src, n);
  }
};

} // namespace _reloc
} // namespace internal

struct relocate_n {
  VEG_TEMPLATE(
      (typename T),
      requires !meta::const_<T>::value &&
          (meta::move_constructible<T>::value ||
           meta::constructible<T, T const&>::value),
      HEDLEY_ALWAYS_INLINE __VEG_CPP20(constexpr) void
      operator(),
      (dest, T*),
      (src, T*),
      (n, i64))
  const noexcept(
      meta::nothrow_move_constructible<T>::value ||
      meta::nothrow_constructible<T, T const&>::value) {
    namespace impl = internal::_reloc;
    impl::reloc_impl<
        meta::trivially_relocatable<T>::value               //
            ? impl::which::trivial                          //
            : meta::nothrow_move_constructible<T>::value    //
                  ? impl::which::nothrow_move               //
                  : meta::constructible<T, T const&>::value //
                        ? impl::which::copy                 //
                        : impl::which::throw_move           //
        >::apply(dest, src, n);
  }
};
} // namespace fn

__VEG_ODR_VAR(construct_at, fn::construct_at);
__VEG_ODR_VAR(construct_with, fn::construct_with);
__VEG_ODR_VAR(destroy_at, fn::destroy_at);
__VEG_ODR_VAR(relocate_n, fn::relocate_n);
__VEG_ODR_VAR(addressof, fn::addressof);

} // namespace mem
} // namespace veg

#endif /* end of include guard __VEG_NEW_HPP_43XG2FSKS */

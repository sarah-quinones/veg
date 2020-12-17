#ifndef VEG_NEW_HPP_43XG2FSKS
#define VEG_NEW_HPP_43XG2FSKS

#include "veg/internal/type_traits.hpp"

#include "veg/internal/std.hpp"

#if VEG_HAS_BUILTIN(__builtin_launder) || __GNUC__ >= 7
#define VEG_LAUNDER(p) __builtin_launder(p);
#elif __cplusplus >= 201703L
#define VEG_LAUNDER(p) ::std::launder(p)
#else
#define VEG_LAUNDER(p) (+p)
#endif

namespace veg {
namespace mem {

auto malloc(i64 sz) noexcept -> void*;
auto realloc(void* ptr, i64 sz) noexcept -> void*;
void free(void* ptr) noexcept;

#if !(VEG_HAS_BUILTIN(__builtin_addressof) || __cplusplus >= 201703L)

namespace _addr {
template <typename T, typename Enable = void>
struct has_member_addr_op : std::false_type {};
template <typename T>
struct has_member_addr_op<
    T,
    meta::void_t<decltype(VEG_DECLVAL(T&).operator&())>> : std::true_type {};

template <typename T, typename Enable = void>
struct has_nonmember_addr_op : std::false_type {};
template <typename T>
struct has_nonmember_addr_op<
    T,
    meta::void_t<decltype(operator&(VEG_DECLVAL(T&)))>> : std::true_type {};

template <bool Has_Overloaded_Address_Of /* true */>
struct addressof_impl {
  template <typename T>
  static auto apply(T& var) -> T* {
    using char_ref = char&;
    return static_cast<T*>(static_cast<void*>(&char_ref(var)));
  }
};

template <>
struct addressof_impl<false> {
  template <typename T>
  static constexpr auto apply(T& var) -> T* {
    return &var;
  }
};
} // namespace _addr

#endif

namespace _ctor_at {

template <i64 which /* = not_found */>
struct construct_at_impl;

template <>
struct construct_at_impl<0> {
  template <typename T, typename... Args>
  static VEG_CPP20(constexpr) auto apply(T* mem, Args&&... args) -> T* {
#if __cplusplus >= 202002L
    return ::std::construct_at(mem, VEG_FWD(args)...);
#else
    return new (mem) T(VEG_FWD(args)...);
#endif
  }
};
template <>
struct construct_at_impl<1> {
  template <typename T, typename... Args>
  static auto apply(T* mem, Args&&... args) -> T* {
    return new (mem) T{VEG_FWD(args)...};
  }
};

} // namespace _ctor_at

namespace fn {
struct construct_at_fn {
  template <typename T, typename... Args>
  VEG_CPP20(constexpr)
  auto operator()(T* mem, Args&&... args) const noexcept -> VEG_REQUIRES_RET(
      (meta::first_true<
           std::is_constructible<T, Args...>,
           meta::is_uniform_init_constructible<T, Args...>>::value !=
       meta::not_found),
      T*) {
    return _ctor_at::construct_at_impl<meta::first_true<
        std::is_constructible<T, Args...>,
        meta::is_uniform_init_constructible<T, Args...>>::value>::
        apply(mem, VEG_FWD(args)...);
  }
};

struct destroy_at_fn {
  template <typename T, typename... Args>
  VEG_CPP20(constexpr)
  void operator()(T* mem) const noexcept {

#if __cplusplus >= 202002L
    ::std::destroy_at(mem);
#else
    mem->~T();
#endif
  }
};

struct addressof_fn {
  template <typename T>
  constexpr void operator()(T const&& var) const noexcept = delete;

  template <typename T>
  constexpr auto operator()(T& var) const noexcept -> T* {
#if VEG_HAS_BUILTIN(__builtin_addressof)

    return __builtin_addressof(var);

#elif __cplusplus >= 201703L

    return ::std::addressof(var);

#else

    return _addr::addressof_impl< //
        (meta::first_true<
             _addr::has_member_addr_op<T>,                               //
             _addr::has_nonmember_addr_op<T>>::value != meta::not_found) //
        >::apply(var);

#endif
  }
};

struct align_next_fn {
  auto
  operator()(i64 alignment, i64 size, void*& ptr, i64& space) const noexcept
      -> void*;
};
} // namespace fn

VEG_ODR_VAR(construct_at, fn::construct_at_fn);
VEG_ODR_VAR(destroy_at, fn::destroy_at_fn);
VEG_ODR_VAR(addressof, fn::addressof_fn);
VEG_ODR_VAR(align_next, fn::align_next_fn);

} // namespace mem
} // namespace veg

#endif /* end of include guard VEG_NEW_HPP_43XG2FSKS */

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
struct member_addr {
  template <typename T>
  using type = decltype(void(VEG_DECLVAL(T&).operator&()));

  template <typename T>
  static auto apply(T& var) -> T* {
    using char_ref = char&;
    return static_cast<T*>(static_cast<void*>(&char_ref(var)));
  }
};
struct adl_addr : member_addr {
  template <typename T>
  using type = decltype(void(operator&(VEG_DECLVAL(T&))));
};
struct builtin_addr : std::true_type {
  template <typename T>
  static constexpr auto apply(T& var) -> T* {
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

#endif

namespace _ctor_at {

struct uniform_init_ctor {
  template <typename T, typename... Args>
  static auto apply(T* mem, Args&&... args) -> T* {
    return new (mem) T{VEG_FWD(args)...};
  }
};
struct ctor {
  template <typename T, typename... Args>
  static VEG_CPP20(constexpr) auto apply(T* mem, Args&&... args) -> T* {
#if __cplusplus >= 202002L
    return ::std::construct_at(mem, VEG_FWD(args)...);
#else
    return new (mem) T(VEG_FWD(args)...);
#endif
  }
};

template <typename T, typename... Args>
struct uniform_constructible
    : meta::is_uniform_init_constructible<T, Args&&...>,
      uniform_init_ctor {};
template <typename T, typename... Args>
struct constructible : meta::is_constructible<T, Args&&...>, ctor {};

template <typename T, typename... Args>
struct ctor_at_impl : meta::disjunction<
                          constructible<T, Args&&...>,
                          uniform_constructible<T, Args&&...>> {};

} // namespace _ctor_at

namespace fn {
struct construct_at_fn {
  VEG_TEMPLATE(
      (typename T, typename... Args),
      requires(_ctor_at::ctor_at_impl<T, Args&&...>::value),
      VEG_CPP20(constexpr) auto
      operator(),
      (mem, T*),
      (... args, Args&&))
  const noexcept(meta::is_nothrow_constructible<T, Args&&...>::value)->T* {
    return _ctor_at::ctor_at_impl<T, Args&&...>::apply(mem, VEG_FWD(args)...);
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

    return _addr::addr_impl<T>::apply(var);

#endif
  }
};

} // namespace fn

VEG_ODR_VAR(construct_at, fn::construct_at_fn);
VEG_ODR_VAR(destroy_at, fn::destroy_at_fn);
VEG_ODR_VAR(addressof, fn::addressof_fn);

} // namespace mem
} // namespace veg

#endif /* end of include guard VEG_NEW_HPP_43XG2FSKS */

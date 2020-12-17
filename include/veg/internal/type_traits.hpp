#ifndef VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S
#define VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S

#include "veg/internal/std.hpp"
#include "veg/internal/.external/hedley.h"
#include "veg/internal/typedefs.hpp"

#define VEG_ODR_VAR(name, obj)                                                 \
  namespace { /* NOLINT */                                                     \
  constexpr auto const& name /* NOLINT */ =                                    \
      ::veg::meta::internal::static_const<obj>::value;                         \
  }                                                                            \
  static_assert(true, "")

#define VEG_UNARY_TYPE_TRAIT(name)                                             \
  template <typename T>                                                        \
  constexpr bool const& name##_v = name<T>::value

#define VEG_IMPLICIT(...) __VA_ARGS__ /* NOLINT */

#define VEG_DECLVAL(...)                                                       \
  (static_cast < ::veg::meta::no_op<__VA_ARGS__> && (*)() > (nullptr)())

#if __cplusplus >= 201703L
#define VEG_DECLVAL_NOEXCEPT(...)                                              \
  (static_cast < ::veg::meta::no_op<__VA_ARGS__> &&                            \
   (*)() noexcept > (nullptr)())
#else
#define VEG_DECLVAL_NOEXCEPT(...)                                              \
  static_cast<::veg::meta::no_op<__VA_ARGS__>&&>(                              \
      *static_cast<::veg::meta::remove_reference_t<__VA_ARGS__>*>(nullptr))
#endif

#define VEG_FWD(x) static_cast<decltype(x)&&>(x)
#define VEG_MOV(x)                                                             \
  static_cast<typename ::std::remove_reference<decltype(x)>::type&&>(x)

// macros
#if __cplusplus >= 202002L
#define VEG_CPP20(...) __VA_ARGS__
#else
#define VEG_CPP20(...)
#endif

#if __cplusplus >= 201703L
#define VEG_CPP17(...) __VA_ARGS__
#define VEG_ALL_OF(...) (__VA_ARGS__ && ...)
#else
#define VEG_CPP17(...)
#define VEG_ALL_OF(...) (::veg::meta::all_of({__VA_ARGS__...}))
#endif

#if defined(__has_builtin)
#define VEG_HAS_BUILTIN(x) __has_builtin(x)
#else
#define VEG_HAS_BUILTIN(x) 0
#endif

#if __cplusplus >= 201703L
#define VEG_NODISCARD [[nodiscard]]
#elif defined(__clang__)
#define VEG_NODISCARD HEDLEY_WARN_UNUSED_RESULT
#else
#define VEG_NODISCARD
#endif

namespace veg {
namespace _req {
struct empty {};
} // namespace _req
} // namespace veg

#if __cplusplus >= 202002L
#define VEG_REQUIRES_RET(requirement, ...) __VA_ARGS__ requires(requirement)
#define VEG_REQUIRES_RET2(requirement, ...) __VA_ARGS__ requires(requirement)
#define VEG_REQUIRES_CTOR(...) ::veg::_req::empty = {}) requires(__VA_ARGS__
#else
#define VEG_REQUIRES_RET(requirement, ...)                                     \
  ::veg::meta::discard_1st<::veg::meta::enable_if_t<requirement>, __VA_ARGS__>
#define VEG_REQUIRES_RET2(requirement, ...)                                    \
  ::veg::meta::enable_if_t<requirement, __VA_ARGS__>
#define VEG_REQUIRES_CTOR(...)                                                 \
  ::veg::meta::discard_1st<                                                    \
      ::veg::meta::enable_if_t<__VA_ARGS__>,                                   \
      ::veg::_req::empty> = {}
#endif

#define VEG_DECLTYPE_RET(...)                                                  \
  ->decltype(__VA_ARGS__) { return __VA_ARGS__; }                              \
  static_assert(true, "")

namespace veg {

// dev-only
template <typename... Ts>
struct incomplete_t;
template <typename... Types, typename... Args>
auto show_types(Args&&...) -> incomplete_t<Types..., Args...>;

namespace meta {

constexpr i64 not_found = -1;

namespace internal {
template <typename T>
struct static_const {
  static constexpr T value{};
};
template <typename T>
constexpr T static_const<T>::value; // NOLINT(readability-redundant-declaration)

constexpr auto incr_or_not_found(i64 n) -> i64 {
  return n == not_found ? not_found : (n + 1);
}
} // namespace internal
constexpr auto all_of(std::initializer_list<bool> lst) noexcept -> bool {
  for (auto b : lst) { // NOLINT
    if (!b) {
      return false;
    }
  }
  return true;
}
constexpr auto any_of(std::initializer_list<bool> lst) noexcept -> bool {
  for (auto b : lst) { // NOLINT
    if (b) {
      return true;
    }
  }
  return false;
}

using std::is_constructible;
using std::is_convertible;

using std::is_same;

using std::is_arithmetic;
using std::is_const;
using std::is_enum;
using std::is_floating_point;
using std::is_function;
using std::is_integral;
using std::is_pointer;
using std::is_signed;
using std::is_trivially_copyable;

namespace {
template <typename U, typename V>
constexpr bool const& is_same_v = is_same<U, V>::value;
template <typename T, typename... Args>
constexpr bool const& is_constructible_v = is_constructible<T, Args...>::value;
template <typename From, typename To>
constexpr bool const& is_convertible_v = is_convertible<From, To>::value;
VEG_UNARY_TYPE_TRAIT(is_arithmetic);
VEG_UNARY_TYPE_TRAIT(is_const);
VEG_UNARY_TYPE_TRAIT(is_enum);
VEG_UNARY_TYPE_TRAIT(is_floating_point);
VEG_UNARY_TYPE_TRAIT(is_function);
VEG_UNARY_TYPE_TRAIT(is_integral);
VEG_UNARY_TYPE_TRAIT(is_pointer);
VEG_UNARY_TYPE_TRAIT(is_signed);
VEG_UNARY_TYPE_TRAIT(is_trivially_copyable);
} // namespace

namespace internal {
template <typename T>
struct make_unsigned : std::make_unsigned<T> {};

template <>
struct make_unsigned<signed char> {
  using type = unsigned char;
};
template <>
struct make_unsigned<signed short> {
  using type = unsigned short;
};
template <>
struct make_unsigned<signed int> {
  using type = unsigned int;
};
template <>
struct make_unsigned<signed long> {
  using type = unsigned long;
};
template <>
struct make_unsigned<signed long long> {
  using type = unsigned long long;
};
} // namespace internal
template <typename T>
using make_unsigned_t = typename internal::make_unsigned<T>::type;

namespace internal {
template <bool B>
struct conditional_;
template <>
struct conditional_<true> {
  template <typename T, typename F>
  using type = T;
};
template <>
struct conditional_<false> {
  template <typename T, typename F>
  using type = F;
};
} // namespace internal

template <bool B, typename T, typename F>
using conditional_t = typename internal::conditional_<B>::template type<T, F>;

template <bool B>
struct bool_constant : std::integral_constant<bool, B> {};

template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename U, typename V>
using discard_1st = V;

template <typename T>
using no_op = T;

// COMPAT: unused parameters are ignored, messes with SFINAE
template <typename...>
using void_t = void;

template <typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template <typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

template <typename T>
using remove_cvref_t =
    typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template <typename T>
using decay_t = typename std::decay<T>::type;

enum struct category_e {
  mut_lval,
  cst_lval,
  mut_rval,
  cst_rval,
};

template <typename T>
struct value_category;

template <typename T>
struct value_category<T&>
    : std::integral_constant<category_e, category_e::mut_lval> {};
template <typename T>
struct value_category<T const&>
    : std::integral_constant<category_e, category_e::cst_lval> {};
template <typename T>
struct value_category<T&&>
    : std::integral_constant<category_e, category_e::mut_rval> {};
template <typename T>
struct value_category<T const&&>
    : std::integral_constant<category_e, category_e::cst_rval> {};

template <typename T>
struct is_bounded_array : std::false_type {};

template <typename T, size_t N>
struct is_bounded_array<T[N]> : std::true_type {};

template <typename T>
using remove_extent_t = typename std::remove_extent<T>::type;

template <typename... Preds>
struct first_true;

template <>
struct first_true<> : std::integral_constant<i64, not_found> {
  static constexpr std::false_type found{};
};

template <typename First, typename... Preds>
struct first_true<First, Preds...>
    : std::integral_constant<
          i64,
          First::value
              ? 0
              : internal::incr_or_not_found(conditional_t<
                                            First::value,
                                            std::true_type,
                                            first_true<Preds...>>::value)> {
  static constexpr conditional_t<
      (First::value ? 0
                    : internal::incr_or_not_found(
                          conditional_t<
                              First::value,
                              std::true_type,
                              first_true<Preds...>>::value)) == not_found,
      std::false_type,
      std::true_type>
      found{};
};

namespace internal {
template <
    typename Enable,
    typename Default,
    template <typename...>
    class Ftor,
    typename... Args>
struct detector {
  using found = std::false_type;
  using type = Default;
};
template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector<void_t<Ftor<Args...>>, Default, Ftor, Args...> {
  using found = std::true_type;
  using type = Ftor<Args...>;
};
struct none {};
} // namespace internal

template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector : internal::detector<void, Default, Ftor, Args...> {};

template <template <class...> class Op, class... Args>
using is_detected = typename detector<internal::none, Op, Args...>::found;

template <template <class...> class Op, class... Args>
using detected_t = typename detector<internal::none, Op, Args...>::type;

namespace internal {
template <typename T, typename... Args>
using uniform_ctor = decltype(static_cast<void>(T{VEG_DECLVAL(Args)...}));

template <typename Enable, typename T, typename... Args>
struct is_uniform_init_constructible_impl
    : is_detected<internal::uniform_ctor, T, Args...> {};

} // namespace internal

template <typename T, typename... Args>
struct is_uniform_init_constructible
    : internal::is_uniform_init_constructible_impl<void, T, Args...> {};

template <typename... Ts>
struct dependent_true : std::true_type {};

} // namespace meta

template <typename T>
struct tag_t {};

template <typename T>
constexpr tag_t<T> tag{};

struct init_list_t {};

enum struct safety_e { safe, unsafe };

template <safety_e S>
struct safety_tag_t;

using safe_t = safety_tag_t<safety_e::safe>;
using unsafe_t = safety_tag_t<safety_e::unsafe>;

template <>
struct safety_tag_t<safety_e::safe>
    : std::integral_constant<safety_e, safety_e::safe> {};

namespace internal {
struct make_unsafe;
} // namespace internal

template <>
struct safety_tag_t<safety_e::unsafe>
    : private std::integral_constant<safety_e, safety_e::unsafe> {

  using std::integral_constant<safety_e, safety_e::unsafe>::value;
  using typename std::integral_constant<safety_e, safety_e::unsafe>::value_type;
  using typename std::integral_constant<safety_e, safety_e::unsafe>::type;
  using std::integral_constant<safety_e, safety_e::unsafe>::operator value_type;
  using std::integral_constant<safety_e, safety_e::unsafe>::operator();

  constexpr
  operator safe_t() const noexcept // NOLINT(hicpp-explicit-conversions)
  {
    return {};
  }

private:
  friend struct internal::make_unsafe;
  constexpr safety_tag_t() = default;
};

namespace internal {
struct make_unsafe {
  static constexpr unsafe_t value = {};
};
} // namespace internal

namespace { // NOLINT(cert-dcl59-cpp)
constexpr auto const& unsafe /* NOLINT */ = internal::make_unsafe::value;
}
VEG_ODR_VAR(safe, safe_t);
VEG_ODR_VAR(init_list, init_list_t);

namespace fn {
struct tag_invoke_fn {
  template <typename CP, typename... Args>
  constexpr auto operator()(tag_t<CP> tag, Args&&... args) const

      noexcept(noexcept(tag_invoke(tag, VEG_FWD(args)...)))

          -> decltype(tag_invoke(tag, VEG_FWD(args)...))

  {
    return tag_invoke(tag, VEG_FWD(args)...);
  }
};
} // namespace fn
VEG_ODR_VAR(tag_invoke, fn::tag_invoke_fn);
namespace meta {

template <typename CP, typename... Args>
using tag_invoke_result_t =
    decltype(veg::tag_invoke(tag_t<CP>{}, VEG_DECLVAL(Args&&)...));

namespace internal {
template <typename Enable, typename CP, typename... Args>
struct tag_invocable_impl : std::false_type {};
template <typename CP, typename... Args>
struct tag_invocable_impl<void_t<tag_invoke_result_t<CP, Args...>>, CP, Args...>
    : std::true_type {};

} // namespace internal
template <typename CP, typename... Args>
struct tag_invocable : internal::tag_invocable_impl<void, CP, Args...> {};
template <typename CP, typename... Args>
constexpr bool const& tag_invocable_v = tag_invocable<CP, Args...>::value;

} // namespace meta
} // namespace veg

#endif /* end of include guard VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S */

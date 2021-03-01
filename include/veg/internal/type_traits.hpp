#ifndef __VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S
#define __VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S

#include "veg/internal/std.hpp"
#include "veg/internal/typedefs.hpp"
#include "veg/internal/.external/hedley.ext.h"
#include "veg/internal/concept_macros.hpp"

#define __VEG_DEFAULTS(class_name)                                             \
  ~class_name() = default;                                                     \
  class_name /* NOLINT */ (class_name &&) = default;                           \
  class_name(class_name const&) = default;                                     \
  auto operator= /* NOLINT */(class_name&&)&->class_name& = default;           \
  auto operator=(class_name const&)&->class_name& = default

#if defined(VEG_ENABLE_CPP14_EXTENSIONS) || defined(__clang__) ||              \
    __cplusplus >= 201403L

#if HEDLEY_HAS_WARNING("-Wc++14-extensions")
#define __VEG_IGNORE_CPP14_EXTENSION_WARNING(...)                              \
  HEDLEY_DIAGNOSTIC_PUSH                                                       \
  HEDLEY_PRAGMA(clang diagnostic ignored "-Wc++14-extensions")                 \
  __VA_ARGS__                                                                  \
  HEDLEY_DIAGNOSTIC_POP
#else
#define __VEG_IGNORE_CPP14_EXTENSION_WARNING(...) __VA_ARGS__
#endif

#else
#define __VEG_IGNORE_CPP14_EXTENSION_WARNING(...)
#endif

#define __VEG_ODR_VAR(name, obj)                                               \
  namespace { /* NOLINT */                                                     \
  constexpr auto const& name /* NOLINT */ =                                    \
      ::veg::meta::internal::static_const<obj>::value;                         \
  }                                                                            \
  static_assert(true, "")

#define __VEG_DECLVAL(...)                                                     \
  (static_cast < ::veg::meta::no_op<__VA_ARGS__> && (*)() > (nullptr)())

#if __cplusplus >= 201703L
#define __VEG_DECLVAL_NOEXCEPT(...)                                            \
  (static_cast < ::veg::meta::no_op<__VA_ARGS__> &&                            \
   (*)() noexcept > (nullptr)())
#else
#define __VEG_DECLVAL_NOEXCEPT(...)                                            \
  static_cast<::veg::meta::no_op<__VA_ARGS__>&&>(                              \
      *static_cast<::veg::meta::remove_reference_t<__VA_ARGS__>*>(nullptr))
#endif

#define __VEG_DEDUCE_NOEXCEPT(type, ...)                                       \
  noexcept(noexcept(__VA_ARGS__))->type { return __VA_ARGS__; }                \
  static_assert(true, "")

#define __VEG_DEDUCE_RET(...)                                                  \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) {                     \
    return __VA_ARGS__;                                                        \
  }                                                                            \
  static_assert(true, "")

#define VEG_FWD(x) static_cast<decltype(x)&&>(x)

// disallows moving const rvalues
#define VEG_MOV(x)                                                             \
  static_cast<typename ::std::remove_cv<                                       \
      typename ::std::remove_reference<decltype(x)>::type>::type&&>(x)

#define __VEG_PP_LPAREN() (
#define __VEG_PP_RPAREN() )

#ifdef VEG_NO_INSTANTIATE
#define VEG_INSTANTIATE(fn, ...) static_assert(true, "")
#define VEG_INSTANTIATE_CLASS(fn, ...) static_assert(true, "")
#else
#define VEG_INSTANTIATE(fn, ...)                                               \
  __VEG_IMPL_INSTANTIATE(                                                      \
      fn,                                                                      \
      __VEG_PP_CAT(                                                            \
          __VEG_PP_CAT(_dummy_explicit_instantiation, __LINE__),               \
          __VEG_PP_CAT(_, __COUNTER__)),                                       \
      __VA_ARGS__)
#define __VEG_IMPL_INSTANTIATE(fn, name, ...)                                  \
  template <typename... Ts>                                                    \
  struct name {                                                                \
    void apply(Ts&&... args) { fn(VEG_FWD(args)...); }                         \
  };                                                                           \
  template struct name<__VA_ARGS__>

#define VEG_INSTANTIATE_CLASS(class_name, ...)                                 \
  template struct class_name<__VA_ARGS__>
#endif

// macros
#if __cplusplus >= 202002L
#define __VEG_CPP20(...) __VA_ARGS__
#else
#define __VEG_CPP20(...)
#endif

#if __cplusplus >= 201402L
#define __VEG_CPP14(...) __VA_ARGS__
#else
#define __VEG_CPP14(...)
#endif

#if __cplusplus >= 201703L
#define __VEG_CPP17(...) __VA_ARGS__
#else
#define __VEG_CPP17(...)
#endif

#if defined(__has_builtin)
#define __VEG_HAS_BUILTIN(x) __has_builtin(x)
#else
#define __VEG_HAS_BUILTIN(x) 0
#endif

#if __cplusplus >= 201703L
#define VEG_NODISCARD [[nodiscard]]
#elif defined(__clang__)
#define VEG_NODISCARD HEDLEY_WARN_UNUSED_RESULT
#else
#define VEG_NODISCARD
#endif

#if __cplusplus >= 201402L
#define __VEG_CPP11_DECLTYPE_AUTO(...)                                         \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) {                     \
    return __VA_ARGS__;                                                        \
  }                                                                            \
  static_assert(true, "")
#else
#define __VEG_CPP11_DECLTYPE_AUTO(...)                                         \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) {                     \
    return __VA_ARGS__;                                                        \
  }                                                                            \
  static_assert(true, "")
#endif

// deducing `this' please. :clauded:
#define __VEG_CVREF_DUPLICATE_2(decl, handler, args)                           \
  decl const& __VEG_CPP11_DECLTYPE_AUTO(                                       \
      handler(*this __VEG_PP_REMOVE_PAREN(args)));                             \
  decl&& __VEG_CPP11_DECLTYPE_AUTO(                                            \
      handler(VEG_MOV(*this) __VEG_PP_REMOVE_PAREN(args)))

#define __VEG_CVREF_DUPLICATE(decl, handler, args)                             \
  decl const& __VEG_CPP11_DECLTYPE_AUTO(                                       \
      handler(*this __VEG_PP_REMOVE_PAREN(args)));                             \
  decl& __VEG_CPP11_DECLTYPE_AUTO(handler(*this __VEG_PP_REMOVE_PAREN(args))); \
  decl&& __VEG_CPP11_DECLTYPE_AUTO(                                            \
      handler(VEG_MOV(*this) __VEG_PP_REMOVE_PAREN(args)))

// const&, &&
#define __VEG_CVREF_DUPLICATE_TEMPLATE_2(                                      \
    self, handler, args, tparams, requirement, attr_name, ...)                 \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self const&),                   \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  const& __VEG_CPP11_DECLTYPE_AUTO(                                            \
      handler(*this __VEG_PP_REMOVE_PAREN(args)));                             \
                                                                               \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self &&),                       \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  &&__VEG_CPP11_DECLTYPE_AUTO(                                                 \
      handler(VEG_MOV(*this) __VEG_PP_REMOVE_PAREN(args)))

// const&, &, &&
#define __VEG_CVREF_DUPLICATE_TEMPLATE(                                        \
    self, handler, args, tparams, requirement, attr_name, ...)                 \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self const&),                   \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  const& __VEG_CPP11_DECLTYPE_AUTO(                                            \
      handler(*this __VEG_PP_REMOVE_PAREN(args)));                             \
                                                                               \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self&),                         \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  &__VEG_CPP11_DECLTYPE_AUTO(handler(*this __VEG_PP_REMOVE_PAREN(args)));      \
                                                                               \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self &&),                       \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  &&__VEG_CPP11_DECLTYPE_AUTO(handler(*this __VEG_PP_REMOVE_PAREN(args)))

#define __VEG_IMPL_PARAM_EXPAND(r, _, param)                                   \
  __VEG_PP_COMMA_IF(__VEG_PP_IS_BEGIN_PARENS(param))                           \
  __VEG_PP_REMOVE_PAREN(__VEG_PP_IF(                                           \
      __VEG_PP_IS_BEGIN_PARENS(param),                                         \
      (__VEG_PP_TAIL param)__VEG_PP_HEAD param, ))

#define VEG_TEMPLATE(tparams, requirement, attr_name, ...)                     \
  __VEG_IMPL_TEMPLATE(                                                         \
      attr_name, tparams, requirement, __VEG_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#if __cplusplus >= 202002L
#define VEG_TEMPLATE_EXPLICIT(                                                 \
    cond, tparams, requirement, attr_name, args, ...)                          \
  VEG_TEMPLATE(                                                                \
      tparams,                                                                 \
      requirement,                                                             \
      explicit(cond) attr_name,                                                \
      __VEG_PP_REMOVE_PAREN(args))                                             \
  __VA_ARGS__
#else
#define VEG_TEMPLATE_EXPLICIT(                                                 \
    cond, tparams, requirement, attr_name, args, ...)                          \
  VEG_TEMPLATE(                                                                \
      (__VEG_PP_REMOVE_PAREN tparams,                                          \
       typename ::veg::meta::enable_if<(cond), int>::type = 0),                \
      requirement,                                                             \
      explicit attr_name,                                                      \
      __VEG_PP_REMOVE_PAREN(args))                                             \
  __VA_ARGS__                                                                  \
  VEG_TEMPLATE(                                                                \
      (__VEG_PP_REMOVE_PAREN tparams,                                          \
       typename ::veg::meta::enable_if<!(cond), unsigned>::type = 0),          \
      requirement,                                                             \
      attr_name,                                                               \
      __VEG_PP_REMOVE_PAREN(args))                                             \
  __VA_ARGS__
#endif

#define __VEG_IMPL_TEMPLATE(attr_name, tparams, requirement, args)             \
  __VEG_IMPL_TEMPLATE2(                                                        \
      attr_name,                                                               \
      tparams,                                                                 \
      __VEG_PP_CAT2(                                                           \
          __VEG_IMPL_REQUIRES_HANDLER,                                         \
          __VEG_PP_CAT2(__VEG_IMPL_PREFIX_, requirement) __VEG_PP_RPAREN()),   \
      __VEG_PP_SEQ_HEAD(args),                                                 \
      __VEG_PP_SEQ_TAIL(args))

#define __VEG_IMPL_PREFIX_requires _ONE __VEG_PP_LPAREN()
#define __VEG_IMPL_PREFIX_requires_all _ALL __VEG_PP_LPAREN()

#if __cplusplus >= 202002L
#define __VEG_IMPL_TEMPLATE2(                                                  \
    attr_name, tparams, requirement, first_param, args)                        \
  template <__VEG_PP_REMOVE_PAREN(tparams)>                                    \
  requires(requirement) attr_name /* NOLINT */ (                               \
      __VEG_PP_TAIL first_param __VEG_PP_HEAD first_param                      \
          __VEG_PP_SEQ_FOR_EACH(__VEG_IMPL_PARAM_EXPAND, _, args))
#else
#define __VEG_IMPL_TEMPLATE2(                                                  \
    attr_name, tparams, requirement, first_param, args)                        \
  template <__VEG_PP_REMOVE_PAREN(tparams)>                                    \
  attr_name /* NOLINT */ (                                                     \
      ::veg::meta::discard_1st<                                                \
          typename ::veg::meta::enable_if<(requirement)>::type,                \
          __VEG_PP_TAIL first_param> __VEG_PP_HEAD first_param                 \
          __VEG_PP_SEQ_FOR_EACH(__VEG_IMPL_PARAM_EXPAND, _, args))

#endif

#define VEG_DECLTYPE_RET(...)                                                  \
  ->decltype(__VA_ARGS__) { return __VA_ARGS__; }                              \
  static_assert(true, "")

#define __VEG_HAS_BUILTIN_OR_0(true, false) __VEG_PP_REMOVE_PAREN(false)
#define __VEG_HAS_BUILTIN_OR_1(true, false) __VEG_PP_REMOVE_PAREN(true)
#define __VEG_HAS_BUILTIN_OR(builtin, true, false)                             \
  __VEG_PP_CAT(__VEG_HAS_BUILTIN_OR_, __VEG_HAS_BUILTIN(builtin))(true, false)

#define __VEG_SAME_AS(T, U)                                                    \
  __VEG_HAS_BUILTIN_OR(                                                        \
      __is_same,                                                               \
      __is_same(__VEG_PP_REMOVE_PAREN(T), __VEG_PP_REMOVE_PAREN(U)),           \
      (::veg::meta::                                                           \
           same_as<__VEG_PP_REMOVE_PAREN(T), __VEG_PP_REMOVE_PAREN(U)>))

namespace veg {

// dev-only
template <typename... Ts>
struct incomplete_t;
template <typename... Types, typename... Args>
auto show_types(Args&&...) -> incomplete_t<Types..., Args...>;

namespace meta {
using std::false_type;
using std::true_type;

template <bool B>
struct bool_constant : std::integral_constant<bool, B> {};

namespace internal {
using int_arr = int[];

template <typename T>
struct static_const {
  static constexpr T value{};
};
template <typename T>
constexpr T static_const<T>::value; // NOLINT(readability-redundant-declaration)
} // namespace internal
template <typename T>
constexpr auto max2(T a, T b) noexcept -> T {
  return a > b ? a : b;
}
template <typename T>
constexpr auto min2(T a, T b) noexcept -> T {
  return a < b ? a : b;
}

template <typename T>
constexpr auto max_of_slice(T const* arr, i64 size) noexcept -> T {
  return size == 1 ? arr[0]
                   : meta::max2(arr[0], max_of_slice(arr + 1, size - 1));
}

template <typename T>
constexpr auto max_of(std::initializer_list<T> lst) noexcept -> T {
  return meta::max_of_slice(lst.begin(), static_cast<i64>(lst.size()));
}

constexpr auto all_of_slice(bool const* arr, i64 size) noexcept -> bool {
  return size == 0 ? true : (arr[0] && meta::all_of_slice(arr + 1, size - 1));
}
constexpr auto all_of(std::initializer_list<bool> lst) noexcept -> bool {
  return meta::all_of_slice(lst.begin(), static_cast<i64>(lst.size()));
}

constexpr auto any_of_slice(bool const* arr, i64 size) noexcept -> bool {
  return size == 0 ? false : (arr[0] || meta::any_of_slice(arr + 1, size - 1));
}
constexpr auto any_of(std::initializer_list<bool> lst) noexcept -> bool {
  return meta::any_of_slice(lst.begin(), static_cast<i64>(lst.size()));
}

using std::is_convertible;

using std::is_arithmetic;
using std::is_enum;
using std::is_integral;
using std::is_signed;

using std::is_base_of;

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

template <bool B, typename T = void>
struct enable_if {
  using type = T;
};
template <typename T>
struct enable_if<false, T> {};

template <bool B, typename T, typename F>
using conditional_t = typename internal::conditional_<B>::template type<T, F>;

template <bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <typename U, typename V>
using discard_1st = V;

template <typename T>
using no_op = T;

// COMPAT: unused parameters are ignored, messes with SFINAE
template <typename...>
using void_t = void;

template <typename... Preds>
struct disjunction;

template <>
struct disjunction<> : false_type {};

template <typename First, typename... Preds>
struct disjunction<First, Preds...>
    : internal::conditional_<
          First::value>::template type<First, disjunction<Preds...>> {};

template <typename... Preds>
struct conjunction;

template <>
struct conjunction<> : true_type {};

template <typename First, typename... Preds>
struct conjunction<First, Preds...>
    : internal::conditional_<
          First::value>::template type<conjunction<Preds...>, First> {};

template <typename Pred>
struct negation : bool_constant<!Pred::value> {};

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
  own,
  ref,
  ref_mut,
  ref_mov,
};

namespace internal {
// clang-format off
template <category_e C>
struct apply_categ;
template <> struct apply_categ<category_e::own> { template <typename T> using type = T; };
template <> struct apply_categ<category_e::ref> { template <typename T> using type = T const&; };
template <> struct apply_categ<category_e::ref_mut> { template <typename T> using type = T&; };
template <> struct apply_categ<category_e::ref_mov> { template <typename T> using type = T&&; };
// clang-format on
} // namespace internal
template <category_e C, typename T>
using apply_category_t = typename internal::apply_categ<C>::template type<T>;

template <typename T>
struct value_category : std::integral_constant<category_e, category_e::own> {};

template <typename T>
struct value_category<T const&>
    : std::integral_constant<category_e, category_e::ref> {};
template <typename T>
struct value_category<T const&&>
    : std::integral_constant<category_e, category_e::ref> {};

template <typename T>
struct value_category<T&>
    : std::integral_constant<category_e, category_e::ref_mut> {};
template <typename T>
struct value_category<T&&>
    : std::integral_constant<category_e, category_e::ref_mov> {};

template <typename To, typename From>
using collapse_category_t = typename internal::apply_categ<
    value_category<From>::value>::template type<To>;

namespace internal {
struct wrapper_base {
  static auto test(void*) -> false_type;
};
template <typename T>
struct wrapper : wrapper_base {
  using wrapper_base::test;
  static auto test(wrapper<T>*) -> true_type;
};
} // namespace internal

template <typename T, typename U>
using same_as = decltype(
    internal::wrapper<T>::test(static_cast<internal::wrapper<U>*>(nullptr)));

template <typename T>
using void_ = same_as<T, void>;

namespace internal {
template <typename T>
struct is_const : false_type {};
template <typename T>
struct is_const<T const> : true_type {};
template <typename T>
struct is_pointer : false_type {};
template <typename T>
struct is_pointer<T*> : true_type {};
template <typename T>
struct is_lvalue_ref : false_type {};
template <typename T>
struct is_lvalue_ref<T&> : true_type {};
template <typename T>
struct is_rvalue_ref : false_type {};
template <typename T>
struct is_rvalue_ref<T&&> : true_type {};
} // namespace internal

template <typename T>
struct const_
    : __VEG_HAS_BUILTIN_OR(
          __is_const, bool_constant<__is_const(T)>, internal::is_const<T>) {};

// can't use __is_pointer because of <bits/cpp_type_traits.h> header
template <typename T>
struct pointer : internal::is_pointer<T> {};

template <typename T>
struct lvalue_reference : __VEG_HAS_BUILTIN_OR(
                              __is_lvalue_reference,
                              bool_constant<__is_lvalue_reference(T)>,
                              internal::is_lvalue_ref<T>) {};

template <typename T>
struct rvalue_reference : __VEG_HAS_BUILTIN_OR(
                              __is_rvalue_reference,
                              bool_constant<__is_rvalue_reference(T)>,
                              internal::is_rvalue_ref<T>) {};

template <typename T>
struct reference : bool_constant<__VEG_HAS_BUILTIN_OR(
                       __is_reference,
                       __is_reference(T),
                       internal::is_lvalue_ref<T>::value ||
                           internal::is_rvalue_ref<T>::value)> {};

template <typename T>
struct signed_integral
    : bool_constant<std::is_integral<T>::value && std::is_signed<T>::value> {};

template <typename T>
struct unsigned_integral
    : bool_constant<std::is_integral<T>::value && std::is_unsigned<T>::value> {
};

template <typename T>
struct integral
    : bool_constant<signed_integral<T>::value || unsigned_integral<T>::value> {
};

template <typename T>
struct floating_point
    : bool_constant<
          __VEG_SAME_AS(T, float) || __VEG_SAME_AS(T, double) ||
          __VEG_SAME_AS(T, long double)> {};

template <typename T>
struct arithmetic
    : bool_constant<integral<T>::value || floating_point<T>::value> {};

template <typename T>
struct scalar : disjunction<
                    std::is_arithmetic<T>,
                    std::is_pointer<T>,
                    std::is_scalar<T>> {};

template <typename T>
struct function : bool_constant<!const_<T>::value && !reference<T>::value> {};

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

template <typename T>
struct is_bounded_array : false_type {};
template <typename T, usize N>
struct is_bounded_array<T[N]> : true_type {};

template <typename T>
using bounded_array = is_bounded_array<T>;

template <typename T>
using remove_extent_t = typename std::remove_extent<T>::type;

template <template <typename...> class F, typename... Ts>
struct meta_apply {
  using type = F<Ts...>;
};
template <typename T>
struct type_identity {
  using type = T;
};

namespace internal {
struct none {};
} // namespace internal

#if __cplusplus >= 202002L

template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector {
  using found = bool_constant<(requires { typename Ftor<Args...>; })>;
  using type = typename conditional_t<
      found::value,
      meta_apply<Ftor, Args...>,
      type_identity<Default>>::type;
};

#else

namespace internal {

template <
    typename Enable,
    typename Default,
    template <typename...>
    class Ftor,
    typename... Args>
struct detector {
  using found = false_type;
  using type = Default;
};
template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector<void_t<Ftor<Args...>>, Default, Ftor, Args...> {
  using found = true_type;
  using type = Ftor<Args...>;
};

} // namespace internal

template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector : internal::detector<void, Default, Ftor, Args...> {};

#endif

template <template <class...> class Op, class... Args>
struct is_detected : detector<internal::none, Op, Args...>::found {};

template <template <class...> class Op, class... Args>
using detected_t = typename detector<internal::none, Op, Args...>::type;

namespace internal {
template <typename T, typename... Args>
using uniform_ctor = decltype(static_cast<void>(
    new (static_cast<void*>(nullptr)) T{__VEG_DECLVAL(Args)...}));

template <typename Enable, typename T, typename... Args>
struct uniform_init_constructible_impl
    : is_detected<internal::uniform_ctor, T, Args...> {};

} // namespace internal

template <typename T, typename... Args>
struct uniform_init_constructible
    : internal::uniform_init_constructible_impl<void, T, Args&&...> {};

template <typename... Ts>
struct dependent_true : true_type {};

namespace internal {

template <typename T, typename... Args>
using ctor_t =
    decltype(void(new (static_cast<void*>(nullptr)) T(__VEG_DECLVAL(Args)...)));
template <typename T, typename U>
using assign_t = decltype(void(__VEG_DECLVAL(T) = __VEG_DECLVAL(U)));

template <bool Reference, typename T, typename... Args>
struct is_nothrow_constructible_impl2 //
    : bool_constant<noexcept(T(__VEG_DECLVAL_NOEXCEPT(Args)...))> {};
template <typename T, typename... Args>
struct is_nothrow_constructible_impl2<false, T, Args...> //
    : bool_constant<noexcept(new (static_cast<void*>(nullptr))
                                 T(__VEG_DECLVAL_NOEXCEPT(Args)...))> {};

template <bool Constructible, typename T, typename... Args>
struct is_nothrow_constructible_impl //
    : is_nothrow_constructible_impl2<reference<T>::value, T, Args...> {};
template <typename T, typename... Args>
struct is_nothrow_constructible_impl<false, T, Args...> : false_type {};

template <bool Assignable, typename T, typename U>
struct is_nothrow_assignable_impl
    : bool_constant<noexcept(
          __VEG_DECLVAL_NOEXCEPT(T&&) = __VEG_DECLVAL_NOEXCEPT(U &&))> {};

template <typename T, typename U>
struct is_nothrow_assignable_impl<false, T, U> : false_type {};

} // namespace internal

template <typename T, typename U>
struct is_same : bool_constant<__VEG_SAME_AS(T, U)> {};

template <typename T>
struct trivially_copy_constructible
    : __VEG_HAS_BUILTIN_OR(
          __is_trivially_constructible,
          (bool_constant<__is_trivially_constructible(T, T const&)>),
          (std::is_trivially_copy_constructible<T>)) {};

template <typename T>
struct trivially_move_constructible
    : __VEG_HAS_BUILTIN_OR(
          __is_trivially_constructible,
          (bool_constant<__is_trivially_constructible(T, T&&)>),
          (std::is_trivially_move_constructible<T>)) {};

template <typename T>
struct trivially_copy_assignable
    : __VEG_HAS_BUILTIN_OR(
          __is_trivially_assignable,
          (bool_constant<__is_trivially_assignable(T&, T const&)>),
          (std::is_trivially_copy_assignable<T>)) {};

template <typename T>
struct trivially_move_assignable
    : __VEG_HAS_BUILTIN_OR(
          __is_trivially_assignable,
          (bool_constant<__is_trivially_assignable(T&, T&&)>),
          (std::is_trivially_move_assignable<T>)) {};

template <typename T>
struct trivially_copyable : __VEG_HAS_BUILTIN_OR(
                                __is_trivially_copyable,
                                (bool_constant<__is_trivially_copyable(T)>),
                                (std::is_trivially_copyable<T>)) {};

template <typename T>
struct trivially_destructible
    : __VEG_HAS_BUILTIN_OR(
          __has_trivial_destructor,
          (bool_constant<__has_trivial_destructor(T)>),
          (std::is_trivially_destructible<T>)) {};

template <typename T, typename U>
struct assignable : __VEG_HAS_BUILTIN_OR(
                        __is_assignable,
                        (bool_constant<__is_assignable(T&&, U&&)>),
                        (is_detected<internal::assign_t, T&&, &&>)) {};

template <typename T, typename U>
struct nothrow_assignable
    : __VEG_HAS_BUILTIN_OR(
          __is_nothrow_assignable,
          (bool_constant<__is_nothrow_assignable(T&&, U&&)>),
          (internal::is_nothrow_assignable_impl<
              assignable<T&&, U&&>::value,
              T&&,
              U&&>)) {};

template <typename T, typename... Ts>
struct constructible : __VEG_HAS_BUILTIN_OR(
                           __is_constructible,
                           (bool_constant<__is_constructible(T, Ts&&...)>),
                           (is_detected<internal::ctor_t, T, Ts&&...>)) {};

template <typename T, typename... Ts>
struct nothrow_constructible
    : __VEG_HAS_BUILTIN_OR(
          __is_nothrow_constructible,
          (bool_constant<__is_nothrow_constructible(T, Ts&&...)>),
          (internal::is_nothrow_constructible_impl<
              constructible<T, Ts&&...>::value,
              T,
              Ts&&...>)) {};

template <typename From, typename To>
struct convertible_to : __VEG_HAS_BUILTIN_OR(
                            __is_convertible,
                            (bool_constant<__is_convertible(From, To)>),
                            (std::is_convertible<From, To>)) {};

template <typename T>
struct move_constructible : constructible<T, T&&> {};
template <typename T>
struct nothrow_move_constructible : nothrow_constructible<T, T&&> {};

template <typename T>
struct move_assignable : assignable<T&, T&&> {};
template <typename T>
struct nothrow_move_assignable : nothrow_assignable<T&, T&&> {};

template <typename T>
struct copy_constructible : constructible<T, T const&> {};
template <typename T>
struct nothrow_copy_constructible : nothrow_constructible<T, T const&> {};

template <typename T>
struct copy_assignable : assignable<T&, T const&> {};
template <typename T>
struct nothrow_copy_assignable : nothrow_assignable<T&, T const&> {};

namespace internal {
struct callable {
  template <typename Fn, typename... Args>
  using apply_t = decltype(__VEG_DECLVAL(Fn)(__VEG_DECLVAL(Args)...));

  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE static constexpr auto apply(Fn&& fn, Args&&... args)
      __VEG_DEDUCE_RET(VEG_FWD(fn)(VEG_FWD(args)...));
};
struct mem_fn_callable {
  template <typename Fn, typename T, typename... Args>
  using apply_t =
      decltype((__VEG_DECLVAL(T).*__VEG_DECLVAL(Fn))(__VEG_DECLVAL(Args)...));

  template <typename Fn, typename T, typename... Args>
  HEDLEY_ALWAYS_INLINE static constexpr auto
  apply(Fn&& fn, T&& t, Args&&... args)
      __VEG_DEDUCE_RET((VEG_FWD(t).*VEG_FWD(fn))(VEG_FWD(args)...));
};
template <typename Fn, typename... Args>
struct is_mem_fn_callable_
    : is_detected<mem_fn_callable::apply_t, Fn, Args&&...>,
      mem_fn_callable {};

template <typename T>
struct tag_workaround {
  enum type { _ };
};
} // namespace internal
template <typename Fn, typename... Args>
struct callable : is_detected<internal::callable::apply_t, Fn, Args&&...>,
                  internal::callable {};

template <typename Fn, typename... Args>
struct invocable : disjunction<
                       callable<Fn, Args&&...>,
                       internal::is_mem_fn_callable_<Fn, Args&&...>> {};

namespace internal {
template <bool B, typename T>
struct defer_if : T {};
template <typename T>
struct defer_if<false, T> {};

template <typename Fn, typename... Args>
struct is_invocable_impl {
  using type =
      typename invocable<Fn&&, Args&&...>::template apply_t<Fn&&, Args&&...>;
};
template <typename Fn, typename... Args>
struct is_nothrow_invocable_impl
    : bool_constant<noexcept(invocable<Fn&&, Args&&...>::apply(
          __VEG_DECLVAL_NOEXCEPT(Fn), __VEG_DECLVAL_NOEXCEPT(Args)...))> {};

} // namespace internal

template <typename T>
struct identity {
  using type = T;
};
template <typename T>
using identity_t = typename identity<T>::type;

template <typename Fn, typename... Args>
struct call_result : internal::defer_if<
                         callable<Fn&&, Args&&...>::value,
                         detector<
                             internal::none,
                             internal::callable::apply_t,
                             Fn&&,
                             Args&&...>> {};
template <typename Fn, typename... Args>
struct invoke_result : internal::defer_if<
                           invocable<Fn&&, Args&&...>::value,
                           internal::is_invocable_impl<Fn&&, Args&&...>> {};
template <typename Fn, typename... Args>
using call_result_t = typename call_result<Fn&&, Args&&...>::type;
template <typename Fn, typename... Args>
using invoke_result_t = typename invoke_result<Fn&&, Args&&...>::type;

template <typename Fn, typename... Args>
using nothrow_invocable = conjunction<
    invocable<Fn&&, Args&&...>,
    internal::is_nothrow_invocable_impl<Fn&&, Args&&...>>;

namespace internal {
template <typename A, typename B>
using cmp_eq = decltype(void(
    static_cast<bool>(__VEG_DECLVAL(A const&) == __VEG_DECLVAL(B const&))));
template <typename A, typename B>
using cmp_lt = decltype(
    void(static_cast<bool>(__VEG_DECLVAL(A const&) < __VEG_DECLVAL(B const&))));

template <typename A, typename B>
struct equality_comparable_impl
    : bool_constant<
          (__VEG_SAME_AS(A, B) || (!std::is_enum<A>::value &&   //
                                   !std::is_enum<B>::value)) && //
          is_detected<cmp_eq, A, B>::value> {};

template <typename A, typename B>
struct less_than_comparable_impl
    : bool_constant<
          (__VEG_SAME_AS(A, B) || (!std::is_enum<A>::value &&   //
                                   !std::is_enum<B>::value)) && //
          is_detected<cmp_lt, A, B>::value> {};

} // namespace internal

template <typename A, typename B>
struct equality_comparable_with
    : meta::disjunction<
          meta::bool_constant<
              (arithmetic<remove_cvref_t<A>>::value &&
               arithmetic<remove_cvref_t<B>>::value) ||
              (__VEG_SAME_AS(remove_cvref_t<A>, remove_cvref_t<B>) &&
               scalar<remove_cvref_t<A>>::value)>,
          internal::
              equality_comparable_impl<remove_cvref_t<A>, remove_cvref_t<B>>> {
};

template <typename A, typename B>
struct partially_ordered_with
    : meta::disjunction<
          meta::bool_constant<
              (arithmetic<remove_cvref_t<A>>::value &&
               arithmetic<remove_cvref_t<B>>::value) ||
              (__VEG_SAME_AS(remove_cvref_t<A>, remove_cvref_t<B>) &&
               scalar<remove_cvref_t<A>>::value)>,
          internal::
              less_than_comparable_impl<remove_cvref_t<A>, remove_cvref_t<B>>> {
};

} // namespace meta

namespace fn {
struct invoke {
  VEG_TEMPLATE(
      (typename Fn, typename... Args),
      requires(__VEG_CONCEPT(meta::invocable<Fn&&, Args&&...>)),
      HEDLEY_ALWAYS_INLINE constexpr auto
      operator(),
      (fn, Fn&&),
      (... args, Args&&))
  const noexcept(__VEG_CONCEPT(meta::nothrow_invocable<Fn&&, Args&&...>))
      ->meta::invoke_result_t<Fn&&, Args&&...> {
    return meta::invocable<Fn&&, Args&&...>::apply(
        VEG_FWD(fn), VEG_FWD(args)...);
  }
};
} // namespace fn
__VEG_ODR_VAR(invoke, fn::invoke);

template <typename T>
struct tag_t {};

enum struct safety_e { safe, unsafe };

template <safety_e S>
struct safety_tag_t;

using safe_t = safety_tag_t<safety_e::safe>;
using unsafe_t = safety_tag_t<safety_e::unsafe>;

template <>
struct safety_tag_t<safety_e::safe>
    : std::integral_constant<safety_e, safety_e::safe> {};

namespace internal {
template <typename T>
struct make_unsafe;
} // namespace internal

template <>
struct safety_tag_t<safety_e::unsafe>
    : private std::integral_constant<safety_e, safety_e::unsafe> {

  using std::integral_constant<safety_e, safety_e::unsafe>::value;
  using typename std::integral_constant<safety_e, safety_e::unsafe>::value_type;
  using typename std::integral_constant<safety_e, safety_e::unsafe>::type;
  using std::integral_constant<safety_e, safety_e::unsafe>::operator value_type;

  constexpr
  operator safe_t() const noexcept // NOLINT(hicpp-explicit-conversions)
  {
    return {};
  }

private:
  friend struct internal::make_unsafe<void>;
  constexpr safety_tag_t() = default;
};

namespace internal {
template <typename T>
struct make_unsafe {
  static constexpr unsafe_t value = {};
};
template <typename T>
constexpr unsafe_t make_unsafe<T>::value; // NOLINT
} // namespace internal

namespace { // NOLINT(cert-dcl59-cpp)
constexpr auto const& unsafe /* NOLINT */ = internal::make_unsafe<void>::value;
} // namespace
__VEG_ODR_VAR(safe, safe_t);

namespace internal {
template <typename CP, typename... Args>
HEDLEY_ALWAYS_INLINE constexpr auto
adl_tag_invoke(tag_t<CP> tag, Args&&... args)
    __VEG_DEDUCE_RET(tag_invoke(tag, VEG_FWD(args)...));
} // namespace internal

namespace fn {
struct tag_invoke {
  template <typename CP, typename... Args>
  HEDLEY_ALWAYS_INLINE constexpr auto
  operator()(tag_t<CP> tag, Args&&... args) const
      __VEG_DEDUCE_RET(internal::adl_tag_invoke(tag, VEG_FWD(args)...));
};
} // namespace fn
__VEG_ODR_VAR(tag_invoke, fn::tag_invoke);
namespace meta {

template <typename CP, typename... Args>
using tag_invoke_result_t =
    decltype(veg::tag_invoke(tag_t<CP>{}, __VEG_DECLVAL(Args&&)...));

namespace internal {
template <typename Enable, typename CP, typename... Args>
struct tag_invocable_impl : false_type {};
template <typename CP, typename... Args>
struct tag_invocable_impl<void_t<tag_invoke_result_t<CP, Args...>>, CP, Args...>
    : true_type {};

} // namespace internal
template <typename CP, typename... Args>
struct tag_invocable : internal::tag_invocable_impl<void, CP, Args...> {};

} // namespace meta

namespace internal {
namespace swap_ {

struct adl_fn_swap {
  template <typename U, typename V>
  using type = decltype(swap(__VEG_DECLVAL(U &&), (__VEG_DECLVAL(V &&))));

  template <typename U, typename V>
  HEDLEY_ALWAYS_INLINE static __VEG_CPP14(constexpr) void apply(
      U&& u, V&& v) noexcept(noexcept(swap(VEG_FWD(u), VEG_FWD(v)))) {
    swap(VEG_FWD(u), (VEG_FWD(v)));
  }
};
struct mov_fn_swap {
  template <typename U, typename V>
  using type = void;

  template <typename U>
  HEDLEY_ALWAYS_INLINE static __VEG_CPP14(constexpr) void apply(U& u, U& v) noexcept(
      (__VEG_CONCEPT(meta::nothrow_move_constructible<U>) &&
       __VEG_CONCEPT(meta::nothrow_move_assignable<U>))) {
    auto tmp = static_cast<U&&>(u);
    u = static_cast<U&&>(v);
    v = static_cast<U&&>(tmp);
  }
};

template <typename U, typename V>
struct has_adl_swap : meta::is_detected<adl_fn_swap::type, U&&, V&&>,
                      adl_fn_swap {};

template <typename U, typename V>
struct has_mov_swap : meta::false_type {};
template <typename U>
struct has_mov_swap<U&, U&> : meta::bool_constant<
                                  meta::move_constructible<U>::value &&
                                  meta::move_assignable<U>::value>,
                              mov_fn_swap {};

template <typename U, typename V>
struct swap_impl : meta::disjunction<has_adl_swap<U, V>, has_mov_swap<U, V>> {};

template <typename U, typename V>
struct no_throw_swap
    : meta::bool_constant<noexcept(swap_impl<U, V>::apply(
          __VEG_DECLVAL_NOEXCEPT(U), __VEG_DECLVAL_NOEXCEPT(V)))> {};

} // namespace swap_
} // namespace internal

namespace meta {

template <typename U, typename V>
struct swappable : bool_constant<veg::internal::swap_::swap_impl<U, V>::value> {
};

template <typename U, typename V>
struct nothrow_swappable
    : conjunction<swappable<U, V>, veg::internal::swap_::no_throw_swap<U, V>> {
};

template <typename T>
struct value_sentinel_for : std::integral_constant<i64, 0> {
  static __VEG_CPP14(constexpr) void invalid(i64 /*unused*/) noexcept;
  static __VEG_CPP14(constexpr) auto id(T const& /*unused*/) noexcept -> i64;
};

} // namespace meta

namespace fn {
struct swap {
  VEG_TEMPLATE(
      (typename U, typename V),
      requires(__VEG_CONCEPT(meta::swappable<U, V>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void
      operator(),
      (u, U&&),
      (v, V&&))
  const noexcept(__VEG_CONCEPT(meta::nothrow_swappable<U, V>)) {
    internal::swap_::swap_impl<U, V>::apply(VEG_FWD(u), VEG_FWD(v));
  }
};
} // namespace fn
__VEG_ODR_VAR(swap, fn::swap);

template <typename Fn>
struct VEG_NODISCARD defer {
  Fn fn;
  constexpr defer /* NOLINT */ (Fn _fn) noexcept(
      __VEG_CONCEPT(meta::nothrow_move_constructible<Fn>))
      : fn(VEG_FWD(_fn)) {}
  defer(defer const&) = delete;
  defer(defer&&) noexcept = delete;
  auto operator=(defer const&) -> defer& = delete;
  auto operator=(defer&&) noexcept -> defer& = delete;
  __VEG_CPP20(constexpr)
  HEDLEY_ALWAYS_INLINE ~defer() noexcept(
      noexcept(__VEG_CONCEPT(meta::nothrow_invocable<Fn&&>))) {
    VEG_FWD(fn)();
  }
};
__VEG_CPP17(

    template <typename Fn> defer(Fn) -> defer<Fn>;

)

namespace make {
namespace fn {
struct defer {
  VEG_TEMPLATE(
      typename Fn,
      requires(
          __VEG_CONCEPT(meta::move_constructible<Fn>) &&
          __VEG_CONCEPT(meta::invocable<Fn&&>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP20(constexpr) auto
      operator(),
      (fn, Fn))
  const noexcept(__VEG_CONCEPT(meta::nothrow_move_constructible<Fn>))
      ->veg::defer<Fn> {
    return {VEG_FWD(fn)};
  }
};
} // namespace fn
__VEG_ODR_VAR(defer, fn::defer);
} // namespace make

[[noreturn]] void terminate() noexcept;

namespace meta {
// unsafe trait: trivial copy/move ctor/assignment, trivial dtor,
// default ctor (value initialization) must not have any observable side effects
// (usually sets to zero)
template <typename T>
struct mostly_trivial : __VEG_HAS_BUILTIN_OR(
                            __is_trivial,
                            (bool_constant<__is_trivial(T)>),
                            (std::is_trivial<T>)) {};

template <typename T>
struct trivially_relocatable : bool_constant<
                                   trivially_copyable<T>::value &&
                                   trivially_move_constructible<T>::value> {};

template <typename U, typename V>
struct trivially_swappable : false_type {};

template <typename T>
struct trivially_swappable<T&, T&>
    : bool_constant<(
          trivially_copyable<T>::value &&        //
          trivially_move_assignable<T>::value && //
          trivially_move_constructible<T>::value &&
          !::veg::internal::swap_::has_adl_swap<T&, T&>::value) //
                    > {};
} // namespace meta

namespace internal {
namespace get {

template <typename T>
void get() = delete;

struct array_get {
  template <typename I, typename T>
  using type = decltype(void(__VEG_DECLVAL(T).template get<I::value>()));
  template <usize I, typename T>
  HEDLEY_ALWAYS_INLINE static constexpr auto apply(T&& arr)
      __VEG_DEDUCE_RET(VEG_FWD(arr)[I]);
};

struct member_get {
  template <typename I, typename T>
  using type = decltype(void(__VEG_DECLVAL(T).template get<I::value>()));
  template <usize I, typename T>
  HEDLEY_ALWAYS_INLINE static constexpr auto apply(T&& arg)
      __VEG_DEDUCE_RET(VEG_FWD(arg).template get<I>());
};
struct adl_get {
  template <typename I, typename T>
  using type = decltype(void(get<I::value>(__VEG_DECLVAL(T))));

  template <usize I, typename T>
  HEDLEY_ALWAYS_INLINE static constexpr auto apply(T&& arg)
      __VEG_DEDUCE_RET(get<I>(VEG_FWD(arg)));
};

template <usize I, typename T>
struct has_array_get : meta::bounded_array<meta::remove_cvref_t<T>>,
                       array_get {};

template <usize I, typename T>
struct has_member_get
    : meta::
          is_detected<member_get::type, std::integral_constant<usize, I>, T&&>,
      member_get {};
template <usize I, typename T>
struct has_adl_get
    : meta::is_detected<adl_get::type, std::integral_constant<usize, I>, T&&>,
      adl_get {};

template <usize I, typename T>
struct get_impl : meta::disjunction<
                      has_array_get<I, T>,
                      has_member_get<I, T>,
                      has_adl_get<I, T>> {};
} // namespace get
} // namespace internal

namespace fn {
template <i64 I>
struct get {
  VEG_TEMPLATE(
      typename T,
      requires(internal::get::get_impl<static_cast<usize>(I), T>::value),
      HEDLEY_ALWAYS_INLINE constexpr auto
      operator(),
      (arg, T&&))
  const __VEG_DEDUCE_RET(
      internal::get::get_impl<static_cast<usize>(I), T>::template apply<I>(
          VEG_FWD(arg)));
};
} // namespace fn

template <typename... Ts>
struct tuple;
template <typename T>
struct option;

inline namespace tags {
struct elems_t {
  VEG_TEMPLATE(
      typename... Ts,
      requires_all(__VEG_CONCEPT(meta::constructible<meta::decay_t<Ts>, Ts&&>)),
      constexpr auto
      operator(),
      (... args, Ts&&))
  const noexcept(meta::all_of({__VEG_CONCEPT(
                     meta::nothrow_constructible<meta::decay_t<Ts>, Ts&&>)...}))
      ->veg::tuple<meta::decay_t<Ts>...> {
    return veg::tuple<meta::decay_t<Ts>...>{VEG_FWD(args)...};
  }
};
struct init_list_t {};
struct inplace_t {};
struct none_t {
  friend constexpr auto operator==(none_t /*lhs*/, none_t /*rhs*/) noexcept
      -> bool {
    return true;
  }
  friend constexpr auto operator!=(none_t /*lhs*/, none_t /*rhs*/) noexcept
      -> bool {
    return false;
  }

private:
  constexpr none_t() = default;
  constexpr explicit none_t(none_t* /*unused*/) noexcept {}
  template <typename T>
  friend struct meta::internal::static_const;
};
struct some_t {
  VEG_TEMPLATE(
      (typename T),
      requires(
          __VEG_CONCEPT(meta::constructible<meta::remove_cvref_t<T>, T&&>)),
      __VEG_CPP14(constexpr) auto
      operator(),
      (arg, T&&))
  const noexcept(
      __VEG_CONCEPT(meta::nothrow_constructible<meta::remove_cvref_t<T>, T&&>))
      ->option<meta::remove_cvref_t<T>> {
    return {*this, VEG_FWD(arg)};
  }

private:
  constexpr some_t() = default;
  constexpr explicit some_t(some_t* /*unused*/) noexcept {}
  template <typename T>
  friend struct meta::internal::static_const;
};

__VEG_IGNORE_CPP14_EXTENSION_WARNING(
    namespace { /* NOLINT */
                template <i64 I>
                constexpr auto const& get /* NOLINT */ =
                    ::veg::meta::internal::static_const<fn::get<I>>::value;

                template <typename T>
                constexpr auto const& tag =
                    ::veg::meta::internal::static_const<tag_t<T>>::value;
    } // namespace
)

__VEG_ODR_VAR(some, some_t);
__VEG_ODR_VAR(init_list, init_list_t);
__VEG_ODR_VAR(inplace, inplace_t);
__VEG_ODR_VAR(elems, elems_t);
__VEG_ODR_VAR(none, none_t);
} // namespace tags

} // namespace veg

#endif /* end of include guard __VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S */

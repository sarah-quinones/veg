#ifndef VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S
#define VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S

#include "veg/internal/std.hpp"
#include "veg/internal/.external/hedley.h"
#include "veg/internal/typedefs.hpp"
#include "veg/internal/.external/boostpp.h"

#define VEG_ODR_VAR(name, obj)                                                 \
  namespace { /* NOLINT */                                                     \
  constexpr auto const& name /* NOLINT */ =                                    \
      ::veg::meta::internal::static_const<obj>::value;                         \
  }                                                                            \
  static_assert(true, "")

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

#define VEG_DEDUCE_RET(...)                                                    \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) {                     \
    return __VA_ARGS__;                                                        \
  }                                                                            \
  static_assert(true, "")

#define VEG_FWD(x) static_cast<decltype(x)&&>(x)

// disallows moving const rvalues
#define VEG_MOV(x)                                                             \
  static_cast<typename ::std::remove_cv<                                       \
      typename ::std::remove_reference<decltype(x)>::type&&>::type>(x)

#define VEG_PP_LPAREN() (
#define VEG_PP_RPAREN() )

#undef VEG_PP_CAT
#define VEG_IMPL_CAT(A, ...) A##__VA_ARGS__
#define VEG_PP_CAT(A, ...) VEG_IMPL_CAT(A, __VA_ARGS__)

#undef VEG_PP_SEQ_TAIL
#define VEG_PP_SEQ_TAIL(seq) VEG_PP_CONSUME seq
#define VEG_PP_CONSUME(x)

#define VEG_IMPL_HEAD(arg, ...) arg
#define VEG_IMPL_TAIL(arg, ...) __VA_ARGS__
#define VEG_PP_HEAD(arg, ...) VEG_IMPL_HEAD(arg, __VA_ARGS__)
#define VEG_PP_TAIL(arg, ...) VEG_IMPL_TAIL(arg, __VA_ARGS__)

#define VEG_IMPL_REMOVE_PAREN1(...) VEG_IMPL_REMOVE_PAREN1 __VA_ARGS__
#define VEG_IMPL_REMOVE_PAREN2(...) VEG_PP_CAT(VEG_IMPL_, __VA_ARGS__)
#define VEG_IMPL_VEG_IMPL_REMOVE_PAREN1
#define VEG_PP_REMOVE_PAREN(...)                                               \
  VEG_IMPL_REMOVE_PAREN2(VEG_IMPL_REMOVE_PAREN1 __VA_ARGS__)
#define VEG_PP_APPEND(tup, elem) (elem, VEG_PP_REMOVE_PAREN(tup))

#ifdef VEG_NO_INSTANTIATE
#define VEG_INSTANTIATE(fn, ...) static_assert(true, "")
#define VEG_INSTANTIATE_CLASS(fn, ...) static_assert(true, "")
#else
#define VEG_INSTANTIATE(fn, ...)                                               \
  VEG_IMPL_INSTANTIATE(                                                        \
      fn,                                                                      \
      VEG_PP_CAT(                                                              \
          VEG_PP_CAT(_dummy_explicit_instantiation, __LINE__),                 \
          VEG_PP_CAT(_, __COUNTER__)),                                         \
      __VA_ARGS__)
#define VEG_IMPL_INSTANTIATE(fn, name, ...)                                    \
  template <typename... Ts>                                                    \
  struct name {                                                                \
    void apply(Ts&&... args) { fn(VEG_FWD(args)...); }                         \
  };                                                                           \
  template struct name<__VA_ARGS__>

#define VEG_INSTANTIATE_CLASS(class, ...) template struct class<__VA_ARGS__>
#endif

// macros
#if __cplusplus >= 202002L
#define VEG_CPP20(...) __VA_ARGS__
#else
#define VEG_CPP20(...)
#endif

#if __cplusplus >= 201402L
#define VEG_CPP14(...) __VA_ARGS__
#else
#define VEG_CPP14(...)
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

#if __cplusplus >= 201402L
#define VEG_CPP11_DECLTYPE_AUTO(...)                                           \
  noexcept(noexcept(__VA_ARGS__))->decltype(auto) { return __VA_ARGS__; }      \
  static_assert(true, "")
#else
#define VEG_CPP11_DECLTYPE_AUTO(...)                                           \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) {                     \
    return __VA_ARGS__;                                                        \
  }                                                                            \
  static_assert(true, "")
#endif

// deducing `this' please. :clauded:
#define VEG_CVREF_DUPLICATE_2(decl, handler, args)                             \
  decl const& VEG_CPP11_DECLTYPE_AUTO(                                         \
      handler(*this VEG_PP_REMOVE_PAREN(args)));                               \
  decl&& VEG_CPP11_DECLTYPE_AUTO(                                              \
      handler(VEG_MOV(*this) VEG_PP_REMOVE_PAREN(args)))

#define VEG_CVREF_DUPLICATE(decl, handler, args)                               \
  decl const& VEG_CPP11_DECLTYPE_AUTO(                                         \
      handler(*this VEG_PP_REMOVE_PAREN(args)));                               \
  decl& VEG_CPP11_DECLTYPE_AUTO(handler(*this VEG_PP_REMOVE_PAREN(args)));     \
  decl&& VEG_CPP11_DECLTYPE_AUTO(                                              \
      handler(VEG_MOV(*this) VEG_PP_REMOVE_PAREN(args)))

// const&, &&
#define VEG_CVREF_DUPLICATE_TEMPLATE_2(                                        \
    self, handler, args, tparams, requirement, attr_name, ...)                 \
  VEG_TEMPLATE(                                                                \
      VEG_PP_APPEND(tparams, typename Self = self const&),                     \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  const& VEG_CPP11_DECLTYPE_AUTO(handler(*this VEG_PP_REMOVE_PAREN(args)));    \
                                                                               \
  VEG_TEMPLATE(                                                                \
      VEG_PP_APPEND(tparams, typename Self = self &&),                         \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  &&VEG_CPP11_DECLTYPE_AUTO(handler(VEG_MOV(*this) VEG_PP_REMOVE_PAREN(args)))

// const&, &, &&
#define VEG_CVREF_DUPLICATE_TEMPLATE(                                          \
    self, handler, args, tparams, requirement, attr_name, ...)                 \
  VEG_TEMPLATE(                                                                \
      VEG_PP_APPEND(tparams, typename Self = self const&),                     \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  const& VEG_CPP11_DECLTYPE_AUTO(handler(*this VEG_PP_REMOVE_PAREN(args)));    \
                                                                               \
  VEG_TEMPLATE(                                                                \
      VEG_PP_APPEND(tparams, typename Self = self&),                           \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  &VEG_CPP11_DECLTYPE_AUTO(handler(*this VEG_PP_REMOVE_PAREN(args)));          \
                                                                               \
  VEG_TEMPLATE(                                                                \
      VEG_PP_APPEND(tparams, typename Self = self &&),                         \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  &&VEG_CPP11_DECLTYPE_AUTO(handler(*this VEG_PP_REMOVE_PAREN(args)))

#define VEG_IMPL_PARAM_EXPAND(r, _, param)                                     \
  VEG_PP_COMMA_IF(VEG_PP_IS_BEGIN_PARENS(param))                               \
  VEG_PP_IF(                                                                   \
      VEG_PP_IS_BEGIN_PARENS(param), VEG_PP_TAIL param VEG_PP_HEAD param, )

#define VEG_TEMPLATE(tparams, requirement, attr_name, ...)                     \
  VEG_IMPL_TEMPLATE(                                                           \
      attr_name, tparams, requirement, VEG_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define VEG_IMPL_TEMPLATE(attr_name, tparams, requirement, args)               \
  VEG_IMPL_TEMPLATE2(                                                          \
      attr_name,                                                               \
      tparams,                                                                 \
      VEG_PP_CAT(                                                              \
          VEG_IMPL_REQUIRES_HANDLER,                                           \
          VEG_PP_CAT(VEG_IMPL_PREFIX_, requirement) VEG_PP_RPAREN()),          \
      VEG_PP_SEQ_HEAD(args),                                                   \
      VEG_PP_SEQ_TAIL(args))

#define VEG_IMPL_PREFIX_requires _ONE VEG_PP_LPAREN()
#define VEG_IMPL_PREFIX_requires_all _ALL VEG_PP_LPAREN()
#define VEG_IMPL_REQUIRES_HANDLER_ONE(...) __VA_ARGS__

#if __cplusplus >= 202002L
#define VEG_IMPL_TEMPLATE2(attr_name, tparams, requirement, first_param, args) \
  template <VEG_PP_REMOVE_PAREN(tparams)>                                      \
  requires(requirement) attr_name(                                             \
      VEG_PP_TAIL first_param VEG_PP_HEAD first_param VEG_PP_SEQ_FOR_EACH(     \
          VEG_IMPL_PARAM_EXPAND, _, args))
#define VEG_IMPL_REQUIRES_HANDLER_ALL(...) (__VA_ARGS__) && ...
#else
#define VEG_IMPL_TEMPLATE2(attr_name, tparams, requirement, first_param, args) \
  template <VEG_PP_REMOVE_PAREN(tparams)>                                      \
  attr_name(::veg::meta::discard_1st<                                          \
            ::veg::meta::enable_if_t<(requirement)>,                           \
            VEG_PP_TAIL first_param> VEG_PP_HEAD first_param                   \
                VEG_PP_SEQ_FOR_EACH(VEG_IMPL_PARAM_EXPAND, _, args))

#define VEG_IMPL_REQUIRES_HANDLER_ALL(...) (__VA_ARGS__)
#endif

#define VEG_DECLTYPE_RET(...)                                                  \
  ->decltype(__VA_ARGS__) { return __VA_ARGS__; }                              \
  static_assert(true, "")

#define VEG_HAS_BUILTIN_OR_0(true, false) VEG_PP_REMOVE_PAREN(false)
#define VEG_HAS_BUILTIN_OR_1(true, false) VEG_PP_REMOVE_PAREN(true)
#define VEG_HAS_BUILTIN_OR(builtin, true, false)                               \
  VEG_PP_CAT(VEG_HAS_BUILTIN_OR_, VEG_HAS_BUILTIN(builtin))(true, false)

#define VEG_SAME_AS(T, U)                                                      \
  VEG_HAS_BUILTIN_OR(                                                          \
      __is_same,                                                               \
      __is_same(VEG_PP_REMOVE_PAREN(T), VEG_PP_REMOVE_PAREN(U)),               \
      (::veg::meta::same_as<VEG_PP_REMOVE_PAREN(T), VEG_PP_REMOVE_PAREN(U)>))

namespace veg {

// dev-only
template <typename... Ts>
struct incomplete_t;
template <typename... Types, typename... Args>
auto show_types(Args&&...) -> incomplete_t<Types..., Args...>;

namespace meta {

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
template <bool B, typename T = void>
struct enable_if {
  using type = T;
};
template <typename T>
struct enable_if<false, T> {};
} // namespace internal

template <bool B, typename T, typename F>
using conditional_t = typename internal::conditional_<B>::template type<T, F>;

template <bool B, typename T = void>
using enable_if_t = typename internal::enable_if<B, T>::type;

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
struct disjunction<> : std::false_type {};

template <typename First, typename... Preds>
struct disjunction<First, Preds...>
    : internal::conditional_<
          First::value>::template type<First, disjunction<Preds...>> {};

template <typename... Preds>
struct conjunction;

template <>
struct conjunction<> : std::true_type {};

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
  static auto test(void*) -> std::false_type;
};
template <typename T>
struct wrapper : wrapper_base {
  using wrapper_base::test;
  static auto test(wrapper<T>*) -> std::true_type;
};
} // namespace internal

template <typename T, typename U>
using same_as = decltype(
    internal::wrapper<T>::test(static_cast<internal::wrapper<U>*>(nullptr)));

template <typename T>
using void_ = same_as<T, void>;

namespace internal {
template <typename T>
struct is_const : std::false_type {};
template <typename T>
struct is_const<T const> : std::true_type {};
template <typename T>
struct is_pointer : std::false_type {};
template <typename T>
struct is_pointer<T*> : std::true_type {};
template <typename T>
struct is_lvalue_ref : std::false_type {};
template <typename T>
struct is_lvalue_ref<T&> : std::true_type {};
template <typename T>
struct is_rvalue_ref : std::false_type {};
template <typename T>
struct is_rvalue_ref<T&&> : std::true_type {};
} // namespace internal

template <typename T>
using const_ = VEG_HAS_BUILTIN_OR(
    __is_const, bool_constant<__is_const(T)>, internal::is_const<T>);

template <typename T>
constexpr auto foo() -> bool {
  static_assert(sizeof(T) < 0, "");
  return true;
}

// can't use __is_pointer because of <bits/cpp_type_traits.h> header
template <typename T>
using pointer = internal::is_pointer<T>;

template <typename T>
using lvalue_reference = VEG_HAS_BUILTIN_OR(
    __is_lvalue_reference,
    bool_constant<__is_lvalue_reference(T)>,
    internal::is_lvalue_ref<T>);

template <typename T>
using rvalue_reference = VEG_HAS_BUILTIN_OR(
    __is_rvalue_reference,
    bool_constant<__is_rvalue_reference(T)>,
    internal::is_rvalue_ref<T>);

template <typename T>
using reference = bool_constant<VEG_HAS_BUILTIN_OR(
    __is_reference,
    __is_reference(T),
    internal::is_lvalue_ref<T>::value || internal::is_rvalue_ref<T>::value)>;

template <typename T>
using signed_integral =
    bool_constant<std::is_integral<T>::value && std::is_signed<T>::value>;

template <typename T>
using unsigned_integral =
    bool_constant<std::is_integral<T>::value && std::is_unsigned<T>::value>;

template <typename T>
using integral =
    bool_constant<signed_integral<T>::value || unsigned_integral<T>::value>;

template <typename T>
using floating_point = bool_constant<
    VEG_SAME_AS(T, float) || VEG_SAME_AS(T, double) ||
    VEG_SAME_AS(T, long double)>;

template <typename T>
using arithmetic =
    bool_constant<integral<T>::value || floating_point<T>::value>;

template <typename T>
struct scalar : disjunction<
                    std::is_arithmetic<T>,
                    std::is_pointer<T>,
                    std::is_scalar<T>> {};

template <typename T>
using function = bool_constant<!const_<T>::value && !reference<T>::value>;

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
struct bounded_array : std::false_type {};

template <typename T, usize N>
struct bounded_array<T[N]> : std::true_type {};

template <typename T>
using remove_extent_t = typename std::remove_extent<T>::type;

namespace internal {
struct none {};

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

template <
    typename Enable,
    typename Default,
    template <usize, typename...>
    class Ftor,
    usize I,
    typename... Args>
struct detector_i {
  using found = std::false_type;
  using type = Default;
};
template <
    typename Default,
    template <usize, typename...>
    class Ftor,
    usize I,
    typename... Args>
struct detector_i<void_t<Ftor<I, Args...>>, Default, Ftor, I, Args...> {
  using found = std::true_type;
  using type = Ftor<I, Args...>;
};
} // namespace internal

template <typename Default, template <typename...> class Ftor, typename... Args>
struct detector : internal::detector<void, Default, Ftor, Args...> {};

template <template <class...> class Op, class... Args>
using is_detected = typename detector<internal::none, Op, Args...>::found;

template <template <usize, class...> class Op, usize I, class... Args>
using is_detected_i =
    typename internal::detector_i<void, internal::none, Op, I, Args...>::found;

template <template <class...> class Op, class... Args>
using detected_t = typename detector<internal::none, Op, Args...>::type;

namespace internal {
template <typename T, typename... Args>
using uniform_ctor = decltype(static_cast<void>(T{VEG_DECLVAL(Args)...}));

template <typename Enable, typename T, typename... Args>
struct uniform_init_constructible_impl
    : is_detected<internal::uniform_ctor, T, Args...> {};

} // namespace internal

template <typename T, typename... Args>
struct uniform_init_constructible
    : internal::uniform_init_constructible_impl<void, T, Args&&...> {};

template <typename... Ts>
struct dependent_true : std::true_type {};

namespace internal {

template <typename T, typename... Args>
using ctor_t =
    decltype(void(new (static_cast<void*>(nullptr)) T(VEG_DECLVAL(Args)...)));
template <typename T, typename U>
using assign_t = decltype(void(VEG_DECLVAL(T) = VEG_DECLVAL(U)));

template <bool Reference, typename T, typename... Args>
struct is_nothrow_constructible_impl2 //
    : bool_constant<noexcept(T(VEG_DECLVAL_NOEXCEPT(Args)...))> {};
template <typename T, typename... Args>
struct is_nothrow_constructible_impl2<false, T, Args...> //
    : bool_constant<noexcept(new (static_cast<void*>(nullptr))
                                 T(VEG_DECLVAL_NOEXCEPT(Args)...))> {};

template <bool Constructible, typename T, typename... Args>
struct is_nothrow_constructible_impl //
    : is_nothrow_constructible_impl2<reference<T>::value, T, Args...> {};
template <typename T, typename... Args>
struct is_nothrow_constructible_impl<false, T, Args...> : std::false_type {};

template <bool Assignable, typename T, typename U>
struct is_nothrow_assignable_impl
    : bool_constant<noexcept(
          VEG_DECLVAL_NOEXCEPT(T&&) = VEG_DECLVAL_NOEXCEPT(U &&))> {};

template <typename T, typename U>
struct is_nothrow_assignable_impl<false, T, U> : std::false_type {};

} // namespace internal

template <typename T, typename U>
struct is_same : bool_constant<VEG_SAME_AS(T, U)> {};

template <typename T>
using trivially_copy_constructible = VEG_HAS_BUILTIN_OR(
    __is_trivially_constructible,
    (bool_constant<__is_trivially_constructible(T, T const&)>),
    (std::is_trivially_copy_constructible<T>));

template <typename T>
using trivially_move_constructible = VEG_HAS_BUILTIN_OR(
    __is_trivially_constructible,
    (bool_constant<__is_trivially_constructible(T, T&&)>),
    (std::is_trivially_move_constructible<T>));

template <typename T>
using trivially_copy_assignable = VEG_HAS_BUILTIN_OR(
    __is_trivially_assignable,
    (bool_constant<__is_trivially_assignable(T&, T const&)>),
    (std::is_trivially_copy_assignable<T>));

template <typename T>
using trivially_move_assignable = VEG_HAS_BUILTIN_OR(
    __is_trivially_assignable,
    (bool_constant<__is_trivially_assignable(T&, T&&)>),
    (std::is_trivially_move_assignable<T>));

template <typename T>
using trivially_copyable = VEG_HAS_BUILTIN_OR(
    __is_trivially_copyable,
    (bool_constant<__is_trivially_copyable(T)>),
    (std::is_trivially_copyable<T>));

template <typename T>
struct mostly_trivial : VEG_HAS_BUILTIN_OR(
                            __is_trivial,
                            (bool_constant<__is_trivial(T)>),
                            (std::is_trivial<T>)) {};

template <typename T>
using trivially_destructible = VEG_HAS_BUILTIN_OR(
    __has_trivial_destructor,
    (bool_constant<__has_trivial_destructor(T)>),
    (std::is_trivially_destructible<T>));

template <typename T, typename U>
using assignable = VEG_HAS_BUILTIN_OR(
    __is_assignable,
    (bool_constant<__is_assignable(T&&, U&&)>),
    (is_detected<internal::assign_t, T&&, &&>));

template <typename T, typename U>
using nothrow_assignable = VEG_HAS_BUILTIN_OR(
    __is_nothrow_assignable,
    (bool_constant<__is_nothrow_assignable(T&&, U&&)>),
    (internal::
         is_nothrow_assignable_impl<assignable<T&&, U&&>::value, T&&, U&&>));

template <typename T, typename... Ts>
using constructible = VEG_HAS_BUILTIN_OR(
    __is_constructible,
    (bool_constant<__is_constructible(T, Ts&&...)>),
    (is_detected<internal::ctor_t, T, Ts&&...>));

template <typename T, typename... Ts>
using nothrow_constructible = VEG_HAS_BUILTIN_OR(
    __is_nothrow_constructible,
    (bool_constant<__is_nothrow_constructible(T, Ts&&...)>),
    (internal::is_nothrow_constructible_impl<
        constructible<T, Ts&&...>::value,
        T,
        Ts&&...>));

template <typename From, typename To>
using convertible_to = VEG_HAS_BUILTIN_OR(
    __is_convertible,
    (bool_constant<__is_convertible(From, To)>),
    (std::is_convertible<From, To>));

template <typename T>
using nothrow_move_constructible = nothrow_constructible<T, T&&>;
template <typename T>
using nothrow_move_assignable = nothrow_assignable<T&, T&&>;

template <typename T>
using move_constructible = constructible<T, T&&>;

template <typename T>
struct is_nothrow_move_constructible : nothrow_constructible<T, T&&> {};

template <typename T>
struct is_copy_constructible : constructible<T, T const&> {};

namespace internal {
struct callable {
  template <typename Fn, typename... Args>
  using apply_t = decltype(VEG_DECLVAL(Fn)(VEG_DECLVAL(Args)...));

  template <typename Fn, typename... Args>
  static constexpr auto apply(Fn&& fn, Args&&... args)
      VEG_DEDUCE_RET(VEG_FWD(fn)(VEG_FWD(args)...));
};
struct mem_fn_callable {
  template <typename Fn, typename T, typename... Args>
  using apply_t =
      decltype((VEG_DECLVAL(T).*VEG_DECLVAL(Fn))(VEG_DECLVAL(Args)...));

  template <typename Fn, typename T, typename... Args>
  static constexpr auto apply(Fn&& fn, T&& t, Args&&... args)
      VEG_DEDUCE_RET((VEG_FWD(t).*VEG_FWD(fn))(VEG_FWD(args)...));
};
template <typename Fn, typename... Args>
struct is_mem_fn_callable_
    : is_detected<mem_fn_callable::apply_t, Fn, Args&&...>,
      mem_fn_callable {};

template <typename T>
struct tag_workaround {
  enum type { _ };
};
#define VEG_FRIEND_WORKAROUND                                                  \
  typename ::veg::meta::internal::tag_workaround<tuple>::type =                \
      ::veg::meta::internal::tag_workaround<tuple>::type::_
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
          VEG_DECLVAL_NOEXCEPT(Fn), VEG_DECLVAL_NOEXCEPT(Args)...))> {};

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
template <typename A, typename B, typename Enable = void>
struct is_equality_comparable_impl : std::false_type {};
template <typename A, typename B>
struct is_equality_comparable_impl<
    A,
    B,
    decltype(void(static_cast<bool>(
        VEG_DECLVAL(remove_reference_t<A> const&) ==
        VEG_DECLVAL(remove_reference_t<B> const&))))>

    : meta::disjunction<
          meta::is_same<A, B>,
          meta::negation<meta::disjunction<std::is_enum<A>, std::is_enum<B>>>> {
};

template <typename A, typename B, typename Enable = void>
struct is_less_than_comparable_impl : std::false_type {};
template <typename A, typename B>
struct is_less_than_comparable_impl<
    A,
    B,
    decltype(void(static_cast<bool>(
        VEG_DECLVAL(remove_reference_t<A> const&) <
        VEG_DECLVAL(remove_reference_t<B> const&))))> :

    meta::bool_constant<
        (std::is_enum<A>::value && std::is_same<A, B>::value) ||

        (!std::is_enum<A>::value && //
         !std::is_enum<B>::value)> {};

} // namespace internal

template <typename A, typename B>
struct is_equality_comparable_with
    : meta::disjunction<
          meta::bool_constant<
              (arithmetic<A>::value && arithmetic<B>::value) ||
              (VEG_SAME_AS(A, B) && scalar<A>::value)>,
          internal::is_equality_comparable_impl<A, B>> {};

template <typename A, typename B>
struct is_partially_ordered_with
    : meta::disjunction<
          meta::bool_constant<
              (arithmetic<A>::value && arithmetic<B>::value) ||
              (VEG_SAME_AS(A, B) && scalar<A>::value)>,
          internal::is_less_than_comparable_impl<A, B>> {};

} // namespace meta

namespace fn {
struct invoke_fn {
  VEG_TEMPLATE(
      (typename Fn, typename... Args),
      requires(meta::invocable<Fn&&, Args&&...>::value),
      constexpr auto
      operator(),
      (fn, Fn&&),
      (... args, Args&&))
  const noexcept(meta::nothrow_invocable<Fn&&, Args&&...>::value)
      ->meta::invoke_result_t<Fn&&, Args&&...> {
    return meta::invocable<Fn&&, Args&&...>::apply(
        VEG_FWD(fn), VEG_FWD(args)...);
  }
};
} // namespace fn
VEG_ODR_VAR(invoke, fn::invoke_fn);

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
}
VEG_ODR_VAR(safe, safe_t);
VEG_ODR_VAR(init_list, init_list_t);

namespace fn {
struct tag_invoke_fn {
  template <typename CP, typename... Args>
  constexpr auto operator()(tag_t<CP> tag, Args&&... args) const
      VEG_DEDUCE_RET(tag_invoke(tag, VEG_FWD(args)...));
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

} // namespace meta

namespace internal {
namespace swap_ {

struct mem_fn_swap {
  template <typename U, typename V>
  using type = decltype(VEG_DECLVAL(U &&).swap(VEG_DECLVAL(V &&)));

  template <typename U, typename V>
  static VEG_CPP14(constexpr) void apply(U&& u, V&& v) noexcept(
      noexcept(VEG_FWD(u).swap(VEG_FWD(v)))) {
    VEG_FWD(u).swap(VEG_FWD(v));
  }
};

struct adl_fn_swap {
  template <typename U, typename V>
  using type = decltype(swap(VEG_DECLVAL(U &&), (VEG_DECLVAL(V &&))));

  template <typename U, typename V>
  static VEG_CPP14(constexpr) void apply(U&& u, V&& v) noexcept(
      noexcept(swap(VEG_FWD(u), VEG_FWD(v)))) {
    swap(VEG_FWD(u), (VEG_FWD(v)));
  }
};
struct mov_fn_swap {
  template <typename U, typename V>
  using type = void;

  template <typename U>
  static VEG_CPP14(constexpr) void apply(U& u, U& v) noexcept(
      std::is_nothrow_move_constructible<U>::value&&
          std::is_nothrow_move_assignable<U>::value) {
    auto tmp = static_cast<U&&>(u);
    u = static_cast<U&&>(v);
    v = static_cast<U&&>(tmp);
  }
};

template <typename U, typename V>
struct has_mem_fn_swap : meta::is_detected<mem_fn_swap::type, U&&, V&&>,
                         mem_fn_swap {};
template <typename U, typename V>
struct has_adl_swap : meta::is_detected<adl_fn_swap::type, U&&, V&&>,
                      adl_fn_swap {};

template <typename U, typename V>
struct has_mov_swap : std::false_type {};
template <typename U>
struct has_mov_swap<U&, U&> : meta::bool_constant<
                                  std::is_move_constructible<U>::value &&
                                  std::is_move_assignable<U>::value>,
                              mov_fn_swap {};

template <typename U, typename V>
struct swap_impl : meta::disjunction<
                       has_mem_fn_swap<U, V>,
                       has_adl_swap<U, V>,
                       has_mov_swap<U, V>> {};

template <typename U, typename V>
struct no_throw_swap : meta::bool_constant<noexcept(swap_impl<U, V>::apply(
                           VEG_DECLVAL_NOEXCEPT(U), VEG_DECLVAL_NOEXCEPT(V)))> {
};

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
  static VEG_CPP14(constexpr) void invalid(i64 /*unused*/) noexcept {}
  static VEG_CPP14(constexpr) auto id(T const& /*unused*/) noexcept -> i64 {
    return -1;
  }
};

} // namespace meta

namespace fn {
struct swap_fn {
  VEG_TEMPLATE(
      (typename U, typename V),
      requires(meta::swappable<U, V>::value),
      VEG_CPP14(constexpr) void
      operator(),
      (u, U&&),
      (v, V&&))
  const noexcept(meta::nothrow_swappable<U, V>::value) {
    internal::swap_::swap_impl<U, V>::apply(VEG_FWD(u), VEG_FWD(v));
  }
};
} // namespace fn
VEG_ODR_VAR(swap, fn::swap_fn);

template <typename Fn>
struct VEG_NODISCARD finally {
  Fn fn;
  finally(finally const&) = delete;
  finally(finally&&) noexcept(meta::is_nothrow_move_constructible<Fn>::value) =
      default;
  auto operator=(finally const&) -> finally& = default;
  auto operator=(finally&&) noexcept(meta::nothrow_move_assignable<Fn>::value)
      -> finally& = default;
  VEG_CPP20(constexpr)
  ~finally() noexcept(noexcept(meta::nothrow_invocable<Fn&&>::value)) {
    invoke(fn);
  }
};
VEG_CPP17(

    template <typename Fn> finally(Fn) -> finally<Fn>;

)

namespace make {
namespace fn {
struct finally_fn {
  VEG_TEMPLATE(
      typename Fn,
      requires((
          meta::move_constructible<Fn>::value && meta::invocable<Fn&&>::value)),
      VEG_CPP20(constexpr) auto
      operator(),
      (fn, Fn))
  const noexcept(meta::is_nothrow_move_constructible<Fn>::value)->finally<Fn> {
    return {VEG_FWD(fn)};
  }
};
} // namespace fn
VEG_ODR_VAR(finally, fn::finally_fn);
} // namespace make

[[noreturn]] void terminate() noexcept;
} // namespace veg

#endif /* end of include guard VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S */

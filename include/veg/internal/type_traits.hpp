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

// disallows moving const rvalues
#define VEG_MOV(x)                                                             \
  static_cast<typename ::std::remove_cv<                                       \
      typename ::std::remove_reference<decltype(x)>::type&&>::type>(x)

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

// deducing `this' please. :clauded:
#define VEG_CVREF_DUPLICATE(decl, handler, args)                               \
  decl const& noexcept(noexcept(handler(*this VEG_PP_REMOVE_PAREN args)))      \
      ->decltype(auto) {                                                       \
    return handler(*this VEG_PP_REMOVE_PAREN args);                            \
  }                                                                            \
  decl& noexcept(noexcept(handler(*this VEG_PP_REMOVE_PAREN args)))            \
      ->decltype(auto) {                                                       \
    return handler(*this VEG_PP_REMOVE_PAREN args);                            \
  }                                                                            \
  decl&& noexcept(noexcept(handler(VEG_MOV(*this) VEG_PP_REMOVE_PAREN args)))  \
      ->decltype(auto) {                                                       \
    return handler(VEG_MOV(*this) VEG_PP_REMOVE_PAREN args);                   \
  }                                                                            \
  static_assert(true, "")

#define VEG_IMPL_PARAM_EXPAND(r, _, param)                                     \
  VEG_PP_COMMA_IF(VEG_PP_IS_BEGIN_PARENS(param))                               \
  VEG_PP_IF(                                                                   \
      VEG_PP_IS_BEGIN_PARENS(param),                                           \
      VEG_PP_REMOVE_PAREN VEG_PP_TUPLE_POP_FRONT(param)                        \
          VEG_PP_TUPLE_ELEM(0, param), )

#define VEG_TEMPLATE(tparams, requirement, attr_name, ...)                     \
  VEG_IMPL_TEMPLATE(                                                           \
      attr_name, tparams, requirement, VEG_PP_TUPLE_TO_SEQ((__VA_ARGS__)))

#define VEG_PP_LPAREN() (
#define VEG_PP_RPAREN() )

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
#define VEG_PP_REMOVE_PAREN(...) __VA_ARGS__
#define VEG_PP_HEAD(arg, ...) arg
#define VEG_PP_TAIL(arg, ...) __VA_ARGS__

#if __cplusplus >= 202002L
#define VEG_IMPL_TEMPLATE2(attr_name, tparams, requirement, first_param, args) \
  template <VEG_PP_REMOVE_PAREN tparams>                                       \
  requires(requirement) attr_name(                                             \
      VEG_PP_TAIL first_param VEG_PP_HEAD first_param VEG_PP_SEQ_FOR_EACH(     \
          VEG_IMPL_PARAM_EXPAND, _, args))
#define VEG_IMPL_REQUIRES_HANDLER_ALL(...) (__VA_ARGS__) && ...
#else
#define VEG_IMPL_TEMPLATE2(attr_name, tparams, requirement, first_param, args) \
  template <VEG_PP_REMOVE_PAREN tparams>                                       \
  attr_name(::veg::meta::discard_1st<                                          \
            ::veg::meta::enable_if_t<(requirement)>,                           \
            VEG_PP_TAIL first_param> VEG_PP_HEAD first_param                   \
                VEG_PP_SEQ_FOR_EACH(VEG_IMPL_PARAM_EXPAND, _, args))

#define VEG_IMPL_REQUIRES_HANDLER_ALL(...) (__VA_ARGS__)
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

template <bool B>
struct bool_constant : std::integral_constant<bool, B> {};

namespace internal {
template <typename T>
struct static_const {
  static constexpr T value{};
};
template <typename T>
constexpr T static_const<T>::value; // NOLINT(readability-redundant-declaration)
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
template <bool B>
struct enable_if {
  using type = void;
};
template <>
struct enable_if<false> {};
} // namespace internal

template <bool B, typename T, typename F>
using conditional_t = typename internal::conditional_<B>::template type<T, F>;

template <bool B>
using enable_if_t = typename internal::enable_if<B>::type;

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

template <typename T>
struct is_bounded_array : std::false_type {};

template <typename T, usize N>
struct is_bounded_array<T[N]> : std::true_type {};

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
struct is_uniform_init_constructible_impl
    : is_detected<internal::uniform_ctor, T, Args...> {};

} // namespace internal

template <typename T, typename... Args>
struct is_uniform_init_constructible
    : internal::is_uniform_init_constructible_impl<void, T, Args&&...> {};

template <typename... Ts>
struct dependent_true : std::true_type {};

template <typename T>
struct is_scalar : disjunction<
                       std::is_arithmetic<T>,
                       std::is_pointer<T>,
                       std::is_scalar<T>> {};

namespace internal {
template <typename, typename T, typename... Args>
struct is_constructible_impl : std::false_type {};
template <typename T, typename... Args>
struct is_constructible_impl<
    decltype(void(T(VEG_DECLVAL(Args)...))),
    T,
    Args...> : std::true_type {};

template <typename T, typename = void>
struct is_default_constructible_impl : std::false_type {};
template <typename T>
struct is_default_constructible_impl<T, decltype(void(T()))> : std::true_type {
};
template <typename T, typename... Args>
struct is_nothrow_constructible_impl1
    : bool_constant<noexcept(noexcept(new (static_cast<void*>(nullptr))
                                          T(VEG_DECLVAL(Args)...)))> {};
} // namespace internal

#define VEG_HAS_BUILTIN_OR_0(true, false) VEG_PP_REMOVE_PAREN false
#define VEG_HAS_BUILTIN_OR_1(true, false) VEG_PP_REMOVE_PAREN true
#define VEG_HAS_BUILTIN_OR(builtin, true, false)                               \
  VEG_PP_CAT(VEG_HAS_BUILTIN_OR_, VEG_HAS_BUILTIN(builtin))(true, false)

template <typename T, typename... Ts>
struct is_constructible
    : VEG_HAS_BUILTIN_OR(
          __is_constructible,
          (bool_constant<__is_constructible(T, Ts&&...)>),
          (internal::is_constructible_impl<void, T, Ts&&...>)) {};

template <typename T>
struct is_default_constructible : VEG_HAS_BUILTIN_OR(
                                      __is_constructible,
                                      (bool_constant<__is_constructible(T)>),
                                      (is_constructible<T>)) {};

template <typename T, typename... Args>
struct is_nothrow_constructible
    : conjunction<
          is_constructible<T, Args&&...>,
          internal::is_nothrow_constructible_impl1<T, Args&&...>> {};

template <typename T>
struct is_move_constructible : is_constructible<T, T&&> {};

template <typename T>
struct is_copy_constructible : is_constructible<T, T const&> {};

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
  using std::integral_constant<safety_e, safety_e::unsafe>::operator();

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

template <typename T>
struct side_effect_free_default_constuctible
    : std::is_trivially_default_constructible<T>::value {};
} // namespace meta

namespace internal {
namespace swap_ {

struct mem_fn_swap {
  template <typename U, typename V>
  using type = decltype(VEG_DECLVAL(U &&).swap(VEG_DECLVAL(V &&)));

  template <typename U, typename V>
  static constexpr void
  apply(U&& u, V&& v) noexcept(noexcept(VEG_FWD(u).swap(VEG_FWD(v)))) {
    VEG_FWD(u).swap(VEG_FWD(v));
  }
};

struct adl_fn_swap {
  template <typename U, typename V>
  using type = decltype(swap(VEG_DECLVAL(U &&), (VEG_DECLVAL(V &&))));

  template <typename U, typename V>
  static constexpr void
  apply(U&& u, V&& v) noexcept(noexcept(swap(VEG_FWD(u), VEG_FWD(v)))) {
    swap(VEG_FWD(u), (VEG_FWD(v)));
  }
};
struct mov_fn_swap {
  template <typename U, typename V>
  using type = void;

  template <typename U>
  static constexpr void
  apply(U& u, U& v) noexcept(std::is_nothrow_move_constructible<U>::value&&
                                 std::is_nothrow_move_assignable<U>::value) {
    auto tmp = VEG_MOV(u);
    u = VEG_MOV(v);
    v = VEG_MOV(tmp);
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
struct is_swappable
    : bool_constant<veg::internal::swap_::swap_impl<U, V>::value> {};

template <typename U, typename V>
struct is_nothrow_swappable : conjunction<
                                  is_swappable<U, V>,
                                  veg::internal::swap_::no_throw_swap<U, V>> {};

} // namespace meta

namespace fn {
struct swap_fn {
  VEG_TEMPLATE(
      (typename U, typename V),
      requires(meta::is_swappable<U, V>::value),
      constexpr void
      operator(),
      (u, U&&),
      (v, V&&))
  const noexcept(meta::is_nothrow_swappable<U, V>::value) {
    internal::swap_::swap_impl<U, V>::apply(VEG_FWD(u), VEG_FWD(v));
  }
};
} // namespace fn
} // namespace veg

#endif /* end of include guard VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S */

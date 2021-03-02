#ifndef __VEG_CONCEPT_MACROS_HPP_3AJ8JOUWS
#define __VEG_CONCEPT_MACROS_HPP_3AJ8JOUWS

#include "veg/internal/.external/boostpp.ext.h"

#undef __VEG_PP_CAT
#define __VEG_IMPL_CAT(A, ...) A##__VA_ARGS__
#define __VEG_PP_CAT(A, ...) __VEG_IMPL_CAT(A, __VA_ARGS__)

#define __VEG_IMPL_CAT2(A, ...) A##__VA_ARGS__
#define __VEG_PP_CAT2(A, ...) __VEG_IMPL_CAT2(A, __VA_ARGS__)

#undef __VEG_PP_SEQ_TAIL
#define __VEG_PP_SEQ_TAIL(seq) __VEG_PP_CONSUME seq
#define __VEG_PP_CONSUME(x)

#define __VEG_IMPL_HEAD(arg, ...) arg
#define __VEG_IMPL_TAIL(arg, ...) __VA_ARGS__
#define __VEG_PP_HEAD(arg, ...) __VEG_IMPL_HEAD(arg, __VA_ARGS__)
#define __VEG_PP_TAIL(arg, ...) __VEG_IMPL_TAIL(arg, __VA_ARGS__)

#define __VEG_IMPL_REMOVE_PAREN1(...) __VEG_IMPL_REMOVE_PAREN1 __VA_ARGS__
#define __VEG_IMPL_REMOVE_PAREN2(...) __VEG_PP_CAT(__VEG_IMPL_, __VA_ARGS__)
#define __VEG_IMPL___VEG_IMPL_REMOVE_PAREN1
#define __VEG_PP_REMOVE_PAREN(...)                                             \
  __VEG_IMPL_REMOVE_PAREN2(__VEG_IMPL_REMOVE_PAREN1 __VA_ARGS__)
#define __VEG_PP_APPEND(tup, elem) (elem, __VEG_PP_REMOVE_PAREN(tup))

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

#if __cplusplus >= 202002L

#define __VEG_DEF_CONCEPT(name, ...) concept name = __VA_ARGS__
#define __VEG_DEF_CONCEPT_EVAL(name, ...) concept name = __VA_ARGS__
#define __VEG_CONCEPT(...) (__VA_ARGS__)
#define __VEG_TO_CONCEPT(...) (__VA_ARGS__::value)
#define __VEG_ALL_OF(...) ((__VA_ARGS__) && ...)
#define __VEG_IMPL_REQUIRES_HANDLER_ONE(...) (__VA_ARGS__)
#define __VEG_IMPL_REQUIRES_HANDLER_ALL(...) ((__VA_ARGS__) && ...)

#define __VEG_IMPL_AND(r, _, param) &&param
#define __VEG_IMPL_OR(r, _, param) || param

#define __VEG_CONJUNCTION(...)                                                 \
  (true __VEG_PP_SEQ_FOR_EACH(                                                 \
      __VEG_IMPL_AND, _, __VEG_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

#define __VEG_DISJUNCTION(...)                                                 \
  (false __VEG_PP_SEQ_FOR_EACH(                                                \
      __VEG_IMPL_OR, _, __VEG_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

#else

#define __VEG_DEF_CONCEPT(name, ...)                                           \
  struct name : ::veg::meta::bool_constant<__VA_ARGS__> {}
#define __VEG_DEF_CONCEPT_EVAL(name, ...)                                      \
  using name = ::veg::meta::bool_constant<(__VA_ARGS__)>
#define __VEG_CONCEPT(...) __VA_ARGS__::value
#define __VEG_TO_CONCEPT(...) __VA_ARGS__
#define __VEG_IMPL_REQUIRES_HANDLER_ONE(...) (__VA_ARGS__)
#define __VEG_IMPL_REQUIRES_HANDLER_ALL(...) (__VA_ARGS__)

#define __VEG_IMPL_REM_PAREN(r, _, param) , __VEG_PP_REMOVE_PAREN(param)
#define __VEG_CONJUNCTION(...)                                                 \
  ::veg::meta::conjunction<::veg::meta::true_type __VEG_PP_SEQ_FOR_EACH(       \
      __VEG_IMPL_REM_PAREN, _, __VEG_PP_VARIADIC_TO_SEQ(__VA_ARGS__))>
#define __VEG_DISJUNCTION(...)                                                 \
  ::veg::meta::disjunction<::veg::meta::false_type __VEG_PP_SEQ_FOR_EACH(      \
      __VEG_IMPL_REM_PAREN, _, __VEG_PP_VARIADIC_TO_SEQ(__VA_ARGS__))>

#if __cplusplus >= 201703L
#define __VEG_ALL_OF(...) ((__VA_ARGS__) && ...)
#else
#define __VEG_ALL_OF(...) (::veg::meta::all_of({(__VA_ARGS__)...}))
#endif

#endif

// deducing `this' please. :clauded:
#define __VEG_CVREF_DUPLICATE_2(decl, handler, args)                           \
  decl const& __VEG_DEDUCE_RET(handler(*this __VEG_PP_REMOVE_PAREN(args)));    \
  decl&& __VEG_DEDUCE_RET(handler(VEG_MOV(*this) __VEG_PP_REMOVE_PAREN(args)))

#define __VEG_CVREF_DUPLICATE(decl, handler, args)                             \
  decl const& __VEG_DEDUCE_RET(handler(*this __VEG_PP_REMOVE_PAREN(args)));    \
  decl& __VEG_DEDUCE_RET(handler(*this __VEG_PP_REMOVE_PAREN(args)));          \
  decl&& __VEG_DEDUCE_RET(handler(VEG_MOV(*this) __VEG_PP_REMOVE_PAREN(args)))

// const&, &&
#define __VEG_CVREF_DUPLICATE_TEMPLATE_2(                                      \
    self, handler, args, tparams, requirement, attr_name, ...)                 \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self const&),                   \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  const& __VEG_DEDUCE_RET(handler(*this __VEG_PP_REMOVE_PAREN(args)));         \
                                                                               \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self &&),                       \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  &&__VEG_DEDUCE_RET(handler(VEG_MOV(*this) __VEG_PP_REMOVE_PAREN(args)))

// const&, &, &&
#define __VEG_CVREF_DUPLICATE_TEMPLATE(                                        \
    self, handler, args, tparams, requirement, attr_name, ...)                 \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self const&),                   \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  const& __VEG_DEDUCE_RET(handler(*this __VEG_PP_REMOVE_PAREN(args)));         \
                                                                               \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self&),                         \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  &__VEG_DEDUCE_RET(handler(*this __VEG_PP_REMOVE_PAREN(args)));               \
                                                                               \
  VEG_TEMPLATE(                                                                \
      __VEG_PP_APPEND(tparams, typename Self = self &&),                       \
      requirement,                                                             \
      attr_name,                                                               \
      __VA_ARGS__)                                                             \
  &&__VEG_DEDUCE_RET(handler(*this __VEG_PP_REMOVE_PAREN(args)))

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
      ((__VEG_CONCEPT(                                                         \
          ::veg::meta::                                                        \
              same_as<__VEG_PP_REMOVE_PAREN(T), __VEG_PP_REMOVE_PAREN(U)>))))

#endif /* end of include guard __VEG_CONCEPT_MACROS_HPP_3AJ8JOUWS */

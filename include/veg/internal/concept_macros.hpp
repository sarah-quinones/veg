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

#if __cplusplus >= 202002L

#define __VEG_CONCEPT(...) __VA_ARGS__
#define __VEG_ALL_OF(...) ((__VA_ARGS__) && ...)
#define __VEG_IMPL_REQUIRES_HANDLER_ONE(...) (__VA_ARGS__)
#define __VEG_IMPL_REQUIRES_HANDLER_ALL(...) ((__VA_ARGS__) && ...)

#else

#define __VEG_CONCEPT(...) __VA_ARGS__::value
#define __VEG_IMPL_REQUIRES_HANDLER_ONE(...) (__VA_ARGS__)
#define __VEG_IMPL_REQUIRES_HANDLER_ALL(...) (__VA_ARGS__)

#if __cplusplus >= 201703L
#define __VEG_ALL_OF(...) ((__VA_ARGS__) && ...)
#else
#define __VEG_ALL_OF(...) (::veg::meta::all_of({(__VA_ARGS__)...}))
#endif

#endif

#endif /* end of include guard __VEG_CONCEPT_MACROS_HPP_3AJ8JOUWS */

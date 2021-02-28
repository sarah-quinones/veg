#ifndef __VEG_ASSERT_HPP_VQDAJ2IBS
#define __VEG_ASSERT_HPP_VQDAJ2IBS

#include "veg/internal/typedefs.hpp"
#include "veg/internal/type_traits.hpp"
#include "veg/internal/cmp.hpp"
#include "veg/internal/byte_string_ref.hpp"
#include "veg/internal/simple_string.hpp"

#if HEDLEY_HAS_WARNING("-Woverloaded-shift-op-parentheses")
#define __VEG_IGNORE_SHIFT_PAREN_WARNING(Macro, ...)                           \
  HEDLEY_DIAGNOSTIC_PUSH _Pragma(                                              \
      "clang diagnostic ignored \"-Woverloaded-shift-op-parentheses\" ")       \
      Macro(__VA_ARGS__) HEDLEY_DIAGNOSTIC_POP
#else
#define __VEG_IGNORE_SHIFT_PAREN_WARNING(Macro, ...) Macro(__VA_ARGS__)
#endif

#if defined(__GNUC__)
#define __VEG_THIS_FUNCTION __PRETTY_FUNCTION__
#elif defined(HEDLEY_MSVC_VERSION)
#define __VEG_THIS_FUNCTION __FUNCSIG__
#else
#define __VEG_THIS_FUNCTION __func__
#endif

namespace veg {
namespace assert {

namespace fn {
struct to_string;
} // namespace fn

namespace internal {

struct tag_invoke_print_ {
  template <typename T>
  using type = decltype(
      (void(static_cast<char const*>(
           __VEG_DECLVAL(meta::tag_invoke_result_t<fn::to_string, T const&>)
               .data())),
       (void(static_cast<i64>(
           __VEG_DECLVAL(meta::tag_invoke_result_t<fn::to_string, T const&>)
               .size())))));

  template <typename T>
  HEDLEY_ALWAYS_INLINE static auto apply(T const* arg)
      __VEG_DEDUCE_RET(tag_invoke(tag<fn::to_string>, *arg));
};
template <typename T>
struct tag_invoke_printable
    : meta::is_detected<tag_invoke_print_::type, T const&>,
      tag_invoke_print_ {};

template <typename T>
auto to_string_primitive(T arg) -> assert::internal::string;

template <typename T>
using cast_up = meta::conditional_t<
    meta::pointer<T>::value, //
    void const volatile*,    //
    meta::conditional_t<
        meta::floating_point<T>::value,
        long double,
        meta::conditional_t<
            meta::signed_integral<T>::value,
            signed long long,
            unsigned long long>>>;

struct default_impl_print_ {
  template <typename T>
  HEDLEY_ALWAYS_INLINE static auto apply(T const* arg)
      -> assert::internal::string {
    auto const upscaled = static_cast<cast_up<T>>(*arg);
    return internal::to_string_primitive(upscaled);
  }
};

template <typename T>
struct is_ptr_non_fn : meta::bool_constant<(
                           meta::pointer<T>::value &&
                           !meta::function<meta::remove_pointer_t<T>>::value)> {
};

template <typename T>
struct default_impl_printable
    : meta::disjunction<meta::is_arithmetic<T>, is_ptr_non_fn<T>>,
      default_impl_print_ {};

struct bool_print_ {
  HEDLEY_ALWAYS_INLINE static constexpr auto apply(bool const* arg) noexcept
      -> assert::internal::char_string_ref {
    return (*arg)
               ? assert::internal::char_string_ref{"true", sizeof("true") - 1}
               : assert::internal::char_string_ref{
                     "false", sizeof("false") - 1};
  }
};
template <typename T>
struct bool_printable : meta::is_same<T, bool>, bool_print_ {};

struct boolish_print_ {
  static constexpr auto impl(bool arg) noexcept
      -> assert::internal::char_string_ref {
    return arg ? assert::internal::
                     char_string_ref{"truthy", sizeof("truthy") - 1}
               : assert::internal::char_string_ref{
                     "falsy", sizeof("falsy") - 1};
  }
  template <typename T>
  HEDLEY_ALWAYS_INLINE static constexpr auto
  apply(T const* arg) noexcept(noexcept(static_cast<bool>(*arg)))
      -> assert::internal::char_string_ref {
    return boolish_print_::impl(static_cast<bool>(*arg));
  }
};
template <typename T>
struct boolish_printable : meta::constructible<bool, T>, boolish_print_ {};

struct unprintable_print_ {
  template <typename T>
  HEDLEY_ALWAYS_INLINE static constexpr auto
  apply(void volatile const* /*arg*/) noexcept
      -> assert::internal::char_string_ref {
    return {"{?}", sizeof("{?}") - 1};
  }
};

template <typename T>
struct unprintable : std::true_type, unprintable_print_ {};

template <typename T>
struct print_impl : meta::disjunction<
                        bool_printable<T>,
                        default_impl_printable<T>,
                        tag_invoke_printable<T>,
                        boolish_printable<T>,
                        unprintable<T>> {};

} // namespace internal

namespace fn {

struct to_string {
  template <typename T>
  HEDLEY_ALWAYS_INLINE auto operator()(T const& arg) const
      __VEG_DEDUCE_RET(internal::print_impl<T>::apply(&arg));
};

} // namespace fn
__VEG_ODR_VAR(to_string, fn::to_string);

namespace internal {

void incr_counter() noexcept;
void decr_counter() noexcept;

template <typename T>
auto to_string(T const& arg) -> string {
  string buf{};

  incr_counter();
  try {
    auto res = print_impl<T>::apply(&arg);

    char_string_ref view(res);
    buf.copy(view.data(), view.size());
  } catch (...) {
    decr_counter();
    throw;
  }
  decr_counter();
  return buf;
}

[[noreturn]] void
on_assert_fail(long line, char_string_ref file, char_string_ref func);
void on_expect_fail(long line, char_string_ref file, char_string_ref func);
void set_assert_params1(char_string_ref op, string lhs, string rhs);
void set_assert_params2(
    char_string_ref expression, char_string_ref msg) noexcept;

template <typename T>
struct lhs_all_of_t {
  T const& lhs;

  template <typename U>
  HEDLEY_ALWAYS_INLINE void
  on_assertion_fail(U const& rhs, char_string_ref op_str) const {
    internal::set_assert_params1(
        op_str, internal::to_string(lhs), internal::to_string(rhs));
  }

#define __VEG_CMP(Op, Fn)                                                      \
  template <typename U>                                                        \
  HEDLEY_ALWAYS_INLINE constexpr auto operator Op(U const& rhs) const->bool {  \
    return process_op(                                                         \
        Fn(lhs, rhs), rhs, {" " #Op " ", sizeof(" " #Op " ") - 1});            \
  }                                                                            \
  static_assert(true, "")

#define __VEG_DISABLE(Op)                                                      \
  template <typename U>                                                        \
  void operator Op(U const& rhs) = delete

  __VEG_DISABLE(<<);
  __VEG_DISABLE(>>);
  __VEG_DISABLE(&);
  __VEG_DISABLE(|);
  __VEG_DISABLE(+);
  __VEG_DISABLE(-);
  __VEG_DISABLE(*);
  __VEG_DISABLE(/);
  __VEG_DISABLE(%);
  __VEG_DISABLE(^);

  // if a VEG_ASSERT fails during the evaluation of the comparison result, the
  // behavior is undefined (potential recursion)
  __VEG_CMP(==, ::veg::cmp_equal);
  __VEG_CMP(!=, ::veg::cmp_not_equal);
  __VEG_CMP(<, ::veg::cmp_less);
  __VEG_CMP(>, ::veg::cmp_greater);
  __VEG_CMP(<=, ::veg::cmp_less_equal);
  __VEG_CMP(>=, ::veg::cmp_greater_equal);

#undef __VEG_CMP
#undef __VEG_DISABLE

  HEDLEY_ALWAYS_INLINE explicit constexpr
  operator bool() const { // NOLINT(hicpp-explicit-conversions)
    return process_bool(static_cast<bool>(lhs));
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto process_bool(bool res) const
      -> bool {
    return (res ? void(0)
                : internal::set_assert_params1(
                      empty_str, internal::to_string(lhs), {})),
           res;
  }
  template <typename U>
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
  process_op(bool res, U const& rhs, char_string_ref op) const -> bool {
    return (res ? void(0) : on_assertion_fail(rhs, op)), res;
  }
};

struct decomposer {
  template <typename T>
  constexpr auto operator<<(T const& lhs) const noexcept -> lhs_all_of_t<T> {
    return {lhs};
  }
};
struct cleanup { // NOLINT(cppcoreguidelines-special-member-functions)
  ~cleanup() noexcept;
};
} // namespace internal

#define __VEG_IMPL_ASSERT_IMPL(Callback, If_Fail, ...)                         \
  static_cast<void>(                                                           \
      (::veg::assert::internal::decomposer{} << __VA_ARGS__)                   \
          ? (void)(0)                                                          \
          : (::veg::assert::internal::cleanup{},                               \
             ::veg::assert::internal::set_assert_params2(                      \
                 ::veg::assert::internal::char_string_ref{                     \
                     static_cast<char const*>(#__VA_ARGS__),                   \
                     sizeof(#__VA_ARGS__) - 1},                                \
                 If_Fail),                                                     \
             ::veg::assert::internal::Callback(                                \
                 __LINE__,                                                     \
                 ::veg::assert::internal::char_string_ref{                     \
                     static_cast<char const*>(__FILE__),                       \
                     sizeof(__FILE__) - 1},                                    \
                 ::veg::assert::internal::char_string_ref{                     \
                     static_cast<char const*>(__VEG_THIS_FUNCTION),            \
                     sizeof(__VEG_THIS_FUNCTION) - 1})))

#define VEG_ASSERT_ELSE(...)                                                   \
  __VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
      __VEG_IMPL_ASSERT_IMPL, on_assert_fail, __VA_ARGS__)
#define VEG_EXPECT_ELSE(...)                                                   \
  __VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
      __VEG_IMPL_ASSERT_IMPL, on_expect_fail, __VA_ARGS__)

#define VEG_ASSERT(...)                                                        \
  __VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
      __VEG_IMPL_ASSERT_IMPL,                                                  \
      on_assert_fail,                                                          \
      (::veg::assert::internal::empty_str),                                    \
      __VA_ARGS__)

#define VEG_EXPECT(...)                                                        \
  __VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
      __VEG_IMPL_ASSERT_IMPL,                                                  \
      on_expect_fail,                                                          \
      (::veg::assert::internal::empty_str),                                    \
      __VA_ARGS__)

#define __VEG_IMPL_ALL_OF_1(Ftor, Decomposer, Callback, ...)                   \
  __VEG_IMPL_ALL_OF_2(                                                         \
      Ftor, Decomposer, Callback, __VEG_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define __VEG_IMPL_ASSERT_FTOR_EMPTY(_, Decomposer, Elem)                      \
  __VEG_IMPL_ASSERT_FTOR(                                                      \
      _,                                                                       \
      Decomposer,                                                              \
      (::veg::assert::internal::empty_str, __VEG_PP_REMOVE_PARENS(Elem)))

#define __VEG_IMPL_ASSERT_FTOR(_, Decomposer, Elem)                            \
  (::veg::assert::internal::Decomposer{} << __VEG_PP_TAIL Elem)                \
      ? true                                                                   \
      : ((void)(::veg::assert::internal::cleanup{}),                           \
         ::veg::assert::internal::set_assert_params2(                          \
             {static_cast<char const*>(                                        \
                  __VEG_PP_STRINGIZE(__VEG_PP_TAIL Elem)),                     \
              sizeof(__VEG_PP_STRINGIZE(__VEG_PP_TAIL Elem)) - 1},             \
             __VEG_PP_HEAD Elem),                                              \
         false),

#define __VEG_IMPL_ALL_OF_2(Ftor, Decomposer, Callback, Seq)                   \
  (::veg::meta::all_of({__VEG_PP_SEQ_FOR_EACH(Ftor, Decomposer, Seq)})         \
       ? (void)(0)                                                             \
       : ::veg::assert::internal::Callback(                                    \
             __LINE__,                                                         \
             {static_cast<char const*>(__FILE__), sizeof(__FILE__) - 1},       \
             {static_cast<char const*>(__VEG_THIS_FUNCTION),                   \
              sizeof(__VEG_THIS_FUNCTION) - 1}))

#define VEG_ASSERT_ALL_OF(...)                                                 \
  __VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
      __VEG_IMPL_ALL_OF_1,                                                     \
      __VEG_IMPL_ASSERT_FTOR_EMPTY,                                            \
      decomposer,                                                              \
      on_assert_fail,                                                          \
      __VA_ARGS__)

#define VEG_EXPECT_ALL_OF(...)                                                 \
  __VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
      __VEG_IMPL_ALL_OF_1,                                                     \
      __VEG_IMPL_ASSERT_FTOR_EMPTY,                                            \
      decomposer,                                                              \
      on_expect_fail,                                                          \
      __VA_ARGS__)

#define VEG_ASSERT_ALL_OF_ELSE(...)                                            \
  __VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
      __VEG_IMPL_ALL_OF_1,                                                     \
      __VEG_IMPL_ASSERT_FTOR,                                                  \
      decomposer,                                                              \
      on_assert_fail,                                                          \
      __VA_ARGS__)

#define VEG_EXPECT_ALL_OF_ELSE(...)                                            \
  __VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
      __VEG_IMPL_ALL_OF_1,                                                     \
      __VEG_IMPL_ASSERT_FTOR,                                                  \
      decomposer,                                                              \
      on_expect_fail,                                                          \
      __VA_ARGS__)

} // namespace assert
} // namespace veg

#endif /* end of include guard __VEG_ASSERT_HPP_VQDAJ2IBS */

#ifdef VEG_DEBUG_ASSERT
#undef VEG_DEBUG_ASSERT
#endif
#ifdef VEG_DEBUG_EXPECT
#undef VEG_DEBUG_EXPECT
#endif
#ifdef VEG_DEBUG_ASSERT_ELSE
#undef VEG_DEBUG_ASSERT_ELSE
#endif
#ifdef VEG_DEBUG_EXPECT_ELSE
#undef VEG_DEBUG_EXPECT_ELSE
#endif
#ifdef VEG_DEBUG_ASSERT_ALL_OF
#undef VEG_DEBUG_ASSERT_ALL_OF
#endif
#ifdef VEG_DEBUG_EXPECT_ALL_OF
#undef VEG_DEBUG_EXPECT_ALL_OF
#endif
#ifdef VEG_DEBUG_ASSERT_ALL_OF_ELSE
#undef VEG_DEBUG_ASSERT_ALL_OF_ELSE
#endif
#ifdef VEG_DEBUG_EXPECT_ALL_OF_ELSE
#undef VEG_DEBUG_EXPECT_ALL_OF_ELSE
#endif

#ifdef NDEBUG
#define VEG_DEBUG_ASSERT(...) ((void)(0))
#define VEG_DEBUG_EXPECT(...) ((void)(0))
#define VEG_DEBUG_ASSERT_ELSE(Message, ...) ((void)(0))
#define VEG_DEBUG_EXPECT_ELSE(Message, ...) ((void)(0))
#define VEG_DEBUG_ASSERT_ALL_OF(...) ((void)(0))
#define VEG_DEBUG_EXPECT_ALL_OF(...) ((void)(0))
#define VEG_DEBUG_ASSERT_ALL_OF_ELSE(...) ((void)(0))
#define VEG_DEBUG_EXPECT_ALL_OF_ELSE(...) ((void)(0))
#else
#define VEG_DEBUG_ASSERT(...) VEG_ASSERT(__VA_ARGS__)
#define VEG_DEBUG_EXPECT(...) VEG_EXPECT(__VA_ARGS__)
#define VEG_DEBUG_ASSERT_ELSE(Message, ...)                                    \
  VEG_ASSERT_ELSE(Message, __VA_ARGS__)
#define VEG_DEBUG_EXPECT_ELSE(Message, ...)                                    \
  VEG_EXPECT_ELSE(Message, __VA_ARGS__)
#define VEG_DEBUG_ASSERT_ALL_OF(...) VEG_ASSERT_ALL_OF(__VA_ARGS__)
#define VEG_DEBUG_EXPECT_ALL_OF(...) VEG_EXPECT_ALL_OF(__VA_ARGS__)
#define VEG_DEBUG_ASSERT_ALL_OF_ELSE(...) VEG_ASSERT_ALL_OF_ELSE(__VA_ARGS__)
#define VEG_DEBUG_EXPECT_ALL_OF_ELSE(...) VEG_EXPECT_ALL_OF_ELSE(__VA_ARGS__)
#endif

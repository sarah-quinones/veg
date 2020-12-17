#ifndef VEG_ASSERT_HPP_VQDAJ2IBS
#define VEG_ASSERT_HPP_VQDAJ2IBS

#include "veg/internal/typedefs.hpp"
#include "veg/internal/type_traits.hpp"
#include "veg/cmp.hpp"
#include "veg/internal/.external/boostpp.h"
#include "veg/internal/.external/hedley.h"

#if HEDLEY_HAS_WARNING("-Woverloaded-shift-op-parentheses")
#define VEG_IGNORE_SHIFT_PAREN_WARNING(Macro, ...)                             \
  HEDLEY_DIAGNOSTIC_PUSH _Pragma(                                              \
      "clang diagnostic ignored \"-Woverloaded-shift-op-parentheses\" ")       \
      Macro(__VA_ARGS__) HEDLEY_DIAGNOSTIC_POP
#else
#define VEG_IGNORE_SHIFT_PAREN_WARNING(Macro, ...) Macro(__VA_ARGS__)
#endif

#if defined(__GNUC__)
#define VEG_THIS_FUNCTION __PRETTY_FUNCTION__
#elif defined(HEDLEY_MSVC_VERSION)
#define VEG_THIS_FUNCTION __FUNCSIG__
#else
#define VEG_THIS_FUNCTION __func__
#endif

namespace veg {
[[noreturn]] void terminate() noexcept;
namespace assert {

namespace fn {
struct to_string_fn;
} // namespace fn

namespace internal {

struct string {
  struct layout {
    char* ptr = nullptr;
    i64 len = 0;
    i64 cap = 0;
  } self;

  ~string();
  string() = default;
  constexpr string(string&& other) noexcept : self{other.self} {
    other.self = {nullptr, 0, 0};
  }
  string(string const&) = delete;
  auto operator=(string const&) -> string& = delete;
  auto operator=(string&&) -> string& = delete;

  void resize(i64 new_len);
  void copy(char const* data, i64 len);
  void insert(i64 pos, char const* data, i64 len);

  VEG_NODISCARD constexpr auto data() const noexcept -> char* {
    return self.ptr;
  };
  VEG_NODISCARD constexpr auto size() const noexcept -> i64 {
    return self.len;
  };
};

struct char_string_ref {
  char const* data_;
  i64 len_;

  VEG_NODISCARD constexpr auto data() const noexcept -> char const* {
    return data_;
  }
  VEG_NODISCARD constexpr auto size() const noexcept -> i64 { return len_; }
  VEG_NODISCARD auto starts_with(char_string_ref other) const noexcept -> bool;
  VEG_NODISCARD auto operator==(char_string_ref other) const noexcept -> bool;
};

template <typename T>
constexpr auto char_string_ref_(T const& arg) noexcept -> VEG_REQUIRES_RET(
    (meta::is_constructible_v<char const*, decltype(arg.data())> &&
     meta::is_constructible_v<i64, decltype(arg.size())>),
    char_string_ref) {
  return {static_cast<char const*>(arg.data()), static_cast<i64>(arg.size())};
}

auto char_string_ref_(char const* str) noexcept -> char_string_ref;

template <typename T, typename Enable = void>
struct tag_invoke_printable : std::false_type {};

template <typename T>
struct tag_invoke_printable<
    T,
    meta::enable_if_t<meta::tag_invocable_v<fn::to_string_fn, T const&>>>
    : meta::bool_constant<
          meta::is_constructible_v<
              char const*,
              decltype(
                  VEG_DECLVAL(
                      meta::tag_invoke_result_t<fn::to_string_fn, T const&>)
                      .data())> &&
          meta::is_constructible_v<
              i64,
              decltype(
                  VEG_DECLVAL(
                      meta::tag_invoke_result_t<fn::to_string_fn, T const&>)
                      .size())> && //
          !meta::is_pointer_v<T> &&
          !meta::is_arithmetic_v<T>> {};

template <typename T>
struct default_impl_printable
    : meta::bool_constant<
          (meta::is_pointer_v<T> &&
           !meta::is_function_v<meta::remove_pointer_t<T>>) ||
          meta::is_arithmetic_v<T>> {};

template <typename T>
struct truthy_printable : std::is_constructible<bool, T> {};

template <typename T>
auto to_string_primitive(T arg) -> assert::internal::string;

template <typename T>
using cast_up = meta::conditional_t<
    meta::is_pointer_v<T>, //
    void const volatile*,  //
    meta::conditional_t<
        meta::is_floating_point_v<T>,
        long double,
        meta::conditional_t<
            meta::is_signed_v<T>,
            signed long long,
            unsigned long long>>>;

template <i64 which /* = not found */>
struct to_string_impl : std::false_type {
  template <typename T>
  static constexpr auto apply(T const& /*arg*/) noexcept
      -> assert::internal::char_string_ref {
    return {"{?}", sizeof("{?}") - 1};
  }
};
template <>
struct to_string_impl<0> : std::true_type {
  static constexpr auto apply(bool arg) -> assert::internal::char_string_ref {
    if (arg) {
      return {"true", sizeof("true") - 1};
    }
    return {"false", sizeof("false") - 1};
  }
};
template <>
struct to_string_impl<1> : std::true_type {
  template <typename T>
  static auto apply(T const& arg) {
    auto const upscaled = static_cast<cast_up<T>>(arg);
    return internal::to_string_primitive(upscaled);
  }
};
template <>
struct to_string_impl<2> : std::true_type {
  template <typename T>
  static auto apply(T const& arg) {
    return tag_invoke(tag<fn::to_string_fn>, arg);
  }
};
template <>
struct to_string_impl<3> : std::true_type {
  template <typename T>
  static constexpr auto
  apply(T const& arg) noexcept(noexcept(static_cast<bool>(arg)))
      -> assert::internal::char_string_ref {
    bool arg_ = static_cast<bool>(arg);
    if (arg_) {
      return {"truthy", sizeof("truthy") - 1};
    }
    return {"falsy", sizeof("falsy") - 1};
  }
};

} // namespace internal

namespace fn {

struct to_string_fn {
  template <
      typename T,
      typename Impl = internal::to_string_impl<meta::first_true<
          meta::is_same<T, bool>,
          internal::default_impl_printable<T>,
          internal::tag_invoke_printable<T>,
          internal::truthy_printable<T>>::found>>
  auto operator()(T const& arg) const noexcept(noexcept(Impl::apply(arg)))
      -> VEG_REQUIRES_RET((Impl::value), decltype(Impl::apply(arg))) {
    return Impl::apply(arg);
  }
};

} // namespace fn
VEG_ODR_VAR(to_string, fn::to_string_fn);

namespace internal {

void incr_counter() noexcept;
void decr_counter() noexcept;

template <typename T>
auto to_string(T const& arg) -> string {
  string buf{};

  incr_counter();
  try {
    auto res = internal::to_string_impl<meta::first_true<
        meta::is_same<T, bool>,
        internal::default_impl_printable<T>,
        internal::tag_invoke_printable<T>,
        internal::truthy_printable<T>>::value>::apply(arg);

    char_string_ref view = char_string_ref_(res);
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
  void on_assertion_fail(U const& rhs, char_string_ref op_str) const {
    internal::set_assert_params1(
        op_str, internal::to_string(lhs), internal::to_string(rhs));
  }

#define VEG_CMP(Op, Fn)                                                        \
  template <typename U>                                                        \
  HEDLEY_ALWAYS_INLINE constexpr auto operator Op(U const& rhs) const->bool {  \
    return process_op(                                                         \
        Fn(lhs, rhs), rhs, {" " #Op " ", sizeof(" " #Op " ") - 1});            \
  }                                                                            \
  static_assert(true, "")

#define VEG_DISABLE(Op)                                                        \
  template <typename U>                                                        \
  void operator Op(U const& rhs) = delete

  VEG_DISABLE(<<);
  VEG_DISABLE(>>);
  VEG_DISABLE(&);
  VEG_DISABLE(|);
  VEG_DISABLE(+);
  VEG_DISABLE(-);
  VEG_DISABLE(*);
  VEG_DISABLE(/);
  VEG_DISABLE(%);
  VEG_DISABLE(^);

  // if a VEG_ASSERT fails during the evaluation of the comparison result, the
  // behavior is undefined (potential recursion)
  VEG_CMP(==, ::veg::cmp_equal);
  VEG_CMP(!=, ::veg::cmp_not_equal);
  VEG_CMP(<, ::veg::cmp_less);
  VEG_CMP(>, ::veg::cmp_greater);
  VEG_CMP(<=, ::veg::cmp_less_equal);
  VEG_CMP(>=, ::veg::cmp_greater_equal);

#undef VEG_CMP
#undef VEG_DISABLE

  HEDLEY_ALWAYS_INLINE explicit constexpr
  operator bool() const { // NOLINT(hicpp-explicit-conversions)
    return process_bool(static_cast<bool>(lhs));
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto process_bool(bool res) const
      -> bool {
    return (res ? void(0)
                : internal::set_assert_params1(
                      {}, internal::to_string(lhs), {})),
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

#define VEG_IMPL_ASSERT_IMPL(Callback, If_Fail, ...)                           \
  (static_cast<bool>(::veg::assert::internal::decomposer{} << __VA_ARGS__)     \
       ? static_cast<void>(0)                                                  \
       : (::veg::assert::internal::cleanup{},                                  \
          ::veg::assert::internal::set_assert_params2(                         \
              ::veg::assert::internal::char_string_ref{                        \
                  #__VA_ARGS__, sizeof(#__VA_ARGS__) - 1},                     \
              ::veg::assert::internal::char_string_ref_(If_Fail)),             \
          ::veg::assert::internal::Callback(                                   \
              __LINE__,                                                        \
              ::veg::assert::internal::char_string_ref{                        \
                  __FILE__, sizeof(__FILE__) - 1},                             \
              ::veg::assert::internal::char_string_ref{                        \
                  VEG_THIS_FUNCTION, sizeof(VEG_THIS_FUNCTION) - 1})))

#define VEG_ASSERT_ELSE(...)                                                   \
  VEG_IGNORE_SHIFT_PAREN_WARNING(                                              \
      VEG_IMPL_ASSERT_IMPL, on_assert_fail, __VA_ARGS__)
#define VEG_EXPECT_ELSE(...)                                                   \
  VEG_IGNORE_SHIFT_PAREN_WARNING(                                              \
      VEG_IMPL_ASSERT_IMPL, on_expect_fail, __VA_ARGS__)

#define VEG_ASSERT(...)                                                        \
  VEG_IGNORE_SHIFT_PAREN_WARNING(                                              \
      VEG_IMPL_ASSERT_IMPL,                                                    \
      on_assert_fail,                                                          \
      (::veg::assert::internal::char_string_ref{"", 0}),                       \
      __VA_ARGS__)

#define VEG_EXPECT(...)                                                        \
  VEG_IGNORE_SHIFT_PAREN_WARNING(                                              \
      VEG_IMPL_ASSERT_IMPL,                                                    \
      on_expect_fail,                                                          \
      (::veg::assert::internal::char_string_ref{"", 0}),                       \
      __VA_ARGS__)

#define VEG_IMPL_ALL_OF_1(Ftor, Decomposer, Callback, ...)                     \
  VEG_IMPL_ALL_OF_2(                                                           \
      Ftor, Decomposer, Callback, VEG_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define VEG_IMPL_ASSERT_FTOR_EMPTY(_, Decomposer, Elem)                        \
  VEG_IMPL_ASSERT_FTOR(                                                        \
      _,                                                                       \
      Decomposer,                                                              \
      ((::veg::assert::internal::char_string_ref{"", 0}),                      \
       VEG_PP_REMOVE_PARENS(Elem)))

#define VEG_IMPL_ASSERT_FTOR(_, Decomposer, Elem)                              \
  static_cast<bool>(                                                           \
      ::veg::assert::internal::Decomposer{}                                    \
      << VEG_PP_REMOVE_PARENS(VEG_PP_TUPLE_POP_FRONT(Elem)))                   \
      ? true                                                                   \
      : (::veg::assert::internal::cleanup{},                                   \
         ::veg::assert::internal::set_assert_params2(                          \
             {VEG_PP_STRINGIZE(                                                \
                  VEG_PP_REMOVE_PARENS(VEG_PP_TUPLE_POP_FRONT(Elem))),         \
              sizeof(VEG_PP_STRINGIZE(                                         \
                  VEG_PP_REMOVE_PARENS(VEG_PP_TUPLE_POP_FRONT(Elem)))) -       \
                  1},                                                          \
             ::veg::assert::internal::char_string_ref_(                        \
                 VEG_PP_TUPLE_ELEM(0, Elem))),                                 \
         false),

#define VEG_IMPL_ALL_OF_2(Ftor, Decomposer, Callback, Seq)                     \
  (::veg::meta::all_of({VEG_PP_SEQ_FOR_EACH(Ftor, Decomposer, Seq)})           \
       ? static_cast<void>(0)                                                  \
       : ::veg::assert::internal::Callback(                                    \
             __LINE__,                                                         \
             {__FILE__, sizeof(__FILE__) - 1},                                 \
             {VEG_THIS_FUNCTION, sizeof(VEG_THIS_FUNCTION) - 1}))

#define VEG_ASSERT_ALL_OF(...)                                                 \
  VEG_IGNORE_SHIFT_PAREN_WARNING(                                              \
      VEG_IMPL_ALL_OF_1,                                                       \
      VEG_IMPL_ASSERT_FTOR_EMPTY,                                              \
      decomposer,                                                              \
      on_assert_fail,                                                          \
      __VA_ARGS__)

#define VEG_EXPECT_ALL_OF(...)                                                 \
  VEG_IGNORE_SHIFT_PAREN_WARNING(                                              \
      VEG_IMPL_ALL_OF_1,                                                       \
      VEG_IMPL_ASSERT_FTOR_EMPTY,                                              \
      decomposer,                                                              \
      on_expect_fail,                                                          \
      __VA_ARGS__)

#define VEG_ASSERT_ALL_OF_ELSE(...)                                            \
  VEG_IGNORE_SHIFT_PAREN_WARNING(                                              \
      VEG_IMPL_ALL_OF_1,                                                       \
      VEG_IMPL_ASSERT_FTOR,                                                    \
      decomposer,                                                              \
      on_assert_fail,                                                          \
      __VA_ARGS__)

#define VEG_EXPECT_ALL_OF_ELSE(...)                                            \
  VEG_IGNORE_SHIFT_PAREN_WARNING(                                              \
      VEG_IMPL_ALL_OF_1,                                                       \
      VEG_IMPL_ASSERT_FTOR,                                                    \
      decomposer,                                                              \
      on_expect_fail,                                                          \
      __VA_ARGS__)

} // namespace assert
} // namespace veg

#endif /* end of include guard VEG_ASSERT_HPP_VQDAJ2IBS */

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
#define VEG_DEBUG_ASSERT(...) (static_cast<void>(0))
#define VEG_DEBUG_EXPECT(...) (static_cast<void>(0))
#define VEG_DEBUG_ASSERT_ELSE(Message, ...) (static_cast<void>(0))
#define VEG_DEBUG_EXPECT_ELSE(Message, ...) (static_cast<void>(0))
#define VEG_DEBUG_ASSERT_ALL_OF(...) (static_cast<void>(0))
#define VEG_DEBUG_EXPECT_ALL_OF(...) (static_cast<void>(0))
#define VEG_DEBUG_ASSERT_ALL_OF_ELSE(...) (static_cast<void>(0))
#define VEG_DEBUG_EXPECT_ALL_OF_ELSE(...) (static_cast<void>(0))
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

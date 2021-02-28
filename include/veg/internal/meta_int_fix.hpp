#ifndef __VEG_META_INT_FIX_HPP_7S9Y48TFS
#define __VEG_META_INT_FIX_HPP_7S9Y48TFS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/std.hpp"
#include "veg/internal/byte_string_ref.hpp"
#include "veg/internal/simple_string.hpp"

namespace veg {

namespace assert {
namespace fn {
struct to_string;
} // namespace fn
} // namespace assert

enum struct ternary_e : unsigned char {
  no,
  maybe,
  yes,
};

constexpr auto no = ternary_e::no;
constexpr auto maybe = ternary_e::maybe;
constexpr auto yes = ternary_e::yes;
using no_c = std::integral_constant<ternary_e, ternary_e::no>;
using maybe_c = std::integral_constant<ternary_e, ternary_e::maybe>;
using yes_c = std::integral_constant<ternary_e, ternary_e::yes>;

namespace int_c {

template <ternary_e T>
struct boolean;

template <ternary_e T>
struct boolean {
  constexpr boolean() noexcept = default;
  using type = std::integral_constant<ternary_e, T>;

  HEDLEY_ALWAYS_INLINE constexpr boolean(
      boolean<maybe> /*b*/, unsafe_t /*tag*/) noexcept;
  HEDLEY_ALWAYS_INLINE constexpr boolean // NOLINT(hicpp-explicit-conversions)
      (boolean<maybe> b, safe_t /*tag*/ = {}) noexcept;

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
  operator!(boolean /*arg*/) noexcept -> boolean<T == yes ? no : yes> {
    return {};
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE explicit constexpr
  operator bool() const noexcept {
    return T == yes;
  }

  friend auto __VEG_CPP14(constexpr)
      tag_invoke(tag_t<assert::fn::to_string> /*tag*/, boolean /*arg*/) noexcept
      -> assert::internal::char_string_ref {
    constexpr auto const& yes_str = "maybe(true)";
    constexpr auto const& no_str = "maybe(false)";
    auto str = T == yes ? yes_str : no_str;
    auto len = i64(T == yes ? sizeof(yes_str) : sizeof(no_str)) - 1;

    return {str, len};
  }

  template <typename CharT, typename Traits>
  friend auto
  operator<<(std::basic_ostream<CharT, Traits>& out, boolean arg) noexcept
      -> std::basic_ostream<CharT, Traits>& {
    auto str = tag_invoke(tag<assert::fn::to_string>, arg);
    out.write(str.data(), str.size());
    return out;
  }
};

struct dyn;
template <i64 N>
struct fix;

} // namespace int_c
namespace meta {
namespace internal {
template <typename T>
struct is_fix : std::false_type {};
template <i64 N>
struct is_fix<int_c::fix<N>> : std::true_type {};
} // namespace internal

template <typename T>
using meta_int =
    bool_constant<__VEG_SAME_AS(T, int_c::dyn) || internal::is_fix<T>::value>;

} // namespace meta

namespace int_c {

template <i64 N>
struct fix {
  constexpr fix() noexcept = default;
  HEDLEY_ALWAYS_INLINE constexpr fix(dyn /*arg*/, unsafe_t /*tag*/) noexcept;
  HEDLEY_ALWAYS_INLINE constexpr fix // NOLINT(hicpp-explicit-conversions)
      (dyn arg, safe_t /*tag*/ = {}) noexcept;
  VEG_TEMPLATE((i64 M), requires M != N, constexpr fix, (/*arg*/, fix<M>)) =
      delete;

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE explicit constexpr
  operator i64() const noexcept {
    return N;
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
  operator+(fix /**/) noexcept -> fix {
    return {};
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
  operator-(fix /**/) noexcept -> fix<-N> {
    return {};
  }

  friend auto
  tag_invoke(tag_t<assert::fn::to_string> /*tag*/, fix /*arg*/) noexcept
      -> assert::internal::string {
    auto str = assert::internal::to_string_primitive<long long>(N);
    str.insert(0, "fix[", 4);
    str.insert(str.size(), "]", 1);

    return str;
  }

  template <typename CharT, typename Traits>
  friend auto
  operator<<(std::basic_ostream<CharT, Traits>& out, fix arg) noexcept
      -> std::basic_ostream<CharT, Traits>& {
    auto str = tag_invoke(tag<assert::fn::to_string>, arg);
    out.write(str.data(), str.size());
    return out;
  }
};

namespace internal {

struct error {
  constexpr auto operator()(u64 const* fail = nullptr) const noexcept -> u64 {
    return *fail;
  }
};

using parser = auto (*)(char, error) -> u64;
constexpr auto parse_digit_2(char c, error e) noexcept -> u64 {
  return (c == '0') ? 0 : (c == '1' ? 1 : e());
}
constexpr auto parse_digit_8(char c, error e) noexcept -> u64 {
  return (c >= '0' && c <= '7') ? u64(c - '0') : e();
}
constexpr auto parse_digit_10(char c, error e) noexcept -> u64 {
  return (c >= '0' && c <= '9') ? u64(c - '0') : e();
}
constexpr auto parse_digit_16(char c, error e) noexcept -> u64 {
  return (c >= '0' && c <= '9') //
             ? u64(c - '0')
             : (c >= 'a' && c <= 'f') //
                   ? u64(c - 'a')
                   : (c >= 'A' && c <= 'F') //
                         ? u64(c - 'A')
                         : e();
}

constexpr auto parse_digit(u64 radix) noexcept -> parser {
  return radix == 2
             ? parse_digit_2
             : (radix == 8
                    ? parse_digit_8
                    : (radix == 10 ? parse_digit_10
                                   : (radix == 16 ? parse_digit_16 : nullptr)));
}

constexpr auto parse_num(char const* str, u64 len, u64 radix, error e) noexcept
    -> u64 {
  return (len == 0) ? 0
                    : radix * parse_num(str, len - 1, radix, e) +
                          (parse_digit(radix)(str[len - 1], e));
}

constexpr auto parse_int(char const* str, u64 len, error e) noexcept -> u64 {
  return (len == 0) //
             ? e()
             : ((str[0] == '0')   //
                    ? ((len == 1) //
                           ? 0
                           : (str[1] == 'b' || str[1] == 'B') //
                                 ? parse_num(str + 2, len - 2, 2, e)
                                 : (str[1] == 'x' || str[1] == 'X') //
                                       ? parse_num(str + 2, len - 2, 16, e)
                                       : parse_num(str + 1, len - 1, 8, e))
                    : parse_num(str, len, 10, e));
}

constexpr auto init_parse_int(std::initializer_list<char> str) noexcept -> u64 {
  return parse_int(str.begin(), str.size(), error{});
}

template <typename L, typename R>
struct binary_traits {
  using add = void;
  using sub = void;
  using mul = void;
  using div = void;

  using cmp_eq = void;
  using cmp_neq = void;
  using cmp_lt = void;
  using cmp_le = void;
  using cmp_gt = void;
  using cmp_ge = void;
};

template <i64 N, i64 M>
struct binary_traits<fix<N>, fix<M>> {
  template <i64 I>
  struct dims {
    HEDLEY_ALWAYS_INLINE constexpr dims(fix<N> /*r*/, fix<M> /*c*/) noexcept {}
    VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto nrows() const noexcept
        -> fix<N> {
      return {};
    }
    VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto ncols() const noexcept
        -> fix<M> {
      return {};
    }
  };

#define __VEG_OP(Name, Op)                                                     \
  using Name /* NOLINT(bugprone-macro-parentheses) */ = fix<N Op M>;           \
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto Name##_fn(          \
      fix<N>, fix<M>) noexcept->Name {                                         \
    return {};                                                                 \
  }                                                                            \
  static_assert(true, "")

#define __VEG_CMP(Name, Op)                                                    \
  using Name /* NOLINT(bugprone-macro-parentheses) */ =                        \
      boolean<(N Op M) ? yes : no>;                                            \
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto Name##_fn(          \
      fix<N>, fix<M>) noexcept->Name {                                         \
    return {};                                                                 \
  }                                                                            \
  static_assert(true, "")

  __VEG_OP(add, +);
  __VEG_OP(sub, -);
  __VEG_OP(mul, *);
  __VEG_CMP(cmp_eq, ==);
  __VEG_CMP(cmp_neq, !=);
  __VEG_CMP(cmp_lt, <);
  __VEG_CMP(cmp_le, <=);
  __VEG_CMP(cmp_gt, >);
  __VEG_CMP(cmp_ge, >=);

  using div = meta::conditional_t<M == 0, void, fix<N / (M != 0 ? M : 1)>>;
  using mod = meta::conditional_t<M == 0, void, fix<N % (M != 0 ? M : 1)>>;

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto
  div_fn(fix<N> /*a*/, fix<M> /*b*/) noexcept -> div {
    return div();
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto
  mod_fn(fix<N> /*a*/, fix<M> /*b*/) noexcept -> mod {
    return mod();
  }

#undef __VEG_OP
#undef __VEG_CMP
};

} // namespace internal

VEG_TEMPLATE(
    (typename L, typename R),
    requires meta::meta_int<L>::value&& meta::meta_int<R>::value,
    VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
    operator+,
    (a, L),
    (b, R))
__VEG_DEDUCE_RET(internal::binary_traits<L, R>::add_fn(a, b));

VEG_TEMPLATE(
    (typename L, typename R),
    requires meta::meta_int<L>::value&& meta::meta_int<R>::value,
    VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
    operator-,
    (a, L),
    (b, R))
__VEG_DEDUCE_RET(internal::binary_traits<L, R>::sub_fn(a, b));

VEG_TEMPLATE(
    (typename L, typename R),
    requires meta::meta_int<L>::value&& meta::meta_int<R>::value,
    VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
    operator*,
    (a, L),
    (b, R))
__VEG_DEDUCE_RET(internal::binary_traits<L, R>::mul_fn(a, b));

VEG_TEMPLATE(
    (typename L, typename R),
    requires(
        meta::meta_int<L>::value&& meta::meta_int<R>::value&&
            meta::meta_int<typename internal::binary_traits<L, R>::div>::value),
    VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
    operator/,
    (a, L),
    (b, R))
__VEG_DEDUCE_RET(internal::binary_traits<L, R>::div_fn(a, b));

VEG_TEMPLATE(
    (typename L, typename R),
    requires(
        meta::meta_int<L>::value&& meta::meta_int<R>::value&&
            meta::meta_int<typename internal::binary_traits<L, R>::mod>::value),
    VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
    operator%,
    (a, L),
    (b, R))
__VEG_DEDUCE_RET(internal::binary_traits<L, R>::mod_fn(a, b));

#define __VEG_CMP(Name, Op)                                                    \
  VEG_TEMPLATE(                                                                \
      (typename L, typename R),                                                \
      requires meta::meta_int<L>::value&& meta::meta_int<R>::value,            \
      VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto                        \
      operator Op,                                                             \
      (a, L),                                                                  \
      (b, R))                                                                  \
  __VEG_DEDUCE_RET(internal::binary_traits<L, R>::cmp_##Name##_fn(a, b))

__VEG_CMP(eq, ==);
__VEG_CMP(neq, !=);
__VEG_CMP(lt, <);
__VEG_CMP(le, <=);
__VEG_CMP(gt, >);
__VEG_CMP(ge, >=);

#undef __VEG_CMP

} // namespace int_c
using int_c::boolean;
using int_c::dyn;
using int_c::fix;

inline namespace literals {
template <char... Chars>
HEDLEY_ALWAYS_INLINE constexpr auto operator"" _c() noexcept
    -> fix<int_c::internal::init_parse_int({Chars...})> {
  return {};
}
} // namespace literals
} // namespace veg

#endif /* end of include guard __VEG_META_INT_FIX_HPP_7S9Y48TFS */

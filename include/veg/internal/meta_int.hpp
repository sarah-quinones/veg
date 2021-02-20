#ifndef VEG_META_INT_HPP_5VQBGXGYS
#define VEG_META_INT_HPP_5VQBGXGYS

#include "veg/internal/type_traits.hpp"
#include "veg/assert.hpp"
#include "veg/internal/std.hpp"

namespace veg {

namespace fn {
template <typename To>
struct narrow_fn {
  VEG_TEMPLATE(
      (typename From),
      requires meta::arithmetic<From>::value&& meta::arithmetic<To>::value,
      constexpr auto
      operator(),
      (from, From))
  const noexcept->To;
};
template <typename To>
VEG_TEMPLATE(
    (typename From),
    requires meta::arithmetic<From>::value&& meta::arithmetic<To>::value,
    constexpr auto narrow_fn<To>::operator(),
    (from, From))
const noexcept -> To {
  return VEG_ASSERT(static_cast<From>(static_cast<To>(from)) == from),
         static_cast<To>(from);
}
} // namespace fn
template <typename To>
constexpr fn::narrow_fn<To> narrow{};

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

template <>
struct boolean<maybe> {
  using type = maybe_c;

  constexpr boolean /* NOLINT(hicpp-explicit-conversions) */ (
      bool val = false) noexcept
      : m_val(val) {}
  template <ternary_e T>
  constexpr boolean /* NOLINT(hicpp-explicit-conversions) */ (
      boolean<T> /*arg*/, safe_t /*tag*/ = {}) noexcept
      : m_val(T == yes) {}

  VEG_NODISCARD constexpr friend auto operator!(boolean arg) noexcept
      -> boolean {
    return {!arg.m_val};
  }
  VEG_NODISCARD explicit constexpr operator bool() const noexcept {
    return m_val;
  }

  friend auto VEG_CPP14(constexpr)
      tag_invoke(tag_t<assert::fn::to_string_fn> /*tag*/, boolean arg) noexcept
      -> assert::internal::char_string_ref {
    constexpr auto const& yes_str = "maybe(true)";
    constexpr auto const& no_str = "maybe(false)";
    char const* str = arg.m_val ? yes_str : no_str;
    auto len = i64(arg.m_val ? sizeof(yes_str) : sizeof(no_str)) - 1;

    return {str, len};
  }

  template <typename CharT, typename Traits>
  friend auto
  operator<<(std::basic_ostream<CharT, Traits>& out, boolean arg) noexcept
      -> std::basic_ostream<CharT, Traits>& {
    auto str = tag_invoke(tag<assert::fn::to_string_fn>, arg);
    out.write(str.data(), str.size());
    return out;
  }

private:
  bool m_val;
};

template <ternary_e T>
struct boolean {
  constexpr boolean() noexcept = default;
  using type = std::integral_constant<ternary_e, T>;

  constexpr boolean(boolean<maybe> /*b*/, unsafe_t /*tag*/) noexcept {}
  constexpr boolean /* NOLINT(hicpp-explicit-conversions) */ (
      boolean<maybe> b, safe_t /*tag*/ = {}) noexcept
      : boolean((VEG_ASSERT(b.m_val == (T == yes)), b), unsafe) {}

  VEG_NODISCARD constexpr friend auto operator!(boolean /*arg*/) noexcept
      -> boolean<T == yes ? no : yes> {
    return {};
  }
  VEG_NODISCARD explicit constexpr operator bool() const noexcept {
    return T == yes;
  }

  friend auto VEG_CPP14(constexpr) tag_invoke(
      tag_t<assert::fn::to_string_fn> /*tag*/, boolean /*arg*/) noexcept
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
    auto str = tag_invoke(tag<assert::fn::to_string_fn>, arg);
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
    bool_constant<VEG_SAME_AS(T, int_c::dyn) || internal::is_fix<T>::value>;

} // namespace meta

namespace int_c {

struct dyn {
  constexpr dyn /* NOLINT(hicpp-explicit-conversions) */ (i64 val = 0) noexcept
      : m_val(val) {}
  template <i64 N>
  constexpr dyn /* NOLINT(hicpp-explicit-conversions) */ (
      fix<N> /*arg*/) noexcept
      : m_val(N) {}

  VEG_NODISCARD explicit constexpr operator i64() const noexcept {
    return m_val;
  }
  VEG_NODISCARD constexpr friend auto operator+(dyn arg) noexcept -> dyn {
    return arg;
  }
  VEG_NODISCARD constexpr friend auto operator-(dyn arg) noexcept -> dyn {
    return {-arg.m_val};
  }

  friend auto
  tag_invoke(tag_t<assert::fn::to_string_fn> /*tag*/, dyn arg) noexcept
      -> assert::internal::string {
    auto str = assert::to_string(i64(arg.m_val));
    str.insert(0, "dyn[", 4);
    str.insert(str.size(), "]", 1);

    return str;
  }

  template <typename CharT, typename Traits>
  friend auto
  operator<<(std::basic_ostream<CharT, Traits>& out, dyn arg) noexcept
      -> std::basic_ostream<CharT, Traits>& {
    auto str = tag_invoke(tag<assert::fn::to_string_fn>, arg);
    out.write(str.data(), str.size());
    return out;
  }

private:
  i64 m_val;
};

template <i64 N>
struct fix {
  constexpr fix() noexcept = default;
  constexpr fix(dyn /*arg*/, unsafe_t /*tag*/) noexcept {}
  constexpr fix /* NOLINT(hicpp-explicit-conversions) */ (
      dyn arg, safe_t /*tag*/ = {}) noexcept
      : fix((VEG_ASSERT(i64(arg) == N), arg), unsafe) {}
  VEG_TEMPLATE((i64 M), requires M != N, constexpr fix, (/*arg*/, fix<M>)) =
      delete;

  VEG_NODISCARD explicit constexpr operator i64() const noexcept { return N; }
  VEG_NODISCARD constexpr friend auto operator+(fix /**/) noexcept -> fix {
    return {};
  }
  VEG_NODISCARD constexpr friend auto operator-(fix /**/) noexcept -> fix<-N> {
    return {};
  }

  friend auto
  tag_invoke(tag_t<assert::fn::to_string_fn> /*tag*/, fix /*arg*/) noexcept
      -> assert::internal::string {
    auto str = assert::to_string(N);
    str.insert(0, "fix[", 4);
    str.insert(str.size(), "]", 1);

    return str;
  }

  template <typename CharT, typename Traits>
  friend auto
  operator<<(std::basic_ostream<CharT, Traits>& out, fix arg) noexcept
      -> std::basic_ostream<CharT, Traits>& {
    auto str = tag_invoke(tag<assert::fn::to_string_fn>, arg);
    out.write(str.data(), str.size());
    return out;
  }
};

namespace _ {

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
    constexpr dims(fix<N> /*r*/, fix<M> /*c*/) noexcept {}
    VEG_NODISCARD constexpr auto nrows() const noexcept -> fix<N> { return {}; }
    VEG_NODISCARD constexpr auto ncols() const noexcept -> fix<M> { return {}; }
  };

#define VEG_OP(Name, Op)                                                       \
  using Name /* NOLINT(bugprone-macro-parentheses) */ = fix<N Op M>;           \
  VEG_NODISCARD static constexpr auto Name##_fn(                               \
      fix<N>, fix<M>) noexcept->Name {                                         \
    return {};                                                                 \
  }                                                                            \
  static_assert(true, "")

#define VEG_CMP(Name, Op)                                                      \
  using Name /* NOLINT(bugprone-macro-parentheses) */ =                        \
      boolean<(N Op M) ? yes : no>;                                            \
  VEG_NODISCARD static constexpr auto Name##_fn(                               \
      fix<N>, fix<M>) noexcept->Name {                                         \
    return {};                                                                 \
  }                                                                            \
  static_assert(true, "")

  VEG_OP(add, +);
  VEG_OP(sub, -);
  VEG_OP(mul, *);
  VEG_CMP(cmp_eq, ==);
  VEG_CMP(cmp_neq, !=);
  VEG_CMP(cmp_lt, <);
  VEG_CMP(cmp_le, <=);
  VEG_CMP(cmp_gt, >);
  VEG_CMP(cmp_ge, >=);

  using div = meta::conditional_t<M == 0, void, fix<N / (M != 0 ? M : 1)>>;
  using mod = meta::conditional_t<M == 0, void, fix<N % (M != 0 ? M : 1)>>;

  VEG_NODISCARD static constexpr auto
  div_fn(fix<N> /*a*/, fix<M> /*b*/) noexcept -> div {
    return div();
  }
  VEG_NODISCARD static constexpr auto
  mod_fn(fix<N> /*a*/, fix<M> /*b*/) noexcept -> mod {
    return mod();
  }

#undef VEG_OP
#undef VEG_CMP
};

template <>
struct binary_traits<dyn, dyn> {
  template <i64 I>
  struct dims {
    constexpr dims(dyn r, dyn c) noexcept : m_rows(r), m_cols(c) {}
    VEG_NODISCARD constexpr auto nrows() const noexcept -> dyn {
      return m_rows;
    }
    VEG_NODISCARD constexpr auto ncols() const noexcept -> dyn {
      return m_cols;
    }
    dyn m_rows;
    dyn m_cols;
  };

#define VEG_OP(Name, Op)                                                       \
  using Name /* NOLINT(bugprone-macro-parentheses) */ = dyn;                   \
  VEG_NODISCARD static constexpr auto Name##_fn(dyn a, dyn b) noexcept->Name { \
    return {i64(a) Op i64(b)};                                                 \
  }                                                                            \
  static_assert(true, "")

#define VEG_CMP(Name, Op)                                                      \
  using Name /* NOLINT(bugprone-macro-parentheses) */ = boolean<maybe>;        \
  VEG_NODISCARD static constexpr auto Name##_fn(dyn a, dyn b) noexcept->Name { \
    return (i64(a) Op i64(b));                                                 \
  }                                                                            \
  static_assert(true, "")

  VEG_OP(add, +);
  VEG_OP(sub, -);
  VEG_OP(mul, *);
  VEG_CMP(cmp_eq, ==);
  VEG_CMP(cmp_neq, !=);
  VEG_CMP(cmp_lt, <);
  VEG_CMP(cmp_le, <=);
  VEG_CMP(cmp_gt, >);
  VEG_CMP(cmp_ge, >=);

  using div = dyn;
  using mod = dyn;

  VEG_NODISCARD static constexpr auto div_fn(dyn a, dyn b) noexcept -> div {
    return VEG_ASSERT(i64(b) != 0), i64(a) / i64(b);
  }
  VEG_NODISCARD static constexpr auto mod_fn(dyn a, dyn b) noexcept -> mod {

    return VEG_ASSERT(i64(b) != 0), i64(a) % i64(b);
  }

#undef VEG_OP
#undef VEG_CMP
};

template <i64 N>
struct binary_traits<fix<N>, dyn> : binary_traits<dyn, dyn> {
  template <i64 I>
  struct dims {
    constexpr dims(fix<N> /*r*/, dyn c) noexcept : m_cols(c) {}
    VEG_NODISCARD constexpr auto nrows() const noexcept -> fix<N> { return {}; }
    VEG_NODISCARD constexpr auto ncols() const noexcept -> dyn {
      return m_cols;
    }
    dyn m_cols;
  };
};

template <>
struct binary_traits<fix<0>, dyn> : binary_traits<dyn, dyn> {
  template <i64 I>
  struct dims {
    constexpr dims(fix<0> /*r*/, dyn c) noexcept : m_cols(c) {}
    VEG_NODISCARD constexpr auto nrows() const noexcept -> fix<0> { return {}; }
    VEG_NODISCARD auto ncols() const noexcept -> dyn { return m_cols; }
    dyn m_cols;
  };

  using mul = fix<0>;
  VEG_NODISCARD
  constexpr static auto mul_fn(fix<0> /*a*/, dyn /*b*/) noexcept -> mul {
    return {};
  }
};

template <i64 N>
struct binary_traits<dyn, fix<N>> : binary_traits<dyn, dyn> {
  template <i64 I>
  struct dims {
    constexpr dims(dyn r, fix<N> /*c*/) noexcept : m_rows(r) {}
    VEG_NODISCARD constexpr auto nrows() const noexcept -> dyn {
      return m_rows;
    }
    VEG_NODISCARD constexpr auto ncols() const noexcept -> fix<N> { return {}; }
    dyn m_rows;
  };

  using mul = typename binary_traits<fix<N>, dyn>::mul;
  static constexpr auto mul_fn(dyn a, fix<N> /*b*/) noexcept -> mul {
    return binary_traits<fix<N>, dyn>::mul_fn({}, a);
  }

  using div = meta::conditional_t<N == 0, void, dyn>;
  using mod = meta::conditional_t<N == 0, void, dyn>;

  VEG_NODISCARD static constexpr auto div_fn(dyn a, fix<N> /*b*/) noexcept
      -> div {
    return div(i64(a) / N);
  }
  VEG_NODISCARD static constexpr auto mod_fn(dyn a, fix<N> /*b*/) noexcept
      -> mod {
    return mod(i64(a) % N);
  }
};

} // namespace _

VEG_TEMPLATE(
    (typename L, typename R),
    requires meta::meta_int<L>::value&& meta::meta_int<R>::value,
    VEG_NODISCARD constexpr auto
    operator+,
    (a, L),
    (b, R))
VEG_DEDUCE_RET(_::binary_traits<L, R>::add_fn(a, b));

VEG_TEMPLATE(
    (typename L, typename R),
    requires meta::meta_int<L>::value&& meta::meta_int<R>::value,
    VEG_NODISCARD constexpr auto
    operator-,
    (a, L),
    (b, R))
VEG_DEDUCE_RET(_::binary_traits<L, R>::sub_fn(a, b));

VEG_TEMPLATE(
    (typename L, typename R),
    requires meta::meta_int<L>::value&& meta::meta_int<R>::value,
    VEG_NODISCARD constexpr auto
    operator*,
    (a, L),
    (b, R))
VEG_DEDUCE_RET(_::binary_traits<L, R>::mul_fn(a, b));

VEG_TEMPLATE(
    (typename L, typename R),
    requires(meta::meta_int<L>::value&& meta::meta_int<R>::value&&
                 meta::meta_int<typename _::binary_traits<L, R>::div>::value),
    VEG_NODISCARD constexpr auto
    operator/,
    (a, L),
    (b, R))
VEG_DEDUCE_RET(_::binary_traits<L, R>::div_fn(a, b));

VEG_TEMPLATE(
    (typename L, typename R),
    requires(meta::meta_int<L>::value&& meta::meta_int<R>::value&&
                 meta::meta_int<typename _::binary_traits<L, R>::mod>::value),
    VEG_NODISCARD constexpr auto
    operator%,
    (a, L),
    (b, R))
VEG_DEDUCE_RET(_::binary_traits<L, R>::mod_fn(a, b));

#define VEG_CMP(Name, Op)                                                      \
  VEG_TEMPLATE(                                                                \
      (typename L, typename R),                                                \
      requires meta::meta_int<L>::value&& meta::meta_int<R>::value,            \
      VEG_NODISCARD constexpr auto                                             \
      operator Op,                                                             \
      (a, L),                                                                  \
      (b, R))                                                                  \
  VEG_DEDUCE_RET(_::binary_traits<L, R>::cmp_##Name##_fn(a, b))

VEG_CMP(eq, ==);
VEG_CMP(neq, !=);
VEG_CMP(lt, <);
VEG_CMP(le, <=);
VEG_CMP(gt, >);
VEG_CMP(ge, >=);

#undef VEG_CMP

} // namespace int_c
using int_c::boolean;
using int_c::dyn;
using int_c::fix;

inline namespace literals {
template <char... Chars>
constexpr auto operator"" _c() noexcept
    -> fix<int_c::_::init_parse_int({Chars...})> {
  return {};
}
constexpr auto operator"" _v(unsigned long long n) noexcept -> dyn {
  return veg::narrow<i64>(n);
}
} // namespace literals
} // namespace veg

#endif /* end of include guard VEG_META_INT_HPP_5VQBGXGYS */

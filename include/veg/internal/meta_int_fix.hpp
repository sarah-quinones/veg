#ifndef VEG_META_INT_FIX_HPP_7S9Y48TFS
#define VEG_META_INT_FIX_HPP_7S9Y48TFS

#include "veg/type_traits/tags.hpp"
#include "veg/internal/std.hpp"
#include "veg/internal/fmt.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {

namespace int_c {
struct Dyn;
template <i64 N>
struct Fix;
} // namespace int_c

namespace internal {
namespace meta_ {
template <typename T>
struct is_fix : false_type {};
template <i64 N>
struct is_fix<int_c::Fix<N>> : true_type {};
} // namespace meta_
} // namespace internal

namespace concepts {
VEG_DEF_CONCEPT(
		typename T,
		meta_int,
		VEG_CONCEPT(same<T, int_c::Dyn>) || internal::meta_::is_fix<T>::value);
} // namespace concepts

enum struct ternary_e : unsigned char {
	no,
	maybe,
	yes,
};

constexpr auto no = ternary_e::no;
constexpr auto maybe = ternary_e::maybe;
constexpr auto yes = ternary_e::yes;
using no_c = meta::constant<ternary_e, ternary_e::no>;
using maybe_c = meta::constant<ternary_e, ternary_e::maybe>;
using yes_c = meta::constant<ternary_e, ternary_e::yes>;

namespace int_c {

template <ternary_e T>
struct Boolean;

template <ternary_e T>
struct Boolean {
	constexpr Boolean() noexcept = default;
	using type = meta::constant<ternary_e, T>;

	HEDLEY_ALWAYS_INLINE constexpr Boolean(
			Boolean<maybe> /*b*/, Unsafe /*tag*/) noexcept;
	HEDLEY_ALWAYS_INLINE constexpr Boolean // NOLINT(hicpp-explicit-conversions)
			(Boolean<maybe> b, Safe /*tag*/ = {}) noexcept;

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
	operator!(Boolean /*arg*/) noexcept -> Boolean<T == yes ? no : yes> {
		return {};
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE explicit constexpr
	operator bool() const noexcept {
		return T == yes;
	}

private:
	void print(fmt::Buffer& out) const {
		constexpr auto const& yes_str = "maybe(true)";
		constexpr auto const& no_str = "maybe(false)";
		char const* str = (T == yes) ? yes_str : no_str;
		auto len = i64((T == yes) ? sizeof(yes_str) : sizeof(no_str)) - 1;
		out.insert(out.size(), str, len);
	}
};

} // namespace int_c

namespace int_c {

template <i64 N>
struct Fix {
	constexpr Fix() noexcept = default;
	HEDLEY_ALWAYS_INLINE constexpr Fix(Dyn /*arg*/, Unsafe /*tag*/) noexcept;
	HEDLEY_ALWAYS_INLINE constexpr Fix // NOLINT(hicpp-explicit-conversions)
			(Dyn arg, Safe /*tag*/ = {}) noexcept;
	VEG_TEMPLATE((i64 M), requires((M != N)), constexpr Fix, (/*arg*/, Fix<M>)) =
			delete;

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE explicit constexpr
	operator i64() const noexcept {
		return N;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
	operator+(Fix /**/) noexcept -> Fix {
		return {};
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
	operator-(Fix /**/) noexcept -> Fix<-N> {
		return {};
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

template <char... Chars>
struct char_seq {
	static constexpr char value[] = {Chars...};
};

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
struct binary_traits<Fix<N>, Fix<M>> {

#define VEG_OP(Name, Op)                                                       \
	using Name /* NOLINT(bugprone-macro-parentheses) */ = Fix<N Op M>;           \
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto Name##_fn(          \
			Fix<N>, Fix<M>) noexcept->Name {                                         \
		return {};                                                                 \
	}                                                                            \
	static_assert(true, "")

#define VEG_CMP(Name, Op)                                                      \
	using Name /* NOLINT(bugprone-macro-parentheses) */ =                        \
			Boolean<(N Op M) ? yes : no>;                                            \
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto Name##_fn(          \
			Fix<N>, Fix<M>) noexcept->Name {                                         \
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

	using div = meta::conditional_t<M == 0, void, Fix<N / (M != 0 ? M : 1)>>;
	using mod = meta::conditional_t<M == 0, void, Fix<N % (M != 0 ? M : 1)>>;

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto
	div_fn(Fix<N> /*a*/, Fix<M> /*b*/) noexcept -> div {
		return div();
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto
	mod_fn(Fix<N> /*a*/, Fix<M> /*b*/) noexcept -> mod {
		return mod();
	}

#undef VEG_OP
#undef VEG_CMP
};

} // namespace internal

VEG_TEMPLATE(
		(typename L, typename R),
		requires(VEG_CONCEPT(meta_int<L>) && VEG_CONCEPT(meta_int<R>)),
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
		operator+,
		(a, L),
		(b, R))
VEG_DEDUCE_RET(internal::binary_traits<L, R>::add_fn(a, b));

VEG_TEMPLATE(
		(typename L, typename R),
		requires(VEG_CONCEPT(meta_int<L>) && VEG_CONCEPT(meta_int<R>)),
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
		operator-,
		(a, L),
		(b, R))
VEG_DEDUCE_RET(internal::binary_traits<L, R>::sub_fn(a, b));

VEG_TEMPLATE(
		(typename L, typename R),
		requires(VEG_CONCEPT(meta_int<L>) && VEG_CONCEPT(meta_int<R>)),
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
		operator*,
		(a, L),
		(b, R))
VEG_DEDUCE_RET(internal::binary_traits<L, R>::mul_fn(a, b));

VEG_TEMPLATE(
		(typename L, typename R),
		requires(
				VEG_CONCEPT(meta_int<L>) && //
				VEG_CONCEPT(meta_int<R>) &&
				VEG_CONCEPT(meta_int<typename internal::binary_traits<L, R>::div>)),
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
		operator/,
		(a, L),
		(b, R))
VEG_DEDUCE_RET(internal::binary_traits<L, R>::div_fn(a, b));

VEG_TEMPLATE(
		(typename L, typename R),
		requires(
				VEG_CONCEPT(meta_int<L>) && //
				VEG_CONCEPT(meta_int<R>) &&
				VEG_CONCEPT(meta_int<typename internal::binary_traits<L, R>::mod>)),
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
		operator%,
		(a, L),
		(b, R))
VEG_DEDUCE_RET(internal::binary_traits<L, R>::mod_fn(a, b));

#define VEG_CMP(Name, Op)                                                      \
	VEG_TEMPLATE(                                                                \
			(typename L, typename R),                                                \
			requires(VEG_CONCEPT(meta_int<L>) && VEG_CONCEPT(meta_int<R>)),          \
			VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto                        \
			operator Op,                                                             \
			(a, L),                                                                  \
			(b, R))                                                                  \
	VEG_DEDUCE_RET(internal::binary_traits<L, R>::cmp_##Name##_fn(a, b))

VEG_CMP(eq, ==);
VEG_CMP(neq, !=);
VEG_CMP(lt, <);
VEG_CMP(le, <=);
VEG_CMP(gt, >);
VEG_CMP(ge, >=);

#undef VEG_CMP

} // namespace int_c
using int_c::Boolean;
using int_c::Dyn;
using int_c::Fix;

inline namespace literals {
template <char... Chars>
HEDLEY_ALWAYS_INLINE constexpr auto operator"" _c() noexcept
		-> Fix<int_c::internal::parse_int(
				int_c::internal::char_seq<Chars...>::value,
				sizeof...(Chars),
				int_c::internal::error{})> {
	return {};
}
} // namespace literals

namespace fmt {
template <>
struct Debug<Boolean<yes>> {
	static void to_string(fmt::Buffer& out, Boolean<yes> /*val*/) {
		out.insert(out.size(), "yes", 3);
	}
};
template <>
struct Debug<Boolean<no>> {
	static void to_string(fmt::Buffer& out, Boolean<no> /*val*/) {
		out.insert(out.size(), "no", 2);
	}
};

template <i64 N>
struct Debug<Fix<N>> {
	static void to_string(fmt::Buffer& out, Fix<N> val) {
		out.insert(out.size(), "Fix[", 4);
		Debug<i64>::to_string(out, i64(val));
		out.insert(out.size(), "]", 1);
	}
};
} // namespace fmt
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_META_INT_FIX_HPP_7S9Y48TFS */

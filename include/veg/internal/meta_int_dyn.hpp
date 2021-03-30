#ifndef VEG_META_INT_DYN_HPP_GC385NKBS
#define VEG_META_INT_DYN_HPP_GC385NKBS

#include "veg/util/assert.hpp"
#include "veg/internal/meta_int_fix.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace int_c {

template <ternary_e T>
struct boolean;

template <>
struct boolean<maybe> {
	using type = maybe_c;

	constexpr boolean /* NOLINT(hicpp-explicit-conversions) */ (
			bool _val = false) noexcept
			: val{_val} {}
	template <ternary_e T>
	HEDLEY_ALWAYS_INLINE constexpr boolean /* NOLINT(hicpp-explicit-conversions)
	                                        */
			(boolean<T> /*arg*/, safe_t /*tag*/ = {}) noexcept
			: val(T == yes) {}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
	operator!(boolean arg) noexcept -> boolean {
		return {!arg.val};
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE explicit constexpr
	operator bool() const noexcept {
		return val;
	}

private:
	bool val;
};

struct dyn {
	constexpr dyn /* NOLINT(hicpp-explicit-conversions) */ (i64 val = 0) noexcept
			: m_val(val) {}
	template <i64 N>
	constexpr dyn /* NOLINT(hicpp-explicit-conversions) */ (
			fix<N> /*arg*/) noexcept
			: m_val(N) {}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE explicit constexpr
	operator i64() const noexcept {
		return m_val;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
	operator+(dyn arg) noexcept -> dyn {
		return arg;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
	operator-(dyn arg) noexcept -> dyn {
		return {-arg.m_val};
	}

private:
	i64 m_val;
};

template <ternary_e T>
HEDLEY_ALWAYS_INLINE constexpr boolean<T>::boolean(
		boolean<maybe> /*b*/, unsafe_t /*tag*/) noexcept {}
template <ternary_e T>
HEDLEY_ALWAYS_INLINE constexpr boolean<
		T>::boolean // NOLINT(hicpp-explicit-conversions)
		(boolean<maybe> b, safe_t /*tag*/) noexcept
		: boolean(((void)VEG_ASSERT(b.val == (T == yes)), b), unsafe) {}

template <i64 N>
HEDLEY_ALWAYS_INLINE constexpr fix<N>::fix(
		dyn /*arg*/, unsafe_t /*tag*/) noexcept {}
template <i64 N>
HEDLEY_ALWAYS_INLINE constexpr fix<N>::fix // NOLINT(hicpp-explicit-conversions)
		(dyn arg, safe_t /*tag*/) noexcept
		: fix((VEG_ASSERT(i64(arg) == N), arg), unsafe) {}

namespace internal {

template <>
struct binary_traits<dyn, dyn> {
#define VEG_OP(Name, Op)                                                       \
	using Name /* NOLINT(bugprone-macro-parentheses) */ = dyn;                   \
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto Name##_fn(          \
			dyn a, dyn b) noexcept->Name {                                           \
		return {i64(a) Op i64(b)};                                                 \
	}                                                                            \
	static_assert(true, "")

#define VEG_CMP(Name, Op)                                                      \
	using Name /* NOLINT(bugprone-macro-parentheses) */ = boolean<maybe>;        \
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto Name##_fn(          \
			dyn a, dyn b) noexcept->Name {                                           \
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
		return VEG_ASSERT(i64(b) != i64(0)), i64(a) / i64(b);
	}
	VEG_NODISCARD static constexpr auto mod_fn(dyn a, dyn b) noexcept -> mod {

		return VEG_ASSERT(i64(b) != i64(0)), i64(a) % i64(b);
	}

#undef VEG_OP
#undef VEG_CMP
};

template <i64 N>
struct binary_traits<fix<N>, dyn> : binary_traits<dyn, dyn> {};

template <>
struct binary_traits<fix<0>, dyn> : binary_traits<dyn, dyn> {
	using mul = fix<0>;
	VEG_NODISCARD
	constexpr HEDLEY_ALWAYS_INLINE static auto
	mul_fn(fix<0> /*a*/, dyn /*b*/) noexcept -> mul {
		return {};
	}
};

template <i64 N>
struct binary_traits<dyn, fix<N>> : binary_traits<dyn, dyn> {
	using mul = typename binary_traits<fix<N>, dyn>::mul;
	HEDLEY_ALWAYS_INLINE static constexpr auto
	mul_fn(dyn a, fix<N> /*b*/) noexcept -> mul {
		return binary_traits<fix<N>, dyn>::mul_fn({}, a);
	}

	using div = meta::conditional_t<N == 0, void, dyn>;
	using mod = meta::conditional_t<N == 0, void, dyn>;

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto
	div_fn(dyn a, fix<N> /*b*/) noexcept -> div {
		return div(i64(a) / N);
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto
	mod_fn(dyn a, fix<N> /*b*/) noexcept -> mod {
		return mod(i64(a) % N);
	}
};

} // namespace internal
} // namespace int_c

inline namespace literals {
HEDLEY_ALWAYS_INLINE constexpr auto operator"" _v(unsigned long long n) noexcept
		-> dyn {
	return nb::narrow<i64>{}(n);
}
} // namespace literals

namespace fmt {
template <>
struct debug<boolean<maybe>> {
	static void to_string(fmt::buffer& out, boolean<maybe> val) {
		out.insert(out.size(), "maybe[", 6);
		debug<bool>::to_string(out, bool(val));
		out.insert(out.size(), "]", 1);
	}
};

template <>
struct debug<dyn> {
	static void to_string(fmt::buffer& out, dyn val) {
		out.insert(out.size(), "dyn[", 4);
		debug<i64>::to_string(out, i64(val));
		out.insert(out.size(), "]", 1);
	}
};
} // namespace fmt
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_META_INT_DYN_HPP_GC385NKBS */

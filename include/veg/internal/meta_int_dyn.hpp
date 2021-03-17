#ifndef __VEG_META_INT_DYN_HPP_GC385NKBS
#define __VEG_META_INT_DYN_HPP_GC385NKBS

#include "veg/assert.hpp"
#include "veg/internal/meta_int_fix.hpp"
#include "veg/internal/narrow.hpp"

namespace veg {

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
	HEDLEY_ALWAYS_INLINE constexpr boolean /* NOLINT(hicpp-explicit-conversions)
	                                        */
			(boolean<T> /*arg*/, safe_t /*tag*/ = {}) noexcept
			: m_val(T == yes) {}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr friend auto
	operator!(boolean arg) noexcept -> boolean {
		return {!arg.m_val};
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE explicit constexpr
	operator bool() const noexcept {
		return m_val;
	}

	friend auto __VEG_CPP14(constexpr)
			tag_invoke(tag_t<assert::fn::to_string> /*tag*/, boolean arg) noexcept
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
		auto str = tag_invoke(tag_t<assert::fn::to_string>{}, arg);
		out.write(str.data(), str.size());
		return out;
	}

private:
	bool m_val;
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

	friend auto tag_invoke(tag_t<assert::fn::to_string> /*tag*/, dyn arg) noexcept
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
		auto str = tag_invoke(tag_t<assert::fn::to_string>{}, arg);
		out.write(str.data(), str.size());
		return out;
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
		: boolean(((void)VEG_ASSERT(b.m_val == (T == yes)), b), unsafe) {}

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
	template <i64 I>
	struct dims {
		HEDLEY_ALWAYS_INLINE constexpr dims(dyn r, dyn c) noexcept
				: m_rows(r), m_cols(c) {}
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto nrows() const noexcept
				-> dyn {
			return m_rows;
		}
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto ncols() const noexcept
				-> dyn {
			return m_cols;
		}
		dyn m_rows;
		dyn m_cols;
	};

#define __VEG_OP(Name, Op)                                                     \
	using Name /* NOLINT(bugprone-macro-parentheses) */ = dyn;                   \
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto Name##_fn(          \
			dyn a, dyn b) noexcept->Name {                                           \
		return {i64(a) Op i64(b)};                                                 \
	}                                                                            \
	static_assert(true, "")

#define __VEG_CMP(Name, Op)                                                    \
	using Name /* NOLINT(bugprone-macro-parentheses) */ = boolean<maybe>;        \
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE static constexpr auto Name##_fn(          \
			dyn a, dyn b) noexcept->Name {                                           \
		return (i64(a) Op i64(b));                                                 \
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

	using div = dyn;
	using mod = dyn;

	VEG_NODISCARD static constexpr auto div_fn(dyn a, dyn b) noexcept -> div {
		return VEG_ASSERT(i64(b) != i64(0)), i64(a) / i64(b);
	}
	VEG_NODISCARD static constexpr auto mod_fn(dyn a, dyn b) noexcept -> mod {

		return VEG_ASSERT(i64(b) != i64(0)), i64(a) % i64(b);
	}

#undef __VEG_OP
#undef __VEG_CMP
};

template <i64 N>
struct binary_traits<fix<N>, dyn> : binary_traits<dyn, dyn> {
	template <i64 I>
	struct dims {
		HEDLEY_ALWAYS_INLINE constexpr dims(fix<N> /*r*/, dyn c) noexcept
				: m_cols(c) {}
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto nrows() const noexcept
				-> fix<N> {
			return {};
		}
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto ncols() const noexcept
				-> dyn {
			return m_cols;
		}
		dyn m_cols;
	};
};

template <>
struct binary_traits<fix<0>, dyn> : binary_traits<dyn, dyn> {
	template <i64 I>
	struct dims {
		HEDLEY_ALWAYS_INLINE constexpr dims(fix<0> /*r*/, dyn c) noexcept
				: m_cols(c) {}
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto nrows() const noexcept
				-> fix<0> {
			return {};
		}
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto ncols() const noexcept -> dyn {
			return m_cols;
		}
		dyn m_cols;
	};

	using mul = fix<0>;
	VEG_NODISCARD
	constexpr HEDLEY_ALWAYS_INLINE static auto
	mul_fn(fix<0> /*a*/, dyn /*b*/) noexcept -> mul {
		return {};
	}
};

template <i64 N>
struct binary_traits<dyn, fix<N>> : binary_traits<dyn, dyn> {
	template <i64 I>
	struct dims {
		HEDLEY_ALWAYS_INLINE constexpr dims(dyn r, fix<N> /*c*/) noexcept
				: m_rows(r) {}
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto nrows() const noexcept
				-> dyn {
			return m_rows;
		}
		VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto ncols() const noexcept
				-> fix<N> {
			return {};
		}
		dyn m_rows;
	};

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
	return fn::narrow<i64>{}(n);
}

} // namespace literals
} // namespace veg

#endif /* end of include guard __VEG_META_INT_DYN_HPP_GC385NKBS */

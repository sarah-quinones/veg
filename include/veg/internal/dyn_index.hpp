#ifndef VEG_META_INT_DYN_HPP_GC385NKBS
#define VEG_META_INT_DYN_HPP_GC385NKBS

#include "veg/util/assert.hpp"
#include "veg/internal/fix_index.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {

template <ternary_e T>
struct Boolean;

template <>
struct Boolean<maybe> {
	using type = maybe_c;

	constexpr Boolean() = default;
	constexpr Boolean /* NOLINT(hicpp-explicit-conversions) */ (bool _val)
			VEG_NOEXCEPT : val{_val} {}
	template <ternary_e T>
	VEG_INLINE constexpr Boolean /* NOLINT(hicpp-explicit-conversions)
	                              */
			(Boolean<T> /*arg*/) VEG_NOEXCEPT : val(T == yes) {}

	VEG_NODISCARD VEG_INLINE constexpr friend auto
	operator!(Boolean arg) VEG_NOEXCEPT -> Boolean {
		return {!arg.val};
	}
	VEG_NODISCARD VEG_INLINE explicit constexpr
	operator bool() const VEG_NOEXCEPT {
		return val;
	}

private:
	bool val = false;
};

struct Dyn {
	constexpr Dyn() = default;
	constexpr Dyn /* NOLINT(hicpp-explicit-conversions) */ (i64 val) VEG_NOEXCEPT
			: m_val(val) {}
	template <i64 N>
	constexpr Dyn /* NOLINT(hicpp-explicit-conversions) */ (Fix<N> /*arg*/)
			VEG_NOEXCEPT : m_val(N) {}

	VEG_NODISCARD VEG_INLINE explicit constexpr
	operator i64() const VEG_NOEXCEPT {
		return m_val;
	}
	VEG_NODISCARD VEG_INLINE constexpr auto operator+() const VEG_NOEXCEPT
			-> Dyn {
		return *this;
	}
	VEG_NODISCARD VEG_INLINE constexpr auto operator-() const VEG_NOEXCEPT
			-> Dyn {
		return Dyn{-m_val};
	}

	VEG_TEMPLATE(
			(typename R),
			requires(VEG_CONCEPT(index<R>)),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator+,
			(b, R))
	const VEG_DEDUCE_RET(internal::binary_traits<Dyn, R>::add_fn(*this, b));

	VEG_TEMPLATE(
			(typename R),
			requires(VEG_CONCEPT(index<R>)),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator-,
			(b, R))
	const VEG_DEDUCE_RET(internal::binary_traits<Dyn, R>::sub_fn(*this, b));

	VEG_TEMPLATE(
			(typename R),
			requires(VEG_CONCEPT(index<R>)),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator*,
			(b, R))
	const VEG_DEDUCE_RET(internal::binary_traits<Dyn, R>::mul_fn(*this, b));

	VEG_TEMPLATE(
			(typename R),
			requires(
					VEG_CONCEPT(index<R>) &&
					VEG_CONCEPT(index<typename internal::binary_traits<Dyn, R>::div>)),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator/,
			(b, R))
	const VEG_DEDUCE_RET(internal::binary_traits<Dyn, R>::div_fn(*this, b));

	VEG_TEMPLATE(
			(typename R),
			requires(
					VEG_CONCEPT(index<R>) &&
					VEG_CONCEPT(index<typename internal::binary_traits<Dyn, R>::mod>)),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator%,
			(b, R))
	const VEG_DEDUCE_RET(internal::binary_traits<Dyn, R>::mod_fn(*this, b));

#define VEG_CMP(Name, Op)                                                      \
	VEG_TEMPLATE(                                                                \
			(typename R),                                                            \
			requires(VEG_CONCEPT(index<R>)),                                         \
			VEG_NODISCARD VEG_INLINE constexpr auto                                  \
			operator Op,                                                             \
			(b, R))                                                                  \
	const VEG_DEDUCE_RET(                                                        \
			internal::binary_traits<Dyn, R>::cmp_##Name##_fn(*this, b))

	VEG_CMP(eq, ==);
	VEG_CMP(neq, !=);
	VEG_CMP(lt, <);
	VEG_CMP(le, <=);
	VEG_CMP(gt, >);
	VEG_CMP(ge, >=);

#undef VEG_CMP
private:
	i64 m_val = 0;
};

template <ternary_e T>
VEG_INLINE constexpr Boolean<T>::Boolean(Boolean<maybe> /*b*/, Unsafe /*tag*/)
		VEG_NOEXCEPT {}
template <ternary_e T>
VEG_INLINE constexpr Boolean<T>::Boolean // NOLINT(hicpp-explicit-conversions)
		(Boolean<maybe> b) VEG_NOEXCEPT
		: Boolean(
					((void)VEG_INTERNAL_ASSERT_PRECONDITION(b.val == (T == yes)), b),
					unsafe) {}

template <i64 N>
VEG_INLINE constexpr Fix<N>::Fix(Dyn /*arg*/, Unsafe /*tag*/) VEG_NOEXCEPT {}
template <i64 N>
VEG_INLINE constexpr Fix<N>::Fix // NOLINT(hicpp-explicit-conversions)
		(Dyn arg) VEG_NOEXCEPT
		: Fix((VEG_INTERNAL_ASSERT_PRECONDITION(i64(arg) == N), arg), unsafe) {}

namespace internal {

template <>
struct binary_traits<Dyn, Dyn> {
#define VEG_OP(Name, Op)                                                       \
	using Name /* NOLINT(bugprone-macro-parentheses) */ = Dyn;                   \
	VEG_NODISCARD VEG_INLINE static constexpr auto Name##_fn(Dyn a, Dyn b)       \
			VEG_NOEXCEPT->Name {                                                     \
		return {i64(a) Op i64(b)};                                                 \
	}                                                                            \
	static_assert(true, "")

#define VEG_CMP(Name, Op)                                                      \
	using Name /* NOLINT(bugprone-macro-parentheses) */ = Boolean<maybe>;        \
	VEG_NODISCARD VEG_INLINE static constexpr auto Name##_fn(Dyn a, Dyn b)       \
			VEG_NOEXCEPT->Name {                                                     \
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

	using div = Dyn;
	using mod = Dyn;

	VEG_NODISCARD static constexpr auto div_fn(Dyn a, Dyn b) VEG_NOEXCEPT -> div {
		return VEG_INTERNAL_ASSERT_PRECONDITION(i64(b) != i64(0)), i64(a) / i64(b);
	}
	VEG_NODISCARD static constexpr auto mod_fn(Dyn a, Dyn b) VEG_NOEXCEPT -> mod {

		return VEG_INTERNAL_ASSERT_PRECONDITION(i64(b) != i64(0)), i64(a) % i64(b);
	}

#undef VEG_OP
#undef VEG_CMP
};

template <i64 N>
struct binary_traits<Fix<N>, Dyn> : binary_traits<Dyn, Dyn> {};

template <>
struct binary_traits<Fix<0>, Dyn> : binary_traits<Dyn, Dyn> {
	using mul = Fix<0>;
	VEG_NODISCARD
	constexpr VEG_INLINE static auto mul_fn(Fix<0> /*a*/, Dyn /*b*/) VEG_NOEXCEPT
			-> mul {
		return {};
	}
};

template <i64 N>
struct binary_traits<Dyn, Fix<N>> : binary_traits<Dyn, Dyn> {
	using mul = typename binary_traits<Fix<N>, Dyn>::mul;
	VEG_INLINE static constexpr auto mul_fn(Dyn a, Fix<N> /*b*/) VEG_NOEXCEPT
			-> mul {
		return binary_traits<Fix<N>, Dyn>::mul_fn({}, a);
	}

	using div = meta::conditional_t<N == 0, void, Dyn>;
	using mod = meta::conditional_t<N == 0, void, Dyn>;

	VEG_NODISCARD VEG_INLINE static constexpr auto
	div_fn(Dyn a, Fix<N> /*b*/) VEG_NOEXCEPT -> div {
		return div(i64(a) / N);
	}
	VEG_NODISCARD VEG_INLINE static constexpr auto
	mod_fn(Dyn a, Fix<N> /*b*/) VEG_NOEXCEPT -> mod {
		return mod(i64(a) % N);
	}
};

} // namespace internal

inline namespace literals {
VEG_INLINE constexpr auto operator"" _v(unsigned long long n) VEG_NOEXCEPT
		-> Dyn {
	return nb::narrow<i64>{}(n);
}
} // namespace literals

namespace fmt {
template <>
struct Debug<Boolean<maybe>> {
	static void to_string(fmt::Buffer& out, Ref<Boolean<maybe>> val) {
		out.insert(out.size(), "maybe[", 6);
		Debug<bool>::to_string(out, ref(bool(val.get())));
		out.insert(out.size(), "]", 1);
	}
};

template <>
struct Debug<Dyn> {
	static void to_string(fmt::Buffer& out, Ref<Dyn> val) {
		out.insert(out.size(), "Dyn[", 4);
		Debug<i64>::to_string(out, ref(i64(val.get())));
		out.insert(out.size(), "]", 1);
	}
};
} // namespace fmt
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_META_INT_DYN_HPP_GC385NKBS */

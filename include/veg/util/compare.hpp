#ifndef VEG_CMP_HPP_6QBW4XNOS
#define VEG_CMP_HPP_6QBW4XNOS

#include "veg/type_traits/primitives.hpp"
#include "veg/type_traits/core.hpp"
#include "veg/internal/integer_seq.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {

namespace internal {
namespace meta_ {

template <typename A, typename B>
using cmp_eq = decltype(void(
		static_cast<bool>(VEG_DECLVAL(A&&) == VEG_DECLVAL(B &&)) &&
		static_cast<bool>(VEG_DECLVAL(B &&) == VEG_DECLVAL(A &&))));
template <typename A, typename B>
using cmp_ord = decltype(void(
		static_cast<bool>(VEG_DECLVAL(B&&) < VEG_DECLVAL(A &&)) &&
		static_cast<bool>(VEG_DECLVAL(B &&) > VEG_DECLVAL(A &&)) &&
		static_cast<bool>(VEG_DECLVAL(B &&) <= VEG_DECLVAL(A &&)) &&
		static_cast<bool>(VEG_DECLVAL(B &&) >= VEG_DECLVAL(A &&)) &&
		static_cast<bool>(VEG_DECLVAL(A &&) < VEG_DECLVAL(B &&)) &&
		static_cast<bool>(VEG_DECLVAL(A &&) > VEG_DECLVAL(B &&)) &&
		static_cast<bool>(VEG_DECLVAL(A &&) <= VEG_DECLVAL(B &&)) &&
		static_cast<bool>(VEG_DECLVAL(A &&) >= VEG_DECLVAL(B &&))));

} // namespace meta_
} // namespace internal

namespace concepts {
VEG_DEF_CONCEPT(
		(typename A, typename B),
		equality_comparable_with,
		VEG_CONCEPT(detected<
								internal::meta_::cmp_eq,
								uncvref_t<A> const&,
								uncvref_t<B> const&>));

VEG_DEF_CONCEPT(
		(typename A, typename B),
		partially_ordered_with,
		VEG_CONCEPT(detected<
								internal::meta_::cmp_ord,
								uncvref_t<A> const&,
								uncvref_t<B> const&>));

} // namespace concepts

namespace cmp {
struct strong_ordering;
struct weak_ordering;
struct partial_ordering;
} // namespace cmp

namespace internal {
namespace cmp {
struct aux_zero {};
template <typename Ptr = aux_zero*> // suppress clang-tidy warning
struct zero {
	HEDLEY_ALWAYS_INLINE constexpr zero(Ptr /*unused*/) noexcept {}
	VEG_TEMPLATE(
			(typename T),
			requires(VEG_CONCEPT(pointer<T>) || VEG_CONCEPT(same<T, std::nullptr_t>)),
			zero,
			(arg, T)) = delete;
};

enum struct values : signed char {
	unordered = -2,
	less = -1,
	equal = 0,
	equivalent = equal,
	greater = 1,
};

struct values_t {
	values value;
};

template <typename Ord>
struct ordering_impl {
	static Ord constexpr unordered = {values::unordered};
	static Ord constexpr less = {values::less};
	static Ord constexpr equal = {values::equal};
	static Ord constexpr equivalent = {values::equivalent};
	static Ord constexpr greater = {values::greater};
};

template <typename Ord>
Ord constexpr ordering_impl<Ord>::unordered;
template <typename Ord>
Ord constexpr ordering_impl<Ord>::less;
template <typename Ord>
Ord constexpr ordering_impl<Ord>::equal;
template <typename Ord>
Ord constexpr ordering_impl<Ord>::equivalent;
template <typename Ord>
Ord constexpr ordering_impl<Ord>::greater;

template <typename Ord>
struct ordering_impl_base {
	static Ord const& unordered;
	static Ord const& less;
	static Ord const& equal;
	static Ord const& equivalent;
	static Ord const& greater;
};
template <typename Ord>
constexpr Ord const& ordering_impl_base<Ord>::unordered =
		ordering_impl<Ord>::unordered;
template <typename Ord>
constexpr Ord const& ordering_impl_base<Ord>::less = ordering_impl<Ord>::less;
template <typename Ord>
constexpr Ord const& ordering_impl_base<Ord>::equal = ordering_impl<Ord>::equal;
template <typename Ord>
constexpr Ord const& ordering_impl_base<Ord>::equivalent =
		ordering_impl<Ord>::equivalent;
template <typename Ord>
constexpr Ord const& ordering_impl_base<Ord>::greater =
		ordering_impl<Ord>::greater;

struct cmp_base {
	friend HEDLEY_ALWAYS_INLINE constexpr auto
	operator==(internal::cmp::values_t lhs, internal::cmp::values_t rhs) noexcept
			-> bool {
		return lhs.value == rhs.value;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto
	operator!=(internal::cmp::values_t lhs, internal::cmp::values_t rhs) noexcept
			-> bool {
		return lhs.value != rhs.value;
	}

	friend HEDLEY_ALWAYS_INLINE constexpr auto operator==(
			internal::cmp::values_t lhs, internal::cmp::zero<> /*rhs*/) noexcept
			-> bool {
		return lhs.value == internal::cmp::values::equivalent;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator==(
			internal::cmp::zero<> /*lhs*/, internal::cmp::values_t rhs) noexcept
			-> bool {
		return rhs.value == internal::cmp::values::equivalent;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator!=(
			internal::cmp::values_t lhs, internal::cmp::zero<> /*rhs*/) noexcept
			-> bool {
		return lhs.value != internal::cmp::values::equivalent;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator!=(
			internal::cmp::zero<> /*lhs*/, internal::cmp::values_t rhs) noexcept
			-> bool {
		return rhs.value != internal::cmp::values::equivalent;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto
	operator<(internal::cmp::values_t lhs, internal::cmp::zero<> /*rhs*/) noexcept
			-> bool {
		return lhs.value == internal::cmp::values::less;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto
	operator<(internal::cmp::zero<> /*lhs*/, internal::cmp::values_t rhs) noexcept
			-> bool {
		return rhs.value == internal::cmp::values::greater;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto
	operator>(internal::cmp::values_t lhs, internal::cmp::zero<> /*rhs*/) noexcept
			-> bool {
		return lhs.value == internal::cmp::values::greater;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto
	operator>(internal::cmp::zero<> /*lhs*/, internal::cmp::values_t rhs) noexcept
			-> bool {
		return rhs.value == internal::cmp::values::less;
	}
};

struct cmp_base_partial : cmp_base {
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator<=(
			internal::cmp::values_t lhs, internal::cmp::zero<> /*rhs*/) noexcept
			-> bool {
		return lhs.value == internal::cmp::values::less ||
					 lhs.value == internal::cmp::values::equivalent;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator<=(
			internal::cmp::zero<> /*lhs*/, internal::cmp::values_t rhs) noexcept
			-> bool {
		return rhs.value == internal::cmp::values::greater ||
					 rhs.value == internal::cmp::values::equivalent;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator>=(
			internal::cmp::values_t lhs, internal::cmp::zero<> /*rhs*/) noexcept
			-> bool {
		return lhs.value == internal::cmp::values::greater ||
					 lhs.value == internal::cmp::values::equivalent;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator>=(
			internal::cmp::zero<> /*lhs*/, internal::cmp::values_t rhs) noexcept
			-> bool {
		return rhs.value == internal::cmp::values::less ||
					 rhs.value == internal::cmp::values::equivalent;
	}
};

namespace _ {
struct cmp_base_strong : cmp_base {
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator<=(
			internal::cmp::values_t lhs, internal::cmp::zero<> /*rhs*/) noexcept
			-> bool {
		return lhs.value != internal::cmp::values::greater;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator<=(
			internal::cmp::zero<> /*lhs*/, internal::cmp::values_t rhs) noexcept
			-> bool {
		return rhs.value != internal::cmp::values::less;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator>=(
			internal::cmp::values_t lhs, internal::cmp::zero<> /*rhs*/) noexcept
			-> bool {
		return lhs.value != internal::cmp::values::less;
	}
	friend HEDLEY_ALWAYS_INLINE constexpr auto operator>=(
			internal::cmp::zero<> /*lhs*/, internal::cmp::values_t rhs) noexcept
			-> bool {
		return rhs.value != internal::cmp::values::greater;
	}
};
} // namespace _

} // namespace cmp
} // namespace internal

namespace cmp {

struct partial_ordering
		: private internal::cmp::cmp_base_partial,
			private internal::cmp::ordering_impl_base<partial_ordering> {
	constexpr operator internal::cmp::values_t() const noexcept {
		return {value};
	}

private:
	friend struct internal::cmp::ordering_impl<partial_ordering>;
	friend struct weak_ordering;
	friend struct strong_ordering;
	using values = internal::cmp::values;
	values value;
	constexpr partial_ordering(values val) noexcept : value{val} {}

public:
	using ordering_impl_base::unordered;
	using ordering_impl_base::less;
	using ordering_impl_base::equivalent;
	using ordering_impl_base::greater;
};

struct weak_ordering
		: private internal::cmp::cmp_base_partial,
			private internal::cmp::ordering_impl_base<weak_ordering> {
	constexpr operator partial_ordering() const noexcept { return value; }
	constexpr operator internal::cmp::values_t() const noexcept {
		return {value};
	}

private:
	friend struct internal::cmp::ordering_impl<weak_ordering>;
	friend struct strong_ordering;
	using values = internal::cmp::values;
	values value;
	constexpr weak_ordering(values val) noexcept : value{val} {}

public:
	using ordering_impl_base::less;
	using ordering_impl_base::equivalent;
	using ordering_impl_base::greater;
};

struct strong_ordering
		: private internal::cmp::_::cmp_base_strong,
			private internal::cmp::ordering_impl_base<strong_ordering> {
	constexpr operator partial_ordering() const noexcept { return value; }
	constexpr operator weak_ordering() const noexcept { return value; }
	constexpr operator internal::cmp::values_t() const noexcept {
		return {value};
	}

private:
	friend struct internal::cmp::ordering_impl<strong_ordering>;
	using values = internal::cmp::values;
	values value;
	constexpr strong_ordering(values val) noexcept : value{val} {}

public:
	using ordering_impl_base::less;
	using ordering_impl_base::equivalent;
	using ordering_impl_base::equal;
	using ordering_impl_base::greater;
};

} // namespace cmp
namespace internal {
namespace cmp {
using ::veg::cmp::strong_ordering;
using ::veg::cmp::weak_ordering;
using ::veg::cmp::partial_ordering;

struct compare_three_way_generic {
	VEG_TEMPLATE(
			(typename L, typename R),
			requires(VEG_CONCEPT(partially_ordered_with<L, R>)),
			HEDLEY_ALWAYS_INLINE static constexpr auto fn,
			(lhs, L const&),
			(rhs, R const&))
	noexcept -> weak_ordering {
		return lhs < rhs                              //
							 ? weak_ordering::less              //
							 : lhs > rhs                        //
										 ? weak_ordering::greater     //
										 : weak_ordering::equivalent; //
	}
};

struct compare_three_way_signed_unsigned {
	template <typename L, typename R>
	HEDLEY_ALWAYS_INLINE static constexpr auto fn(L lhs, R rhs) noexcept
			-> strong_ordering {
		return ((lhs < 0) || static_cast<R>(lhs) < rhs) //
							 ? strong_ordering::less              //
							 : (static_cast<R>(lhs) == rhs)       //
										 ? strong_ordering::equal       //
										 : strong_ordering::greater;    //
	}
};

struct compare_three_way_unsigned_signed {
	template <typename L, typename R>
	HEDLEY_ALWAYS_INLINE static constexpr auto fn(L lhs, R rhs) noexcept
			-> strong_ordering {
		return ((rhs < 0) || static_cast<L>(rhs) < lhs) //
							 ? strong_ordering::greater           //
							 : (static_cast<L>(rhs) == lhs)       //
										 ? strong_ordering::equal       //
										 : strong_ordering::less;       //
	}
};

struct compare_three_way_common_type_int {
	template <typename L, typename R>
	HEDLEY_ALWAYS_INLINE static constexpr auto fn(L lhs, R rhs) noexcept
			-> strong_ordering {
		using common_type = decltype(lhs + rhs);
		return common_type(lhs) < common_type(rhs)       //
							 ? strong_ordering::less               //
							 : common_type(lhs) > common_type(rhs) //
										 ? strong_ordering::greater      //
										 : strong_ordering::equivalent;  //
	}
};

struct compare_three_way_common_type_float {
	template <typename L, typename R>
	HEDLEY_ALWAYS_INLINE static constexpr auto fn(L lhs, R rhs) noexcept
			-> partial_ordering {
		using common_type = decltype(lhs + rhs);
		return common_type(lhs) == common_type(rhs) //
							 ? partial_ordering::equivalent
							 : common_type(lhs) < common_type(rhs)       //
										 ? partial_ordering::less              //
										 : common_type(lhs) > common_type(rhs) //
													 ? partial_ordering::greater     //
													 : partial_ordering::unordered;  //
	}
};

template <typename A, typename B>
using cmp_3way_impl = meta::conditional_t< //
		(VEG_CONCEPT(integral<A>) && VEG_CONCEPT(integral<B>)),
		meta::conditional_t<
				VEG_CONCEPT(signed_integral<A>) && VEG_CONCEPT(unsigned_integral<B>) &&
						(sizeof(A) == sizeof(B)),
				compare_three_way_signed_unsigned,
				meta::conditional_t<
						VEG_CONCEPT(unsigned_integral<A>) &&
								VEG_CONCEPT(signed_integral<B>) && (sizeof(A) == sizeof(B)),
						compare_three_way_unsigned_signed,
						compare_three_way_common_type_int>>,

		meta::conditional_t<
				VEG_CONCEPT(arithmetic<A>) && VEG_CONCEPT(arithmetic<B>) &&
						(VEG_CONCEPT(floating_point<A>) || VEG_CONCEPT(floating_point<B>)),
				compare_three_way_common_type_float,
				void>>;

enum struct which {
	int_signed_unsigned,
	int_unsigned_signed,
	to_common_type,
	generic,
};

template <which Signedness /* same_sign */>
struct cmp_impl {
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto eq(A const& a, B const& b) noexcept
			-> bool {
		return static_cast<bool>(a == b);
	}
};

template <>
struct cmp_impl<which::to_common_type> {
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto eq(A a, B b) noexcept -> bool {
		using common = decltype(a + b);
		return static_cast<common>(a) == static_cast<common>(b);
	}
};

template <>
struct cmp_impl<which::int_signed_unsigned> {
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto eq(A a, B b) noexcept -> bool {
		return (a >= 0) && (static_cast<B>(a) == b);
	}
};

template <>
struct cmp_impl<which::int_unsigned_signed> {
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto eq(A a, B b) noexcept -> bool {
		return cmp_impl<which::int_signed_unsigned>::eq(b, a);
	}
};

template <typename T>
using pre_sizeof_ = meta::constant<usize, sizeof(T)>;
template <typename T>
using sizeof_ = meta::detected_or_t<meta::constant<usize, 0>, pre_sizeof_, T>;

template <typename A, typename B>
using same_size = meta::bool_constant<sizeof_<A>::value == sizeof_<B>::value>;

template <typename A, typename B>
using cmp_impl_ = internal::cmp::cmp_impl<
		(VEG_CONCEPT(signed_integral<A>) &&                                    //
     VEG_CONCEPT(unsigned_integral<B>) &&                                  //
     (sizeof(A) == sizeof(B)))                                             //
				? which::int_signed_unsigned                                       //
				: (VEG_CONCEPT(unsigned_integral<A>) &&                            //
           VEG_CONCEPT(signed_integral<B>) &&                              //
           (sizeof(A) == sizeof(B)))                                       //
							? which::int_unsigned_signed                                 //
							: (VEG_CONCEPT(arithmetic<A>) && VEG_CONCEPT(arithmetic<B>)) //
										? which::to_common_type                                //
										: which::generic>;

namespace builtin_ops {
#define VEG_CMP_FLT(A, B)                                                      \
	HEDLEY_ALWAYS_INLINE constexpr auto cmp_3way(                                \
			A a, B b) noexcept->partial_ordering {                                   \
		using T = decltype(a + b);                                                 \
		return compare_three_way_common_type_float::fn(T(a), T(b));                \
	}                                                                            \
	VEG_NOM_SEMICOLON

#define VEG_CMP_INT(A, B)                                                      \
	HEDLEY_ALWAYS_INLINE constexpr auto cmp_3way(                                \
			A a, B b) noexcept->strong_ordering {                                    \
		using T = decltype(a + b);                                                 \
		return compare_three_way_common_type_int::fn(T(a), T(b));                  \
	}                                                                            \
	VEG_NOM_SEMICOLON

#define VEG_CMP_SIGN_UNSIGN(A, B)                                              \
	HEDLEY_ALWAYS_INLINE constexpr auto cmp_3way(                                \
			A a, B b) noexcept->strong_ordering {                                    \
		using T = decltype(a + b);                                                 \
		return meta::conditional_t<                                                \
				sizeof(A) == sizeof(B),                                                \
				compare_three_way_signed_unsigned,                                     \
				compare_three_way_common_type_int>::fn(T(a), T(b));                    \
	}                                                                            \
	VEG_NOM_SEMICOLON

#define VEG_CMP_UNSIGN_SIGN(A, B)                                              \
	HEDLEY_ALWAYS_INLINE constexpr auto cmp_3way(                                \
			A a, B b) noexcept->strong_ordering {                                    \
		using T = decltype(a + b);                                                 \
		return meta::conditional_t<                                                \
				sizeof(A) == sizeof(B),                                                \
				compare_three_way_unsigned_signed,                                     \
				compare_three_way_common_type_int>::fn(T(a), T(b));                    \
	}                                                                            \
	VEG_NOM_SEMICOLON

VEG_CMP_FLT(float, float);
VEG_CMP_FLT(float, double);
VEG_CMP_FLT(float, long double);
VEG_CMP_FLT(float, int);
VEG_CMP_FLT(float, long);
VEG_CMP_FLT(float, long long);
VEG_CMP_FLT(float, unsigned int);
VEG_CMP_FLT(float, unsigned long);
VEG_CMP_FLT(float, unsigned long long);

VEG_CMP_FLT(double, float);
VEG_CMP_FLT(long double, float);
VEG_CMP_FLT(int, float);
VEG_CMP_FLT(long, float);
VEG_CMP_FLT(long long, float);
VEG_CMP_FLT(unsigned int, float);
VEG_CMP_FLT(unsigned long, float);
VEG_CMP_FLT(unsigned long long, float);

VEG_CMP_FLT(double, double);
VEG_CMP_FLT(double, long double);
VEG_CMP_FLT(double, int);
VEG_CMP_FLT(double, long);
VEG_CMP_FLT(double, long long);
VEG_CMP_FLT(double, unsigned int);
VEG_CMP_FLT(double, unsigned long);
VEG_CMP_FLT(double, unsigned long long);

VEG_CMP_FLT(long double, double);
VEG_CMP_FLT(int, double);
VEG_CMP_FLT(long, double);
VEG_CMP_FLT(long long, double);
VEG_CMP_FLT(unsigned int, double);
VEG_CMP_FLT(unsigned long, double);
VEG_CMP_FLT(unsigned long long, double);

VEG_CMP_FLT(long double, long double);
VEG_CMP_FLT(long double, int);
VEG_CMP_FLT(long double, long);
VEG_CMP_FLT(long double, long long);
VEG_CMP_FLT(long double, unsigned int);
VEG_CMP_FLT(long double, unsigned long);
VEG_CMP_FLT(long double, unsigned long long);

VEG_CMP_FLT(int, long double);
VEG_CMP_FLT(long, long double);
VEG_CMP_FLT(long long, long double);
VEG_CMP_FLT(unsigned int, long double);
VEG_CMP_FLT(unsigned long, long double);
VEG_CMP_FLT(unsigned long long, long double);

VEG_CMP_INT(int, int);
VEG_CMP_INT(int, long);
VEG_CMP_INT(int, long long);
VEG_CMP_INT(long, int);
VEG_CMP_INT(long long, int);

VEG_CMP_INT(long, long);
VEG_CMP_INT(long, long long);
VEG_CMP_INT(long long, long);

VEG_CMP_INT(long long, long long);

VEG_CMP_INT(unsigned int, unsigned int);
VEG_CMP_INT(unsigned int, unsigned long);
VEG_CMP_INT(unsigned int, unsigned long long);
VEG_CMP_INT(unsigned long, unsigned int);
VEG_CMP_INT(unsigned long long, unsigned int);

VEG_CMP_INT(unsigned long, unsigned long);
VEG_CMP_INT(unsigned long, unsigned long long);
VEG_CMP_INT(unsigned long long, unsigned long);

VEG_CMP_INT(unsigned long long, unsigned long long);

VEG_CMP_SIGN_UNSIGN(int, unsigned int);
VEG_CMP_SIGN_UNSIGN(int, unsigned long);
VEG_CMP_SIGN_UNSIGN(int, unsigned long long);

VEG_CMP_SIGN_UNSIGN(long, unsigned int);
VEG_CMP_SIGN_UNSIGN(long, unsigned long);
VEG_CMP_SIGN_UNSIGN(long, unsigned long long);

VEG_CMP_SIGN_UNSIGN(long long, unsigned int);
VEG_CMP_SIGN_UNSIGN(long long, unsigned long);
VEG_CMP_SIGN_UNSIGN(long long, unsigned long long);

VEG_CMP_UNSIGN_SIGN(unsigned int, int);
VEG_CMP_UNSIGN_SIGN(unsigned long, int);
VEG_CMP_UNSIGN_SIGN(unsigned long long, int);

VEG_CMP_UNSIGN_SIGN(unsigned int, long);
VEG_CMP_UNSIGN_SIGN(unsigned long, long);
VEG_CMP_UNSIGN_SIGN(unsigned long long, long);

VEG_CMP_UNSIGN_SIGN(unsigned int, long long);
VEG_CMP_UNSIGN_SIGN(unsigned long, long long);
VEG_CMP_UNSIGN_SIGN(unsigned long long, long long);

#undef VEG_CMP_SIGN_UNSIGN
#undef VEG_CMP_UNSIGN_SIGN
#undef VEG_CMP_FLT
#undef VEG_CMP_INT

template <typename T>
HEDLEY_ALWAYS_INLINE constexpr auto cmp_3way(T const* a, T const* b)
		-> strong_ordering {
	return a == b //
						 ? strong_ordering::equal
						 : a < b //
									 ? strong_ordering::less
									 : strong_ordering::greater;
}

template <typename A, typename B>
using threeway_result =
		decltype(cmp_3way(VEG_DECLVAL(A const&), VEG_DECLVAL(B const&)));

struct adl_3way {
	template <typename A, typename B>
	HEDLEY_ALWAYS_INLINE static constexpr auto fn(A const& a, B const& b)
			VEG_DEDUCE_RET(cmp_3way(VEG_FWD(a), VEG_FWD(b)));
};
} // namespace builtin_ops

} // namespace cmp
} // namespace internal

namespace meta {

template <typename A, typename B>
using three_way_comparison_result_t = detected_t<
		veg::internal::cmp::builtin_ops::threeway_result,
		A const&,
		B const&>;

} // namespace meta
namespace internal {
namespace meta_ {
template <typename C>
using test_ordering = decltype(
		static_cast<bool>(VEG_DECLVAL(C) < 0) &&
		static_cast<bool>(VEG_DECLVAL(C) > 0) &&
		static_cast<bool>(VEG_DECLVAL(C) <= 0) &&
		static_cast<bool>(VEG_DECLVAL(C) >= 0) &&
		static_cast<bool>(VEG_DECLVAL(C) != 0) &&
		static_cast<bool>(VEG_DECLVAL(C) == 0));

template <typename A, typename B>
using threeway_result_test =
		test_ordering<veg::internal::cmp::builtin_ops::threeway_result<A, B>>;

template <typename T>
using less_t =
		enable_if_t<VEG_CONCEPT(same<decltype(T::less) const&, T const&>)>;
template <typename T>
using unordered_t =
		enable_if_t<VEG_CONCEPT(same<decltype(T::unordered) const&, T const&>)>;
template <typename T>
using equal_t =
		enable_if_t<VEG_CONCEPT(same<decltype(T::equal) const&, T const&>)>;
template <typename T>
using equivalent_t =
		enable_if_t<VEG_CONCEPT(same<decltype(T::equivalent) const&, T const&>)>;
template <typename T>
using greater_t =
		enable_if_t<VEG_CONCEPT(same<decltype(T::greater) const&, T const&>)>;

template <typename T>
struct is_partial_ordering : false_type {};
template <>
struct is_partial_ordering<cmp::strong_ordering> : true_type {};
template <>
struct is_partial_ordering<cmp::weak_ordering> : true_type {};
template <>
struct is_partial_ordering<cmp::partial_ordering> : true_type {};

template <typename T>
struct is_weak_ordering : false_type {};
template <>
struct is_weak_ordering<cmp::strong_ordering> : true_type {};
template <>
struct is_weak_ordering<cmp::weak_ordering> : true_type {};

template <typename T>
struct is_strong_ordering : false_type {};
template <>
struct is_strong_ordering<cmp::strong_ordering> : true_type {};
} // namespace meta_
} // namespace internal
namespace concepts {

namespace aux {
VEG_DEF_CONCEPT(
		typename R,
		ordering_fallback,
		VEG_CONCEPT(detected<internal::meta_::test_ordering, R>));

VEG_DEF_CONCEPT_CONJUNCTION(
		(typename R),
		partial_ordering_fallback,
		((, ordering_fallback<R>),
     (concepts::, detected<internal::meta_::unordered_t, R>),
     (concepts::, detected<internal::meta_::less_t, R>),
     (concepts::, detected<internal::meta_::equivalent_t, R>),
     (concepts::, detected<internal::meta_::greater_t, R>)));

VEG_DEF_CONCEPT(
		(typename R),
		partial_ordering_shortcut,
		internal::meta_::is_partial_ordering<R>::value);
} // namespace aux

VEG_DEF_CONCEPT_DISJUNCTION(
		typename R,
		partial_ordering,
		((aux::, partial_ordering_shortcut<R>),
     (aux::, partial_ordering_fallback<R>)));

VEG_DEF_CONCEPT(
		(typename A, typename B),
		three_way_comparable_with,
		VEG_CONCEPT(partial_ordering<meta::detected_t<
										internal::cmp::builtin_ops::threeway_result,
										uncvref_t<A> const&,
										uncvref_t<B> const&>>));

} // namespace concepts

namespace internal {
namespace meta_ {

template <usize I, typename T>
using ignore_I = T;

template <typename Seq>
struct common_comparison_category_impl;

template <usize... Is>
struct common_comparison_category_impl<index_sequence<Is...>> {
	static auto apply(ignore_I<Is, cmp::strong_ordering>...)
			-> cmp::strong_ordering;
	static auto apply(ignore_I<Is, cmp::partial_ordering>...)
			-> cmp::partial_ordering;
	static auto apply(ignore_I<Is, cmp::weak_ordering>...) -> cmp::weak_ordering;
	static auto apply(...) -> void;
};
} // namespace meta_
} // namespace internal
namespace meta {

template <typename... Ts>
struct common_comparison_category
		: type_identity<decltype(
					internal::meta_::common_comparison_category_impl<
							make_index_sequence<sizeof...(Ts)>>::apply(VEG_DECLVAL(Ts)...))> {
};

template <>
struct common_comparison_category<> : type_identity<cmp::strong_ordering> {};

template <typename... Ts>
using common_comparison_category_t =
		typename common_comparison_category<Ts...>::type;

} // namespace meta
namespace cmp {
namespace nb {

struct three_way {
	VEG_TEMPLATE(
			(typename A, typename B),
			requires(VEG_CONCEPT(three_way_comparable_with<A, B>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(a, A const&),
			(b, B const&))
	const noexcept->meta::three_way_comparison_result_t<A, B> {
		using internal::cmp::builtin_ops::cmp_3way;
		return cmp_3way(a, b);
	}
};

struct synth_three_way {

	VEG_TEMPLATE(
			(typename A, typename B),
			requires(VEG_CONCEPT(three_way_comparable_with<A, B>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(a, A const&),
			(b, B const&))
	const noexcept->meta::three_way_comparison_result_t<A, B> {
		using internal::cmp::builtin_ops::cmp_3way;
		return cmp_3way(a, b);
	}

	VEG_TEMPLATE(
			(typename A, typename B),
			requires(
					!VEG_CONCEPT(three_way_comparable_with<A, B>) &&
					VEG_CONCEPT(partially_ordered_with<A, B>)),
			constexpr auto
			operator(),
			(a, A const&),
			(b, B const&))
	const noexcept->cmp::weak_ordering {
		return a < b                                  //
							 ? weak_ordering::less              //
							 : a > b                            //
										 ? weak_ordering::greater     //
										 : weak_ordering::equivalent; //
	}
};

struct equal {
	VEG_TEMPLATE(
			(typename A, typename B),
			requires(VEG_CONCEPT(equality_comparable_with<A, B>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(a, A&&),
			(b, B&&))
	const noexcept->bool {
		return internal::cmp::cmp_impl_<A&&, B&&>::eq(VEG_FWD(a), VEG_FWD(b));
	}
};
} // namespace nb
VEG_NIEBLOID(three_way);
VEG_NIEBLOID(synth_three_way);
VEG_NIEBLOID(equal);
} // namespace cmp

namespace concepts {

VEG_DEF_CONCEPT_DISJUNCTION(
		(typename A, typename B),
		synth_three_way_comparable_with,
		((, three_way_comparable_with<A, B>), (, partially_ordered_with<A, B>)));

} // namespace concepts
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_CMP_HPP_6QBW4XNOS */

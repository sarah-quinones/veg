#ifndef VEG_CMP_HPP_6QBW4XNOS
#define VEG_CMP_HPP_6QBW4XNOS

#include "veg/type_traits/primitives.hpp"
#include "veg/type_traits/core.hpp"
#include "veg/internal/integer_seq.hpp"
#include "veg/ref.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {

namespace cmp {
enum struct Ordering : unsigned char {
	equal = 0,
	less = 1,
	greater = 2,
	unordered = 3,
};

template <typename T, typename U>
struct PartialEq;
template <typename T, typename U>
struct PartialOrd;

template <typename T>
struct is_eq : meta::false_type {};
template <typename T>
struct is_ord : meta::false_type {};

#define VEG_ORD_SPEC(Type)                                                     \
	template <>                                                                  \
	struct is_eq<Type> : meta::true_type {};                                     \
	template <>                                                                  \
	struct is_ord<Type> : meta::true_type {}

VEG_ORD_SPEC(char);
VEG_ORD_SPEC(signed char);
VEG_ORD_SPEC(unsigned char);
VEG_ORD_SPEC(signed short);
VEG_ORD_SPEC(unsigned short);
VEG_ORD_SPEC(signed int);
VEG_ORD_SPEC(unsigned int);
VEG_ORD_SPEC(signed long);
VEG_ORD_SPEC(unsigned long);
VEG_ORD_SPEC(signed long long);
VEG_ORD_SPEC(unsigned long long);

template <typename T>
struct is_eq<T*> {};
template <typename T>
struct is_ord<T*> {};

template <typename Ret, typename... Args>
struct is_eq<auto(Args...)->Ret> {};
template <typename Ret, typename... Args>
struct is_eq<auto (*)(Args...)->Ret> {};
#if __cplusplus >= 201703L
template <typename Ret, typename... Args>
struct is_eq<auto(Args...) noexcept->Ret> {};
template <typename Ret, typename... Args>
struct is_eq<auto (*)(Args...) noexcept->Ret> {};
#endif

#undef VEG_ORD_SPEC

} // namespace cmp

namespace internal {
namespace meta_ {

template <typename A, typename B>
using cmp_eq = decltype(Ref<bool>{
		as_ref,
		cmp::PartialEq<A, B>::eq(VEG_DECLVAL(Ref<A>), VEG_DECLVAL(Ref<B>)),
});
template <typename A, typename B>
using cmp_ord = decltype(Ref<cmp::Ordering>{
		as_ref,
		cmp::PartialOrd<A, B>::cmp(VEG_DECLVAL(Ref<A>), VEG_DECLVAL(Ref<B>)),
});

template <typename A, typename B>
using legacy_cmp_eq =
		decltype(void(static_cast<bool>(VEG_DECLVAL(A&&) == VEG_DECLVAL(B &&))));
template <typename A, typename B>
using legacy_cmp_ord =
		decltype(void(static_cast<bool>(VEG_DECLVAL(A&&) < VEG_DECLVAL(B &&))));

} // namespace meta_
} // namespace internal

namespace concepts {
VEG_DEF_CONCEPT(
		(typename A, typename B),
		legacy_equality_comparable_with,
		VEG_CONCEPT(detected<internal::meta_::legacy_cmp_eq, A const&, B const&>));

VEG_DEF_CONCEPT(
		(typename A, typename B),
		legacy_partially_ordered_with,
		VEG_CONCEPT(detected<internal::meta_::legacy_cmp_ord, A const&, B const&>));

VEG_DEF_CONCEPT(
		(typename A, typename B),
		partial_eq,
		VEG_CONCEPT(detected<internal::meta_::cmp_eq, A, B>));

VEG_DEF_CONCEPT(
		(typename A, typename B),
		partial_ord,
		VEG_CONCEPT(detected<internal::meta_::cmp_ord, A, B>));
} // namespace concepts

namespace internal {
namespace _cmp {
struct PartialEqBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(legacy_equality_comparable_with<T, U>)),
			VEG_INLINE static constexpr auto eq,
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	VEG_NOEXCEPT->bool { return static_cast<bool>(lhs.get() == rhs.get()); }
};
struct PartialOrdBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(
					VEG_CONCEPT(legacy_partially_ordered_with<T, U>) &&
					VEG_CONCEPT(legacy_equality_comparable_with<T, U>)),
			VEG_INLINE static constexpr auto cmp,
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	VEG_NOEXCEPT->cmp::Ordering {
		return static_cast<bool>(lhs.get() == rhs.get())            //
		           ? cmp::Ordering::equal                           //
		           : static_cast<bool>(lhs.get() < rhs.get())       //
		                 ? cmp::Ordering::less                      //
		                 : static_cast<bool>(lhs.get() > rhs.get()) //
		                       ? cmp::Ordering::greater             //
		                       : cmp::Ordering::unordered;          //
	}
};

struct PartialEqRefBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_eq<T, U>)),
			VEG_INLINE static constexpr auto eq,
			(lhs, Ref<Ref<T>>),
			(rhs, Ref<Ref<U>>))
	VEG_NOEXCEPT->bool { return cmp::PartialEq<T, U>::eq(lhs.get(), rhs.get()); }
};
struct PartialOrdRefBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_eq<T, U>)),
			VEG_INLINE static constexpr auto cmp,
			(lhs, Ref<Ref<T>>),
			(rhs, Ref<Ref<U>>))
	VEG_NOEXCEPT->cmp::Ordering {
		return cmp::PartialEq<T, U>::cmp(lhs.get(), rhs.get());
	}
};

struct PartialEqRefMutBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_eq<T, U>)),
			VEG_INLINE static constexpr auto eq,
			(lhs, Ref<RefMut<T>>),
			(rhs, Ref<RefMut<U>>))
	VEG_NOEXCEPT->bool {
		return cmp::PartialEq<T, U>::eq( //
				lhs.get().as_const(),
				rhs.get().as_const());
	}
};
struct PartialOrdRefMutBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_eq<T, U>)),
			VEG_INLINE static constexpr auto cmp,
			(lhs, Ref<Ref<T>>),
			(rhs, Ref<Ref<U>>))
	VEG_NOEXCEPT->cmp::Ordering {
		return cmp::PartialEq<T, U>::cmp( //
				lhs.get().as_const(),
				rhs.get().as_const());
	}
};
} // namespace _cmp
} // namespace internal

namespace cmp {
template <typename T, typename U>
struct PartialEq : internal::_cmp::PartialEqBase {};

template <typename T, typename U>
struct PartialOrd : internal::_cmp::PartialOrdBase {};

template <typename T, typename U>
struct PartialEq<Ref<T>, Ref<U>> : internal::_cmp::PartialEqRefBase {};
template <typename T, typename U>
struct PartialOrd<Ref<T>, Ref<U>> : internal::_cmp::PartialOrdRefBase {};
template <typename T>
struct is_eq<Ref<T>> : is_eq<T> {};
template <typename T>
struct is_ord<Ref<T>> : is_ord<T> {};

template <typename T, typename U>
struct PartialEq<RefMut<T>, RefMut<U>> : internal::_cmp::PartialEqRefMutBase {};
template <typename T, typename U>
struct PartialOrd<RefMut<T>, RefMut<U>> : internal::_cmp::PartialOrdRefMutBase {
};
template <typename T>
struct is_eq<RefMut<T>> : is_eq<T> {};
template <typename T>
struct is_ord<RefMut<T>> : is_ord<T> {};

namespace nb {
struct eq {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_eq<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT->bool { return PartialEq<T, U>::eq(lhs, rhs); }
};
struct ne {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_eq<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT->bool { return !PartialEq<T, U>::eq(lhs, rhs); }
};

struct lt {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT->bool {
		return PartialOrd<T, U>::cmp(lhs, rhs) == Ordering::less;
	}
};
struct gt {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT->bool {
		return PartialOrd<T, U>::cmp(lhs, rhs) == Ordering::greater;
	}
};
struct leq {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT->bool {
		return unsigned(PartialOrd<T, U>::cmp(lhs, rhs)) < 2U;
	}
};
struct geq {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT->bool {
		return unsigned(PartialOrd<T, U>::cmp(lhs, rhs)) % 2U == 0;
	}
};

struct cmp {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(partial_ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT->Ordering { return PartialOrd<T, U>::cmp(lhs, rhs); }
};
} // namespace nb
VEG_NIEBLOID(eq);
VEG_NIEBLOID(ne);
VEG_NIEBLOID(lt);
VEG_NIEBLOID(gt);
VEG_NIEBLOID(leq);
VEG_NIEBLOID(geq);
VEG_NIEBLOID(cmp);
} // namespace cmp
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_CMP_HPP_6QBW4XNOS */

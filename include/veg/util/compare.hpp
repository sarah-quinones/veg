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
struct Ord;

} // namespace cmp

namespace concepts {
VEG_CONCEPT_EXPR(
		(typename T, typename U),
		(T, U),
		legacy_ord,
		VEG_DECLVAL(T const&) < VEG_DECLVAL(U const&),
		VEG_CONCEPT(constructible<bool, ExprType>));

VEG_CONCEPT_EXPR(
		(typename T, typename U),
		(T, U),
		eq,
		VEG_DECLVAL(T const&) == VEG_DECLVAL(U const&),
		VEG_CONCEPT(constructible<bool, ExprType>));

VEG_CONCEPT_EXPR(
		(typename T, typename U),
		(T, U),
		ord,
		(cmp::Ord<T, U>::cmp(VEG_DECLVAL(Ref<T>), VEG_DECLVAL(Ref<U>))),
		VEG_CONCEPT(constructible<cmp::Ordering, ExprType>));
} // namespace concepts

namespace cmp {
namespace ref {
VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(eq<T, U>)),
		VEG_INLINE constexpr auto
		operator==,
		(lhs, Ref<T>),
		(rhs, Ref<U>))
VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<T, U>))->bool {
	return lhs.get() == rhs.get();
}
} // namespace ref
namespace mut {
VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(eq<T, U>)),
		VEG_INLINE constexpr auto
		operator==,
		(lhs, RefMut<T>),
		(rhs, RefMut<U>))
VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<T, U>))->cmp::Ordering {
	return static_cast<T const&>(lhs.get()) == static_cast<U const&>(rhs.get());
}
} // namespace mut
} // namespace cmp

namespace internal {
namespace _cmp {
struct OrdBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(legacy_ord<T, U>) && VEG_CONCEPT(eq<T, U>)),
			VEG_INLINE static constexpr auto cmp,
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_legacy_ord<T, U>) && VEG_CONCEPT(nothrow_eq<T, U>))
			->cmp::Ordering {
		return static_cast<bool>(lhs.get() == rhs.get())            //
		           ? cmp::Ordering::equal                           //
		           : static_cast<bool>(lhs.get() < rhs.get())       //
		                 ? cmp::Ordering::less                      //
		                 : static_cast<bool>(lhs.get() > rhs.get()) //
		                       ? cmp::Ordering::greater             //
		                       : cmp::Ordering::unordered;          //
	}
};

struct OrdRefBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_INLINE static constexpr auto cmp,
			(lhs, Ref<Ref<T>>),
			(rhs, Ref<Ref<U>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->cmp::Ordering {
		return cmp::Ord<T, U>::cmp(lhs.get(), rhs.get());
	}
};

struct OrdRefMutBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_INLINE static constexpr auto cmp,
			(lhs, Ref<Ref<T>>),
			(rhs, Ref<Ref<U>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->cmp::Ordering {
		return cmp::Ord<T, U>::cmp( //
				lhs.get().as_const(),
				rhs.get().as_const());
	}
};
} // namespace _cmp
} // namespace internal

namespace cmp {
template <typename T, typename U>
struct Ord : internal::_cmp::OrdBase {};
template <typename T, typename U>
struct Ord<Ref<T>, Ref<U>> : internal::_cmp::OrdRefBase {};
template <typename T, typename U>
struct Ord<RefMut<T>, RefMut<U>> : internal::_cmp::OrdRefMutBase {};

namespace nb {
struct eq {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(eq<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<T, U>))->bool {
		return lhs.get() == rhs.get();
	}
};
struct ne {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(eq<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<T, U>))->bool {
		return !eq{}(lhs, rhs);
	}
};

struct lt {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->bool {
		return static_cast<Ordering>(Ord<T, U>::cmp(lhs, rhs)) == Ordering::less;
	}
};
struct gt {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->bool {
		return static_cast<Ordering>(Ord<T, U>::cmp(lhs, rhs)) == Ordering::greater;
	}
};
struct leq {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->bool {
		return unsigned(static_cast<Ordering>(Ord<T, U>::cmp(lhs, rhs))) < 2U;
	}
};
struct geq {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->bool {
		return unsigned(static_cast<Ordering>(Ord<T, U>::cmp(lhs, rhs))) % 2U == 0;
	}
};

struct cmp {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_INLINE constexpr auto
			operator(),
			(lhs, Ref<T>),
			(rhs, Ref<U>))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->Ordering {
		return static_cast<Ordering>(Ord<T, U>::cmp(lhs, rhs));
	}
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

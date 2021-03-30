#ifndef VEG_TUPLE_HPP_B8PHUNWES
#define VEG_TUPLE_HPP_B8PHUNWES

#include "veg/internal/tuple_generic.hpp"
#include "veg/internal/fmt.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/util/get.hpp"
#include "veg/internal/prologue.hpp"

// STD INCLUDE: std::tuple_{size,element}
#include <utility>

/******************************************************************************/
#define __VEG_IMPL_BIND(I, Tuple, Identifier)                                  \
	auto&& Identifier = ::veg::nb::get<I>{}(VEG_FWD(Tuple));

#define __VEG_IMPL_BIND_ID_SEQ(                                                \
		CV_Auto, Identifiers, Tuple, Tuple_Size, TupleId)                          \
	CV_Auto TupleId = Tuple;                                                     \
	static_assert(                                                               \
			::std::tuple_size<                                                       \
					typename ::veg::meta::uncvref_t<decltype(TupleId)>>::value ==        \
					Tuple_Size,                                                          \
			"wrong number of identifiers");                                          \
	__VEG_PP_TUPLE_FOR_EACH_I(__VEG_IMPL_BIND, TupleId, Identifiers)             \
	VEG_NOM_SEMICOLON

// example: difference vs c++17 structure bindings
// auto get() -> tuple<A, B&, C&&>;
//
// auto [a, b, c] = get();
// VEG_BIND(auto, (x, y, z), get());
// decltype({a,b,c}) => {A,B&,C&&}     same as tuple_element<i, E>
// decltype({x,y,z}) => {A&&,B&,C&&}   always a reference, lvalue if initializer
//                                     expression or tuple_element<i, E> is an
//                                     lvalue, rvalue otherwise.
//
#define VEG_BIND(CV_Auto, Identifiers, Tuple)                                  \
	__VEG_IMPL_BIND_ID_SEQ(                                                      \
			CV_Auto,                                                                 \
			Identifiers,                                                             \
			Tuple,                                                                   \
			__VEG_PP_TUPLE_SIZE(Identifiers),                                        \
			__VEG_PP_CAT(_dummy_tuple_variable_id_, __LINE__))
/******************************************************************************/

namespace veg {
inline namespace VEG_ABI {

VEG_DOC_CLASS
template <typename... Ts>
struct tuple : internal::tup_::adl::tuple_base<Ts...> {

	VEG_DOC_CTOR
	/// default constructor
	/// default or value initializes all the elements
	constexpr tuple() VEG_DOC(noexcept(nothrow_constructible<Ts>&&...) requires(
			constructible<Ts>&&...)) = default;

	VEG_DOC_CTOR
	/// move constructor
	/// move (or copy if the member can't be moved) constructs the elements
	/// memberwise
	tuple(tuple&&) VEG_DOC(noexcept(nothrow_move_constructible<Ts>&&...) requires(
			(move_constructible<Ts> || copy_constructible<Ts>)&&...)) = default;

	VEG_DOC_CTOR
	/// copy constructor
	/// copy constructs the elements memberwise
	tuple(tuple const&)
			VEG_DOC(noexcept(nothrow_copy_constructible<Ts>&&...) requires(
					(copy_constructible<Ts>)&&...)) = default;

	VEG_DOC_CTOR
	/// move assignment operator
	/// move assigns the elements memberwise
	auto operator=(tuple&&) &
			VEG_DOC(noexcept(nothrow_move_assignable<Ts>&&...))
					-> tuple& VEG_DOC(requires((move_assignable<Ts>)&&...)) = default;

	VEG_DOC_CTOR
	/// copy assignment operator
	/// copy assigns the elements memberwise
	auto operator=(tuple const&) &
			VEG_DOC(noexcept(nothrow_copy_assignable<Ts>&&...))
					-> tuple& VEG_DOC(requires((copy_assignable<Ts>)&&...)) = default;

	VEG_DOC_CTOR
	/// converting constructor
	/// initializes each member by forwarding the corresponding argument
	VEG_TEMPLATE(
			typename... Us,
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us&&>))),
			HEDLEY_ALWAYS_INLINE constexpr tuple,
			(/*tag*/, cvt_t),
			(... args, Us&&))
	noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Us&&>))))
			: m_impl(cvt_t{}, VEG_FWD(args)...) {}

	VEG_DOC_CTOR
	/// tuple converting move constructor
	/// initializes the member `arg_i` from `FWD(other.arg_i)`
	VEG_TEMPLATE_EXPLICIT(
			(typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us&&>))),
			explicit(VEG_ANY_OF(!VEG_CONCEPT(convertible<Us&&, Ts>))),
			HEDLEY_ALWAYS_INLINE constexpr tuple,
			((tup, tuple<Us...>&&)),
			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Us&&>))))
			: m_impl(
					internal::tup_::hidden_tag2{},
					static_cast<
							meta::constant<meta::category_e, meta::category_e::ref_mov>*>(
							nullptr),
					internal::tup_::get_inner(VEG_FWD(tup))){})

	VEG_DOC_CTOR
	/// tuple converting constructor
	/// initializes the member `arg_i` from `other.arg_i`
	VEG_TEMPLATE_EXPLICIT(
			(typename... Us),
			requires(VEG_ALL_OF((VEG_CONCEPT(constructible<Ts, Us&>)))),
			explicit(VEG_ANY_OF(!VEG_CONCEPT(convertible<Us&, Ts>))),
			HEDLEY_ALWAYS_INLINE constexpr tuple,
			((tup, tuple<Us...>&)),
			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Us&>))))
			: m_impl(
					internal::tup_::hidden_tag2{},
					static_cast<
							meta::constant<meta::category_e, meta::category_e::ref_mut>*>(
							nullptr),
					internal::tup_::get_inner(VEG_FWD(tup))){})

	VEG_DOC_CTOR
	/// tuple converting copy constructor
	/// initializes the member `arg_i` from `other.arg_i`
	VEG_TEMPLATE_EXPLICIT(
			(typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us const&>))),
			explicit(VEG_ANY_OF(!VEG_CONCEPT(convertible<Us const&, Ts>))),
			HEDLEY_ALWAYS_INLINE constexpr tuple,
			((tup, tuple<Us...> const&)),
			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Us const&>))))
			: m_impl(
					internal::tup_::hidden_tag2{},
					static_cast<meta::constant<meta::category_e, meta::category_e::ref>*>(
							nullptr),
					internal::tup_::get_inner(VEG_FWD(tup))){})

	VEG_DOC_CTOR
	template <typename... Us>
	tuple(tuple<Us...> const&& tup) = delete;

	VEG_DOC_FN
	template <i64 I>
	void operator[](fix<I> /*arg*/) const&& = delete;

	VEG_DOC_FN
	/// moves the ith element out of the tuple
	VEG_TEMPLATE(
			(i64 I),
			requires(
					I < sizeof...(Ts) && (I >= 0) &&
					VEG_CONCEPT(move_constructible<ith<usize{I}, Ts...>>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/,
	     fix<I>)) && noexcept(VEG_CONCEPT(nothrow_move_constructible<ith<usize{I}, Ts...>>))
			-> ith<usize{I}, Ts...> {
		return static_cast<ith<usize{I}, Ts...>&&>(
				internal::tup_::get_impl<I>(internal::tup_::get_inner(*this)));
	}

	VEG_DOC_FN
	/// returns a mutable reference to the ith element
	VEG_TEMPLATE(
			(i64 I),
			requires(I < sizeof...(Ts) && (I >= 0)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, fix<I>)) & noexcept -> ith<usize{I}, Ts...>& {
		return internal::tup_::get_impl<I>(internal::tup_::get_inner(*this));
	}

	VEG_DOC_FN
	/// returns a const reference to the ith element
	VEG_TEMPLATE(
			(i64 I),
			requires(I < sizeof...(Ts) && (I >= 0)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, fix<I>))
	const& noexcept->ith<usize{I}, Ts...> const& {
		return internal::tup_::get_impl<I>(internal::tup_::get_inner(*this));
	}

	VEG_DOC_FN
	/// copy assignment operator
	/// `arg_i = rhs.arg_i`
	VEG_TEMPLATE(
			(typename... Us),
			requires(
					(VEG_ALL_OF(!VEG_CONCEPT(reference<Ts>))) &&
					(VEG_ALL_OF(VEG_CONCEPT(assignable<Ts&, Us const&>)))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator=,
			(rhs, tuple<Us...> const&)) &

			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us const&>))))
					-> tuple& {

		internal::tup_::binop_apply<
				internal::tup_::assign_ftor,
				(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us const&>)))>(

				meta::type_sequence<Ts&...>{},
				meta::type_sequence<Us const&...>{},
				internal::tup_::get_inner(*this),
				internal::tup_::get_inner(rhs)

		);

		return *this;
	}

	VEG_DOC_FN
	/// move assignment operator
	/// `arg_i = FWD(rhs.arg_i)`
	VEG_TEMPLATE(
			(typename... Us),
			requires(
					(VEG_ALL_OF(!VEG_CONCEPT(reference<Ts>))) &&
					(VEG_ALL_OF(VEG_CONCEPT(assignable<Ts&, Us&&>)))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator=,
			(rhs, tuple<Us...>&&)) &

			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us&&>))))
					-> tuple& {

		internal::tup_::binop_apply<
				internal::tup_::assign_ftor,
				(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us&&>)))>(

				meta::type_sequence<Ts&...>{},
				meta::type_sequence<Us&&...>{},
				internal::tup_::get_inner(*this),
				internal::tup_::get_inner(rhs)

		);

		return *this;
	}

	VEG_DOC_FN
	/// proxy copy assignment operator
	/// `FWD_CONST(arg_i) = rhs.arg_i`
	VEG_TEMPLATE(
			(typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(assignable<Ts const&&, Us const&>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator=,
			(rhs, tuple<Us...> const&))
	const&&

			noexcept(
					(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us const&>))))
					->tuple const&& {

		internal::tup_::binop_apply<
				internal::tup_::assign_ftor,
				(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us const&>)))>(

				meta::type_sequence<Ts const&&...>{},
				meta::type_sequence<Us const&...>{},
				internal::tup_::get_inner(*this),
				internal::tup_::get_inner(rhs)

		);

		return static_cast<tuple const&&>(*this);
	}

	VEG_DOC_FN
	/// proxy copy assignment operator
	/// `FWD_CONST(arg_i) = FWD(rhs.arg_i)`
	VEG_TEMPLATE(
			(typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(assignable<Ts const&&, Us&&>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator=,
			(rhs, tuple<Us...>&&))
	const&&

			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us&&>))))
					->tuple const&& {

		internal::tup_::binop_apply<
				internal::tup_::assign_ftor,
				(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us&&>)))>(

				meta::type_sequence<Ts const&&...>{},
				meta::type_sequence<Us&&...>{},
				internal::tup_::get_inner(*this),
				internal::tup_::get_inner(rhs)

		);

		return static_cast<tuple const&&>(*this);
	}

	VEG_DOC_FN
	/// returns a tuple of references `Ts&...`
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto as_ref() & noexcept
			-> tuple<Ts&...> {
		return tuple::as_ref_impl<Ts&...>(
				*this, meta::make_index_sequence<sizeof...(Ts)>{});
	}
	VEG_DOC_FN
	/// returns a tuple of references `Ts const&...`
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto as_ref() const& noexcept
			-> tuple<Ts const&...> {
		return tuple::as_ref_impl<Ts const&...>(
				*this, meta::make_index_sequence<sizeof...(Ts)>{});
	}
	VEG_DOC_FN
	/// returns a tuple of references `Ts&&...`
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto as_ref() && noexcept
			-> tuple<Ts&&...> {
		return tuple::as_ref_impl<Ts&&...>(
				static_cast<tuple&&>(*this),
				meta::make_index_sequence<sizeof...(Ts)>{});
	}

private:
	template <typename... Us, usize... Is>
	static constexpr auto HEDLEY_ALWAYS_INLINE as_ref_impl(
			tuple<Ts...> const& self, meta::index_sequence<Is...> /*seq*/) noexcept
			-> tuple<Us...> {
		return {
				cvt_t{},

				const_cast<Us> /* NOLINT */ (
						static_cast<internal::tup_::tuple_leaf<Is, Ts> const&>(
								internal::tup_::get_inner(self))
								.inner)

						...};
	}

	internal::tup_::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
			m_impl;

	template <typename T>
	friend auto constexpr internal::tup_::get_inner(T&& tup) noexcept
			-> decltype((VEG_FWD(tup).m_impl))&&;
};

VEG_CPP17(template <typename... Ts> tuple(cvt_t, Ts...) -> tuple<Ts...>;)

namespace internal {
namespace tup_ {

/// veg::@namespace::/* implementation defined, accessible by `tuple` through
/// adl */
namespace adl {
VEG_DOC_FN
/// returns a reference `Ti const&` to the ith member

VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr auto get,
		(tup, tuple<Ts...>&))
noexcept -> ith<I, Ts...>& {
	return tup_::get_impl<I>(tup_::get_inner(tup));
}
VEG_DOC_FN
/// returns a reference `Ti&` to the ith member

VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr auto get,
		(tup, tuple<Ts...> const&))
noexcept -> ith<I, Ts...> const& {
	return tup_::get_impl<I>(tup_::get_inner(tup));
}
VEG_DOC_FN
/// returns a forwarded reference `Ti&&` to the ith member

VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr auto get,
		(tup, tuple<Ts...>&&))
noexcept -> ith<I, Ts...>&& {
	return static_cast<ith<I, Ts...>&&>(tup_::get_impl<I>(tup_::get_inner(tup)));
}
VEG_DOC_FN

VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr void get,
		(tup, tuple<Ts...> const&&))
noexcept = delete;

VEG_DOC_FN
/// swaps the `lhs.arg_i` and `rhs.arg_i` memberwise

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				(!VEG_CONCEPT(reference<Ts>) && !VEG_CONCEPT(reference<Us>) &&
         VEG_CONCEPT(swappable<Ts&, Us&>)))),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap,
		(lhs, tuple<Ts...>&),
		(rhs, tuple<Us...>&))
noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts&, Us&>)))) {
	internal::tup_::impl<(VEG_ALL_OF(
			(VEG_CONCEPT(same<Ts, Us>) && VEG_CONCEPT(trivially_swappable<Ts&>))))>::
			template apply<((VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts&, Us&>))))>(
					lhs, rhs);
}

VEG_DOC_FN
/// swaps the `FWD_CONST(lhs.arg_i)` and `FWD_CONST(rhs.arg_i)` memberwise

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(VEG_CONCEPT(swappable<Ts const&&, Us const&&>))),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap,
		(lhs, tuple<Ts...> const&&),
		(rhs, tuple<Us...> const&&))

noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts const&&, Us const&&>)))) {
	internal::tup_::impl<false>::template apply<(VEG_ALL_OF(VEG_CONCEPT(
			nothrow_swappable<Ts const&&, Us const&&>)))>(VEG_FWD(lhs), VEG_FWD(rhs));
}
VEG_DOC_FN
/// swaps the `lhs.arg_i` and `FWD_CONST(rhs.arg_i)` memberwise

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(
				(VEG_ALL_OF(!VEG_CONCEPT(reference<Ts>))) &&
				(VEG_ALL_OF(VEG_CONCEPT(swappable<Ts&, Us const&&>)))),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap,
		(lhs, tuple<Ts...>&),
		(rhs, tuple<Us...> const&&))

noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts&, Us const&&>)))) {
	internal::tup_::impl<false>::template apply<((VEG_ALL_OF(VEG_CONCEPT(
			nothrow_swappable<Ts&, Us const&&>))))>(VEG_FWD(lhs), VEG_FWD(rhs));
}
VEG_DOC_FN
/// swaps the `FWD_CONST(lhs.arg_i)` and `rhs.arg_i` memberwise

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(
				(VEG_ALL_OF(!VEG_CONCEPT(reference<Us>))) &&
				(VEG_ALL_OF(VEG_CONCEPT(swappable<Ts const&&, Us&>)))),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap,
		(lhs, tuple<Ts...> const&&),
		(rhs, tuple<Us...>&))
noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts const&&, Us&>)))) {
	internal::tup_::impl<false>::template apply<(VEG_ALL_OF(VEG_CONCEPT(
			nothrow_swappable<Ts const&&, Us&>)))>(VEG_FWD(lhs), VEG_FWD(rhs));
}

VEG_DOC_FN
/// short-circuiting equality test of all the members in order

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				VEG_CONCEPT(equality_comparable_with<Ts const&, Us const&>))),
		HEDLEY_ALWAYS_INLINE constexpr auto
		operator==,
		(lhs, tuple<Ts...> const&),
		(rhs, tuple<Us...> const&))
noexcept -> bool {
	return cmp_impl::eq(tup_::get_inner(lhs), tup_::get_inner(rhs));
}

VEG_DOC_FN

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				VEG_CONCEPT(equality_comparable_with<Ts const&, Us const&>))),
		HEDLEY_ALWAYS_INLINE constexpr auto
		operator!=,
		(lhs, tuple<Ts...> const&),
		(rhs, tuple<Us...> const&))
noexcept -> bool {
	return !adl::operator==(lhs, rhs);
}

VEG_DOC_FN

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				VEG_CONCEPT(synth_three_way_comparable_with<Ts const&, Us const&>))),
		HEDLEY_ALWAYS_INLINE constexpr auto cmp_3way,
		(lhs, tuple<Ts...> const&),
		(rhs, tuple<Us...> const&))
noexcept -> meta::common_comparison_category_t<
		meta::three_way_comparison_result_t<Ts const&, Us const&>...> {
	return cmp_impl::tway<decltype(adl::cmp_3way(lhs, rhs))>(
			tup_::get_inner(lhs), tup_::get_inner(rhs));
}

} // namespace adl
} // namespace tup_
} // namespace internal

namespace nb {
struct unpack_args {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(VEG_CONCEPT(invocable<Fn, Args&&...>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(fn, Fn&&),
			(args, tuple<Args...>&&))
	const noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>))
			->meta::invoke_result_t<Fn, Args&&...> {
		return internal::tup_::fn_apply_impl(
				meta::type_sequence<Args&&...>{},
				VEG_FWD(fn),
				internal::tup_::get_inner(args));
	}

	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(VEG_CONCEPT(invocable<Fn, Args const&...>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(fn, Fn&&),
			(args, tuple<Args...> const&))
	const noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args const&...>))
			->meta::invoke_result_t<Fn, Args const&...> {
		return internal::tup_::fn_apply_impl(
				meta::type_sequence<Args const&...>{},
				VEG_FWD(fn),
				internal::tup_::get_inner(args));
	}

	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(VEG_CONCEPT(invocable<Fn, Args&...>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(fn, Fn&&),
			(args, tuple<Args...>&))
	const noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args&...>))
			->meta::invoke_result_t<Fn, Args&...> {
		return internal::tup_::fn_apply_impl(
				meta::type_sequence<Args&...>{},
				VEG_FWD(fn),
				internal::tup_::get_inner(args));
	}

	template <typename Fn, typename... Args>
	void operator()(Fn&&, tuple<Args...> const&&) const noexcept = delete;
};
} // namespace nb
VEG_NIEBLOID(unpack_args);

namespace make {
namespace nb {
struct tuple {
	VEG_TEMPLATE(
			typename... Ts,
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<meta::decay_t<Ts>, Ts&&>))),
			constexpr auto
			operator(),
			(... args, Ts&&))
	const noexcept(meta::all_of({VEG_CONCEPT(
										 nothrow_constructible<meta::decay_t<Ts>, Ts&&>)...}))
			->veg::tuple<meta::decay_t<Ts>...> {
		return {cvt_t{}, VEG_FWD(args)...};
	}
};

struct tuple_ref {
	template <typename... Ts>
	HEDLEY_ALWAYS_INLINE constexpr auto operator()(Ts&&... args) const noexcept
			-> veg::tuple<Ts&&...> {
		return {cvt_t{}, VEG_FWD(args)...};
	}
};

struct tuple_fwd {
	VEG_TEMPLATE(
			typename... Ts,
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Ts&&>))),
			constexpr auto
			operator(),
			(... args, Ts&&))
	const noexcept(
			meta::all_of({VEG_CONCEPT(nothrow_constructible<Ts, Ts&&>)...}))
			->veg::tuple<Ts...> {
		return {cvt_t{}, VEG_FWD(args)...};
	}
};
} // namespace nb
VEG_NIEBLOID(tuple);
VEG_NIEBLOID(tuple_ref);
VEG_NIEBLOID(tuple_fwd);
} // namespace make

namespace meta {

template <typename... Ts>
struct is_trivially_relocatable<tuple<Ts...>>
		: bool_constant<(VEG_ALL_OF(is_trivially_relocatable<Ts>::value))> {};

template <typename... Ts>
struct is_mostly_trivial<tuple<Ts...>>
		: bool_constant<(VEG_ALL_OF(is_mostly_trivial<Ts>::value))> {};

template <typename... Ts>
struct is_trivially_swappable<tuple<Ts...>&>
		: veg::internal::tup_::trivially_swappable<tuple<Ts...>&, tuple<Ts...>&> {};
} // namespace meta

template <typename... Ts>
struct fmt::debug<tuple<Ts...>> {

	template <usize... Is>
	static void to_string_impl(
			fmt::buffer& out,
			internal::tup_::tuple_impl<meta::index_sequence<Is...>, Ts...> const&
					tup) {

		out.insert(out.size(), "{", 1);
		(void)internal::empty_arr{
				internal::empty{},
				((Is > 0) ? (out.insert(out.size(), ", ", 2)) : void(0),
		     debug<Ts>::to_string(
						 out,
						 static_cast<internal::tup_::tuple_leaf<Is, Ts> const&>(tup).inner),
		     internal::empty{})...};
		out.insert(out.size(), "}", 1);
	}

	static void to_string(fmt::buffer& out, tuple<Ts...> const& tup) {
		debug::to_string_impl(out, internal::tup_::get_inner(tup));
	}
};

} // namespace VEG_ABI
} // namespace veg

namespace std {
template <typename... Ts>
struct tuple_size<veg::tuple<Ts...>>
		: integral_constant<veg::usize, sizeof...(Ts)> {};

template <veg::usize I, typename... Ts>
struct tuple_element<I, veg::tuple<Ts...>> {
	using type =
			typename veg::internal::tup_::pack_ith_elem<I>::template type<Ts...>;
};
} // namespace std

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TUPLE_HPP_B8PHUNWES */

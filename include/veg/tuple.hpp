#ifndef VEG_TUPLE_HPP_B8PHUNWES
#define VEG_TUPLE_HPP_B8PHUNWES

#include "veg/internal/tuple_generic.hpp"
#include "veg/internal/fmt.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/util/get.hpp"
#include "veg/functional/into.hpp"
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

namespace internal {
namespace meta_ {
struct is_tuple_helper {
	static auto test(void*) noexcept -> false_type;
	template <typename... Ts>
	static auto test(Tuple<Ts...>*) noexcept -> true_type;

	static auto size(void*) noexcept -> constant<usize, 0>;
	template <typename... Ts>
	static auto size(Tuple<Ts...>*) noexcept -> constant<usize, sizeof...(Ts)>;

	template <usize I>
	static auto element(void*) noexcept -> void;
	template <usize I, typename... Ts>
	static auto element(Tuple<Ts...>*) noexcept -> ith<I, Ts...>;

	static auto seq(void*) noexcept -> void;
	template <typename... Ts>
	static auto seq(Tuple<Ts...>*) noexcept -> meta::type_sequence<Ts...>;
};
} // namespace meta_
} // namespace internal
namespace meta {
template <typename T>
struct is_tuple
		: decltype(internal::meta_::is_tuple_helper::test(VEG_DECLVAL(T*))) {};
template <typename T>
using tuple_size =
		decltype(internal::meta_::is_tuple_helper::size(VEG_DECLVAL(T*)));

template <usize I, typename T>
using tuple_element_t = decltype(
		internal::meta_::is_tuple_helper::template element<I>(VEG_DECLVAL(T*)));

template <typename T>
using tuple_seq_t =
		decltype(internal::meta_::is_tuple_helper::seq(VEG_DECLVAL(T*)));
} // namespace meta

namespace concepts {
VEG_DEF_CONCEPT(typename T, tuple, meta::is_tuple<T>::value);
} // namespace concepts

VEG_DOC_CLASS
template <typename... Ts>
struct Tuple
		: internal::tup_::
					IndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...>,
			internal::tup_::adl::tuple_base<Ts...> {

	using Indexed = internal::tup_::
			IndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...>;

	~Tuple() = default;

	VEG_DOC_CTOR
	/// default constructor
	/// default or value initializes all the elements
	constexpr Tuple() VEG_DOC(noexcept(nothrow_constructible<Ts>&&...) requires(
			constructible<Ts>&&...)) = default;

	VEG_DOC_CTOR
	/// move constructor
	/// move (or copy if the member can't be moved) constructs the elements
	/// memberwise
	Tuple(Tuple&&) VEG_DOC(noexcept(nothrow_move_constructible<Ts>&&...) requires(
			(move_constructible<Ts> || copy_constructible<Ts>)&&...)) = default;

	VEG_DOC_CTOR
	/// copy constructor
	/// copy constructs the elements memberwise
	explicit Tuple(Tuple const&)
			VEG_DOC(noexcept(nothrow_copy_constructible<Ts>&&...) requires(
					(copy_constructible<Ts>)&&...)) = default;

	VEG_DOC_CTOR
	/// move assignment operator
	/// move assigns the elements memberwise
	auto operator=(Tuple&&) &
			VEG_DOC(noexcept(nothrow_move_assignable<Ts>&&...))
					-> Tuple& VEG_DOC(requires((move_assignable<Ts>)&&...)) = default;

	VEG_DOC_CTOR
	/// copy assignment operator
	/// copy assigns the elements memberwise
	auto operator=(Tuple const&) &
			VEG_DOC(noexcept(nothrow_copy_assignable<Ts>&&...))
					-> Tuple& VEG_DOC(requires((copy_assignable<Ts>)&&...)) = default;

	VEG_DOC_CTOR
	/// converting constructor
	/// initializes each member by forwarding the corresponding argument
	VEG_TEMPLATE(
			typename... Us,
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us&&>))),
			HEDLEY_ALWAYS_INLINE constexpr Tuple,
			(/*tag*/, Cvt),
			(... args, Us&&))
	noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Us&&>))))
			: Indexed(Cvt{}, VEG_FWD(args)...) {}

	VEG_DOC_CTOR
	/// tuple converting move constructor
	/// initializes the member `arg_i` from `FWD(other.arg_i)`
	VEG_TEMPLATE(
			(typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us&&>))),
			HEDLEY_ALWAYS_INLINE constexpr Tuple,
			(/*tag*/, Into),
			(tup, Tuple<Us...>&&))
	noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Us&&>))))
			: Indexed(
						internal::tup_::hidden_tag2{},
						static_cast<
								meta::constant<meta::category_e, meta::category_e::ref_mov>*>(
								nullptr),
						tup) {}

	VEG_DOC_CTOR
	/// tuple converting constructor
	/// initializes the member `arg_i` from `other.arg_i`
	VEG_TEMPLATE(
			(typename... Us),
			requires(VEG_ALL_OF((VEG_CONCEPT(constructible<Ts, Us&>)))),
			HEDLEY_ALWAYS_INLINE constexpr Tuple,
			(/*tag*/, Into),
			(tup, Tuple<Us...>&))
	noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Us&>))))
			: Indexed(
						internal::tup_::hidden_tag2{},
						static_cast<
								meta::constant<meta::category_e, meta::category_e::ref_mut>*>(
								nullptr),
						tup) {}

	VEG_DOC_CTOR
	/// tuple converting copy constructor
	/// initializes the member `arg_i` from `other.arg_i`
	VEG_TEMPLATE(
			(typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Us const&>))),
			HEDLEY_ALWAYS_INLINE constexpr Tuple,
			(/*tag*/, Into),
			(tup, Tuple<Us...> const&))
	noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Us const&>))))
			: Indexed(
						internal::tup_::hidden_tag2{},
						static_cast<
								meta::constant<meta::category_e, meta::category_e::ref>*>(
								nullptr),
						tup) {}

	VEG_DOC_CTOR
	template <typename... Us>
	Tuple(Tuple<Us...> const&& tup) = delete;

	VEG_DOC_FN
	template <i64 I>
	void operator[](Fix<I> /*arg*/) const&& = delete;

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
	     Fix<I>)) && noexcept(VEG_CONCEPT(nothrow_move_constructible<ith<usize{I}, Ts...>>))
			-> ith<usize{I}, Ts...> {
		return static_cast<ith<usize{I}, Ts...>&&>(
				internal::tup_::get_impl<I>(*this));
	}

	VEG_DOC_FN
	/// returns a mutable reference to the ith element
	VEG_TEMPLATE(
			(i64 I),
			requires(I < sizeof...(Ts) && (I >= 0)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>)) & noexcept -> ith<usize{I}, Ts...>& {
		return internal::tup_::get_impl<I>(*this);
	}

	VEG_DOC_FN
	/// returns a const reference to the ith element
	VEG_TEMPLATE(
			(i64 I),
			requires(I < sizeof...(Ts) && (I >= 0)),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>))
	const& noexcept->ith<usize{I}, Ts...> const& {
		return internal::tup_::get_impl<I>(*this);
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
			(rhs, Tuple<Us...> const&)) &

			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us const&>))))
					-> Tuple& {

		internal::tup_::binop_apply<
				internal::tup_::assign_ftor,
				(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us const&>)))>(

				meta::type_sequence<Ts&...>{},
				meta::type_sequence<Us const&...>{},
				*this,
				rhs

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
			(rhs, Tuple<Us...>&&)) &

			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us&&>))))
					-> Tuple& {

		internal::tup_::binop_apply<
				internal::tup_::assign_ftor,
				(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us&&>)))>(

				meta::type_sequence<Ts&...>{},
				meta::type_sequence<Us&&...>{},
				*this,
				rhs

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
			(rhs, Tuple<Us...> const&))
	const&&

			noexcept(
					(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us const&>))))
					->Tuple const&& {

		internal::tup_::binop_apply<
				internal::tup_::assign_ftor,
				(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us const&>)))>(

				meta::type_sequence<Ts const&&...>{},
				meta::type_sequence<Us const&...>{},
				*this,
				rhs

		);

		return static_cast<Tuple const&&>(*this);
	}

	VEG_DOC_FN
	/// proxy copy assignment operator
	/// `FWD_CONST(arg_i) = FWD(rhs.arg_i)`
	VEG_TEMPLATE(
			(typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(assignable<Ts const&&, Us&&>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator=,
			(rhs, Tuple<Us...>&&))
	const&&

			noexcept((VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us&&>))))
					->Tuple const&& {

		internal::tup_::binop_apply<
				internal::tup_::assign_ftor,
				(VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts const&&, Us&&>)))>(

				meta::type_sequence<Ts const&&...>{},
				meta::type_sequence<Us&&...>{},
				*this,
				rhs

		);

		return static_cast<Tuple const&&>(*this);
	}

	VEG_DOC_FN
	/// returns a tuple of references `Ts&...`
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto as_ref() & noexcept
			-> Tuple<Ts&...> {
		return Tuple::as_ref_impl<Ts&...>(
				*this, meta::make_index_sequence<sizeof...(Ts)>{});
	}
	VEG_DOC_FN
	/// returns a tuple of references `Ts const&...`
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto as_ref() const& noexcept
			-> Tuple<Ts const&...> {
		return Tuple::as_ref_impl<Ts const&...>(
				*this, meta::make_index_sequence<sizeof...(Ts)>{});
	}
	VEG_DOC_FN
	/// returns a tuple of references `Ts&&...`
	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto as_ref() && noexcept
			-> Tuple<Ts&&...> {
		return Tuple::as_ref_impl<Ts&&...>(
				static_cast<Tuple&&>(*this),
				meta::make_index_sequence<sizeof...(Ts)>{});
	}

private:
	template <typename... Us, usize... Is>
	static constexpr auto HEDLEY_ALWAYS_INLINE as_ref_impl(
			Tuple<Ts...> const& self, meta::index_sequence<Is...> /*seq*/) noexcept
			-> Tuple<Us...> {
		return {
				Cvt{},

				const_cast<Us> /* NOLINT */ (
						static_cast<internal::tup_::tuple_leaf<Is, Ts> const&>(self).inner)

						...};
	}
};

VEG_CPP17(template <typename... Ts> Tuple(Cvt, Ts...) -> Tuple<Ts...>;)

namespace internal {
namespace tup_ {
namespace adl {
VEG_DOC_FN
/// returns a reference `Ti const&` to the ith member

VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr auto get,
		(tup, Tuple<Ts...>&))
noexcept -> ith<I, Ts...>& {
	return tup_::get_impl<I>(tup);
}
VEG_DOC_FN
/// returns a reference `Ti&` to the ith member

VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr auto get,
		(tup, Tuple<Ts...> const&))
noexcept -> ith<I, Ts...> const& {
	return tup_::get_impl<I>(tup);
}
VEG_DOC_FN
/// returns a forwarded reference `Ti&&` to the ith member

VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr auto get,
		(tup, Tuple<Ts...>&&))
noexcept -> ith<I, Ts...>&& {
	return static_cast<ith<I, Ts...>&&>(tup_::get_impl<I>(tup));
}
VEG_DOC_FN

VEG_TEMPLATE(
		(usize I, typename... Ts),
		requires(I < sizeof...(Ts)),
		HEDLEY_ALWAYS_INLINE constexpr void get,
		(tup, Tuple<Ts...> const&&))
noexcept = delete;

VEG_DOC_FN
/// swaps the `lhs.arg_i` and `rhs.arg_i` memberwise

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				(!VEG_CONCEPT(reference<Ts>) && !VEG_CONCEPT(reference<Us>) &&
         VEG_CONCEPT(swappable<Ts&, Us&>)))),
		HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void swap,
		(lhs, Tuple<Ts...>&),
		(rhs, Tuple<Us...>&))
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
		(lhs, Tuple<Ts...> const&&),
		(rhs, Tuple<Us...> const&&))

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
		(lhs, Tuple<Ts...>&),
		(rhs, Tuple<Us...> const&&))

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
		(lhs, Tuple<Ts...> const&&),
		(rhs, Tuple<Us...>&))
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
		(lhs, Tuple<Ts...> const&),
		(rhs, Tuple<Us...> const&))
noexcept -> bool {
	return cmp_impl::eq(lhs, rhs);
}

VEG_DOC_FN

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				VEG_CONCEPT(equality_comparable_with<Ts const&, Us const&>))),
		HEDLEY_ALWAYS_INLINE constexpr auto
		operator!=,
		(lhs, Tuple<Ts...> const&),
		(rhs, Tuple<Us...> const&))
noexcept -> bool {
	return !adl::operator==(lhs, rhs);
}

VEG_DOC_FN

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				VEG_CONCEPT(synth_three_way_comparable_with<Ts const&, Us const&>))),
		HEDLEY_ALWAYS_INLINE constexpr auto cmp_3way,
		(lhs, Tuple<Ts...> const&),
		(rhs, Tuple<Us...> const&))
noexcept -> meta::common_comparison_category_t<
		meta::three_way_comparison_result_t<Ts const&, Us const&>...> {
	return cmp_impl::tway<decltype(adl::cmp_3way(lhs, rhs))>(lhs, rhs);
}

} // namespace adl
} // namespace tup_
} // namespace internal

namespace nb {
template <typename ISeq, typename... Ts>
using IndexedTuple = internal::tup_::IndexedTuple<ISeq, Ts...>;

struct tuple_zip {
	constexpr auto operator()() const noexcept -> Tuple<> { return {}; }

	template <usize I, typename T>
	struct tuple_zip_helper {
		template <typename... Ts>
		using type = Tuple<T, meta::tuple_element_t<I, Ts>...>;

		template <typename... Ts>
		using type_ref = Tuple<T&&, meta::tuple_element_t<I, Ts>&&...>;

		template <typename... Ts>
		constexpr auto apply(Ts&&... tups) const noexcept -> type_ref<Ts...> {
			return {
					Cvt{},
					VEG_FWD(first),
					static_cast<meta::tuple_element_t<I, Ts>&&>(
							internal::tup_::get_impl<I>(tups))...};
		}
		T&& first;
	};

	VEG_TEMPLATE(
			(usize... Is, typename... Ts, typename... Tuples),
			requires(
					VEG_ALL_OF(VEG_CONCEPT(tuple<Tuples>)) &&
					VEG_ALL_OF(VEG_CONCEPT(same<
																 meta::constant<usize, sizeof...(Ts)>,
																 meta::tuple_size<Tuples>>)) &&

					VEG_CONCEPT(move_constructible<Tuple<Ts...>>) &&
					VEG_ALL_OF(VEG_CONCEPT(move_constructible<Tuples>))),
			constexpr auto
			operator(),
			(tup, IndexedTuple<meta::index_sequence<Is...>, Ts...>),
			(... tups, Tuples))
	const noexcept(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Tuples>)))
			->Tuple<                                //
					typename tuple_zip_helper<Is, Ts>:: //
					template type<Tuples...>...> {
		return {
				Cvt{},
				fn::into_ref(tuple_zip_helper<Is, Ts>{
						static_cast<Ts&&>(internal::tup_::get_impl<Is>(tup))}
		                     .template apply<Tuples...>(VEG_FWD(tups)...))...};
	}
};

struct tuple_cat {

	template <typename... Ts>
	using TupleRef = Tuple<Ts&&...>;

	VEG_TEMPLATE(
			(typename... Tuples),
			requires(VEG_ALL_OF(VEG_CONCEPT(tuple<Tuples>))),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(... tups, Tuples))
	const noexcept(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Tuples>)))
			->meta::type_sequence_apply<
					Tuple,
					meta::type_sequence_cat<meta::tuple_seq_t<Tuples>...>> {
		return tuple_cat::apply(VEG_FWD(tups).as_ref()...);
	}

private:
	static constexpr auto apply() noexcept -> Tuple<> { return {}; }

	template <usize... Is, typename... Ts, typename... Tuples>
	HEDLEY_ALWAYS_INLINE static constexpr auto apply(
			IndexedTuple<meta::index_sequence<Is...>, Ts...>&& first,
			Tuples&&... rest) noexcept
			-> meta::type_sequence_apply<
					Tuple,
					meta::type_sequence_cat<
							meta::type_sequence<Ts...>,
							meta::tuple_seq_t<Tuples>...>> {
		return tuple_cat::apply2(
				VEG_FWD(first), tuple_cat::apply(VEG_FWD(rest)...));
	}

	template <usize... Is, typename... Ts, usize... Js, typename... Us>
	HEDLEY_ALWAYS_INLINE static constexpr auto apply2(
			IndexedTuple<meta::index_sequence<Is...>, Ts...>&& first,
			IndexedTuple<meta::index_sequence<Js...>, Us...>&& second) noexcept
			-> Tuple<Ts..., Us...> {
		return {
				Cvt{},
				static_cast<Ts&&>(internal::tup_::get_impl<Is>(first))...,
				static_cast<Us&&>(internal::tup_::get_impl<Js>(second))...,
		};
	}
};

struct tuple_unpack {
	VEG_TEMPLATE(
			(typename Fn, typename... Args, usize... Is),
			requires(VEG_CONCEPT(invocable<Fn, Args&&...>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(fn, Fn&&),
			(args, IndexedTuple<meta::index_sequence<Is...>, Args...>))
	const noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>))
			->meta::invoke_result_t<Fn, Args&&...> {

		return VEG_FWD(fn)(
				static_cast<Args&&>(internal::tup_::get_impl<Is>(args))...);
	}
};

struct tuple_for_each_i {
	VEG_TEMPLATE(
			(typename Fn, typename... Args, usize... Is),
			requires(VEG_ALL_OF(VEG_CONCEPT(invocable<Fn&, Fix<i64{Is}>, Args&&>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void
			operator(),
			(fn, Fn&&),
			(args, IndexedTuple<meta::index_sequence<Is...>, Args...>))
	const noexcept(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Fn&, Fix<i64{Is}>, Args&&>))) {
		static_cast<void>(internal::EmptyArr{
				(static_cast<void>(
						 fn(Fix<i64{Is}>{},
		            static_cast<Args&&>(internal::tup_::get_impl<Is>(args)))),
		     internal::Empty{})...});
	}
};

struct tuple_for_each {
	VEG_TEMPLATE(
			(typename Fn, typename... Args, usize... Is),
			requires(VEG_ALL_OF(VEG_CONCEPT(invocable<Fn&, Args&&>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) void
			operator(),
			(fn, Fn&&),
			(args, IndexedTuple<meta::index_sequence<Is...>, Args...>))
	const noexcept(VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Fn&, Args&&>))) {
		static_cast<void>(internal::EmptyArr{
				(static_cast<void>(
						 fn(static_cast<Args&&>(internal::tup_::get_impl<Is>(args)))),
		     internal::Empty{})...});
	}
};

struct tuple_map_i {
	VEG_TEMPLATE(
			(typename Fn, typename... Args, usize... Is),
			requires(
					VEG_ALL_OF(VEG_CONCEPT(invocable<Fn&, Fix<i64{Is}>, Args&&>)) &&
					VEG_ALL_OF(
							VEG_CONCEPT(move_constructible<
													meta::invoke_result_t<Fn&, Fix<i64{Is}>, Args&&>>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(fn, Fn&&),
			(args, IndexedTuple<meta::index_sequence<Is...>, Args...>))
	const noexcept(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Fn&, Fix<i64{Is}>, Args&&>)) &&
			VEG_ALL_OF(
					VEG_CONCEPT(nothrow_move_constructible<
											meta::invoke_result_t<Fn&, Fix<i64{Is}>, Args&&>>)))
			->Tuple<meta::invoke_result_t<Fn&, Fix<i64{Is}>, Args&&>...> {
		return {
				Cvt{},
				fn(Fix<i64{Is}>{},
		       static_cast<Args&&>(internal::tup_::get_impl<Is>(args)))...,
		};
	}
};

struct tuple_map {
	VEG_TEMPLATE(
			(typename Fn, typename... Args, usize... Is),
			requires(VEG_ALL_OF(VEG_CONCEPT(invocable<Fn&, Args&&>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(fn, Fn&&),
			(args, IndexedTuple<meta::index_sequence<Is...>, Args...>))
	const noexcept(VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Fn&, Args&&>)))
			->Tuple<meta::invoke_result_t<Fn&, Args&&>...> {
		return {
				Cvt{},
				fn(static_cast<Args&&>(internal::tup_::get_impl<Is>(args)))...,
		};
	}
};
} // namespace nb
VEG_NIEBLOID(tuple_unpack);
VEG_NIEBLOID(tuple_for_each_i);
VEG_NIEBLOID(tuple_for_each);
VEG_NIEBLOID(tuple_map_i);
VEG_NIEBLOID(tuple_map);
VEG_NIEBLOID(tuple_zip);
VEG_NIEBLOID(tuple_cat);

namespace nb {
struct tuple {
	VEG_TEMPLATE(
			typename... Ts,
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<meta::decay_t<Ts>, Ts&&>))),
			constexpr auto
			operator(),
			(... args, Ts&&))
	const noexcept(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<meta::decay_t<Ts>, Ts&&>)))
			->veg::Tuple<meta::decay_t<Ts>...> {
		return {Cvt{}, VEG_FWD(args)...};
	}
};

struct tuple_ref {
	template <typename... Ts>
	HEDLEY_ALWAYS_INLINE constexpr auto operator()(Ts&&... args) const noexcept
			-> veg::Tuple<Ts&&...> {
		return {Cvt{}, VEG_FWD(args)...};
	}
};

struct tuple_fwd {
	VEG_TEMPLATE(
			typename... Ts,
			requires(VEG_ALL_OF(VEG_CONCEPT(constructible<Ts, Ts&&>))),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(... args, Ts&&))
	const noexcept(VEG_ALL_OF(VEG_CONCEPT(nothrow_constructible<Ts, Ts&&>)))
			->veg::Tuple<Ts...> {
		return {Cvt{}, VEG_FWD(args)...};
	}
};
} // namespace nb
VEG_NIEBLOID(tuple);
VEG_NIEBLOID(tuple_ref);
VEG_NIEBLOID(tuple_fwd);

namespace meta {

template <typename... Ts>
struct is_trivially_relocatable<Tuple<Ts...>>
		: bool_constant<(VEG_ALL_OF(is_trivially_relocatable<Ts>::value))> {};

template <typename... Ts>
struct is_mostly_trivial<Tuple<Ts...>>
		: bool_constant<(VEG_ALL_OF(is_mostly_trivial<Ts>::value))> {};

template <typename... Ts>
struct is_trivially_swappable<Tuple<Ts...>&>
		: veg::internal::tup_::trivially_swappable<Tuple<Ts...>&, Tuple<Ts...>&> {};
} // namespace meta

template <typename... Ts>
struct fmt::Debug<Tuple<Ts...>> {

	template <usize... Is>
	static void to_string_impl(
			fmt::Buffer& out,
			internal::tup_::IndexedTuple<meta::index_sequence<Is...>, Ts...> const&
					tup) {

		out.insert(out.size(), "{", 1);
		(void)internal::EmptyArr{
				internal::Empty{},
				((Is > 0) ? (out.insert(out.size(), ", ", 2)) : void(0),
		     Debug<meta::uncvref_t<Ts>>::to_string(
						 out,
						 static_cast<internal::tup_::tuple_leaf<Is, Ts> const&>(tup).inner),
		     internal::Empty{})...};
		out.insert(out.size(), "}", 1);
	}

	static void to_string(fmt::Buffer& out, Tuple<Ts...> const& tup) {
		Debug::to_string_impl(out, tup);
	}
};

} // namespace VEG_ABI
} // namespace veg

namespace std {
template <typename... Ts>
struct tuple_size<veg::Tuple<Ts...>>
		: integral_constant<veg::usize, sizeof...(Ts)> {};

template <veg::usize I, typename... Ts>
struct tuple_element<I, veg::Tuple<Ts...>> {
	using type =
			typename veg::internal::tup_::pack_ith_elem<I>::template type<Ts...>;
};
} // namespace std

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TUPLE_HPP_B8PHUNWES */

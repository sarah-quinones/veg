#ifndef VEG_TUPLE_HPP_B8PHUNWES
#define VEG_TUPLE_HPP_B8PHUNWES

#include "veg/internal/tuple_generic.hpp"
#include "veg/internal/fmt.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/util/get.hpp"
#include "veg/functional/utils.hpp"
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
namespace internal {
namespace meta_ {
struct is_tuple_helper {
	static auto test(void*) -> false_type;
	template <typename... Ts>
	static auto test(Tuple<Ts...>*) -> true_type;

	static auto size(void*) -> constant<usize, 0>;
	template <typename... Ts>
	static auto size(Tuple<Ts...>*) -> constant<usize, sizeof...(Ts)>;

	template <usize I>
	static auto element(void*) -> void;
	template <usize I, typename... Ts>
	static auto element(Tuple<Ts...>*) -> ith<I, Ts...>;

	static auto seq(void*) -> void;
	template <typename... Ts>
	static auto seq(Tuple<Ts...>*) -> meta::type_sequence<Ts...>;
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
using tuple_element_t =
		decltype(internal::meta_::is_tuple_helper::template element<I>(
				VEG_DECLVAL(T*)));

template <typename T>
using tuple_seq_t =
		decltype(internal::meta_::is_tuple_helper::seq(VEG_DECLVAL(T*)));
} // namespace meta

namespace concepts {
VEG_DEF_CONCEPT(typename T, tuple, meta::is_tuple<T>::value);
} // namespace concepts

template <typename... Ts>
struct Tuple
		: internal::tup_::
					IndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...>,
			internal::tup_::adl::tuple_base<Ts...> {

	using Indexed = internal::tup_::
			IndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...>;

	using Indexed::Indexed;
	using Indexed::operator=;

	VEG_EXPLICIT_COPY(Tuple);
};

VEG_CPP17(template <typename... Ts> Tuple(Direct, Ts...) -> Tuple<Ts...>;)

namespace internal {
namespace tup_ {
namespace adl {
/// returns a reference `Ti const&` to the ith member

template <usize I, usize... Is, typename... Ts>
VEG_NODISCARD VEG_INLINE constexpr auto
get(IndexedTuple<meta::index_sequence<Is...>, Ts...> const& tup) VEG_NOEXCEPT
		-> ith<I, Ts...> const& {
	return static_cast<TupleLeaf<I, ith<I, Ts...>> const&>(tup).leaf;
}
template <usize I, usize... Is, typename... Ts>
VEG_NODISCARD VEG_INLINE constexpr auto
get(IndexedTuple<meta::index_sequence<Is...>, Ts...>& tup) VEG_NOEXCEPT
		-> ith<I, Ts...>& {
	return static_cast<TupleLeaf<I, ith<I, Ts...>>&>(tup).leaf;
}
template <usize I, usize... Is, typename... Ts>
VEG_NODISCARD VEG_INLINE constexpr auto
get(IndexedTuple<meta::index_sequence<Is...>, Ts...> const&& tup) VEG_NOEXCEPT
		-> ith<I, Ts...> const&& {
	return static_cast<ith<I, Ts...> const&&>(
			static_cast<TupleLeaf<I, ith<I, Ts...>> const&&>(tup).leaf);
}
template <usize I, usize... Is, typename... Ts>
VEG_NODISCARD VEG_INLINE constexpr auto
get(IndexedTuple<meta::index_sequence<Is...>, Ts...>&& tup) VEG_NOEXCEPT
		-> ith<I, Ts...>&& {
	return static_cast<ith<I, Ts...>&&>(
			static_cast<TupleLeaf<I, ith<I, Ts...>>&&>(tup).leaf);
}

/// swaps the `lhs.arg_i` and `rhs.arg_i` memberwise

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				(!VEG_CONCEPT(reference<Ts>) && !VEG_CONCEPT(reference<Us>) &&
         VEG_CONCEPT(swappable<Ts&, Us&>)))),
		VEG_INLINE VEG_CPP14(constexpr) void swap,
		(lhs, Tuple<Ts...>&),
		(rhs, Tuple<Us...>&))
VEG_NOEXCEPT_IF((VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts&, Us&>)))) {
	internal::tup_::SwapImpl<(VEG_ALL_OF(
			(VEG_CONCEPT(same<Ts, Us>) && VEG_CONCEPT(trivially_swappable<Ts&>))))> //
			::apply(lhs, rhs);
}

/// swaps the `FWD_CONST(lhs.arg_i)` and `FWD_CONST(rhs.arg_i)` memberwise

VEG_TEMPLATE(
		(usize... Is, typename... Ts, typename... Us),
		requires(VEG_ALL_OF(VEG_CONCEPT(swappable<Ts const&&, Us const&&>))),
		VEG_INLINE VEG_CPP14(constexpr) void swap,
		(lhs, IndexedTuple<meta::index_sequence<Is...>, Ts...> const&&),
		(rhs, IndexedTuple<meta::index_sequence<Is...>, Us...> const&&))

VEG_NOEXCEPT_IF(
		(VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts const&&, Us const&&>)))) {
	(void)EmptyArr{
			Empty{},
			(veg::swap( //
					 static_cast<Ts const&&>(
							 static_cast<TupleLeaf<Is, Ts> const&>(lhs).leaf),
					 static_cast<Us const&&>(
							 static_cast<TupleLeaf<Is, Us> const&>(rhs).leaf)),
	     Empty{})...};
}
/// swaps the `lhs.arg_i` and `FWD_CONST(rhs.arg_i)` memberwise

VEG_TEMPLATE(
		(usize... Is, typename... Ts, typename... Us),
		requires(
				(VEG_ALL_OF(!VEG_CONCEPT(reference<Ts>))) &&
				(VEG_ALL_OF(VEG_CONCEPT(swappable<Ts&, Us const&&>)))),
		VEG_INLINE VEG_CPP14(constexpr) void swap,
		(lhs, IndexedTuple<meta::index_sequence<Is...>, Ts...>&),
		(rhs, IndexedTuple<meta::index_sequence<Is...>, Us...> const&&))

VEG_NOEXCEPT_IF((VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts&, Us const&&>)))) {
	(void)EmptyArr{
			Empty{},
			(veg::swap( //
					 static_cast<TupleLeaf<Is, Ts>&>(lhs).leaf,
					 static_cast<Us const&&>(
							 static_cast<TupleLeaf<Is, Us> const&>(rhs).leaf)),
	     Empty{})...};
}
/// swaps the `FWD_CONST(lhs.arg_i)` and `rhs.arg_i` memberwise

VEG_TEMPLATE(
		(usize... Is, typename... Ts, typename... Us),
		requires(
				(VEG_ALL_OF(!VEG_CONCEPT(reference<Us>))) &&
				(VEG_ALL_OF(VEG_CONCEPT(swappable<Ts const&&, Us&>)))),
		VEG_INLINE VEG_CPP14(constexpr) void swap,
		(lhs, Tuple<Ts...> const&&),
		(rhs, Tuple<Us...>&))
VEG_NOEXCEPT_IF((VEG_ALL_OF(VEG_CONCEPT(nothrow_swappable<Ts const&&, Us&>)))) {
	(void)EmptyArr{
			Empty{},
			(veg::swap( //
					 static_cast<Ts const&&>(
							 static_cast<TupleLeaf<Is, Ts> const&>(lhs).leaf),
					 static_cast<TupleLeaf<Is, Us>&>(rhs).leaf),
	     Empty{})...};
}

/// short-circuiting equality test of all the members in order

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				VEG_CONCEPT(equality_comparable_with<Ts const&, Us const&>))),
		VEG_NODISCARD VEG_INLINE constexpr auto
		operator==,
		(lhs, Tuple<Ts...> const&),
		(rhs, Tuple<Us...> const&))
VEG_NOEXCEPT->bool {
	return cmp_impl::eq(lhs, rhs);
}

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				VEG_CONCEPT(equality_comparable_with<Ts const&, Us const&>))),
		VEG_NODISCARD VEG_INLINE constexpr auto
		operator!=,
		(lhs, Tuple<Ts...> const&),
		(rhs, Tuple<Us...> const&))
VEG_NOEXCEPT->bool {
	return !adl::operator==(lhs, rhs);
}

VEG_TEMPLATE(
		(typename... Ts, typename... Us),
		requires(VEG_ALL_OF(
				VEG_CONCEPT(synth_three_way_comparable_with<Ts const&, Us const&>))),
		VEG_NODISCARD VEG_INLINE constexpr auto cmp_3way,
		(lhs, Tuple<Ts...> const&),
		(rhs, Tuple<Us...> const&))
VEG_NOEXCEPT->meta::common_comparison_category_t<
		meta::three_way_comparison_result_t<Ts const&, Us const&>...> {
	return cmp_impl::tway<decltype(adl::cmp_3way(lhs, rhs))>(lhs, rhs);
}

} // namespace adl
} // namespace tup_
} // namespace internal

namespace nb {
struct tuple {
	template <typename ISeq, typename... Ts>
	using IndexedTuple = internal::tup_::IndexedTuple<ISeq, Ts...>;
	template <usize I, typename... Ts>
	using TupleLeaf = internal::tup_::TupleLeaf<I, Ts...>;

	VEG_TEMPLATE(
			typename... Ts,
			requires(VEG_ALL_OF(VEG_CONCEPT(move_constructible<Ts>))),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator(),
			(... args, Ts))
	const VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)))
			->veg::Tuple<Ts...> {
		return {Direct{}, Ts(VEG_FWD(args))...};
	}

	struct ref {
		template <typename... Ts>
		VEG_NODISCARD VEG_INLINE constexpr auto
		operator()(Ts&&... args) const VEG_NOEXCEPT -> veg::Tuple<Ts&&...> {
			return {Direct{}, static_cast<Ts&&>(VEG_FWD(args))...};
		}
	} ref;

	struct with {
		VEG_TEMPLATE(
				typename... Fns,
				requires(VEG_ALL_OF(VEG_CONCEPT(invocable<Fns>))),
				VEG_NODISCARD VEG_INLINE constexpr auto
				operator(),
				(... args, Fns))
		const VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Fns>)))
				->veg::Tuple<meta::invoke_result_t<Fns>...> {
			return {InPlace{}, VEG_FWD(args)...};
		}
	} with;

	struct fwd {
		VEG_TEMPLATE(
				typename... Ts,
				requires(VEG_ALL_OF(VEG_CONCEPT(move_constructible<Ts>))),
				VEG_NODISCARD VEG_INLINE constexpr auto
				operator(),
				(... args, Ts&&))
		const VEG_NOEXCEPT_IF(
				VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)))
				->veg::Tuple<Ts...> {
			return {Direct{}, Ts(VEG_FWD(args))...};
		}
	} fwd;

	struct zip {

		template <typename... Tuples>
		using PreZip = meta::type_sequence_zip<Tuple, Tuples...>;

		template <typename... Tuples>
		using Zip = meta::detected_t<PreZip, Tuples...>;

		VEG_TEMPLATE(
				(typename... Tuples),
				requires(
						VEG_ALL_OF(VEG_CONCEPT(tuple<Tuples>)) &&
						VEG_CONCEPT(all_same<meta::tuple_size<Tuples>...>) &&
						VEG_ALL_OF(VEG_CONCEPT(move_constructible<Tuples>))),
				VEG_NODISCARD VEG_INLINE constexpr auto
				operator(),
				(... tups, Tuples))
		const VEG_NOEXCEPT_IF(
				VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Tuples>)))
				->Zip<Tuples...> {
			return zip::pre_apply(
					meta::bool_constant<VEG_ALL_OF(
							VEG_CONCEPT(nothrow_move_constructible<Tuples>))>{},
					VEG_FWD(tups)...);
		}

	private:
		template <typename... Tuples>
		VEG_INLINE static constexpr auto
		pre_apply(meta::true_type /*unused*/, Tuples&&... tups) VEG_NOEXCEPT
				-> Zip<Tuples...> {
			return zip::apply(VEG_FWD(tups)...);
		}
		template <typename... Tuples>
		VEG_INLINE static constexpr auto
		pre_apply(meta::false_type /*unused*/, Tuples&&... tups)
				VEG_NOEXCEPT_IF(VEG_NOEXCEPT_IF(
						VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Tuples>))))
						-> Zip<Tuples...> {
			return zip::from_ref_to_result(
					Tag<meta::type_sequence_zip<Tuple, Tuples...>>{},
					zip::apply(

#if __cplusplus >= 201402L
							VEG_FWD(tups).as_ref()
#else
							tuple::as_ref(VEG_FWD(tups))
#endif

									...));
		}

		VEG_INLINE static constexpr auto apply() VEG_NOEXCEPT -> Tuple<> {
			return {};
		}

		template <usize I, typename T>
		struct Helper {
			template <typename... Ts>
			using Type = Tuple<T, meta::tuple_element_t<I, Ts>...>;

			template <typename... Ts>
			VEG_INLINE constexpr auto apply(Ts&&... tups) const VEG_NOEXCEPT_IF(
					VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)))
					-> Type<Ts...> {
				return {
						Direct{},
						VEG_FWD(first),
						static_cast<meta::tuple_element_t<I, Ts>&&>(
								static_cast<TupleLeaf<I, meta::tuple_element_t<I, Ts>>&&>(tups)
										.leaf)...,
				};
			}
			T&& first;
		};

		template <usize... Is, typename... Ts, typename... Tuples>
		VEG_INLINE static constexpr auto apply(
				IndexedTuple<meta::index_sequence<Is...>, Ts...> first,
				Tuples... rest)
				VEG_NOEXCEPT_IF(
						VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)) &&
						VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Tuples>)))
						-> Tuple<                     //
								typename Helper<Is, Ts>:: //
								template Type<Tuples...>...> {
			return {
					((void)first, Direct{}),
					Helper<Is, Ts>{
							static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&&>(first).leaf)}
							.template apply<Tuples...>(VEG_FWD(rest)...)...,
			};
		}

		template <typename ISeq, typename... InnerTargets>
		struct ConverterImpl;
		template <typename OuterTarget>
		struct Converter;

		template <usize... Is, typename... InnerTargets>
		struct ConverterImpl<meta::index_sequence<Is...>, InnerTargets...> {
			IndexedTuple<meta::index_sequence<Is...>, InnerTargets&&...>&& refs;

			VEG_INLINE constexpr auto operator()() const&& VEG_NOEXCEPT_IF(
					VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<InnerTargets>)))
					-> Tuple<InnerTargets...> {

				return {
						InPlace{},
						internal::ConvertingFn<InnerTargets&&, InnerTargets>{
								static_cast<InnerTargets&&>(
										static_cast<TupleLeaf<Is, InnerTargets&&>&>(refs).leaf)}...,
				};
			}
		};

		template <typename... InnerTargets>
		struct Converter<Tuple<InnerTargets...>> {
			using Type = ConverterImpl<
					meta::make_index_sequence<sizeof...(InnerTargets)>,
					InnerTargets...>;
		};

		template <usize... Is, typename... Tups, typename... OuterTargets>
		VEG_INLINE static constexpr auto from_ref_to_result(
				Tag<Tuple<OuterTargets...>> /*tag*/,
				IndexedTuple<meta::index_sequence<Is...>, Tups...> zipped_refs)
				VEG_NOEXCEPT_IF(
						VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<OuterTargets>)))
						-> Tuple<OuterTargets...> {
			return {
					((void)zipped_refs, InPlace{}),
					typename Converter<OuterTargets>::Type{
							static_cast<Tups&&>(
									static_cast<TupleLeaf<Is, Tups>&>(zipped_refs).leaf),
					}...,
			};
		}
	} zip;

private:
	template <usize... Is, typename... Ts>
	VEG_INLINE static constexpr auto
	as_ref(IndexedTuple<meta::index_sequence<Is...>, Ts...>&& tup) VEG_NOEXCEPT
			-> Tuple<Ts&&...> {
		return {
				((void)(tup), Direct{}),
				static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&&>(tup).leaf)...,
		};
	}

public:
	struct cat {

		template <typename... Tuples>
		using PreConcat = meta::type_sequence_cat<Tuple, Tuples...>;
		template <typename... Tuples>
		using Concat = meta::detected_t<PreConcat, Tuples...>;

		VEG_TEMPLATE(
				(typename... Tuples),
				requires(
						VEG_ALL_OF(VEG_CONCEPT(tuple<Tuples>)) &&
						VEG_ALL_OF(VEG_CONCEPT(move_constructible<Tuples>))),
				VEG_NODISCARD VEG_INLINE constexpr auto
				operator(),
				(... tups, Tuples))
		const VEG_NOEXCEPT_IF(
				VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Tuples>)))
				->Concat<Tuples...> {
			return cat::pre_apply(
					meta::bool_constant<VEG_ALL_OF(
							VEG_CONCEPT(nothrow_move_constructible<Tuples>))>{},
					VEG_FWD(tups)...);
		}

	private:
		template <typename... Tuples>
		VEG_INLINE static constexpr auto
		pre_apply(meta::true_type /*unused*/, Tuples&&... tups) VEG_NOEXCEPT
				-> Concat<Tuples...> {
			return cat::apply(VEG_FWD(tups)...);
		}

		template <typename... Tuples>
		VEG_INLINE static constexpr auto
		pre_apply(meta::false_type /*unused*/, Tuples&&... tups) VEG_NOEXCEPT_IF(
				VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Tuples>)))
				-> Concat<Tuples...> {
			return cat::template from_ref_to_result(
					Tag<meta::type_sequence_cat<Tuple, Tuples...>>{},
					cat::apply(

#if __cplusplus >= 201402L
							VEG_FWD(tups).as_ref()
#else
							tuple::as_ref(VEG_FWD(tups))
#endif

									...));
		}

		template <typename... Targets, usize... Is, typename... Refs>
		VEG_INLINE static constexpr auto from_ref_to_result(
				Tag<Tuple<Targets...>> /*tag*/,
				IndexedTuple<meta::index_sequence<Is...>, Refs...> refs)
				VEG_NOEXCEPT_IF(
						VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Targets>)))
						-> veg::Tuple<Targets...> {
			return {
					InPlace{},
					internal::ConvertingFn<Targets&&, Targets>{
							static_cast<Targets&&>(static_cast<Targets&&>(
									static_cast<TupleLeaf<Is, Targets&&>&>(refs).leaf))}...,
			};
		}

		VEG_INLINE static constexpr auto apply() VEG_NOEXCEPT -> Tuple<> {
			return {};
		}

		template <usize... Is, typename... Ts, typename... Tuples>
		VEG_INLINE static constexpr auto apply(
				IndexedTuple<meta::index_sequence<Is...>, Ts...>&& first,
				Tuples&&... rest) VEG_NOEXCEPT
				-> meta::type_sequence_cat<Tuple, Tuple<Ts...>, Tuples...> {
			return cat::apply2(VEG_FWD(first), cat::apply(VEG_FWD(rest)...));
		}

		template <usize... Is, typename... Ts, usize... Js, typename... Us>
		VEG_INLINE static constexpr auto apply2(
				IndexedTuple<meta::index_sequence<Is...>, Ts...>&& first,
				IndexedTuple<meta::index_sequence<Js...>, Us...>&& second) VEG_NOEXCEPT
				-> Tuple<Ts..., Us...> {
			return {
					Direct{},
					static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&&>(first).leaf)...,
					static_cast<Us&&>(static_cast<TupleLeaf<Js, Us>&&>(second).leaf)...,
			};
		}
	} cat;

	struct unpack {
		VEG_TEMPLATE(
				(typename Fn, typename... Ts, usize... Is),
				requires(VEG_CONCEPT(invocable<Fn, Ts&&...>)),
				VEG_INLINE constexpr auto
				operator(),
				(args, IndexedTuple<meta::index_sequence<Is...>, Ts...>),
				(fn, Fn))
		const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn, Ts&&...>))
				->meta::invoke_result_t<Fn, Ts&&...> {

			return VEG_FWD(fn)(
					static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&&>(args).leaf)...);
		}
	} unpack;

	struct for_each_i {
		VEG_TEMPLATE(
				(typename Fn, typename... Ts, usize... Is),
				requires(VEG_ALL_OF(VEG_CONCEPT(invocable<Fn&, Fix<i64{Is}>, Ts&&>))),
				VEG_INLINE VEG_CPP14(constexpr) void
				operator(),
				(args, IndexedTuple<meta::index_sequence<Is...>, Ts...>),
				(fn, Fn))
		const VEG_NOEXCEPT_IF(
				VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Fn&, Fix<i64{Is}>, Ts&&>))) {
			(void)(internal::EmptyArr{
					internal::Empty{},
					((void)(fn(
							 Fix<i64{Is}>{},
							 static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&&>(args).leaf))),
			     internal::Empty{})...,
			});
		}
	} for_each_i;

	struct for_each {
		VEG_TEMPLATE(
				(typename Fn, typename... Ts, usize... Is),
				requires(VEG_ALL_OF(VEG_CONCEPT(invocable<Fn&, Ts&&>))),
				VEG_INLINE VEG_CPP14(constexpr) void
				operator(),
				(args, IndexedTuple<meta::index_sequence<Is...>, Ts...>),
				(fn, Fn))
		const VEG_NOEXCEPT_IF(
				VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Fn&, Ts&&>))) {
			(void)(internal::EmptyArr{
					internal::Empty{},
					((void)(fn(
							 static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&&>(args).leaf))),
			     internal::Empty{})...,
			});
		}
	} for_each;

private:
	template <i64 I, typename Fn, typename Arg>
	struct UnindexedFn {
		Fn&& fn;
		Arg&& arg;
		VEG_INLINE constexpr auto
		operator()() const&& VEG_DEDUCE_RET(VEG_FWD(fn)(Fix<I>{}, VEG_FWD(arg)));
	};
	template <typename Fn, typename Arg>
	struct CurriedFn {
		Fn&& fn;
		Arg&& arg;
		VEG_INLINE constexpr auto
		operator()() const&& VEG_DEDUCE_RET(VEG_FWD(fn)(VEG_FWD(arg)));
	};

public:
	struct map_i {
		VEG_TEMPLATE(
				(typename Fn, typename... Ts, usize... Is),
				requires(VEG_ALL_OF(VEG_CONCEPT(invocable<Fn, Fix<i64{Is}>, Ts&&>))),
				VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
				operator(),
				(args, IndexedTuple<meta::index_sequence<Is...>, Ts...>),
				(fn, Fn))
		const VEG_NOEXCEPT_IF(
				VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Fn, Fix<i64{Is}>, Ts&&>)))
				->Tuple<meta::invoke_result_t<Fn, Fix<i64{Is}>, Ts&&>...> {
			return {
					InPlace{},
					UnindexedFn<Is, Fn, Ts&&>{
							VEG_FWD(fn),
							static_cast<Ts&&>(
									static_cast<TupleLeaf<Is, Ts>&&>(args).leaf)}...,
			};
		}
	} map_i;

	struct map {

		VEG_TEMPLATE(
				(typename Fn, typename... Ts, usize... Is),
				requires(VEG_ALL_OF(VEG_CONCEPT(invocable<Fn, Ts&&>))),
				VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
				operator(),
				(args, IndexedTuple<meta::index_sequence<Is...>, Ts...>),
				(fn, Fn))
		const VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_invocable<Fn, Ts&&>)))
				->Tuple<meta::invoke_result_t<Fn, Ts&&>...> {
			return {
					InPlace{},
					CurriedFn<Fn, Ts&&>{
							VEG_FWD(fn),
							static_cast<Ts&&>(
									static_cast<TupleLeaf<Is, Ts>&&>(args).leaf)}...,
			};
		}
	} map;
};
} // namespace nb
VEG_NIEBLOID(tuple);

namespace cpo {
template <typename... Ts>
struct is_trivially_relocatable<Tuple<Ts...>>
		: meta::bool_constant<(VEG_ALL_OF(is_trivially_relocatable<Ts>::value))> {};
template <typename... Ts>
struct is_trivially_constructible<Tuple<Ts...>>
		: meta::bool_constant<(VEG_ALL_OF(is_trivially_constructible<Ts>::value))> {
};
template <typename... Ts>
struct is_trivially_swappable<Tuple<Ts...>>
		: meta::bool_constant<(VEG_ALL_OF(is_trivially_swappable<Ts>::value))> {};
} // namespace cpo

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
						 static_cast<internal::tup_::TupleLeaf<Is, Ts> const&>(tup).leaf),
		     internal::Empty{})...};
		out.insert(out.size(), "}", 1);
	}

	static void to_string(fmt::Buffer& out, Tuple<Ts...> const& tup) {
		Debug::to_string_impl(out, tup);
	}
};
} // namespace veg

namespace std {
template <typename... Ts>
struct tuple_size<veg::Tuple<Ts...>>
		: integral_constant<veg::usize, sizeof...(Ts)> {};

template <veg::usize I, typename... Ts>
struct tuple_element<I, veg::Tuple<Ts...>> {
	using type = veg::ith<I, Ts...>;
};
} // namespace std

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TUPLE_HPP_B8PHUNWES */

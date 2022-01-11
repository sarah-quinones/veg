#ifndef VEG_TUPLE_HPP_B8PHUNWES
#define VEG_TUPLE_HPP_B8PHUNWES

#include "veg/type_traits/assignable.hpp"
#include "veg/internal/fmt.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/util/get.hpp"
#include "veg/internal/fix_index.hpp"
#include "veg/internal/prologue.hpp"

#if defined(__GLIBCXX__)
namespace std /* NOLINT */ {
_GLIBCXX_BEGIN_NAMESPACE_VERSION
template <typename T>
struct tuple_size;
template <::veg::usize, typename T>
struct tuple_element;
_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std
#else
#include <utility> // std::tuple_{size,element}
#endif

/******************************************************************************/
#define __VEG_IMPL_BIND(I, Tuple, Identifier) /* NOLINT */                     \
	auto&& Identifier /* NOLINT */ = ::veg::nb::get<I>{}(VEG_FWD(Tuple));

#define __VEG_IMPL_BIND_ID_SEQ(/* NOLINT */                                    \
                               CV_Auto,                                        \
                               Identifiers,                                    \
                               Tuple,                                          \
                               Tuple_Size,                                     \
                               TupleId)                                        \
	CV_Auto TupleId = Tuple;                                                     \
	static_assert(                                                               \
			::std::tuple_size<                                                       \
					typename ::veg::meta::uncvref_t<decltype(TupleId)>>::value ==        \
					(Tuple_Size),                                                        \
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
template <typename T, usize I>
using inner_ith = decltype(VEG_DECLVAL(T)[Fix<isize{I}>{}]);

template <typename... Ts>
struct Tuple;

namespace tuple {
namespace nb {
struct tuplify {
	template <typename... Ts>
	VEG_NODISCARD VEG_INLINE constexpr auto operator()(Ts... args) const
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Ts>)))
					-> veg::Tuple<Ts...> {
		return {tuplify{}, Ts(VEG_FWD(args))...};
	}
};
} // namespace nb
VEG_NIEBLOID(tuplify);
} // namespace tuple

inline namespace tags {
using Tuplify = tuple::nb::tuplify;
using tuple::tuplify;
} // namespace tags

namespace tuple {
using veg::Tuple;
} // namespace tuple

namespace tuple {
template <typename ISeq, typename... Ts>
struct IndexedTuple;

#if VEG_HAS_NO_UNIQUE_ADDRESS
#define __VEG_IMPL_LEAF_GET() /* NOLINT */ leaf

template <usize I, typename T>
struct TupleLeaf {
	VEG_NO_UNIQUE_ADDRESS T leaf;
};
#else
#define __VEG_IMPL_LEAF_GET() /* NOLINT */ leaf_get()

template <typename T, bool = (VEG_CONCEPT(empty<T>) && !VEG_CONCEPT(final<T>))>
struct TupleLeafImpl;

template <typename T>
struct TupleLeafImpl<T, true> : T {
	template <typename Fn>
	VEG_INLINE constexpr TupleLeafImpl(InPlace<void> /*tag*/, Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)())
			: T{VEG_FWD(fn)()} {}
	TupleLeafImpl() = default;
	VEG_INLINE constexpr auto leaf_get() const VEG_NOEXCEPT -> T& {
		return const_cast<T&>(static_cast<T const&>(*this));
	}
};
template <typename T>
struct TupleLeafImpl<T, false> {
	T leaf;

	template <typename Fn>
	VEG_INLINE constexpr TupleLeafImpl(InPlace<void> /*tag*/, Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)())
			: leaf{VEG_FWD(fn)()} {}
	TupleLeafImpl() = default;

	VEG_INLINE constexpr auto leaf_get() const VEG_NOEXCEPT -> T& {
		return const_cast<T&>(leaf);
	}
};

template <usize I, typename T>
struct TupleLeaf : TupleLeafImpl<T> {
	using TupleLeafImpl<T>::TupleLeafImpl;
};
#endif

namespace nb {
struct unpack {
	VEG_TEMPLATE(
			(typename Fn, typename... Ts, usize... Is),
			requires(VEG_CONCEPT(
					fn_once<Fn, veg::meta::invoke_result_t<Fn, Ts&&...>, Ts&&...>)),
			VEG_INLINE constexpr auto
			operator(),
			(args, IndexedTuple<veg::meta::index_sequence<Is...>, Ts...>&&),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<
																		Fn,
																		veg::meta::invoke_result_t<Fn, Ts&&...>,
																		Ts&&...>))
			->veg::meta::invoke_result_t<Fn, Ts&&...> {

		return VEG_FWD(fn)(static_cast<Ts&&>(
				static_cast<TupleLeaf<Is, Ts>&&>(args).__VEG_IMPL_LEAF_GET())...);
	}
};

struct for_each_i {
	VEG_TEMPLATE(
			(typename Fn, typename... Ts, usize... Is),
			requires(VEG_ALL_OF(VEG_CONCEPT(fn_mut<Fn, void, Fix<isize{Is}>, Ts&&>))),
			VEG_INLINE VEG_CPP14(constexpr) void
			operator(),
			(args, IndexedTuple<veg::meta::index_sequence<Is...>, Ts...>&&),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_fn_mut<Fn, void, Fix<isize{Is}>, Ts&&>))) {
		VEG_EVAL_ALL(
				fn(Fix<isize{Is}>{},
		       static_cast<Ts&&>(
							 static_cast<TupleLeaf<Is, Ts>&&>(args).__VEG_IMPL_LEAF_GET())));
	}
};

struct for_each {
	VEG_TEMPLATE(
			(typename Fn, typename... Ts, usize... Is),
			requires(VEG_ALL_OF(VEG_CONCEPT(fn_mut<Fn, void, Ts&&>))),
			VEG_INLINE VEG_CPP14(constexpr) void
			operator(),
			(args, IndexedTuple<veg::meta::index_sequence<Is...>, Ts...>&&),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_fn_mut<Fn, void, Ts&&>))) {
		VEG_EVAL_ALL(fn(static_cast<Ts&&>(
				static_cast<TupleLeaf<Is, Ts>&&>(args).__VEG_IMPL_LEAF_GET())));
	}
};

struct map_i {
	VEG_TEMPLATE(
			(typename Fn, typename... Ts, usize... Is),
			requires(VEG_ALL_OF(
					VEG_CONCEPT(fn_mut< //
											inner_ith<Fn&, Is>,
											veg::meta::invoke_result_t<inner_ith<Fn&, Is>, Ts>,
											Ts>))),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(args, IndexedTuple<veg::meta::index_sequence<Is...>, Ts...>&&),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_fn_mut< //
														 inner_ith<Fn&, Is>,
														 veg::meta::invoke_result_t<inner_ith<Fn&, Is>, Ts>,
														 Ts>)))
			->Tuple<veg::meta::invoke_result_t<inner_ith<Fn&, Is>, Ts>...> {
		return {
				inplace[tuplify{}],
				_detail::WithArg<inner_ith<Fn&, Is>, Ts>{
						fn[Fix<isize{Is}>{}],
						static_cast<Ts&&>(static_cast<TupleLeaf<Is, Ts>&>(args)
		                              .__VEG_IMPL_LEAF_GET())}...};
	}
};

struct map {
	VEG_TEMPLATE(
			(typename Fn, typename... Ts, usize... Is),
			requires(VEG_ALL_OF(VEG_CONCEPT(
					fn_mut<Fn, veg::meta::invoke_result_t<Fn&, Ts&&>, Ts&&>))),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(args, IndexedTuple<veg::meta::index_sequence<Is...>, Ts...>&&),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(
					nothrow_fn_mut<Fn, veg::meta::invoke_result_t<Fn&, Ts&&>, Ts&&>)))
			->Tuple<veg::meta::invoke_result_t<Fn&, Ts&&>...> {
		return {
				inplace[tuplify{}],
				_detail::WithArg<Fn&, Ts&&>{
						fn,
						static_cast<Ts&&>(
								static_cast<TupleLeaf<Is, Ts>&&>(args).__VEG_IMPL_LEAF_GET()),
				}...,
		};
	}
};
} // namespace nb

template <usize... Is, typename... Ts>
struct IndexedTuple<meta::index_sequence<Is...>, Ts...> : TupleLeaf<Is, Ts>... {

	IndexedTuple() = default;

#if VEG_HAS_NO_UNIQUE_ADDRESS
#define __VEG_IMPL_FWD /* NOLINT */ Ts(VEG_FWD(args))
#define __VEG_IMPL_INPLACE /* NOLINT */ Ts(VEG_FWD(fns)())
#else
#define __VEG_IMPL_FWD /* NOLINT */ inplace, _detail::MoveFn<Ts>{VEG_FWD(args)},
#define __VEG_IMPL_INPLACE /* NOLINT */ inplace, VEG_FWD(fns)
#endif

	VEG_INLINE constexpr IndexedTuple(Tuplify /*tag*/, Ts... args)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Ts>)))
			: TupleLeaf<Is, Ts>{__VEG_IMPL_FWD}... {}

	VEG_TEMPLATE(
			(typename _, typename... Fns),
			requires(
					VEG_CONCEPT(same<_, Tuplify>) &&
					VEG_ALL_OF(VEG_CONCEPT(fn_once<Fns, Ts>))),
			VEG_INLINE constexpr IndexedTuple,
			(/*tag*/, InPlace<_>),
			(... fns, Fns))
	VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_fn_once<Fns, Ts>)))
			: TupleLeaf<Is, Ts>{__VEG_IMPL_INPLACE}... {}

#undef __VEG_IMPL_FWD
#undef __VEG_IMPL_INPLACE

	VEG_EXPLICIT_COPY(IndexedTuple);

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() const
			& VEG_NOEXCEPT -> Tuple<Ref<Ts>...> {
		return {
				tuplify,
				ref(static_cast<TupleLeaf<Is, Ts> const&>(*this)
		            .__VEG_IMPL_LEAF_GET())...,
		};
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_mut() VEG_NOEXCEPT
			-> Tuple<RefMut<Ts>...> {
		return {
				tuplify,
				mut(static_cast<TupleLeaf<Is, Ts>&>(*this).__VEG_IMPL_LEAF_GET())...,
		};
	}

	template <isize I>
	void operator[](Fix<I> /*arg*/) const&& = delete;

	VEG_TEMPLATE(
			(isize I),
			requires(usize{I} < sizeof...(Ts)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>)) &&
			VEG_NOEXCEPT -> ith<usize{I}, Ts...> {
		using Ti = ith<usize{I}, Ts...>;

		return static_cast<Ti&&>(
				static_cast<TupleLeaf<usize{I}, Ti>&&>(*this).__VEG_IMPL_LEAF_GET());
	}

	VEG_TEMPLATE(
			(isize I),
			requires(usize{I} < sizeof...(Ts)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>)) &
			VEG_NOEXCEPT -> ith<usize{I}, Ts...>& {
		return static_cast<TupleLeaf<usize{I}, ith<usize{I}, Ts...>>&>(*this)
		    .__VEG_IMPL_LEAF_GET();
	}

	VEG_TEMPLATE(
			(isize I),
			requires(usize{I} < sizeof...(Ts)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
			operator[],
			(/*arg*/, Fix<I>))
	const & VEG_NOEXCEPT->ith<usize{I}, Ts...> const& {
		return static_cast<TupleLeaf<usize{I}, ith<usize{I}, Ts...>> const&>(*this)
		    .__VEG_IMPL_LEAF_GET();
	}
};
} // namespace tuple

namespace _detail {
namespace _tuple {

struct cmp_impl {
	template <usize... Is, typename... Ts, typename... Us>
	static constexpr auto
	eq(tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...> const& lhs,
	   tuple::IndexedTuple<meta::index_sequence<Is...>, Us...> const& rhs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_eq<Ts, Us>))) -> bool {
#if VEG_HAS_FOLD_EXPR
		return VEG_ALL_OF((
				static_cast<Ts const&>(static_cast<tuple::TupleLeaf<Is, Ts> const&>(lhs)
		                               .__VEG_IMPL_LEAF_GET()) ==
				static_cast<Us const&>(static_cast<tuple::TupleLeaf<Is, Us> const&>(rhs)
		                               .__VEG_IMPL_LEAF_GET())));
#else
		return CmpImpl<Ts...>::eq(
				static_cast<tuple::TupleLeaf<Is, Ts> const&>(lhs)
						.__VEG_IMPL_LEAF_GET()...,
				static_cast<tuple::TupleLeaf<Is, Us> const&>(rhs)
						.__VEG_IMPL_LEAF_GET()...);
#endif
	}

	template <usize... Is, typename... Ts, typename... Us>
	static constexpr auto
	cmp(tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...> const& lhs,
	    tuple::IndexedTuple<meta::index_sequence<Is...>, Us...> const& rhs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_ord<Ts, Us>)))
					-> cmp::Ordering {
		return CmpImpl<Ts...>::cmp(
				static_cast<tuple::TupleLeaf<Is, Ts> const&>(lhs)
						.__VEG_IMPL_LEAF_GET()...,
				static_cast<tuple::TupleLeaf<Is, Us> const&>(rhs)
						.__VEG_IMPL_LEAF_GET()...);
	}
};
} // namespace _tuple
} // namespace _detail

namespace _detail {
namespace meta_ {

struct NonTupleBaseInfoImpl {
	static constexpr bool is_tuple = false;
	static constexpr usize size = 0;
	template <usize I>
	using ith = void;
	using seq = void;
};

template <typename... Ts>
struct TupleBaseInfoImpl {
	static constexpr bool is_tuple = true;
	static constexpr usize size = sizeof...(Ts);
	template <usize I>
	using ith = veg::ith<I, Ts...>;
	using seq = meta::type_sequence<Ts...>;
	using Tuple = veg::Tuple<Ts...>;
	using IndexedTuple =
			veg::tuple::IndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...>;
};

struct is_tuple_helper {
	static auto test(void*) -> NonTupleBaseInfoImpl;
	template <usize... Is, typename... Ts>
	static auto test(tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>*)
			-> TupleBaseInfoImpl<Ts...>;
};

template <typename T>
struct IndexedToTuple;

template <usize... Is, typename... Ts>
struct IndexedToTuple<tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>> {
	using Type = Tuple<Ts...>;
};
} // namespace meta_
} // namespace _detail

namespace tuple {
namespace meta {
template <typename T>
using TupleBaseInfo =
		decltype(_detail::meta_::is_tuple_helper::test(static_cast<T*>(nullptr)));

template <typename T>
using is_tuple = veg::meta::bool_constant<TupleBaseInfo<T>::is_tuple>;
template <typename T>
using tuple_size = veg::meta::constant<usize, TupleBaseInfo<T>::size>;

template <usize I, typename T>
using tuple_element = typename TupleBaseInfo<T>::template ith<I>;

} // namespace meta
} // namespace tuple

namespace concepts {
namespace tuple {
VEG_DEF_CONCEPT(typename T, tuple, veg::tuple::meta::is_tuple<T>::value);
} // namespace tuple
} // namespace concepts

template <typename... Ts>
struct Tuple
		: tuple::IndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...> {

	using Indexed =
			tuple::IndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...>;

	using Indexed::Indexed;

	VEG_EXPLICIT_COPY(Tuple);
};

VEG_CPP17(template <typename... Ts> Tuple(Tuplify, Ts...) -> Tuple<Ts...>;)
namespace tuple {

template <usize I, usize... Is, typename... Ts>
VEG_NODISCARD VEG_INLINE constexpr auto
get(tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Ts...> const& tup)
		VEG_NOEXCEPT -> ith<I, Ts...> const& {
	return static_cast<tuple::TupleLeaf<I, ith<I, Ts...>> const&>(tup)
	    .__VEG_IMPL_LEAF_GET();
}
template <usize I, usize... Is, typename... Ts>
VEG_NODISCARD VEG_INLINE constexpr auto
get(tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Ts...>& tup)
		VEG_NOEXCEPT -> ith<I, Ts...>& {
	return static_cast<tuple::TupleLeaf<I, ith<I, Ts...>>&>(tup)
	    .__VEG_IMPL_LEAF_GET();
}
template <usize I, usize... Is, typename... Ts>
VEG_NODISCARD VEG_INLINE constexpr auto
get(tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Ts...> const&& tup)
		VEG_NOEXCEPT -> ith<I, Ts...> const&& {
	return static_cast<ith<I, Ts...> const&&>(
			static_cast<tuple::TupleLeaf<I, ith<I, Ts...>> const&&>(tup)
					.__VEG_IMPL_LEAF_GET());
}
template <usize I, usize... Is, typename... Ts>
VEG_NODISCARD VEG_INLINE constexpr auto
get(tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Ts...>&& tup)
		VEG_NOEXCEPT -> ith<I, Ts...>&& {
	return static_cast<ith<I, Ts...>&&>(
			static_cast<tuple::TupleLeaf<I, ith<I, Ts...>>&&>(tup)
					.__VEG_IMPL_LEAF_GET());
}

/// short-circuiting equality test of all the members in order
VEG_TEMPLATE(
		(typename... Ts, typename... Us, usize... Is),
		requires(VEG_ALL_OF(VEG_CONCEPT(eq<Ts, Us>))),
		VEG_NODISCARD VEG_INLINE constexpr auto
		operator==,
		(lhs, tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Ts...> const&),
		(rhs, tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Us...> const&))
VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_eq<Ts, Us>)))->bool {
	return _detail::_tuple::cmp_impl::eq(lhs, rhs);
}

VEG_TEMPLATE(
		(typename... Ts, typename... Us, usize... Is),
		requires(VEG_ALL_OF(VEG_CONCEPT(eq<Ts, Us>))),
		VEG_NODISCARD VEG_INLINE constexpr auto
		operator!=,
		(lhs, tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Ts...> const&),
		(rhs, tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Us...> const&))
VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_eq<Ts, Us>)))->bool {
	return !tuple::operator==(lhs, rhs);
}
} // namespace tuple

namespace _detail {
namespace _tuple {
template <usize... Is, typename... Ts>
VEG_INLINE static constexpr auto tuple_fwd(
		tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>&& tup) VEG_NOEXCEPT
		-> Tuple<Ts&&...> {
	return {
			((void)(tup), tuplify),
			static_cast<Ts&&>(static_cast<tuple::TupleLeaf<Is, Ts>&&>(tup)
	                          .__VEG_IMPL_LEAF_GET())...,
	};
}

} // namespace _tuple
} // namespace _detail

namespace tuple {
namespace nb {
struct with {
	VEG_TEMPLATE(
			typename... Fns,
			requires(VEG_ALL_OF(
					VEG_CONCEPT(fn_once<Fns, veg::meta::invoke_result_t<Fns>>))),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator(),
			(... args, Fns))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(
					VEG_CONCEPT(nothrow_fn_once<Fns, veg::meta::invoke_result_t<Fns>>)))
			->veg::Tuple<veg::meta::invoke_result_t<Fns>...> {
		return {inplace[tuplify{}], VEG_FWD(args)...};
	}
};

struct zip {

	template <typename... Tuples>
	using PreZip = veg::meta::type_sequence_zip<Tuple, Tuples...>;

	template <typename... Tuples>
	using Zip = veg::meta::
			detected_t<PreZip, typename meta::TupleBaseInfo<Tuples>::Tuple...>;

	VEG_TEMPLATE(
			(typename... Tuples),
			requires(
					VEG_ALL_OF(VEG_CONCEPT(tuple::tuple<Tuples>)) &&
					VEG_CONCEPT(all_same<tuple::meta::tuple_size<Tuples>...>)),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator(),
			(... tups, Tuples))
	const VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Tuples>)))
			->Zip<Tuples...> {
		return zip::pre_apply(
				veg::meta::bool_constant<VEG_ALL_OF(
						VEG_CONCEPT(trivially_copyable<Tuples>))>{},
				static_cast<typename meta::TupleBaseInfo<Tuples>::IndexedTuple&&>(
						tups)...);
	}

private:
	template <typename... Tuples>
	VEG_INLINE static constexpr auto
	pre_apply(veg::meta::true_type /*unused*/, Tuples&&... tups) VEG_NOEXCEPT
			-> Zip<Tuples...> {
		return zip::apply(VEG_FWD(tups)...);
	}
	template <typename... Tuples>
	VEG_INLINE static constexpr auto
	pre_apply(veg::meta::false_type /*unused*/, Tuples&&... tups) VEG_NOEXCEPT_IF(
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Tuples>))))
			-> Zip<Tuples...> {
		return zip::from_ref_to_result(
				Tag<veg::meta::type_sequence_zip<
						Tuple,
						typename _detail::meta_::IndexedToTuple<Tuples>::Type...>>{},
				zip::apply(_detail::_tuple::tuple_fwd(VEG_FWD(tups))...));
	}

	VEG_INLINE static constexpr auto apply() VEG_NOEXCEPT -> Tuple<> {
		return {};
	}

	template <usize I, typename T>
	struct Helper {
		template <typename... Ts>
		using Type = Tuple<T, meta::tuple_element<I, Ts>...>;

		template <typename... Ts>
		VEG_INLINE constexpr auto apply(Ts&&... tups) const VEG_NOEXCEPT
				-> Type<Ts...> {
			return {
					tuplify{},
					VEG_FWD(first),
					static_cast<meta::tuple_element<I, Ts>&&>(
							static_cast<TupleLeaf<I, meta::tuple_element<I, Ts>>&&>(tups)
									.__VEG_IMPL_LEAF_GET())...,
			};
		}
		T&& first;
	};

	template <usize... Is, typename... Ts, typename... Tuples>
	VEG_INLINE static constexpr auto apply(
			IndexedTuple<veg::meta::index_sequence<Is...>, Ts...> first,
			Tuples... rest) VEG_NOEXCEPT -> Tuple< //
			typename Helper<Is, Ts>::              //
			template Type<Tuples...>...> {
		return {
				((void)first, tuplify{}),
				Helper<Is, Ts>{
						static_cast<Ts&&>(
								static_cast<TupleLeaf<Is, Ts>&&>(first).__VEG_IMPL_LEAF_GET())}
						.template apply<Tuples...>(VEG_FWD(rest)...)...,
		};
	}

	template <typename ISeq, typename... InnerTargets>
	struct ConverterImpl;
	template <typename OuterTarget>
	struct Converter;

	template <usize... Is, typename... InnerTargets>
	struct ConverterImpl<veg::meta::index_sequence<Is...>, InnerTargets...> {
		IndexedTuple<veg::meta::index_sequence<Is...>, InnerTargets&&...>&& refs;

		VEG_INLINE constexpr auto operator()() const&& VEG_NOEXCEPT_IF(VEG_ALL_OF(
				VEG_CONCEPT(nothrow_movable<InnerTargets>))) -> Tuple<InnerTargets...> {

			return {
					inplace[tuplify{}],
					_detail::MoveFn<InnerTargets>{static_cast<InnerTargets&&>(
							static_cast<TupleLeaf<Is, InnerTargets&&>&>(refs)
									.__VEG_IMPL_LEAF_GET())}...,
			};
		}
	};

	template <typename... InnerTargets>
	struct Converter<Tuple<InnerTargets...>> {
		using Type = ConverterImpl<
				veg::meta::make_index_sequence<sizeof...(InnerTargets)>,
				InnerTargets...>;
	};

	template <usize... Is, typename... Tups, typename... OuterTargets>
	VEG_INLINE static constexpr auto from_ref_to_result(
			Tag<Tuple<OuterTargets...>> /*tag*/,
			IndexedTuple<veg::meta::index_sequence<Is...>, Tups...> zipped_refs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<OuterTargets>)))
					-> Tuple<OuterTargets...> {
		return {
				((void)zipped_refs, inplace[tuplify{}]),
				typename Converter<OuterTargets>::Type{
						static_cast<Tups&&>(static_cast<TupleLeaf<Is, Tups>&>(zipped_refs)
		                                .__VEG_IMPL_LEAF_GET()),
				}...,
		};
	}
};

struct cat {

	template <typename... Tuples>
	using PreConcat = veg::meta::type_sequence_cat<Tuple, Tuples...>;
	template <typename... Tuples>
	using Concat = veg::meta::
			detected_t<PreConcat, typename meta::TupleBaseInfo<Tuples>::Tuple...>;

	VEG_TEMPLATE(
			(typename... Tuples),
			requires(VEG_ALL_OF(VEG_CONCEPT(tuple::tuple<Tuples>))),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator(),
			(... tups, Tuples))
	const VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Tuples>)))
			->Concat<Tuples...> {
		return cat::pre_apply(
				veg::meta::bool_constant<VEG_ALL_OF(
						VEG_CONCEPT(trivially_copyable<Tuples>))>{},
				static_cast<typename meta::TupleBaseInfo<Tuples>::IndexedTuple&&>(
						tups)...);
	}

private:
	template <typename... Tuples>
	VEG_INLINE static constexpr auto
	pre_apply(veg::meta::true_type /*unused*/, Tuples&&... tups) VEG_NOEXCEPT
			-> Concat<Tuples...> {
		return cat::apply(VEG_FWD(tups)...);
	}

	template <typename... Tuples>
	VEG_INLINE static constexpr auto
	pre_apply(veg::meta::false_type /*unused*/, Tuples&&... tups)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Tuples>)))
					-> Concat<Tuples...> {
		return cat::template from_ref_to_result(
				Tag<veg::meta::type_sequence_cat<Tuple, Tuples...>>{},
				cat::apply(_detail::_tuple::tuple_fwd(VEG_FWD(tups))...));
	}

	template <typename... Targets, usize... Is, typename... Refs>
	VEG_INLINE static constexpr auto from_ref_to_result(
			Tag<Tuple<Targets...>> /*tag*/,
			IndexedTuple<veg::meta::index_sequence<Is...>, Refs...> refs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Targets>)))
					-> veg::Tuple<Targets...> {
		return {
				inplace[tuplify{}],
				_detail::MoveFn<Targets>{static_cast<Targets&&>(
						static_cast<Targets&&>(static_cast<TupleLeaf<Is, Targets&&>&>(refs)
		                                   .__VEG_IMPL_LEAF_GET()))}...,
		};
	}

	VEG_INLINE static constexpr auto apply() VEG_NOEXCEPT -> Tuple<> {
		return {};
	}

	template <usize... Is, typename... Ts, typename... Tuples>
	VEG_INLINE static constexpr auto apply(
			IndexedTuple<veg::meta::index_sequence<Is...>, Ts...>&& first,
			Tuples&&... rest) VEG_NOEXCEPT
			-> veg::meta::type_sequence_cat<
					Tuple,
					Tuple<Ts...>,
					typename _detail::meta_::IndexedToTuple<Tuples>::Type...> {
		return cat::apply2(VEG_FWD(first), cat::apply(VEG_FWD(rest)...));
	}

	template <usize... Is, typename... Ts, usize... Js, typename... Us>
	VEG_INLINE static constexpr auto apply2(
			IndexedTuple<veg::meta::index_sequence<Is...>, Ts...>&& first,
			IndexedTuple<veg::meta::index_sequence<Js...>, Us...>&& second)
			VEG_NOEXCEPT -> Tuple<Ts..., Us...> {
		return {
				tuplify{},
				static_cast<Ts&&>(
						static_cast<TupleLeaf<Is, Ts>&&>(first).__VEG_IMPL_LEAF_GET())...,
				static_cast<Us&&>(
						static_cast<TupleLeaf<Js, Us>&&>(second).__VEG_IMPL_LEAF_GET())...,
		};
	}
};

struct deref_assign {
	VEG_TEMPLATE(
			(typename... Ts, typename... Us, usize... Is),
			requires(VEG_ALL_OF(VEG_CONCEPT(assignable<Ts&, Us const&>))),
			VEG_INLINE VEG_CPP14(constexpr) void
			operator(),
			(ts, IndexedTuple<veg::meta::index_sequence<Is...>, RefMut<Ts>...>),
			(us, IndexedTuple<veg::meta::index_sequence<Is...>, Ref<Us>...>))
	const VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_assignable<Ts&, Us const&>))) {
		VEG_EVAL_ALL(
				static_cast<TupleLeaf<Is, RefMut<Ts>>&>(ts)
						.__VEG_IMPL_LEAF_GET()
						.get() = static_cast<TupleLeaf<Is, Ref<Us>>&>(us)
		                     .__VEG_IMPL_LEAF_GET()
		                     .get());
	}
};
} // namespace nb
VEG_NIEBLOID(with);

VEG_NIEBLOID(zip);
VEG_NIEBLOID(cat);

VEG_NIEBLOID(unpack);

VEG_NIEBLOID(for_each);
VEG_NIEBLOID(for_each_i);
VEG_NIEBLOID(map);
VEG_NIEBLOID(map_i);

VEG_NIEBLOID(deref_assign);
} // namespace tuple

namespace cpo {
template <usize... Is, typename... Ts>
struct is_trivially_relocatable<
		tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>>
		: meta::bool_constant<(VEG_ALL_OF(is_trivially_relocatable<Ts>::value))> {};
template <usize... Is, typename... Ts>
struct is_trivially_constructible<
		tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>>
		: meta::bool_constant<(VEG_ALL_OF(is_trivially_constructible<Ts>::value))> {
};

template <typename... Ts>
struct is_trivially_relocatable<tuple::Tuple<Ts...>>
		: meta::bool_constant<(VEG_ALL_OF(is_trivially_relocatable<Ts>::value))> {};
template <typename... Ts>
struct is_trivially_constructible<tuple::Tuple<Ts...>>
		: meta::bool_constant<(VEG_ALL_OF(is_trivially_constructible<Ts>::value))> {
};
} // namespace cpo

namespace _detail {
namespace _tuple {
template <typename... Ts, typename... Us, usize... Is>
VEG_NODISCARD VEG_INLINE constexpr auto
ord(tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Ts...> const& lhs,
    tuple::IndexedTuple<veg::meta::index_sequence<Is...>, Us...> const& rhs)
		VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_ord<Ts, Us>)))
				-> cmp::Ordering {
	return _detail::_tuple::cmp_impl::cmp(lhs, rhs);
}
struct OrdTupleBase {
	VEG_TEMPLATE(
			(typename... Ts, typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(ord<Ts, Us>))),
			VEG_NODISCARD VEG_INLINE static constexpr auto cmp,
			(lhs, Ref<Tuple<Ts...>>),
			(rhs, Ref<Tuple<Us...>>))
	VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_ord<Ts, Us>)))->cmp::Ordering {
		return _tuple::ord(lhs.get(), rhs.get());
	}
};
struct OrdITupleBase {
	VEG_TEMPLATE(
			(usize... Is, typename... Ts, typename... Us),
			requires(VEG_ALL_OF(VEG_CONCEPT(ord<Ts, Us>))),
			VEG_NODISCARD VEG_INLINE static constexpr auto cmp,
			(lhs, Ref<tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>>),
			(rhs, Ref<tuple::IndexedTuple<meta::index_sequence<Is...>, Us...>>))
	VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_ord<Ts, Us>)))->cmp::Ordering {
		return _tuple::ord(lhs.get(), rhs.get());
	}
};

template <typename... Ts, usize... Is>
static void to_string_impl(
		fmt::BufferMut out,
		tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...> const& tup) {
	_detail::_fmt::DbgStructScope _{VEG_FWD(out)};
	VEG_EVAL_ALL(
			(_.out.append_ln(),
	     fmt::Debug<Ts>::to_string(
					 VEG_FWD(_.out),
					 ref(static_cast<Ts const&>(
							 static_cast<tuple::TupleLeaf<Is, Ts> const&>(tup)
									 .__VEG_IMPL_LEAF_GET()))),
	     _.out.append_literal(",")));
}

struct DebugITupleBase {
	template <usize... Is, typename... Ts>
	static void to_string(
			fmt::BufferMut out,
			Ref<tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>> tup) {
		_tuple::to_string_impl(VEG_FWD(out), tup.get());
	}
};
struct DebugTupleBase {
	template <typename... Ts>
	static void to_string(fmt::BufferMut out, Ref<Tuple<Ts...>> tup) {
		_tuple::to_string_impl(VEG_FWD(out), tup.get());
	}
};
} // namespace _tuple
} // namespace _detail

template <typename... Ts, typename... Us>
struct cmp::Ord<Tuple<Ts...>, Tuple<Us...>> : _detail::_tuple::OrdTupleBase {};
template <usize... Is, typename... Ts, typename... Us>
struct cmp::Ord<
		tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>,
		tuple::IndexedTuple<meta::index_sequence<Is...>, Us...>>
		: _detail::_tuple::OrdITupleBase {};

template <typename... Ts>
struct fmt::Debug<Tuple<Ts...>> : _detail::_tuple::DebugTupleBase {};
template <usize... Is, typename... Ts>
struct fmt::Debug<tuple::IndexedTuple<meta::index_sequence<Is...>, Ts...>>
		: _detail::_tuple::DebugITupleBase {};
} // namespace veg

template <typename... Ts>
struct std::tuple_size<veg::Tuple<Ts...>>
		: ::veg::meta::constant<veg::usize, sizeof...(Ts)> {};
template <veg::usize I, typename... Ts>
struct std::tuple_element<I, veg::Tuple<Ts...>> {
	using type = veg::ith<I, Ts...>;
};

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TUPLE_HPP_B8PHUNWES */

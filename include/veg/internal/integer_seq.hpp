#ifndef VEG_INTEGER_SEQ_HPP_JBT0EKAQS
#define VEG_INTEGER_SEQ_HPP_JBT0EKAQS

#include "veg/internal/typedefs.hpp"
#include "veg/internal/macros.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
using usize = decltype(sizeof(0));
namespace internal {

template <i64 I>
struct EmptyI {};

using Empty = EmptyI<0>;
using EmptyArr = Empty[];
namespace meta_ {

template <typename T, T... Nums>
struct integer_sequence;

#if VEG_HAS_BUILTIN(__make_integer_seq)

template <typename T, T N>
using make_integer_sequence = __make_integer_seq<integer_sequence, T, N>;

#elif __GNUC__ >= 8

template <typename T, T N>
using make_integer_sequence = integer_sequence<T, __integer_pack(N)...>;

#else

namespace internal {

template <typename Seq1, typename Seq2>
struct _merge;

template <typename Seq1, typename Seq2>
struct _merge_p1;

template <typename T, T... Nums1, T... Nums2>
struct _merge<integer_sequence<T, Nums1...>, integer_sequence<T, Nums2...>> {
	using type = integer_sequence<T, Nums1..., (sizeof...(Nums1) + Nums2)...>;
};

template <typename T, T... Nums1, T... Nums2>
struct _merge_p1<integer_sequence<T, Nums1...>, integer_sequence<T, Nums2...>> {
	using type = integer_sequence<
			T,
			Nums1...,
			(sizeof...(Nums1) + Nums2)...,
			sizeof...(Nums1) + sizeof...(Nums2)>;
};

template <typename T, usize N, bool Even = (N % 2) == 0>
struct _make_integer_sequence {
	using type = typename _merge<
			typename _make_integer_sequence<T, N / 2>::type,
			typename _make_integer_sequence<T, N / 2>::type>::type;
};

template <typename T, usize N>
struct _make_integer_sequence<T, N, false> {
	using type = typename _merge_p1<
			typename _make_integer_sequence<T, N / 2>::type,
			typename _make_integer_sequence<T, N / 2>::type>::type;
};

template <typename T>
struct _make_integer_sequence<T, 0> {
	using type = integer_sequence<T>;
};
template <typename T>
struct _make_integer_sequence<T, 1> {
	using type = integer_sequence<T, 0>;
};

} // namespace internal

template <typename T, T N>
using make_integer_sequence =
		typename internal::_make_integer_sequence<T, N>::type;

#endif

template <usize N>
using make_index_sequence = make_integer_sequence<usize, N>;

template <typename... Ts>
struct type_sequence;

} // namespace meta_
} // namespace internal
namespace meta {

template <typename T, T Value>
struct constant {
	static constexpr T value = Value;
};
template <typename T, T Value>
constexpr T constant<T, Value>::value;

template <bool B>
using bool_constant = constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template <typename T, T N>
using make_integer_sequence = internal::meta_::make_integer_sequence<T, N>*;
template <usize N>
using make_index_sequence = internal::meta_::make_integer_sequence<usize, N>*;

template <typename T, T... Nums>
using integer_sequence = internal::meta_::integer_sequence<T, Nums...>*;
template <usize... Nums>
using index_sequence = integer_sequence<usize, Nums...>;
template <typename... Ts>
using type_sequence = internal::meta_::type_sequence<Ts...>*;

template <typename Seq, typename... Bs>
struct and_test : false_type {};
template <typename Seq, typename... Bs>
struct or_test : true_type {};

template <usize Is, typename T>
using indexed = T;

template <typename... Ts>
struct pack_size {
	static constexpr usize value = sizeof...(Ts);
};
template <usize... Is>
struct and_test<index_sequence<Is...>, indexed<Is, true_type>...> : true_type {
};
template <usize... Is>
struct or_test<index_sequence<Is...>, indexed<Is, false_type>...> : false_type {
};

} // namespace meta
namespace internal {
namespace meta_ {
using namespace meta;

template <typename ISeq, typename... Ts>
struct all_same_impl : false_type {};
template <usize... Is, typename T>
struct all_same_impl<
		meta::index_sequence<Is...>,
		discard_1st<decltype(Is), T>...> : true_type {};

template <>
struct all_same_impl<meta::index_sequence<>> : true_type {};
} // namespace meta_
} // namespace internal

namespace concepts {
VEG_DEF_CONCEPT(
		typename... Ts,
		all_same,
		internal::meta_::
				all_same_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>::value);
} // namespace concepts

namespace internal {
namespace meta_ {
template <template <typename...> class F, typename Seq>
struct apply_type_seq;
template <template <typename...> class F, typename... Ts>
struct apply_type_seq<F, meta::type_sequence<Ts...>> {
	using type = F<Ts...>;
};

template <typename Valid, template <typename...> class F, typename... Seqs>
struct concat_type_seq;

template <typename Valid, template <typename...> class F, typename... Seqs>
struct zip_type_seq;
template <template <typename...> class F, typename... Seqs>
struct zip_type_seq2;

template <template <typename...> class F>
struct zip_type_seq<meta::true_type, F> {
	using type = F<>;
};

template <template <typename...> class F, typename... Ts>
struct zip_type_seq<meta::true_type, F, F<Ts...>> {
	using type = F<F<Ts>...>;
};

template <template <typename...> class F, typename... Ts, typename... Zipped>
struct zip_type_seq2<F, F<Ts...>, F<Zipped...>> {
	using type =
			F<typename concat_type_seq<true_type, F, F<Ts>, Zipped>::type...>;
};

template <template <typename...> class F, typename T>
struct specializes : meta::false_type {};
template <template <typename...> class F, typename... Ts>
struct specializes<F, F<Ts...>> : meta::true_type {};

template <template <typename...> class F, typename T>
struct specialize_len : meta::constant<usize, 0> {};
template <template <typename...> class F, typename... Ts>
struct specialize_len<F, F<Ts...>> : meta::constant<usize, sizeof...(Ts)> {};

template <
		template <typename...>
		class F,
		typename... Ts,
		typename Seq,
		typename... Seqs>
struct zip_type_seq<meta::true_type, F, F<Ts...>, Seq, Seqs...> {
	using type = typename zip_type_seq2<
			F,
			F<Ts...>,
			typename zip_type_seq<meta::true_type, F, Seq, Seqs...>::type>::type;
};

template <template <typename...> class F>
struct concat_type_seq<true_type, F> {
	using type = F<>;
};

template <template <typename...> class F, typename... Ts>
struct concat_type_seq<true_type, F, F<Ts...>> {
	using type = F<Ts...>;
};

template <template <typename...> class F, typename... Ts, typename... Us>
struct concat_type_seq<true_type, F, F<Ts...>, F<Us...>> {
	using type = F<Ts..., Us...>;
};

template <
		template <typename...>
		class F,
		typename... Ts,
		typename... Us,
		typename... Vs,
		typename... Seqs>
struct concat_type_seq<true_type, F, F<Ts...>, F<Us...>, F<Vs...>, Seqs...> {
	using type = typename concat_type_seq<
			true_type,
			F,
			F<Ts..., Us..., Vs...>,
			typename concat_type_seq<true_type, F, Seqs...>::type>::type;
};
} // namespace meta_
} // namespace internal
namespace meta {
template <template <typename... F> class F, typename... Seqs>
using type_sequence_cat = typename internal::meta_::concat_type_seq<
		bool_constant<VEG_ALL_OF(internal::meta_::specializes<F, Seqs>::value)>,
		F,
		Seqs...>::type;

template <template <typename...> class F, typename... Seqs>
using type_sequence_zip = typename internal::meta_::zip_type_seq<
		meta::bool_constant<
				VEG_ALL_OF(internal::meta_::specializes<F, Seqs>::value) &&
				VEG_CONCEPT(all_same<constant<
												usize,
												internal::meta_::specialize_len<F, Seqs>::value>...>)>,
		F,
		Seqs...>::type;

template <template <typename...> class F, typename Seq>
using type_sequence_apply =
		typename internal::meta_::apply_type_seq<F, Seq>::type;
} // namespace meta

namespace internal {

template <usize I, typename T>
struct HollowLeaf {};

template <typename ISeq, typename... Ts>
struct HollowIndexedTuple;
template <usize... Is, typename... Ts>
struct HollowIndexedTuple<meta::index_sequence<Is...>, Ts...>
		: HollowLeaf<Is, Ts>... {};

template <usize I, typename T>
auto get_type(HollowLeaf<I, T> const*) VEG_NOEXCEPT -> T;

template <usize I>
struct pack_ith_elem {
	template <typename... Ts>
	using Type = decltype(internal::get_type<I>(
			static_cast<
					HollowIndexedTuple<meta::make_index_sequence<sizeof...(Ts)>, Ts...>*>(
					nullptr)));
};

} // namespace internal

template <usize I, typename... Ts>
using ith = typename internal::pack_ith_elem<I>::template Type<Ts...>;
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_INTEGER_SEQ_HPP_JBT0EKAQS */

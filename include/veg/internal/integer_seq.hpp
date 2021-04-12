#ifndef VEG_INTEGER_SEQ_HPP_JBT0EKAQS
#define VEG_INTEGER_SEQ_HPP_JBT0EKAQS

#include "veg/internal/typedefs.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
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

} // namespace meta
namespace internal {
namespace meta_ {
template <template <typename...> class F, typename Seq>
struct apply_type_seq;
template <template <typename...> class F, typename... Ts>
struct apply_type_seq<F, meta::type_sequence<Ts...>> {
	using type = F<Ts...>;
};

template <typename... Seqs>
struct concat_type_seq;

template <>
struct concat_type_seq<> {
	using type = meta::type_sequence<>;
};

template <typename... Ts>
struct concat_type_seq<meta::type_sequence<Ts...>> {
	using type = meta::type_sequence<Ts...>;
};

template <typename... Ts, typename... Us>
struct concat_type_seq<meta::type_sequence<Ts...>, meta::type_sequence<Us...>> {
	using type = meta::type_sequence<Ts..., Us...>;
};

template <typename... Ts, typename... Us, typename... Seqs>
struct concat_type_seq<
		meta::type_sequence<Ts...>,
		meta::type_sequence<Us...>,
		Seqs...> {
	using type = typename concat_type_seq<
			meta::type_sequence<Ts..., Us...>,
			typename concat_type_seq<Seqs...>::type>::type;
};
} // namespace meta_
} // namespace internal
namespace meta {
template <typename... Seqs>
using type_sequence_cat =
		typename internal::meta_::concat_type_seq<Seqs...>::type;

template <template <typename...> class F, typename Seq>
using type_sequence_apply =
		typename internal::meta_::apply_type_seq<F, Seq>::type;
} // namespace meta
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_INTEGER_SEQ_HPP_JBT0EKAQS */

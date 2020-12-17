#ifndef VEG_INTEGER_SEQ_HPP_JBT0EKAQS
#define VEG_INTEGER_SEQ_HPP_JBT0EKAQS

#include "veg/internal/type_traits.hpp"

namespace veg {
namespace meta {
using size_t = decltype(sizeof(int));

template <typename T, T... Nums>
struct integer_sequence {
  using value_type = T;
  static constexpr auto size() noexcept -> size_t { return sizeof...(Nums); }
};
template <size_t... Nums>
using index_sequence = integer_sequence<size_t, Nums...>;

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

template <typename T, size_t N, bool Even = (N % 2) == 0>
struct _make_integer_sequence {
  using type = typename _merge<
      typename _make_integer_sequence<T, N / 2>::type,
      typename _make_integer_sequence<T, N / 2>::type>::type;
};

template <typename T, size_t N>
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

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

} // namespace meta
} // namespace veg

#endif /* end of include guard VEG_INTEGER_SEQ_HPP_JBT0EKAQS */

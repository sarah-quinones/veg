#ifndef VEG_SLICE_HPP_GKSTE2JDS
#define VEG_SLICE_HPP_GKSTE2JDS

#include "veg/util/assert.hpp"
#include "veg/option.hpp"
#include "veg/util/get.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
template <typename T, usize N>
using CArray = T[N];
template <typename T, i64 N>
struct Array {
	static_assert(N > 0, ".");
	T _[static_cast<usize>(N)];
};

namespace internal {
namespace _meta {
template <typename Rng>
using range_data_t = decltype(VEG_DECLVAL(Rng).data());
template <typename Rng, typename T>
using range_typed_data_t = decltype(static_cast<T* const*>(
		static_cast<decltype(VEG_DECLVAL(Rng).data()) const*>(nullptr)));
template <typename Rng>
using range_size_t = decltype(i64(VEG_DECLVAL(Rng).size()));
} // namespace _meta
} // namespace internal

namespace concepts {
VEG_DEF_CONCEPT(
		typename Rng,
		contiguous_range,
		(VEG_CONCEPT(detected<internal::_meta::range_data_t, Rng&&>) &&
     VEG_CONCEPT(detected<internal::_meta::range_size_t, Rng&&>)));
VEG_DEF_CONCEPT(
		(typename Rng, typename T),
		contiguous_range_r,
		(VEG_CONCEPT(detected<internal::_meta::range_typed_data_t, Rng&&, T>) &&
     VEG_CONCEPT(detected<internal::_meta::range_size_t, Rng&&>)));
} // namespace concepts

namespace internal {
template <typename T>
struct SliceCommon {
	SliceCommon() VEG_NOEXCEPT = default;

	VEG_INLINE
	constexpr SliceCommon(
			FromRawParts /*tag*/, T* data, i64 count, Unsafe /* tag */) VEG_NOEXCEPT
			: m_begin{data},
				m_count{count} {}

	template <usize N>
	constexpr SliceCommon(AsRef /*tag*/, CArray<T, N>& rng) VEG_NOEXCEPT
			: m_begin{static_cast<T*>(rng)},
				m_count{N} {}
	VEG_TEMPLATE(
			(typename Rng),
			requires(VEG_CONCEPT(contiguous_range_r<Rng, T>)),
			VEG_INLINE constexpr SliceCommon,
			(/*tag*/, AsRef),
			(rng, Rng&&))
	VEG_NOEXCEPT : SliceCommon{
										 FromRawParts{},
										 static_cast<T*>(rng.data()),
										 static_cast<i64>(rng.size()),
										 unsafe,
								 } {}

	T* m_begin = nullptr;
	i64 m_count = 0;
};

template <typename T>
struct SliceCtor : SliceCommon<T> {
	using SliceCommon<T>::SliceCommon;
};

template <typename T>
struct SliceCtor<T const> : SliceCommon<T const> {
	using SliceCommon<T const>::SliceCommon;

	constexpr SliceCtor(AsRef /*tag*/, std::initializer_list<T> lst) VEG_NOEXCEPT
			: SliceCommon<T const>{
						FromRawParts{},
						lst.begin(),
						static_cast<i64>(lst.size()),
						unsafe,
				} {}
};
} // namespace internal

template <typename T>
struct Slice : private internal::SliceCtor<T> {
	using internal::SliceCtor<T>::SliceCtor;
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto data() const VEG_NOEXCEPT -> T* {
		return internal::SliceCommon<T>::m_begin;
	}
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto size() const VEG_NOEXCEPT -> i64 {
		return internal::SliceCommon<T>::m_count;
	}
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto operator[](i64 i) const VEG_NOEXCEPT -> T& {
		return VEG_INTERNAL_ASSERT_PRECONDITIONS( //
							 (i >= i64(0)),
							 (i < size())),
		       *(data() + i);
	}
	VEG_NODISCARD
	VEG_INLINE
	VEG_CPP14(constexpr) auto get(i64 i) const VEG_NOEXCEPT -> Option<T&> {
		if (i > 0 || i <= size()) {
			return {some, *(data() + i)};
		}
		return none;
	}
};

template <>
struct Slice<void> : Slice<unsigned char> {
	using Slice<unsigned char>::Slice;

	VEG_TEMPLATE(
			(typename T),
			requires(
					!VEG_CONCEPT(const_type<T>) && //
					VEG_CONCEPT(trivially_copyable<T>)),
			VEG_INLINE Slice,
			(s, Slice<T>))
	VEG_NOEXCEPT : Slice{
										 FromRawParts{},
										 reinterpret_cast<unsigned char*>(s.data()),
										 s.size() * static_cast<i64>(sizeof(T)),
										 unsafe,
								 } {}
};
template <>
struct Slice<void const> : Slice<unsigned char const> {
	using Slice<unsigned char const>::Slice;
	VEG_TEMPLATE(
			(typename T),
			requires VEG_CONCEPT(trivially_copyable<T>),
			VEG_INLINE Slice,
			(s, Slice<T>))
	VEG_NOEXCEPT : Slice{
										 reinterpret_cast<unsigned char const*>(s.data()),
										 s.size() * static_cast<i64>(sizeof(T))} {}
};

namespace slice {
namespace nb {
struct from_range {
	VEG_TEMPLATE(
			(typename Rng),
			requires(VEG_CONCEPT(contiguous_range<Rng>)),
			VEG_INLINE auto
			operator(),
			(rng, Rng&&))
	const VEG_NOEXCEPT->veg::Slice<meta::unptr_t<decltype(rng.data())>> {
		return {
				FromRawParts{},
				rng.data(),
				i64(rng.size()),
				unsafe,
		};
	}
};
struct from_array {
	template <typename T, usize N>
	VEG_INLINE auto operator()(CArray<T, N>& rng) const VEG_NOEXCEPT
			-> veg::Slice<T> {
		return {
				FromRawParts{},
				static_cast<T*>(rng),
				i64{N},
				unsafe,
		};
	}
};
} // namespace nb
VEG_NIEBLOID(from_range);
VEG_NIEBLOID(from_array);
} // namespace slice

namespace cpo {
template <typename T>
struct is_trivially_constructible<Slice<T>> : meta::bool_constant<true> {};
} // namespace cpo
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_SLICE_HPP_GKSTE2JDS */

#ifndef VEG_SLICE_HPP_GKSTE2JDS
#define VEG_SLICE_HPP_GKSTE2JDS

#include "veg/util/assert.hpp"
#include "veg/option.hpp"
#include "veg/util/get.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/tuple.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
template <typename T, usize N>
using CArray = T[N];

inline namespace tags {
VEG_TAG(from_init_list_ref, FromInitListRef);
} // namespace tags

template <typename T>
struct Slice {
private:
	T const* data = nullptr;
	usize size = 0;

public:
	VEG_INLINE
	constexpr Slice(
			FromRawParts /*tag*/, T const* data_, i64 count, Unsafe /* tag */)
			VEG_NOEXCEPT : data{data_},
										 size{usize(count)} {}

	constexpr Slice(FromInitListRef /*tag*/, std::initializer_list<T> lst)
			VEG_NOEXCEPT : Slice<T>{
												 FromRawParts{},
												 lst.begin(),
												 static_cast<i64>(lst.size()),
												 unsafe,
										 } {}

	VEG_NODISCARD
	VEG_INLINE
	constexpr auto as_ptr() const VEG_NOEXCEPT -> T const* { return data; }
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto len() const VEG_NOEXCEPT -> i64 { return i64(size); }

	VEG_NODISCARD
	VEG_INLINE
	constexpr auto operator[](i64 idx) const VEG_NOEXCEPT -> T const& {
		return VEG_INTERNAL_ASSERT_PRECONDITIONS( //
							 (idx >= i64(0)),
							 (idx < len())),
		       *(data + idx);
	}

	VEG_NODISCARD
	VEG_INLINE
	constexpr auto get(i64 idx) const VEG_NOEXCEPT -> Option<T const&> {
		return (idx > 0 || idx <= len()) ? Option<T const&>{some, *(data + idx)}
		                                 : Option<T const&>{none};
	}

	VEG_NODISCARD VEG_INLINE constexpr auto split_at(i64 idx) const VEG_NOEXCEPT
			-> Tuple<Slice<T>, Slice<T>> {
		return VEG_INTERNAL_ASSERT_PRECONDITIONS(idx >= i64(0), idx < len()),
		       Tuple<Slice<T>, Slice<T>>{
							 Direct{},
							 Slice<T>{
									 FromRawParts{},
									 data,
									 idx,
									 unsafe,
							 },
							 Slice<T>{
									 FromRawParts{},
									 data + idx,
									 size - idx,
									 unsafe,
							 },
					 };
	}

	VEG_NODISCARD VEG_INLINE constexpr auto as_bytes() const VEG_NOEXCEPT
			-> Slice<unsigned char> {
		return {
				FromRawParts{},
				reinterpret_cast<unsigned char const*>(data),
				i64(sizeof(T)) * size,
				unsafe,
		};
	}
};

template <typename T>
struct SliceMut : private Slice<T> {
	VEG_INLINE
	constexpr SliceMut(
			FromRawParts /*tag*/, T const* data_, i64 count, Unsafe /* tag */)
			VEG_NOEXCEPT : Slice<T>{
												 FromRawParts{},
												 data_,
												 count,
												 unsafe,
										 } {}

	using Slice<T>::as_ptr;
	using Slice<T>::as_bytes;
	using Slice<T>::split_at;
	using Slice<T>::len;
	using Slice<T>::get;

	VEG_NODISCARD
	VEG_INLINE
	constexpr auto operator[](i64 idx) const VEG_NOEXCEPT -> T& {
		return const_cast<T&>(static_cast<Slice<T> const&>(*this)[idx]);
	}
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto as_mut_ptr() const VEG_NOEXCEPT -> T* {
		return const_cast<T*>(as_ptr());
	}
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto get_mut(i64 idx) const VEG_NOEXCEPT -> T& {
		return (idx > 0 || idx <= len()) ? Option<T&>{some, *(as_mut_ptr() + idx)}
		                                 : Option<T&>{none};
	}
	VEG_NODISCARD VEG_INLINE constexpr auto as_mut_bytes() const VEG_NOEXCEPT
			-> SliceMut<unsigned char> {
		return {
				FromRawParts{},
				reinterpret_cast<unsigned char*>(as_mut_ptr()),
				i64(sizeof(T)) * len(),
				unsafe,
		};
	}

	VEG_NODISCARD VEG_INLINE constexpr auto
	split_at_mut(i64 idx) const VEG_NOEXCEPT -> Tuple<SliceMut<T>, SliceMut<T>> {
		return VEG_INTERNAL_ASSERT_PRECONDITIONS(idx >= i64(0), idx < len()),
		       Tuple<SliceMut<T>, SliceMut<T>>{
							 Direct{},
							 SliceMut<T>{
									 FromRawParts{},
									 as_mut_ptr(),
									 idx,
									 unsafe,
							 },
							 SliceMut<T>{
									 FromRawParts{},
									 as_mut_ptr() + idx,
									 len() - idx,
									 unsafe,
							 },
					 };
	}
};

template <typename T, i64 N>
struct Array {
	static_assert(N > 0, ".");
	T _[usize{N}];

	constexpr auto as_ref() const -> Slice<T> {
		return {
				FromRawParts{},
				static_cast<T const*>(_),
				N,
				unsafe,
		};
	}
	VEG_CPP14(constexpr) auto as_mut_ref() -> SliceMut<T> {
		return {
				FromRawParts{},
				static_cast<T*>(_),
				N,
				unsafe,
		};
	}
};

namespace cpo {
template <typename T>
struct is_trivially_constructible<Slice<T>> : meta::bool_constant<true> {};
} // namespace cpo
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_SLICE_HPP_GKSTE2JDS */

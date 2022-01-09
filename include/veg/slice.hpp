#ifndef VEG_SLICE_HPP_GKSTE2JDS
#define VEG_SLICE_HPP_GKSTE2JDS

#include "veg/util/assert.hpp"
#include "veg/option.hpp"
#include "veg/util/get.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/tuple.hpp"
#include "veg/util/compare.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
template <typename T, usize N>
using CArray = T[N];

template <typename T>
struct InitList {
	T const* data;
	isize len;
};

namespace nb {
struct init_list {
	template <typename T>
	VEG_NODISCARD VEG_INLINE constexpr auto
	operator()(std::initializer_list<T> l) const noexcept -> InitList<T> {
		return {l.begin(), l.size()};
	}
};
} // namespace nb
VEG_NIEBLOID(init_list);

namespace internal {
namespace _slice {
namespace adl {
struct AdlBase {};
} // namespace adl
} // namespace _slice
} // namespace internal

template <typename T>
struct Slice {
private:
	T const* data = nullptr;
	isize size = 0;

public:
	VEG_INLINE
	constexpr Slice() = default;

	VEG_INLINE
	constexpr Slice(
			FromRawParts /*tag*/, Unsafe /*tag*/, T const* data_, isize count)
			VEG_NOEXCEPT : data{data_},
										 size{count} {}

	constexpr Slice(InitList<T> lst) VEG_NOEXCEPT
			: Slice<T>{unsafe, from_raw_parts, lst.data, lst.len} {}

	VEG_NODISCARD
	VEG_INLINE
	constexpr auto ptr() const VEG_NOEXCEPT -> T const* { return data; }
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto len() const VEG_NOEXCEPT -> isize { return size; }

	VEG_NODISCARD
	VEG_INLINE
	constexpr auto operator[](isize idx) const VEG_NOEXCEPT -> T const& {
		return VEG_INTERNAL_ASSERT_PRECONDITION(usize(idx) < usize(len())),
		       *(data + idx);
	}
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto get_unchecked(Unsafe /*tag*/, isize idx) const VEG_NOEXCEPT
			-> Ref<T> {
		return ref(*(data + idx));
	}

	VEG_NODISCARD VEG_INLINE constexpr auto split_at(isize idx) const VEG_NOEXCEPT
			-> Tuple<Slice<T>, Slice<T>> {
		return VEG_INTERNAL_ASSERT_PRECONDITION(usize(idx) < usize(len())),
		       Tuple<Slice<T>, Slice<T>>{
							 tuplify,
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
				from_raw_parts,
				unsafe,
				reinterpret_cast<unsigned char const*>(data),
				isize(sizeof(T)) * size,
		};
	}
};

template <typename T>
struct SliceMut : private Slice<T> {
	VEG_INLINE
	constexpr SliceMut() = default;

	VEG_INLINE
	constexpr SliceMut(
			FromRawParts /*tag*/, Unsafe /*tag*/, T const* data_, isize count)
			VEG_NOEXCEPT : Slice<T>{
												 FromRawParts{},
												 unsafe,
												 data_,
												 count,
										 } {}

	using Slice<T>::ptr;
	using Slice<T>::as_bytes;
	using Slice<T>::split_at;
	using Slice<T>::len;
	using Slice<T>::get_unchecked;

	VEG_NODISCARD
	VEG_INLINE
	constexpr auto operator[](isize idx) const VEG_NOEXCEPT -> T& {
		return const_cast<T&>(static_cast<Slice<T> const&>(*this)[idx]);
	}
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto mut_ptr() const VEG_NOEXCEPT -> T* {
		return const_cast<T*>(ptr());
	}
	VEG_NODISCARD
	VEG_INLINE
	constexpr auto get_mut_unchecked(Unsafe /*tag*/, isize idx) const VEG_NOEXCEPT
			-> RefMut<T> {
		return mut(const_cast<T&>(*(this->data + idx)));
	}
	VEG_NODISCARD VEG_INLINE constexpr auto as_mut_bytes() const VEG_NOEXCEPT
			-> SliceMut<unsigned char> {
		return {
				from_raw_parts,
				unsafe,
				reinterpret_cast<unsigned char*>(mut_ptr()),
				isize(sizeof(T)) * len(),
		};
	}

	VEG_NODISCARD VEG_INLINE constexpr auto
	split_at_mut(isize idx) const VEG_NOEXCEPT
			-> Tuple<SliceMut<T>, SliceMut<T>> {
		return VEG_INTERNAL_ASSERT_PRECONDITION(usize(idx) < usize(len())),
		       Tuple<SliceMut<T>, SliceMut<T>>{
							 tuplify,
							 SliceMut<T>{
									 FromRawParts{},
									 unsafe,
									 mut_ptr(),
									 idx,
							 },
							 SliceMut<T>{
									 FromRawParts{},
									 unsafe,
									 mut_ptr() + idx,
									 len() - idx,
							 },
					 };
	}
};

namespace array {
template <typename T, isize N>
struct Array {
	static_assert(N > 0, ".");
	T _[usize{N}];

	constexpr auto as_ref() const -> Slice<T> {
		return {
				from_raw_parts,
				unsafe,
				static_cast<T const*>(_),
				N,
		};
	}
	VEG_CPP14(constexpr) auto as_mut() -> SliceMut<T> {
		return {
				from_raw_parts,
				unsafe,
				static_cast<T*>(_),
				N,
		};
	}
};
} // namespace array
using array::Array;

namespace internal {
namespace _slice {
namespace adl {
VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(eq<T, U>)),
		VEG_NODISCARD static VEG_CPP14(constexpr) auto
		operator==,
		(lhs, Slice<T>),
		(rhs, Slice<U>))
VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<T, U>))->bool {
	if (lhs.len() != rhs.len()) {
		return false;
	}
	for (isize i = 0; i < lhs.len(); ++i) {
		if (!(lhs.get_unchecked(unsafe, i).get() ==
		      rhs.get_unchecked(unsafe, i).get())) {
			return false;
		}
	}
	return true;
};
VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(eq<T, U>)),
		VEG_NODISCARD static VEG_CPP14(constexpr) auto
		operator==,
		(lhs, SliceMut<T>),
		(rhs, SliceMut<U>))
VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<T, U>))->bool {
	return adl::operator==(Slice<T>(lhs), Slice<U>(rhs));
};
} // namespace adl

struct DbgSliceBase {
	template <typename T>
	static void to_string(fmt::BufferMut out, Ref<Slice<T>> arg) {
		T const* ptr = arg.get().ptr();
		isize len = arg.get().len();

		internal::_fmt::DbgStructScope _{out};
		for (isize i = 0; i < len; ++i) {
			out.append_ln();
			fmt::Debug<T>::to_string(out, ref(ptr[i]));
			out.append_literal(",");
		}
	}
};
struct DbgSliceMutBase {
	template <typename T>
	static void to_string(fmt::BufferMut out, Ref<SliceMut<T>> arg) {
		DbgSliceBase::to_string(VEG_FWD(out), ref(Slice<T>(arg.get())));
	}
};
struct DbgArrayBase {
	template <typename T, isize N>
	static void to_string(fmt::BufferMut out, Ref<Array<T, N>> arg) {
		DbgSliceBase::to_string(VEG_FWD(out), ref(arg.get().as_ref()));
	}
};

struct OrdSliceBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_NODISCARD static VEG_CPP14(constexpr) auto cmp,
			(lhs, Ref<Slice<T>>),
			(rhs, Ref<Slice<U>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->cmp::Ordering {
		Slice<T> lhs_ = lhs.get();
		Slice<U> rhs_ = rhs.get();

		isize common_len = lhs_.len() < rhs_.len() ? lhs_.len() : rhs_.len();
		for (isize i = 0; i < common_len; ++i) {
			auto const val = static_cast<cmp::Ordering>(cmp::Ord<T, U>::cmp( //
					lhs_.get_unchecked(unsafe, i),
					rhs_.get_unchecked(unsafe, i)));
			if (val != cmp::Ordering::equal) {
				return val;
			}
		}
		return cmp::Ord<isize, isize>::cmp(ref(lhs_.len()), ref(rhs_.len()));
	}
};
struct OrdSliceMutBase {
	VEG_TEMPLATE(
			(typename T, typename U),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_NODISCARD static VEG_CPP14(constexpr) auto cmp,
			(lhs, Ref<SliceMut<T>>),
			(rhs, Ref<SliceMut<U>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->cmp::Ordering {
		return OrdSliceBase::cmp( //
				ref(Slice<T>(lhs.get())),
				ref(Slice<U>(rhs.get())));
	}
};
struct OrdArrayBase {
	VEG_TEMPLATE(
			(typename T, isize N, typename U, isize M),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_NODISCARD static VEG_CPP14(constexpr) auto cmp,
			(lhs, Ref<Array<T, N>>),
			(rhs, Ref<Array<U, M>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->cmp::Ordering {
		return OrdSliceBase::cmp( //
				ref(lhs.get().as_ref()),
				ref(rhs.get().as_ref()));
	}
};
} // namespace _slice
} // namespace internal
namespace array {
VEG_TEMPLATE(
		(typename T, isize N, typename U, isize M),
		requires(VEG_CONCEPT(eq<T, U>)),
		VEG_NODISCARD static VEG_CPP14(constexpr) auto
		operator==,
		(lhs, Array<T, N> const&),
		(rhs, Array<U, M> const&))
VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<T, U>))->bool {
	return (N == M) &&
	       internal::_slice::adl::operator==(lhs.as_ref(), rhs.as_ref());
}
} // namespace array

template <typename T>
struct cpo::is_trivially_constructible<Slice<T>> : meta::bool_constant<true> {};

template <typename T, typename U>
struct cmp::Ord<Slice<T>, Slice<U>> : internal::_slice::OrdSliceBase {};
template <typename T, typename U>
struct cmp::Ord<SliceMut<T>, SliceMut<U>> : internal::_slice::OrdSliceMutBase {
};
template <typename T, isize N, typename U, isize M>
struct cmp::Ord<Array<T, N>, Array<U, M>> : internal::_slice::OrdArrayBase {};

template <typename T>
struct fmt::Debug<Slice<T>> : internal::_slice::DbgSliceBase {};
template <typename T>
struct fmt::Debug<SliceMut<T>> : internal::_slice::DbgSliceBase {};
template <typename T, isize N>
struct fmt::Debug<Array<T, N>> : internal::_slice::DbgArrayBase {};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_SLICE_HPP_GKSTE2JDS */

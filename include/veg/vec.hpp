#ifndef VEG_VECTOR_HPP_QWFSH3ROS
#define VEG_VECTOR_HPP_QWFSH3ROS

#include "veg/internal/delete_special_members.hpp"
#include "veg/memory/alloc.hpp"
#include "veg/internal/collection_algo.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/slice.hpp"
#include "veg/util/compare.hpp"
#include "veg/util/unreachable.hpp"

#include "veg/internal/prologue.hpp"

namespace veg {
namespace _detail {
namespace _vector {
namespace adl {
struct AdlBase {};
} // namespace adl
} // namespace _vector
} // namespace _detail

namespace _detail {
template <typename T>
VEG_INLINE constexpr auto min2(T a, T b) noexcept -> T {
	return (static_cast<T const&>(a) < static_cast<T const&>(b)) ? VEG_FWD(a)
	                                                             : VEG_FWD(b);
}
template <typename T>
VEG_INLINE constexpr auto max2(T a, T b) noexcept -> T {
	return (static_cast<T const&>(a) < static_cast<T const&>(b)) ? VEG_FWD(b)
	                                                             : VEG_FWD(a);
}

namespace _collections {
VEG_INLINE constexpr auto vector_grow_compute(usize current_cap) noexcept
		-> usize {
	return current_cap + current_cap;
}
// new_cap must be larger than current_cap
VEG_INLINE constexpr auto
vector_grow_choose(usize current_cap, usize new_cap) noexcept -> usize {
	return _detail::max2(_collections::vector_grow_compute(current_cap), new_cap);
}

template <typename T>
auto relocate(void* out, void const* in, usize nbytes) noexcept -> void*;

struct relocate_pointer_trivial {
	static constexpr void* (*value)(void*, void const*, usize) = &mem::memmove;
};
template <typename T, bool = cpo::is_trivially_relocatable<T>::value>
struct relocate_pointer : relocate_pointer_trivial {};

template <typename T>
struct relocate_pointer<T, false> {
	static constexpr void* (*value)(void*, void const*, usize) =
			_collections::relocate<T>;
};
} // namespace _collections
} // namespace _detail

namespace collections {
template <typename T>
struct relocate_pointer : _detail::_collections::relocate_pointer<T> {};
} // namespace collections

namespace vector {
template <typename T>
struct RawVector {
	T* data;
	T* end;
	T* end_alloc;

	VEG_INLINE constexpr auto len() const noexcept -> usize { return end - data; }
};
} // namespace vector

namespace _detail {
namespace _collections {
template <bool IsNoExcept>
struct CloneImpl;

template <bool NoThrow, typename T, typename A, typename C>
struct CloneFn {
	RefMut<A> alloc;
	RefMut<C> cloner;
	T const* in;
	VEG_CPP14(constexpr)
	VEG_INLINE auto operator()() VEG_NOEXCEPT_IF(NoThrow) -> T {
		return mem::Cloner<C>::clone(VEG_FWD(cloner), ref(*in), VEG_FWD(alloc));
	}
};

template <>
struct CloneImpl<true> {
	template <typename T, typename A, typename C>
	static VEG_CPP14(constexpr) void fn( //
			RefMut<A> alloc,
			RefMut<C> cloner,
			T* out,
			T* out_end,
			T const* in) VEG_NOEXCEPT {
		for (; out < out_end; ++out, ++in) {
			mem::construct_with(
					out,
					CloneFn<true, T, A, C>{//
			                           VEG_FWD(alloc),
			                           VEG_FWD(cloner),
			                           in});
		}
	}
};

template <>
struct CloneImpl<false> {
	template <typename T, typename A, typename C>
	static void fn( //
			RefMut<A> alloc,
			RefMut<C> cloner,
			T* out,
			T* out_end,
			T const* in) VEG_NOEXCEPT_IF(false) {

		Defer<Cleanup<T, A, C>> _{{VEG_FWD(alloc), VEG_FWD(cloner), out, out_end}};
		for (; _.fn.ptr < _.fn.ptr_end; ++_.fn.ptr, ++in) {
			mem::construct_with(
					_.fn.ptr,
					CloneFn<false, T, A, C>{
							VEG_FWD(_.fn.alloc),
							VEG_FWD(_.fn.cloner),
							in,
					});
		}
	}
};

template <typename T, typename A, typename C>
VEG_CPP14(constexpr)
void slice_clone(
		RefMut<A> alloc, RefMut<C> cloner, T* out, T* out_end, T const* in)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(alloc::nothrow_clone<C, T, A>)) {
	CloneImpl<VEG_CONCEPT(alloc::nothrow_clone<C, T, A>)>::fn(
			VEG_FWD(alloc), VEG_FWD(cloner), out, out_end, in);
}

template <typename T, typename A, typename C>
VEG_CPP14(constexpr)
void slice_clone_from(
		RefMut<A> alloc, RefMut<C> cloner, T* out, T* out_end, T const* in)
		VEG_NOEXCEPT_IF(true) {
	while (true) {
		if (out == out_end) {
			break;
		}
		mem::Cloner<C>::clone_from( //
				VEG_FWD2(cloner),
				mut(*out),
				ref(*in),
				VEG_FWD2(alloc));
		++out;
		++in;
	}
}

template <typename T>
auto relocate(void* out, void const* in, usize nbytes) noexcept -> void* {
	T* out_T = static_cast<T*>(out);
	T* in_T = const_cast<T*>(static_cast<T const*>(in));
	usize n = nbytes / sizeof(T);

	for (usize i = 0; i < n; ++i) {
		out_T[i] = static_cast<T&&>(in_T[i]);
		in_T[i].~T();
	}

	return out;
}

template <typename T>
auto relocate_backward(void* out, void const* in, usize nbytes) noexcept
		-> void* {
	T* out_T = static_cast<T*>(out);
	T* in_T = const_cast<T*>(static_cast<T const*>(in));
	usize n = nbytes / sizeof(T);

	for (usize i = 0; i < n; ++i) {
		out_T[n - i - 1] = static_cast<T&&>(in_T[n - i - 1]);
		in_T[n - i - 1].~T();
	}

	return out;
}

template <typename A>
struct AllocCleanup {
	RefMut<A> alloc;
	void* data;
	mem::Layout layout;

	VEG_INLINE VEG_CPP14(constexpr) void operator()()
			VEG_NOEXCEPT_IF(VEG_CONCEPT(alloc::nothrow_dealloc<A>)) {
		if (data != nullptr) {
			mem::Alloc<A>::dealloc(
					VEG_FWD(alloc), static_cast<void*>(data), mem::Layout(layout));
		}
	}
};

template <typename T, typename A, typename C>
auto alloc_and_copy(RefMut<A> alloc, RefMut<C> cloner, T const* data, usize len)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(alloc::nothrow_alloc<A>) &&
				VEG_CONCEPT(alloc::nothrow_clone<C, T, A>)) -> mem::AllocBlock {

	mem::AllocBlock block = mem::Alloc<A>::alloc(
			VEG_FWD(alloc), mem::Layout{sizeof(T) * usize(len), alignof(T)});

	// if copying fails, this takes care of deallocating
	Defer<AllocCleanup<A>> _{{
			VEG_FWD(alloc),
			block.data,
			mem::Layout{block.byte_cap, alignof(T)},
	}};

	// copy construct elements
	_collections::slice_clone(
			VEG_FWD(_.fn.alloc),
			VEG_FWD(cloner),
			static_cast<T*>(block.data),
			static_cast<T*>(block.data) + len,
			data);

	_.fn.data = nullptr;
	return block;
}

template <typename T, typename A, typename C>
auto realloc_and_append( //
		RefMut<A> alloc,
		RefMut<C> cloner,
		mem::AllocBlock out,
		usize out_len,
		T const* in,
		usize in_len)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(alloc::nothrow_grow<A>) &&
				VEG_CONCEPT(alloc::nothrow_clone<C, T, A>)) -> mem::AllocBlock {

	if (in_len == 0) {
		return out;
	}

	if (out.byte_cap >= (in_len + out_len) * sizeof(T)) {
		mem::AllocBlock block = mem::Alloc<A>::grow(
				VEG_FWD(alloc),
				VEG_FWD(out.data),
				mem::Layout{out.byte_cap, alignof(T)},
				out_len * sizeof(T),
				mem::RelocFn{collections::relocate_pointer<T>::value});

		// if copying fails, this takes care of deallocating
		Defer<AllocCleanup<A>> _{{
				VEG_FWD(alloc),
				block.data,
				mem::Layout{block.byte_cap, alignof(T)},
		}};
		// if copying fails, this takes care of destroying
		Defer<Cleanup<T, A, C>> destroy{{
				VEG_FWD(_.fn.alloc),
				VEG_FWD(cloner),
				static_cast<T*>(block.data),
				static_cast<T*>(block.data) + out_len,
		}};

		// copy construct elements
		_collections::slice_clone( //
				VEG_FWD(destroy.fn.alloc),
				VEG_FWD(destroy.fn.cloner),
				static_cast<T*>(block.data) + out_len,
				static_cast<T*>(block.data) + in_len,
				in);
		// disable destruction
		destroy.fn.ptr = nullptr;
		destroy.fn.ptr_end = nullptr;
		// disable deallocation
		_.fn.data = nullptr;
		out = block;
	} else {
		// copy construct elements
		_collections::slice_clone( //
				VEG_FWD(alloc),
				VEG_FWD(cloner),
				static_cast<T*>(out.data) + out_len,
				static_cast<T*>(out.data) + in_len,
				in);
	}
	return out;
}

template <bool TrivialAssign>
struct CloneFromImpl;

template <>
struct CloneFromImpl<false> {
	template <typename T, typename A, typename C>
	static void
	fn(RefMut<A> lhs_alloc,
	   RefMut<C> cloner,
	   vector::RawVector<T>& lhs_raw,
	   Ref<A> rhs_alloc,
	   vector::RawVector<T> const rhs_raw)
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_copy_assignable<A>) &&    //
					VEG_CONCEPT(alloc::nothrow_alloc<A>) &&       //
					VEG_CONCEPT(alloc::nothrow_clone<C, T, A>) && //
					VEG_CONCEPT(alloc::nothrow_clone_from<C, T, A>)) {

		vector::RawVector<T> lhs_copy = lhs_raw;
		usize rhs_len = (rhs_raw.end - rhs_raw.data);

		if (cmp::ne(lhs_alloc.as_const(), rhs_alloc)) {
			T* data = lhs_copy.data;
			T* data_end = lhs_copy.end;

			// clean up old alloc
			_collections::backward_destroy(
					VEG_FWD(lhs_alloc), VEG_FWD(cloner), data, data_end);

			// assign before deallocation in case it fails
			lhs_raw = {};
			lhs_copy = {};

			// don't need to deallocate on backward_destroy failure, since lhs can
			// still access and reuse the allocation
			mem::Alloc<A>::dealloc( //
					VEG_FWD(lhs_alloc),
					static_cast<void*>(data),
					mem::Layout{
							(lhs_copy.end_alloc - lhs_copy.data) * sizeof(T), alignof(T)});
		}

		lhs_alloc.get() = rhs_alloc.get();

		if (lhs_raw.data == nullptr) {
			usize len = rhs_raw.end - rhs_raw.data;

			mem::AllocBlock blk = _collections::alloc_and_copy(
					VEG_FWD(lhs_alloc), VEG_FWD(cloner), rhs_raw.data, len);
			T* data = static_cast<T*>(blk.data);
			lhs_raw = {
					data,
					data + len,
					data + blk.byte_cap / sizeof(T),
			};
			return;
		}

		usize assign_len = _detail::min2(lhs_copy.len(), rhs_raw.len());
		// copy assign until the shared len
		_collections::slice_clone_from( //
				VEG_FWD(lhs_alloc),
				VEG_FWD(cloner),
				lhs_copy.data,
				lhs_copy.data + assign_len,
				rhs_raw.data);

		// destroy from the shared len until end of lhs
		lhs_raw.end = lhs_raw.data + assign_len;
		_collections::backward_destroy( //
				VEG_FWD(lhs_alloc),
				VEG_FWD(cloner),
				lhs_copy.data + assign_len,
				lhs_copy.end);

		// pass allocation to realloc_and_append

		lhs_raw = {};
		// realloc and copy construct new elements until end of rhs
		mem::AllocBlock block = _collections::realloc_and_append(
				VEG_FWD(lhs_alloc),
				VEG_FWD(cloner),
				mem::AllocBlock{
						lhs_copy.data,
						(lhs_copy.end_alloc - lhs_copy.data) * sizeof(T),
				},                         // out
				assign_len,                // out_len
				rhs_raw.data + assign_len, // in
				rhs_len - assign_len);     // in_len

		lhs_raw = vector::RawVector<T>{
				static_cast<T*>(block.data),
				static_cast<T*>(block.data) + rhs_len,
				static_cast<T*>(block.data) + block.byte_cap / sizeof(T),
		};
	}
};
template <>
struct CloneFromImpl<true> {
	template <typename T, typename A, typename C>
	static void
	fn(RefMut<A> lhs_alloc,
	   vector::RawVector<T>& lhs_raw,
	   RefMut<C> cloner,
	   Ref<A> rhs_alloc,
	   vector::RawVector<T> const rhs_raw)
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_copy_assignable<A>) && //
					VEG_CONCEPT(alloc::nothrow_dealloc<A>) &&
					VEG_CONCEPT(alloc::nothrow_alloc<A>)) {

		vector::RawVector<T> lhs_copy = lhs_raw;

		bool need_to_realloc =
				(cmp::ne(lhs_alloc.as_const(), rhs_alloc) ||
		     (lhs_copy.cap < rhs_raw.len));
		if (need_to_realloc) {
			T* data = lhs_copy.data;
			T* data_end = lhs_copy.data + lhs_copy.len;
			usize cap = lhs_copy.cap;

			// assign before deallocation in case it fails
			lhs_raw = {};
			mem::Alloc<A>::dealloc( //
					VEG_FWD(lhs_alloc),
					static_cast<void*>(data),
					mem::Layout{cap * sizeof(T), alignof(T)});
			lhs_copy = {};
		}

		lhs_alloc.get() = rhs_alloc.get();

		// allocate and copy all elements
		if (need_to_realloc) {
			mem::AllocBlock block = _collections::alloc_and_copy( //
					VEG_FWD(lhs_alloc),
					VEG_FWD(cloner),
					rhs_raw.data,
					rhs_raw.len);
			lhs_raw.data = block.data;
			lhs_raw.cap = block.byte_cap / sizeof(T);
		} else {
			_collections::slice_clone( //
					VEG_FWD(lhs_alloc),
					VEG_FWD(cloner),
					lhs_copy.data,
					lhs_copy.data + rhs_raw.len,
					rhs_raw.data);
		}
		lhs_raw.len = rhs_raw.len;
	}
};

template <typename T, typename A, typename C>
VEG_INLINE void clone_from(
		RefMut<A> lhs_alloc,
		RefMut<C> cloner,
		vector::RawVector<T>& lhs_raw,
		Ref<A> rhs_alloc,
		vector::RawVector<T> const rhs_raw)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_copy_assignable<A>) &&    //
				VEG_CONCEPT(alloc::nothrow_alloc<A>) &&       //
				VEG_CONCEPT(alloc::nothrow_clone<C, T, A>) && //
				VEG_CONCEPT(alloc::nothrow_clone_from<C, T, A>)) {
	_collections::
			CloneFromImpl<mem::Cloner<C>::template trivial_clone<T>::value>::fn(
					VEG_FWD(lhs_alloc), VEG_FWD(cloner), lhs_raw, rhs_alloc, rhs_raw);
}
} // namespace _collections
} // namespace _detail

namespace _detail {
namespace _vector {

template <typename T>
struct RawVectorMoveRaii /* NOLINT */ {
	vector::RawVector<T> _ = {};

	RawVectorMoveRaii() = default;
	RawVectorMoveRaii(FromRawParts /*tag*/, vector::RawVector<T> inner)
			VEG_NOEXCEPT : _{inner} {};
	VEG_INLINE VEG_CPP14(constexpr)
			RawVectorMoveRaii(RawVectorMoveRaii&& rhs) VEG_NOEXCEPT : _{rhs._} {
		rhs._ = {};
	}
	VEG_INLINE VEG_CPP14(constexpr)
			RawVectorMoveRaii(RawVectorMoveRaii const& /*rhs*/) VEG_NOEXCEPT {}
};

template <typename T, typename A>
struct VecAlloc :
		// alloc manager needs to be constructed first
		Tuple<A, RawVectorMoveRaii<T>> {
	using Tuple<A, RawVectorMoveRaii<T>>::Tuple;

public:
	VecAlloc(VecAlloc const&) = default;
	VecAlloc(VecAlloc&&) = default;
	auto operator=(VecAlloc const&) -> VecAlloc& = default;
	auto operator=(VecAlloc&&) -> VecAlloc& = default;

	VEG_INLINE ~VecAlloc()
			VEG_NOEXCEPT_IF(VEG_CONCEPT(alloc::nothrow_dealloc<A>)) {
		vector::RawVector<T> raw = (*this)[1_c]._;
		if ((raw.data != nullptr) && (raw.end_alloc != 0)) {

			// FIXME: if asan is enabled, before sanitizing make sure that:
			//  - begin is 8 byte aligned
			//  - either:
			//    - end is 8 byte aligned
			//    - A is the SystemAlloc
#if VEG_HAS_ASAN
			_detail::__sanitizer_annotate_contiguous_container( //
					raw.data,
					raw.end_alloc,
					raw.data,
					raw.end_alloc);
#endif

			mem::Alloc<A>::dealloc(
					mut((*this)[0_c]),
					static_cast<void*>(raw.data),
					mem::Layout{usize(raw.end_alloc - raw.data) * sizeof(T), alignof(T)});
		}
	}
};
} // namespace _vector
} // namespace _detail

#if VEG_HAS_ASAN
#define __VEG_ASAN_ANNOTATE() /* NOLINT */                                     \
	if (ptr() != nullptr) {                                                      \
		_detail::__sanitizer_annotate_contiguous_container(                        \
				ptr(), ptr() + capacity(), ptr() + len(), ptr() + capacity());         \
	}                                                                            \
	auto&& _veglib_asan = defer([&]() noexcept {                                 \
		if (ptr() != nullptr) {                                                    \
			_detail::__sanitizer_annotate_contiguous_container(                      \
					ptr(), ptr() + capacity(), ptr() + capacity(), ptr() + len());       \
		}                                                                          \
	});                                                                          \
	(void)_veglib_asan
#else
#define __VEG_ASAN_ANNOTATE() /* NOLINT */ (void)0;
#endif

namespace collections {
template <
		typename T,
		typename A = mem::SystemAlloc,
		bool NoThrowCopy = VEG_CONCEPT(nothrow_copyable<T>) &&
                       VEG_CONCEPT(nothrow_copy_assignable<T>)>
struct VecIncomplete : private _detail::_vector::adl::AdlBase {
private:
	_detail::_vector::VecAlloc<T, A> _;

public:
	VEG_NODISCARD VEG_INLINE
	VEG_CPP14(constexpr) auto alloc_ref() const VEG_NOEXCEPT -> Ref<A> {
		return ref(_[0_c]);
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto raw_ref() const
			VEG_NOEXCEPT -> Ref<vector::RawVector<T>> {
		return ref(_[1_c]._);
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto alloc_mut(Unsafe /*tag*/)
			VEG_NOEXCEPT -> RefMut<A> {
		return mut(_[0_c]);
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto raw_mut(Unsafe /*tag*/)
			VEG_NOEXCEPT -> RefMut<vector::RawVector<T>> {
		return mut(_[1_c]._);
	}

private:
	VEG_INLINE void _reserve_grow_exact_impl(Unsafe /*tag*/, usize new_cap)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(alloc::nothrow_grow<A>)) {
		__VEG_ASAN_ANNOTATE();

		vector::RawVector<T>& raw = this->raw_mut(unsafe).get();
		auto len = usize(this->len());

		mem::AllocBlock new_block = mem::Alloc<A>::grow(
				this->alloc_mut(unsafe),
				VEG_FWD(raw.data),
				mem::Layout{
						usize(byte_capacity()),
						alignof(T),
				},
				new_cap * sizeof(T),
				mem::RelocFn{collections::relocate_pointer<T>::value});

		T* data = static_cast<T*>(new_block.data);
		raw = {
				data,
				data + len,
				data + new_block.byte_cap / sizeof(T),
		};
	}
	VEG_INLINE void _reserve_grow_exact(Unsafe /*tag*/, isize new_cap)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(alloc::nothrow_grow<A>)) {
		isize old_len = len();
		this->_reserve_grow_exact_impl(unsafe, usize(new_cap));
		meta::unreachable_if(capacity() < new_cap);
		meta::unreachable_if(len() != old_len);
	}
	VEG_INLINE void _reserve_one_more(Unsafe /*tag*/) {
		this->_reserve_grow_exact(
				unsafe,
				1 + isize(
								_detail::_collections::vector_grow_compute(usize(capacity()))));
	}

	static_assert(VEG_CONCEPT(nothrow_move_assignable<A>), ".");
	static_assert(VEG_CONCEPT(nothrow_movable<A>), ".");

public:
	VEG_INLINE ~VecIncomplete() {
		static_assert(
				(VEG_CONCEPT(alloc::nothrow_dealloc<A>) &&
		     VEG_CONCEPT(nothrow_destructible<T>)),
				".");
		vector::RawVector<T> raw = this->raw_ref().get();
		if (raw.data != nullptr) {
			this->clear();
		}
	}

	VEG_INLINE VecIncomplete() = default;

	VEG_INLINE
	VecIncomplete(FromRawParts /*tag*/, vector::RawVector<T> rawvec, A alloc)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<A>))
			: _{
						tuplify,
						VEG_FWD(alloc),
						_detail::_vector::RawVectorMoveRaii<T>{
								from_raw_parts, VEG_FWD(rawvec)},
				} {}

	VEG_INLINE VecIncomplete(VecIncomplete&&) = default;
	VEG_INLINE auto operator=(VecIncomplete&& rhs) -> VecIncomplete& {
		auto tmp = VEG_FWD(rhs);
		{ auto cleanup = static_cast<VecIncomplete&&>(*this); }

		// can't fail
		this->alloc_mut(unsafe).get() =
				static_cast<A&&>(tmp.alloc_mut(unsafe).get());
		this->raw_mut(unsafe).get() = tmp.raw_ref().get();
		tmp.raw_mut(unsafe).get() = {};

		return *this;
	};

	explicit VecIncomplete(VecIncomplete const& rhs) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_copyable<A>) && //
			VEG_CONCEPT(alloc::nothrow_alloc<A>) && NoThrowCopy)
			: _{rhs._} {
		__VEG_ASAN_ANNOTATE();
		vector::RawVector<T> rhs_raw = rhs.raw_ref().get();
		mem::AllocBlock blk = _detail::_collections::alloc_and_copy(
				this->alloc_mut(unsafe),
				mut(mem::DefaultCloner{}),
				rhs_raw.data,
				usize(rhs.len()));

		T* data = static_cast<T*>(blk.data);
		this->raw_mut(unsafe).get() = vector::RawVector<T>{
				data,
				data + usize(rhs.len()),
				data + blk.byte_cap / sizeof(T),
		};
	}

	auto operator=(VecIncomplete const& rhs) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_copy_assignable<A>) &&
			VEG_CONCEPT(alloc::nothrow_alloc<A>) && NoThrowCopy) -> VecIncomplete& {
		if (this != mem::addressof(rhs)) {
			__VEG_ASAN_ANNOTATE();

			_detail::_collections::clone_from(
					this->alloc_mut(unsafe),
					mut(mem::DefaultCloner{}),
					this->raw_mut(unsafe).get(),
					rhs.alloc_ref(),
					rhs.raw_ref().get());
		}
		return *this;
	}

	VEG_INLINE void reserve_exact(isize new_cap)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(alloc::nothrow_grow<A>)) {
		if (new_cap > capacity()) {
			this->_reserve_grow_exact(unsafe, new_cap);
		}
	}
	VEG_INLINE void reserve(isize new_cap)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(alloc::nothrow_grow<A>)) {
		auto cap = capacity();
		if (new_cap > cap) {
			this->reserve_exact(isize(_detail::_collections::vector_grow_choose(
					usize(cap), usize(new_cap))));
		}
	}

	VEG_INLINE void pop_several_unchecked(Unsafe unsafe, isize n) VEG_NOEXCEPT {
		VEG_DEBUG_ASSERT(n <= len());
		__VEG_ASAN_ANNOTATE();

		vector::RawVector<T>& raw = this->raw_mut(unsafe).get();

		T* end = raw.end;
		raw.end -= n;
		_detail::_collections::backward_destroy(
				this->alloc_mut(unsafe), mut(mem::DefaultCloner{}), end - n, end);
	}

	VEG_INLINE void pop_several(isize n) VEG_NOEXCEPT {
		VEG_ASSERT(n <= len());
		pop_several_unchecked(unsafe, n);
	}

	VEG_INLINE auto pop_unchecked(Unsafe /*unsafe*/)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) -> T {
		VEG_DEBUG_ASSERT(1 <= len());
		T* last = raw_ref().get().end - 1;
		T t = static_cast<T&&>(*last);
		--raw_mut(unsafe).get().end;
		mem::destroy_at(last);
		return t;
	}
	VEG_INLINE auto pop_mid_unchecked(Unsafe /*unsafe*/, isize i)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) -> T {
		VEG_DEBUG_ASSERT(i <= len());
		T* elem = raw_ref().get().data + i;
		T t = static_cast<T&&>(*elem);

		--raw_mut(unsafe).get().end;

		// this block does not throw
		{
			mem::destroy_at(elem);
			_detail::_collections::relocate_backward<T>( //
					elem,
					elem + 1,
					sizeof(T) * usize(len() - i - 1));
		}
		return t;
	}

	VEG_INLINE auto pop() VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) -> T {
		VEG_ASSERT(1 <= len());
		return pop_unchecked(unsafe);
	}
	VEG_INLINE auto pop_mid(isize i)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) -> T {
		VEG_ASSERT(i <= len());
		return pop_mid_unchecked(unsafe, i);
	}

	VEG_INLINE void clear() VEG_NOEXCEPT { pop_several_unchecked(unsafe, len()); }

	VEG_TEMPLATE(
			typename U = T,
			requires(VEG_CONCEPT(constructible<U>)),
			void resize,
			(n, isize))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(alloc::nothrow_grow<A>) &&
			VEG_CONCEPT(nothrow_constructible<T>)) {
		__VEG_ASAN_ANNOTATE();

		vector::RawVector<T>& raw = raw_mut(unsafe).get();

		if (n > len()) {
			reserve(n);
			::new (static_cast<void*>(ptr_mut() + len())) T[usize(n - len())]{};
			raw.end = raw.data + n;
		} else {
			pop_several_unchecked(unsafe, len() - n);
		}
	}

	VEG_TEMPLATE(
			typename U = T,
			requires(VEG_CONCEPT(constructible<U>)),
			void resize_for_overwrite,
			(n, isize))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(alloc::nothrow_grow<A>) &&
			VEG_CONCEPT(nothrow_constructible<T>)) {
		__VEG_ASAN_ANNOTATE();

		vector::RawVector<T>& raw = raw_mut(unsafe).get();

		if (n > len()) {
			reserve(n);
			::new (static_cast<void*>(ptr_mut() + len())) T[usize(n - len())];
			raw.end = raw.data + n;
		} else {
			pop_several_unchecked(unsafe, len() - n);
		}
	}

	VEG_TEMPLATE(
			typename Fn,
			requires(VEG_CONCEPT(fn_once<Fn, T>)),
			VEG_INLINE void push_with_unchecked,
			(/*tag*/, Unsafe),
			(fn, Fn))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn_once<Fn, T>) &&
			VEG_CONCEPT(alloc::nothrow_alloc<A>)) {
		__VEG_ASAN_ANNOTATE();

		vector::RawVector<T>& raw = this->raw_mut(unsafe).get();
		mem::construct_with(raw.end, VEG_FWD(fn));
		++raw.end;
	}

	VEG_TEMPLATE(
			typename Fn,
			requires(VEG_CONCEPT(fn_once<Fn, T>)),
			VEG_INLINE void push_with,
			(fn, Fn))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn_once<Fn, T>) &&
			VEG_CONCEPT(alloc::nothrow_alloc<A>)) {
		vector::RawVector<T> raw = this->raw_ref().get();
		if (HEDLEY_UNLIKELY(raw.end == raw.end_alloc)) {
			this->_reserve_one_more(unsafe);
		}
		this->push_with_unchecked(unsafe, VEG_FWD(fn));
	}
	VEG_INLINE void push(T value) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<T>) && VEG_CONCEPT(alloc::nothrow_alloc<A>)) {
		this->push_with(_detail::MoveFn<T>{VEG_FWD(value)});
	}
	VEG_INLINE void push_unchecked(Unsafe /*tag*/, T value)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) {
		this->push_with_unchecked(unsafe, _detail::MoveFn<T>{VEG_FWD(value)});
	}

	VEG_NODISCARD VEG_INLINE auto as_ref() const VEG_NOEXCEPT -> Slice<T> {
		return {from_raw_parts, unsafe, ptr(), len()};
	}
	VEG_NODISCARD VEG_INLINE auto as_mut() VEG_NOEXCEPT -> SliceMut<T> {
		return {from_raw_parts, unsafe, ptr_mut(), len()};
	}

	VEG_NODISCARD VEG_INLINE auto ptr() const VEG_NOEXCEPT -> T const* {
		return this->raw_ref().get().data;
	}
	VEG_NODISCARD VEG_INLINE auto ptr_mut() VEG_NOEXCEPT -> T* {
		return const_cast<T*>(this->ptr());
	}
	VEG_NODISCARD VEG_INLINE auto len() const VEG_NOEXCEPT -> isize {
		auto& raw = this->raw_ref().get();
		return isize(raw.end - raw.data);
	}
	VEG_NODISCARD VEG_INLINE auto capacity() const VEG_NOEXCEPT -> isize {
		auto& raw = this->raw_ref().get();
		return isize(raw.end_alloc - raw.data);
	}
	VEG_NODISCARD VEG_INLINE auto byte_capacity() const VEG_NOEXCEPT -> isize {
		auto& raw = this->raw_ref().get();
		return meta::is_consteval()
		           ? (raw.end_alloc - raw.data) * isize(sizeof(T))
		           : (reinterpret_cast<char const*>(raw.end_alloc) -
		              reinterpret_cast<char const*>(raw.data));
	}
	VEG_NODISCARD VEG_INLINE auto operator[](isize i) const VEG_NOEXCEPT
			-> T const& {
		VEG_ASSERT(usize(i) < usize(len()));
		return this->ptr()[i];
	}
	VEG_NODISCARD VEG_INLINE auto operator[](isize i) VEG_NOEXCEPT -> T& {
		return const_cast<T&>(
				static_cast<VecIncomplete const*>(this)->operator[](i));
	}
};
} // namespace collections

template <typename T, typename A = mem::SystemAlloc>
struct Vec : collections::VecIncomplete<T, A>,
						 meta::conditional_t<
								 VEG_CONCEPT(copyable<T>),
								 _detail::EmptyI<0>,
								 _detail::NoCopyCtor>,
						 meta::conditional_t<
								 VEG_CONCEPT(copy_assignable<T>),
								 _detail::EmptyI<1>,
								 _detail::NoCopyAssign> {

	using collections::VecIncomplete<T, A>::VecIncomplete;
	Vec() = default;
	VEG_EXPLICIT_COPY(Vec);
};

namespace _detail {
namespace _vector {
namespace adl {
VEG_TEMPLATE(
		(typename T, typename LhsA, typename U, typename RhsA),
		requires(VEG_CONCEPT(eq<T, U>)),
		VEG_NODISCARD static VEG_CPP14(constexpr) auto
		operator==,
		(lhs, collections::VecIncomplete<T, LhsA> const&),
		(rhs, collections::VecIncomplete<U, RhsA> const&))
VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<T, U>))->bool {
	return _detail::_slice::adl::operator==(lhs.as_ref(), rhs.as_ref());
}
} // namespace adl
struct DbgVecI {
	template <typename T, typename A>
	static void
	to_string(fmt::BufferMut out, Ref<collections::VecIncomplete<T, A>> arg) {
		fmt::Debug<Slice<T>>::to_string(out, ref(arg.get().as_ref()));
	}
};
struct DbgVec {
	template <typename T, typename A>
	static void to_string(fmt::BufferMut out, Ref<Vec<T, A>> arg) {
		fmt::Debug<Slice<T>>::to_string(out, ref(arg.get().as_ref()));
	}
};
struct OrdVec {
	VEG_TEMPLATE(
			(typename T, typename LhsA, typename U, typename RhsA),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_NODISCARD static constexpr auto cmp,
			(lhs, Ref<Vec<T, LhsA>>),
			(rhs, Ref<Vec<U, RhsA>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->cmp::Ordering {
		return cmp::Ord<Slice<T>, Slice<U>>::cmp( //
				ref(lhs.get().as_ref()),
				ref(rhs.get().as_ref()));
	}
};
struct OrdVecI {
	VEG_TEMPLATE(
			(typename T, typename LhsA, typename U, typename RhsA),
			requires(VEG_CONCEPT(ord<T, U>)),
			VEG_NODISCARD static constexpr auto cmp,
			(lhs, Ref<collections::VecIncomplete<T, LhsA>>),
			(rhs, Ref<collections::VecIncomplete<U, RhsA>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<T, U>))->cmp::Ordering {
		return cmp::Ord<Slice<T>, Slice<U>>::cmp( //
				ref(lhs.get().as_ref()),
				ref(rhs.get().as_ref()));
	}
};
} // namespace _vector
} // namespace _detail

template <typename T, typename A>
struct cpo::is_trivially_relocatable<collections::VecIncomplete<T, A>>
		: cpo::is_trivially_relocatable<A> {};
template <typename T, typename A>
struct cpo::is_trivially_constructible<collections::VecIncomplete<T, A>>
		: cpo::is_trivially_constructible<A> {};

template <typename T, typename A>
struct cpo::is_trivially_relocatable<Vec<T, A>>
		: cpo::is_trivially_relocatable<A> {};
template <typename T, typename A>
struct cpo::is_trivially_constructible<Vec<T, A>>
		: cpo::is_trivially_constructible<A> {};

template <typename T, typename A>
struct fmt::Debug<Vec<T, A>> : _detail::_vector::DbgVec {};
template <typename T, typename A>
struct fmt::Debug<collections::VecIncomplete<T, A>>
		: _detail::_vector::DbgVecI {};

template <typename T, typename LhsA, typename U, typename RhsA>
struct cmp::Ord<Vec<T, LhsA>, Vec<U, RhsA>> : _detail::_vector::OrdVec {};
template <typename T, typename LhsA, typename U, typename RhsA>
struct cmp::Ord< //
		collections::VecIncomplete<T, LhsA>,
		collections::VecIncomplete<U, RhsA>> : _detail::_vector::OrdVecI {};
} // namespace veg

#undef __VEG_ASAN_ANNOTATE

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_VECTOR_HPP_QWFSH3ROS */

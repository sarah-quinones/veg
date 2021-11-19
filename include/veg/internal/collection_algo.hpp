#ifndef VEG_CONTAINER_ALGOS_HPP_SGBCMQAYS
#define VEG_CONTAINER_ALGOS_HPP_SGBCMQAYS

#include "veg/memory/alloc.hpp"
#include "veg/util/defer.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace internal {
namespace _collections {
template <bool IsNoExcept>
struct DestroyImpl;

template <>
struct DestroyImpl<true> {
	template <typename T, typename A, typename C>
	VEG_INLINE static VEG_CPP14(constexpr) void fn(
			RefMut<A> alloc, RefMut<C> cloner, T* ptr, T* ptr_end) VEG_NOEXCEPT {
		while (true) {
			if (ptr_end <= ptr) {
				break;
			}
			--ptr_end;
			mem::Cloner<C>::destroy( //
					VEG_FWD(cloner),
					static_cast<T*>(ptr_end),
					VEG_FWD(alloc));
		}
	}
};

template <typename T, typename A, typename C>
struct Cleanup {
	RefMut<A> alloc;
	RefMut<C> cloner;
	T* ptr;
	T* ptr_end;

	VEG_CPP14(constexpr) void operator()() noexcept {
		DestroyImpl<true>::fn(VEG_FWD(alloc), VEG_FWD(cloner), ptr, ptr_end);
	}
};

template <>
struct DestroyImpl<false> {
	template <typename T, typename A, typename C>
	VEG_INLINE static VEG_CPP20(constexpr) void fn(
			RefMut<A> alloc, RefMut<C> cloner, T* ptr, T* ptr_end)
			VEG_NOEXCEPT_IF(false) {

		Defer<Cleanup<T, A, C>> _{{VEG_FWD(alloc), VEG_FWD(cloner), ptr, ptr_end}};

		while (true) {
			if (_.fn.ptr_end <= _.fn.ptr) {
				break;
			}
			--_.fn.ptr_end;
			mem::Cloner<C>::destroy( //
					VEG_FWD(_.fn.cloner),
					static_cast<T*>(_.fn.ptr_end),
					VEG_FWD(_.fn.alloc));
		}
	}
};

template <typename T, typename A, typename C>
VEG_INLINE VEG_CPP14(constexpr) void backward_destroy(
		RefMut<A> alloc, RefMut<C> cloner, T* ptr, T* ptr_end)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(alloc::nothrow_destroy<C, T, A>)) {
	DestroyImpl<VEG_CONCEPT(alloc::nothrow_destroy<C, T, A>)>::fn(
			VEG_FWD(alloc), VEG_FWD(cloner), ptr, ptr_end);
}
} // namespace _collections
} // namespace internal
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_CONTAINER_ALGOS_HPP_SGBCMQAYS */

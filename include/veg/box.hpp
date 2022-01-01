#ifndef VEG_BOX_HPP_HUJDLY6PS
#define VEG_BOX_HPP_HUJDLY6PS

#include "veg/type_traits/constructible.hpp"
#include "veg/memory/alloc.hpp"
#include "veg/tuple.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace internal {
namespace _mem {
template <typename T>
struct UniquePtr {
	T* inner = nullptr;
	UniquePtr() = default;
	UniquePtr(UniquePtr&& rhs) noexcept : inner{rhs.inner} { rhs.inner = {}; }
	VEG_INLINE UniquePtr(UniquePtr const& /*unused*/) noexcept {}
	auto operator=(UniquePtr const&) -> UniquePtr& = delete;
	auto operator=(UniquePtr&&) -> UniquePtr& = delete;
	~UniquePtr() = default;
};

template <typename T, typename A>
struct BoxAlloc : private Tuple<A, UniquePtr<T>> {
	BoxAlloc() = default;

	BoxAlloc(BoxAlloc&&) = default;
	BoxAlloc(BoxAlloc const&) = default;
	auto operator=(BoxAlloc&&) -> BoxAlloc& = delete;
	auto operator=(BoxAlloc const&) -> BoxAlloc& = delete;

	VEG_INLINE constexpr auto alloc_ref() const noexcept -> Ref<A> {
		return ref((*this)[0_c]);
	}
	VEG_INLINE constexpr auto alloc_mut(Unsafe /*tag*/) const noexcept
			-> RefMut<A> {
		return mut((*this)[0_c]);
	}
	VEG_INLINE constexpr auto ptr_ref() const noexcept -> Ref<T*> {
		return ref((*this)[1_c].inner);
	}
	VEG_INLINE constexpr auto ptr_mut(Unsafe /*tag*/) const noexcept
			-> RefMut<T*> {
		return mut((*this)[1_c].inner);
	}

	~BoxAlloc() {
		if ((*this)[1_c].inner != nullptr) {
			mem::Alloc<A>::dealloc();
		}
	}
};
} // namespace _mem
} // namespace internal

namespace mem {
template <typename T, typename A>
struct BoxIncomplete : private internal::_mem::BoxAlloc<T, A> {
	using Base = internal::_mem::BoxAlloc<T, A>;

	BoxIncomplete() = default;
	BoxIncomplete(BoxIncomplete&&) = default;

	BoxIncomplete(BoxIncomplete const& rhs) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_copyable<A>) &&
			VEG_CONCEPT(alloc::nothrow_alloc<A>) && VEG_CONCEPT(nothrow_copyable<T>))
			: Base(static_cast<Base const&>(rhs)) {
		bool could_copy = false;
	}

	~BoxIncomplete() {
		auto ptr = (*this)[1_c].inner;
		if (ptr != nullptr) {
			mem::destroy_at(ptr);
		}
	}
};
} // namespace mem
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_BOX_HPP_HUJDLY6PS */

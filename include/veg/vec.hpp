#ifndef VEG_VECTOR_HPP_QWFSH3ROS
#define VEG_VECTOR_HPP_QWFSH3ROS

#include "veg/internal/delete_special_members.hpp"
#include "veg/memory/alloc.hpp"
#include "veg/internal/algorithm.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/slice.hpp"
#include "veg/memory/alloc.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace tags {
VEG_TAG(from_slice, FromSlice);
} // namespace tags

namespace vector {
template <typename T>
struct RawVector {
	T* data;
	usize len;
	usize cap;
};
} // namespace vector
namespace internal {
namespace _vector {

template <typename T>
struct RawVectorMoveRaii /* NOLINT */ {
	vector::RawVector<T> _ = {};

	RawVectorMoveRaii() = default;
	RawVectorMoveRaii(FromRawParts /*tag*/, vector::RawVector<T> inner)
			VEG_NOEXCEPT : _{inner} {};
	VEG_INLINE VEG_CPP20(constexpr)
			RawVectorMoveRaii(RawVectorMoveRaii&& rhs) VEG_NOEXCEPT : _{rhs._} {
		rhs = {};
	}
	VEG_INLINE VEG_CPP20(constexpr)
			RawVectorMoveRaii(RawVectorMoveRaii const& /*rhs*/) VEG_NOEXCEPT {}
};

#define VEG_DECLPTR(...) (static_cast<__VA_ARGS__*>(nullptr))
#define VEG_DECLREF(...) (*VEG_DECLPTR(__VA_ARGS__))
#define VEG_DECL_VEGREF(...)                                                   \
	(static_cast<::veg::Ref<__VA_ARGS__>&&>(                                     \
			*static_cast<::veg::Ref<__VA_ARGS__>*>(nullptr)))

#define VEG_DECL_VEGMUT(...)                                                   \
	(static_cast<::veg::RefMut<__VA_ARGS__>&&>(                                  \
			*static_cast<::veg::RefMut<__VA_ARGS__>*>(nullptr)))

template <typename T, typename AllocManager>
struct VecSharedDealloc /* NOLINT */ {
	Tuple<AllocManager, RawVectorMoveRaii<T>>
			_; // alloc manager needs to be constructed first

	VEG_INLINE VEG_CPP20(constexpr) ~VecSharedDealloc() VEG_IS_NOEXCEPT(
			VEG_DECLREF(AllocManager).deallocate(VEG_DECLPTR(T), usize{})) {
		vector::RawVector<T> raw = _[1_c]._;
		if ((raw.data != nullptr) && (raw.cap != 0)) {
			_[0_c].deallocate(+raw.data, +raw.cap);
		}
	}
};

struct CopyCtorRaiiTag {};

template <typename T, typename AllocManager>
struct VecSharedImpl {
private:
	VecSharedDealloc<T, AllocManager> self;

public:
	static constexpr bool nothrow_alloc = VEG_IS_NOEXCEPT(
			VEG_DECLREF(AllocManager).allocate(veg::Tag<T>{}, usize{}));
	static constexpr bool nothrow_dealloc = VEG_IS_NOEXCEPT(
			VEG_DECLREF(AllocManager).deallocate(VEG_DECLPTR(T), usize{}));

	static constexpr bool nothrow_dtor = VEG_IS_NOEXCEPT(
			VEG_DECLREF(AllocManager).destroy(VEG_DECLVAL_NOEXCEPT(RefMut<T>)));

	static constexpr bool nothrow_copy_ctor = VEG_IS_NOEXCEP(
			VEG_DECLREF(AllocManager).clone(VEG_DECLVAL_NOEXCEPT(Ref<T>)));
	static constexpr bool nothrow_copy_assign =
			VEG_IS_NOEXCEPT(VEG_DECLREF(AllocManager)
	                        .clone_assign_to(
															VEG_DECLVAL_NOEXCEPT(RefMut<T>),
															VEG_DECLVAL_NOEXCEPT(Ref<T>),
															VEG_DECLVAL_NOEXCEPT(Ref<AllocManager>)));

	static constexpr bool trivial_copy =
			AllocManager::trivial_copy(veg::Tag<T>{});
	static constexpr bool trivial_relocate =
			AllocManager::trivial_relocate(veg::Tag<T>{});
	static constexpr bool trivial_dtor =
			AllocManager::trivial_dtor(veg::Tag<T>{});

	VEG_INLINE VEG_CPP20(constexpr) ~VecSharedImpl()
			VEG_NOEXCEPT_IF((nothrow_dtor && nothrow_dealloc)) {
		vector::RawVector<T> raw = self._[1_c]._;
		if (raw.data != nullptr) {
			this->destroy_nonnull();
		}
	}

	VEG_INLINE VEG_CPP20(constexpr) VecSharedImpl() = default;

	VEG_INLINE VEG_CPP20(constexpr) VecSharedImpl(
			FromRawParts /*tag*/, vector::RawVector<T> rawvec, AllocManager alloc)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<AllocManager>))
			: self{Tuple<AllocManager, RawVectorMoveRaii<T>>{
						direct,
						VEG_FWD(alloc),
						internal::_vector::RawVectorMoveRaii<T>{
								from_raw_parts, VEG_FWD(rawvec)},
				}} {}

	VEG_INLINE VEG_CPP20(constexpr) VecSharedImpl(VecSharedImpl&&) = default;

private:
	VEG_CPP20(constexpr)
	VecSharedImpl(CopyCtorRaiiTag /*tag*/, VecSharedImpl const& rhs)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_copyable<AllocManager>))
			: self{rhs.self} {}

public:
	VEG_NO_INLINE
	VEG_CPP20(constexpr)
	VecSharedImpl(VecSharedImpl const& rhs) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_copyable<AllocManager>) && //
			nothrow_alloc &&                               //
			nothrow_copy_ctor)
			: VecSharedImpl{CopyCtorRaiiTag{}, rhs} // <────────────────────────────┐
	//                                                                          |
	{                                                                        // |
		AllocManager& alloc = self._[0_c];                                     // |
		vector::RawVector<T> rhs_raw = rhs.self._[1_c]._;                      // |
		                                                                       // |
		mem::AllocBlock<T> block = alloc.allocate(+rhs_raw.data);              // |
		self._[1_c]._ = {block.data, 0, block.cap};                            // |
		                                                                       // |
		usize target_len = rhs_raw.len;                                        // |
		usize& current_len = self._[1_c]._.len;                                // |
		                                                                       // |
		struct Copy {                                                          // |
			AllocManager& alloc;                                                 // |
			T const& rhs;                                                        // |
			VEG_INLINE VEG_CPP20(constexpr) auto operator()() const              // |
					VEG_NOEXCEPT_IF(nothrow_copy_ctor) -> T {                        // |
				return alloc.clone(Ref<T>{as_ref, rhs});                           // |
			}                                                                    // |
		};                                                                     // |
		                                                                       // |
		if (trivial_copy && !meta::is_consteval()) {                           // |
			abi::internal::veglib_opaque_memmove(                                // |
					block.data,                                                      // |
					rhs_raw.data,                                                    // |
					sizeof(T) * target_len);                                         // |
		} else {                                                               // |
			for (usize i = 0; i < target_len; ++i) {                             // |
				mem::construct_with(block.data + i, Copy{alloc, rhs_raw.data[i]}); // |
				++current_len;                                                     // |
			}                                                                    // |
		}                                                                      // |
		// cleanup is handled automatically if loop throws,                       |
		// thanks to deferred ctor  ──────────────────────────────────────────────┘
	}

	VEG_NO_INLINE void reserve_exact(usize new_cap);

	VEG_NO_INLINE
	VEG_CPP20(constexpr)
	auto operator=(VecSharedImpl const& rhs) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_copy_assignable<AllocManager>) && //
			nothrow_alloc &&                                      //
			nothrow_copy_assign) -> VecSharedImpl& {
		if (this != mem::addressof(rhs)) {
			// first, copy assign the elements, passing the old and new allocators as
			// arguments
			// then, copy assign the allocator
			//
			// if element assignment fails, destroy the tail, then assign the
			// allocator

			struct AllocAssign {
				AllocManager& lhs;
				AllocManager const& rhs;
				VEG_INLINE VEG_CPP20(constexpr) void operator()() const VEG_NOEXCEPT_IF(
						VEG_CONCEPT(nothrow_copy_assignable<AllocManager>)) {
					lhs = rhs;
				}
			};

			auto&& _assign_alloc = defer(AllocAssign{self._[0_c], rhs.self._[0_c]});
			for (usize i = 0; i < 0; ++i) {
			}
		}
		return *this;
	}

private:
	VEG_INLINE VEG_CPP20(constexpr) void destroy_nonnull()
			VEG_NOEXCEPT_IF(nothrow_dtor) {
		struct Destroy {
			AllocManager& alloc;
			VEG_INLINE VEG_CPP20(constexpr) void operator()(T* ptr) const
					VEG_NOEXCEPT_LIKE(alloc.destroy(ptr)) {
				alloc.destroy(RefMut<T>{as_mut, *VEG_FWD(ptr)});
			}
		};

		AllocManager& alloc = self._[0_c];
		vector::RawVector<T> raw = self._[1_c]._;

		meta::conditional_t<
				trivial_dtor,
				veg::nb::unused,
				veg::nb::backward_destroy_n>{}(raw.data, raw.len, Destroy{alloc});
	}
};
} // namespace _vector
} // namespace internal

template <typename T>
struct Vec;

template <typename T>
struct cpo::is_trivially_relocatable<Vec<T>> : meta::true_type {};
template <typename T>
struct cpo::is_trivially_swappable<Vec<T>> : meta::true_type {};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_VECTOR_HPP_QWFSH3ROS */

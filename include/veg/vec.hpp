#ifndef VEG_VECTOR_HPP_QWFSH3ROS
#define VEG_VECTOR_HPP_QWFSH3ROS

#include "veg/internal/delete_special_members.hpp"
#include "veg/memory/aligned_alloc.hpp"
#include "veg/internal/algorithm.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/slice.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace tags {
VEG_TAG(from_slice, FromSlice);
} // namespace tags

template <typename T>
struct AllocBlock {
	T* data;
	usize cap;
};

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
	static constexpr bool nothrow_alloc =
			VEG_IS_NOEXCEPT(VEG_DECLREF(AllocManager).allocate(+VEG_DECLREF(usize)));
	static constexpr bool nothrow_dealloc =
			VEG_IS_NOEXCEPT(VEG_DECLREF(AllocManager)
	                        .deallocate(VEG_DECLPTR(T), +VEG_DECLREF(usize)));

	static constexpr bool nothrow_dtor =
			VEG_IS_NOEXCEPT(VEG_DECLREF(AllocManager).destroy(VEG_DECL_VEGMUT(T)));

	static constexpr bool nothrow_copy_ctor =
			VEG_IS_NOEXCEP(VEG_DECLREF(AllocManager).clone(VEG_DECL_VEGREF(T)));
	static constexpr bool nothrow_copy_assign =
			VEG_IS_NOEXCEPT(VEG_DECLREF(AllocManager)
	                        .clone_assign_to(
															VEG_DECL_VEGMUT(T),
															VEG_DECL_VEGREF(T),
															VEG_DECL_VEGREF(AllocManager)));

	static constexpr bool nothrow_move_ctor = VEG_IS_NOEXCEPT(
			VEG_DECLREF(AllocManager).move(VEG_DECLVAL_NOEXCEPT(T &&)));
	static constexpr bool nothrow_move_assign =
			VEG_IS_NOEXCEPT(VEG_DECLREF(AllocManager)
	                        .move_assign_to(
															VEG_DECL_VEGMUT(T),
															VEG_DECLVAL_NOEXCEPT(T&&),
															VEG_DECL_VEGREF(AllocManager)));

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
		AllocBlock<T> block = alloc.allocate(+rhs_raw.data);                   // |
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
			abi::internal::opaque_memmove(                                       // |
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

	VEG_NO_INLINE void reserve_exact(usize new_cap) {}

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

namespace internal {
namespace vec_ {

constexpr auto size_hint_2(i64 hint, i64 new_cap) VEG_NOEXCEPT -> i64 {
	return new_cap > hint ? new_cap : hint;
}

constexpr auto size_hint(i64 cap, i64 new_cap) VEG_NOEXCEPT -> i64 {
	return vec_::size_hint_2(cap + cap / 2 + 1, new_cap);
}

template <typename T>
struct VecBase {

private:
	VEG_INLINE
	auto _self() VEG_NOEXCEPT -> Vec<T>& { return static_cast<Vec<T>&>(*this); }

public:
	struct RawParts {
		T* begin;
		usize len;
		usize cap;
	};
	RawParts raw = {};

	VEG_CPP14(constexpr) VecBase() VEG_NOEXCEPT = default;

	VecBase(FromSlice /*tag*/, Slice<T const> slice) VEG_NOEXCEPT_IF(false)
			: VecBase{
						from_raw_parts,
						RawParts{
								static_cast<T*>(mem::aligned_alloc(
										alignof(T), static_cast<i64>(sizeof(T)) * slice.size())),
								0,
								slice.size(),
						},
						unsafe,
				} {
		uninitialized_copy_n(_self().data(), slice.data(), slice.size());
		raw.len = nb::narrow<usize>{}(slice.size());
	}

	VEG_INLINE auto as_slice() const VEG_NOEXCEPT -> Slice<T const> {
		return {
				FromRawParts{},
				raw.begin,
				i64(raw.len),
				unsafe,
		};
	}
	VEG_INLINE auto as_mut_slice() VEG_NOEXCEPT -> Slice<T> {
		return {
				FromRawParts{},
				raw.begin,
				i64(raw.len),
				unsafe,
		};
	}

	VEG_INLINE
	VecBase(VecBase const& rhs) VEG_NOEXCEPT_IF(false)
			: VecBase{
						from_slice,
						rhs.as_slice(),
				} {}

	VEG_INLINE
	VecBase(VecBase&& rhs) VEG_NOEXCEPT : raw{rhs.raw} { rhs.raw = {}; }

	VEG_INLINE
	VecBase(FromRawParts /*tag*/, RawParts parts, Unsafe /*tag*/) VEG_NOEXCEPT
			: raw{parts} {}

	VEG_INLINE
	~VecBase() VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		auto&& cleanup = defer([&]() noexcept {
			mem::aligned_free(raw.begin, alignof(T), nb::narrow<usize>{}(raw.cap));
		});
		internal::algo_::backward_destroy_n_may_throw<T>(raw.begin, raw.len);
	}

	VEG_INLINE
	auto operator=(VecBase const& rhs) & VEG_NOEXCEPT_IF(false) -> VecBase& {
		if (this == addressof(rhs)) {
			return *this;
		}
		_self().copy_from_slice(rhs.as_slice());
		return *this;
	}
	VEG_INLINE
	auto operator=(VecBase&& rhs) &
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) -> VecBase& {
		RawParts tmp_rhs = rhs.raw;
		rhs.raw = RawParts{};

		VecBase tmp_lhs = static_cast<VecBase&&>(*this);
		raw = tmp_rhs.raw;
		return *this;
	}
};

} // namespace vec_
} // namespace internal

template <typename T>
struct Vec : internal::vec_::VecBase<T>,
						 meta::conditional_t<
								 VEG_CONCEPT(copyable<T>),
								 internal::EmptyI<0>,
								 internal::NoCopyCtor>,
						 meta::conditional_t<
								 VEG_CONCEPT(copy_assignable<T>) &&
										 (VEG_CONCEPT(movable<T>) || VEG_CONCEPT(copyable<T>)),
								 internal::EmptyI<1>,
								 internal::NoCopyAssign> {

	using typename internal::vec_::VecBase<T>::RawParts;
	using internal::vec_::VecBase<T>::VecBase;

private:
	using internal::vec_::VecBase<T>::raw;

public:
	VEG_NODISCARD VEG_INLINE auto into_raw_parts() && VEG_NOEXCEPT -> RawParts {
		RawParts copy = raw;
		raw = {};
		return copy;
	}

	VEG_NODISCARD VEG_INLINE auto as_mut_ptr() VEG_NOEXCEPT -> T* {
		return raw.begin;
	}
	VEG_NODISCARD VEG_INLINE auto as_ptr() const VEG_NOEXCEPT -> T const* {
		return raw.begin;
	}
	VEG_NODISCARD VEG_INLINE auto size() const VEG_NOEXCEPT -> i64 {
		return i64(raw.len);
	}
	VEG_NODISCARD VEG_INLINE auto capacity() const VEG_NOEXCEPT -> i64 {
		return i64(raw.cap);
	}

	VEG_INLINE
	void resize_down(i64 new_size)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		VEG_INTERNAL_ASSERT_PRECONDITIONS(new_size >= i64(0), new_size <= size());
		backward_destroy_n(as_mut_ptr() + new_size, size() - new_size);
		raw.len = usize(new_size);
	}

	VEG_CONSTRAINED_MEMBER_FN(
			requires(VEG_CONCEPT(copyable<T>) && VEG_CONCEPT(copy_assignable<T>)),
			void copy_from_slice,
			((rhs, Slice<T const>)),
			&VEG_NOEXCEPT_IF(false)) {
		if (capacity() < rhs.size()) {
			*this = Vec<T>(from_slice, rhs);
			return *this;
		}

		mixed_init_copy_n(as_mut_ptr(), rhs.data(), rhs.size(), size());
		raw.len = rhs.size();

		return *this;
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(invocable_r<Fn, T>)),
			VEG_INLINE auto push_back_with,
			(fn, Fn))
	VEG_NOEXCEPT_IF(false)->T& {
		_grow_to(size() + 1);
		T& ref = *mem::construct_with(as_mut_ptr() + size(), VEG_FWD(fn));
		++raw.len;
		return ref;
	}

	VEG_INLINE auto push_back(T value) VEG_NOEXCEPT_IF(false) -> T& {
		return push_back_with(internal::ConvertingFn<T&&, T>{VEG_FWD(value)});
	}

	VEG_INLINE
	void reserve(i64 new_capacity) VEG_NOEXCEPT_IF(false) {
		if (new_capacity > capacity()) {
			_grow_to_unchecked(new_capacity);
		}
	}

private:
	VEG_NO_INLINE void _grow_to_unchecked(i64 new_capacity)
			VEG_NOEXCEPT_IF(false) {
		raw.begin = internal::algo_::reallocate_memory<T*>(
				as_mut_ptr(), alignof(T), size(), capacity(), new_capacity);
		raw.cap = nb::narrow<usize>{}(new_capacity);
	}

	VEG_INLINE auto _grow_to(i64 n) VEG_NOEXCEPT_IF(false) {
		if (n > capacity()) {
			_grow_to_unchecked(internal::vec_::size_hint(capacity(), n));
		}
	}
};

template <typename T>
struct cpo::is_trivially_relocatable<Vec<T>> : meta::true_type {};
template <typename T>
struct cpo::is_trivially_swappable<Vec<T>> : meta::true_type {};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_VECTOR_HPP_QWFSH3ROS */

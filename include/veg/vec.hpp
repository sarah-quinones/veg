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
struct FromSlice {};
VEG_INLINE_VAR(from_slice, FromSlice);
} // namespace tags

template <typename T>
struct Vec;

namespace internal {
namespace vec_ {

constexpr auto size_hint(i64 cap, i64 new_cap) VEG_NOEXCEPT -> i64 {
	return new_cap > (2 * cap) ? new_cap : 2 * cap;
}

template <typename T>
struct VecBase {

private:
	VEG_INLINE
	auto _self() VEG_NOEXCEPT -> Vec<T>& { return static_cast<Vec<T>&>(*this); }

public:
	struct raw_parts {
		T* begin;
		usize len;
		usize cap;
	};
	raw_parts self = {};

	VEG_CPP14(constexpr) VecBase() VEG_NOEXCEPT = default;

	VecBase(FromSlice /*tag*/, Slice<T const> slice) VEG_NOEXCEPT_IF(false)
			: VecBase{
						from_raw_parts,
						raw_parts{
								static_cast<T*>(mem::aligned_alloc(
										alignof(T), static_cast<i64>(sizeof(T)) * slice.size())),
								0,
								slice.size()},
						unsafe,
				} {
		uninitialized_copy_n(_self().data(), slice.data(), slice.size());
		self.len = nb::narrow<usize>{}(slice.size());
	}

	VEG_INLINE
	VecBase(VecBase const& other) VEG_NOEXCEPT_IF(false)
			: VecBase{
						from_slice,
						Slice<T const>{other.self.begin, i64(other.self.len), unsafe}} {}

	VEG_INLINE
	VecBase(VecBase&& other) VEG_NOEXCEPT : self{other.self} { other.self = {}; }

	VEG_INLINE
	VecBase(FromRawParts /*tag*/, raw_parts parts, Unsafe /*tag*/) VEG_NOEXCEPT
			: self{parts} {}

	VEG_INLINE
	~VecBase() { this->_destroy(); }

	VEG_INLINE
	auto operator=(VecBase const& rhs) & VEG_NOEXCEPT_IF(false) -> VecBase& {
		if (this == addressof(rhs)) {
			return *this;
		}
		_self() = Slice<T const>{rhs.self.begin, i64(rhs.self.len), unsafe};
		return *this;
	}
	VEG_INLINE
	auto operator=(VecBase&& rhs) & VEG_NOEXCEPT -> VecBase& {
		this->_destroy();
		self = rhs.self;
		rhs.self = {};
		return *this;
	}

private:
	VEG_INLINE
	void _destroy() VEG_NOEXCEPT {
		aligned_free(self.begin, alignof(T), nb::narrow<usize>{}(self.cap));
	}
};

} // namespace vec_
} // namespace internal

template <typename T>
struct Vec : internal::vec_::VecBase<T>,
						 meta::conditional_t<
								 VEG_CONCEPT(copy_constructible<T>),
								 internal::EmptyI<0>,
								 internal::NoCopyCtor>,
						 meta::conditional_t<
								 VEG_CONCEPT(copy_assignable<T>) &&
										 (VEG_CONCEPT(move_constructible<T>) ||
                      VEG_CONCEPT(copy_constructible<T>)),
								 internal::EmptyI<1>,
								 internal::NoCopyAssign> {

	using typename internal::vec_::VecBase<T>::raw_parts;
	using internal::vec_::VecBase<T>::VecBase;

private:
	using internal::vec_::VecBase<T>::self;

public:
	VEG_NODISCARD VEG_INLINE auto into_raw_parts() && VEG_NOEXCEPT -> raw_parts {
		raw_parts copy = self;
		self = {};
		return copy;
	}

	VEG_NODISCARD VEG_INLINE auto data() VEG_NOEXCEPT -> T* { return self.begin; }
	VEG_NODISCARD VEG_INLINE auto data() const VEG_NOEXCEPT -> T const* {
		return self.begin;
	}
	VEG_NODISCARD VEG_INLINE auto size() const VEG_NOEXCEPT -> i64 {
		return i64(self.len);
	}
	VEG_NODISCARD VEG_INLINE auto capacity() const VEG_NOEXCEPT -> i64 {
		return i64(self.cap);
	}

	VEG_INLINE
	void resize_down(i64 new_size, Unsafe /*tag*/) VEG_NOEXCEPT {
		backward_destroy_n(data() + new_size, size() - new_size);
		self.len = usize(new_size);
	}
	VEG_INLINE
	void resize_down(i64 new_size, Safe /*tag*/ = {}) VEG_NOEXCEPT {
		VEG_INTERNAL_ASSERT_PRECONDITION(new_size <= size());
		resize_down(new_size, unsafe);
	}

	VEG_CONSTRAINED_MEMBER_FN(
			requires(
					VEG_CONCEPT(copy_constructible<T>) &&
					VEG_CONCEPT(copy_assignable<T>)),
			auto
			operator=,
			((rhs, Slice<T const>)),
			&VEG_NOEXCEPT_IF(false)->Vec&) {
		// FIXME: backward or forward copy
		if (capacity() < rhs.size()) {
			*this = Vec<T>(from_slice, rhs);
			return *this;
		}

		mixed_init_copy_n(data(), rhs.data(), rhs.size(), size());
		self.len = rhs.size();

		return *this;
	}

	VEG_TEMPLATE(
			(typename... Args),
			requires(VEG_CONCEPT(constructible<T, Args&&...>)),
			VEG_INLINE auto emplace_back,
			(... args, Args&&))
	VEG_NOEXCEPT_IF(false)->T& {
		_grow_to(capacity() + 1);
		T& ref = *::new (data() + size()) T(VEG_FWD(args)...);
		++self.len;
		return ref;
	}

	VEG_INLINE auto push_back(T value) VEG_NOEXCEPT_IF(false) {
		return emplace_back(VEG_FWD(value));
	}

	VEG_INLINE
	void reserve(i64 new_capacity) VEG_NOEXCEPT_IF(false) {
		if (new_capacity > capacity()) {
			self.begin = internal::algo_::reallocate_memory(
					data(), alignof(T), size(), capacity(), new_capacity);
			self.cap = nb::narrow<usize>{}(new_capacity);
		}
	}

private:
	VEG_INLINE auto _grow_to(i64 n) VEG_NOEXCEPT_IF(false) {
		if (n > capacity()) {
			reserve(internal::vec_::size_hint(capacity(), n));
		}
	}
};

template <typename T>
struct meta::is_trivially_relocatable<Vec<T>> : true_type {};
template <typename T>
struct meta::is_trivially_swappable<Vec<T>&> : true_type {};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_VECTOR_HPP_QWFSH3ROS */

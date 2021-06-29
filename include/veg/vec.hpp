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
								slice.size()},
						unsafe,
				} {
		uninitialized_copy_n(_self().data(), slice.data(), slice.size());
		raw.len = nb::narrow<usize>{}(slice.size());
	}

	VEG_INLINE
	VecBase(VecBase const& other) VEG_NOEXCEPT_IF(false)
			: VecBase{
						from_slice,
						Slice<T const>{other.raw.begin, i64(other.raw.len), unsafe}} {}

	VEG_INLINE
	VecBase(VecBase&& other) VEG_NOEXCEPT : raw{other.raw} { other.raw = {}; }

	VEG_INLINE
	VecBase(FromRawParts /*tag*/, RawParts parts, Unsafe /*tag*/) VEG_NOEXCEPT
			: raw{parts} {}

	VEG_INLINE
	~VecBase() VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		this->_destroy();
	}

	VEG_INLINE
	auto operator=(VecBase const& rhs) & VEG_NOEXCEPT_IF(false) -> VecBase& {
		if (this == addressof(rhs)) {
			return *this;
		}
		_self() = Slice<T const>{rhs.raw.begin, i64(rhs.raw.len), unsafe};
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

private:
	VEG_INLINE
	void _destroy() VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		internal::algo_::backward_destroy_n_may_throw<T>(raw.begin, raw.len);
		mem::aligned_free(raw.begin, alignof(T), nb::narrow<usize>{}(raw.cap));
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

	VEG_NODISCARD VEG_INLINE auto data() VEG_NOEXCEPT -> T* { return raw.begin; }
	VEG_NODISCARD VEG_INLINE auto data() const VEG_NOEXCEPT -> T const* {
		return raw.begin;
	}
	VEG_NODISCARD VEG_INLINE auto size() const VEG_NOEXCEPT -> i64 {
		return i64(raw.len);
	}
	VEG_NODISCARD VEG_INLINE auto capacity() const VEG_NOEXCEPT -> i64 {
		return i64(raw.cap);
	}

	VEG_INLINE
	void resize_down_unchecked(i64 new_size, Unsafe /*tag*/)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		backward_destroy_n(data() + new_size, size() - new_size);
		raw.len = usize(new_size);
	}
	VEG_INLINE
	void resize_down(i64 new_size)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
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
		T& ref = *mem::construct_with(data() + size(), VEG_FWD(fn));
		++raw.len;
		return ref;
	}

	VEG_INLINE auto push_back(T value) VEG_NOEXCEPT_IF(false) -> T& {
		return push_back_with(internal::ConvertingFn<T&&, T>{VEG_FWD(value)});
	}

	VEG_INLINE
	void reserve(i64 new_capacity) VEG_NOEXCEPT_IF(false) {
		if (new_capacity > capacity()) {
			raw.begin = internal::algo_::reallocate_memory(
					data(), alignof(T), size(), capacity(), new_capacity);
			raw.cap = nb::narrow<usize>{}(new_capacity);
		}
	}

private:
	VEG_NO_INLINE void _grow_to_unchecked(i64 new_capacity)
			VEG_NOEXCEPT_IF(false) {
		raw.begin = internal::algo_::reallocate_memory(
				data(), alignof(T), size(), capacity(), new_capacity);
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

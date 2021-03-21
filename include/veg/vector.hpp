#ifndef __VEG_VECTOR_HPP_QWFSH3ROS
#define __VEG_VECTOR_HPP_QWFSH3ROS

#include "veg/internal/memory.hpp"
#include "veg/internal/algorithm.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/slice.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
inline namespace tags {
struct from_slice_t {};
VEG_INLINE_VAR(from_slice, from_slice_t);
} // namespace tags

template <typename T>
struct vec;

namespace internal {
namespace vec_ {

constexpr auto size_hint(i64 cap, i64 new_cap) noexcept -> i64 {
	return new_cap > (2 * cap) ? new_cap : 2 * cap;
}

template <
		typename T,
		bool =
				VEG_CONCEPT(copy_constructible<T>) && VEG_CONCEPT(copy_assignable<T>)>
struct vec_copy_base {

private:
	HEDLEY_ALWAYS_INLINE
	auto _self() noexcept -> vec<T>& { return static_cast<vec<T>&>(*this); }

public:
	auto operator=(slice<T const> rhs) & noexcept(false) -> vec_copy_base& {
		// FIXME: backward or forward copy
		if (_self().capacity() < rhs.size()) {
			_self() = vec<T>(from_slice, rhs);
			return *this;
		}

		mixed_init_copy_n(_self().data(), rhs.data(), rhs.size(), _self().size());
		_self().self.len = rhs.size();

		return *this;
	}
};

template <typename T>
struct vec_copy_base<T, false> {};

template <
		typename T,
		bool = VEG_CONCEPT(move_constructible<T>) ||
					 VEG_CONCEPT(copy_constructible<T>)>
struct vec_move_base {
private:
	HEDLEY_ALWAYS_INLINE
	auto _self() noexcept -> vec<T>& { return static_cast<vec<T>&>(*this); }

	HEDLEY_ALWAYS_INLINE
	auto _grow_to(i64 n) noexcept(false) {
		if (n > _self().capacity()) {
			reserve(internal::vec_::size_hint(_self().capacity(), n));
		}
	}

public:
	VEG_TEMPLATE(
			(typename... Args),
			requires(VEG_CONCEPT(constructible<T, Args&&...>)),
			HEDLEY_ALWAYS_INLINE auto emplace_back,
			(... args, Args&&))
	noexcept(false) -> T& {
		_grow_to(_self().capacity() + 1);
		T& ref = *new (_self().data() + _self().size()) T(VEG_FWD(args)...);
		++_self().self.len;
		return ref;
	}

	HEDLEY_ALWAYS_INLINE auto push_back(T value) noexcept(false) {
		return emplace_back(VEG_FWD(value));
	}

	HEDLEY_ALWAYS_INLINE
	void reserve(i64 new_capacity) noexcept(false) {
		if (new_capacity > _self().capacity()) {
			_self().self.begin = internal::algo::reallocate_memory(
					_self().data(),
					alignof(T),
					_self().size(),
					_self().capacity(),
					new_capacity);
			_self().self.cap = fn::narrow<usize>{}(new_capacity);
		}
	}
};

template <typename T>
struct vec_move_base<T, false> {};

template <typename T>
struct vec_base {

private:
	HEDLEY_ALWAYS_INLINE
	auto _self() noexcept -> vec<T>& { return static_cast<vec<T>&>(*this); }

public:
	struct raw_parts {
		T* begin;
		usize len;
		usize cap;
	};
	raw_parts self = {};

	VEG_CPP14(constexpr) vec_base() noexcept = default;

	vec_base(from_slice_t /*tag*/, slice<T const> slice) noexcept(false)
			: vec_base{
						from_raw_parts,
						raw_parts{
								static_cast<T*>(internal::memory::aligned_alloc(
										alignof(T), static_cast<i64>(sizeof(T)) * slice.size())),
								0,
								slice.size()},
						unsafe,
				} {
		uninitialized_copy_n(_self().data(), slice.data(), slice.size());
		self.len = fn::narrow<usize>{}(slice.size());
	}

	HEDLEY_ALWAYS_INLINE
	vec_base(vec_base const& other) noexcept(false)
			: vec_base{
						from_slice,
						slice<T const>{other.self.begin, i64(other.self.len), unsafe}} {}

	HEDLEY_ALWAYS_INLINE
	vec_base(vec_base&& other) noexcept : self{other.self} { other.self = {}; }

	HEDLEY_ALWAYS_INLINE
	vec_base(from_raw_parts_t /*tag*/, raw_parts parts, unsafe_t /*tag*/) noexcept
			: self{parts} {}

	HEDLEY_ALWAYS_INLINE
	~vec_base() { aligned_free(self.begin, fn::narrow<usize>{}(self.cap)); }

	HEDLEY_ALWAYS_INLINE
	auto operator=(vec_base const& rhs) & noexcept(false) -> vec_base& {
		if (this == addressof(rhs)) {
			return *this;
		}
		_self() = slice<T const>{rhs.self.begin, i64(rhs.self.len), unsafe};
		return *this;
	}
	HEDLEY_ALWAYS_INLINE
	auto operator=(vec_base&& rhs) & noexcept -> vec_base& {
		this->destroy();
		self = rhs.self;
		rhs.self = {};
		return *this;
	}

	HEDLEY_ALWAYS_INLINE
	void destroy() noexcept {
		aligned_free(self.begin, fn::narrow<usize>{}(self.cap));
	}
};

} // namespace vec_
} // namespace internal

template <typename T>
struct vec : internal::vec_::vec_base<T>,
						 internal::vec_::vec_move_base<T>,
						 internal::vec_::vec_copy_base<T>,
						 meta::conditional_t<
								 VEG_CONCEPT(copy_constructible<T>),
								 meta::internal::empty_i<0>,
								 meta::internal::nocopy_ctor>,
						 meta::conditional_t<
								 VEG_CONCEPT(copy_assignable<T>) &&
										 (VEG_CONCEPT(move_constructible<T>) ||
                      VEG_CONCEPT(copy_constructible<T>)),
								 meta::internal::empty_i<1>,
								 meta::internal::nocopy_assign> {

	using typename internal::vec_::vec_base<T>::raw_parts;
	using internal::vec_::vec_base<T>::vec_base;
	using internal::vec_::vec_copy_base<T>::operator=;

private:
	friend struct internal::vec_::vec_move_base<T>;
	friend struct internal::vec_::vec_copy_base<T>;
	using internal::vec_::vec_base<T>::self;

public:
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto into_raw_parts() && noexcept
			-> raw_parts {
		raw_parts copy = self;
		self = {};
		return copy;
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto data() noexcept -> T* {
		return self.begin;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto data() const noexcept -> T const* {
		return self.begin;
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto size() const noexcept -> i64 {
		return i64(self.len);
	}
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE auto capacity() const noexcept -> i64 {
		return i64(self.cap);
	}

	HEDLEY_ALWAYS_INLINE
	void resize_down(i64 new_size, unsafe_t /*tag*/) noexcept {
		backward_destroy_n(data() + new_size, size() - new_size);
		self.len = usize(new_size);
	}
	HEDLEY_ALWAYS_INLINE
	void resize_down(i64 new_size, safe_t /*tag*/ = {}) noexcept {
		VEG_ASSERT(new_size <= size());
		resize_down(new_size, unsafe);
	}
};

template <typename T>
struct meta::is_trivially_relocatable<vec<T>> : true_type {};
template <typename T>
struct meta::is_trivially_swappable<vec<T>&> : true_type {};
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard __VEG_VECTOR_HPP_QWFSH3ROS */

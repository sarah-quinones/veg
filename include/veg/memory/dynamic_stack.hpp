#ifndef VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS
#define VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS

#include "veg/util/assert.hpp"
#include "veg/slice.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/memory/placement.hpp"
#include "veg/memory/address.hpp"
#include "veg/option.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
auto align_next(i64 alignment, i64 size, void*& ptr, i64& space) noexcept
		-> void*;
}
} // namespace VEG_ABI_VERSION
} // namespace abi

inline namespace VEG_ABI {
namespace internal {
namespace dynstack {

struct cleanup;
struct dynamic_alloc_base;

struct default_init_fn {
	template <typename T>
	auto make(void* ptr, i64 len) -> T* {
		return new (ptr) T[static_cast<usize>(len)];
	}
};

struct zero_init_fn {
	template <typename T>
	auto make(void* ptr, i64 len) -> T* {
		return new (ptr) T[static_cast<usize>(len)]{};
	}
};

struct no_init_fn {
	template <typename T>
	auto make(void* ptr, i64 len) -> T* {
		return mem::launder(static_cast<T*>(static_cast<void*>(
				new (ptr) unsigned char[static_cast<usize>(len) * sizeof(T)])));
	}
};

} // namespace dynstack
} // namespace internal

template <typename T>
struct dynstack_array;
template <typename T>
struct dynstack_alloc;

struct dynamic_stack_view {
public:
	dynamic_stack_view /* NOLINT(hicpp-explicit-conversions) */ (
			slice<void> s) noexcept
			: stack_data(s.data()), stack_bytes(s.size()) {}

	VEG_NODISCARD
	auto remaining_bytes() const noexcept -> i64 { return stack_bytes; }
	VEG_NODISCARD
	auto data() const noexcept -> void* { return stack_data; }

	VEG_TEMPLATE(
			(typename T),
			requires VEG_CONCEPT(constructible<T>),
			VEG_NODISCARD,
			auto make_new,
			(/*unused*/, tag_t<T>),
			(len, i64),
			(align = alignof(T), i64))
	noexcept(VEG_CONCEPT(nothrow_constructible<T>)) -> option<dynstack_array<T>> {
		dynstack_array<T> get{
				*this, len, align, internal::dynstack::zero_init_fn{}};
		if (get.data() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

	VEG_TEMPLATE(
			(typename T),
			requires VEG_CONCEPT(constructible<T>),
			VEG_NODISCARD,
			auto make_new_for_overwrite,
			(/*unused*/, tag_t<T>),
			(len, i64),
			(align = alignof(T), i64))

	noexcept(VEG_CONCEPT(nothrow_constructible<T>)) -> option<dynstack_array<T>> {
		dynstack_array<T> get{
				*this, len, align, internal::dynstack::default_init_fn{}};
		if (get.data() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

	template <typename T>
	VEG_NODISCARD auto
	make_alloc(tag_t<T> /*unused*/, i64 len, i64 align = alignof(T)) noexcept
			-> option<dynstack_alloc<T>> {
		dynstack_alloc<T> get{*this, len, align, internal::dynstack::no_init_fn{}};
		if (get.data() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

private:
	void* stack_data;
	i64 stack_bytes;

	template <typename T>
	friend struct dynstack_alloc;
	template <typename T>
	friend struct dynstack_array;
	friend struct internal::dynstack::cleanup;
	friend struct internal::dynstack::dynamic_alloc_base;
};

namespace internal {
namespace dynstack {

struct cleanup {
	bool const& success;
	dynamic_stack_view& parent;
	void* old_data;
	i64 old_rem_bytes;

	HEDLEY_ALWAYS_INLINE void operator()() const noexcept {
		if (!success) {
			parent.stack_data = old_data;
			parent.stack_bytes = old_rem_bytes;
		}
	}
};

struct dynamic_alloc_base {
	dynamic_stack_view& parent;
	void* old_pos;
	void const volatile* data;
	i64 len;

	void destroy(void const volatile* void_data_end) noexcept {
		if (len != 0) {
			// in case resource lifetimes are reodered by moving ownership
			auto* parent_stack_data = static_cast<unsigned char*>(parent.stack_data);
			auto* old_position = static_cast<unsigned char*>(old_pos);
			auto* data_end =
					static_cast<unsigned char*>(const_cast<void*>(void_data_end));

			VEG_ASSERT_ALL_OF( //
					parent_stack_data == data_end,
					parent_stack_data >= old_position);

			parent.stack_bytes += nb::narrow<i64>{}(
					static_cast<unsigned char*>(parent.stack_data) -
					static_cast<unsigned char*>(old_pos));
			parent.stack_data = old_pos;
		}
	}
};
} // namespace dynstack
} // namespace internal

template <typename T>
struct dynstack_alloc : internal::dynstack::dynamic_alloc_base {
private:
	using base = internal::dynstack::dynamic_alloc_base;

public:
	~dynstack_alloc() { base::destroy(data() + base::len); }
	dynstack_alloc(dynstack_alloc const&) = delete;
	dynstack_alloc(dynstack_alloc&& other) noexcept : base{base(other)} {
		other.base::len = 0;
		other.base::data = nullptr;
	};

	auto operator=(dynstack_alloc const&) -> dynstack_alloc& = delete;
	auto operator=(dynstack_alloc&& rhs) noexcept -> dynstack_alloc& {
		if (this != mem::addressof(rhs)) {
			mem::destroy_at(this);
			mem::construct_at(this, VEG_FWD(rhs));
		}
		return *this;
	}

	VEG_NODISCARD auto data() const noexcept -> T* {
		return static_cast<T*>(const_cast<void*>(base::data));
	}
	VEG_NODISCARD auto size() const noexcept -> i64 { return base::len; }

private:
	friend struct dynstack_array<T>;
	friend struct dynamic_stack_view;

	template <typename Fn>
	dynstack_alloc(
			dynamic_stack_view& parent_ref,
			i64 alloc_size,
			i64 align,
			Fn fn) noexcept(VEG_CONCEPT(nothrow_constructible<T>))
			: base{parent_ref, parent_ref.stack_data, nullptr, 0} {

		void* const parent_data = parent_ref.stack_data;
		i64 const parent_bytes = parent_ref.stack_bytes;

		void* const data = abi::internal::align_next(
				align,
				alloc_size * i64{sizeof(T)},
				parent_ref.stack_data,
				parent_ref.stack_bytes);

		if (data != nullptr) {
			bool success = false;
			auto&& cleanup = make::defer(internal::dynstack::cleanup{
					success, parent, parent_data, parent_bytes});
			(void)cleanup;

			base::len = alloc_size;
			base::data = fn.template make<T>(data, alloc_size);

			success = true;
		}
	}
};

template <typename T>
struct dynstack_array : private dynstack_alloc<T> {

public:
	using dynstack_alloc<T>::data;
	using dynstack_alloc<T>::size;

	dynstack_array(dynstack_array const&) = delete;
	dynstack_array(dynstack_array&&) noexcept = default;
	auto operator=(dynstack_array const&) -> dynstack_array& = delete;
	auto operator=(dynstack_array&& rhs) noexcept -> dynstack_array& {
		if (this != mem::addressof(rhs)) {
			mem::destroy_at(this);
			mem::construct_at(this, VEG_FWD(rhs));
		}
		return *this;
	}

	~dynstack_array() {
		for (i64 i = this->dynstack_alloc<T>::base::len - 1; i >= 0; --i) {
			mem::destroy_at(this->data() + i);
		}
	}

private:
	using dynstack_alloc<T>::dynstack_alloc;
	friend struct dynamic_stack_view;
};
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS   \
        */

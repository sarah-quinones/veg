#ifndef VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS
#define VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS

#include "veg/util/assert.hpp"
#include "veg/slice.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/memory/placement.hpp"
#include "veg/memory/address.hpp"
#include "veg/option.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/internal/algorithm.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
auto align_next(i64 alignment, i64 size, void*& ptr, i64& space) VEG_NOEXCEPT
		-> void*;
}
} // namespace VEG_ABI_VERSION
} // namespace abi

namespace internal {
namespace dynstack {

struct cleanup;
struct DynAllocBase;

struct default_init_fn {
	template <typename T>
	auto make(void* ptr, i64 len) -> T* {
		return ::new (ptr) T[static_cast<usize>(len)];
	}
};

struct zero_init_fn {
	template <typename T>
	auto make(void* ptr, i64 len) -> T* {
		return ::new (ptr) T[static_cast<usize>(len)]{};
	}
};

struct no_init_fn {
	template <typename T>
	auto make(void* ptr, i64 len) -> T* {
		return mem::launder(static_cast<T*>(static_cast<void*>(
				::new (ptr) unsigned char[static_cast<usize>(len) * sizeof(T)])));
	}
};

} // namespace dynstack
} // namespace internal

template <typename T>
struct DynStackArray;
template <typename T>
struct DynStackAlloc;

struct DynStackView {
public:
	DynStackView(SliceMut<unsigned char> s) VEG_NOEXCEPT
			: stack_data(s.as_mut_ptr()),
				stack_bytes(s.len()) {}

	VEG_NODISCARD
	auto remaining_bytes() const VEG_NOEXCEPT -> i64 { return stack_bytes; }
	VEG_NODISCARD
	auto as_mut_ptr() const VEG_NOEXCEPT -> void* { return stack_data; }
	VEG_NODISCARD
	auto as_ptr() const VEG_NOEXCEPT -> void const* { return stack_data; }

	VEG_TEMPLATE(
			(typename T),
			requires VEG_CONCEPT(constructible<T>),
			VEG_NODISCARD auto make_new,
			(/*unused*/, Tag<T>),
			(len, i64),
			(align = alignof(T), i64))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T>))
			->Option<DynStackArray<T>> {
		DynStackArray<T> get{*this, len, align, internal::dynstack::zero_init_fn{}};
		if (get.as_ptr() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

	VEG_TEMPLATE(
			(typename T),
			requires VEG_CONCEPT(constructible<T>),
			VEG_NODISCARD auto make_new_for_overwrite,
			(/*unused*/, Tag<T>),
			(len, i64),
			(align = alignof(T), i64))

	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T>))
			->Option<DynStackArray<T>> {
		DynStackArray<T> get{
				*this, len, align, internal::dynstack::default_init_fn{}};
		if (get.as_ptr() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

	template <typename T>
	VEG_NODISCARD auto
	make_alloc(Tag<T> /*unused*/, i64 len, i64 align = alignof(T)) VEG_NOEXCEPT
			-> Option<DynStackAlloc<T>> {
		DynStackAlloc<T> get{*this, len, align, internal::dynstack::no_init_fn{}};
		if (get.as_ptr() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

private:
	void* stack_data;
	i64 stack_bytes;

	template <typename T>
	friend struct DynStackAlloc;
	template <typename T>
	friend struct DynStackArray;
	friend struct internal::dynstack::cleanup;
	friend struct internal::dynstack::DynAllocBase;
};

namespace internal {
namespace dynstack {

struct cleanup {
	bool const& success;
	DynStackView& parent;
	void* old_data;
	i64 old_rem_bytes;

	VEG_INLINE void operator()() const VEG_NOEXCEPT {
		if (!success) {
			parent.stack_data = old_data;
			parent.stack_bytes = old_rem_bytes;
		}
	}
};

struct DynAllocBase {
	DynStackView* parent;
	void* old_pos;
	void const volatile* data;
	i64 len;

	void destroy(void const volatile* void_data_end) VEG_NOEXCEPT {
		if (len != 0) {
			// in case resource lifetimes are reodered by moving ownership
			auto* parent_stack_data = static_cast<unsigned char*>(parent->stack_data);
			auto* old_position = static_cast<unsigned char*>(old_pos);
			auto* data_end =
					static_cast<unsigned char*>(const_cast<void*>(void_data_end));

			VEG_INTERNAL_ASSERT_PRECONDITIONS( //
					parent_stack_data == data_end,
					parent_stack_data >= old_position);

			parent->stack_bytes += nb::narrow<i64>{}(
					static_cast<unsigned char*>(parent->stack_data) -
					static_cast<unsigned char*>(old_pos));
			parent->stack_data = old_pos;
		}
	}
};
} // namespace dynstack
} // namespace internal

template <typename T>
struct DynStackAlloc : internal::dynstack::DynAllocBase {
private:
	using Base = internal::dynstack::DynAllocBase;

public:
	~DynStackAlloc() VEG_NOEXCEPT { Base::destroy(as_mut_ptr() + Base::len); }

	DynStackAlloc(DynStackAlloc const&) = delete;
	DynStackAlloc(DynStackAlloc&& other) VEG_NOEXCEPT : Base{Base(other)} {
		other.Base::len = 0;
		other.Base::data = nullptr;
	};

	auto operator=(DynStackAlloc const&) -> DynStackAlloc& = delete;
	auto operator=(DynStackAlloc&& rhs) VEG_NOEXCEPT -> DynStackAlloc& {
		Base tmp_rhs = static_cast<Base>(rhs);
		static_cast<Base&>(rhs) = {};

		DynStackAlloc tmp_lhs = static_cast<DynStackAlloc&&>(*this);
		static_cast<Base&>(*this) = tmp_rhs;
		return *this;
	}

	VEG_NODISCARD auto as_mut_ptr() VEG_NOEXCEPT -> T* {
		return static_cast<T*>(const_cast<void*>(Base::data));
	}
	VEG_NODISCARD auto as_ptr() const VEG_NOEXCEPT -> T const* {
		return static_cast<T const*>(const_cast<void const*>(Base::data));
	}
	VEG_NODISCARD auto len() const VEG_NOEXCEPT -> i64 { return Base::len; }

private:
	friend struct DynStackArray<T>;
	friend struct DynStackView;

	template <typename Fn>
	DynStackAlloc(DynStackView& parent_ref, i64 alloc_size, i64 align, Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T>))
			: Base{
						&parent_ref,
						parent_ref.stack_data,
						nullptr,
						0,
				} {

		void* const parent_data = parent_ref.stack_data;
		i64 const parent_bytes = parent_ref.stack_bytes;

		void* const ptr = abi::internal::align_next(
				align,
				alloc_size * i64{sizeof(T)},
				parent_ref.stack_data,
				parent_ref.stack_bytes);

		if (ptr != nullptr) {
			bool success = false;
			auto&& cleanup = defer(internal::dynstack::cleanup{
					success, *parent, parent_data, parent_bytes});
			(void)cleanup;

			Base::len = alloc_size;
			Base::data = fn.template make<T>(ptr, alloc_size);

			success = true;
		}
	}
};

template <typename T>
struct DynStackArray : private DynStackAlloc<T> {
private:
	using Base = internal::dynstack::DynAllocBase;

public:
	using DynStackAlloc<T>::as_ptr;
	using DynStackAlloc<T>::as_mut_ptr;
	using DynStackAlloc<T>::len;

	DynStackArray(DynStackArray const&) = delete;
	DynStackArray(DynStackArray&&) VEG_NOEXCEPT = default;
	auto operator=(DynStackArray const&) -> DynStackArray& = delete;

	auto operator=(DynStackArray&& rhs) VEG_NOEXCEPT -> DynStackArray& {
		Base tmp_rhs = static_cast<Base>(rhs);
		static_cast<Base&>(rhs) = {};

		DynStackArray tmp_lhs = static_cast<DynStackArray&&>(*this);
		static_cast<Base&>(*this) = tmp_rhs;
		return *this;
	}

	~DynStackArray() VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		internal::algo_::backward_destroy_n_may_throw<T>(
				as_mut_ptr(), this->DynStackAlloc<T>::Base::len);
	}

private:
	using DynStackAlloc<T>::DynStackAlloc;
	friend struct DynStackView;
};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS   \
        */

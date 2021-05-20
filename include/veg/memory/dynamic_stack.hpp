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
	DynStackView /* NOLINT(hicpp-explicit-conversions) */ (Slice<void> s)
			VEG_NOEXCEPT : stack_data(s.data()),
										 stack_bytes(s.size()) {}

	VEG_NODISCARD
	auto remaining_bytes() const VEG_NOEXCEPT -> i64 { return stack_bytes; }
	VEG_NODISCARD
	auto data() const VEG_NOEXCEPT -> void* { return stack_data; }

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
		if (get.data() == nullptr) {
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
		if (get.data() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

	template <typename T>
	VEG_NODISCARD auto
	make_alloc(Tag<T> /*unused*/, i64 len, i64 align = alignof(T)) VEG_NOEXCEPT
			-> Option<DynStackAlloc<T>> {
		DynStackAlloc<T> get{*this, len, align, internal::dynstack::no_init_fn{}};
		if (get.data() == nullptr) {
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
	using base = internal::dynstack::DynAllocBase;

public:
	~DynStackAlloc()
#ifdef __VEG_DISABLE_NOEXCEPT
			noexcept(false)
#endif
	{
		base::destroy(data() + base::len);
	}

	DynStackAlloc(DynStackAlloc const&) = delete;
	DynStackAlloc(DynStackAlloc&& other) VEG_NOEXCEPT : base{base(other)} {
		other.base::len = 0;
		other.base::data = nullptr;
	};

	auto operator=(DynStackAlloc const&) -> DynStackAlloc& = delete;
	auto operator=(DynStackAlloc&& rhs) VEG_NOEXCEPT -> DynStackAlloc& {
		base::destroy(data() + base::len);
		static_cast<base&>(*this) = static_cast<base&>(rhs);
		static_cast<base&>(rhs) = {};
		return *this;
	}

	VEG_NODISCARD auto data() const VEG_NOEXCEPT -> T* {
		return static_cast<T*>(const_cast<void*>(base::data));
	}
	VEG_NODISCARD auto size() const VEG_NOEXCEPT -> i64 { return base::len; }

private:
	friend struct DynStackArray<T>;
	friend struct DynStackView;

	template <typename Fn>
	DynStackAlloc(DynStackView& parent_ref, i64 alloc_size, i64 align, Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T>))
			: base{&parent_ref, parent_ref.stack_data, nullptr, 0} {

		void* const parent_data = parent_ref.stack_data;
		i64 const parent_bytes = parent_ref.stack_bytes;

		void* const data = abi::internal::align_next(
				align,
				alloc_size * i64{sizeof(T)},
				parent_ref.stack_data,
				parent_ref.stack_bytes);

		if (data != nullptr) {
			bool success = false;
			auto&& cleanup = defer(internal::dynstack::cleanup{
					success, *parent, parent_data, parent_bytes});
			(void)cleanup;

			base::len = alloc_size;
			base::data = fn.template make<T>(data, alloc_size);

			success = true;
		}
	}
};

template <typename T>
struct DynStackArray : private DynStackAlloc<T> {
private:
	using base = internal::dynstack::DynAllocBase;

public:
	using DynStackAlloc<T>::data;
	using DynStackAlloc<T>::size;

	DynStackArray(DynStackArray const&) = delete;
	DynStackArray(DynStackArray&&) VEG_NOEXCEPT = default;
	auto operator=(DynStackArray const&) -> DynStackArray& = delete;

	auto operator=(DynStackArray&& rhs) VEG_NOEXCEPT -> DynStackArray& {
		for (i64 i = this->DynStackAlloc<T>::base::len - 1; i >= 0; --i) {
			mem::destroy_at(this->data() + i);
		}
		base::destroy(data() + base::len);
		static_cast<base&>(*this) = static_cast<base&>(rhs);
		static_cast<base&>(rhs) = {};
		return *this;
	}

	~DynStackArray()
#ifdef __VEG_DISABLE_NOEXCEPT
			noexcept(false)
#endif
	{
		for (i64 i = this->DynStackAlloc<T>::base::len - 1; i >= 0; --i) {
			mem::destroy_at(this->data() + i);
		}
	}

private:
	using DynStackAlloc<T>::DynStackAlloc;
	friend struct DynStackView;
};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS   \
        */

#ifndef VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS
#define VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS

#include "veg/util/assert.hpp"
#include "veg/internal/collection_algo.hpp"
#include "veg/memory/alloc.hpp"
#include "veg/memory/placement.hpp"
#include "veg/slice.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/memory/placement.hpp"
#include "veg/memory/address.hpp"
#include "veg/option.hpp"
#include "veg/internal/narrow.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
template <typename T>
struct DynStackArray;
template <typename T>
struct DynStackAlloc;

namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
// if possible:
// aligns the pointer
// then advances it by `size` bytes, and decreases `space` by `size`
// returns the previous aligned value
//
// otherwise, if there is not enough space for aligning or advancing the
// pointer, returns nullptr and the values are left unmodified
inline auto align_next(usize alignment, usize size, void*& ptr, usize& space)
		VEG_ALWAYS_NOEXCEPT -> void* {
	static_assert(
			sizeof(std::uintptr_t) >= sizeof(void*),
			"std::uintptr_t can't hold a pointer value");

	using byte_ptr = unsigned char*;

	// assert alignment is power of two
	VEG_ASSERT_ALL_OF( //
			(alignment > usize{0}),
			((u64(alignment) & (u64(alignment) - 1)) == u64(0)));

	if (space < size) {
		return nullptr;
	}

	std::uintptr_t lo_mask = alignment - 1;
	std::uintptr_t hi_mask = ~lo_mask;

	auto const intptr = reinterpret_cast<std::uintptr_t>(ptr);
	auto* const byteptr = static_cast<byte_ptr>(ptr);

	auto offset = ((intptr + alignment - 1) & hi_mask) - intptr;

	if (space - size < offset) {
		return nullptr;
	}

	void* const rv = byteptr + offset;

	ptr = byteptr + (offset + size);
	space = space - (offset + size);

	return rv;
}
} // namespace internal
} // namespace VEG_ABI_VERSION
} // namespace abi

namespace internal {
namespace dynstack {
template <typename T, bool = VEG_CONCEPT(trivially_destructible<T>)>
struct DynStackArrayDtor {};

template <typename T>
struct DynStackArrayDtor<T, false> {
	DynStackArrayDtor() = default;
	DynStackArrayDtor(DynStackArrayDtor const&) = default;
	DynStackArrayDtor(DynStackArrayDtor&&) = default;
	auto operator=(DynStackArrayDtor const&) -> DynStackArrayDtor& = default;
	auto operator=(DynStackArrayDtor&&) -> DynStackArrayDtor& = default;
	VEG_INLINE ~DynStackArrayDtor()
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		auto& self = static_cast<DynStackArray<T>&>(*this);
		veg::internal::_collections::backward_destroy(
				mut(mem::SystemAlloc{}),
				mut(mem::DefaultCloner{}),
				self.mut_ptr(),
				self.mut_ptr() + self.DynStackAlloc<T>::Base::len);
	}
};

struct cleanup;
struct DynAllocBase;

struct default_init_fn {
	template <typename T>
	auto make(void* ptr, usize len) -> T* {
		return ::new (ptr) T[len];
	}
};

struct zero_init_fn {
	template <typename T>
	auto make(void* ptr, usize len) -> T* {
		return ::new (ptr) T[len]{};
	}
};

struct no_init_fn {
	template <typename T>
	auto make(void* ptr, usize len) -> T* {
		return veg::mem::launder(static_cast<T*>(
				static_cast<void*>(::new (ptr) unsigned char[len * sizeof(T)])));
	}
};

} // namespace dynstack
} // namespace internal

struct DynStackView {
public:
	DynStackView(SliceMut<unsigned char> s) VEG_NOEXCEPT
			: stack_data(s.mut_ptr()),
				stack_bytes(usize(s.len())) {}

	VEG_NODISCARD
	auto remaining_bytes() const VEG_NOEXCEPT -> isize {
		return isize(stack_bytes);
	}
	VEG_NODISCARD
	auto mut_ptr() const VEG_NOEXCEPT -> void* { return stack_data; }
	VEG_NODISCARD
	auto ptr() const VEG_NOEXCEPT -> void const* { return stack_data; }

private:
	VEG_INLINE void assert_valid_len(usize len, usize sizeofT) VEG_NOEXCEPT {
		VEG_INTERNAL_ASSERT_PRECONDITION(usize(len) < ((-usize(1)) / sizeofT));
	}

public:
	VEG_TEMPLATE(
			(typename T),
			requires VEG_CONCEPT(constructible<T>),
			VEG_NODISCARD auto make_new,
			(/*unused*/, Tag<T>),
			(len, usize),
			(align = alignof(T), usize))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T>))
			->Option<DynStackArray<T>> {
		assert_valid_len(len, sizeof(T));
		DynStackArray<T> get{
				*this, usize(len), align, internal::dynstack::zero_init_fn{}};
		if (get.ptr() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

	VEG_TEMPLATE(
			(typename T),
			requires VEG_CONCEPT(constructible<T>),
			VEG_NODISCARD auto make_new_for_overwrite,
			(/*unused*/, Tag<T>),
			(len, usize),
			(align = alignof(T), usize))

	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T>))
			->Option<DynStackArray<T>> {
		assert_valid_len(len, sizeof(T));
		DynStackArray<T> get{
				*this, usize(len), align, internal::dynstack::default_init_fn{}};
		if (get.ptr() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

	template <typename T>
	VEG_NODISCARD auto make_alloc(
			Tag<T> /*unused*/, usize len, usize align = alignof(T)) VEG_NOEXCEPT
			-> Option<DynStackAlloc<T>> {
		assert_valid_len(len, sizeof(T));
		DynStackAlloc<T> get{
				*this, usize(len), align, internal::dynstack::no_init_fn{}};
		if (get.ptr() == nullptr) {
			return none;
		}
		return {some, VEG_FWD(get)};
	}

private:
	void* stack_data;
	usize stack_bytes;

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
	usize old_rem_bytes;

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
	usize len;

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

			parent->stack_bytes += static_cast<usize>(
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
	VEG_INLINE ~DynStackAlloc() VEG_NOEXCEPT {
		Base::destroy(mut_ptr() + Base::len);
	}

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

	VEG_NODISCARD auto as_mut() VEG_NOEXCEPT -> SliceMut<T> {
		return {
				FromRawParts{},
				mut_ptr(),
				len(),
				unsafe,
		};
	}

	VEG_NODISCARD auto as_ref() const VEG_NOEXCEPT -> Slice<T> {
		return {
				FromRawParts{},
				ptr(),
				len(),
				unsafe,
		};
	}

	VEG_NODISCARD auto mut_ptr() VEG_NOEXCEPT -> T* {
		return static_cast<T*>(const_cast<void*>(Base::data));
	}
	VEG_NODISCARD auto ptr() const VEG_NOEXCEPT -> T const* {
		return static_cast<T const*>(const_cast<void const*>(Base::data));
	}
	VEG_NODISCARD auto len() const VEG_NOEXCEPT -> isize {
		return isize(Base::len);
	}

private:
	friend struct DynStackArray<T>;
	friend struct DynStackView;
	friend struct internal::dynstack::DynStackArrayDtor<T>;

	template <typename Fn>
	DynStackAlloc(DynStackView& parent_ref, usize alloc_size, usize align, Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T>))
			: Base{
						&parent_ref,
						parent_ref.stack_data,
						nullptr,
						0,
				} {

		void* const parent_data = parent_ref.stack_data;
		usize const parent_bytes = parent_ref.stack_bytes;

		void* const ptr = abi::internal::align_next(
				align,
				alloc_size * sizeof(T),
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
struct DynStackArray : private // destruction order matters
											 DynStackAlloc<T>,
											 internal::dynstack::DynStackArrayDtor<T> {
private:
	using Base = internal::dynstack::DynAllocBase;

public:
	using DynStackAlloc<T>::as_ref;
	using DynStackAlloc<T>::as_mut;
	using DynStackAlloc<T>::ptr;
	using DynStackAlloc<T>::mut_ptr;
	using DynStackAlloc<T>::len;

	~DynStackArray() = default;
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

private:
	using DynStackAlloc<T>::DynStackAlloc;
	friend struct DynStackView;
	friend struct internal::dynstack::DynStackArrayDtor<T>;
};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS   \
        */

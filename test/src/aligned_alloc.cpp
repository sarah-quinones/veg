#include <veg/memory/alloc.hpp>
#include <doctest.h>
#include <cstring>
#include "static_assert.hpp"

veg::usize aligns[] = {1, 2, 16, 64, 256};

TEST_CASE("alloc free") {
	for (auto align : aligns) {
		veg::usize cap = 0;
		void* alloc = veg::abi::mem::veglib_aligned_alloc(&cap, align, 13);
		REQUIRE(cap >= 13);
		veg::abi::mem::veglib_aligned_free(align, {alloc, cap});
	}
}

TEST_CASE("alloc zeroed free") {
	for (auto align : aligns) {
		veg::usize cap = 0;
		void* alloc = veg::abi::mem::veglib_aligned_alloc_zeroed(&cap, align, 13);
		REQUIRE(cap >= 13);
		for (veg::usize i = 0; i < cap; ++i) {
			CHECK(static_cast<veg::mem::byte*>(alloc)[i] == 0);
		}
		veg::abi::mem::veglib_aligned_free(align, {alloc, cap});
	}
}

TEST_CASE("grow") {
	for (auto align : aligns) {
		veg::usize cap = 0;
		void* alloc = veg::abi::mem::veglib_aligned_alloc(&cap, align, 13);
		REQUIRE(cap >= 13);
		char a[] = {
				'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '?', '!'};
		STATIC_ASSERT(sizeof(a) == 13);
		std::memcpy(alloc, &a, 13);

		veg::usize new_cap = 0;
		void* realloc = veg::abi::mem::veglib_aligned_realloc_zeroed(
				&new_cap, align, {alloc, cap}, cap + 1);
		REQUIRE(new_cap >= (cap + 1));

		CHECK(std::memcmp(a, realloc, 13) == 0);
		veg::abi::mem::veglib_aligned_free(align, {realloc, new_cap});
	}
}

TEST_CASE("grow zeroed") {
	for (auto align : aligns) {
		veg::usize cap = 0;
		void* alloc = veg::abi::mem::veglib_aligned_alloc(&cap, align, 13);
		REQUIRE(cap >= 13);
		char a[] = {
				'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '?', '!'};
		STATIC_ASSERT(sizeof(a) == 13);
		std::memcpy(alloc, &a, 13);

		veg::usize new_cap = 0;
		void* realloc = veg::abi::mem::veglib_aligned_realloc_zeroed(
				&new_cap, align, {alloc, cap}, cap + 1);
		REQUIRE(new_cap >= (cap + 1));

		CHECK(std::memcmp(a, realloc, 13) == 0);
		for (veg::usize i = cap; i < new_cap; ++i) {
			CHECK(static_cast<veg::mem::byte*>(realloc)[i] == 0);
		}

		veg::abi::mem::veglib_aligned_free(align, {realloc, new_cap});
	}
}

TEST_CASE("shrink") {
	for (auto align : aligns) {
		veg::usize cap = 0;
		void* alloc = veg::abi::mem::veglib_aligned_alloc(&cap, align, 13);
		REQUIRE(cap >= 13);
		char a[] = {
				'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '?', '!'};
		STATIC_ASSERT(sizeof(a) == 13);
		std::memcpy(alloc, &a, 13);

		veg::usize new_cap = 0;
		void* realloc = veg::abi::mem::veglib_aligned_realloc_zeroed(
				&new_cap, align, {alloc, cap}, 10);
		REQUIRE(new_cap >= 10);

		CHECK(std::memcmp(a, realloc, 10) == 0);
		veg::abi::mem::veglib_aligned_free(align, {realloc, new_cap});
	}
}

TEST_CASE("shrink zeroed") {
	for (auto align : aligns) {
		veg::usize cap = 0;
		void* alloc = veg::abi::mem::veglib_aligned_alloc(&cap, align, 13);
		REQUIRE(cap >= 13);
		char a[] = {
				'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '?', '!'};
		STATIC_ASSERT(sizeof(a) == 13);
		std::memcpy(alloc, &a, 13);

		veg::usize new_cap = 0;
		void* realloc = veg::abi::mem::veglib_aligned_realloc_zeroed(
				&new_cap, align, {alloc, cap}, 10);
		REQUIRE(new_cap >= 10);

		CHECK(std::memcmp(a, realloc, 10) == 0);
		for (veg::usize i = 10; i < new_cap; ++i) {
			CHECK(static_cast<veg::mem::byte*>(realloc)[i] == 0);
		}

		veg::abi::mem::veglib_aligned_free(align, {realloc, new_cap});
	}
}

#include <veg/vec.hpp>
#include <veg/memory/stack_alloc.hpp>
#include <vector>
#include <doctest.h>

using namespace veg;

TEST_CASE("empty vector") {
	Vec<int> v;
	REQUIRE(v.ptr() == nullptr);
	REQUIRE(v.len() == 0);
	REQUIRE(v.capacity() == 0);
}

TEST_CASE("push some values") {
	Vec<int> v;
	v.push(1);
	v.push(2);
	v.push(3);
	v.push(4);
	v.push(5);

	REQUIRE(v[0] == 1);
	REQUIRE(v[1] == 2);
	REQUIRE(v[2] == 3);
	REQUIRE(v[3] == 4);
	REQUIRE(v[4] == 5);
}

TEST_CASE("reserve and push some values") {
	Vec<int> v;
	v.reserve(16);
	v.push(1);
	v.push(2);
	v.push(3);
	v.push(4);
	v.push(5);

	REQUIRE(v[0] == 1);
	REQUIRE(v[1] == 2);
	REQUIRE(v[2] == 3);
	REQUIRE(v[3] == 4);
	REQUIRE(v[4] == 5);
}

TEST_CASE("reserve and push some values") {
	using A = mem::StackAlloc<alignof(int)>;
	alignas(int) Array<mem::byte, 4096> stack{};

	Vec<int, A> v{from_raw_parts, {}, A{from_slice, stack.as_mut()}};

	v.reserve(16);
	v.push(1);
	v.push(2);
	v.push(3);
	v.push(4);
	v.push(5);

	REQUIRE(v[0] == 1);
	REQUIRE(v[1] == 2);
	REQUIRE(v[2] == 3);
	REQUIRE(v[3] == 4);
	REQUIRE(v[4] == 5);
}

TEST_CASE("clone") {
	Vec<int> v1;
	v1.push(1);
	v1.push(2);
	v1.push(3);
	v1.push(4);
	v1.push(5);

	auto v2 = clone(ref(v1));

	for (auto p : {ref(v1), ref(v2)}) {
		REQUIRE(p->capacity() >= 5);
		REQUIRE(p->len() == 5);
		REQUIRE((*p)[0] == 1);
		REQUIRE((*p)[1] == 2);
		REQUIRE((*p)[2] == 3);
		REQUIRE((*p)[3] == 4);
		REQUIRE((*p)[4] == 5);
	}
}

TEST_CASE("stack alloc clone") {
	using A = mem::StackAlloc<alignof(int)>;
	alignas(int) Array<mem::byte, 4096> stack{};

	Vec<int, A> v1{from_raw_parts, {}, A{from_slice, stack.as_mut()}};

	v1.push(1);
	v1.push(2);
	v1.push(3);
	v1.push(4);
	v1.push(5);

	auto v2 = clone(ref(v1));

	for (auto p : {ref(v1), ref(v2)}) {
		REQUIRE(p->capacity() >= 5);
		REQUIRE(p->len() == 5);
		REQUIRE((*p)[0] == 1);
		REQUIRE((*p)[1] == 2);
		REQUIRE((*p)[2] == 3);
		REQUIRE((*p)[3] == 4);
		REQUIRE((*p)[4] == 5);
	}
}

TEST_CASE("move") {
	Vec<int> v1;
	v1.push(1);
	v1.push(2);
	v1.push(3);
	v1.push(4);
	v1.push(5);

	auto v2 = VEG_FWD(v1);
	REQUIRE(v1.ptr() == nullptr);
	REQUIRE(v1.len() == 0);
	REQUIRE(v2.len() == 5);

	REQUIRE(v2[0] == 1);
	REQUIRE(v2[1] == 2);
	REQUIRE(v2[2] == 3);
	REQUIRE(v2[3] == 4);
	REQUIRE(v2[4] == 5);
}

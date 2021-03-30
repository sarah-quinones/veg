#include "veg/memory/dynamic_stack.hpp"
#include "assert_death.hpp"
#include <doctest.h>

#include <atomic>

using namespace veg;

struct S {
private:
	static auto n_instances_mut() -> std::atomic<int>& {
		static std::atomic<int> var{0};
		return var;
	}

public:
	static auto n_instances() -> int { return n_instances_mut(); }

	S(S const&) = delete;
	S(S&&) = delete;
	auto operator=(S const&) -> S& = delete;
	auto operator=(S&&) -> S& = delete;
	S() { ++n_instances_mut(); }
	~S() { --n_instances_mut(); }
};

TEST_CASE("dynamic stack: raii") {
	unsigned char buf[4096];

	dynamic_stack_view stack{slice<void>(buf)};

	{
		auto s1 = stack.make_new(tag_t<S>{}, 3).unwrap();
		CHECK(s1.data() != nullptr);
		CHECK(s1.size() == 3);
		CHECK(stack.remaining_bytes() == 4093);
		CHECK(S::n_instances() == 3);

		{
			auto s2 = stack.make_new(tag_t<S>{}, 4).unwrap();
			CHECK(s2.data() != nullptr);
			CHECK(s2.size() == 4);
			CHECK(stack.remaining_bytes() == 4089);
			CHECK(S::n_instances() == 7);

			{
				auto i3 = stack.make_new(tag_t<int>{}, 30000);
				CHECK(!i3);
				CHECK(stack.remaining_bytes() == 4089);
				{
					auto i4 = stack.make_new(tag_t<int>{}, 300).unwrap();
					CHECK(i4.data() != nullptr);
					CHECK(i4.size() == 300);
					CHECK(stack.remaining_bytes() < 4089 - 300 * sizeof(int));
				}
			}
		}
		CHECK(stack.remaining_bytes() == 4093);
		CHECK(S::n_instances() == 3);
	}
	CHECK(stack.remaining_bytes() == 4096);
	CHECK(S::n_instances() == 0);

	auto s1 = stack.make_new(tag_t<S const>{}, 3).unwrap();
	CHECK(stack.remaining_bytes() == 4093);
	CHECK(S::n_instances() == 3);
}

TEST_CASE("dynamic stack: evil_reorder") {
	unsigned char buf[4096];
	dynamic_stack_view stack{make::slice(buf)};
	auto good = [&] {
		auto s1 = stack.make_new(tag_t<int>{}, 30);
		auto s2 = stack.make_new(tag_t<double>{}, 20);
		auto s3 = VEG_MOV(s2);
	};
	auto bad = [&] {
		auto s1 = stack.make_new(tag_t<int>{}, 30);
		auto s2 = stack.make_new(tag_t<double>{}, 20);
		auto s3 = VEG_MOV(s1);
	};
	CHECK_NO_DEATH({ good(); });
	CHECK_DEATH({ bad(); });
}

TEST_CASE("dynamic stack: assign") {
	alignas(double) unsigned char buf[100];
	dynamic_stack_view stack{make::slice(buf)};

	{
		auto s1 = stack.make_new(tag_t<char>{}, 30);
		auto s2 = stack.make_new(tag_t<char>{}, 20);
		auto s3 = VEG_MOV(s2);
		CHECK(stack.remaining_bytes() == 50);
		s3 = VEG_FWD(s1);
		CHECK(stack.remaining_bytes() == 70);
		s3 = VEG_FWD(s3);
    // either no-op or destroys
		CHECK((stack.remaining_bytes() == 70 || stack.remaining_bytes() == 100));
	}
	CHECK(stack.remaining_bytes() == 100);
}

TEST_CASE("dynamic stack: return") {
	unsigned char buf[4096];
	dynamic_stack_view stack(make::slice(buf));

	auto s = [&] {
		auto s1 = stack.make_new(tag_t<S>{}, 3).unwrap();
		auto s2 = stack.make_new(tag_t<S>{}, 4).unwrap();
		auto s3 = stack.make_new(tag_t<S>{}, 5).unwrap();
		CHECK(stack.remaining_bytes() == 4084);
		CHECK(S::n_instances() == 12);
		return s1;
	}();

	CHECK(stack.remaining_bytes() == 4093);
	CHECK(S::n_instances() == 3);

	CHECK(s.data() != nullptr);
	CHECK(s.size() == 3);
}

TEST_CASE("dynamic stack: manual_lifetimes") {
	unsigned char buf[4096];
	dynamic_stack_view stack(make::slice(buf));

	{
		auto s = stack.make_alloc(tag_t<S>{}, 3).unwrap();
		CHECK(s.data() != nullptr);
		CHECK(s.size() == 3);
		CHECK(S::n_instances() == 0);

		{
			new (s.data() + 0) S{};
			CHECK(S::n_instances() == 1);
			new (s.data() + 1) S{};
			CHECK(S::n_instances() == 2);
			new (s.data() + 2) S{};
			CHECK(S::n_instances() == 3);

			(s.data() + 2)->~S();
			CHECK(S::n_instances() == 2);
			(s.data() + 1)->~S();
			CHECK(S::n_instances() == 1);
			(s.data() + 0)->~S();
			CHECK(S::n_instances() == 0);
		}
		CHECK(S::n_instances() == 0);
	}
	CHECK(stack.remaining_bytes() == 4096);
}

struct T : S {
	T() = default;
	int a = 0;
};

TEST_CASE("dynamic stack: alignment") {
	alignas(T) unsigned char buffer[4096 + 1];
	dynamic_stack_view stack{{buffer + 1, 4096}};
	CHECK(stack.remaining_bytes() == 4096);
	CHECK(T::n_instances() == 0);
	{
		auto s1 = stack.make_new(tag_t<T>{}, 0);
		CHECK(stack.remaining_bytes() == 4096 - alignof(T) + 1);
		auto s2 = stack.make_new(tag_t<S>{}, 0);
		CHECK(stack.remaining_bytes() == 4096 - alignof(T) + 1);
		CHECK(T::n_instances() == 0);
	}
	CHECK(T::n_instances() == 0);
}

struct throwing {
private:
	static auto n_instances_mut() -> std::atomic<int>& {
		static std::atomic<int> var{0};
		return var;
	}

public:
	static auto n_instances() -> int { return n_instances_mut(); }

	throwing(throwing const&) = delete;
	throwing(throwing&&) = delete;
	auto operator=(throwing const&) -> throwing& = delete;
	auto operator=(throwing&&) -> throwing& = delete;
	throwing() {
		if (n_instances() == 5) {
			throw 0;
		}
		++n_instances_mut();
	}
	~throwing() { --n_instances_mut(); }
};

TEST_CASE("dynamic stack: throwing") {
	unsigned char buf[4096];
	dynamic_stack_view stack{slice<void>(buf)};

	CHECK(throwing::n_instances() == 0);
	auto s1 = stack.make_new(tag_t<throwing>{}, 3);
	(void)s1;

	CHECK(throwing::n_instances() == 3);
	CHECK(stack.remaining_bytes() == 4093);
	try {
		auto s2 = stack.make_new(tag_t<throwing>{}, 7);
		(void)s2;
		CHECK(false); // must not be reached
	} catch (int) {
		CHECK(throwing::n_instances() == 3);
		CHECK(stack.remaining_bytes() == 4093);
	}
}

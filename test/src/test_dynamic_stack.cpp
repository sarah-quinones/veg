#define __VEG_DISABLE_NOEXCEPT

#include "veg/memory/dynamic_stack.hpp"
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

	DynStackView stack{Slice<void>(as_ref, buf)};

	{
		auto s1 = stack.make_new(Tag<S>{}, 3).unwrap();
		CHECK(s1.as_ptr() != nullptr);
		CHECK(s1.len() == 3);
		CHECK(stack.remaining_bytes() == 4093);
		CHECK(S::n_instances() == 3);

		{
			auto s2 = stack.make_new(Tag<S>{}, 4).unwrap();
			CHECK(s2.as_ptr() != nullptr);
			CHECK(s2.len() == 4);
			CHECK(stack.remaining_bytes() == 4089);
			CHECK(S::n_instances() == 7);

			{
				auto i3 = stack.make_new(Tag<int>{}, 30000);
				CHECK(i3.is_none());
				CHECK(stack.remaining_bytes() == 4089);
				{
					auto i4 = stack.make_new(Tag<int>{}, 300).unwrap();
					CHECK(i4.as_ptr() != nullptr);
					CHECK(i4.len() == 300);
					CHECK(stack.remaining_bytes() < 4089 - 300 * sizeof(int));
				}
			}
		}
		CHECK(stack.remaining_bytes() == 4093);
		CHECK(S::n_instances() == 3);
	}
	CHECK(stack.remaining_bytes() == 4096);
	CHECK(S::n_instances() == 0);

	auto s1 = stack.make_new(Tag<S const>{}, 3).unwrap();
	CHECK(stack.remaining_bytes() == 4093);
	CHECK(S::n_instances() == 3);
}

TEST_CASE("dynamic stack: evil_reorder") {
	unsigned char buf[4096];
	DynStackView stack{slice::from_array(buf)};
	auto good = [&] {
		auto s1 = stack.make_new(Tag<int>{}, 30).unwrap();
		auto s2 = stack.make_new(Tag<double>{}, 20).unwrap();
		auto s3 = VEG_MOV(s2);
	};
	auto bad = [&] {
		auto s1 = stack.make_new(Tag<int>{}, 30).unwrap();
		auto s2 = stack.make_new(Tag<double>{}, 20).unwrap();
		auto s3 = VEG_MOV(s1);
	};
	CHECK_NOTHROW(good());
	CHECK_THROWS(bad());
}

TEST_CASE("dynamic stack: assign") {
	alignas(double) unsigned char buf[100];
	DynStackView stack{slice::from_array(buf)};

	{
		auto s1 = stack.make_new(Tag<char>{}, 30);
		auto s2 = stack.make_new(Tag<char>{}, 20);
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
	DynStackView stack(slice::from_array(buf));

	auto s = [&] {
		auto s1 = stack.make_new(Tag<S>{}, 3).unwrap();
		auto s2 = stack.make_new(Tag<S>{}, 4).unwrap();
		auto s3 = stack.make_new(Tag<S>{}, 5).unwrap();
		CHECK(stack.remaining_bytes() == 4084);
		CHECK(S::n_instances() == 12);
		return s1;
	}();

	CHECK(stack.remaining_bytes() == 4093);
	CHECK(S::n_instances() == 3);

	CHECK(s.as_ptr() != nullptr);
	CHECK(s.len() == 3);
}

TEST_CASE("dynamic stack: manual_lifetimes") {
	unsigned char buf[4096];
	DynStackView stack({as_ref, buf});

	{
		auto s = stack.make_alloc(Tag<S>{}, 3).unwrap();
		CHECK(s.as_ptr() != nullptr);
		CHECK(s.len() == 3);
		CHECK(S::n_instances() == 0);

		{
			new (s.as_mut_ptr() + 0) S{};
			CHECK(S::n_instances() == 1);
			new (s.as_mut_ptr() + 1) S{};
			CHECK(S::n_instances() == 2);
			new (s.as_mut_ptr() + 2) S{};
			CHECK(S::n_instances() == 3);

			(s.as_ptr() + 2)->~S();
			CHECK(S::n_instances() == 2);
			(s.as_ptr() + 1)->~S();
			CHECK(S::n_instances() == 1);
			(s.as_ptr() + 0)->~S();
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
	DynStackView stack{{from_raw_parts, buffer + 1, 4096, unsafe}};
	CHECK(stack.remaining_bytes() == 4096);
	CHECK(T::n_instances() == 0);
	{
		auto s1 = stack.make_new(Tag<T>{}, 0);
		CHECK(stack.remaining_bytes() == 4096 - alignof(T) + 1);
		auto s2 = stack.make_new(Tag<S>{}, 0);
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
	DynStackView stack{Slice<void>{as_ref, buf}};

	CHECK(throwing::n_instances() == 0);
	auto s1 = stack.make_new(Tag<throwing>{}, 3);
	(void)s1;

	CHECK(throwing::n_instances() == 3);
	CHECK(stack.remaining_bytes() == 4093);
	try {
		auto s2 = stack.make_new(Tag<throwing>{}, 7);
		(void)s2;
		CHECK(false); // must not be reached
	} catch (int) {
		CHECK(throwing::n_instances() == 3);
		CHECK(stack.remaining_bytes() == 4093);
	}
}

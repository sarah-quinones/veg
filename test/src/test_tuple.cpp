#include "static_assert.hpp"
#include <fmt/core.h>
#include <limits>
#include <veg/tuple.hpp>
#include <utility>
#include <doctest.h>
#include "veg/memory/placement.hpp"
#include "veg/functional/utils.hpp"
#include "veg/functional/copy_fn.hpp"
#include "veg/internal/prologue.hpp"

#define FWD VEG_FWD
#define ASSERT_SAME(...) STATIC_ASSERT(::std::is_same<__VA_ARGS__>::value)

using namespace veg::literals;
using namespace veg::tags;

template <typename>
void get() = delete;

TEST_CASE("tuple: adl_get") {
	veg::Tuple<int, float> t{};
	veg::unused(get<0>(t));
	STATIC_ASSERT(VEG_CONCEPT(same<decltype(get<0>(t)), int&>));
	STATIC_ASSERT(VEG_CONCEPT(
			same<
					decltype(get<0>(static_cast<veg::Tuple<int, float> const&>(t))),
					int const&>));
	STATIC_ASSERT(VEG_CONCEPT(
			same<decltype(get<0>(static_cast<veg::Tuple<int, float>&&>(t))), int&&>));
}

TEST_CASE("tuple: all") {
	using namespace veg;

	auto tup = tuple::make(1, 'c', true);
	veg::Tuple<int, char&&, char, bool&, bool const&> tup_ref{
			direct,
			1,
			FWD(tup).as_ref()[1_c],
			'c',
			nb::get<2>{}(tup),
			nb::get<2>{}(tup)};
	{
		STATIC_ASSERT(veg::cpo::is_trivially_relocatable<decltype(tup)>::value);
		STATIC_ASSERT(VEG_CONCEPT(trivially_copyable<decltype(tup)>));

		STATIC_ASSERT(!VEG_CONCEPT(copy_assignable<veg::Tuple<int&, int const&>>));
		STATIC_ASSERT(!VEG_CONCEPT(move_assignable<veg::Tuple<int&, int const&>>));

		STATIC_ASSERT(!VEG_CONCEPT(copy_assignable<veg::Tuple<int&, float&>>));
		STATIC_ASSERT(VEG_CONCEPT(assignable<
															veg::Tuple<int&, float&> const&&,
															veg::Tuple<int&, float&>&&>));
		STATIC_ASSERT(VEG_CONCEPT(assignable<
															veg::Tuple<int&, float&> const&&,
															veg::Tuple<int&, float&>&&>));

		STATIC_ASSERT(!VEG_CONCEPT(copy_assignable<decltype(tup_ref)>));
		STATIC_ASSERT(!VEG_CONCEPT(move_assignable<decltype(tup_ref)>));
		STATIC_ASSERT(!VEG_CONCEPT(copy_constructible<decltype(tup_ref)>));
		STATIC_ASSERT(VEG_CONCEPT(trivially_move_constructible<decltype(tup_ref)>));
		STATIC_ASSERT(!std::is_copy_constructible<decltype(tup_ref)>::value);
		using val_tup = veg::Tuple<int, bool>;
		using ref_tup = veg::Tuple<int&, bool&>;

		STATIC_ASSERT(VEG_CONCEPT(swappable<ref_tup const&&, ref_tup const&&>));
		STATIC_ASSERT(VEG_CONCEPT(swappable<ref_tup&&, ref_tup const&&>));
		STATIC_ASSERT(VEG_CONCEPT(swappable<ref_tup const&&, ref_tup&&>));
		STATIC_ASSERT(VEG_CONCEPT(swappable<ref_tup const&&, ref_tup const&&>));
		STATIC_ASSERT(VEG_CONCEPT(swappable<ref_tup&&, ref_tup&&>));

		STATIC_ASSERT(VEG_CONCEPT(nothrow_swappable<ref_tup&&, ref_tup&&>));

		STATIC_ASSERT(VEG_CONCEPT(swappable<ref_tup const&&, val_tup&>));
		STATIC_ASSERT(VEG_CONCEPT(swappable<val_tup&, val_tup&>));
		STATIC_ASSERT(!VEG_CONCEPT(swappable<val_tup&&, val_tup&&>));
		STATIC_ASSERT(!VEG_CONCEPT(swappable<val_tup&, val_tup&&>));

		STATIC_ASSERT(VEG_CONCEPT(nothrow_swappable<ref_tup&&, ref_tup&&>));
		STATIC_ASSERT(VEG_CONCEPT(nothrow_swappable<ref_tup&&, ref_tup const&&>));
		STATIC_ASSERT(VEG_CONCEPT(nothrow_swappable<ref_tup const&&, ref_tup&&>));
		STATIC_ASSERT(
				VEG_CONCEPT(nothrow_swappable<ref_tup const&&, ref_tup const&&>));
		STATIC_ASSERT(VEG_CONCEPT(nothrow_swappable<ref_tup&&, ref_tup&&>));
		STATIC_ASSERT(VEG_CONCEPT(nothrow_swappable<ref_tup&&, ref_tup&&>));

		STATIC_ASSERT(VEG_CONCEPT(nothrow_swappable<ref_tup const&&, val_tup&>));
		STATIC_ASSERT(VEG_CONCEPT(nothrow_swappable<val_tup&, val_tup&>));
		STATIC_ASSERT(!VEG_CONCEPT(nothrow_swappable<val_tup&&, val_tup&&>));
		STATIC_ASSERT(!VEG_CONCEPT(nothrow_swappable<val_tup&, val_tup&&>));
	}
	{
		using val_tup = veg::Tuple<int, bool>;
		val_tup a = tuple::make(5, true);
		val_tup b = tuple::make(3, false);

		unused(Tuple<long, bool>{
				tuple::map_i(
						val_tup{},
						fn::copy_fn(overload(
								nb::indexed<0>{}(nb::convert<long>{}),
								nb::indexed<1>{}(nb::convert<bool>{})))),
		});

		veg::swap(a, b);
		CHECK(a[0_c] == 3);
		CHECK(b[0_c] == 5);
		CHECK(a[1_c] == false);
		CHECK(b[1_c] == true);
		STATIC_ASSERT(val_tup{direct, 1, true} == val_tup{direct, 1, true});
		STATIC_ASSERT(
				noexcept(val_tup{direct, 1, true} == val_tup{direct, 1, true}));
		STATIC_ASSERT(
				cmp::three_way(val_tup{direct, 1, true}, val_tup{direct, 1, true}) ==
				0);

		CHECK(a == (val_tup{direct, 3, false}));
		CHECK(b == (val_tup{direct, 5, true}));
	}
	{
		using ref_tup = veg::Tuple<int&>;
		using val_tup = veg::Tuple<int>;
		int i = 13;
		val_tup j{direct, 12};
		ref_tup a{direct, i};
		ref_tup const b = j.as_ref();
		veg::swap(FWD(a), FWD(b));

		CHECK(i == 12);
		CHECK(j[0_c] == 13);

		FWD(a) = b.as_ref();
		CHECK(i == 13);
		CHECK(j[0_c] == 13);

		STATIC_ASSERT(VEG_CONCEPT(assignable<ref_tup const&&, ref_tup&&>));
	}

	{
		using ref_tup = veg::Tuple<int&>;
		using rref_tup = veg::Tuple<int&&>;
		STATIC_ASSERT(VEG_CONCEPT(trivially_copyable<ref_tup>));
		STATIC_ASSERT(VEG_CONCEPT(trivially_copyable<rref_tup>));
		STATIC_ASSERT(VEG_CONCEPT(constructible<ref_tup, ref_tup>));
		STATIC_ASSERT(!VEG_CONCEPT(constructible<ref_tup, rref_tup&&>));

		int i = 13;
		int j = 12;
		ref_tup a{direct, i};
		rref_tup b{direct, FWD(j)};

		veg::swap(a.as_ref(), b.as_ref());

		CHECK(i == 12);
		CHECK(j == 13);

		FWD(a) = FWD(b);
		static_cast<ref_tup const&&>(a) = b.as_ref();
		static_cast<ref_tup const&&>(a) = FWD(b);
		CHECK(i == 13);
		CHECK(j == 13);

		STATIC_ASSERT(VEG_CONCEPT(assignable<ref_tup const&&, ref_tup&&>));
	}

	CHECK(tup[0_c] == 1);
	CHECK(tup[1_c] == 'c');
	CHECK(tup[2_c]);

	{
		auto&& ref1 = FWD(tup)[2_c];
		auto&& ref2 = FWD(tup).as_ref()[2_c];
		CHECK(&ref1 != &tup[2_c]);
		CHECK(&ref2 == &tup[2_c]);
	}

	using veg::clone;

	VEG_BIND(auto, (e, f, g), [&] { return clone(tup); }());
#if __cplusplus >= 201703L
	auto [i, c, b] = [&] { return clone(tup); }();
	CHECK(i == 1);
	CHECK(c == 'c');
	CHECK(b);
	veg::Tuple tup_deduce{direct, 1, 'c', true};
#endif

	STATIC_ASSERT(std::is_copy_assignable<veg::Tuple<int, char>>());
	STATIC_ASSERT(std::is_trivially_copyable<veg::Tuple<int, char>>());
	STATIC_ASSERT(!std::is_copy_assignable<veg::Tuple<int&, char&>>());
	STATIC_ASSERT(!std::is_copy_assignable<veg::Tuple<int&>>());
	ASSERT_SAME(decltype(tup[0_c]), int&);
	ASSERT_SAME(decltype(tup[0_c]), decltype(tup[0_c]));
	ASSERT_SAME(decltype(FWD(tup)[0_c]), decltype(FWD(tup)[0_c]));
	ASSERT_SAME(decltype(FWD(tup)[0_c]), int);
	ASSERT_SAME(decltype(FWD(tup).as_ref()[0_c]), int&&);

	ASSERT_SAME(decltype(tup.as_ref()), veg::Tuple<int&, char&, bool&>);
	ASSERT_SAME(decltype(FWD(tup).as_ref()), veg::Tuple<int&&, char&&, bool&&>);

	ASSERT_SAME(decltype(e), int&&);
	ASSERT_SAME(decltype((e)), int&);
	ASSERT_SAME(decltype(f), char&&);
	ASSERT_SAME(decltype((f)), char&);
	ASSERT_SAME(decltype((g)), bool&);

	ASSERT_SAME(decltype(tup_ref[0_c]), int&);
	ASSERT_SAME(decltype(FWD(tup_ref).as_ref()[0_c]), int&&);

	ASSERT_SAME(decltype(tup_ref[1_c]), char&);
	ASSERT_SAME(decltype(tup_ref[2_c]), char&);
	ASSERT_SAME(decltype(FWD(tup_ref)[1_c]), char&&);
	ASSERT_SAME(decltype(FWD(tup_ref)[2_c]), char);
	ASSERT_SAME(decltype(FWD(tup_ref).as_ref()[2_c]), char&&);

	ASSERT_SAME(decltype(tup_ref[3_c]), bool&);
	ASSERT_SAME(decltype(tup_ref[4_c]), bool const&);
	ASSERT_SAME(decltype(FWD(tup_ref)[3_c]), bool&);
	ASSERT_SAME(decltype(FWD(tup_ref)[4_c]), bool const&);

#if __cplusplus >= 201703L
	ASSERT_SAME(decltype(i), int);
	ASSERT_SAME(decltype((i)), int&);
	ASSERT_SAME(decltype((b)), bool&);
	ASSERT_SAME(decltype(tup_deduce), veg::Tuple<int, char, bool>);
#endif

	STATIC_ASSERT(Tuple<int>{direct, 1} == Tuple<int>{direct, 1});
	STATIC_ASSERT(Tuple<int>{direct, 1} == Tuple<double>{direct, 1});
	STATIC_ASSERT(Tuple<int>{direct, 1} != Tuple<double>{direct, 2});
	STATIC_ASSERT(
			Tuple<int, double>{direct, 1, 2.0} == Tuple<double, int>{direct, 1.0, 2});
	STATIC_ASSERT(
			Tuple<int, double>{direct, 1, 2.0} == Tuple<double, int>{direct, 1.0, 2});
	STATIC_ASSERT(
			Tuple<int, double>{direct, 1, 2.0} != Tuple<double, int>{direct, 2.0, 2});
}

TEST_CASE("tuple: nested") {
	using namespace veg;
	Tuple<int, Tuple<int, float>> tup{direct, 1, {direct, 2, 3.0F}};
	CHECK(tup[0_c] == 1);

	CHECK(tup[1_c][1_c] == 3.0F);
	CHECK(tup[1_c][0_c] == 2);
	ASSERT_SAME(decltype(tup[1_c][0_c]), int&);
	ASSERT_SAME(decltype(tup[1_c][0_c]), int&);
	ASSERT_SAME(decltype(FWD(tup)[1_c][0_c]), int);

	STATIC_ASSERT(sizeof(Tuple<int>) == sizeof(int));
	STATIC_ASSERT(sizeof(Tuple<Tuple<int>>) == sizeof(int));
	STATIC_ASSERT(sizeof(Tuple<Tuple<Tuple<int>>>) == sizeof(int));
}

TEST_CASE("tuple: cmp") {
	using namespace veg;
	using veg::cmp::three_way;
	using tup_i = Tuple<int, int, int>;
	using tup_d = Tuple<int, int, double>;
	constexpr double nan = std::numeric_limits<double>::quiet_NaN();

	STATIC_ASSERT(VEG_CONCEPT(
			same<decltype(three_way(tup_d{}, tup_d{})), cmp::partial_ordering>));
	STATIC_ASSERT(VEG_CONCEPT(
			same<decltype(cmp_3way(tup_i{}, tup_d{})), cmp::partial_ordering>));
	STATIC_ASSERT(VEG_CONCEPT(
			same<decltype(cmp_3way(tup_d{}, tup_i{})), cmp::partial_ordering>));
	STATIC_ASSERT(VEG_CONCEPT(
			same<decltype(cmp_3way(tup_i{}, tup_i{})), cmp::strong_ordering>));

	{
		constexpr auto t1 = tuple::make(1, 2, 3);
		constexpr auto t2 = tuple::make(1, 2, 3);
		constexpr auto t3 = tuple::make(0, 2, 3);
		constexpr auto t4 = tuple::make(1, 2, 4);

		STATIC_ASSERT(three_way(t1, t2) == 0);
		STATIC_ASSERT(three_way(t1, t3) > 0);
		STATIC_ASSERT(three_way(t1, t4) < 0);
		STATIC_ASSERT(three_way(t2, t3) > 0);
		STATIC_ASSERT(three_way(t2, t4) < 0);
		STATIC_ASSERT(three_way(t3, t4) < 0);
	}
	CHECK(
			three_way(tuple::make(1, 1, nan), tuple::make(1, 1, 0)) ==
			cmp::partial_ordering::unordered);

	CHECK(
			three_way(tuple::make(0, 1, nan), tuple::make(1, 1, 0)) ==
			cmp::partial_ordering::less);

	CHECK(
			three_way(tuple::make(1, 2, nan), tuple::make(1, 1, 0)) ==
			cmp::partial_ordering::greater);
}

TEST_CASE("tuple: empty") {
	using namespace veg;
	Tuple<> t1{};
	Tuple<> t2;
	CHECK(t1 == t2);
	STATIC_ASSERT(sizeof(t1) == 1);
	STATIC_ASSERT(Tuple<>{} == Tuple<>{});
}

TEST_CASE("tuple: direct") {
	using namespace veg;
	Tuple<int, double> t1{direct, 1, 1.5};
	Tuple<long, double> t2(direct, 3, 2.5);
	Tuple<long, double> t3 = tuple::map_i(
			FWD(t1),
			fn::copy_fn(overload(
					nb::indexed<0>{}(nb::convert<long>{}),
					nb::indexed<1>{}(nb::convert<double>{}))));
	Tuple<int, double> t4 = tuple::map_i(
			FWD(t3),
			fn::copy_fn(overload(
					nb::indexed<0>{}(nb::convert<int>{}),
					nb::indexed<1>{}(nb::convert<double>{}))));

	STATIC_ASSERT(sizeof(t2) == sizeof(long) + sizeof(double));

	CHECK(t1 == t4);
	t2 = t1.as_ref();
	CHECK(t1 == t2);
}

TEST_CASE("tuple: get") {
	using namespace veg;
	i64 arr[] = {1, 2, 3};
	nb::get<0>{}(arr);
	nb::get<0>{}(FWD(arr));
}

TEST_CASE("tuple: *sparkles* functional programming *sparkles*") {
	using namespace veg;
	using veg::clone;

	{
		constexpr auto x = tuple::make(i64(1), i64(2), i64(3));
		CHECK(tuple::map(x.as_ref(), [](i64 i) -> float {
						return 2 * float(i);
					}) == tuple::make(2.0F, 4.0F, 6.0F));
		i64 acc = 0;
		tuple::for_each(x.as_ref(), [&](i64 i) -> void { acc += i; });
		CHECK(acc == 1 + 2 + 3);
	}

	{
		constexpr auto x = tuple::make(2.5, 3, tuple::make(&tuple::make, 10.0));
		tuple::unpack(
				clone(x),
				[](double d, int i, Tuple<decltype(&tuple::make), double> tup) {
					CHECK(d == 2.5);
					CHECK(i == i);
					CHECK(tup == tuple::make(&tuple::make, 10.0));
					CHECK(tup[0_c] == &tuple::make);
					CHECK(tup[1_c] == 10.0);
				});
	}
	{
		unused(Tuple<Tuple<int>>{
				tuple::map(tuple::make(tuple::make(2.0)), [](Tuple<double> inner) {
					return tuple::map(FWD(inner), fn::copy_fn(nb::convert<int>{}));
				})});
	}

	{
		STATIC_ASSERT(
				tuple::cat(tuple::make(1.0, 2), tuple::make(1, 4.0)) ==
				tuple::make(1.0, 2, 1, 4.0));
	}

	// sfinae test
	STATIC_ASSERT(!VEG_CONCEPT(invocable<struct tuple::nb::zip, int>));
	STATIC_ASSERT(!VEG_CONCEPT(
			invocable<struct tuple::nb::zip, Tuple<int>, Tuple<float, bool>>));
	{
		STATIC_ASSERT(tuple::ref(1.0, 2) == tuple::make(1.0, 2));
		STATIC_ASSERT(
				tuple::zip(tuple::ref(1.0, 2)) ==
				tuple::make(tuple::make(1.0), tuple::make(2)));
		STATIC_ASSERT(
				tuple::zip(tuple::ref(1.0), tuple::make(2)) ==
				tuple::make(tuple::make(1.0, 2)));

		constexpr void* p = nullptr;
		STATIC_ASSERT(
				tuple::zip(
						tuple::fwd(nullptr, 'c'),
						tuple::ref(1.0, 2),
						tuple::make(13, 4.0)) ==
				tuple::make(tuple::make(p, 1.0, 13), tuple::make('c', 2, 4.0)));
	}

	{
		STATIC_ASSERT(
				tuple::zip(tuple::fwd(), tuple::ref(), tuple::make()) == tuple::make());
	}

#if __cplusplus >= 201402L
	{
		constexpr auto x = tuple::make(i64(1), float(2.5), i64(3));
		float acc = 0;
		tuple::for_each_i(
				x.as_ref(), [&](auto N, auto i) -> void { acc += float(i) * i64{N}; });
		CHECK(acc == 1 * 0 + 2.5F * 1 + 3 * 2);
	}
#endif
}
#include "veg/internal/epilogue.hpp"

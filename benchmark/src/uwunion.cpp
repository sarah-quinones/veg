#include <array>
#include <cstdlib>
#include <veg/uwunion.hpp>
#include <variant>
#include <benchmark/benchmark.h>

using namespace veg;

template <usize I>
struct S {
	static constexpr usize v = I;
};

template <usize... Is>
static void
bench_std_visit_impl(std::index_sequence<Is...> /*tag*/, benchmark::State& s) {
	using V = std::variant<S<Is>...>;
	using F = V (*)();
	constexpr F fn[] = {[] { return V(S<Is>{}); }...};

	usize v = 0;
	benchmark::DoNotOptimize(&v);
	usize i = 0;
	for (auto _ : s) {
		auto idx = i % sizeof...(Is);
		++i;
		v += std::visit([&](auto I) { return decltype(I)::v; }, fn[idx]());
		benchmark::ClobberMemory();
	}
}
template <usize... Is>
static void
bench_veg_visit_impl(std::index_sequence<Is...> /*tag*/, benchmark::State& s) {
	using V = Uwunion<S<Is>...>;
	using F = V (*)();
	constexpr F fn[] = {[] { return V(Fix<Is>{}, {}); }...};

	usize v = 0;
	benchmark::DoNotOptimize(&v);
	usize i = 0;
	for (auto _ : s) {
		auto idx = i % sizeof...(Is);
		++i;
		v += fn[idx]().visit([&](auto I) { return decltype(I)::v; });
		benchmark::ClobberMemory();
	}
}

template <usize N>
static void bench_veg_visit(benchmark::State& s) {
	::bench_veg_visit_impl(std::make_index_sequence<N>{}, s);
}
template <usize N>
static void bench_std_visit(benchmark::State& s) {
	::bench_std_visit_impl(std::make_index_sequence<N>{}, s);
}

BENCHMARK_TEMPLATE(bench_std_visit, 32);
BENCHMARK_TEMPLATE(bench_std_visit, 64);
BENCHMARK_TEMPLATE(bench_std_visit, 128);
BENCHMARK_TEMPLATE(bench_veg_visit, 32);
BENCHMARK_TEMPLATE(bench_veg_visit, 64);
BENCHMARK_TEMPLATE(bench_veg_visit, 128);

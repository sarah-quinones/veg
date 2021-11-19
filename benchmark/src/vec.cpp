#include <benchmark/benchmark.h>
#include <veg/vec.hpp>
#include <vector>
#include <iostream>

using namespace veg;

template <typename T>
static void bench_std_vec(benchmark::State& s) {
	auto n = usize(s.range(0));
	for (auto _ : s) {
		[=]() __attribute__((noinline)) {
			std::vector<T> v;
			v.reserve(n);
			benchmark::DoNotOptimize(&v);
			for (usize i = 0; i < n; ++i) {
				v.push_back(T(i));
			}
			benchmark::ClobberMemory();
		}
		();
	}
}

template <typename T>
static void bench_veg_vec(benchmark::State& s) {
	auto n = isize(s.range(0));
	for (auto _ : s) {
		[=]() __attribute__((noinline)) {
			Vec<T> v;
			v.reserve(n);
			benchmark::DoNotOptimize(&v);
			for (isize i = 0; i < n; ++i) {
				v.push(T(i));
			}
			benchmark::ClobberMemory();
		}
		();
	}
}

template <typename T>
static void bench_std_vec_unchecked(benchmark::State& s) {
	auto n = usize(s.range(0));
	for (auto _ : s) {
		[=]() __attribute__((noinline)) {
			std::vector<T> v;
			benchmark::DoNotOptimize(&v);
			v.reserve(n);
			for (usize i = 0; i < n; ++i) {
				auto tmp = T(i);
				v.push_back(tmp);
			}
			benchmark::ClobberMemory();
		}
		();
	}
}

template <typename T>
static void bench_veg_vec_unchecked(benchmark::State& s) {
	auto n = isize(s.range(0));
	Vec<T> v_in;
	for (isize i = 0; i < n; ++i) {
		v_in.push(T(i));
	}

	for (auto _ : s) {
		[n, v_in = v_in.ptr()]() __attribute__((noinline)) {
			Vec<T> v;
			benchmark::DoNotOptimize(&v);
			v.reserve(n);
			for (isize i = 0; i < n; ++i) {
				v.push_unchecked(unsafe, T(v_in[i]));
			}
			benchmark::ClobberMemory();
		}
		();
	}
}

template <typename T>
static void bench_veg_vec_rsfo(benchmark::State& s) {
	auto n = isize(s.range(0));
	Vec<T> v_in;
	for (isize i = 0; i < n; ++i) {
		v_in.push(T(i));
	}

	for (auto _ : s) {
		[n, v_in = v_in.ptr()]() __attribute__((noinline)) {
			Vec<T> v;
			benchmark::DoNotOptimize(&v);
			v.reserve(n);
			benchmark::DoNotOptimize(v.ptr_mut());

      T* v_out = v.ptr_mut();
			for (isize i = 0; i < n; ++i) {
				v_out[i] = T(v_in[i]);
			}
			v.raw_mut(unsafe)->end += n;
			benchmark::ClobberMemory();
		}
		();
	}
}

static void make_args(benchmark::internal::Benchmark* b) {
	isize ns[] = {4096};
	for (auto n : ns) {
		b->Arg(n);
	}
}

// BENCHMARK_TEMPLATE(bench_veg_vec_unchecked, int)->Apply(make_args);
// BENCHMARK_TEMPLATE(bench_veg_vec_rsfo, int)->Apply(make_args);
// BENCHMARK_TEMPLATE(bench_veg_vec, int)->Apply(make_args);
// BENCHMARK_TEMPLATE(bench_std_vec, int)->Apply(make_args);
BENCHMARK_TEMPLATE(bench_veg_vec_unchecked, float)->Apply(make_args);
BENCHMARK_TEMPLATE(bench_veg_vec_rsfo, float)->Apply(make_args);
BENCHMARK_TEMPLATE(bench_veg_vec, float)->Apply(make_args);
BENCHMARK_TEMPLATE(bench_std_vec, float)->Apply(make_args);
BENCHMARK_MAIN();


/*
 * Copyright © 2019 Paweł Dziepak
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "pleione/intrusive/list.hpp"

#include "../data_set.hpp"

namespace perf {

using namespace data_set;

struct object {
  pleione::intrusive::list_hook hook_;
  int value_ = 0;
};

template<template<typename> typename T> void for_each(benchmark::State& s) {
  auto [objects, pointers] = T<object>{}(size_t(s.range(0)));
  (void)objects;
  auto list = pleione::intrusive::list<object, &object::hook_>();
  for (auto p : pointers) { list.push_back(*p); }

  static constexpr bool enable_prefetch = std::is_same_v<T<object>, perf::data_set::random<object>>;

  uint64_t iterations = 0;
  for (auto _ : s) {
    benchmark::ClobberMemory();
    for_each(pleione::prefetch<enable_prefetch>{}, list.begin(), list.end(),
             [](object& obj) { benchmark::DoNotOptimize(obj.value_); });
    ++iterations;
  }
  s.counters["ops"] = benchmark::Counter(double(iterations) * pointers.size(), benchmark::Counter::kIsRate);
}

PLEIONE_DATA_SET_PERF_TEST(for_each);

template<template<typename> typename T> void std_for_each_rev(benchmark::State& s) {
  auto [objects, pointers] = T<object>{}(size_t(s.range(0)));
  (void)objects;
  auto list = pleione::intrusive::list<object, &object::hook_>();
  for (auto p : pointers) { list.push_back(*p); }

  uint64_t iterations = 0;
  for (auto _ : s) {
    benchmark::ClobberMemory();
    std::for_each(list.rbegin(), list.rend(), [](object& obj) { benchmark::DoNotOptimize(obj.value_); });
    ++iterations;
  }
  s.counters["ops"] = benchmark::Counter(double(iterations) * pointers.size(), benchmark::Counter::kIsRate);
}

PLEIONE_DATA_SET_PERF_TEST(std_for_each_rev);

template<template<typename> typename T> void std_any_of(benchmark::State& s) {
  auto [objects, pointers] = T<object>{}(size_t(s.range(0)));
  (void)objects;
  auto list = pleione::intrusive::list<object, &object::hook_>();
  for (auto p : pointers) { list.push_back(*p); }

  uint64_t iterations = 0;
  for (auto _ : s) {
    benchmark::ClobberMemory();
    auto ret = std::any_of(list.begin(), list.end(), [](object const& obj) { return obj.value_ == 1; });
    benchmark::DoNotOptimize(ret);
    ++iterations;
  }
  s.counters["ops"] = benchmark::Counter(double(iterations) * pointers.size(), benchmark::Counter::kIsRate);
}

PLEIONE_DATA_SET_PERF_TEST(std_any_of);

} // namespace perf

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

#ifndef PLEIONE_PERF_DATA_SET_HPP
#define PLEIONE_PERF_DATA_SET_HPP

#include <algorithm>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>

namespace perf::data_set {

template<typename T> struct sequential {
  std::tuple<std::vector<T>, std::vector<T*>> operator()(size_t n) const {
    auto objects = std::vector<T>(n);
    auto pointers = std::vector<T*>(n);
    std::transform(objects.begin(), objects.end(), pointers.begin(), [](T& obj) { return &obj; });
    return {std::move(objects), std::move(pointers)};
  }
};

template<typename T> class random {
  sequential<T> sequential_;

public:
  std::tuple<std::vector<T>, std::vector<T*>> operator()(size_t n) const {
    auto [objects, pointers] = sequential_(n);
    auto eng = std::default_random_engine(std::random_device{}());
    std::shuffle(pointers.begin(), pointers.end(), eng);
    return {std::move(objects), std::move(pointers)};
  }
};

template<typename T> class reversed {
  sequential<T> sequential_;

public:
  std::tuple<std::vector<T>, std::vector<T*>> operator()(size_t n) const {
    auto [objects, pointers] = sequential_(n);
    std::reverse(pointers.begin(), pointers.end());
    return {std::move(objects), std::move(pointers)};
  }
};

} // namespace perf::data_set

#define PLEIONE_DATA_SET_PERF_TEST(function)                                                                           \
  BENCHMARK_TEMPLATE(function, sequential)->RangeMultiplier(1000)->Range(10, 1'000'000);                               \
  BENCHMARK_TEMPLATE(function, reversed)->RangeMultiplier(1000)->Range(10, 1'000'000);                                 \
  BENCHMARK_TEMPLATE(function, random)->RangeMultiplier(1000)->Range(10, 1'000'000)

#endif

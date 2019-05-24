/*
 * Copyright © 2018 Paweł Dziepak
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

#ifndef PLEIONE_DETAIL_CONFIG_HPP
#define PLEIONE_DETAIL_CONFIG_HPP

#include <cstdio>
#include <cstdlib>

#if defined(__clang__) || defined(__GNUC__)
#define PLEIONE_NAMESPACE_BEGIN                                                                                        \
  namespace pleione {                                                                                                  \
  inline namespace v0 __attribute__((abi_tag("pleione_v0"))) {
#else
#define PLEIONE_NAMESPACE_BEGIN                                                                                        \
  namespace pleione {                                                                                                  \
  inline namespace v0 {
#endif

#define PLEIONE_NAMESPACE_END                                                                                          \
  }                                                                                                                    \
  }

#if defined(__clang__) || defined(__GNUC__)

#define PLEIONE_HOT [[gnu::hot]]
#define PLEIONE_COLD [[gnu::cold]]

#define PLEIONE_NOINLINE [[gnu::noinline]]
#define PLEIONE_ALWAYS_INLINE [[gnu::always_inline]]

#elif defined(_MSC_VER)

#define PLEIONE_HOT
#define PLEIONE_COLD

#define PLEIONE_NOINLINE __declspec(noinline)
#define PLEIONE_ALWAYS_INLINE __forceinline

#else

#define PLEIONE_HOT
#define PLEIONE_COLD

#define PLEIONE_NOINLINE
#define PLEIONE_ALWAYS_INLINE

#endif

PLEIONE_NAMESPACE_BEGIN

/// Internal namespace
namespace detail {

[[noreturn]] PLEIONE_COLD PLEIONE_NOINLINE inline void assert_failure(char const* msg, char const* file,
                                                                      int line) noexcept {
  std::fprintf(stderr, "%s:%d: %s\n", file, line, msg);
  std::abort();
}

} // namespace detail

PLEIONE_NAMESPACE_END

#if defined(__clang__)
#define PLEIONE_ASSUME(...) __builtin_assume(__VA_ARGS__)
#elif defined(__GNUC__)
#define PLEIONE_ASSUME(...) static_cast<void>((__VA_ARGS__) ? void(0) : __builtin_unreachable())
#elif defined(_MSC_VER)
#define PLEIONE_ASSUME(...) __assume(__VA_ARGS__)
#else
#define PLEIONE_ASSUME(...)
#endif

#if PLEIONE_DEBUG
#define PLEIONE_ASSERT(...)                                                                                            \
  static_cast<void>((__VA_ARGS__)                                                                                      \
                        ? void(0)                                                                                      \
                        : ::pleione::detail::assert_failure("assertion failed: " #__VA_ARGS__, __FILE__, __LINE__))
#else
#define PLEIONE_ASSERT(...) PLEIONE_ASSUME(__VA_ARGS__)
#endif

#if defined(__clang__) || defined(__GNUC__)

#define PLEIONE_LIKELY(...) __builtin_expect(bool(__VA_ARGS__), true)
#define PLEIONE_UNLIKELY(...) __builtin_expect(bool(__VA_ARGS__), false)

#else

#define PLEIONE_LIKELY(...) (__VA_ARGS__)
#define PLEIONE_UNLIKELY(...) (__VA_ARGS__)

#endif

#if defined(__clang__) || defined(__GNUC__)
#define PLEIONE_PREFETCH(...) __builtin_prefetch((__VA_ARGS__))
#else
#define PLEIONE_PREFETCH(...)
#endif

#endif

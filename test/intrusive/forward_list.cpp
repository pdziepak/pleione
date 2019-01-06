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

#include "pleione/intrusive/forward_list.hpp"

#include <list>
#include <type_traits>

#include <gtest/gtest.h>

static_assert(std::is_trivial_v<pleione::intrusive::forward_list_hook>);

struct foo {
  int value;
  pleione::intrusive::forward_list_hook hook;

  friend bool operator==(foo const& a, foo const& b) noexcept { return &a == &b; }
};

using list_type = pleione::intrusive::forward_list<foo, &foo::hook>;

template<typename ForwardIt> static void check_equal_range(list_type& actual, ForwardIt first, ForwardIt last) {
  EXPECT_EQ(actual.empty(), std::distance(first, last) == 0);
  EXPECT_TRUE(std::equal(actual.begin(), actual.end(), first, last));
  if (first != last) { EXPECT_EQ(actual.front(), *first); }
}

template<typename Range> static void check_equal_range(list_type& actual, Range&& range) {
  check_equal_range(actual, range.begin(), range.end());
}

static void check_empty(list_type& actual) {
  EXPECT_TRUE(actual.empty());
  EXPECT_EQ(actual.begin(), actual.end());
}

TEST(intrusive_forward_list, default_constructor) {
  auto l = list_type();
  check_empty(l);
}

TEST(intrusive_forward_list, move_constructor) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  auto l2 = std::move(l);
  l.clear();
  check_equal_range(l2, fs);
}

TEST(intrusive_forward_list, move_assignment) {
  auto fs = std::vector<foo>(8);
  auto l = list_type();
  auto f = foo();
  l.push_front(f);
  {
    auto l2 = list_type(fs.begin(), fs.end());
    l = std::move(l2);
  }
  check_equal_range(l, fs);
}

TEST(intrusive_forward_list, range_constructor) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
}

TEST(intrusive_forward_list, empty_range_constructor) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.end(), fs.end());
  check_empty(l);
}

TEST(intrusive_forward_list, assign) {
  auto fs = std::vector<foo>(8);
  auto l = list_type();
  auto f = foo();
  l.push_front(f);
  l.assign(fs.begin(), fs.end());
  check_equal_range(l, fs);
}

TEST(intrusive_forward_list, assign_empty) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  l.assign(fs.end(), fs.end());
  check_empty(l);
}

TEST(intrusive_forward_list, clear) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  l.clear();
  check_empty(l);
  l.clear();
  check_empty(l);
}

TEST(intrusive_forward_list, push_front) {
  auto fs = std::vector<foo>(8);
  auto l = list_type();
  check_empty(l);
  for (auto it = fs.begin(); it != fs.end(); ++it) {
    l.push_front(*it);
    check_equal_range(l, std::make_reverse_iterator(std::next(it)), fs.rend());
  }
}

TEST(intrusive_forward_list, pop_front) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto it = fs.begin(); it != fs.end(); ++it) {
    l.pop_front();
    check_equal_range(l, std::next(it), fs.end());
  }
  check_empty(l);
}

TEST(intrusive_forward_list, insert_erase_single) {
  auto fs = std::list<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto idx = 0u; idx < fs.size(); ++idx) {
    auto it = std::next(fs.begin(), idx);
    auto it2 = fs.emplace(it);
    auto l_it = std::next(l.before_begin(), idx);
    EXPECT_EQ(*std::next(l_it), *it);
    auto l_it2 = l.insert_after(l_it, *it2);
    EXPECT_EQ(*l_it2, *it2);
    check_equal_range(l, fs);
    auto l_it3 = l.erase_after(l_it);
    EXPECT_EQ(l_it3, std::next(l_it));
    fs.erase(it2);
    check_equal_range(l, fs);
  }
}

TEST(intrusive_forward_list, insert_erase_empty_range) {
  auto fs = std::list<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto idx = 0u; idx < fs.size(); ++idx) {
    auto it = std::next(fs.begin(), idx);
    auto l_it = std::next(l.before_begin(), idx);
    EXPECT_EQ(*std::next(l_it), *it);
    auto l_it2 = l.insert_after(l_it, it, it);
    EXPECT_EQ(l_it2, l_it);
    check_equal_range(l, fs);
    auto l_it3 = l.erase_after(l_it2, l_it2);
    EXPECT_EQ(l_it3, l_it);
    check_equal_range(l, fs);
  }
}

TEST(intrusive_forward_list, insert_erase_singular_range) {
  auto fs = std::list<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto idx = 0u; idx < fs.size(); ++idx) {
    auto it = std::next(fs.begin(), idx);
    auto it2 = fs.emplace(it);
    auto l_it = std::next(l.before_begin(), idx);
    EXPECT_EQ(*std::next(l_it), *it);
    auto l_it2 = l.insert_after(l_it, it2, std::next(it2));
    EXPECT_EQ(*l_it2, *it2);
    check_equal_range(l, fs);
    auto l_it3 = l.erase_after(l_it, std::next(l_it2));
    EXPECT_EQ(l_it3, std::next(l_it));
    fs.erase(it2);
    check_equal_range(l, fs);
  }
}

TEST(intrusive_forward_list, insert_erase_range) {
  auto fs = std::list<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto idx = 0u; idx < fs.size(); ++idx) {
    auto it = std::next(fs.begin(), idx);
    auto it_first = fs.emplace(it);
    auto it_last = it_first;
    for (auto i = 0; i < 3; i++) { it_last = fs.emplace(std::next(it_last)); }
    auto l_it = std::next(l.before_begin(), idx);
    EXPECT_EQ(*std::next(l_it), *it);
    auto l_it_last = l.insert_after(l_it, it_first, std::next(it_last));
    EXPECT_EQ(*l_it_last, *it_last);
    check_equal_range(l, fs);
    auto l_it3 = l.erase_after(l_it, std::next(l_it_last));
    EXPECT_EQ(l_it3, std::next(l_it));
    fs.erase(it_first, std::next(it_last));
    check_equal_range(l, fs);
  }
}

TEST(intrusive_forward_list, splice_all_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());

    auto it = std::next(la.before_begin(), idx);
    auto f_it = std::next(fa.begin(), idx);
    la.splice_after(it, lb);
    fa.splice(f_it, fb);
    check_equal_range(la, fa);
    check_empty(lb);
  }
}

TEST(intrusive_forward_list, splice_all_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());

    auto it = std::next(la.before_begin(), idx);
    auto f_it = std::next(fa.begin(), idx);
    la.splice_after(it, std::move(lb));
    fa.splice(f_it, fb);
    check_equal_range(la, fa);
  }
}

TEST(intrusive_forward_list, splice_all_empty_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type();

    auto it = std::next(la.before_begin(), idx);
    la.splice_after(it, lb);
    check_equal_range(la, fa);
    check_empty(lb);
  }
}

TEST(intrusive_forward_list, splice_all_empty_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type();

    auto it = std::next(la.before_begin(), idx);
    la.splice_after(it, std::move(lb));
    check_equal_range(la, fa);
  }
}

TEST(intrusive_forward_list, splice_single_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 8; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.before_begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.before_begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice_after(it_a, lb, it_b);
      fa.splice(f_it_a, fb, f_it_b);
      check_equal_range(la, fa);
      check_equal_range(lb, fb);
    }
  }
}

TEST(intrusive_forward_list, splice_single_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 8; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.before_begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.before_begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice_after(it_a, std::move(lb), it_b);
      fa.splice(f_it_a, std::move(fb), f_it_b);
      check_equal_range(la, fa);
    }
  }
}

TEST(intrusive_forward_list, splice_empty_range_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());
    auto it_a = std::next(la.before_begin(), idx);
    la.splice_after(it_a, lb, lb.end(), lb.end());
    check_equal_range(la, fa);
    check_equal_range(lb, fb);
  }
}

TEST(intrusive_forward_list, splice_empty_range_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());
    auto it_a = std::next(la.before_begin(), idx);
    la.splice_after(it_a, std::move(lb), lb.end(), lb.end());
    check_equal_range(la, fa);
  }
}

TEST(intrusive_forward_list, splice_full_range_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());
    auto it_a = std::next(la.before_begin(), idx);
    auto f_it_a = std::next(fa.begin(), idx);
    la.splice_after(it_a, lb, lb.before_begin(), lb.end());
    fa.splice(f_it_a, fb, fb.begin(), fb.end());
    check_equal_range(la, fa);
    check_empty(lb);
  }
}

TEST(intrusive_forward_list, splice_full_range_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());
    auto it_a = std::next(la.before_begin(), idx);
    auto f_it_a = std::next(fa.begin(), idx);
    la.splice_after(it_a, lb, lb.before_begin(), lb.end());
    fa.splice(f_it_a, fb, fb.begin(), fb.end());
    check_equal_range(la, fa);
  }
}

TEST(intrusive_forward_list, splice_singular_range_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 8; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.before_begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.before_begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice_after(it_a, lb, it_b, std::next(it_b, 2));
      fa.splice(f_it_a, fb, f_it_b, std::next(f_it_b));
      check_equal_range(la, fa);
      check_equal_range(lb, fb);
    }
  }
}

TEST(intrusive_forward_list, splice_singular_range_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 8; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.before_begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.before_begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice_after(it_a, std::move(lb), it_b, std::next(it_b, 2));
      fa.splice(f_it_a, std::move(fb), f_it_b, std::next(f_it_b));
      check_equal_range(la, fa);
    }
  }
}

TEST(intrusive_forward_list, splice_range_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 7; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.before_begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.before_begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice_after(it_a, lb, it_b, std::next(it_b, 3));
      fa.splice(f_it_a, fb, f_it_b, std::next(f_it_b, 2));
      check_equal_range(la, fa);
      check_equal_range(lb, fb);
    }
  }
}

TEST(intrusive_forward_list, splice_range_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 7; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.before_begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.before_begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice_after(it_a, std::move(lb), it_b, std::next(it_b, 3));
      fa.splice(f_it_a, std::move(fb), f_it_b, std::next(f_it_b, 2));
      check_equal_range(la, fa);
    }
  }
}

TEST(intrusive_forward_list, forward_iterator) {
  static_assert(
      std::is_base_of_v<std::forward_iterator_tag, std::iterator_traits<list_type::iterator>::iterator_category>);
  static_assert(
      std::is_base_of_v<std::forward_iterator_tag, std::iterator_traits<list_type::const_iterator>::iterator_category>);

  static_assert(std::is_same_v<std::iterator_traits<list_type::iterator>::value_type, foo>);
  static_assert(std::is_same_v<std::iterator_traits<list_type::const_iterator>::value_type, foo const>);

  auto fs = std::vector<foo>(16);

  auto test_iterators = [&](auto& l) {
    {
      auto idx = 0;
      auto it = l.before_begin();
      EXPECT_EQ(it, l.before_begin());
      EXPECT_EQ(++it, l.begin());
      while (it != l.end()) {
        EXPECT_LT(idx, fs.size());
        EXPECT_EQ(*it, fs[idx]);
        EXPECT_EQ(&it->value, &fs[idx].value);
        ++it;
        ++idx;
      }
    }

    {
      auto idx = 0;
      auto it = l.before_begin();
      EXPECT_EQ(it++, l.before_begin());
      EXPECT_EQ(it, l.begin());
      while (it != l.end()) {
        EXPECT_LT(idx, fs.size());
        EXPECT_EQ(&it->value, &fs[idx].value);
        EXPECT_EQ(*it++, fs[idx]);
        ++idx;
      }
    }
  };

  auto l = list_type(fs.begin(), fs.end());
  test_iterators(l);
  list_type const& lc = l;
  test_iterators(lc);

  list_type::iterator it1{};
  list_type::iterator it2{};
  EXPECT_TRUE(it1 == it2);
  EXPECT_FALSE(it1 != it2);

  list_type::const_iterator cit1{};
  list_type::const_iterator cit2{};
  EXPECT_TRUE(cit1 == cit2);
  EXPECT_FALSE(cit1 != cit2);
}

TEST(intrusive_forward_list, front) {
  auto fs = std::vector<foo>(16);

  auto l = list_type(fs.begin(), fs.end());
  EXPECT_EQ(l.front(), fs.front());

  list_type const& lc = l;
  EXPECT_EQ(lc.front(), fs.front());
}

TEST(intrusive_forward_list, swap) {
  auto fa = std::vector<foo>(8);
  auto fb = std::vector<foo>(16);
  auto la = list_type();
  auto lb = list_type();
  using std::swap;
  swap(la, la);
  check_empty(la);
  swap(la, lb);
  check_empty(la);
  check_empty(lb);
  la.assign(fa.begin(), fa.end());
  swap(la, lb);
  check_empty(la);
  check_equal_range(lb, fa);
  swap(lb, lb);
  check_equal_range(lb, fa);
  la.assign(fb.begin(), fb.end());
  swap(la, lb);
  check_equal_range(la, fa);
  check_equal_range(lb, fb);
}

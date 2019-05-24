/*
 * Copyright © 2018-2019 Paweł Dziepak
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

#include <array>
#include <list>
#include <type_traits>

#include <gtest/gtest.h>

#include "../state_walk.hpp"

static_assert(std::is_trivially_default_constructible_v<pleione::intrusive::list_hook>);

struct foo {
  int value;
  pleione::intrusive::list_hook hook;

  friend bool operator==(foo const& a, foo const& b) noexcept { return &a == &b; }
};

using list_type = pleione::intrusive::list<foo, &foo::hook>;

template<typename ForwardIt> static void check_equal_range(list_type& actual, ForwardIt first, ForwardIt last) {
  EXPECT_EQ(actual.empty(), std::distance(first, last) == 0);
  EXPECT_EQ(actual.size(), std::distance(first, last));
  EXPECT_TRUE(std::equal(actual.begin(), actual.end(), first, last));
  EXPECT_TRUE(std::equal(std::make_reverse_iterator(actual.end()), std::make_reverse_iterator(actual.begin()),
                         std::make_reverse_iterator(last), std::make_reverse_iterator(first)));
  if (first != last) {
    EXPECT_EQ(actual.front(), *first);
    EXPECT_EQ(actual.back(), *std::prev(last));
  }
}

template<typename Range> static void check_equal_range(list_type& actual, Range&& range) {
  check_equal_range(actual, range.begin(), range.end());
}

static void check_empty(list_type& actual) {
  EXPECT_TRUE(actual.empty());
  EXPECT_EQ(actual.size(), 0);
  EXPECT_EQ(actual.begin(), actual.end());
  EXPECT_EQ(actual.rbegin(), actual.rend());
}

TEST(intrusive_list, default_constructor) {
  auto l = list_type();
  check_empty(l);
}

TEST(intrusive_list, range_constructor) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
}

TEST(intrusive_list, empty_range_constructor) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.end(), fs.end());
  check_empty(l);
}

TEST(intrusive_list, move_constructor) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  auto l2 = std::move(l);
  l.clear();
  check_equal_range(l2, fs);
}

TEST(intrusive_list, move_assignment) {
  auto fs = std::vector<foo>(8);
  auto l = list_type();
  auto f = foo();
  l.push_back(f);
  {
    auto l2 = list_type(fs.begin(), fs.end());
    l = std::move(l2);
  }
  check_equal_range(l, fs);
}

TEST(intrusive_list, assign) {
  auto fs = std::vector<foo>(8);
  auto l = list_type();
  auto f = foo();
  l.push_back(f);
  l.assign(fs.begin(), fs.end());
  check_equal_range(l, fs);
}

TEST(intrusive_list, assign_empty) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  l.assign(fs.end(), fs.end());
  check_empty(l);
}

TEST(intrusive_list, clear) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  l.clear();
  check_empty(l);
  l.clear();
  check_empty(l);
}

TEST(intrusive_list, push_back) {
  auto fs = std::vector<foo>(8);
  auto l = list_type();
  check_empty(l);
  for (auto it = fs.begin(); it != fs.end(); ++it) {
    l.push_back(*it);
    check_equal_range(l, fs.begin(), std::next(it));
  }
}

TEST(intrusive_list, push_front) {
  auto fs = std::vector<foo>(8);
  auto l = list_type();
  check_empty(l);
  for (auto it = fs.begin(); it != fs.end(); ++it) {
    l.push_front(*it);
    check_equal_range(l, std::make_reverse_iterator(std::next(it)), fs.rend());
  }
}

TEST(intrusive_list, pop_back) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto it = fs.rbegin(); it != fs.rend(); ++it) {
    l.pop_back();
    check_equal_range(l, fs.begin(), std::prev(it.base()));
  }
  check_empty(l);
}

TEST(intrusive_list, pop_front) {
  auto fs = std::vector<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto it = fs.begin(); it != fs.end(); ++it) {
    l.pop_front();
    check_equal_range(l, std::next(it), fs.end());
  }
  check_empty(l);
}

TEST(intrusive_list, insert_erase_single) {
  auto fs = std::list<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto it = fs.begin(); it != fs.end(); ++it) {
    auto it2 = fs.emplace(it);
    auto l_it = std::find(l.begin(), l.end(), *it);
    EXPECT_EQ(*l_it, *it);
    auto l_it2 = l.insert(l_it, *it2);
    EXPECT_EQ(*l_it2, *it2);
    check_equal_range(l, fs);
    auto l_it3 = l.erase(l_it2);
    EXPECT_EQ(l_it3, l_it);
    fs.erase(it2);
    check_equal_range(l, fs);
  }

  auto it2 = fs.emplace(fs.end());
  auto l_it2 = l.insert(l.end(), *it2);
  EXPECT_EQ(*l_it2, *it2);
  check_equal_range(l, fs);
  auto l_it3 = l.erase(l_it2);
  EXPECT_EQ(l_it3, l.end());
  fs.erase(it2);
  check_equal_range(l, fs);
}

TEST(intrusive_list, insert_erase_empty_range) {
  auto fs = std::list<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto it = fs.begin(); it != fs.end(); ++it) {
    auto l_it = std::find(l.begin(), l.end(), *it);
    EXPECT_EQ(*l_it, *it);
    auto l_it2 = l.insert(l_it, it, it);
    EXPECT_EQ(l_it2, l_it);
    check_equal_range(l, fs);
    auto l_it3 = l.erase(l_it2, l_it2);
    EXPECT_EQ(l_it3, l_it);
    check_equal_range(l, fs);
  }

  auto l_it2 = l.insert(l.end(), fs.end(), fs.end());
  EXPECT_EQ(l_it2, l.end());
  check_equal_range(l, fs);
  auto l_it3 = l.erase(l_it2, l_it2);
  EXPECT_EQ(l_it3, l.end());
  check_equal_range(l, fs);
}

TEST(intrusive_list, insert_erase_singular_range) {
  auto fs = std::list<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto it = fs.begin(); it != fs.end(); ++it) {
    auto it2 = fs.emplace(it);
    auto l_it = std::find(l.begin(), l.end(), *it);
    EXPECT_EQ(*l_it, *it);
    auto l_it2 = l.insert(l_it, it2, std::next(it2));
    EXPECT_EQ(*l_it2, *it2);
    check_equal_range(l, fs);
    auto l_it3 = l.erase(l_it2, std::next(l_it2));
    EXPECT_EQ(l_it3, l_it);
    fs.erase(it2);
    check_equal_range(l, fs);
  }

  auto it2 = fs.emplace(fs.end());
  auto l_it2 = l.insert(l.end(), it2, std::next(it2));
  EXPECT_EQ(*l_it2, *it2);
  check_equal_range(l, fs);
  auto l_it3 = l.erase(l_it2, std::next(l_it2));
  EXPECT_EQ(l_it3, l.end());
  fs.erase(it2);
  check_equal_range(l, fs);
}

TEST(intrusive_list, insert_erase_range) {
  auto fs = std::list<foo>(8);
  auto l = list_type(fs.begin(), fs.end());
  check_equal_range(l, fs);
  for (auto it = fs.begin(); it != fs.end(); ++it) {
    auto it_first = fs.emplace(it);
    auto it_last = it_first;
    for (auto i = 0; i < 3; i++) { it_last = fs.emplace(std::next(it_last)); }
    auto l_it = std::find(l.begin(), l.end(), *it);
    EXPECT_EQ(*l_it, *it);
    auto l_it_first = l.insert(l_it, it_first, std::next(it_last));
    EXPECT_EQ(*l_it_first, *it_first);
    check_equal_range(l, fs);
    auto l_it3 = l.erase(l_it_first, std::next(l_it_first, 4));
    EXPECT_EQ(l_it3, l_it);
    fs.erase(it_first, std::next(it_last));
    check_equal_range(l, fs);
  }

  auto it_first = fs.emplace(fs.end());
  auto it_last = it_first;
  for (auto i = 0; i < 3; i++) { it_last = fs.emplace(std::next(it_last)); }
  auto l_it_first = l.insert(l.end(), it_first, std::next(it_last));
  EXPECT_EQ(*l_it_first, *it_first);
  check_equal_range(l, fs);
  auto l_it3 = l.erase(l_it_first, std::next(l_it_first, 4));
  EXPECT_EQ(l_it3, l.end());
  fs.erase(it_first, std::next(it_last));
  check_equal_range(l, fs);
}

TEST(intrusive_list, splice_all_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());

    auto it = std::next(la.begin(), idx);
    auto f_it = std::next(fa.begin(), idx);
    la.splice(it, lb);
    fa.splice(f_it, fb);
    check_equal_range(la, fa);
    check_empty(lb);
  }
}

TEST(intrusive_list, splice_all_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());

    auto it = std::next(la.begin(), idx);
    auto f_it = std::next(fa.begin(), idx);
    la.splice(it, std::move(lb));
    fa.splice(f_it, fb);
    check_equal_range(la, fa);
  }
}

TEST(intrusive_list, splice_all_empty_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type();

    auto it = std::next(la.begin(), idx);
    la.splice(it, lb);
    check_equal_range(la, fa);
    check_empty(lb);
  }
}

TEST(intrusive_list, splice_all_empty_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type();

    auto it = std::next(la.begin(), idx);
    la.splice(it, std::move(lb));
    check_equal_range(la, fa);
  }
}

TEST(intrusive_list, splice_single_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 8; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice(it_a, lb, it_b);
      fa.splice(f_it_a, fb, f_it_b);
      check_equal_range(la, fa);
      check_equal_range(lb, fb);
    }
  }
}

TEST(intrusive_list, splice_single_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 8; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice(it_a, std::move(lb), it_b);
      fa.splice(f_it_a, std::move(fb), f_it_b);
      check_equal_range(la, fa);
    }
  }
}

TEST(intrusive_list, splice_empty_range_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());
    auto it_a = std::next(la.begin(), idx);
    la.splice(it_a, lb, lb.end(), lb.end());
    check_equal_range(la, fa);
    check_equal_range(lb, fb);
  }
}

TEST(intrusive_list, splice_empty_range_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());
    auto it_a = std::next(la.begin(), idx);
    la.splice(it_a, std::move(lb), lb.end(), lb.end());
    check_equal_range(la, fa);
  }
}

TEST(intrusive_list, splice_full_range_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());
    auto it_a = std::next(la.begin(), idx);
    auto f_it_a = std::next(fa.begin(), idx);
    la.splice(it_a, lb, lb.begin(), lb.end());
    fa.splice(f_it_a, fb, fb.begin(), fb.end());
    check_equal_range(la, fa);
    check_empty(lb);
  }
}

TEST(intrusive_list, splice_full_range_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    auto fa = std::list<foo>(8);
    auto fb = std::list<foo>(8);
    auto la = list_type(fa.begin(), fa.end());
    auto lb = list_type(fb.begin(), fb.end());
    auto it_a = std::next(la.begin(), idx);
    auto f_it_a = std::next(fa.begin(), idx);
    la.splice(it_a, lb, lb.begin(), lb.end());
    fa.splice(f_it_a, fb, fb.begin(), fb.end());
    check_equal_range(la, fa);
  }
}

TEST(intrusive_list, splice_singular_range_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 8; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice(it_a, lb, it_b, std::next(it_b));
      fa.splice(f_it_a, fb, f_it_b, std::next(f_it_b));
      check_equal_range(la, fa);
      check_equal_range(lb, fb);
    }
  }
}

TEST(intrusive_list, splice_singular_range_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 8; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice(it_a, std::move(lb), it_b, std::next(it_b));
      fa.splice(f_it_a, std::move(fb), f_it_b, std::next(f_it_b));
      check_equal_range(la, fa);
    }
  }
}

TEST(intrusive_list, splice_range_lvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 7; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice(it_a, lb, it_b, std::next(it_b, 2));
      fa.splice(f_it_a, fb, f_it_b, std::next(f_it_b, 2));
      check_equal_range(la, fa);
      check_equal_range(lb, fb);
    }
  }
}

TEST(intrusive_list, splice_range_rvalue) {
  for (auto idx = 0; idx <= 8; ++idx) {
    for (auto jdx = 0; jdx < 7; ++jdx) {
      auto fa = std::list<foo>(8);
      auto fb = std::list<foo>(8);
      auto la = list_type(fa.begin(), fa.end());
      auto lb = list_type(fb.begin(), fb.end());
      auto it_a = std::next(la.begin(), idx);
      auto f_it_a = std::next(fa.begin(), idx);
      auto it_b = std::next(lb.begin(), jdx);
      auto f_it_b = std::next(fb.begin(), jdx);
      la.splice(it_a, std::move(lb), it_b, std::next(it_b, 2));
      fa.splice(f_it_a, std::move(fb), f_it_b, std::next(f_it_b, 2));
      check_equal_range(la, fa);
    }
  }
}

TEST(intrusive_list, forward_iterator) {
  static_assert(
      std::is_base_of_v<std::bidirectional_iterator_tag, std::iterator_traits<list_type::iterator>::iterator_category>);
  static_assert(std::is_base_of_v<std::bidirectional_iterator_tag,
                                  std::iterator_traits<list_type::const_iterator>::iterator_category>);

  static_assert(std::is_same_v<std::iterator_traits<list_type::iterator>::value_type, foo>);
  static_assert(std::is_same_v<std::iterator_traits<list_type::const_iterator>::value_type, foo const>);

  auto fs = std::vector<foo>(16);

  auto test_iterators = [&](auto& l) {
    {
      auto idx = 0u;
      auto it = l.begin();
      while (it != l.end()) {
        EXPECT_LT(idx, fs.size());
        EXPECT_EQ(*it, fs[idx]);
        EXPECT_EQ(&it->value, &fs[idx].value);
        ++it;
        ++idx;
      }
    }

    {
      auto idx = 0u;
      auto it = l.begin();
      while (it != l.end()) {
        EXPECT_LT(idx, fs.size());
        EXPECT_EQ(&it->value, &fs[idx].value);
        EXPECT_EQ(*it++, fs[idx]);
        ++idx;
      }
    }

    {
      auto idx = fs.size();
      auto it = l.end();
      while (it != l.begin()) {
        --idx;
        EXPECT_GE(idx, 0u);
        EXPECT_EQ(*--it, fs[idx]);
        EXPECT_EQ(&it->value, &fs[idx].value);
      }
      EXPECT_EQ(it, l.begin());
    }

    {
      auto idx = fs.size();
      auto it = l.end();
      while (it != l.begin()) {
        --idx;
        it--;
        EXPECT_GE(idx, 0u);
        EXPECT_EQ(*it, fs[idx]);
        EXPECT_EQ(&it->value, &fs[idx].value);
      }
      EXPECT_EQ(it, l.begin());
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

TEST(intrusive_list, reverse_iterator) {
  static_assert(std::is_base_of_v<std::bidirectional_iterator_tag,
                                  std::iterator_traits<list_type::reverse_iterator>::iterator_category>);
  static_assert(std::is_base_of_v<std::bidirectional_iterator_tag,
                                  std::iterator_traits<list_type::const_reverse_iterator>::iterator_category>);

  static_assert(std::is_same_v<std::iterator_traits<list_type::reverse_iterator>::value_type, foo>);
  static_assert(std::is_same_v<std::iterator_traits<list_type::const_reverse_iterator>::value_type, foo const>);

  auto fs = std::vector<foo>(16);

  auto test_iterators = [&](auto& l) {
    {
      auto idx = 0u;
      auto it = l.rbegin();
      while (it != l.rend()) {
        EXPECT_LT(idx, fs.size());
        EXPECT_EQ(*it, fs[fs.size() - idx - 1]);
        EXPECT_EQ(&it->value, &fs[fs.size() - idx - 1].value);
        ++it;
        ++idx;
      }
    }

    {
      auto idx = 0u;
      auto it = l.rbegin();
      while (it != l.rend()) {
        EXPECT_LT(idx, fs.size());
        EXPECT_EQ(&it->value, &fs[fs.size() - idx - 1].value);
        EXPECT_EQ(*it++, fs[fs.size() - idx - 1]);
        ++idx;
      }
    }

    {
      auto idx = fs.size();
      auto it = l.rend();
      while (it != l.rbegin()) {
        --idx;
        EXPECT_GE(idx, 0u);
        EXPECT_EQ(*--it, fs[fs.size() - idx - 1]);
        EXPECT_EQ(&it->value, &fs[fs.size() - idx - 1].value);
      }
      EXPECT_EQ(it, l.rbegin());
    }

    {
      auto idx = fs.size();
      auto it = l.rend();
      while (it != l.rbegin()) {
        --idx;
        it--;
        EXPECT_GE(idx, 0u);
        EXPECT_EQ(*it, fs[fs.size() - idx - 1]);
        EXPECT_EQ(&it->value, &fs[fs.size() - idx - 1].value);
      }
      EXPECT_EQ(it, l.rbegin());
    }
  };

  auto l = list_type(fs.begin(), fs.end());
  test_iterators(l);
  list_type const& lc = l;
  test_iterators(lc);

  list_type::reverse_iterator it1{};
  list_type::reverse_iterator it2{};
  EXPECT_TRUE(it1 == it2);
  EXPECT_FALSE(it1 != it2);

  list_type::const_reverse_iterator cit1{};
  list_type::const_reverse_iterator cit2{};
  EXPECT_TRUE(cit1 == cit2);
  EXPECT_FALSE(cit1 != cit2);
}

TEST(intrusive_list, front_back) {
  auto fs = std::vector<foo>(16);

  auto l = list_type(fs.begin(), fs.end());
  EXPECT_EQ(l.front(), fs.front());
  EXPECT_EQ(l.back(), fs.back());

  list_type const& lc = l;
  EXPECT_EQ(lc.front(), fs.front());
  EXPECT_EQ(lc.back(), fs.back());
}

TEST(intrusive_list, swap) {
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

struct object {
  pleione::intrusive::list_hook hook;
  size_t value;

  friend bool operator==(object const& a, object const& b) noexcept { return a.value == b.value; }
};

size_t value_counter = 0;

struct state {
  std::list<object> std_;
  pleione::intrusive::list<object, &object::hook> pln_;

public:
  state() = default;
  state(state&&) = default;
  state(state const& other) {
    for (auto& obj : other.std_) {
      auto& nobj = std_.emplace_back();
      nobj.value = obj.value;
      pln_.push_back(nobj);
    }
  }

  bool empty() const { return std_.empty(); }
  size_t size() const { return std_.size(); }

  void push_front() {
    auto& obj = std_.emplace_front();
    obj.value = value_counter++;
    pln_.push_front(obj);
  }

  void push_back() {
    auto& obj = std_.emplace_back();
    obj.value = value_counter++;
    pln_.push_back(obj);
  }

  void pop_front() {
    pln_.pop_front();
    std_.pop_front();
  }

  void pop_back() {
    pln_.pop_back();
    std_.pop_back();
  }

  void insert(size_t idx) {
    auto it = std_.emplace(std::next(std_.begin(), idx));
    it->value = value_counter++;
    pln_.insert(std::next(pln_.begin(), idx), *it);
  }

  void insert(size_t idx, size_t n) {
    auto it = std::next(std_.begin(), idx);
    for (auto i = 0u; i < n; i++) {
      auto it2 = std_.emplace(it);
      it2->value = value_counter++;
    }
    pln_.insert(std::next(pln_.begin(), idx), std::next(std_.begin(), idx), std::next(std_.begin(), idx + n));
  }

  void erase(size_t idx) {
    pln_.erase(std::next(pln_.begin(), idx));
    std_.erase(std::next(std_.begin(), idx));
  }

  void erase(size_t first, size_t last) {
    pln_.erase(std::next(pln_.begin(), first), std::next(pln_.begin(), last));
    std_.erase(std::next(std_.begin(), first), std::next(std_.begin(), last));
  }

  void validate() const {
    EXPECT_EQ(std_.empty(), pln_.empty());
    EXPECT_EQ(std_.size(), pln_.size());
    if (!std_.empty()) {
      EXPECT_EQ(&std_.front(), &pln_.front());
      EXPECT_EQ(&std_.back(), &pln_.back());
    } else {
      EXPECT_EQ(pln_.begin(), pln_.end());
      EXPECT_EQ(pln_.rbegin(), pln_.rend());
    }
    EXPECT_EQ(pln_.size(), std::distance(pln_.begin(), pln_.end()));
    EXPECT_EQ(pln_.size(), std::distance(pln_.rbegin(), pln_.rend()));
    EXPECT_TRUE(std::equal(std_.begin(), std_.end(), pln_.begin(), pln_.end()));
    EXPECT_TRUE(std::equal(std_.rbegin(), std_.rend(), pln_.rbegin(), pln_.rend()));
  }

  bool check_bounds() const { return std_.size() < 8; }

  friend bool operator==(state const& a, state const& b) noexcept { return a.std_.size() == b.std_.size(); }
};

namespace std {

template<> struct hash<::state> {
  size_t operator()(state const& st) const noexcept { return st.size(); }
};

} // namespace std

TEST(intrusive_list, state_walk) {
  state_walk<state>({
                        [](state const& in) {
                          std::vector<state> out;
                          out.emplace_back(in).push_front();
                          return out;
                        },
                        [](state const& in) {
                          std::vector<state> out;
                          out.emplace_back(in).push_back();
                          return out;
                        },
                        [](state const& in) {
                          std::vector<state> out;
                          if (!in.empty()) { out.emplace_back(in).pop_front(); }
                          return out;
                        },
                        [](state const& in) {
                          std::vector<state> out;
                          if (!in.empty()) { out.emplace_back(in).pop_back(); }
                          return out;
                        },
                        [](state const& in) {
                          std::vector<state> out;
                          for (auto i = 0u; i <= in.size(); i++) { out.emplace_back(in).insert(i); }
                          return out;
                        },
                        [](state const& in) {
                          std::vector<state> out;
                          for (auto i = 0u; i < in.size(); i++) {
                            for (auto j = 0u; j < 16; j++) { out.emplace_back(in).insert(i, j); }
                          }
                          return out;
                        },
                        [](state const& in) {
                          std::vector<state> out;
                          for (auto i = 0u; i < in.size(); i++) { out.emplace_back(in).erase(i); }
                          return out;
                        },
                        [](state const& in) {
                          std::vector<state> out;
                          for (auto i = 0u; i < in.size(); i++) {
                            for (auto j = 0u; j <= in.size() - i; j++) { out.emplace_back(in).erase(i, i + j); }
                          }
                          return out;
                        },
                    },
                    std::mem_fn(&state::validate), std::mem_fn(&state::check_bounds));
}

TEST(intrusive_list, for_each) {
  auto fs = std::vector<foo>(16);
  for (auto idx = 0u; idx < fs.size(); idx++) { fs[idx].value = idx; }
  auto l = list_type(fs.begin(), fs.end());

  {
    auto visited = std::array<int, 16>{};
    for_each(l.begin(), l.end(), [&](auto const& object) {
      EXPECT_LT(unsigned(object.value), visited.size());
      visited[object.value]++;
    });
    EXPECT_TRUE(std::all_of(visited.begin(), visited.end(), [](int x) { return x == 1; }));
  }

  {
    auto visited = std::array<int, 16>{};
    for_each(pleione::prefetch<true>{}, l.begin(), l.end(), [&](auto const& object) {
      EXPECT_LT(unsigned(object.value), visited.size());
      visited[object.value]++;
    });
    EXPECT_TRUE(std::all_of(visited.begin(), visited.end(), [](int x) { return x == 1; }));
  }

  {
    auto visited = std::array<int, 16>{};
    for_each(pleione::prefetch<false>{}, l.begin(), l.end(), [&](auto const& object) {
      EXPECT_LT(unsigned(object.value), visited.size());
      visited[object.value]++;
    });
    EXPECT_TRUE(std::all_of(visited.begin(), visited.end(), [](int x) { return x == 1; }));
  }
}

TEST(intrusive_list, transform_reduce) {
  auto fs = std::vector<foo>(16);
  for (auto idx = 0u; idx < fs.size(); idx++) { fs[idx].value = idx; }
  auto l = list_type(fs.begin(), fs.end());

  {
    auto visited = std::array<int, 16>{};
    auto value = transform_reduce(l.begin(), l.end(), 4, std::plus<>{}, [&](auto const& object) {
      EXPECT_LT(unsigned(object.value), visited.size());
      visited[object.value]++;
      return object.value;
    });
    EXPECT_TRUE(std::all_of(visited.begin(), visited.end(), [](int x) { return x == 1; }));
    EXPECT_EQ(value, 124);
  }

  {
    auto visited = std::array<int, 16>{};
    auto value =
        transform_reduce(pleione::prefetch<true>{}, l.begin(), l.end(), 4, std::plus<>{}, [&](auto const& object) {
          EXPECT_LT(unsigned(object.value), visited.size());
          visited[object.value]++;
          return object.value;
        });
    EXPECT_TRUE(std::all_of(visited.begin(), visited.end(), [](int x) { return x == 1; }));
    EXPECT_EQ(value, 124);
  }

  {
    auto visited = std::array<int, 16>{};
    auto value =
        transform_reduce(pleione::prefetch<false>{}, l.begin(), l.end(), 4, std::plus<>{}, [&](auto const& object) {
          EXPECT_LT(unsigned(object.value), visited.size());
          visited[object.value]++;
          return object.value;
        });
    EXPECT_TRUE(std::all_of(visited.begin(), visited.end(), [](int x) { return x == 1; }));
    EXPECT_EQ(value, 124);
  }

  {
    auto value = transform_reduce(l.begin(), l.end(), 1, std::multiplies<>{}, [&](auto const&) { return 2; });
    EXPECT_EQ(value, 65536);
  }
}

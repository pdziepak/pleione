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

#include "pleione/detail/container_of.hpp"

#include <gtest/gtest.h>

struct foo {
  int x;
};

struct bar {
  int x;
  foo y;
  int z;
};

TEST(offset_of, standard_layout) {
  EXPECT_EQ((pleione::detail::offset_of<bar, int>(&bar::x)), offsetof(bar, x));
  EXPECT_EQ((pleione::detail::offset_of<bar, foo>(&bar::y)), offsetof(bar, y));
  EXPECT_EQ((pleione::detail::offset_of<bar, int>(&bar::z)), offsetof(bar, z));
}

TEST(container_of, standard_layout) {
  auto b = bar{};
  EXPECT_EQ(&(pleione::detail::container_of<bar, int>(&bar::x, b.x)), &b);
  EXPECT_EQ(&(pleione::detail::container_of<bar, foo>(&bar::y, b.y)), &b);
  EXPECT_EQ(&(pleione::detail::container_of<bar, int>(&bar::z, b.z)), &b);
}

struct vfoo {
  int x;
  virtual ~vfoo() = default;
};

struct vbar {
  int y;
  virtual ~vbar() = default;
};

struct vbaz : public vbar, public vfoo {
  int z;
  vfoo w;
};

TEST(container_of, non_standard_layout) {
  auto v = vbaz{};
  EXPECT_EQ(&(pleione::detail::container_of<vbaz, int>(&vbaz::x, v.x)), &v);
  EXPECT_EQ(&(pleione::detail::container_of<vbaz, int>(&vbaz::y, v.y)), &v);
  EXPECT_EQ(&(pleione::detail::container_of<vbaz, int>(&vbaz::z, v.z)), &v);
  EXPECT_EQ(&(pleione::detail::container_of<vbaz, vfoo>(&vbaz::w, v.w)), &v);

  auto& b = *static_cast<vbar*>(&v);
  EXPECT_EQ(&(pleione::detail::container_of<vbaz, int>(&vbaz::y, b.y)), &v);
}

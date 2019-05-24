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

#ifndef PLEIONE_INTRUSIVE_FORWARD_LIST_HPP
#define PLEIONE_INTRUSIVE_FORWARD_LIST_HPP

#include <algorithm>

#include "core.hpp"

#include "../detail/container_of.hpp"

PLEIONE_NAMESPACE_BEGIN

namespace intrusive {

class forward_list_hook {
  forward_list_hook* next_;

private:
  explicit forward_list_hook(forward_list_hook* next) noexcept : next_(next) {}

  template<typename T, forward_list_hook T::*> friend class forward_list;

public:
  forward_list_hook() = default;
  forward_list_hook(forward_list_hook const&) = delete;
  forward_list_hook(forward_list_hook&&) = delete;
};

template<typename T, forward_list_hook T::*Hook> class forward_list {
  forward_list_hook root_{nullptr};

public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = value_type const&;
  using pointer = value_type*;
  using const_pointer = value_type const*;

public:
  template<bool Constant> class basic_iterator {
    using hook_type = std::conditional_t<Constant, forward_list_hook const, forward_list_hook>;
    hook_type* current_ = nullptr;

  private:
    explicit basic_iterator(hook_type* hook) noexcept : current_(hook) {}

    friend class forward_list;

  public:
    using value_type = std::conditional_t<Constant, T const, T>;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    basic_iterator() = default;

    operator basic_iterator<true>() const noexcept { return basic_iterator<true>(current_); }

    reference operator*() const noexcept { return detail::container_of<value_type, hook_type>(Hook, *current_); }
    pointer operator->() const noexcept { return &detail::container_of<value_type, hook_type>(Hook, *current_); }

    basic_iterator& operator++() noexcept {
      current_ = current_->next_;
      return *this;
    }
    basic_iterator operator++(int) noexcept {
      auto it = *this;
      operator++();
      return it;
    }

    bool operator==(basic_iterator const& other) const noexcept { return current_ == other.current_; }
    bool operator!=(basic_iterator const& other) const noexcept { return !(*this == other); }

    void prefetch_next() const noexcept { PLEIONE_PREFETCH(current_->next_); }
  };

public:
  using iterator = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;

public:
  forward_list() = default;

  template<typename ForwardIt> forward_list(ForwardIt first, ForwardIt last) noexcept {
    static_assert(
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>);
    assign(first, last);
  }

  forward_list(forward_list const&) = delete;
  forward_list(forward_list&& other) noexcept { root_.next_ = other.root_.next_; }

  forward_list& operator=(forward_list const&) = delete;
  forward_list& operator=(forward_list&& other) noexcept {
    root_.next_ = other.root_.next_;
    return *this;
  }

  template<typename ForwardIt> void assign(ForwardIt first, ForwardIt last) noexcept {
    static_assert(
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>);
    auto prev = &root_;
    using std::for_each;
    for_each(first, last, [&](T& object) {
      auto& hook = object.*Hook;
      prev->next_ = &hook;
      prev = &hook;
    });
    prev->next_ = nullptr;
  }

  T& front() noexcept { return detail::container_of<T, forward_list_hook>(Hook, *root_.next_); }
  T const& front() const noexcept { return detail::container_of<T, forward_list_hook>(Hook, *root_.next_); }

  iterator before_begin() noexcept { return iterator(&root_); }
  const_iterator before_begin() const noexcept { return const_iterator(&root_); }
  iterator begin() noexcept { return iterator(root_.next_); }
  const_iterator begin() const noexcept { return const_iterator(root_.next_); }
  iterator end() noexcept { return iterator(nullptr); }
  const_iterator end() const noexcept { return const_iterator(nullptr); }

  bool empty() const noexcept { return !root_.next_; }

  void clear() noexcept { root_.next_ = nullptr; }

  iterator insert_after(iterator position, T& object) noexcept {
    PLEIONE_ASSERT(position.current_);
    auto& hook = object.*Hook;
    hook.next_ = position.current_->next_;
    position.current_->next_ = &hook;
    return iterator(&hook);
  }

  template<typename ForwardIt> iterator insert_after(iterator position, ForwardIt first, ForwardIt last) noexcept {
    PLEIONE_ASSERT(position.current_);
    auto prev = position.current_;
    auto after = position.current_->next_;
    using std::for_each;
    for_each(first, last, [&](T& object) {
      auto& hook = object.*Hook;
      prev->next_ = &hook;
      prev = &hook;
    });
    prev->next_ = after;
    return iterator(prev);
  }

  iterator erase_after(iterator position) noexcept {
    PLEIONE_ASSERT(position.current_);
    PLEIONE_ASSERT(root_.next_);
    auto after = position.current_->next_->next_;
    position.current_->next_ = after;
    return iterator(after);
  }

  iterator erase_after(iterator first, iterator last) noexcept {
    if (PLEIONE_UNLIKELY(first == last || std::next(first) == last)) { return last; }
    PLEIONE_ASSERT(first.current_);
    PLEIONE_ASSERT(root_.next_);
    first.current_->next_ = last.current_;
    return iterator(last.current_);
  }

  void push_front(T& object) noexcept {
    auto& hook = object.*Hook;
    hook.next_ = root_.next_;
    root_.next_ = &hook;
  }

  void pop_front() noexcept {
    PLEIONE_ASSERT(root_.next_);
    root_.next_ = root_.next_->next_;
  }

  void splice_after(iterator position, forward_list& other) noexcept {
    PLEIONE_ASSERT(position.current_);
    if (position.current_->next_) {
      auto other_it = other.before_begin();
      while (std::next(other_it) != other.end()) { other_it = std::next(other_it); }
      other_it.current_->next_ = position.current_->next_;
    }
    position.current_->next_ = other.root_.next_;
    other.root_.next_ = nullptr;
  }
  void splice_after(iterator position, forward_list&& other) noexcept {
    PLEIONE_ASSERT(position.current_);
    if (position.current_->next_) {
      auto other_it = other.before_begin();
      while (std::next(other_it) != other.end()) { other_it = std::next(other_it); }
      other_it.current_->next_ = position.current_->next_;
    }
    position.current_->next_ = other.root_.next_;
  }

  void splice_after(iterator position, forward_list& other, iterator element) noexcept {
    auto& object = *std::next(element);
    other.erase_after(element);
    insert_after(position, object);
  }
  void splice_after(iterator position, forward_list&&, iterator element) noexcept {
    insert_after(position, *std::next(element));
  }

  void splice_after(iterator position, forward_list&, iterator first, iterator last) noexcept {
    if (PLEIONE_UNLIKELY(first == last || std::next(first) == last)) { return; }
    auto first_element = first.current_->next_;
    auto last_element = first_element;
    while (last_element->next_ != last.current_) { last_element = last_element->next_; }
    last_element->next_ = position.current_->next_;
    position.current_->next_ = first_element;
    first.current_->next_ = last.current_;
  }
  void splice_after(iterator position, forward_list&&, iterator first, iterator last) noexcept {
    if (PLEIONE_UNLIKELY(first == last || std::next(first) == last)) { return; }
    auto first_element = first.current_->next_;
    auto last_element = first_element;
    while (last_element->next_ != last.current_) { last_element = last_element->next_; }
    last_element->next_ = position.current_->next_;
    position.current_->next_ = first_element;
  }

public:
  template<bool Prefetch, bool Constant, typename UnaryFunction>
  friend void for_each(prefetch<Prefetch>, basic_iterator<Constant> first, basic_iterator<Constant> last,
                       UnaryFunction&& fn) {
    while (first != last) {
      if constexpr (Prefetch) { first.prefetch_next(); }
      fn(*first++);
    }
  }
  template<bool Constant, typename UnaryFunction>
  friend void for_each(basic_iterator<Constant> first, basic_iterator<Constant> last, UnaryFunction&& fn) {
    for_each(prefetch<true>{}, first, last, std::forward<UnaryFunction>(fn));
  }
};

} // namespace intrusive

PLEIONE_NAMESPACE_END

#endif

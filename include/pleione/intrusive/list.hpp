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

#ifndef PLEIONE_INTRUSIVE_LIST_HPP
#define PLEIONE_INTRUSIVE_LIST_HPP

#include <algorithm>

#include "core.hpp"

#include "../detail/container_of.hpp"

PLEIONE_NAMESPACE_BEGIN

namespace intrusive {

class list_hook {
  list_hook* next_;
  list_hook* prev_;

private:
  list_hook(list_hook* prev, list_hook* next) noexcept : next_(next), prev_(prev) {}

  template<typename T, list_hook T::*> friend class list;

public:
  list_hook() = default;
  list_hook(list_hook const&) = delete;
  list_hook(list_hook&&) = delete;
};

template<typename T, list_hook T::*Hook> class list {
  list_hook root_ = {&root_, &root_};
  std::size_t size_ = 0;

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
    list_hook* current_ = nullptr;

  private:
    explicit basic_iterator(list_hook* hook) noexcept : current_(hook) {}

    friend class list;

  public:
    using value_type = std::conditional_t<Constant, T const, T>;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    basic_iterator() = default;

    operator basic_iterator<true>() const noexcept { return basic_iterator<true>(current_); }

    reference operator*() const noexcept { return detail::container_of<T, list_hook>(Hook, *current_); }
    pointer operator->() const noexcept { return &detail::container_of<T, list_hook>(Hook, *current_); }

    basic_iterator& operator++() noexcept {
      current_ = current_->next_;
      return *this;
    }
    basic_iterator operator++(int) noexcept {
      auto it = *this;
      operator++();
      return it;
    }

    basic_iterator& operator--() noexcept {
      current_ = current_->prev_;
      return *this;
    }
    basic_iterator operator--(int) noexcept {
      auto it = *this;
      operator--();
      return it;
    }

    bool operator==(basic_iterator const& other) const noexcept { return current_ == other.current_; }
    bool operator!=(basic_iterator const& other) const noexcept { return !(*this == other); }
  };

public:
  using iterator = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
  list() = default;

  template<typename ForwardIt> list(ForwardIt first, ForwardIt last) noexcept {
    static_assert(
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>);
    assign(first, last);
  }

  list(list const&) = delete;
  list(list&& other) noexcept : size_(other.size_) {
    if (PLEIONE_UNLIKELY(!other.empty())) {
      root_.next_ = other.root_.next_;
      root_.prev_ = other.root_.prev_;
      root_.next_->prev_ = &root_;
      root_.prev_->next_ = &root_;
    } else {
      root_.next_ = &root_;
      root_.prev_ = &root_;
    }
  }

  list& operator=(list const&) = delete;
  list& operator=(list&& other) noexcept {
    size_ = other.size_;
    if (PLEIONE_UNLIKELY(!other.empty())) {
      root_.next_ = other.root_.next_;
      root_.prev_ = other.root_.prev_;
      root_.next_->prev_ = &root_;
      root_.prev_->next_ = &root_;
    } else {
      root_.next_ = &root_;
      root_.prev_ = &root_;
    }
    return *this;
  }

  template<typename ForwardIt> void assign(ForwardIt first, ForwardIt last) noexcept {
    static_assert(
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>);
    size_ = 0;
    auto prev = &root_;
    using std::for_each;
    for_each(first, last, [&](T& object) {
      auto& hook = object.*Hook;
      hook.prev_ = prev;
      prev->next_ = &hook;
      prev = &hook;
      size_++;
    });
    root_.prev_ = prev;
    prev->next_ = &root_;
  }

  T& front() noexcept {
    PLEIONE_ASSERT(size_);
    return detail::container_of<T, list_hook>(Hook, *root_.next_);
  }
  T const& front() const noexcept {
    PLEIONE_ASSERT(size_);
    return detail::container_of<T, list_hook>(Hook, *root_.next_);
  }

  T& back() noexcept {
    PLEIONE_ASSERT(size_);
    return detail::container_of<T, list_hook>(Hook, *root_.prev_);
  }
  T const& back() const noexcept {
    PLEIONE_ASSERT(size_);
    return detail::container_of<T, list_hook>(Hook, *root_.prev_);
  }

  iterator begin() noexcept { return iterator(root_.next_); }
  const_iterator begin() const noexcept { return const_iterator(root_.next_); }
  iterator end() noexcept { return iterator(&root_); }
  const_iterator end() const noexcept { return const_iterator(const_cast<list_hook*>(&root_)); }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

  bool empty() const noexcept { return size_ == 0; }

  size_type size() const noexcept { return size_; }

  void clear() noexcept {
    root_.next_ = &root_;
    root_.prev_ = &root_;
    size_ = 0;
  }

  iterator insert(const_iterator position, T& object) noexcept {
    auto& hook = object.*Hook;
    hook.next_ = position.current_;
    hook.prev_ = position.current_->prev_;
    position.current_->prev_->next_ = &hook;
    position.current_->prev_ = &hook;
    ++size_;
    return iterator(&hook);
  }

  template<typename ForwardIt> iterator insert(const_iterator position, ForwardIt first, ForwardIt last) noexcept {
    static_assert(
        std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>);
    if (PLEIONE_UNLIKELY(first == last)) { return iterator(position.current_); }
    auto after = position.current_;
    auto prev = after->prev_;
    auto ret = after->prev_;
    using std::for_each;
    for_each(first, last, [&](T& object) {
      auto& hook = object.*Hook;
      hook.prev_ = prev;
      prev->next_ = &hook;
      prev = &hook;
      ++size_;
    });
    after->prev_ = prev;
    prev->next_ = after;
    return iterator(ret->next_);
  }

  iterator erase(const_iterator position) noexcept {
    auto& hook = *position.current_;
    hook.prev_->next_ = hook.next_;
    hook.next_->prev_ = hook.prev_;
    --size_;
    return iterator(hook.next_);
  }

  iterator erase(const_iterator first, const_iterator last) noexcept {
    first.current_->prev_->next_ = last.current_;
    last.current_->prev_ = first.current_->prev_;
    size_ -= std::distance(first, last);
    return iterator(last.current_);
  }

  void push_front(T& object) noexcept {
    auto& hook = object.*Hook;
    hook.prev_ = &root_;
    root_.next_->prev_ = &hook;
    hook.next_ = root_.next_;
    root_.next_ = &hook;
    size_++;
  }

  void push_back(T& object) noexcept {
    auto& hook = object.*Hook;
    hook.next_ = &root_;
    root_.prev_->next_ = &hook;
    hook.prev_ = root_.prev_;
    root_.prev_ = &hook;
    size_++;
  }

  void pop_front() noexcept {
    PLEIONE_ASSERT(size_);
    root_.next_ = root_.next_->next_;
    root_.next_->prev_ = &root_;
    size_--;
  }

  void pop_back() noexcept {
    PLEIONE_ASSERT(size_);
    root_.prev_ = root_.prev_->prev_;
    root_.prev_->next_ = &root_;
    size_--;
  }

  void splice(const_iterator position, list& other) noexcept {
    if (PLEIONE_UNLIKELY(other.empty())) { return; }
    auto after = position.current_;
    other.root_.prev_->next_ = after;
    after->prev_->next_ = other.root_.next_;
    other.root_.next_->prev_ = after->prev_;
    after->prev_ = other.root_.prev_;
    size_ += other.size_;
    other.root_.next_ = &other.root_;
    other.root_.prev_ = &other.root_;
    other.size_ = 0;
  }
  void splice(const_iterator position, list&& other) noexcept {
    if (PLEIONE_UNLIKELY(other.empty())) { return; }
    auto after = position.current_;
    other.root_.prev_->next_ = after;
    after->prev_->next_ = other.root_.next_;
    other.root_.next_->prev_ = after->prev_;
    after->prev_ = other.root_.prev_;
    size_ += other.size_;
  }

  void splice(const_iterator position, list& other, const_iterator element) noexcept {
    auto& object = const_cast<T&>(*element);
    other.erase(element);
    insert(position, object);
  }
  void splice(const_iterator position, list&&, const_iterator element) noexcept {
    insert(position, const_cast<T&>(*element));
  }

  void splice(const_iterator position, list& other, const_iterator first, const_iterator last) noexcept {
    if (PLEIONE_UNLIKELY(first == last)) { return; }
    auto n = std::distance(first, last);
    auto other_before = first.current_->prev_;
    auto other_after = last.current_;
    auto last_prev = other_after->prev_;
    other_before->next_ = other_after;
    other_after->prev_ = other_before;
    other.size_ -= n;
    auto after = position.current_;
    auto before = after->prev_;
    before->next_ = first.current_;
    first.current_->prev_ = before;
    after->prev_ = last_prev;
    last_prev->next_ = after;
    size_ += n;
  }
  void splice(const_iterator position, list&&, const_iterator first, const_iterator last) noexcept {
    if (PLEIONE_UNLIKELY(first == last)) { return; }
    auto n = std::distance(first, last);
    auto after = position.current_;
    auto before = after->prev_;
    before->next_ = first.current_;
    first.current_->prev_ = before;
    after->prev_ = last.current_->prev_;
    last.current_->prev_->next_ = after;
    size_ += n;
  }
};

} // namespace intrusive

PLEIONE_NAMESPACE_END

#endif
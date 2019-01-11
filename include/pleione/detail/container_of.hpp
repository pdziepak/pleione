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

#ifndef PLEIONE_DETAIL_CONTAINER_OF_HPP
#define PLEIONE_DETAIL_CONTAINER_OF_HPP

#include <cstddef>
#include <type_traits>

#include "config.hpp"

PLEIONE_NAMESPACE_BEGIN

namespace detail {

/// \brief Returns offset of a member in an object.
///
/// \note Unlike the `offsetof` macro provided by the standard this function
/// works with pointers to members and non-standard layout types. However, it
/// needs to take advantage of the ABI properties which may cause portability
/// problems.
///
/// \tparam Structure type of the object
/// \tparam MemberType type of the member object
/// \param pointer pointer to the member
/// \returns offset from the beginning of the object to the member
template<typename Structure, typename MemberType> ptrdiff_t offset_of(MemberType Structure::*pointer) noexcept {
#if (defined(__linux__) || defined(__APPLE__)) && defined(__x86_64__)
  using member_pointer_value_type = ptrdiff_t;
#elif defined(_WIN32) && (defined(_M_X64) || defined(_M_IX86))
  using member_pointer_value_type = int32_t;
#else
#error Unknown ABI
#endif
  static_assert(sizeof(pointer) == sizeof(member_pointer_value_type));
  union {
    MemberType Structure::*ptr;
    member_pointer_value_type off;
  };
  ptr = pointer;
  return off;
}

/// \brief Returns reference to a object containing given one
///
/// For a member object this function takes pointer to member and a reference to
/// that object and returns a reference to the object containing it.
///
/// \warning If the `member` object is not a member of an object of a
/// `Structure` type the behaviour is undefined.
///
/// \tparam Structure type of the container object
/// \tparam MemberType type of the member object
/// \param member_pointer pointer to the member
/// \param member reference to the member object
/// \returns reference to the container object
template<typename Structure, typename MemberType>
Structure& container_of(MemberType Structure::*member_pointer, MemberType& member) noexcept {
  auto ptr = reinterpret_cast<char*>(const_cast<std::remove_const_t<MemberType>*>(&member));
  ptr -= offset_of(member_pointer);
  return *reinterpret_cast<Structure*>(ptr);
}

} // namespace detail

PLEIONE_NAMESPACE_END

#endif

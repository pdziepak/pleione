#
# Copyright © 2018 Paweł Dziepak
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.12)

project(pleione
  VERSION 0.0.0
  LANGUAGES CXX
)

include(CTest)
include(GNUInstallDirs)

option(PLEIONE_DOC "Build pleione documentation." ON)
option(PLEIONE_DOC_INTERNAL "Build pleione internal documentation." OFF)

option(PLEIONE_PERF "Build microbenchmarks." OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  list(APPEND PLEIONE_CXX_FLAGS -Wall -Wextra -Wpedantic -Werror)
  list(APPEND PLEIONE_CXX_DEBUG_FLAGS -fsanitize=address -fsanitize=undefined -DPLEIONE_DEBUG=1)
  list(APPEND PLEIONE_LINK_DEBUG_FLAGS -fsanitize=address -fsanitize=undefined)
  list(APPEND PLEIONE_CXX_RELEASE_FLAGS -O3)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  list(APPEND PLEIONE_CXX_FLAGS -W3 -WX -EHsc)
  list(APPEND PLEIONE_CXX_DEBUG_FLAGS -DPLEIONE_DEBUG=1)
  list(APPEND PLEIONE_CXX_RELEASE_FLAGS -O2)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  list(APPEND PLEIONE_CXX_FLAGS ${PLEIONE_CXX_DEBUG_FLAGS})
  list(APPEND PLEIONE_LINK_FLAGS ${PLEIONE_LINK_DEBUG_FLAGS})
else()
  list(APPEND PLEIONE_CXX_FLAGS ${PLEIONE_CXX_RELEASE_FLAGS})
  list(APPEND PLEIONE_LINK_FLAGS ${PLEIONE_LINK_RELEASE_FLAGS})
endif()

add_library(pleione INTERFACE)
target_include_directories(pleione INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>)
target_include_directories(pleione SYSTEM INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(pleione INTERFACE cxx_std_17)

file(GLOB_RECURSE PLEIONE_PUBLIC_HEADERS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include
  include/pleione/*.hpp
)

if(PLEIONE_DOC)
  add_subdirectory(doc)
endif()

if(BUILD_TESTING AND PLEIONE_PERF)
  add_subdirectory(perf)
endif()

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/pleione-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY ExactVersion
)

install(TARGETS pleione EXPORT pleione-targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT pleione-targets FILE pleione-config.cmake NAMESPACE pleione:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pleione)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pleione-config-version.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pleione)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

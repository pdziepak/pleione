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

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(CTest)
include(GNUInstallDirs)

option(PLEIONE_DOC "Build pleione documentation." ON)
option(PLEIONE_DOC_INTERNAL "Build pleione internal documentation." OFF)

option(PLEIONE_PERF "Build microbenchmarks." OFF)

list(APPEND PLEIONE_CXX_FLAGS -Wall -Wextra -Wpedantic -Werror)
list(APPEND PLEIONE_CXX_DEBUG_FLAGS -fsanitize=address -fsanitize=undefined)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  list(APPEND PLEIONE_CXX_FLAGS ${PLEIONE_CXX_DEBUG_FLAGS})
endif()

add_library(pleione INTERFACE)
target_include_directories(pleione INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>)
target_include_directories(pleione SYSTEM INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)

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

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

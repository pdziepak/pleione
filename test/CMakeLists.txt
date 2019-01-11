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

find_package(GTest REQUIRED)

function(pleione_add_test TESTNAME SOURCE)
  add_executable(${TESTNAME} ${SOURCE} ${ARGN})
  target_link_libraries(${TESTNAME} pleione GTest::GTest GTest::Main ${PLEIONE_LINK_FLAGS})
  target_compile_options(${TESTNAME} PRIVATE ${PLEIONE_CXX_FLAGS})
  add_test(${TESTNAME} ${TESTNAME})
endfunction(pleione_add_test)

foreach(HEADER_FILE ${PLEIONE_PUBLIC_HEADERS})
  get_filename_component(SOURCE_FILE "${HEADER_FILE}" NAME_WE)
  get_filename_component(DIRECTORY "${HEADER_FILE}" DIRECTORY)
  set(SOURCE_FILE "${DIRECTORY}/${SOURCE_FILE}.cpp")

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/include_all_dir/${SOURCE_FILE}" "#include <${HEADER_FILE}>\n#include <${HEADER_FILE}>")
  list(APPEND PLEIONE_TEST_INCLUDE_ALL_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/include_all_dir/${SOURCE_FILE}")
endforeach()

pleione_add_test(include_all include_all.cpp ${PLEIONE_TEST_INCLUDE_ALL_SOURCES})

add_subdirectory(detail)
add_subdirectory(intrusive)

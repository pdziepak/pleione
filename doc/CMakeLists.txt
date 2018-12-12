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

find_package(Doxygen REQUIRED dot)

set(PLEIONE_DOC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

set(DOXYGEN_EXCLUDE_SYMBOLS "test;detail")
set(DOXYGEN_EXCLUDE_PATTERNS "test;detail")
set(DOXYGEN_STRIP_FROM_PATH "include")

set(DOXYGEN_STRIP_FROM_INC_PATH "include")
set(DOXYGEN_PREDEFINED "PLEIONE_NAMESPACE_BEGIN=namespace pleione {;PLEIONE_NAMESPACE_END=}")
set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
set(DOXYGEN_FULL_PATH_NAMES YES)

doxygen_add_docs(
  doc
  WORKING_DIRECTORY ${PLEIONE_DOC_SOURCE_DIR}
)

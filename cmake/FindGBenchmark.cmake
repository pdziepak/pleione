# Try to find GBenchmark
#
# Imported targets:
#  GBenchmark::GBenchmark
#  GBenchmark::Main
#
# Result variables:
#  GBenchmark_FOUND
#  GBenchmark_INCLUDE_DIRS
#  GBenchmark_LIBRARIES
#  GBenchmark_MAIN_LIBRARIES

find_path(GBenchmark_INCLUDE_DIR
  NAMES "benchmark/benchmark.h"
  PATH_SUFFIXES include
)
find_library(GBenchmark_LIBRARY
  NAMES "benchmark"
  PATH_SUFFIXES lib lib64
)
find_library(GBenchmark_MAIN_LIBRARY
  NAMES "benchmark_main"
  PATH_SUFFIXES lib lib64
)

mark_as_advanced(GBenchmark_FOUND GBenchmark_INCLUDE_DIR GBenchmark_LIBRARY GBenchmark_MAIN_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GBenchmark FOUND_VAR GBenchmark_FOUND
  REQUIRED_VARS GBenchmark_LIBRARY GBenchmark_MAIN_LIBRARY GBenchmark_INCLUDE_DIR
)

if(GBenchmark_FOUND)
  set(GBenchmark_LIBRARIES ${GBenchmark_LIBRARY})
  set(GBenchmark_MAIN_LIBRARIES ${GBenchmark_MAIN_LIBRARY})
  set(GBenchmark_INCLUDE_DIRS ${GBenchmark_INCLUDE_DIR})

  if(NOT TARGET GBenchmark::GBenchmark)
    find_package(Threads QUIET REQUIRED)

    add_library(GBenchmark::GBenchmark UNKNOWN IMPORTED)
    if(TARGET Threads::Threads)
      set_target_properties(GBenchmark::GBenchmark PROPERTIES
        INTERFACE_LINK_LIBRARIES Threads::Threads)
    endif()
    set_target_properties(GBenchmark::GBenchmark PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${GBenchmark_INCLUDE_DIR}")
    set_target_properties(GBenchmark::GBenchmark PROPERTIES
      IMPORTED_LOCATION "${GBenchmark_LIBRARY}")
  endif()

  if(NOT TARGET GBenchmark::Main)
  add_library(GBenchmark::Main UNKNOWN IMPORTED)
  set_target_properties(GBenchmark::Main PROPERTIES
    INTERFACE_LINK_LIBRARIES GBenchmark::GBenchmark)
  set_target_properties(GBenchmark::Main PROPERTIES
    IMPORTED_LOCATION "${GBenchmark_MAIN_LIBRARY}")
  endif()
endif()

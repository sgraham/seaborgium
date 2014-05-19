project(sg CXX)
cmake_minimum_required(VERSION 2.8)

macro(fix_default_compiler_settings)
  if (MSVC)
    foreach (flag_var
             CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
             CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if (NOT BUILD_SHARED_LIBS AND NOT gtest_force_shared_crt)
        # Always MT.
        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()

      # Always W4.
      string(REPLACE "/W3" "/W4" ${flag_var} "${${flag_var}}")

      # No exceptions.
      string(REPLACE "/EHsc" "/wd4530" ${flag_var} "${${flag_var}}")
    endforeach()

    # Warnings as errors.
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

    # Full path in error messages.
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")

    add_definitions(-DNOMINMAX)
  endif()
endmacro()

fix_default_compiler_settings()


add_subdirectory(third_party/gtest-1.7.0)

include_directories(src)

# Main code library.
set(main_sources
  src/core/gfx.cc
  src/ui/dockable.cc
  src/ui/docking_resizer.cc
  src/ui/docking_split_container.cc
  src/ui/docking_workspace.cc
)
add_library(sglib ${main_sources})


# Main binary.
add_executable(sg
  src/core/entry.cc
  src/main.cc)
target_link_libraries(sg sglib)


# Test binary
include_directories(${gtest_SOURCE_DIR}/include)
set(test_sources
  src/core/rect_pack_test.cc
  src/ui/docking_test.cc
)
add_executable(sg_test ${test_sources})
target_link_libraries(sg_test sglib gtest_main)


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
  endif()
endmacro()

fix_default_compiler_settings()


add_subdirectory(third_party/gtest-1.7.0)

# Main binary
include_directories(src)
set(main_sources
  src/core/entry.cc
  src/core/gfx.cc
  src/main.cc
)
add_executable(sg ${main_sources})


# Test binary
include_directories(${gtest_SOURCE_DIR}/include)
set(test_sources
  src/core/rect_pack_test.cc
)
add_executable(sg_test ${test_sources})
target_link_libraries(sg_test gtest_main)


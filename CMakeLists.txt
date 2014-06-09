project(sg CXX C)
cmake_minimum_required(VERSION 2.8)

macro(fix_default_compiler_settings)
  if (MSVC)
    foreach (flag_var
             CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
             CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
             CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
             CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
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
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX")

    # Full path in error messages.
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FC")

    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
  endif()
endmacro()

fix_default_compiler_settings()


add_subdirectory(third_party/gtest-1.7.0)

add_definitions(-DGLEW_STATIC)
include_directories(src
                    third_party/nanovg/src
                    third_party/glew-1.10.0/include
                    third_party/re2
                   )
# TODO
#include_directories(../llvm/llvm/tools/clang/include)

# Main code library.
set(main_sources
  src/core/gfx.cc

  src/ui/dockable.cc
  src/ui/docking_resizer.cc
  src/ui/docking_split_container.cc
  src/ui/docking_tool_window.cc
  src/ui/docking_workspace.cc
  src/ui/drawing_common.cc
  src/ui/focus.cc
  src/ui/skin.cc
  src/ui/text_edit.cc
  src/ui/tool_window_dragger.cc
  src/ui/tree_grid.cc

  src/ui/source_view/cpp_lexer.cc
  src/ui/source_view/lexer.cc
  src/ui/source_view/lexer_state.cc

  third_party/glew-1.10.0/src/glew.c
  third_party/nanovg/src/nanovg.c
  third_party/nanovg/src/stb_image.c

  third_party/re2/re2/bitstate.cc
  third_party/re2/re2/compile.cc
  third_party/re2/re2/dfa.cc
  third_party/re2/re2/filtered_re2.cc
  third_party/re2/re2/filtered_re2.h
  third_party/re2/re2/mimics_pcre.cc
  third_party/re2/re2/nfa.cc
  third_party/re2/re2/onepass.cc
  third_party/re2/re2/parse.cc
  third_party/re2/re2/perl_groups.cc
  third_party/re2/re2/prefilter.cc
  third_party/re2/re2/prefilter.h
  third_party/re2/re2/prefilter_tree.cc
  third_party/re2/re2/prefilter_tree.h
  third_party/re2/re2/prog.cc
  third_party/re2/re2/prog.h
  third_party/re2/re2/re2.cc
  third_party/re2/re2/re2.h
  third_party/re2/re2/regexp.cc
  third_party/re2/re2/regexp.h
  third_party/re2/re2/set.cc
  third_party/re2/re2/set.h
  third_party/re2/re2/simplify.cc
  third_party/re2/re2/stringpiece.h
  third_party/re2/re2/tostring.cc
  third_party/re2/re2/unicode_casefold.cc
  third_party/re2/re2/unicode_casefold.h
  third_party/re2/re2/unicode_groups.cc
  third_party/re2/re2/unicode_groups.h
  third_party/re2/re2/variadic_function.h
  third_party/re2/re2/walker-inl.h
  third_party/re2/util/arena.cc
  third_party/re2/util/arena.h
  third_party/re2/util/atomicops.h
  third_party/re2/util/flags.h
  third_party/re2/util/hash.cc
  third_party/re2/util/logging.h
  third_party/re2/util/mutex.h
  third_party/re2/util/rune.cc
  third_party/re2/util/sparse_array.h
  third_party/re2/util/sparse_set.h
  third_party/re2/util/stringpiece.cc
  third_party/re2/util/stringprintf.cc
  third_party/re2/util/strutil.cc
  third_party/re2/util/utf.h
  third_party/re2/util/util.h
)
add_library(sglib ${main_sources})


# Main binary.
add_executable(sg
  src/core/entry.cc
  src/main.cc)


if (WIN32)
  set(CMAKE_SHARED_LINKER_FLAGS /MANIFEST:NO)
endif()

add_custom_command(
  TARGET
    sg
  POST_BUILD
    COMMAND
      "mt.exe" -nologo -manifest \"${CMAKE_CURRENT_SOURCE_DIR}/src/core/dpiaware.manifest\" -outputresource:\"${CMAKE_CURRENT_BINARY_DIR}/sg.exe\"\;\#1
  )

target_link_libraries(sg
                      sglib
                      #d:/src/llvm/build/lib/libclang.imp
                      )


# Test binary
include_directories(${gtest_SOURCE_DIR}/include)
set(test_sources
  src/ui/test_stubs.cc
  src/ui/docking_test.cc
  src/ui/source_view/lexer_test.cc
)
add_executable(sg_test ${test_sources})
target_link_libraries(sg_test sglib gtest_main)


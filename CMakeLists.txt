# SPDX-License-Identifier: MIT
# Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

# CProcessing - cross-platform CMake build (Windows, Linux, macOS)
#
# This sits alongside the Visual Studio solution (CProcessing.sln), it
# does not replace it: the .sln/.vcxproj workflow students and faculty use
# today is unchanged. See BUILDING.md for usage.
#
#   cmake -B build                  # configure (downloads GLFW + SoLoud, pinned)
#   cmake --build build             # library, demos, examples and tests
#   ctest --test-dir build          # run the test suites
#
# Everything is placed in build/bin (build/bin/<Config> for multi-config
# generators such as Visual Studio): the CProcessing shared library, the
# CProcessing_Demos executable, the Example_* executables, the test
# executables and an assets folder.

cmake_minimum_required(VERSION 3.20)

# The version is defined once, in cprocessing_common.h
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/CProcessing/inc/cprocessing_common.h"
    _cp_version_lines REGEX "^#define CP_VERSION_(MAJOR|MINOR|PATCH) +[0-9]+")
foreach(_part MAJOR MINOR PATCH)
    string(REGEX MATCH "CP_VERSION_${_part} +([0-9]+)" _ "${_cp_version_lines}")
    set(_cp_version_${_part} "${CMAKE_MATCH_1}")
endforeach()

project(CProcessing
    VERSION ${_cp_version_MAJOR}.${_cp_version_MINOR}.${_cp_version_PATCH}
    DESCRIPTION "A Processing-inspired C library for learning game programming"
    LANGUAGES C CXX
)

set(CPROCESSING_IS_TOP_LEVEL OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CPROCESSING_IS_TOP_LEVEL ON)
endif()

option(CPROCESSING_BUILD_SHARED "Build CProcessing as a shared library (DLL/.so/.dylib)" ON)
option(CPROCESSING_BUILD_DEMOS "Build the CProcessing_Demos sample program" ${CPROCESSING_IS_TOP_LEVEL})
option(CPROCESSING_BUILD_EXAMPLES "Build the example programs in examples/" ${CPROCESSING_IS_TOP_LEVEL})
option(CPROCESSING_BUILD_TESTS "Build the Tier 1 and Tier 2 test suites" ${CPROCESSING_IS_TOP_LEVEL})
option(CPROCESSING_WARNINGS_AS_ERRORS "Treat compiler warnings in CProcessing code as errors" OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Put every runtime artifact in one folder so executables find the library
# (and their assets) without any install step.
if(CPROCESSING_IS_TOP_LEVEL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cmake/cprocessing_dependencies.cmake)

#------------------------------------------------------------------------------
# Shared compile settings for CProcessing's own code
#------------------------------------------------------------------------------
# Read source files as UTF-8. MSVC otherwise uses the computer's code page,
# and text such as "©" in a string isn't passed on as UTF-8, which
# CP_Font_DrawText expects. GCC and Clang read UTF-8 already.
function(cprocessing_set_utf8 target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8)
    endif()
endfunction()

function(cprocessing_set_warnings target)
    cprocessing_set_utf8(${target})
    if(MSVC)
        # C is built as C11 (as /std:c11, MSVC's default is an older dialect
        # with Microsoft extensions); C++ files keep the C++ default
        target_compile_options(${target} PRIVATE /W4 $<$<COMPILE_LANGUAGE:C>:/std:c11>)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
        if(CPROCESSING_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra)
        if(CPROCESSING_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
    if(NOT MSVC)
        # C is C11 (gnu11 keeps the POSIX functions the library uses);
        # Visual Studio gets /std:c11 above
        set_target_properties(${target} PROPERTIES C_STANDARD 11 C_EXTENSIONS ON)
    endif()
endfunction()

# Strict warnings for code built as C++, to check that the public headers,
# and the examples students copy from, work from C++ as well as C.
# -Wpedantic and /permissive- reject what isn't standard C++.
function(cprocessing_set_cxx_warnings target)
    cprocessing_set_utf8(${target})
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
        if(CPROCESSING_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(CPROCESSING_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
    # The oldest C++ a student is likely to use (MSVC starts at C++14)
    set_target_properties(${target} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
endfunction()

# Let executables find the shared library sitting next to them.
function(cprocessing_set_rpath target)
    if(APPLE)
        set_target_properties(${target} PROPERTIES BUILD_RPATH "@loader_path" INSTALL_RPATH "@loader_path")
    elseif(UNIX)
        set_target_properties(${target} PROPERTIES BUILD_RPATH "$ORIGIN" INSTALL_RPATH "$ORIGIN")
    endif()
endfunction()

# Copy an assets folder next to the built executables.
function(cprocessing_copy_assets target assets_dir)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${assets_dir}" "$<TARGET_FILE_DIR:${target}>/assets"
        COMMENT "Copying ${assets_dir} next to ${target}"
        VERBATIM)
endfunction()

#------------------------------------------------------------------------------
# The CProcessing library
#------------------------------------------------------------------------------
set(CP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CProcessing")

set(CPROCESSING_SOURCES
    "${CP_DIR}/third_party/glad/glad.c"
    "${CP_DIR}/third_party/nanovg/src/nanovg.c"
    "${CP_DIR}/src/CP_Color.c"
    "${CP_DIR}/src/CP_File.c"
    "${CP_DIR}/src/CP_Graphics.c"
    "${CP_DIR}/src/CP_Image.c"
    "${CP_DIR}/src/CP_Input.c"
    "${CP_DIR}/src/CP_Math.c"
    "${CP_DIR}/src/CP_Noise.c"
    "${CP_DIR}/src/CP_Random.c"
    "${CP_DIR}/src/CP_Setting.c"
    "${CP_DIR}/src/CP_Sound.c"
    "${CP_DIR}/src/CP_System.c"
    "${CP_DIR}/src/CP_Text.c"
    "${CP_DIR}/third_party/nanovg/nanovg_gl3.c"
    "${CP_DIR}/third_party/stb/stb_image_write.c"
    "${CP_DIR}/third_party/vect/vect.c"
)
# Vendored third-party sources (NanoVG and its OpenGL 3 back end with the
# bundled stb_image/stb_truetype, stb_image_write, GLAD) are compiled as-is;
# don't report their warnings. The Visual Studio project does the same.
set(CPROCESSING_VENDORED_SOURCES
    "${CP_DIR}/third_party/glad/glad.c"
    "${CP_DIR}/third_party/nanovg/src/nanovg.c"
    "${CP_DIR}/third_party/nanovg/nanovg_gl3.c"
    "${CP_DIR}/third_party/stb/stb_image_write.c"
)
if(MSVC)
    set_source_files_properties(${CPROCESSING_VENDORED_SOURCES} PROPERTIES COMPILE_OPTIONS "/W0")
else()
    set_source_files_properties(${CPROCESSING_VENDORED_SOURCES} PROPERTIES COMPILE_OPTIONS "-w")
endif()

if(CPROCESSING_BUILD_SHARED)
    add_library(CProcessing SHARED ${CPROCESSING_SOURCES})
    target_compile_definitions(CProcessing PRIVATE CPROCESSING_EXPORTS)
else()
    add_library(CProcessing STATIC ${CPROCESSING_SOURCES})
    target_compile_definitions(CProcessing PUBLIC CPROCESSING_STATIC)
endif()
add_library(CProcessing::CProcessing ALIAS CProcessing)

target_include_directories(CProcessing
    PUBLIC
        "$<BUILD_INTERFACE:${CP_DIR}/inc>"
    PRIVATE
        "${CP_DIR}/src"
        "${CP_DIR}/third_party/glad"
        "${CP_DIR}/third_party/nanovg/src"
        "${CP_DIR}/third_party/vect"
        "${CPROCESSING_GLFW_INCLUDE_DIR}"
)

if(UNIX AND NOT APPLE AND GLFW_BUILD_X11)
    # Enables CP_System_GetWindowHandle on X11 (glfwGetX11Window)
    target_compile_definitions(CProcessing PRIVATE CP_HAS_X11)
endif()

target_link_libraries(CProcessing PRIVATE glfw soloud Threads::Threads ${CMAKE_DL_LIBS})
if(UNIX AND NOT APPLE)
    target_link_libraries(CProcessing PRIVATE m)
endif()

set_target_properties(CProcessing PROPERTIES
    DEBUG_POSTFIX "d"                 # CProcessingd, matching the Visual Studio project
    C_VISIBILITY_PRESET hidden        # export only CP_API functions, like the Windows DLL
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
cprocessing_set_warnings(CProcessing)

#------------------------------------------------------------------------------
# Demos
#------------------------------------------------------------------------------
if(CPROCESSING_BUILD_DEMOS)
    set(DEMO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/demos")
    add_executable(CProcessing_Demos
        "${DEMO_DIR}/main.c"
        "${DEMO_DIR}/dancing_lines_demo.c"
        "${DEMO_DIR}/demo_manager.c"
        "${DEMO_DIR}/gamepad_sound_demo.c"
        "${DEMO_DIR}/grid_wave_demo.c"
        "${DEMO_DIR}/noise_demo.c"
        "${DEMO_DIR}/screenshot_demo.c"
    )
    target_include_directories(CProcessing_Demos PRIVATE "${DEMO_DIR}")
    target_link_libraries(CProcessing_Demos PRIVATE CProcessing)
    if(UNIX AND NOT APPLE)
        target_link_libraries(CProcessing_Demos PRIVATE m)
    endif()
    cprocessing_set_warnings(CProcessing_Demos)
    cprocessing_set_rpath(CProcessing_Demos)
    cprocessing_copy_assets(CProcessing_Demos "${DEMO_DIR}/assets")
endif()

#------------------------------------------------------------------------------
# Tests
#------------------------------------------------------------------------------
if(CPROCESSING_BUILD_TESTS)
    enable_testing()
    set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/tier1")
    set(TIER2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/tier2")
    set(UNITY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/unity")

    # Tier 1: pure functions, no window/GL/audio needed
    file(GLOB TIER1_SOURCES CONFIGURE_DEPENDS "${TEST_DIR}/*.c")
    add_executable(CProcessing_Tests ${TIER1_SOURCES} "${UNITY_DIR}/unity.c")
    target_include_directories(CProcessing_Tests PRIVATE
        "${UNITY_DIR}"
        "${TEST_DIR}"
        "${CP_DIR}/src"               # Internal_InputLogic.h (pure, header-only)
        "${CPROCESSING_GLFW_INCLUDE_DIR}" # GLFW constants used by that header
    )
    target_compile_definitions(CProcessing_Tests PRIVATE UNITY_INCLUDE_DOUBLE)
    cprocessing_set_utf8(CProcessing_Tests)
    target_link_libraries(CProcessing_Tests PRIVATE CProcessing)
    if(UNIX AND NOT APPLE)
        target_link_libraries(CProcessing_Tests PRIVATE m)
    endif()
    cprocessing_set_rpath(CProcessing_Tests)
    if(NOT MSVC)
        set_target_properties(CProcessing_Tests PROPERTIES C_STANDARD 11 C_EXTENSIONS ON)
    endif()
    add_test(NAME tier1 COMMAND CProcessing_Tests)
    set_tests_properties(tier1 PROPERTIES LABELS "tier1")

    # Tier 2: drives the real engine (window + OpenGL + audio). Needs a
    # display; on a headless Linux machine run it under xvfb-run.
    file(GLOB TIER2_SOURCES CONFIGURE_DEPENDS "${TIER2_DIR}/*.c")
    add_executable(CProcessing_Tests_Tier2 ${TIER2_SOURCES} "${UNITY_DIR}/unity.c")
    target_include_directories(CProcessing_Tests_Tier2 PRIVATE "${UNITY_DIR}" "${TIER2_DIR}")
    target_compile_definitions(CProcessing_Tests_Tier2 PRIVATE UNITY_INCLUDE_DOUBLE)
    cprocessing_set_utf8(CProcessing_Tests_Tier2)
    target_link_libraries(CProcessing_Tests_Tier2 PRIVATE CProcessing)
    if(UNIX AND NOT APPLE)
        target_link_libraries(CProcessing_Tests_Tier2 PRIVATE m)
    endif()
    cprocessing_set_rpath(CProcessing_Tests_Tier2)
    if(NOT MSVC)
        set_target_properties(CProcessing_Tests_Tier2 PROPERTIES C_STANDARD 11 C_EXTENSIONS ON)
    endif()
    cprocessing_copy_assets(CProcessing_Tests_Tier2 "${TIER2_DIR}/assets")
    if(TARGET CProcessing_Demos)
        # Both copy an assets folder into the same bin/assets after linking;
        # in a parallel build the two copies can collide and fail the build
        add_dependencies(CProcessing_Tests_Tier2 CProcessing_Demos)
    endif()
    add_test(NAME tier2 COMMAND CProcessing_Tests_Tier2
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:CProcessing_Tests_Tier2>")
    set_tests_properties(tier2 PROPERTIES LABELS "tier2;gui" TIMEOUT 120)

    # C++: a program written in C++ that includes the headers, links with
    # CProcessing and calls it. No window, so it runs with Tier 1.
    add_executable(CProcessing_Tests_Cxx "${TEST_DIR}/cxx/test_cxx.cpp")
    target_link_libraries(CProcessing_Tests_Cxx PRIVATE CProcessing)
    cprocessing_set_cxx_warnings(CProcessing_Tests_Cxx)
    cprocessing_set_rpath(CProcessing_Tests_Cxx)
    add_test(NAME cxx COMMAND CProcessing_Tests_Cxx)
    set_tests_properties(cxx PROPERTIES LABELS "tier1;cxx")

    # Every test function written is registered (a forgotten RUN_TEST would
    # silently drop the test). Needs no window, so it runs with Tier 1.
    add_test(NAME registration_tier1 COMMAND ${CMAKE_COMMAND}
        -DTESTS_DIR=${TEST_DIR} -DMAIN_FILE=${TEST_DIR}/test_main.c
        -DHEADER=${TEST_DIR}/test_runner.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_test_registration.cmake)
    add_test(NAME registration_tier2 COMMAND ${CMAKE_COMMAND}
        -DTESTS_DIR=${TIER2_DIR} -DMAIN_FILE=${TIER2_DIR}/tier2_main.c
        -DHEADER=${TIER2_DIR}/test_runner_tier2.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_test_registration.cmake)
    set_tests_properties(registration_tier1 registration_tier2 PROPERTIES LABELS "tier1;registration")
endif()

#------------------------------------------------------------------------------
# Examples: one program per file in examples/, named Example_<file>, and one
# per folder in examples/lessons/<folder>/main.c, named Lesson_<folder>
#------------------------------------------------------------------------------
if(CPROCESSING_BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.c")
    file(GLOB LESSON_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/examples/lessons/*/main.c")

    set(example_targets "")
    foreach(source ${EXAMPLE_SOURCES} ${LESSON_SOURCES})
        get_filename_component(name "${source}" NAME_WE)
        set(target "Example_${name}")
        if(name STREQUAL "main")
            get_filename_component(lesson_dir "${source}" DIRECTORY)
            get_filename_component(lesson_name "${lesson_dir}" NAME)
            set(target "Lesson_${lesson_name}")
        endif()
        add_executable(${target} "${source}")
        target_link_libraries(${target} PRIVATE CProcessing)
        if(UNIX AND NOT APPLE)
            target_link_libraries(${target} PRIVATE m)
        endif()
        cprocessing_set_warnings(${target})
        cprocessing_set_rpath(${target})
        set_target_properties(${target} PROPERTIES FOLDER Examples)
        list(APPEND example_targets ${target})
    endforeach()

    # The examples load the templates' assets. One copy into bin/assets
    # serves them all; it waits for the demos' and tests' copies into the
    # same folder, which would otherwise collide in a parallel build.
    list(GET example_targets 0 first_example)
    add_custom_target(Example_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/templates/vs/assets"
            "$<TARGET_FILE_DIR:${first_example}>/assets"
        COMMENT "Copying the template assets for the examples"
        VERBATIM)
    set_target_properties(Example_assets PROPERTIES FOLDER Examples)
    foreach(other CProcessing_Demos CProcessing_Tests_Tier2)
        if(TARGET ${other})
            add_dependencies(Example_assets ${other})
        endif()
    endforeach()
    foreach(target ${example_targets})
        add_dependencies(${target} Example_assets)
    endforeach()

    # Students often write their game in C++, starting from these files. So
    # each example and lesson, the demos and the templates' main.c are also
    # compiled as C++ (not linked: one small file each, in the build folder,
    # includes the real one). A C-only habit in any of them, such as malloc
    # without a cast, fails the build here.
    set(cxx_check_sources ${EXAMPLE_SOURCES} ${LESSON_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/vs/main.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/cmake/main.c")
    if(CPROCESSING_BUILD_DEMOS)
        file(GLOB DEMO_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/demos/*.c")
        list(APPEND cxx_check_sources ${DEMO_SOURCES}) # includes demos/main.c
    endif()
    set(cxx_wrappers "")
    foreach(source ${cxx_check_sources})
        file(RELATIVE_PATH relative "${CMAKE_CURRENT_SOURCE_DIR}" "${source}")
        string(REGEX REPLACE "[/.]" "_" wrapper_name "${relative}")
        set(wrapper "${CMAKE_CURRENT_BINARY_DIR}/cxx_check/${wrapper_name}.cpp")
        file(CONFIGURE OUTPUT "${wrapper}" CONTENT "#include \"${source}\"\n")
        list(APPEND cxx_wrappers "${wrapper}")
    endforeach()
    add_library(Example_CxxCheck OBJECT ${cxx_wrappers})
    target_include_directories(Example_CxxCheck PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/demos")
    target_link_libraries(Example_CxxCheck PRIVATE CProcessing)
    cprocessing_set_cxx_warnings(Example_CxxCheck)
    set_target_properties(Example_CxxCheck PROPERTIES FOLDER Examples)
endif()

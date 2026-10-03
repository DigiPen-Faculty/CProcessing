# CProcessing - cross-platform CMake build (Windows, Linux, macOS)
#
# This sits alongside the Visual Studio solution in Processing_Sample/, it
# does not replace it: the .sln/.vcxproj workflow students and faculty use
# today is unchanged. See BUILDING.md for usage.
#
#   cmake -B build                  # configure (downloads GLFW + SoLoud, pinned)
#   cmake --build build             # library, demos and tests
#   ctest --test-dir build          # run the test suites
#
# Everything is placed in build/bin (build/bin/<Config> for multi-config
# generators such as Visual Studio): the CProcessing shared library, the
# Processing_Demos executable, the test executables and an Assets folder.

cmake_minimum_required(VERSION 3.20)

# The version is defined once, in cprocessing_common.h
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/Processing_Sample/CProcessing/inc/cprocessing_common.h"
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
option(CPROCESSING_BUILD_DEMOS "Build the Processing_Demos sample program" ${CPROCESSING_IS_TOP_LEVEL})
option(CPROCESSING_BUILD_TESTS "Build the Tier 1 and Tier 2 test suites" ${CPROCESSING_IS_TOP_LEVEL})
option(CPROCESSING_WARNINGS_AS_ERRORS "Treat compiler warnings in CProcessing code as errors" OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Put every runtime artifact in one folder so executables find the library
# (and their Assets) without any install step.
if(CPROCESSING_IS_TOP_LEVEL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cmake/CProcessingDependencies.cmake)

#------------------------------------------------------------------------------
# Shared compile settings for CProcessing's own code
#------------------------------------------------------------------------------
function(cprocessing_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
        if(CPROCESSING_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        # -Wmissing-braces only flags the idiomatic { x, y } initializers
        # for the CP_Vector/CP_Color unions, which are well defined.
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wno-missing-braces)
        if(CPROCESSING_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
    if(NOT MSVC)
        # Anonymous structs inside unions (CP_Color, CP_Vector, ...) are C11.
        set_target_properties(${target} PROPERTIES C_STANDARD 11 C_EXTENSIONS ON)
    endif()
endfunction()

# Let executables find the shared library sitting next to them.
function(cprocessing_set_rpath target)
    if(APPLE)
        set_target_properties(${target} PROPERTIES BUILD_RPATH "@loader_path" INSTALL_RPATH "@loader_path")
    elseif(UNIX)
        set_target_properties(${target} PROPERTIES BUILD_RPATH "$ORIGIN" INSTALL_RPATH "$ORIGIN")
    endif()
endfunction()

# Copy an Assets folder next to the built executables.
function(cprocessing_copy_assets target assets_dir)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${assets_dir}" "$<TARGET_FILE_DIR:${target}>/Assets"
        COMMENT "Copying ${assets_dir} next to ${target}"
        VERBATIM)
endfunction()

#------------------------------------------------------------------------------
# The CProcessing library
#------------------------------------------------------------------------------
set(CP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Processing_Sample/CProcessing")

set(CPROCESSING_SOURCES
    "${CP_DIR}/GLAD/glad.c"
    "${CP_DIR}/nanovg/src/nanovg.c"
    "${CP_DIR}/Source/CP_Color.c"
    "${CP_DIR}/Source/CP_File.c"
    "${CP_DIR}/Source/CP_Graphics.c"
    "${CP_DIR}/Source/CP_Image.c"
    "${CP_DIR}/Source/CP_Input.c"
    "${CP_DIR}/Source/CP_Math.c"
    "${CP_DIR}/Source/CP_Noise.c"
    "${CP_DIR}/Source/CP_Random.c"
    "${CP_DIR}/Source/CP_Setting.c"
    "${CP_DIR}/Source/CP_Sound.c"
    "${CP_DIR}/Source/CP_System.c"
    "${CP_DIR}/Source/CP_Text.c"
    "${CP_DIR}/Source/nanovg_gl3.c"
    "${CP_DIR}/Source/tinycthread.c"
    "${CP_DIR}/Source/vect.c"
)
# Vendored third-party sources (NanoVG and its OpenGL 3 back end with the
# bundled stb_image/stb_truetype, GLAD, tinycthread) are compiled as-is;
# don't report their warnings. The Visual Studio project does the same.
set(CPROCESSING_VENDORED_SOURCES
    "${CP_DIR}/GLAD/glad.c"
    "${CP_DIR}/nanovg/src/nanovg.c"
    "${CP_DIR}/Source/nanovg_gl3.c"
    "${CP_DIR}/Source/tinycthread.c"
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
        "${CP_DIR}/Source"
        "${CP_DIR}/GLAD"
        "${CP_DIR}/nanovg/src"
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
    set(DEMO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Processing_Sample")
    add_executable(Processing_Demos
        "${DEMO_DIR}/main.c"
        "${DEMO_DIR}/Demos/DancingLinesDemo.c"
        "${DEMO_DIR}/Demos/DemoManager.c"
        "${DEMO_DIR}/Demos/GamepadDemo.c"
        "${DEMO_DIR}/Demos/GridWaveDemo.c"
        "${DEMO_DIR}/Demos/NoiseDemo.c"
    )
    target_include_directories(Processing_Demos PRIVATE "${DEMO_DIR}/Demos")
    target_link_libraries(Processing_Demos PRIVATE CProcessing)
    if(UNIX AND NOT APPLE)
        target_link_libraries(Processing_Demos PRIVATE m)
    endif()
    cprocessing_set_warnings(Processing_Demos)
    cprocessing_set_rpath(Processing_Demos)
    cprocessing_copy_assets(Processing_Demos "${DEMO_DIR}/Assets")
endif()

#------------------------------------------------------------------------------
# Tests
#------------------------------------------------------------------------------
if(CPROCESSING_BUILD_TESTS)
    enable_testing()
    set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Processing_Sample/CProcessing_Tests")
    set(TIER2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Processing_Sample/CProcessing_Tests_Tier2")
    set(UNITY_DIR "${TEST_DIR}/ThirdParty/Unity")

    # Tier 1: pure functions, no window/GL/audio needed
    file(GLOB TIER1_SOURCES CONFIGURE_DEPENDS "${TEST_DIR}/Tests/*.c")
    add_executable(CProcessing_Tests ${TIER1_SOURCES} "${UNITY_DIR}/unity.c")
    target_include_directories(CProcessing_Tests PRIVATE
        "${UNITY_DIR}"
        "${TEST_DIR}/Tests"
        "${CP_DIR}/Source"               # Internal_InputLogic.h (pure, header-only)
        "${CPROCESSING_GLFW_INCLUDE_DIR}" # GLFW constants used by that header
    )
    target_compile_definitions(CProcessing_Tests PRIVATE UNITY_INCLUDE_DOUBLE)
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
    file(GLOB TIER2_SOURCES CONFIGURE_DEPENDS "${TIER2_DIR}/Tests/*.c")
    add_executable(CProcessing_Tests_Tier2 ${TIER2_SOURCES} "${UNITY_DIR}/unity.c")
    target_include_directories(CProcessing_Tests_Tier2 PRIVATE "${UNITY_DIR}" "${TIER2_DIR}/Tests")
    target_compile_definitions(CProcessing_Tests_Tier2 PRIVATE UNITY_INCLUDE_DOUBLE)
    target_link_libraries(CProcessing_Tests_Tier2 PRIVATE CProcessing)
    if(UNIX AND NOT APPLE)
        target_link_libraries(CProcessing_Tests_Tier2 PRIVATE m)
    endif()
    cprocessing_set_rpath(CProcessing_Tests_Tier2)
    if(NOT MSVC)
        set_target_properties(CProcessing_Tests_Tier2 PROPERTIES C_STANDARD 11 C_EXTENSIONS ON)
    endif()
    cprocessing_copy_assets(CProcessing_Tests_Tier2 "${TIER2_DIR}/Assets")
    add_test(NAME tier2 COMMAND CProcessing_Tests_Tier2
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:CProcessing_Tests_Tier2>")
    set_tests_properties(tier2 PROPERTIES LABELS "tier2;gui" TIMEOUT 120)
endif()

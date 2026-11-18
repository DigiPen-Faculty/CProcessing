# Fails if a test function is written but not registered, so that a forgotten
# RUN_TEST can't silently drop a test.
#
# Every `void test_...(void)` defined in the suite's test_*.c files must be
# declared in the runner header and run by a RUN_TEST in the main file.
#
#   cmake -DTESTS_DIR=<folder> -DMAIN_FILE=<file> -DHEADER=<file> -P check_test_registration.cmake
#
# SPDX-License-Identifier: MIT
# Copyright (c) 2019-2026 DigiPen Institute of Technology

foreach(required TESTS_DIR MAIN_FILE HEADER)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "check_test_registration: -D${required}=... is required")
    endif()
endforeach()

file(READ "${MAIN_FILE}" main_text)
file(READ "${HEADER}" header_text)
file(GLOB test_sources "${TESTS_DIR}/test_*.c")
list(REMOVE_ITEM test_sources "${MAIN_FILE}")

set(problems "")
set(count 0)
foreach(source IN LISTS test_sources)
    # a definition: the name alone on its line, with the body below (not a prototype)
    file(STRINGS "${source}" definitions REGEX "^void test_[A-Za-z0-9_]+\\(void\\)[ \t\r]*$")
    foreach(definition IN LISTS definitions)
        string(REGEX MATCH "test_[A-Za-z0-9_]+" name "${definition}")
        math(EXPR count "${count} + 1")
        get_filename_component(file_name "${source}" NAME)
        if(NOT header_text MATCHES "void[ \t]+${name}\\(void\\);")
            string(APPEND problems "  ${name} (${file_name}) is not declared in the runner header\n")
        endif()
        if(NOT main_text MATCHES "RUN_TEST\\(${name}\\)")
            string(APPEND problems "  ${name} (${file_name}) has no RUN_TEST in the main file\n")
        endif()
    endforeach()
endforeach()

if(count EQUAL 0)
    message(FATAL_ERROR "check_test_registration: found no tests in ${TESTS_DIR}")
endif()
if(problems)
    message(FATAL_ERROR "Tests that would not run:\n${problems}")
endif()
message(STATUS "All ${count} tests in ${TESTS_DIR} are registered")

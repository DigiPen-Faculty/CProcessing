# CProcessing starter project for Windows, Linux and macOS
#
# Build and run, from this folder:
#
#   cmake -B build
#   cmake --build build
#   cd build/bin
#   ./MyGame                (MyGame.exe on Windows)
#
# The game is built into build/bin, next to the CProcessing library and a
# copy of the Assets folder. CProcessing looks for assets from the current
# working directory, and then next to the game, so it finds them there.
#
# Where CProcessing comes from:
#   - The Linux and macOS release packages include a CProcessing folder next
#     to this file, with the library already built. It's used when it's there.
#   - Otherwise CProcessing is downloaded and built from source the first time
#     you configure. That needs an internet connection and, on Linux, the
#     development packages listed in CProcessing's BUILDING.md.

cmake_minimum_required(VERSION 3.20)

# The game's name, which is also the name of the program file
project(MyGame C)

add_executable(${PROJECT_NAME} main.c)

set(CPROCESSING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CProcessing")
if(EXISTS "${CPROCESSING_DIR}/inc/cprocessing.h")
    # Prebuilt CProcessing from a release package
    add_library(CProcessing SHARED IMPORTED)
    set_target_properties(CProcessing PROPERTIES
        IMPORTED_LOCATION "${CPROCESSING_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}CProcessing${CMAKE_SHARED_LIBRARY_SUFFIX}"
        INTERFACE_INCLUDE_DIRECTORIES "${CPROCESSING_DIR}/inc")
else()
    # Download CProcessing's source for this release and build it
    enable_language(CXX)    # CProcessing's audio library, SoLoud, is C++
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    include(FetchContent)
    FetchContent_Declare(cprocessing
        URL https://github.com/DigiPen-Faculty/CProcessing/archive/refs/tags/v3.0.1.tar.gz)
    FetchContent_MakeAvailable(cprocessing)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE CProcessing)

# Put the game in build/bin. ($<0:> stops multi-configuration generators,
# such as Visual Studio's, from adding a Debug or Release subfolder.)
set(GAME_DIR "${CMAKE_BINARY_DIR}/bin")
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${GAME_DIR}$<0:>"
    VS_DEBUGGER_WORKING_DIRECTORY "${GAME_DIR}")

if(WIN32)
    # A window without a console, like the Visual Studio template
    # (CP_System_ShowConsole opens one when it's wanted)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    if(MSVC)
        target_link_options(${PROJECT_NAME} PRIVATE /ENTRY:mainCRTStartup)
    endif()
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "@loader_path")
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "$ORIGIN")
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

# Copy the CProcessing library and the Assets folder next to the game
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:CProcessing>" "${GAME_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets" "${GAME_DIR}/Assets"
    VERBATIM)

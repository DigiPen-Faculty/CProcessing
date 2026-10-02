# Third-party dependencies that are built from source: GLFW, SoLoud, miniaudio.
#
# This file is the single place their versions are pinned, for every
# platform and both build systems: the prebuilt libraries the Visual Studio
# solution links are generated from these same pins by
# tools/update-windows-prebuilt.ps1.
#
# Each is downloaded at configure time from a pinned URL and verified against
# a SHA-256 hash, so every build uses exactly the same source. See
# DEPENDENCIES.md for the policy and for how to update a pin.
#
# Offline builds: point FETCHCONTENT_SOURCE_DIR_GLFW / _SOLOUD / _MINIAUDIO
# at an already-extracted copy of the same release, e.g.
#   cmake -B build -DFETCHCONTENT_SOURCE_DIR_GLFW=/path/to/glfw-3.5.1

include(FetchContent)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW) # use extraction time for timestamps
endif()

# Dependencies are linked statically into the CProcessing shared library.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#------------------------------------------------------------------------------
# GLFW - windowing, input, OpenGL context creation
#------------------------------------------------------------------------------
#
# 3.5.1 rather than 3.4: GLFW 3.4's Wayland backend calls wl_seat_get_version() on a NULL seat when
# the compositor has not advertised an input seat at startup, crashing
# CProcessing intermittently under Wayland (seen under WSLg; found with core
# dumps). 3.5.1 handles a missing seat. The API CProcessing uses is the same.
set(CPROCESSING_GLFW_VERSION "3.5.1")
set(CPROCESSING_GLFW_URL "https://github.com/glfw/glfw/releases/download/3.5.1/glfw-3.5.1.zip")
set(CPROCESSING_GLFW_SHA256 "ea79bc5feffc254c87291980c2d0bce9acebb68c4983b79f961dcd2cb8a611a0")

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)

FetchContent_Declare(glfw
    URL "${CPROCESSING_GLFW_URL}"
    URL_HASH "SHA256=${CPROCESSING_GLFW_SHA256}"
)
FetchContent_MakeAvailable(glfw)

# The library includes "glfw3.h"/"glfw3native.h" directly (no GLFW/ prefix),
# matching how the Visual Studio project lays out its include paths.
set(CPROCESSING_GLFW_INCLUDE_DIR "${glfw_SOURCE_DIR}/include/GLFW")

#------------------------------------------------------------------------------
# SoLoud - audio
#
# Pinned to upstream master rather than the last tagged release
# (RELEASE_20200207, which CProcessing used until 2026): master
# carries ~4 years of fixes including voice-group allocation and miniaudio
# updates, and its C API is a superset of the 20200207 one CProcessing uses.
#
# Backends: miniaudio (WASAPI / ALSA / PulseAudio / JACK / CoreAudio, all
# loaded at runtime - no audio libraries needed at build time) plus "nosound",
# a silent mixer that SoLoud falls back to automatically when no audio device
# can be opened (headless machines, CI), so sounds still load, play and finish.
#------------------------------------------------------------------------------
set(CPROCESSING_SOLOUD_COMMIT "e82fd32c1f62183922f08c14c814a02b58db1873")
set(CPROCESSING_SOLOUD_URL "https://github.com/jarikomppa/soloud/archive/${CPROCESSING_SOLOUD_COMMIT}.tar.gz")
set(CPROCESSING_SOLOUD_SHA256 "863c5a2501ac935fb88bb357b1375e50d75876dd1f37806f4045143f46dbf3f1")

FetchContent_Declare(soloud
    URL "${CPROCESSING_SOLOUD_URL}"
    URL_HASH "SHA256=${CPROCESSING_SOLOUD_SHA256}"
    # SoLoud's own CMake (contrib/) is not used; SOURCE_SUBDIR points at a
    # directory without a CMakeLists.txt so MakeAvailable only downloads it.
    SOURCE_SUBDIR "cprocessing-builds-this-itself"
)
FetchContent_MakeAvailable(soloud)

#------------------------------------------------------------------------------
# miniaudio - used by SoLoud's miniaudio backend
#
# SoLoud's tree bundles miniaudio 0.10.42 (2021), whose PulseAudio backend
# dereferences a NULL sink-info pointer when PulseAudio reports an error
# during device setup. That crashes CProcessing at startup intermittently on
# Linux desktops using PulseAudio/PipeWire (found with AddressSanitizer).
# A current miniaudio release is used instead; its playback API is
# unchanged for what SoLoud's backend uses. SoLoud's backend source is
# compiled from a copy so it picks up this miniaudio.h rather than the one
# sitting next to it in the SoLoud tree.
#------------------------------------------------------------------------------
set(CPROCESSING_MINIAUDIO_VERSION "0.11.25")
set(CPROCESSING_MINIAUDIO_URL "https://github.com/mackron/miniaudio/archive/refs/tags/0.11.25.tar.gz")
set(CPROCESSING_MINIAUDIO_SHA256 "b900edcffe979816e2560a0580b9b1216d674b4f17fbadeca8f777a7f8ab0274")

FetchContent_Declare(miniaudio
    URL "${CPROCESSING_MINIAUDIO_URL}"
    URL_HASH "SHA256=${CPROCESSING_MINIAUDIO_SHA256}"
    SOURCE_SUBDIR "cprocessing-builds-this-itself"
)
FetchContent_MakeAvailable(miniaudio)

set(CPROCESSING_SOLOUD_MINIAUDIO_BACKEND "${CMAKE_CURRENT_BINARY_DIR}/soloud_miniaudio_backend/soloud_miniaudio.cpp")
configure_file("${soloud_SOURCE_DIR}/src/backend/miniaudio/soloud_miniaudio.cpp"
    "${CPROCESSING_SOLOUD_MINIAUDIO_BACKEND}" COPYONLY)

file(GLOB CPROCESSING_SOLOUD_SOURCES CONFIGURE_DEPENDS
    "${soloud_SOURCE_DIR}/src/core/*.cpp"
    "${soloud_SOURCE_DIR}/src/filter/*.cpp"
    "${soloud_SOURCE_DIR}/src/audiosource/*/*.cpp"
    "${soloud_SOURCE_DIR}/src/audiosource/*/*.c"
    "${soloud_SOURCE_DIR}/src/c_api/soloud_c.cpp"
    "${soloud_SOURCE_DIR}/src/backend/nosound/soloud_nosound.cpp"
    "${soloud_SOURCE_DIR}/src/backend/null/soloud_null.cpp"
)
list(APPEND CPROCESSING_SOLOUD_SOURCES "${CPROCESSING_SOLOUD_MINIAUDIO_BACKEND}")

add_library(soloud STATIC ${CPROCESSING_SOLOUD_SOURCES})
target_include_directories(soloud PUBLIC "${soloud_SOURCE_DIR}/include")
set_source_files_properties("${CPROCESSING_SOLOUD_MINIAUDIO_BACKEND}" PROPERTIES
    INCLUDE_DIRECTORIES "${miniaudio_SOURCE_DIR}")
target_compile_definitions(soloud PRIVATE WITH_MINIAUDIO WITH_NOSOUND WITH_NULL)
set_target_properties(soloud PROPERTIES POSITION_INDEPENDENT_CODE ON FOLDER "ThirdParty")

find_package(Threads REQUIRED)
target_link_libraries(soloud PRIVATE Threads::Threads ${CMAKE_DL_LIBS})

if(MSVC)
    target_compile_definitions(soloud PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(soloud PRIVATE /W0)
else()
    target_compile_options(soloud PRIVATE -w)
endif()

if(APPLE)
    target_link_libraries(soloud PRIVATE
        "-framework CoreFoundation" "-framework CoreAudio" "-framework AudioToolbox")
elseif(UNIX)
    target_link_libraries(soloud PRIVATE m)
endif()

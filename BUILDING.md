# Building CProcessing

CProcessing builds on **Windows**, **Linux** and **macOS**. There are two ways
to build it:

- **Visual Studio solution (Windows):** the existing workflow, unchanged.
- **CMake (Windows, Linux, macOS):** builds the library, the demos and the
  tests from source on any platform.

Most students only need the
[latest release](https://github.com/DigiPen-Faculty/CProcessing/releases)
template project. This page is for building CProcessing itself.

## Windows: Visual Studio solution

Open `Processing_Sample/Processing_Sample.sln` in Visual Studio 2022 or later
and build. The solution contains:

| Project | What it is |
|---|---|
| `CProcessing` | The library: `CProcessing.dll`, or `CProcessingd.dll` for Debug. |
| `Processing_Sample` | The demo program. Press 1–4 to switch demos, F for fullscreen, Escape to quit. |
| `CProcessing_Tests` | Tier 1 tests: pure functions, no window needed. |
| `CProcessing_Tests_Tier2` | Tier 2 tests: drives the real engine and checks the rendered pixels. |

## Any platform: CMake

### Prerequisites

- **Windows:** Visual Studio 2022 or later with the "Desktop development with
  C++" workload, which includes CMake. A standalone CMake 3.20+ also works.
- **macOS:** Xcode Command Line Tools (`xcode-select --install`) and CMake
  3.20+ (`brew install cmake`).
- **Linux:** a C/C++ compiler, CMake 3.20+, and the X11/Wayland/OpenGL
  development packages GLFW needs:

  ```sh
  # Debian / Ubuntu
  sudo apt install build-essential cmake ninja-build \
      libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev \
      libwayland-dev libxkbcommon-dev wayland-protocols libgl1-mesa-dev

  # Fedora
  sudo dnf install gcc gcc-c++ cmake ninja-build \
      libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel libXext-devel \
      wayland-devel libxkbcommon-devel wayland-protocols-devel mesa-libGL-devel

  # Arch
  sudo pacman -S base-devel cmake ninja \
      libx11 libxrandr libxinerama libxcursor libxi libxext wayland libxkbcommon wayland-protocols mesa
  ```

  No audio development packages are needed. Audio libraries (PulseAudio,
  PipeWire, ALSA) are loaded when the program runs.

The first configure downloads GLFW, SoLoud and miniaudio. Each one is pinned
to an exact version and checked against a SHA-256 hash, so an internet
connection is needed the first time. See [DEPENDENCIES.md](DEPENDENCIES.md),
including how to build offline.

### Build

From the repository root:

```sh
cmake -B build                       # configure (Release by default)
cmake --build build                  # build library, demos and tests
```

On Windows add `--config Release` (or `Debug`) to the build command, because
Visual Studio generators hold several configurations at once.

Everything ends up in one folder: `build/bin`, or `build/bin/Release` with
Visual Studio generators. That folder holds the CProcessing library, the
`Processing_Demos` program, both test programs and an `Assets` folder.

### Run the demos

CProcessing loads assets relative to the **current working directory**, so
run from the output folder:

```sh
cd build/bin
./Processing_Demos          # Processing_Demos.exe on Windows
```

### Run the tests

```sh
ctest --test-dir build --output-on-failure            # add -C Release on Windows
ctest --test-dir build -L tier1                       # only the windowless tests
```

Tier 2 opens a real window. On a headless Linux machine, such as a server or
a CI runner, run it under a virtual display. Mesa's software renderer is
enough:

```sh
xvfb-run -a ctest --test-dir build -L tier2 --output-on-failure
```

### Options

Pass options to the configure step with `-D`, for example
`cmake -B build -DCPROCESSING_BUILD_TESTS=OFF`.

| Option | Default | Meaning |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug` or `Release`, for single-configuration generators. |
| `CPROCESSING_BUILD_SHARED` | `ON` | Build a shared library (`.dll`/`.so`/`.dylib`). Set `OFF` for a static library. |
| `CPROCESSING_BUILD_DEMOS` | `ON`* | Build `Processing_Demos`. |
| `CPROCESSING_BUILD_TESTS` | `ON`* | Build the Tier 1 and Tier 2 tests. |
| `CPROCESSING_WARNINGS_AS_ERRORS` | `OFF` | Treat warnings in CProcessing's own code as errors. CI turns this on. |
| `CPROCESSING_FORCE_CANVAS_FBO` | `OFF` | Use the offscreen drawing canvas on Windows as well (see Platform notes). |

\* Only when CProcessing is the top-level project. When it's added to another
project with `add_subdirectory`, the demos and tests are off by default.

## Using CProcessing in your own CMake project

A game that lives in its own folder can pull CProcessing in with CMake:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyGame C CXX)   # CXX is needed because SoLoud is C++

include(FetchContent)
FetchContent_Declare(cprocessing
    GIT_REPOSITORY https://github.com/DigiPen-Faculty/CProcessing.git
    GIT_TAG        main)   # better: pin a release tag or commit
FetchContent_MakeAvailable(cprocessing)

add_executable(MyGame main.c)
target_link_libraries(MyGame PRIVATE CProcessing::CProcessing)

# Put the library and the Assets folder next to the game, so it runs from there
set_target_properties(MyGame PROPERTIES BUILD_RPATH "$ORIGIN")   # Linux; use @loader_path on macOS
add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:CProcessing> $<TARGET_FILE_DIR:MyGame>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:MyGame>/Assets)
```

`#include "cprocessing.h"` then works exactly as it does in the Visual Studio
template.

## Platform notes

- **Same API everywhere.** Student code written against `cprocessing.h`
  compiles unchanged on all three platforms. On Windows, `cprocessing.h` still
  includes `<windows.h>` for backward compatibility. Define `CP_NO_WINDOWS_H`
  before including it to opt out.
- **Drawing persists between frames** on every platform, as in Processing.
  Windows does this with a single-buffered window. Linux and macOS draw into
  an offscreen canvas that is copied to the window each frame, because Wayland
  and macOS don't support single-buffered windows reliably.
- **Linux: X11 and Wayland** are both supported. GLFW picks the native one
  automatically. Under Wayland, `CP_System_SetWindowPosition` has no effect,
  because the compositor places windows, and `CP_System_GetWindowHandle`
  returns `NULL`.
- **Gamepads** use GLFW's gamepad mappings on every platform: Xbox
  controllers on Windows, plus most controllers on Linux and macOS. Buttons
  follow the Xbox layout.
- **No audio device** (a headless machine, or a VM without sound) is not an
  error. With the CMake build, sounds still load and "play" silently. With
  the Visual Studio build's prebuilt SoLoud, sound is disabled instead:
  `CP_Sound_Load` returns `NULL`, and the other sound functions do nothing.
- **macOS** builds and passes compile checks, but has not yet been run on a
  Mac. Please report anything that misbehaves there.

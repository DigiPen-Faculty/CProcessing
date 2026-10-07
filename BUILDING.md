# Building CProcessing

CProcessing builds on **Windows**, **Linux** and **macOS**. There are two ways
to build it:

- **Visual Studio solution (Windows):** the workflow students and faculty
  use, with Visual Studio 2026.
- **CMake (Windows, Linux, macOS):** builds the library, the demos and the
  tests from source on any platform.

Most students only need the
[latest release](https://github.com/DigiPen-Faculty/CProcessing/releases)
template project. This page is for building CProcessing itself.

## Windows: Visual Studio solution

Open `Processing_Sample/Processing_Sample.sln` in Visual Studio 2026 and
build. The projects use Visual Studio 2026's compiler (platform toolset
v145), as does the student template. The solution contains:

| Project | What it is |
|---|---|
| `CProcessing` | The library: `CProcessing.dll`, or `CProcessingd.dll` for Debug. |
| `Processing_Sample` | The demo program. Press 1–4 to switch demos, F for fullscreen, C to show or hide the console, Escape to quit. |
| `CProcessing_Tests` | Tier 1 tests: pure functions, no window needed. |
| `CProcessing_Tests_Tier2` | Tier 2 tests: drives the real engine and checks the rendered pixels. |

## Any platform: CMake

### Prerequisites

- **Windows:** Visual Studio 2026 with the "Desktop development with C++"
  workload, which includes CMake. Visual Studio 2022 and a standalone CMake
  3.20+ also work.
- **macOS:** Xcode Command Line Tools (`xcode-select --install`) and CMake
  3.20+ (`brew install cmake`).
- **Linux:** a C/C++ compiler, CMake 3.20+, and the X11/Wayland/OpenGL
  development packages GLFW needs:

  ```sh
  # Debian / Ubuntu
  sudo apt install build-essential cmake ninja-build pkg-config \
      libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev \
      libwayland-dev libxkbcommon-dev wayland-protocols libgl1-mesa-dev

  # Fedora
  sudo dnf install gcc gcc-c++ cmake ninja-build pkgconf-pkg-config \
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

CProcessing looks for assets from the **current working directory**, and
then next to the program, so the demos find their `Assets` folder however
they're started. From a terminal:

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

On Windows without a GPU, such as a virtual machine or a CI runner, use
Mesa's software renderer: copy `opengl32.dll`, `libglapi.dll` and
`libgallium_wgl.dll` from a [Mesa for Windows](https://github.com/pal1000/mesa-dist-win/releases)
release (`x64` or `x86` folder, matching the build) next to
`CProcessing_Tests_Tier2.exe`, and set `GALLIUM_DRIVER=llvmpipe`. CI runs
Tier 2 this way for both the 64-bit and 32-bit builds (see
`.github/workflows/tests.yml`).

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

\* Only when CProcessing is the top-level project. When it's added to another
project with `add_subdirectory`, the demos and tests are off by default.

## Using CProcessing in your own CMake project

The easiest start is the CMake starter project in
[`Processing_Empty_CMake/`](Processing_Empty_CMake/). Copy the folder and
follow its README. It's also on the
[Releases page](https://github.com/DigiPen-Faculty/CProcessing/releases) for
Linux and macOS, with the library already built:
`CProcessingTemplate-linux-x64.zip` and `CProcessingTemplate-macos.zip`.

To add CProcessing to an existing CMake project, download it with
`FetchContent`:

```cmake
project(MyGame C CXX)   # CXX is needed because SoLoud is C++

include(FetchContent)
FetchContent_Declare(cprocessing
    URL https://github.com/DigiPen-Faculty/CProcessing/archive/refs/tags/v3.0.1.tar.gz)
FetchContent_MakeAvailable(cprocessing)

target_link_libraries(MyGame PRIVATE CProcessing::CProcessing)
```

The game also needs the CProcessing library and its `Assets` folder next to
it. The starter's `CMakeLists.txt` shows how to set that up, along with the
output folder, the library search path on Linux and macOS, and a window
without a console on Windows. `#include "cprocessing.h"` then works exactly as
it does in the Visual Studio template.

To build against a local copy of CProcessing instead of the download, add
`-DFETCHCONTENT_SOURCE_DIR_CPROCESSING=/path/to/CProcessing` when configuring.

## Platform notes

- **Same API everywhere.** Student code written against `cprocessing.h`
  compiles unchanged on all three platforms. `cprocessing.h` includes no OS
  headers. Code that needs Windows APIs includes `<windows.h>` itself.
  `CP_System_GetWindowHandle` returns a `void*`; cast it to `HWND` on Windows.
- **Console output.** `CP_System_ShowConsole(TRUE)` makes `printf` output
  visible. On Windows it uses the terminal the program was started from, or
  else opens a console window, which `CP_System_ShowConsole(FALSE)` hides
  again. On Linux and macOS, output goes to the terminal the program was
  started from, so run it from a terminal to see it. On every platform,
  showing the console also turns off output buffering, so each `printf`
  appears immediately.
- **Drawing persists between frames**, as in Processing. Every platform draws
  into the same kind of offscreen canvas, which is copied to the window each
  frame.
- **Linux: X11 and Wayland** are both supported. GLFW picks the native one
  automatically. Under Wayland, `CP_System_SetWindowPosition` has no effect,
  because the compositor places windows, and `CP_System_GetWindowHandle`
  returns `NULL`.
- **Gamepads** use GLFW's gamepad mappings on every platform: Xbox
  controllers on Windows, plus most controllers on Linux and macOS. Buttons
  follow the Xbox layout.
- **macOS: connect Xbox controllers over Bluetooth.** On a USB cable,
  macOS doesn't recognize an Xbox controller as a game controller: it speaks
  Microsoft's own protocol there, not the standard one (HID) that GLFW uses
  on macOS. Browsers and Steam include their own Xbox drivers, so a
  controller that works there can still be invisible to CProcessing on a
  cable. Over Bluetooth, macOS supports it, and so does GLFW.
- **No audio device** (a headless machine, or a VM without sound) is not an
  error. Sounds still load and "play" silently.
- **One library file.** GLFW, SoLoud and miniaudio are linked into the
  CProcessing library itself. A game needs only `CProcessing.dll`
  (`libCProcessing.so` / `libCProcessing.dylib`) next to it.
- **macOS** is a preview. Since 3.0.1, drawing, sound, screenshots, the
  demos and both test tiers work on an Apple silicon Mac (macOS 26);
  gamepads and Intel Macs haven't been confirmed yet. CI builds it and runs Tier 1 only: Tier 2
  and the demos need a GPU, which GitHub's hosted macOS runners don't have.
  Please report anything that misbehaves there.

## Making a release

1. **Set the version** in three places:
   - `CP_VERSION_MAJOR/MINOR/PATCH` in
     `Processing_Sample/CProcessing/inc/cprocessing_common.h`. CMake reads
     its version from these.
   - The download tag in `Processing_Empty_CMake/CMakeLists.txt`.
   - The newest section heading in `CHANGELOG.md`, with the release date
     replacing "(unreleased)".
2. **Regenerate the Windows template** in the repository from a Visual
   Studio 2026 Developer Command Prompt, and commit the result:
   `BuildReleasePackage.bat -r all -c -z`. This updates
   `Processing_Empty/CProcessing`; the zip it writes to `Releases/` is not
   committed.
3. **Optionally, check the packages** by running the Release workflow by hand
   (Actions > Release > Run workflow). It builds all three packages and
   checks them, without releasing anything.
4. **Push a tag** named after the version, such as `v3.0.0`. The Release
   workflow checks that the version matches everywhere and builds:
   - `CProcessingTemplate.zip` (the Visual Studio template)
   - `CProcessingTemplate-linux-x64.zip`
   - `CProcessingTemplate-macos.zip`

   It then creates a draft GitHub Release with those files and the
   changelog section as its notes.
5. **Review the draft** on GitHub and publish it.

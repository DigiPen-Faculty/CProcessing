# Changelog

Notable changes in each CProcessing release. Version numbers are
major.minor.patch. The major number goes up when existing projects may need
changes to build with the new version.

## 3.0.0 (unreleased)

CProcessing now runs on Windows, Linux and macOS, with the same API and the
same behavior everywhere.

### Breaking changes

Each change says what to do in an existing project.

- **Visual Studio 2026 is required** for the template project and the
  Visual Studio solution. They use its compiler (platform toolset v145), and
  the CProcessing DLLs are built with it. A game built this way needs the
  Visual C++ runtime that comes with Visual Studio 2026; on a computer
  without it, install the latest Microsoft Visual C++ Redistributable.
- **`cprocessing.h` no longer includes `<windows.h>`.** Code that uses
  Windows APIs or types through it, such as `Sleep`, `BOOL` or `MAX_PATH`,
  needs `#include <windows.h>` added before `#include "cprocessing.h"`.
- **`CP_System_GetWindowHandle` returns a `void*`** (`CP_WindowHandle`).
  On Windows, cast it: `HWND hwnd = (HWND)CP_System_GetWindowHandle();`.
- **There is no `soloud.dll` anymore.** SoLoud is built into
  `CProcessing.dll`. Ship only `CProcessing.dll` (`CProcessingd.dll` for
  Debug) with a game, and remove any build step that copies `soloud.dll`.
- **Gamepads use GLFW instead of XInput.** Gamepad index N is the Nth
  connected controller, so code that uses index 0 for the first controller
  is unaffected. Xbox controllers work as before on Windows, and most other
  controllers now work too.

### New

- **Linux and macOS support.** The library, demos and tests build and run on
  Linux (X11 and Wayland). macOS builds cleanly but has not been tested on a
  Mac yet; please report problems.
- **A console for `printf` debugging:** `CP_System_ShowConsole(CP_BOOL show)`
  and `CP_System_GetConsoleVisible()`. On Windows it opens a console window,
  or uses the terminal the program was started from. On Linux and macOS,
  output goes to the terminal the program was started from. Showing the
  console also makes each `printf` appear immediately. The template's
  `main.c` has a commented-out call to try it.
- `CP_System_GetCursorVisible()`, the getter for `CP_System_ShowCursor`.
- `CP_VERSION_MAJOR`, `CP_VERSION_MINOR` and `CP_VERSION_PATCH` macros in
  `cprocessing_common.h`.
- **Templates for Linux and macOS:** `CProcessingTemplate-linux-x64.zip` and
  `CProcessingTemplate-macos.zip` on the Releases page. Each is a CMake
  starter project with the library already built, so it needs only a
  compiler and CMake. The same starter is in the repository as
  `Processing_Empty_CMake/`, which builds CProcessing from source on any
  platform. It includes Visual Studio Code settings.
- **A CMake build** for all three platforms, alongside the Visual Studio
  solution. See [BUILDING.md](BUILDING.md).
- **A new demo program** with a demo menu: press 1-4 to switch demos, F for
  fullscreen, C to show or hide the console, Escape to quit.
- **Automated tests** (157 unit tests and 59 rendering and engine tests) and
  continuous integration on Windows, Linux and macOS.

### Changed

- The same versions of GLFW (3.5.1), SoLoud and miniaudio (0.11.25) are used
  on every platform. See [DEPENDENCIES.md](DEPENDENCIES.md).
- Every platform draws into an offscreen canvas that is copied to the window
  each frame. Drawing still persists between frames as before.
- `CP_System_ShowCursor` and `CP_System_SetWindowTitle` can be called before
  `CP_Engine_Run`. The setting is kept and applied when the window opens.
  Before, it was lost.
- When the window can't be created (no display, or no OpenGL 3.2),
  `CP_Engine_Run` prints the reason and returns, instead of crashing.
- With no audio device, sounds load and play silently instead of crashing.

### Fixed

- Key presses and mouse clicks shorter than one frame were missed.
- A possible crash or memory corruption when the program exits.
- Writes outside the input arrays for unrecognized keys and the last mouse
  button.
- Toggling fullscreen off didn't restore the window size properly.
- `CP_Font_Load(NULL)` crashed.
- Screenshots on high-DPI displays captured the wrong region.
- `CP_System_GetDisplayRefreshRate` crashed when called before
  `CP_Engine_Run`.
- Text whose size or scale changes every frame (text pulsing on a sine
  wave, say) vanished, was cut off or showed stray letters for a single
  frame every few seconds. Programs that draw a lot of text at many
  different sizes also slowly leaked video memory.
- Linux and macOS builds could crash or corrupt memory once more than 12
  images or sounds, or more than 16 fonts, were loaded at the same time.

## Earlier releases

2.2.0 (October 2025), 2.1.0 (December 2024) and 2.0.0 (November 2021) are
described on the
[GitHub Releases page](https://github.com/DigiPen-Faculty/CProcessing/releases).

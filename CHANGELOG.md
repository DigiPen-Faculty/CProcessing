# Changelog

Notable changes in each CProcessing release. Version numbers are
major.minor.patch. The major number goes up when existing projects may need
changes to build with the new version.

## 3.0.2 (unreleased)

### Fixed

- **`CP_Sound_SetGroupPitch` was ignored by sounds played at their own
  pitch of 1.0**, which includes every `CP_Sound_Play`: they played at
  normal pitch whatever the group's pitch was. A sound now always plays at
  its own pitch times its group's pitch, as it already did for any other
  pitch.
- **A game started by double-clicking it on macOS couldn't find its
  Assets.** Finder starts a program in the home folder, not in the
  program's own folder, and some Linux file managers do the same. Images,
  fonts and sounds given a relative path, such as `"Assets/jump.wav"`, are
  now also looked for next to the program when they aren't found from the
  current folder. Running from a terminal or from Visual Studio works as
  before.
- **With more than one monitor, fullscreen always used the primary
  monitor**, whichever monitor the window was on, and leaving fullscreen
  moved the window there. Fullscreen now uses the monitor the window is
  on, a window is centered on that monitor, and
  `CP_System_GetDisplayWidth`, `CP_System_GetDisplayHeight` and
  `CP_System_GetDisplayRefreshRate` describe it. With one monitor nothing
  changes.

## 3.0.1 (2026-10-06)

Fixes from the first tests on a real Mac. On macOS, use 3.0.1 or later:
with 3.0.0, the window stays black. macOS support is still a preview:
drawing, the demos and the automated tests now work on an Apple silicon Mac
(macOS 26), but sound, gamepads and Intel Macs haven't been confirmed yet.

### Fixed

- **macOS: the window stayed black.** On macOS 26 on Apple silicon, the
  window opened at the right size but nothing drawn in it ever appeared.
  CProcessing drew everything correctly, but when it showed each frame, its
  offscreen drawing surface was still selected instead of the window, and
  macOS then displays nothing. It now selects the window first, on every
  platform.
- **High-DPI displays: `CP_System_GetWindowWidth` and
  `CP_System_GetWindowHeight` returned the size in screen pixels**, twice
  the drawing coordinates on a Retina Mac, so drawing at
  `CP_System_GetWindowWidth() / 2` landed on the right edge instead of the
  middle. They now return the size in the coordinates you draw in, which
  the mouse position also uses. On Windows the values don't change.
- On high-DPI displays, `CP_Image_Screenshot` now averages the screen pixels
  behind each screenshot pixel, instead of keeping one of them, so a
  screenshot looks smooth like the window.

## 3.0.0 (2026-10-03)

CProcessing now runs on Windows, Linux and macOS, with the same API and the
same behavior everywhere. macOS support is a preview in this release.

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
- **Text is 20% larger at the same `CP_Settings_TextSize`** with the default
  font. The size is now the font's em size in pixels, as in Processing, CSS
  and most other tools; before, it was the font's full height from
  ascender to descender. Line spacing in `CP_Font_DrawTextBox` grows to
  match. To keep the old look, multiply text sizes by 0.83 (for other
  fonts, by the em size divided by the ascender-to-descender height).

### New

- **Linux support, and macOS as a preview.** The library, demos and tests
  build and run on Linux (X11 and Wayland). On macOS they build and the unit
  tests pass, but CProcessing hasn't been tested on a real Mac yet; please
  [report problems](https://github.com/DigiPen-Faculty/CProcessing/issues).
- **A console for `printf` debugging:** `CP_System_ShowConsole(CP_BOOL show)`
  and `CP_System_GetConsoleVisible()`. On Windows it opens a console window,
  or uses the terminal the program was started from. On Linux and macOS,
  output goes to the terminal the program was started from. Showing the
  console also makes each `printf` appear immediately. The template's
  `main.c` has a commented-out call to try it.
- `CP_System_GetCursorVisible()`, the getter for `CP_System_ShowCursor`.
- `CP_Image_Load` loads PNGs with 16 bits per channel, which many paint
  programs save, and `CP_Font_Load` loads OpenType fonts with CFF outlines,
  common for `.otf` files. Before, both failed to load.
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
- **Automated tests** (157 unit tests and 70 rendering and engine tests) and
  continuous integration on Windows, Linux and macOS.

### Changed

- The same versions of GLFW (3.5.1), SoLoud and miniaudio (0.11.25) are used
  on every platform. See [DEPENDENCIES.md](DEPENDENCIES.md).
- NanoVG, which CProcessing draws with, is updated from a 2018 copy to its
  latest version, keeping CProcessing's own changes to it. They are listed
  in `Processing_Sample/CProcessing/nanovg/CPROCESSING.md`. The image and
  font loaders it uses, stb_image and stb_truetype, are updated from 2016
  versions to 2.30 and 1.26, with many fixes for crashes on unusual or
  damaged files.
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
- `CP_Graphics_DrawPoint` was hidden by `CP_Settings_NoFill` and still
  drawn after `CP_Settings_NoStroke`. A point now follows the stroke
  settings like a line does, as documented: `CP_Settings_NoStroke` hides
  it, and `CP_Settings_NoFill` doesn't.
- Text drawn mirrored or upside down, for example after
  `CP_Settings_Scale(-1, 1)` to flip a sprite, didn't appear at all.
- Text at negative coordinates could be shifted by a pixel.
- In 32-bit (x86) builds, large text (around size 600 and up) stopped the
  program with an assertion in Debug builds and didn't appear in Release.
- After `CP_Font_Free`, text in fonts loaded after the freed one was drawn
  in the wrong font, or not at all.

## Earlier releases

2.2.0 (October 2025), 2.1.0 (December 2024) and 2.0.0 (November 2021) are
described on the
[GitHub Releases page](https://github.com/DigiPen-Faculty/CProcessing/releases).

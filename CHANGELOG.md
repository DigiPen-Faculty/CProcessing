# Changelog

Notable changes in each CProcessing release. Version numbers are
major.minor.patch. The major number goes up when existing projects may need
changes to build with the new version.

## 3.1.1 (unreleased)

<!-- On hold (2026-11-10): students are using 3.1.0 to finish a course, so
     nothing here is released, and `main` and the live wiki stay as they are.
     This section grows until one larger release with all the changes. -->

### Added

- **Measuring text:** `CP_Font_GetTextWidth`, `CP_Font_GetTextSize` (width and
  line height) and `CP_Font_GetTextBoxSize` (the size of wrapped text), so you
  can center text, right-align it, or draw a box around it.
- **Math helpers:** `CP_Math_Map` (convert a value from one range to another),
  easing functions (`CP_Math_EaseInQuad`, `EaseOutQuad`, `EaseInOutQuad`, the
  same for `Cubic`, and `EaseOutBounce`) for smooth animation, and the
  constants `CP_PI`, `CP_TWO_PI` and `CP_HALF_PI`. For vectors: `CP_Vector_Lerp`,
  `CP_Vector_Rotate`, `CP_Vector_FromAngle` and `CP_Vector_Limit`.
- **Typed text:** `CP_Input_GetTypedText` gives the text the player typed
  during the last frame, as UTF-8 (including accented letters and other
  languages), so a game can ask for a name for a high score.
- **Examples:** eight small, complete programs, each showing one thing
  games need: `movement.c`, `collision.c`, `sprite_animation.c`,
  `camera.c`, `game_states.c`, `score_and_text.c`,
  `music_and_sounds.c` and `text_input.c`. Each is one file with step-by-step comments and
  ideas to try. They're in an `Examples` folder in every release package
  and in the repository; to run one, copy it over `main.c`.
- **Lessons 1 (Your first window) and 3 (Color):** each is a page of
  step-by-step teaching with a finished program in `Examples/Lessons`, and
  challenges and check-yourself questions. The pages are in `docs/lessons`
  until they are published to the wiki.
- **A Color picker demo** (6 in `Processing_Demos`): drag red, green, blue,
  hue, saturation, lightness and alpha sliders and watch the color, its
  code and the colors between two kept colors.
- **New assets in the templates' `Assets` folder**, made for the examples
  and free to use in your own games: a sprite sheet (`SlimeSheet.png`), a
  music loop (`Music.wav`) and three sound effects (`Coin.wav`,
  `Jump.wav`, `Hit.wav`).
- **When a program ends, CProcessing lists the images, sounds and fonts
  it never freed**, with how many loads of each weren't freed, and
  reminds you that every load needs its own free. (It frees them either
  way.) The list goes to the terminal or console and, on Windows, to
  Visual Studio's Output window. With the Debug library, started from
  Visual Studio's debugger, it also appears in a message box. A load in an
  update function shows up with thousands of loads: load once, in an init
  function, and free in the exit function.

### Changed

- **Every CProcessing source file now says it's MIT licensed**, matching
  `LICENSE`: `SPDX-License-Identifier: MIT` and `Copyright (c) 2019-2026
  DigiPen Institute of Technology`. Before, the files said "All rights
  reserved" and the internal headers said "do not distribute", which
  contradicted the license the project has always been published under.
- **The template's `main.c` belongs to the student:** its header has
  places for your name and what the program does, and no longer claims
  DigiPen's copyright. A course that requires its own header can add it.
- **The templates draw something the first time they run:** an 800x600
  window called "My Game" with an orange circle that follows the mouse,
  instead of an empty grey window. Comments explain what each part does.
  The Visual Studio template and the CMake starter now have the same
  `main.c`.
- **The Visual Studio template's solution is now `MyGame.sln`** (it was
  `Processing_Sample.sln`, the same name and project ID as CProcessing's
  own demo solution), and the program it builds is `MyGame.exe`, as in
  the CMake starter. `CProcessingTemplate.zip` now unzips into one folder,
  `CProcessingTemplate`, like the Linux and macOS packages, and has a
  README.
- **The demo program lists its keys in a bar at the bottom of the
  window,** with the current demo's name: 1-6 switch demos, F fullscreen,
  M the mouse cursor mode, C the console, H hides the bar, Escape quits.
  Before, the keys were only in the documentation. The gamepad demo's
  screenshots are now a demo of their own (5, Screenshots), and the
  gamepad demo is now Gamepad and sound.

### Fixed

- **Looping music loaded with `CP_Sound_LoadStream` went silent after its first
  play**, unless it was an Ogg file: `CP_Sound_PlayAdvanced(music, ..., TRUE, ...)`
  played once and then stopped. The cause was in SoLoud, the audio library,
  which used the wrong decoder to jump back to the start. It is fixed in
  every build, and streamed WAV, MP3 and FLAC files now loop.

- **Calling a `CP_Settings_` or `CP_Graphics_` function before
  `CP_Engine_Run` could crash or corrupt memory**, for example
  `CP_Settings_Fill` in `main` before starting the engine. Settings made
  then are now kept and take effect when the window opens, like
  `CP_System_SetWindowSize` and `CP_System_ShowCursor` already did.
  Transforms (`CP_Settings_Translate`, `Rotate`, `Scale`, `ApplyMatrix`,
  `ResetMatrix`), `CP_Settings_Save`/`Restore` and drawing do nothing
  before `CP_Engine_Run`, since there's nothing to draw on yet and
  transforms reset every frame. `CP_Settings_ImageFilterMode` and
  `CP_Settings_ImageWrapMode` now ignore values that aren't one of their
  modes, as the other settings do.
- **`CP_System_SetFrameRate(0)` froze the program.** A frame rate of 0
  or less now means no limit: frames run as fast as the computer allows.
- **Fullscreen always switched the monitor to 60 Hz**, even a 120 or
  144 Hz monitor. It now uses the monitor's highest refresh rate for the
  size. The frame rate is still set by `CP_System_SetFrameRate` (60 by
  default).
- Release builds of CProcessing no longer ask for an OpenGL debug context
  or check for an OpenGL error after every drawing step, which could slow
  drawing down on some graphics drivers. Debug builds still do both.
- **`CP_Random_Noise` was wrong for negative coordinates:** it returned
  values far outside 0 to 1 and jumped at every negative whole number. It
  now gives the same smooth noise there as for positive coordinates.
  Values at positive coordinates are unchanged.
- **`CP_Random_RangeInt(0, 4294967295)`, the whole range, crashed** with
  a division by zero. Large ranges also favored the lower numbers; every
  number in the range is now equally likely. Seeded sequences for
  ordinary ranges are unchanged.
- **Loading the same image, font or sound twice and freeing one of them
  freed it for both.** Loading a file that's already loaded gives back the
  same handle, so two parts of a program that each loaded
  `"Assets/player.png"` shared one image, and the first `CP_Image_Free`
  broke the other. A file now stays loaded until every load of it has been
  freed. `CP_Sound_Load` and `CP_Sound_LoadStream` on the same file also
  gave back whichever was loaded first; they're now separate sounds.
  Since each load now needs its own free, a file loaded every frame in an
  update function and freed once in the exit function stays loaded until
  the program ends, and is in the list of assets not freed (see Added).
- CProcessing no longer exports internal names such as `state`, `p` or
  `_currState`. Built as a static library (`CPROCESSING_BUILD_SHARED=OFF`),
  a game with its own global of the same name failed to link.
- **Text with characters outside plain English, such as ©, é or ü, came out
  wrong on Windows** when typed straight into a string in your code, for
  example `CP_Font_DrawText("Café", ...)`. Visual Studio read source files
  in the computer's own code page, while `CP_Font_DrawText` expects UTF-8.
  The template, the CMake starter and CProcessing's own projects now pass
  `/utf-8`, so such text draws as typed. In an existing Visual Studio
  project, add `/utf-8` under Project Properties > C/C++ > Command Line >
  Additional Options. Linux and macOS already worked.
  ([#15](https://github.com/DigiPen-Faculty/CProcessing/issues/15))

## 3.1.0 (2026-11-07)

macOS is no longer a preview. Drawing, sound, gamepads, screenshots, the
demos and both test tiers have been tested on an Apple silicon Mac
(macOS 26). Intel Macs haven't been tested yet. The downloaded macOS
library still isn't signed by Apple; the starter project's README says how
to open it.

### Added

- **`CP_System_SetCursorMode` and `CP_System_GetCursorMode`**, contributed
  by [@rezarg](https://github.com/rezarg). Besides normal and hidden, the
  cursor can be captured (visible and kept in the window) or disabled
  (hidden, with unlimited movement for mouse look: use
  `CP_Input_GetMouseDeltaX` and `CP_Input_GetMouseDeltaY`, since the mouse
  position keeps going past the window's edges). Like
  `CP_System_ShowCursor`, it can be called before `CP_Engine_Run`.
  `CP_System_ShowCursor` now sets the normal or hidden mode. On macOS a
  captured cursor isn't kept in the window yet, because GLFW doesn't
  support it there.

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
- **Leaving fullscreen could put the window off center or partly off the
  screen** when fullscreen used a size other than the monitor's own, as
  `CP_System_FullscreenAdvanced(1280, 720)` does. The window was centered
  on the fullscreen size before the monitor went back to its normal
  resolution. It's now centered on the monitor's normal resolution, and a
  centered window's title bar now counts too: the whole window is centered
  in the part of the screen the macOS menu bar and Dock, or the Windows
  taskbar, don't cover, so its title bar is never under the menu bar.

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

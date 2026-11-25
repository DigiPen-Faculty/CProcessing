# Changelog

Notable changes in each CProcessing release. Version numbers are
major.minor.patch. The major number goes up when existing projects may need
changes to build with the new version.

## 4.0.0 (unreleased)

<!-- On hold (2026-11-10): students are using 3.1.0 to finish a course, so
     nothing here is released, and `main` and the live wiki stay as they are.
     This section grows until one larger release with all the changes. It was
     planned as 3.1.1; it is 4.0.0 (decided 2026-11-18) because of the
     breaking changes below. The header's CP_VERSION_* and the CMake
     starter's tag stay at 3.1.0 until the release-prep change, which moves
     them to 4.0.0 together with the tag. -->

### Breaking changes

Each change says what to do in an existing project.

- **Everything CProcessing defines now starts with `CP_`.** The key, mouse
  button and gamepad button names, and the function pointer type, were the
  only exceptions. `KEY_*` collided with Linux's own `KEY_*` macros (the
  kernel's `input-event-codes.h`): included first it was a compile error, and
  included after `cprocessing.h` it silently changed what `KEY_A` meant.
  In an existing project, search and replace (whole words only):

  | Old | New |
  |---|---|
  | `KEY_A`, `KEY_SPACE`, `KEY_ANY`, ... | `CP_KEY_A`, `CP_KEY_SPACE`, `CP_KEY_ANY`, ... |
  | `MOUSE_BUTTON_LEFT`, `MOUSE_BUTTON_MIDDLE`, `MOUSE_BUTTON_RIGHT` | `CP_MOUSE_LEFT`, `CP_MOUSE_MIDDLE`, `CP_MOUSE_RIGHT` |
  | `MOUSE_BUTTON_1` to `MOUSE_BUTTON_8`, `MOUSE_BUTTON_LAST` | `CP_MOUSE_1` to `CP_MOUSE_8`, `CP_MOUSE_LAST` |
  | `GAMEPAD_A`, `GAMEPAD_DPAD_UP`, ... | `CP_GAMEPAD_A`, `CP_GAMEPAD_DPAD_UP`, ... |
  | `FunctionPtr` | `CP_FunctionPtr` |

  `TRUE` and `FALSE` are gone too (see the next entry). `DLL_EXPORT` is
  gone (use `CP_API`, which is what it was for); a program never needed it.
- **Yes-or-no values are C's `bool`, not `CP_BOOL`.** Students learn `bool`,
  `true` and `false` from `<stdbool.h>` in their C course, while CProcessing
  had its own `CP_BOOL` (an `unsigned int`) with its own `TRUE` and `FALSE`:
  two ways to say the same thing. Every function that took or returned a `CP_BOOL` now uses `bool`, and
  `cprocessing.h` includes `<stdbool.h>`, so `bool` works without an include
  of your own. In an existing project, search and replace (whole words only):

  | Old | New |
  |---|---|
  | `CP_BOOL` | `bool` |
  | `TRUE` | `true` |
  | `FALSE` | `false` |

  Code that includes `<windows.h>` still gets Windows' own `TRUE` and
  `FALSE`, for Windows functions.
- **`CP_Random_RangeInt` takes and returns an `int`, not an `unsigned int`.**
  A negative bound used to become a number near 4 billion, so
  `CP_Random_RangeInt(-100, 100)` was wrong on every call, with no warning
  (like `CP_Math_ClampInt` and `CP_Color_Create`, it is now an `int`
  function). Calls with bounds from 0 up to 2,147,483,647 work as before
  (the numbers a given seed gives did change: see the seeding entry below).
  What to do: nothing, unless a bound is
  above 2,147,483,647 (it can't be any more) or the result is stored in an
  `unsigned` and printed with `%u` (use `int` and `%d`). `CP_Random_GetInt`
  still returns the 32 raw bits as an `unsigned int`.
- **A seed gives the same numbers, and the same noise, on every computer,
  so the numbers for a given seed are different from before.** The library
  used the C runtime's `srand` and `rand` to start its generator and to
  shuffle the noise, and those differ between Windows, Linux and macOS (for
  `CP_Random_Seed(42)` the first `CP_Random_GetInt()` was 371,199,488 with
  Visual Studio and 2,722,490,258 with glibc). Seeding now uses plain integer
  arithmetic (SplitMix64), and the generator is xoshiro128** instead of
  xorshift128. What to do: nothing, unless a program depends on the exact
  numbers a seed used to give (a layout made with `CP_Random_Seed(5)`, say).
  That layout is now different, but the same on every computer. Unseeded
  programs behave as before (a different start every run).
- **`CP_Color`, `CP_ColorHSL`, `CP_Vector` and `CP_Matrix` are plain
  structs, without the array views.** They were unions, so a color could be
  read as `c.r` or as `c.rgba[0]`. The names are unchanged (`.r .g .b .a`,
  `.h .s .l .a`, `.x .y`, `.m00` to `.m22`) and so are the sizes (4, 8 and 36
  bytes, no padding), so almost no program is affected: the array views were
  in no example, lesson, demo or wiki page. What to do: if a program uses
  `.rgba[i]`, `.hsla[i]`, `.v[i]` or `.m[i][j]`, write the members out, or
  copy them into your own array. The compiler says exactly where. The
  `CP_ANONYMOUS_STRUCT` macro is gone with them.
- **The Visual Studio projects and the CMake builds compile as C11
  (`/std:c11`)** instead of Visual Studio's default C dialect, which is older
  and has Microsoft extensions. Nothing to do for the template or the
  examples. To move an existing Visual Studio project: Project Properties,
  C/C++, Language, *C Language Standard*, ISO C11 (`/std:c11`).
- **`cprocessing_common.h` no longer turns off warnings C4201, C4204 and
  C4090 in your code.** It disabled them for the whole file that included it.
  C4204 and C4090 never appeared in any CProcessing project, so only C4201
  (a struct or union without a name, in your own code) can show up, and only
  in Visual Studio's default C mode: with `/std:c11` a nameless struct is
  standard C and not a warning.
- **`CP_MAX_GAMEPADS` is a macro, not a `static const int`**, so it can size
  an array in C: `int connected[CP_MAX_GAMEPADS];` (a compile error before).
- **`CP_Image_CreateFromData` takes `const CP_Color*`, not `const unsigned char*`**,
  like `CP_Image_GetPixelData` and `CP_Image_UpdatePixelData`: a color for
  each pixel instead of four bytes. In an existing project that passes a byte
  array, change it to `CP_Color pixels[] = { { 255, 0, 0, 255 }, ... };`, or
  cast the pointer: `(const CP_Color*)bytes` (the bytes are r, g, b, a for each
  pixel, so the data is the same).
- **`CP_Color_CreateHex` takes an `unsigned`**, so `0xFF0000FF` is the number
  it looks like and not an out-of-range `int`. Nothing to do in existing
  programs.
- **The editor's hint names things the same way everywhere:** the
  interpolation factor of `CP_Color_Lerp` and `CP_ColorHSL_Lerp` is
  `lerpFactor` (as in `CP_Math_LerpFloat` and `CP_Vector_Lerp`), and the
  parameter names of `CP_Settings_ApplyMatrix`, `CP_Vector_MatrixMultiply`
  and `CP_Matrix_Inverse` in the source now match the header. Nothing to do
  in existing programs. CI checks the header against the source
  (`tools/check_api_names.py`).
- **A sound's volume and pitch combine with its group's, and stay combined.**
  A sound in a group is heard at its own volume times the group's (0.5 in a
  group at 0.5 is heard at 0.25), and plays at its own pitch times the group's.
  Before, `CP_Sound_SetGroupVolume` and `CP_Sound_SetGroupPitch` *replaced*
  the value of every sound already playing in the group, and
  `CP_Sound_SetVolume` and `CP_Sound_SetPitch` ignored the group, so a later
  change of either one threw the other away. Now each sound keeps its own
  volume and pitch: a group set to 0 and back to 1.0 returns every sound to the
  volume it had. Programs that played sounds at 1.0 and changed only the group
  (the examples and lessons) sound the same. A program that set one sound's
  volume or pitch and then the group's relied on the group winning: set the
  sound's own value to what it should be inside the group.
- **The `Assets` folder is now `assets`.** One naming rule now covers the
  repository and the release packages: every folder and file name is
  lowercase, with underscores between words (`CProcessing` and a short list of
  fixed names are the exceptions; see "Naming and layout" in BUILDING.md). The
  template, the examples and the lessons use `assets/`, and the release
  packages have `assets/` and `examples/` (the examples folder is new in this
  release, so only `assets` affects an existing project). Linux and macOS
  care about case, so a program that loads `"Assets/jump.wav"` has to load
  `"assets/jump.wav"` to use the new template's folder; Windows finds either
  spelling. In an existing project, rename the folder and search and replace
  `Assets/` with `assets/` in the paths. The built-in font's path is now
  `./assets/Exo2-Regular.ttf`: a program that loads the font by its old path,
  `./Assets/Exo2-Regular.ttf`, now needs the file in its assets folder.
- **The `*Advanced` draw functions that turned one shape are gone:**
  `CP_Graphics_DrawLineAdvanced`, `DrawRectAdvanced`, `DrawEllipseAdvanced`,
  `DrawTriangleAdvanced`, `DrawQuadAdvanced`, `CP_Image_DrawAdvanced` and
  `CP_Image_DrawSubImageAdvanced`. Each turned around a different point (a
  rectangle around its corner in CORNER mode, an image always around its
  middle, a triangle around the average of its corners), so a student
  couldn't predict where a shape would go, and they couldn't turn two things
  together. Round corners, which only `DrawRectAdvanced` had, are now
  `CP_Graphics_DrawRoundedRect(x, y, w, h, cornerRadius)`. What to do: the
  compiler points at each call. Turn a shape by moving (0, 0) to it, rotating,
  and drawing it at (0, 0):

  ```c
  CP_Settings_Save();
  CP_Settings_Translate(x, y);
  CP_Settings_Rotate(degrees);
  CP_Graphics_DrawRect(0.0f, 0.0f, w, h); // or CP_Image_Draw(image, 0.0f, 0.0f, w, h, alpha)
  CP_Settings_Restore();
  ```

  A rectangle, ellipse or image in the default CENTER mode then turns around
  its middle, as `DrawRectAdvanced` did in CENTER mode. Give a triangle's or
  quad's corners around (0, 0). To turn around another point, draw the shape
  away from (0, 0): `examples/rotation.c` shows how, with helpers to copy.
  `DrawRectAdvanced(x, y, w, h, 0, radius)` becomes
  `CP_Graphics_DrawRoundedRect(x, y, w, h, radius)`.
- **`CP_Graphics_EndShape` always closes the outline**, back to the first
  vertex. It closed it only when the fill was on, so a shape drawn with
  `CP_Settings_NoFill` was missing its last edge, and a filled shape couldn't
  have an open outline. The new `CP_Graphics_EndShapeOpen` leaves the outline
  open (a path, a zigzag, a graph line). What to do: a shape drawn with no fill
  that was meant to be open ends with `CP_Graphics_EndShapeOpen()`.
- **`CP_KEY_ANY` is now -1 (it was 0), and 0 is the new `CP_KEY_NONE`, no
  key.** A global `CP_KEY` that was never set starts at 0, so it used to mean
  every key: a forgotten `CP_KEY jumpKey;` made the player jump whenever any
  key was held. Now it means no key, which is never down.
  `CP_Input_GetKeyTriggered` returns `CP_KEY_NONE` when no key was pressed.
  What to do: nothing in code that uses the names. Code that used the number
  0 for "any key" uses `CP_KEY_ANY`, and a program built against an older
  CProcessing DLL is rebuilt with the new header.
- **The default wrap mode is `CP_IMAGE_WRAP_CLAMP_EDGE`** (it was
  `CP_IMAGE_WRAP_CLAMP`). With CLAMP, an image drawn bigger than it is, with
  the default smooth filtering, faded out over half of one of its pixels at
  every edge: a soft rim on every sprite, and pale lines between tiles drawn
  side by side. CLAMP_EDGE keeps the edges solid; it is also NanoVG's own
  default. `CP_Image_DrawSubImage` with a part that goes past the image now
  shows its edge pixels stretched out there, instead of nothing. What to do:
  nothing for most programs. To get the soft edges, or nothing drawn past the
  image, back, call `CP_Settings_ImageWrapMode(CP_IMAGE_WRAP_CLAMP)`.
- **The repository's folders were reorganized**, so each name says what is in
  it: the library is `CProcessing/` (it was `Processing_Sample/CProcessing/`),
  the solution is `CProcessing.sln`, the tests are
  `tests/tier1`, `tests/tier2` and `tests/unity`, `examples/` (it was
  `Examples/`), the templates are `templates/vs` and `templates/cmake` (were
  `Processing_Empty` and `Processing_Empty_CMake`), and the scripts are in
  `tools/`, named in snake_case (`build_release_package.bat` was
  `BuildReleasePackage.bat`). The bundled libraries (GLAD, GLFW, NanoVG with
  the stb headers, SoLoud, and `vect`) are in `CProcessing/third_party/`, apart
  from CProcessing's own code in `CProcessing/src/` (it was `Source/`). The
  release packages students download are laid out as before, except for the
  folder names in the entry above. Nothing else to do in a program. A fork or
  script that uses the old paths needs them changed. `git blame` and
  `git log --follow` follow files across the move. Old tags and releases keep
  the old layout.

### Added

- **A stack of game states, for pause menus and more:** think of states as
  cards on a table. `CP_Engine_SetNextGameState` clears the table and puts
  down a new card, as it always has: use it for a new part of the game.
  `CP_Engine_PushGameState` puts a card on top of the others: use it when the
  game stays where it is but something comes up, like a pause menu or a
  dialog box. The states underneath stop, without their exits running, and
  stay visible behind the new one. `CP_Engine_PopGameState` takes the card
  off again, and the state underneath carries on where it was, without its
  init running. So a pause menu is a state of its own that shows the frozen
  game behind it, with no flags.
  - `CP_Engine_PopGameState` takes off the state that calls it, with any
    states above it.
  - When the program ends, every state on the stack exits, not only the top
    one.
  - Each state keeps its own drawing settings while another state draws.
  - Only the top state gets input.
  - `CP_Engine_SetGameStatePassThrough` lets a state choose what the states
    under it do, usually in its init:
    - `CP_PASSTHROUGH_VISIBLE`, the default: they stop and stay visible;
    - `CP_PASSTHROUGH_ALL`: they keep running and drawing, without input, for
      a dialog over a world that keeps moving;
    - `CP_PASSTHROUGH_NONE`: they stop and can't be seen, for a page that
      fills the window.
    
    The values are bits (`CP_PASSTHROUGH_UPDATE`, `CP_PASSTHROUGH_VISIBLE`),
    and a state under several gets only what all of them let through.
  - `CP_Engine_GetGameStateDepth`, `CP_Engine_IsTopGameState` and
    `CP_Engine_GetGameStateTime` tell a state about itself.
    `CP_Engine_GetGameStateTime` gives the seconds a state has run, not
    counting time spent stopped.
  - A change asked for in an init or exit function now happens in the next
    frame. Before, it was lost.
- **Drawing into an image:** `CP_Image_Create(width, height)` makes a blank,
  transparent image. Everything drawn between `CP_Image_BeginDrawing(image)`
  and `CP_Image_EndDrawing()` goes into it instead of the window:
  - (0, 0) is the image's top left corner, and one unit is one of its pixels;
  - settings changed inside are put back by `EndDrawing`.
  
  Use it to draw something once and reuse it: a background built from tiles,
  a minimap, a trail that fades, a picture to save as a screenshot.
  `CP_Image_GetPixelData` and `UpdatePixelData` work on these images as on
  any other.
- **`examples/state_stack.c`** shows the stack, pass-through, a HUD drawn by
  a function, and an options page that changes the game under it. **`examples/state_effects.c`**
  shows effects written with ordinary CProcessing functions: easing into
  slow motion under a level-up box (a time scale of your own, and the
  sound's pitch), and turning the music down under a pause, with the
  frozen game blurred or pixelated behind it (a screenshot in the pause's
  init is the picture below it, drawn small and stretched back up).
  **`examples/transitions.c`** animates changes of state (fade, crossfade,
  slides, and one the program draws) with a small helper to copy: it takes
  a screenshot of the old state and draws it over the new one from the
  post-update function. The old state's exit still runs before the new
  state's init. `examples/game_states.c` now pauses
  by pushing a state, not with a flag.
- **Window and frame rate options:** `CP_System_SetWindowResizable` lets the
  player resize the window (drawing follows the new size), `CP_System_SetVSync`
  turns vsync on, `CP_System_IsFullscreen` tells if the window is fullscreen,
  and `CP_System_GetTargetFrameRate` reads back the limit set with
  `CP_System_SetFrameRate` (0 for none). Vsync and resizable can be chosen
  before `CP_Engine_Run`, and each has a getter
  (`CP_System_GetVSync`, `CP_System_GetWindowResizable`).
  `examples/window_options.c` shows them.
- **Reading settings back:** `CP_Settings_GetFill`, `CP_Settings_GetStroke`,
  `CP_Settings_GetStrokeWeight`, `CP_Settings_GetTextSize` and
  `CP_Font_GetCurrent` return what is in use now, so a function can change a
  setting and put it back. `CP_Settings_Save` and `CP_Settings_Restore` bring
  them back like the settings themselves.
- **Arcs, pies and curves:** `CP_Graphics_DrawArc` draws part of a circle
  as a curved line, `CP_Graphics_DrawPie` draws a filled slice of a circle (for
  pie charts and meters), and `CP_Graphics_DrawBezier` draws a smooth curve
  bent by two control points. Angles are in degrees, 0 points right and a
  positive sweep goes clockwise. `examples/curves.c` shows all three, and
  drives a car along a road made of curves, at a steady speed.
- **Layered noise:** `CP_Random_NoiseOctaves` adds several layers of
  `CP_Random_Noise`, each with twice the detail and less strength, for more
  natural hills and clouds.
- **Random numbers and noise, `examples/random.c` and `examples/noise.c`:**
  random.c rolls dice, picks loot (never the same twice in a row), shuffles
  cards, gives chances and weights, spreads shots evenly or in a bell curve,
  and makes the same world from a seed. noise.c puts noise next to random
  numbers, shows the scale, makes a firefly wander and a flame flicker, and
  draws hills, drifting clouds and an island map from a seed. They replace
  the noise hills in curves.c and the noise demo.
- **Particles, `examples/particles.c`:** fireworks, a campfire, two kinds of
  trail (drawn into an image that fades, and made of particles) and dots
  joined by lines, with a glow from `CP_BLEND_ADD`. The particles are an
  array of structs with a count, added and removed without `malloc`, and
  spawned at the same rate at any frame rate. It replaces the dancing lines
  demo and the trail in state_stack.c.
- **Gamepads, `examples/gamepad.c`:** every button, stick and trigger of up
  to 4 gamepads at once, which one the functions without Advanced read,
  what happens to the indexes when one is unplugged, and a player for each
  gamepad. It replaces the Gamepad and sound demo.
- **Sound control, `examples/sound_control.c`:** a pause menu that pauses
  the game's sound groups while its own music and clicks play, pausing
  every sound while the window isn't in front, and a radio whose volume
  falls with distance and whose pitch slows while a key is held (one
  instance, changed as it plays). Resuming a group brings back every sound
  in it, so the example shows how to pause again what the player paused on
  purpose.
- **Fonts, `examples/fonts.c`:** loading a font from the assets folder and
  checking for NULL, choosing it and getting the built-in one back, a
  helper that puts the font and size back after drawing, measuring text at
  several sizes, a button that fits its label, a name that shrinks to fit,
  and a wrapped box with a background that fits it. It loads
  `Exo2-LightItalic.ttf` from the templates' assets.
- **The mouse, `examples/mouse.c`:** a button to copy (`DoButton`) that
  clicks only when it's pressed and let go over it; pressed, held and let
  go for the left, right and middle buttons; click to choose and double
  click to open; drag and drop into slots; the wheel; and whether, how far
  and how fast the mouse moved.
- **Tiles without seams, `examples/tile_seams.c`:** a scrolling tile map
  with a key for each cause of lines between tiles (a camera or a zoom
  between pixels, anti-aliased edges, a sprite sheet's neighbors bleeding
  in, `CP_IMAGE_WRAP_CLAMP`) and each fix (whole pixels, a rounded camera,
  sharp pixels, a padded sheet, anti-aliasing off, overdraw, a shape
  behind, one image fill), with a magnifier from `CP_Image_Screenshot`. The
  wiki has a guide to go with it, "Tiles without seams".
- **Vectors, `examples/vectors.c`:** a turret that turns toward its target
  at a limited speed, the shorter way round (`CP_Vector_AngleCW`); a guard
  that sees the player only in front, within its view and close enough
  (`CP_Vector_DotProduct`, `CP_Vector_Angle`); which side of a road a point
  is on (`CP_Vector_CrossProduct`); and a creature that chases the mouse or
  runs away from it (`CP_Vector_Negate`), turning smoothly
  (`CP_Vector_Lerp`) at a top speed (`CP_Vector_Limit`).
- **Matrices, `examples/matrices.c`:** parts attached to parts. A tank's
  turret turns on it toward the mouse (`CP_Matrix_Inverse`), a drone circles
  it, and shots leave from the end of the gun (`CP_Vector_MatrixMultiply`),
  facing the way they fly (`CP_Matrix_FromVector`). The turret's matrix is on
  screen, with its columns drawn as arrows.
- **Every function but three is now in an example or a lesson.** The ones
  that weren't went where they fit. The three are left out on purpose, and
  the wiki covers them: `CP_System_GetWindowHandle`, `CP_Matrix_Transpose`
  (which `matrices.c` mentions), and `CP_System_FullscreenAdvanced` (see
  `window_options.c` below):
  - `window_options.c`: the display's size and refresh rate, the window in
    the middle of the display, showing and hiding the cursor, the frame
    count and milliseconds, and reading the stroke back.
    `CP_System_FullscreenAdvanced` is left out on purpose: switching the
    monitor to another size is hard on the player. The wiki warns about it,
    and shows instead how to draw a game at 1280x720 into an image and
    stretch it over a fullscreen window.
  - `camera.c`: numbered markers, with an arrow at the edge of the window
    for each one off the screen (`CP_Math_WorldToScreen`), and the part of
    the world the window shows (`CP_Math_ScreenToWorld`).
  - `shapes.c`: sharp, cut-off or round corners (`CP_Settings_LineJointMode`),
    and the points drawn with `CP_Graphics_DrawPoint`.
  - `gradients.c`: bars that go from green to red through brown
    (`CP_Color_Lerp`) and through yellow (`CP_ColorHSL_Lerp`).
  - `sprite_animation.c`: a white or red flash when the slime is hurt: a
    white copy of the sheet drawn over it, fading out, and colored with
    `CP_Settings_Tint` (a tint multiplies, so on the sprite itself it could
    only darken it).
  - `painting.c`: an eyedropper that picks the swatch closest to the color
    under the mouse (`CP_Image_GetPixelData`).
  - `score_and_text.c`: a score that counts up (`CP_Math_LerpInt`).
  - `collision.c`: a circle check without a square root (`CP_Math_Square`).
  - `game_states.c`: F11 for fullscreen in every state
    (`CP_Engine_SetPreUpdateFunction`).
  - `state_stack.c`: the HUD knows when the keys go to the hint
    (`CP_Engine_IsTopGameState`).
  - `gamepad.c`: A on the default gamepad, counted with
    `CP_Input_GamepadTriggered` and `CP_Input_GamepadReleased`.
- **Control one sound while it plays:** `CP_Sound_Play` and
  `CP_Sound_PlayAdvanced` now return a `CP_SoundInstance`, one play of the
  sound (programs that ignore it work as before). `CP_Sound_Stop`,
  `CP_Sound_Pause`, `CP_Sound_Resume`, `CP_Sound_IsPlaying`,
  `CP_Sound_SetVolume` and `CP_Sound_SetPitch` take an instance, so one
  looping song can be stopped without stopping its whole group.
- **Measuring text:** `CP_Font_GetTextWidth`, `CP_Font_GetTextSize` (width and
  line height) and `CP_Font_GetTextBoxSize` (the size of wrapped text), so you
  can center text, right-align it, or draw a box around it.
- **Math helpers:** `CP_Math_Map` (convert a value from one range to another)
  and the constants `CP_PI`, `CP_TWO_PI` and `CP_HALF_PI`. For vectors: `CP_Vector_Lerp`,
  `CP_Vector_Rotate`, `CP_Vector_FromAngle` and `CP_Vector_Limit`.
- **Round corners and open shapes:** `CP_Graphics_DrawRoundedRect` draws a
  rectangle with round corners (a button, a panel), and
  `CP_Graphics_EndShapeOpen` ends a shape without closing its outline.
- **`examples/easing.c`** draws twelve easing functions side by side (quad,
  cubic, sine, back, elastic, bounce, ...) with the code of each to copy.
  Easing functions are a line or two of math on a lerp factor, so they are
  written in a game, not part of the library: lesson 7 has the student write
  them.
- **`examples/rotation.c`** turns shapes around their middle, a corner, any
  point inside them and a point outside them (an orbit), and then puts that
  to use: an arm with two joints, a sprite rocking on its feet and a cannon
  facing the mouse. Its `BeginRotation`, `EndRotation` and `DrawRectRotated`
  helpers save writing Save, Translate, Rotate and Restore each time.
- **Typed text:** `CP_Input_GetTypedText` gives the text the player typed
  during the last frame, as UTF-8 (including accented letters and other
  languages), so a game can ask for a name for a high score.
- **Which key was pressed:** `CP_Input_GetKeyTriggered` returns the key
  pressed this frame, or `CP_KEY_NONE`; if several were, the one pressed
  first. For a menu where players choose their own controls:
  `examples/controls.c` shows one.
- **Gradients:** `CP_Settings_FillLinearGradient` and
  `CP_Settings_FillRadialGradient` fill shapes with colors that blend along a
  line or out from a point: a sky, a glow, a lit button, a health bar. A
  gradient is a fill, like `CP_Settings_Fill`, and works on every filled
  shape. Its points are placed when each shape is drawn, so it moves and turns
  with the shapes; radial sizes are diameters, as in `CP_Graphics_DrawCircle`.
  Text and outlines keep plain colors (text the gradient's first color, which
  `CP_Settings_GetFill` returns). `examples/gradients.c` draws a sunset.
- **Clipping:** `CP_Settings_Clip(x, y, w, h)` cuts off everything drawn
  outside a rectangle, until `CP_Settings_NoClip`: a list that scrolls inside
  a panel, text kept inside a dialog box, a minimap, a wipe. The rectangle
  follows `CP_Settings_RectMode`, like `CP_Graphics_DrawRect`, and is placed
  when it's set, so drawing moved by a later Translate moves under it. A new
  clip replaces the old one, Save and Restore keep it, and like the transform
  it's undone at the start of every frame and in each game state.
  `examples/clipping.c` scrolls an inventory and draws a minimap.
- **Curves and holes in shapes:** between `CP_Graphics_BeginShape` and
  `CP_Graphics_EndShape`, `CP_Graphics_AddQuadraticVertex` adds a curved side
  that bends toward one control point, and `CP_Graphics_AddBezierVertex` one
  with two control points, like `CP_Graphics_DrawBezier`. After the whole
  outline, each `CP_Graphics_BeginHole`...`CP_Graphics_EndHole` cuts a hole,
  whichever way round its points go. The outline can't go on after a hole: its
  points there are left out, with a message, and `CP_Graphics_AddVertex` outside a shape now
  does nothing (it used to add to whatever was drawn last).
  `examples/shapes.c` draws stars, a speech bubble, a heart, a waving flag, a
  blob through points, an arched window and a donut; the wiki has a new guide,
  "Your own shapes".
- **Text effects:** `CP_Settings_TextBlur` blurs text, in whole pixels up to
  20. Text drawn blurred and then sharp on top gets a glow, or a shadow when
  the blurred copy is dark and moved. `CP_Settings_TextLetterSpacing` adds
  pixels between letters, and `CP_Settings_TextLineSpacing` between the lines
  of a text box; the measuring functions count both. `examples/text_effects.c`
  has a neon sign, a drop shadow, a title that comes in from wide spacing, and
  a dialog box that fits its text. Letters that the font's kerning pulls
  together are now placed to the nearest pixel, so some pairs, like "AV", sit
  a pixel closer than before.
- **Image fills:** `CP_Settings_FillImage(img, x, y, w, h, alpha)` fills the
  shapes after it with tiles of an image: one tile at (x, y), w by h, repeated
  in every direction. It works on any shape (circles, rounded rectangles,
  your own shapes with curves and holes), and the tiles move, turn and grow
  with the transform, so they stay put in a world the camera moves through.
  Moving (x, y) scrolls them. Unlike drawing an image many times, it leaves
  no seams between the tiles. Text keeps the last plain fill color, and a
  NULL or freed image fills nothing. `examples/tiled_fill.c` has hills of
  dirt, brick walls with windows, a moving platform, clouds, water in a
  porthole, a striped loading bar and pixel art, with every tile drawn in
  code.
- **Saving images:** `CP_Image_Save(img, "picture.png")` saves any image as
  a PNG file: one loaded, one drawn into, or a screenshot. It returns
  `true` when the file was written, and when it can't save (no image, a
  name that doesn't end in `.png`, a folder that isn't there) it says why
  and returns `false`. A name without a folder goes in the current folder.
  `examples/painting.c` is a paint program that saves your painting,
  loads it back the next time it starts, and saves numbered screenshots.
  The PNG writing is stb_image_write, from the same stb commit as the
  image loading.
- **Examples:** eight small, complete programs, each showing one thing
  games need: `movement.c`, `collision.c`, `sprite_animation.c`,
  `camera.c`, `game_states.c`, `score_and_text.c`,
  `music_and_sounds.c` and `text_input.c`. Each is one file with step-by-step comments and
  ideas to try. They're in an `examples` folder in every release package
  and in the repository; to run one, copy it over `main.c`.
- **Finding an example:** `examples/README.md` groups every example by what
  you want to make (drawing, text, moving and controls, pictures, sound,
  game structure, random and noise, game feel, the window), has a "Looking for..." table of
  tricks that live inside bigger examples (a health bar, a minimap, slow
  motion, a slider), and pairs each lesson with its example. A new wiki page,
  Examples, lists the same, each API page on the wiki names the examples
  that use it, and each example's header ends with a "See also" line. CI
  checks that every example is listed (`tools/check_examples.py`, and the
  wiki audit for the Examples page).
- **Lessons 1 to 11 (Your first window, Shapes and the coordinate system, Color, Input, Moving with vectors and delta time, Collision, Images and animation, Sound, Game states, a Capstone game, and an extra on writing your own state manager):** each is a page of
  step-by-step teaching with a finished program in `examples/lessons`, and
  challenges and check-yourself questions. The pages are in `docs/lessons`
  until they are published to the wiki.
- **A color picker, `examples/color_picker.c`**, the companion to lesson 3:
  drag red, green, blue, hue, saturation, lightness and alpha sliders and
  watch the color, its code (including the eight-digit number for
  `CP_Color_CreateHex`) and the colors between two kept colors.
- **New assets in the templates' `assets` folder**, made for the examples
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

- **The headers explain what the names can't.** Hover over a function in
  Visual Studio to see its note: a range (`CP_Vector_Angle` is 0 to 180), a
  hidden rule (`CP_Color_CreateHex` needs all 8 digits, `0xRRGGBBAA`), or a
  difference from the function beside it. Each section of `cprocessing.h`
  starts with what holds for all of it: units, which way angles turn, how long
  things last. Three functions have clearer parameter names, which doesn't
  change any program: `CP_Image_DrawSubImage` takes the part of the image as
  `left, top, right, bottom` (pixels, not 0 to 1), and
  `CP_System_SetWindowSize` and `CP_Image_CreateFromData` take
  `width, height`.
- **Frames come on time, with much less work for the computer.** The frame
  limiter slept 1 ms at a time and spun when it got close, but on Windows a
  short sleep often lasts a whole system timer tick (about 15.6 ms). Drawing
  one circle at 60 frames per second used 43% of a CPU core, ran at 58.8
  frames per second, and 1 frame in 12 came more than 10% late. Now frames are
  due on a fixed schedule (so the average is exactly the target), the program
  sleeps with a high-resolution timer until just before the next frame is due,
  and with vsync on and a limit at or above the monitor's refresh rate the
  monitor sets the pace. The same circle runs at 60.00 frames per second, with
  1 late frame in 600, on 17% of a core (11% with vsync).
- **`CP_System_GetDt` is never more than 0.1 seconds.** The frame after a
  breakpoint, a window drag (Windows stops the program while its window is
  dragged) or a long load was seconds long, so anything moved by speed times
  dt jumped, even through walls. Below 10 frames per second the game now slows
  down instead. `CP_System_GetSeconds` and `GetMillis` still tell the real
  time; `CP_Engine_GetGameStateTime` adds up dt, so it follows the game.
- **`CP_System_GetFrameRate` is averaged over half a second,** so a frame rate
  drawn on the screen can be read instead of flickering every frame.
- **`CP_Random_Noise` and `CP_Random_NoiseOctaves` use the whole range from 0
  to 1, and are no longer stuck at 0.5 on whole numbers.** Perlin noise is 0
  at every whole-number point, so `CP_Random_Noise(x, 0.0f, 0.0f)` was exactly
  0.5 whenever x was a whole number (a flat line for
  `CP_Random_Noise(frameCount, 0, 0)`), and with y and z left at 0 it only went
  from 0.25 to 0.75. More octaves narrowed it further (four spanned about 0.23
  to 0.77). Now every point is moved off that grid by a fixed amount, the
  result is stretched so most of 0 to 1 is used (the rare values beyond are
  clamped), and the octaves keep the spread of one layer. The values for a
  seed are different, so hills, clouds and maps made with noise look a little
  bolder. What to do: nothing; if something drawn from noise now reaches too
  far, multiply by less.

- **The templates build C++ as well as C.** A game can be written in C++ by
  renaming `main.c` to `main.cpp`.
  - The Visual Studio template no longer forces every file to compile as C:
    each file is built by its extension, so a `.cpp` file is C++.
  - The CMake starter builds every `.c` and `.cpp` file in its folder, and
    turns C++ on when there is a `.cpp` file. New files no longer need to be
    added to `CMakeLists.txt`.
  - Both READMEs say how, and what C++ needs that C doesn't (a cast to make
    a key from a number, or after `malloc`).
- **`CP_Image_CreateFromData` and `CP_Image_UpdatePixelData` take `const`
  data,** since they only read it. C++ can now pass a `const` array. Nothing
  to do in existing programs.
- **`CP_Graphics_DrawCircle`, `DrawArc` and `DrawPie` name their size
  `diameter`, not `d`**, so the editor's hint says what the number is (it is
  the width across, not the radius). Only the parameter name changed: nothing
  to do in existing programs.
- **Every CProcessing source file now says it's MIT licensed**, matching
  `LICENSE`: `SPDX-License-Identifier: MIT` and `Copyright (c) <years>
  DigiPen Institute of Technology and CProcessing contributors`. This covers
  the library, the examples and lessons, the tests and the
  scripts (CI checks it with `tools/check_license_headers.py`); the bundled
  libraries keep their own licenses, and the built-in font data says
  `OFL-1.1`. Before, the library's files said "All rights reserved" and the
  internal headers said "do not distribute", which contradicted the license
  the project has always been published under. An example you copy over
  `main.c` keeps its two header lines (the MIT license asks for the notice to
  stay with copies of the code); your own changes are yours.
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
- **The demo program is gone, and its demos became examples.** It wasn't
  in the release packages, so students never had it. Now everything it
  showed is in an example that ships: `color_picker.c`, `noise.c`,
  `particles.c` (from the dancing lines), `gamepad.c`, and
  `window_options.c`, which gained the mouse cursor modes and the console
  from the demo program's keys. The grid wave is dropped, and the
  screenshot magnifier is in the wiki's `CP_Image_Screenshot` example.
  `demos/`, `CProcessing_Demos` (it was `Processing_Demos`) and the
  `CPROCESSING_BUILD_DEMOS` CMake option are removed. The Exo 2 font files
  and their license moved to the templates' assets
  (`templates/vs/assets`), where a program can load them.
- **Less inside the library, with no change to how it works:** TinyCThread
  (1,400 lines, used for one 1 millisecond sleep) is gone, the seven
  copies of "turn about a point" are one function, and dead code is out.
  CI builds the whole Visual Studio solution in Release as well as Debug,
  and a new test fails if a test is written but not run.
- **One code style, checked in CI.** The library, examples, templates and
  tests are formatted with clang-format (`.clang-format`: tabs, Allman
  braces), and a `format` job fails a pull request that isn't. The reformat
  is one commit that only moved whitespace and braces, listed in
  `.git-blame-ignore-revs` so `git blame` skips it. The test projects' 4-space
  indent is now tabs, like everything else.

### Fixed

- **A shape drawn after an image, with no `CP_Settings_Fill` in between, is
  filled with the fill color again.** Drawing an image left the drawing's
  fill set to the image, so the shape showed part of the image or nothing at
  all, and text after an image came out white.
- **`CP_Vector_Angle`, `CP_Vector_AngleCW` and `CP_Vector_AngleCCW` returned
  NaN for vectors pointing the same way** (about one pair in five, depending on
  the numbers) and for a zero vector, so something turned to face its target
  could vanish. They now use the arc tangent of the cross and dot products,
  which is exact for those vectors; a zero vector gives 0.
- **Colors lost a step when mixed or converted:** `CP_Color_Lerp` cut the
  fraction off instead of rounding, so its results were half a step too dark
  on average, and mixing a color with itself could change it (3 became 2).
  `CP_ColorHSL_FromColor` and `CP_Color_FromColorHSL` cut off too, so no color
  came back exactly from RGB to HSL and back, and some came back 9 steps off;
  now most come back within 1 or 2 (whole-number HSL can't hold every RGB
  color). `CP_Math_LerpInt` rounds as well, the same way for negative numbers.
- **`CP_ColorHSL_Lerp` went the long way round the color wheel:** from hue
  350 to hue 10 it passed through cyan (180); it now passes through red (0).
- **`CP_Matrix_Inverse` of a matrix with no inverse** (a scale of 0, say)
  divided by 0 and returned NaN; it returns the identity matrix.
- **`CP_Random_GetFloat` and `CP_Random_RangeFloat` no longer return their
  upper bound.** `GetFloat` returned exactly 1.0 for 1 draw in 33 million, so
  `RangeFloat(0, 5)` could return 5.0 and `(int)CP_Random_RangeFloat(0, 5)`
  could be 5, one past the end of a 5-element array. `GetFloat` is now 0.0
  up to, but not including, 1.0, and `RangeFloat` is the lower bound up to,
  but not including, the upper bound. Every other value is unchanged for
  `GetFloat`. `RangeFloat` is now worked out in double and rounded once, so
  it gives the same number whether or not the compiler fuses the multiply and
  the add (about a quarter of results differed in the last bit between
  compilers that do and don't).
- **Seeding no longer disturbs the program's own `rand()`,** `CP_Random_Seed(0)`
  and `CP_Random_Seed(1)` are no longer the same sequence with glibc (which
  treats `srand(0)` as `srand(1)`), and **`CP_Random_Gaussian` repeats after
  reseeding with the same seed:** it keeps the second number of each pair,
  and that used to survive a reseed and come out first.
- **`cprocessing_common.h` no longer causes warnings in C++ with
  `-Wpedantic`, or in Visual Studio at warning level 4.** The color, vector
  and matrix types were unions with an unnamed struct, which C++ doesn't
  allow (and Visual Studio's default C mode warns about), so GCC and Clang
  gave 8 warnings for every file that included it, and a build with
  `-Wpedantic -Werror` failed. They are plain structs now (see Breaking
  changes), so the header needs no warning pragmas, no `__extension__` and
  no macro.
- **C++ is checked in CI.** Every example, lesson and template
  `main.c` is also compiled as C++11, with `-Wpedantic` or `/permissive-`,
  on Windows, Linux and macOS. A small C++ program links with CProcessing
  and runs with the Tier 1 tests.
- **A window size chosen in `game_init` now applies before the first
  `game_update`.** It used to apply one frame later, so the first update still
  saw a 400 x 400 window: `CP_System_GetWindowWidth()` was wrong, and a
  program that draws once (a painting program, say) filled only the top left
  corner. Programs that redraw every frame never noticed.
- **Looping music loaded with `CP_Sound_LoadStream` went silent after its first
  play**, unless it was an Ogg file: `CP_Sound_PlayAdvanced(music, ..., true, ...)`
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
- Release builds of CProcessing no longer check for an OpenGL error after
  every drawing step, which could slow drawing down on some graphics
  drivers. Debug builds still do.
- **In Debug builds, fullscreen at another size usually went back to the
  monitor's own size.** On Intel Iris Xe graphics,
  `CP_System_FullscreenAdvanced(1280, 720)` switched the monitor, then
  Windows switched it back to 2560 × 1600 by itself, with the window
  stretched and frames stalling for seconds. The OpenGL debug context that
  Debug builds asked for made this far more likely, and CProcessing never
  used it (there was no debug message callback), so no build asks for one
  now. It can still happen now and then on that hardware, in GLFW itself,
  without CProcessing.
- **`CP_Random_Noise` was wrong for negative coordinates:** it returned
  values far outside 0 to 1 and jumped at every negative whole number. It
  now gives the same smooth noise there as for positive coordinates.
  Values at positive coordinates are unchanged.
- **`CP_Random_RangeInt` with the whole range crashed** with a division by
  zero (it was `(0, 4294967295)`; it is now `(INT_MIN, INT_MAX)`). Large
  ranges also favored the lower numbers; every number in the range is now
  equally likely.
- **Loading the same image, font or sound twice and freeing one of them
  freed it for both.** Loading a file that's already loaded gives back the
  same handle, so two parts of a program that each loaded
  `"assets/player.png"` shared one image, and the first `CP_Image_Free`
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
  `/utf-8`, and the template and starter come with an `.editorconfig` that
  makes Visual Studio save C files as UTF-8 too (without it, typing © into
  `main.c` saved it in the code page, and the build stopped with warning
  C4828). So such text draws as typed. In an existing Visual Studio project,
  add `/utf-8` under Project Properties > C/C++ > Command Line > Additional
  Options, and copy the template's `.editorconfig` next to the project (or
  use File > Save As, the arrow on the Save button, Save with Encoding,
  "Unicode (UTF-8 without signature)"). Linux and macOS already worked.
  ([#15](https://github.com/DigiPen-Faculty/CProcessing/issues/15))
- **stb_image and stb_truetype, which load images and fonts, are updated to
  2.32 and 1.28** (from 2.30 and 1.26), unmodified. Since then upstream fixed
  a font that has only a symbol character map showing nothing, a division by
  zero and a few out-of-range reads on damaged fonts and images, a memory leak
  when an image conversion ran out of memory, two undefined-behavior cases,
  HDR images reconstructed with the wrong values, and a slow path in
  progressive JPEGs. Nothing to do in a program.

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
  in `CProcessing/third_party/nanovg/CPROCESSING.md`. The image and
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

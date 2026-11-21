# CProcessing examples

Small, complete programs, each showing one thing games need. Every example
is one file, with comments that explain each step, and ideas to try at the
top.

| Example | What it shows |
|---|---|
| [movement.c](movement.c) | Moving a player with WASD, the arrow keys or a gamepad stick, at the same speed at any frame rate. |
| [collision.c](collision.c) | Checking whether points, circles and rectangles touch. |
| [sprite_animation.c](sprite_animation.c) | Animating a character from the frames of a sprite sheet. |
| [camera.c](camera.c) | A camera that moves and zooms around a world bigger than the window. |
| [game_states.c](game_states.c) | A menu, play, pause and game over, each a game state of its own. The pause goes on top of the frozen game. |
| [state_stack.c](state_stack.c) | Going further with game states: menus on top of menus, a hint over a game that keeps running, a HUD, and a trail drawn into an image. |
| [transitions.c](transitions.c) | Animated changes between game states: a fade, a crossfade, slides and one you draw, with a small helper to copy into your own game. |
| [state_effects.c](state_effects.c) | Easing into slow motion under a level-up box, and a pause that turns the music down and blurs the game behind it, with a few lines of your own code. |
| [score_and_text.c](score_and_text.c) | A score, a timer and other text, placed and sized on the screen. |
| [music_and_sounds.c](music_and_sounds.c) | Looping music and sound effects, each with its own volume. |
| [text_input.c](text_input.c) | Typing a name: reading typed characters, Backspace, Enter and a blinking cursor. |
| [controls.c](controls.c) | Letting the player choose their own keys: an array of keys for the actions, CP_Input_GetKeyTriggered to find the key pressed, and each key's name on the screen. |
| [easing.c](easing.c) | Twelve easing functions drawn side by side, each with its code to copy: make movement start slow, arrive gently, overshoot or bounce. |
| [rotation.c](rotation.c) | Turning shapes and sprites around any point: their middle, a corner, a pin anywhere, an orbit, joints, and aiming at the mouse, with helpers to copy. |
| [text_effects.c](text_effects.c) | Glowing and shadowed text: a neon sign, a title with a drop shadow, a halo that keeps small text readable, a title that comes in from wide letter spacing, and a dialog box that fits its text as the line spacing changes. |
| [shapes.c](shapes.c) | Your own shapes: stars and polygons from a loop, a speech bubble and a waving flag with curves, a heart, a blob through points, and an arched window and a donut with holes. Space shows the points and control points. |
| [clipping.c](clipping.c) | Drawing kept inside a box: an inventory that scrolls in its panel, a minimap, ability icons that refill as their cooldowns run out, and a title that wipes in. |
| [gradients.c](gradients.c) | A sunset over the hills in gradients: a three-color sky, a glowing sun, a lit button, a health bar that turns red as it empties, and a flashlight. |
| [curves.c](curves.c) | A pie chart, a loading ring, a Bezier curve with draggable handles, and hills made of layered noise. |
| [window_options.c](window_options.c) | Fullscreen, a resizable window, vsync and the frame rate limit, with keys to change them. |

## Game states: what the engine does, and what you write

Think of game states as cards on a table. `CP_Engine_SetNextGameState`
clears the table and puts down a new card: a new part of the game.
`CP_Engine_PushGameState` puts a card on top, like a pause menu, and the
cards under it stay, frozen; `CP_Engine_PopGameState` takes it off.

The engine does what you couldn't do yourself, or would easily get wrong:
the stack, keeping each state's picture and settings, giving input to the
top state only, and drawing into images. Effects like slow motion, turning
the sound down under a pause, a HUD, or a transition from one state to the
next are a few lines of your own code, so you can make them work however
you want: [state_effects.c](state_effects.c),
[transitions.c](transitions.c) and [state_stack.c](state_stack.c) show how.

## Lessons

The `lessons` folder has the finished program for each lesson, one folder
each (`01_first_window` to `11_state_manager`), with a `main.c` to copy over your own.
The lessons themselves are on the wiki.

## Running an example

Each example is a whole program, with its own `main`. To run one, copy it
over your project's `main.c` (keep a copy of your own first), then build
and run.

The examples load files from the `assets` folder that comes with the
templates: `SlimeSheet.png`, `Music.wav`, `Coin.wav`, `Jump.wav` and
`Hit.wav`. They were made for these examples, and you're free to use them
in your own games.

## In the CProcessing repository

The CMake build builds every example, as `Example_movement`,
`Example_collision` and so on, and each lesson, as `Lesson_01_first_window`
and so on, into `build/bin`. The examples are in the
release packages, in an `examples` folder next to `main.c`.

The assets are made by
[`tools/make_example_assets.py`](https://github.com/DigiPen-Faculty/CProcessing/blob/main/tools/make_example_assets.py),
drawn and synthesized from code.

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
| [game_states.c](game_states.c) | A menu, play, pause and game over, each a game state of its own. |
| [score_and_text.c](score_and_text.c) | A score, a timer and other text, placed and sized on the screen. |
| [music_and_sounds.c](music_and_sounds.c) | Looping music and sound effects, each with its own volume. |

## Running an example

Each example is a whole program, with its own `main`. To run one, copy it
over your project's `main.c` (keep a copy of your own first), then build
and run.

The examples load files from the `Assets` folder that comes with the
templates: `SlimeSheet.png`, `Music.wav`, `Coin.wav`, `Jump.wav` and
`Hit.wav`. They were made for these examples, and you're free to use them
in your own games.

## In the CProcessing repository

The CMake build builds every example, as `Example_movement`,
`Example_collision` and so on, into `build/bin`. The examples are in the
release packages, in an `Examples` folder next to `main.c`.

The assets are made by
[`tools/make-example-assets.py`](https://github.com/DigiPen-Faculty/CProcessing/blob/main/tools/make-example-assets.py),
drawn and synthesized from code.

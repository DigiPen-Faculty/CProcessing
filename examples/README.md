# CProcessing examples

Small, complete programs, each about one thing a game needs. Every example
is one file, with comments that explain each step, and ideas to try at the
top. To run one, copy it over your project's `main.c` (see
[Running an example](#running-an-example)).

Find an example by what you want to make in the groups below, or look for a
particular trick in [Looking for...](#looking-for), since many examples show
more than their title.

## Drawing

| Example | What it shows |
|---|---|
| [shapes.c](shapes.c) | Your own shapes: stars and polygons from a loop, a speech bubble and a waving flag with curves, a heart, a blob through points, and an arched window and a donut with holes. Space shows the points and control points. |
| [curves.c](curves.c) | A pie chart, a loading ring, a Bezier curve with draggable handles, and a car that drives a winding road made of curves, facing the way it goes, at a steady speed. |
| [rotation.c](rotation.c) | Turning shapes and sprites around any point: their middle, a corner, a pin anywhere, an orbit, joints, and aiming at the mouse, with helpers to copy. |
| [gradients.c](gradients.c) | A sunset over the hills in gradients: a three-color sky, a glowing sun, a lit button, a health bar that turns red as it empties, and a flashlight. |
| [tiled_fill.c](tiled_fill.c) | Shapes filled with tiles of an image: hills of dirt, brick walls with windows, a moving platform, drifting clouds, water in a porthole, a striped loading bar and pixel art, with every tile drawn in code. |
| [clipping.c](clipping.c) | Drawing kept inside a box: an inventory that scrolls in its panel, a minimap, ability icons that refill as their cooldowns run out, and a title that wipes in. |
| [color_picker.c](color_picker.c) | The companion to lesson 3: mix a color from red, green and blue or from hue, saturation and lightness, make it see-through, see the colors between two, and get its code. The sliders are a recipe of their own. |

## Text

| Example | What it shows |
|---|---|
| [score_and_text.c](score_and_text.c) | A score, a timer and other text, placed and sized on the screen. |
| [text_effects.c](text_effects.c) | Glowing and shadowed text: a neon sign, a title with a drop shadow, a halo that keeps small text readable, a title that comes in from wide letter spacing, and a dialog box that fits its text as the line spacing changes. |
| [text_input.c](text_input.c) | Typing a name: reading typed characters, Backspace, Enter and a blinking cursor. |

## Moving and controls

| Example | What it shows |
|---|---|
| [movement.c](movement.c) | Moving a player with WASD, the arrow keys or a gamepad stick, at the same speed at any frame rate. |
| [collision.c](collision.c) | Checking whether points, circles and rectangles touch. |
| [camera.c](camera.c) | A camera that moves and zooms around a world bigger than the window. |
| [controls.c](controls.c) | Letting the player choose their own keys: an array of keys for the actions, CP_Input_GetKeyTriggered to find the key pressed, and each key's name on the screen. |
| [gamepad.c](gamepad.c) | Gamepads: every button, stick and trigger of up to 4 at once, which one the functions without Advanced read, gamepads connecting and disconnecting, and a player for each. |

## Pictures

| Example | What it shows |
|---|---|
| [sprite_animation.c](sprite_animation.c) | Animating a character from the frames of a sprite sheet. |
| [painting.c](painting.c) | A paint program: paint into an image with the mouse, save it as a PNG file and pick it up again the next time, and save numbered screenshots. |

## Sound

| Example | What it shows |
|---|---|
| [music_and_sounds.c](music_and_sounds.c) | Looping music and sound effects, each with its own volume. |
| [sound_control.c](sound_control.c) | Controlling sound in a game: a pause menu with music of its own while the game's sounds wait, pausing everything when the window is left, and one playing sound's volume and pitch changed as it plays. |

## Game structure

| Example | What it shows |
|---|---|
| [game_states.c](game_states.c) | A menu, play, pause and game over, each a game state of its own. The pause goes on top of the frozen game. |
| [state_stack.c](state_stack.c) | Going further with game states: menus on top of menus, a hint over a game that keeps running, a HUD, and an options page that changes the game under it. |
| [transitions.c](transitions.c) | Animated changes between game states: a fade, a crossfade, slides and one you draw, with a small helper to copy into your own game. |
| [state_effects.c](state_effects.c) | Easing into slow motion under a level-up box, and a pause that turns the music down and blurs the game behind it, with a few lines of your own code. |

## Random and noise

| Example | What it shows |
|---|---|
| [random.c](random.c) | Random numbers for games: dice and a coin, loot that's never the same twice in a row, a shuffle, a critical hit chance and rare loot, shots spread evenly or in a bell curve, and a world made again from its seed. |
| [noise.c](noise.c) | Random that changes smoothly: noise next to random numbers, the scale, a wandering firefly and a flickering flame, scrolling hills from layers of noise, drifting clouds drawn into pixels, and an island map from a seed. |

## Game feel

| Example | What it shows |
|---|---|
| [easing.c](easing.c) | Twelve easing functions drawn side by side, each with its code to copy: make movement start slow, arrive gently, overshoot or bounce. |
| [particles.c](particles.c) | Fireworks, a campfire with smoke and sparks, a trail drawn into an image next to one made of particles, and dots joined by lines when they come close, with a glow you can switch on and off. |

## Window

| Example | What it shows |
|---|---|
| [window_options.c](window_options.c) | Fullscreen, a resizable window, vsync, the frame rate limit, the mouse cursor modes and a console for printf, with keys to change them. |

## Looking for...

Tricks that live inside an example about something bigger.

| To make | Look in |
|---|---|
| A button that lights up under the mouse | [gradients.c](gradients.c) (`DrawButton`) |
| A click on a button | [collision.c](collision.c) (a point in a box) |
| A slider the mouse drags (volume, options) | [color_picker.c](color_picker.c) (`DoSliders`) |
| Dragging things with the mouse | [curves.c](curves.c) (handles), [clipping.c](clipping.c) (a list), [rotation.c](rotation.c) |
| A health bar | [gradients.c](gradients.c) (green to red as it empties) |
| A cooldown icon that refills | [clipping.c](clipping.c) |
| A minimap | [clipping.c](clipping.c) |
| A scrolling list or inventory | [clipping.c](clipping.c) |
| A loading bar or spinner | [tiled_fill.c](tiled_fill.c) (striped bar), [curves.c](curves.c) (ring) |
| A pie chart | [curves.c](curves.c) |
| Something that follows a path: a car on a road, an enemy's flight path | [curves.c](curves.c) (`BezierPoint`, `BezierDirection`) |
| A smooth path through points | [curves.c](curves.c) (`RoadCurve`), [shapes.c](shapes.c) (`AddSmoothVertices`) |
| A glow, a flashlight, a sky | [gradients.c](gradients.c) |
| Hills or terrain from noise | [noise.c](noise.c) (layers of noise, scrolling), [gradients.c](gradients.c), [tiled_fill.c](tiled_fill.c) |
| Clouds, a map or an island from noise | [noise.c](noise.c) |
| Something that wanders, a flame that flickers | [noise.c](noise.c) |
| Drawing pixel by pixel into an image | [noise.c](noise.c) (`CP_Image_UpdatePixelData`) |
| A wobbling blob | [shapes.c](shapes.c) |
| Brick, grass, water or other patterns | [tiled_fill.c](tiled_fill.c) |
| A moving platform | [tiled_fill.c](tiled_fill.c) |
| Pixel art from a little text map | [tiled_fill.c](tiled_fill.c) (`MakePixelArt`) |
| Aiming at the mouse, an orbit, an arm with joints | [rotation.c](rotation.c) |
| A camera that follows and zooms; clicking in a moved world | [camera.c](camera.c) |
| Text that stays put while the world moves | [camera.c](camera.c) (`CP_Settings_ResetMatrix`) |
| A score and a timer | [score_and_text.c](score_and_text.c) |
| Centered or right-aligned text, wrapped text | [score_and_text.c](score_and_text.c) |
| A dialog box that fits its text | [text_effects.c](text_effects.c) |
| Glowing text, a drop shadow | [text_effects.c](text_effects.c) |
| Typing a name, a blinking cursor | [text_input.c](text_input.c) |
| Rebinding keys, showing a key's name | [controls.c](controls.c) |
| Movement that's the same speed diagonally | [movement.c](movement.c) |
| Two to four players, a gamepad each | [gamepad.c](gamepad.c) (the Advanced functions) |
| Noticing a gamepad plugged in or unplugged | [gamepad.c](gamepad.c) |
| A trigger that's pressed part way, like a throttle | [gamepad.c](gamepad.c) (faster players) |
| A sprite sheet animation | [sprite_animation.c](sprite_animation.c) |
| Drawing that stays from frame to frame | [painting.c](painting.c), [particles.c](particles.c) (a trail that fades) |
| Saving a picture or screenshot; loading it next time | [painting.c](painting.c) |
| Numbered file names (`screenshot_1.png`...) | [painting.c](painting.c) (`snprintf`) |
| Music that loops; effects with their own volume | [music_and_sounds.c](music_and_sounds.c) |
| A sound that sounds different each time | [music_and_sounds.c](music_and_sounds.c) (pitch) |
| A pause menu with its own music and clicks | [sound_control.c](sound_control.c) (`CP_Sound_PauseGroup`) |
| Sound that pauses when the player switches windows | [sound_control.c](sound_control.c) (`CP_Sound_PauseAll`) |
| A sound that's quieter far away | [sound_control.c](sound_control.c) (`CP_Sound_SetVolume`) |
| A coin sound that rises with each coin in a row | [sound_control.c](sound_control.c) |
| A pause menu | [game_states.c](game_states.c), [state_stack.c](state_stack.c) (with options and "really quit?") |
| A HUD over the game | [state_stack.c](state_stack.c) |
| Restarting a level; keeping the best score | [game_states.c](game_states.c) |
| A fade or slide between screens | [transitions.c](transitions.c) |
| Slow motion | [state_effects.c](state_effects.c) |
| Music that goes quiet under a pause | [state_effects.c](state_effects.c) |
| A blurred or pixelated game behind a menu | [state_effects.c](state_effects.c) |
| Dice, a coin flip | [random.c](random.c) |
| Random loot, never the same twice in a row | [random.c](random.c) (`PickLoot`) |
| Shuffling cards or a playlist | [random.c](random.c) (`Shuffle`) |
| A critical hit chance; rare and common loot | [random.c](random.c) (`PickWeighted`) |
| Bullet spread, aim that wobbles | [random.c](random.c) (`CP_Random_Gaussian`) |
| The same level every time, from a seed | [random.c](random.c) (`MakeWorld`), [noise.c](noise.c) (an island), [tiled_fill.c](tiled_fill.c) |
| Explosions, sparks, fire and smoke | [particles.c](particles.c) |
| A trail behind something that moves | [particles.c](particles.c) (drawn into an image, or made of particles) |
| A glow where lights overlap | [particles.c](particles.c) (`CP_BLEND_ADD`) |
| Lots of things that come and go (bullets, sparks, enemies) without malloc | [particles.c](particles.c) (`AddParticle`, `UpdateParticles`) |
| Spawning a number each second, at any frame rate | [particles.c](particles.c) (`SpawnCount`) |
| Lines between things that come close | [particles.c](particles.c) |
| Things that pop in, bounce or overshoot | [easing.c](easing.c) |
| A color's code, or the colors between two colors | [color_picker.c](color_picker.c) |
| Fullscreen, a resizable window, the frame rate | [window_options.c](window_options.c) |
| Hiding the mouse cursor; mouse look | [window_options.c](window_options.c) (`CP_System_SetCursorMode`) |
| A console window for printf | [window_options.c](window_options.c) (`CP_System_ShowConsole`) |

## Examples and lessons

The lessons (in the `lessons` folder, one folder each, `01_first_window` to
`11_state_manager`, with a `main.c` to copy over your own) build one program
step by step, with a page on the wiki that explains each step. An example
is a short recipe to copy from once you know the idea. Where a lesson has an
example, both use the same method, so what you learn in one reads straight
into the other:

| Lesson | Example |
|---|---|
| 2. Shapes and the coordinate system | [shapes.c](shapes.c) goes further: your own shapes |
| 3. Color | [color_picker.c](color_picker.c) |
| 4. Input | [controls.c](controls.c), [text_input.c](text_input.c), [gamepad.c](gamepad.c) |
| 5. Moving with vectors and delta time | [movement.c](movement.c) |
| 6. Collision | [collision.c](collision.c) |
| 7. Images and animation | [sprite_animation.c](sprite_animation.c), and [easing.c](easing.c) for its easing step |
| 8. Sound | [music_and_sounds.c](music_and_sounds.c) |
| 9. Game states | [game_states.c](game_states.c) |
| 11. Your own state manager | [state_stack.c](state_stack.c) is the engine's own stack, to compare |

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

Each example is about one subject, and has a "See also" line naming the
examples next to it; a new example adds its row to a group above, its tricks
to [Looking for...](#looking-for), and its lesson, if it has one, to
[Examples and lessons](#examples-and-lessons) (see "Examples and lessons" in
`DESIGN.md`).

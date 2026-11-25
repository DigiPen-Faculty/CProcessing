# Lessons

A learn-by-building path for CProcessing, for self-study or alongside a class. Each lesson is **one page of prose plus one finished program**.

| Lesson | Page (a wiki page when released) | Program |
|---|---|---|
| 1. Your first window | [Lesson-01-Your-First-Window.md](Lesson-01-Your-First-Window.md) | [examples/lessons/01_first_window](../../examples/lessons/01_first_window/main.c) |
| 2. Shapes and the coordinate system | [Lesson-02-Shapes-and-Coordinates.md](Lesson-02-Shapes-and-Coordinates.md) | [examples/lessons/02_shapes](../../examples/lessons/02_shapes/main.c) |
| 3. Color | [Lesson-03-Color.md](Lesson-03-Color.md) | [examples/lessons/03_color](../../examples/lessons/03_color/main.c), and the color picker, [examples/color_picker.c](../../examples/color_picker.c) |
| 4. Input | [Lesson-04-Input.md](Lesson-04-Input.md) | [examples/lessons/04_input](../../examples/lessons/04_input/main.c) |
| 5. Moving with vectors and delta time | [Lesson-05-Moving-with-Vectors-and-Delta-Time.md](Lesson-05-Moving-with-Vectors-and-Delta-Time.md) | [examples/lessons/05_movement](../../examples/lessons/05_movement/main.c), and [examples/movement.c](../../examples/movement.c) |
| 6. Collision | [Lesson-06-Collision.md](Lesson-06-Collision.md) | [examples/lessons/06_collision](../../examples/lessons/06_collision/main.c), and [examples/collision.c](../../examples/collision.c) |
| 7. Images and animation | [Lesson-07-Images-and-Animation.md](Lesson-07-Images-and-Animation.md) | [examples/lessons/07_images](../../examples/lessons/07_images/main.c), and [examples/sprite_animation.c](../../examples/sprite_animation.c) |
| 8. Sound | [Lesson-08-Sound.md](Lesson-08-Sound.md) | [examples/lessons/08_sound](../../examples/lessons/08_sound/main.c), and [examples/music_and_sounds.c](../../examples/music_and_sounds.c) |
| 9. Game states | [Lesson-09-Game-States.md](Lesson-09-Game-States.md) | [examples/lessons/09_game_states](../../examples/lessons/09_game_states/main.c), and [examples/game_states.c](../../examples/game_states.c) |
| 10. Capstone: Slime Snacks | [Lesson-10-Capstone.md](Lesson-10-Capstone.md) | [examples/lessons/10_capstone](../../examples/lessons/10_capstone/main.c) |
| 11. Your own state manager (an extra) | [Lesson-11-Your-Own-State-Manager.md](Lesson-11-Your-Own-State-Manager.md) | [examples/lessons/11_state_manager](../../examples/lessons/11_state_manager/main.c) |

And six **projects**: shorter builds, each starting from the template alone, of things a first game needs early.

| Project | Page (a wiki page when released) | Program |
|---|---|---|
| 1. A splash screen | [Project-01-Splash-Screen.md](Project-01-Splash-Screen.md) | [examples/projects/01_splash_screen](../../examples/projects/01_splash_screen/main.c) |
| 2. Text and numbers | [Project-02-Text-and-Numbers.md](Project-02-Text-and-Numbers.md) | [examples/projects/02_text_and_numbers](../../examples/projects/02_text_and_numbers/main.c) |
| 3. Turning and rounding shapes | [Project-03-Turning-and-Rounding-Shapes.md](Project-03-Turning-and-Rounding-Shapes.md) | [examples/projects/03_turning_shapes](../../examples/projects/03_turning_shapes/main.c) |
| 4. Drawing with functions | [Project-04-Drawing-with-Functions.md](Project-04-Drawing-with-Functions.md) | [examples/projects/04_drawing_functions](../../examples/projects/04_drawing_functions/main.c) |
| 5. Buttons and restart | [Project-05-Buttons-and-Restart.md](Project-05-Buttons-and-Restart.md) | [examples/projects/05_buttons_and_restart](../../examples/projects/05_buttons_and_restart/main.c) |
| 6. A game in many files | [Project-06-A-Game-in-Many-Files.md](Project-06-A-Game-in-Many-Files.md) | [examples/projects/06_many_files](../../examples/projects/06_many_files) (nine files) |

Lessons 2 and 4 to 11, and the projects, were written before lessons 1 and 3 had been tried with students, and use the same format, so notes from the pilot may change all of them.

## Decisions

* **Where the prose lives.** The wiki is what students read, and it must match the release students are using (see [Releases and the live wiki](#releases-and-the-live-wiki)). So the lessons are written here, reviewed in pull requests like code, and copied to the wiki with the release that has the features they use. The images are in `images/`.
* **Where the code lives.** `examples/lessons/NN_name/main.c`: one complete program per lesson, copied over the template's `main.c` to run, like the other examples. CMake builds each one as `Lesson_NN_name` with warnings as errors, so CI checks them on every platform. They ship in the `examples` folder of every release package.
* **Solutions.** The "make it yours" challenges have no single right answer (they're about choices), so there are no solution files. The finished lesson program is public. If a course wants graded exercises with solutions, those belong with the course.
* **Numbering.** Lessons keep their final numbers (1 and 3 first), so links don't change when the others arrive. Lesson 3 doesn't need lesson 2: it uses circles and rectangles, which lesson 1 introduces.
* **Pause in lesson 9 and the capstone.** Both push the pause on top of the game with `CP_Engine_PushGameState` and take it off with `CP_Engine_PopGameState`, as `game_states.c` does. The lesson teaches states as cards on a table: `SetNextGameState` clears the table for a new part of the game, and `PushGameState` puts a card on top when the game stays where it is. A plain push needs no extra calls (the game underneath is frozen and visible by default), so it's no harder than a flag, and it avoids the `resuming` flag that pausing with `SetNextGameState` needed. The flag version stays in the lesson as a short "another way", because students will see it in other code.
* **Lesson 11 is an extra, after the capstone.** It builds the stack of lesson 9 from arrays and structs, on the single-state API (`SetNextGameState` once, and a manager that hands each frame to a stack of screens). It teaches what a push is, why a change is asked for and made at the start of the next frame, and why only the top state gets input. It shows the cost of not having the engine's layers: the screens have a separate draw function, called every frame, since their pictures aren't kept. It uses no assets, so it runs from the template as it is. The sound-ownership, per-state time scale and input-claiming ideas on the roadmap are left to the "Make it yours" list.
* **The assets in the later lessons.** Lessons 7 to 10 use only the template's `assets` (`SlimeSheet.png`, `Music.wav`, `Coin.wav`, `Jump.wav`, `Hit.wav`). In the capstone each state frees what it loads (the play state owns the sprite sheet and the sounds), because the engine runs the exit of every state on the stack when the program ends, and a state can't tell a switch from a shutdown. That keeps the "whoever loads it frees it" rule from lesson 7 without a global load step.
* **Projects next to the lessons.** The lessons are one path that ends in a capstone; each builds on the one before. A project is a smaller build that needs only the template (and, for 4 to 6, the projects before it), for the things students want early, often for their first assignments: a splash screen, a score on the screen, a shape that turns, a drawing in a function, a button, and a game split into files. A project uses the same format, the same methods and the same helpers as the lessons and examples, and links to the ones that go further. Its finished program is in `examples/projects/NN_name/`, built by CMake as `Project_NN_name` from every `.c` file in the folder, so a project can be several files (project 6 is nine). Projects go wrong on purpose more than lessons do: each wrong turn shows the real message from the compiler or the screen, so students learn to read it.
* **The simple button is `Button`.** Projects 5 and 6 build a button positioned by its middle that acts when the mouse button is let go over it (`Button`, with `MouseIsInside`). The mouse example's `DoButton` is stricter (the press and the release must both be on it) and positioned by its corner. They have different names because they are different code: a helper repeated in several programs keeps one name for one piece of code ([DESIGN.md](../../DESIGN.md)).
* **Format of a lesson.** What you'll make (a picture), what you'll learn, time and prerequisites; small steps, each with a "Try it"; make-it-yours challenges; check-yourself questions; common mistakes. Reference pages are linked, not repeated.

## Trying the pilot with students

All 11 lessons are written, but only as drafts: run them with a class, a TA or a few students, starting with lessons 1 and 3, and watch rather than help. Before a release, give them the template package (`tools\build_release_package.bat -r all -c -z`, from a Developer Command Prompt) and an offline build of the draft wiki plus these pages (`docs/offline/build.ps1 -WikiSource` on a copy of the wiki's draft branch with the lessons added). Notes to collect for each lesson:

* **Time:** how long did each lesson take against the estimate?
* **Where people got stuck:** which step, and what did they try?
* **Errors:** every compiler error, warning and crash, with the exact text, and how they got past it.
* **Silent failures:** times nothing happened and nothing said why (a blank window, a picture or sound that didn't appear). These are candidates for CProcessing's own warnings.
* **The "Try it" prompts:** which were skipped, which were loved, which confused?
* **Check-yourself questions:** could students answer them without scrolling back?
* **Challenges:** which did they pick? Were any too big or too small?
* **Words:** terms that needed more explanation (delta time, handle, `NULL`, `snprintf`, degrees and radians).
* **"I wanted to..." moments:** things they tried that the lesson or the library didn't cover.
* **Verdict:** keep, tweak or rework, and the one change that would help most.

And once for the whole pilot:

* **Setup:** from download to the first window, what went wrong and how long did it take?
* **Getting around:** which other pages and examples did they open, and were the links from lessons to examples used?
* **Reading:** did they read the explanations or skim to the code? Did they type the code or paste it?
* **The color picker:** did it help with lesson 3, or did people play with it and skip the text?
* **Tone and level:** right for high school? For first-semester college?
* **From a TA or another expert:** their own snags, and where they expect a first-semester student would get stuck.

Put what you learn here, under a heading with the date and the group, so the later lessons can use it.

## Releases and the live wiki

Students use a release for a whole course, and the wiki has to match it. Lessons use things the current release doesn't have (the examples folder, the new template and its assets, the color picker), so they wait for the release that has them. Until then, nothing here is on the live wiki.

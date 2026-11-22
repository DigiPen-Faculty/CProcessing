# Why CProcessing is the way it is

This page is for anyone changing CProcessing: what it is for, what it values,
and the rules that follow. When a change is unclear, these decide it.

## What it is for

CProcessing gives students who are learning C a colorful, playful place to
practice it: something on screen in minutes and a game by the end of the
term, so the C they learn elsewhere has somewhere exciting to go.

It started in a game class where students built text games in the console
while other courses taught them C line by line. Processing and p5.js showed
how much fun a few lines of drawing code can be; CProcessing brings that to C.
Students write C at a high level here while they learn its details
elsewhere. A game state, for example, is three function names given to
`CP_Engine_SetNextGameState`; the students don't need to know that those are
function pointers, or how to write one.

## Values

In order: when two of them disagree, the higher one wins.

1. **Fun arrives fast.** A first program draws something in under ten lines.
2. **Students write the C.** The library hides what students haven't learned
   yet or can't reach (function pointers, the GPU, audio devices, the
   operating system). It doesn't hide what *is* the lesson: game logic, a
   state machine, an easing curve.
3. **One obvious way.** Few functions, one way of naming them, one set of
   units, and the same defaults and behavior everywhere.
4. **Correct underneath.** No silent surprises: no angle that comes out as
   NaN, no color that loses a step, frame timing that does what it says, and the
   same seed gives the same results on every computer. Tested.
5. **Familiar to Processing and p5.js users,** where that doesn't fight C.
6. **Plain C11 on every platform.** The headers include no operating system
   headers, and a program behaves the same on Windows, Linux and macOS.
7. **Everything shown compiles.** The examples, the lessons and the wiki's
   code are built in CI.
8. **Stable for students.** Changes that break programs come only in a major
   release, each with a note on what to do. Nothing breaks silently: a
   program either fails to compile, with the compiler pointing at the line, or
   works as before.

## API rules

### What belongs in the library

A function belongs in the library if at least one of these is true:

* it needs the library's internals: the window, drawing, sound, input, the
  operating system;
* it's easy to get subtly wrong and most games need it: random numbers in a
  range without bias, measuring text, frame timing;
* it replaces C that students haven't learned yet.

Processing's everyday math words (map, lerp, clamp, distance, radians and
degrees, square) stay, because every example uses them. Anything else that
a student can write in a few lines, and learns something by writing, is an
**example** to copy from, not a library function. Easing curves are the
typical case: each is a line or two of math on a lerp factor, so
`examples/easing.c` shows them with their code instead.

A released function is removed only when it misleads students or clutters
the library in a way that costs them, never just to be tidy.

### Names

* `CP_Area_VerbNoun`: `CP_Graphics_DrawCircle`, `CP_Sound_SetGroupVolume`.
  Everything public starts with `CP_` (checked against the source by
  `tools/check_api_names.py`).
* `Get` and `Set` come in pairs. A function that answers yes or no reads as a
  question: `CP_System_IsFullscreen`, `CP_System_GetCursorVisible`.
* **`Advanced`** means *the same function with every option*: a gamepad
  number, a volume, pitch, loop and group, a window size. The plain function
  is the common case, with sensible defaults.
* A name says what it means where it's used: `CP_PASSTHROUGH_VISIBLE`
  describes what passes through to the states below a game state, so it's
  named after the pass-through, not after a state.

### Units

Positions and sizes in pixels, angles in **degrees**, time in **seconds**,
colors from 0 to 255, lerp factors from 0 to 1. A function in other units says
so in its name (`CP_Matrix_RotateRadians`).

### Comments in the headers

`cprocessing.h` is the index of the library: one line per function, in
columns, so a student can find a name quickly. Visual Studio shows a comment
on the same line, or just above it, when the mouse is over the function, so
a short comment there is help without leaving the code. The wiki has the
details; the headers say only what the names and types can't.

* A **section note**, under the section's heading, says what is true for the
  whole section: its units, which way angles turn, what lasts until it's
  freed.
* A **note at the end of a function's line** says what its signature can't,
  and only when a student would get it wrong without it: a range
  (`0 to 180`), a hidden rule (`0xRRGGBBAA: all 8 digits`), a difference
  from a function next to it.
* **Before writing a note, try a better parameter name.** `left, top, right,
  bottom` don't look like 0 to 1, the way `u0, v0, u1, v1` did.
* No comment says again what a name already says: `CP_Graphics_DrawCircle`
  has none.
* A note is a fact the wiki also states, so the two say the same thing. A
  change to the behavior changes both.

### Numbers

* A float that becomes an integer (a color channel, a position) is
  **rounded**, not cut off.
* No function returns NaN or infinity for an input a student can easily give
  it (a zero vector, two vectors pointing the same way).
* Random numbers and noise come from the library's own code, never the C
  runtime's `rand`, so a seed means the same thing on every computer.

### Changing the API

* Every change goes in `CHANGELOG.md`. A breaking change says what to do in an
  existing program.
* The wiki pages, the examples and the lessons change with it, and CI builds
  them all.
* The planning notes record why a decision was made, so it isn't made again
  from scratch.

## Examples and lessons

Students look for an example when they want to make something: "a health
bar", "random levels", "a pause menu". These rules keep that search short.

* **One example, one subject,** named for what a student wants to make or
  do. An example may use other features for its scenery (noise for the hills
  behind a gradient sky), but it teaches only its own subject; a second
  subject gets an example of its own.
* **Every trick can be found.** An example's row in `examples/README.md`
  says what it shows, and a trick that lives inside a bigger example goes in
  the README's "Looking for..." table. The wiki's Examples page lists the
  same, and each API page links to the examples that use it.
* **Each example's header** says what it shows, its keys, ideas to try, and
  a "See also" line naming the examples next to it.
* **Everything shown ships.** Examples and lessons are in every release
  package; nothing is only for people who clone the repository.
* **Each example is one file** copied over `main.c`, so small helpers (a
  centered label, an easing curve) are repeated in the examples that need
  them, with the same name and the same code each time. Drawing that gives
  an example its own look (a panel's color) may differ.
* **A lesson builds one program step by step; an example is a short recipe.**
  Where both cover a subject, they use the same method, and each points to
  the other.

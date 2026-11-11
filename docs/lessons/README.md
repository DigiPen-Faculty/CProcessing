# Lessons

A learn-by-building path for CProcessing, for self-study or alongside a class. Each lesson is **one page of prose plus one finished program**.

| Lesson | Page (a wiki page when released) | Program |
|---|---|---|
| 1. Your first window | [Lesson-01-Your-First-Window.md](Lesson-01-Your-First-Window.md) | [Examples/Lessons/01_first_window](../../Examples/Lessons/01_first_window/main.c) |
| 2. Shapes and the coordinate system | [Lesson-02-Shapes-and-Coordinates.md](Lesson-02-Shapes-and-Coordinates.md) | [Examples/Lessons/02_shapes](../../Examples/Lessons/02_shapes/main.c) |
| 3. Color | [Lesson-03-Color.md](Lesson-03-Color.md) | [Examples/Lessons/03_color](../../Examples/Lessons/03_color/main.c), and the Color picker demo (6) in `Processing_Demos` |
| 4. Input | [Lesson-04-Input.md](Lesson-04-Input.md) | [Examples/Lessons/04_input](../../Examples/Lessons/04_input/main.c) |

Lessons 5 to 10 follow. Lessons 2 and 4 were written before lessons 1 and 3 had been tried with students, and use the same format, so notes from the pilot may change all four. The outline is in the planning notes.

## Decisions

* **Where the prose lives.** The wiki is what students read, and it must match the release students are using (see [Releases and the live wiki](#releases-and-the-live-wiki)). So the lessons are written here, reviewed in pull requests like code, and copied to the wiki with the release that has the features they use. The images are in `images/`.
* **Where the code lives.** `Examples/Lessons/NN_name/main.c`: one complete program per lesson, copied over the template's `main.c` to run, like the other examples. CMake builds each one as `Lesson_NN_name` with warnings as errors, so CI checks them on every platform. They ship in the `Examples` folder of every release package.
* **Solutions.** The "make it yours" challenges have no single right answer (they're about choices), so there are no solution files. The finished lesson program is public. If a course wants graded exercises with solutions, those belong with the course.
* **Numbering.** Lessons keep their final numbers (1 and 3 first), so links don't change when the others arrive. Lesson 3 doesn't need lesson 2: it uses circles and rectangles, which lesson 1 introduces.
* **Format of a lesson.** What you'll make (a picture), what you'll learn, time and prerequisites; small steps, each with a "Try it"; make-it-yours challenges; check-yourself questions; common mistakes. Reference pages are linked, not repeated.

## Trying the pilot with students

Before writing the other lessons, run lessons 1 and 3 with a class, a TA or a few students, and watch rather than help. Notes to collect:

* **Time:** how long did each lesson take against the estimate?
* **Where people got stuck:** which step, and what did they try?
* **The "Try it" prompts:** which were skipped, which were loved, which confused?
* **Check-yourself questions:** could students answer them without scrolling back?
* **Challenges:** which did they pick? Were any too big or too small?
* **The color picker:** did it help with step 3, or did people play with it and skip the text?
* **Tone and level:** right for high school? For first-semester college?

Put what you learn here, under a heading with the date and the group, so the later lessons can use it.

## Releases and the live wiki

Students use a release for a whole course, and the wiki has to match it. Lessons use things the current release doesn't have (the Examples folder, the new template and its assets, the Color picker demo), so they wait for the release that has them. Until then, nothing here is on the live wiki.

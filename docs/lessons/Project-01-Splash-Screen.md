# Project 1: A splash screen

![The white DigiPen logo on a dark background, halfway through sliding up and shrinking](images/project-01.png)

A splash screen is the first thing a program shows: a logo that fades in, stays long enough to read, and then moves on. It is a small project that uses a lot of what makes a program feel alive: an image, a timer, and numbers that change smoothly over time.

**You will make:** a logo that fades in, holds still, then slides up and shrinks.

**You will learn:** how to draw an image in its right shape, how to keep time, and how to fade, move and scale something with one number.

**Time:** about 45 minutes.

**You need:** the CProcessing template, built and running once ([Your first program](Your-first-program)). The template's `assets` folder has the DigiPen logo in three colors. Any image works the same way.

The finished program is in the repository as [examples/projects/01_splash_screen/main.c](https://github.com/DigiPen-Faculty/CProcessing/blob/main/examples/projects/01_splash_screen/main.c). [Lesson 7](Lesson-07-Images-and-Animation) goes further with pictures (sprite sheets and animation), the [easing example](https://github.com/DigiPen-Faculty/CProcessing/blob/main/examples/easing.c) has curves that make a slide start and stop gently, and the [transitions example](https://github.com/DigiPen-Faculty/CProcessing/blob/main/examples/transitions.c) fades from one game state to the next.

---

## Step 1: Show the logo

An image takes three steps: **load** it once, **draw** it every frame, and **free** it when the program ends. Loading reads the file from the disk, which is slow, so it goes in `game_init`, not in `game_update`.

```C
#include "cprocessing.h"

CP_Image logo;

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	logo = CP_Image_Load("assets/DigiPen_WHITE.png");
}

void game_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(20, 20, 30, 255));
	CP_Image_Draw(logo, 400.0f, 300.0f, 500.0f, 500.0f, 255);
}

void game_exit(void)
{
	CP_Image_Free(&logo);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
```

`logo` is outside every function so that `game_init`, `game_update` and `game_exit` can all use it. The `&` in `CP_Image_Free(&logo)` hands over where `logo` lives, so `Free` can clear it. Always write it that way for now.

The path starts with `assets/`: a folder named `assets` next to your program. See [assets and paths](Assets-and-paths).

Run it. Something is wrong with the logo.

> **Try it:** what does the logo look like? Change the `500.0f, 500.0f` to other numbers and watch what happens to its shape. Can you make it look right by guessing?

## Step 2: The right shape

The logo is squashed because you told `CP_Image_Draw` to fit it into a 500 by 500 box, and the picture is much wider than it is tall. Guessing numbers until it looks right would work, but it breaks the moment you use another image.

A picture knows its own size in pixels. Ask it with `CP_Image_GetWidth` and `CP_Image_GetHeight`, and scale **both** by the same amount, so the shape stays the same. The ratio of width to height is the *aspect ratio*, and scaling both by the same number keeps it.

<!-- no-compile -->
```C
float size = 0.5f; // half the picture's own size

float width = (float)CP_Image_GetWidth(logo) * size;
float height = (float)CP_Image_GetHeight(logo) * size;
CP_Image_Draw(logo, 400.0f, 300.0f, width, height, 255);
```

> **Try it:** change `size` to 0.25f, 1.0f and 2.0f. Which one is the logo's own size? What happens when it is bigger than the window? Then put the logo exactly in the middle of the window without typing 400 and 300: [CP_System_GetWindowWidth](System/#CP_System_GetWindowWidth) and `GetWindowHeight` divided by `2.0f` do it for any window size.

## Step 3: Keep time

A splash screen needs to know how long it has been showing. Make a number that grows with time. [CP_System_GetDt](System/#CP_System_GetDt) is the seconds since the last frame, so adding it every frame makes a clock:

<!-- no-compile -->
```C
float timer;

void game_init(void)
{
	timer = 0.0f;
}

void game_update(void)
{
	timer += CP_System_GetDt();
}
```

> **Try it:** add `float timer = 0.0f;` as the first line of `game_update` as well, so there are two. Print `timer` (or draw it, see [Debugging](Debugging)). What does it show now, and why? Then take the line back out.

Setting `timer` in `game_init` means the clock starts again at 0 whenever the state starts again.

## Step 4: Fade in

[CP_Image_Draw](Image/#CP_Image_Draw) takes a last number, the **alpha**: 255 is solid, 0 is invisible, and anything between is see-through. To fade in, make alpha grow from 0 to 255 while the timer goes from 0 to 1.

That "how far along am I" number is useful for everything in this project, so give it a function. `Progress` is 0 until `startTime`, grows to 1 at `endTime`, and stays at 1:

<!-- no-compile -->
```C
float Progress(float startTime, float endTime)
{
	return CP_Math_ClampFloat((timer - startTime) / (endTime - startTime), 0.0f, 1.0f);
}
```

`CP_Math_ClampFloat` keeps a number between a smallest and a largest value. Without it, the number would keep growing past 1 and the alpha would go past 255.

Use it to set the alpha:

<!-- no-compile -->
```C
float fadeIn = Progress(0.0f, 1.0f);   // 0 to 1 during the first second
int alpha = (int)(255.0f * fadeIn);    // 0 to 255

CP_Image_Draw(logo, 400.0f, 300.0f, width, height, alpha);
```

> **Try it:** make the fade take 3 seconds, then half a second. Then try `int alpha = (int)(1.0f * fadeIn);`. Why is the logo almost invisible? (Alpha is 0 to 255, not 0 to 1.)

## Step 5: Hold still

The logo should be fully solid and not changing for a while, so people can read it. Nothing needs adding for that: `Progress(0.0f, 1.0f)` is stuck at 1 once the timer passes 1, so the alpha stays at 255. The logo is **held** from second 1 until whatever you do next begins.

Decide when that is, for example second 3. The hold lasts 2 seconds, and everything after it can start from `Progress(3.0f, ...)`.

> **Try it:** add `CP_Graphics_DrawCircle(100.0f, 100.0f, 40.0f);` that appears only after the hold, with `if (timer > 3.0f)`. Change 3.0f and see when it appears.

## Step 6: Move and scale

After the hold, the logo slides up and shrinks. Two things change over the same stretch of time, so they share one progress number. `CP_Math_LerpFloat(a, b, t)` gives `a` when `t` is 0, `b` when `t` is 1, and a mix in between:

<!-- no-compile -->
```C
float move = Progress(3.0f, 4.5f);                       // 0 to 1 from second 3 to 4.5
float y = CP_Math_LerpFloat(centerY, 120.0f, move);      // slides up
float size = CP_Math_LerpFloat(0.5f, 0.25f, move);       // shrinks to half
```

Together, the complete program:

```C
#include "cprocessing.h"

CP_Image logo;
float timer;

// 0 until startTime, then grows to 1 at endTime, and stays at 1
float Progress(float startTime, float endTime)
{
	return CP_Math_ClampFloat((timer - startTime) / (endTime - startTime), 0.0f, 1.0f);
}

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	logo = CP_Image_Load("assets/DigiPen_WHITE.png");
	timer = 0.0f;
}

void game_update(void)
{
	timer += CP_System_GetDt();

	CP_Graphics_ClearBackground(CP_Color_Create(20, 20, 30, 255));

	float centerX = CP_System_GetWindowWidth() / 2.0f;
	float centerY = CP_System_GetWindowHeight() / 2.0f;

	// Fade in during the first second, then stay solid
	int alpha = (int)(255.0f * Progress(0.0f, 1.0f));

	// After the hold, from second 3 to 4.5: slide up and shrink
	float move = Progress(3.0f, 4.5f);
	float y = CP_Math_LerpFloat(centerY, 120.0f, move);
	float size = CP_Math_LerpFloat(0.5f, 0.25f, move);

	float width = (float)CP_Image_GetWidth(logo) * size;
	float height = (float)CP_Image_GetHeight(logo) * size;
	CP_Image_Draw(logo, centerX, y, width, height, alpha);
}

void game_exit(void)
{
	CP_Image_Free(&logo);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
```

> **Try it:** swap the two numbers in the `LerpFloat` for `y` so it slides down instead. Make the logo grow, not shrink. Make it fade out as it moves (what does the alpha need to be?).

## Make it yours

Pick two or more.

* Make the background change color over time with the same `Progress` and [CP_Color_Lerp](Color/#CP_Color_Lerp).
* Move the logo sideways, or make it bounce: `sinf(timer)` from `<math.h>` gives a number that swings between -1 and 1 (see [Math](Math/#CP_PI) for the constants and [Moving with delta time](Moving-with-delta-time)).
* Add a shape or a second image that appears after the hold, and a title that fades in under the logo with [CP_Font_DrawText](Font/#CP_Font_DrawText).
* Add a third step: after the logo is done, fade the whole picture to black.
* Switch to another state when the splash is over (see [Game states](Game-states), and [Project 6](Project-06-A-Game-in-Many-Files) for a splash in a file of its own), with the time from `CP_Engine_GetGameStateTime` instead of your own `timer`.
* Try the red and black logos, and the background colors they look good on.

## Check yourself

* Why is the image loaded in `game_init` and not in `game_update`?
* What stays the same when you scale width and height by the same number?
* What does `Progress(3.0f, 4.5f)` return at second 2? At second 3.75? At second 10?
* What would alpha 128 look like?

## Common mistakes

* **A squashed picture.** The width and height don't match the picture's shape. Scale both with `CP_Image_GetWidth` and `GetHeight`.
* **The timer stays at 0.** The variable is declared inside `game_update`, so it is made again every frame. Declare it outside the functions and set it in `game_init`.
* **Alpha 1 instead of 255.** Opacity for images is 0 to 255.
* **Dividing whole numbers.** `1 / 2` is 0 in C. Write `1.0f / 2.0f`.
* **Nothing shows.** Check the path: `assets/DigiPen_WHITE.png` is a folder named `assets` next to the program, with the capital letters as shown. `CP_Image_Load` gives back nothing if it can't find the file. See [assets and paths](Assets-and-paths).
* **Forgetting `CP_Image_Free`.** Free what you load, in `game_exit`.

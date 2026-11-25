# Project 2: Text and numbers

![A dark window with a click count at the top left, the time at the top middle, the frame count at the top right, and "Click anywhere" in the middle](images/project-02.png)

Text is how a program talks to the player: a score, a timer, a title, a message. Drawing words is easy. The part that surprises people is putting a *number* into them, because a number and a piece of text are different things in C.

**You will make:** a window that shows a click counter, a clock and a frame count, each placed against a different edge.

**You will learn:** how to draw text, how to size and color it, how to place it by its left, middle or right, and how to turn numbers into text with `snprintf`.

**Time:** about 30 minutes.

**You need:** the CProcessing template, built and running once ([Your first program](Your-first-program)).

The finished program is in the repository as [examples/projects/02_text_and_numbers/main.c](https://github.com/DigiPen-Faculty/CProcessing/blob/main/examples/projects/02_text_and_numbers/main.c). The [score and text example](https://github.com/DigiPen-Faculty/CProcessing/blob/main/examples/score_and_text.c) places a score, a timer and wrapped text the same way, and the [Font](Font) page has every text function.

---

## Step 1: Draw some text

[CP_Font_DrawText](Font/#CP_Font_DrawText) draws text at a position. Replace the code in the template's `main.c` with this:

```C
#include "cprocessing.h"

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
}

void game_update(void)
{
	CP_Graphics_ClearBackground(CP_Color_Create(255, 255, 255, 255));
	CP_Font_DrawText("Hello, world!", 100.0f, 100.0f);
}

void game_exit(void)
{
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
```

The text goes between double quotes. Run it, and look closely: is there text?

> **Try it:** the text is there, but it is hard to see. Why? Text is drawn in the **fill color**, the same one shapes use, and the fill color starts out light grey. Add `CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));` before the line that draws the text. Then try other colors, and a background that isn't white.

The color you set stays set. Everything drawn after it, shapes and text, uses it until you change it. See [Using color](Using-color).

## Step 2: Size and position

[CP_Settings_TextSize](Settings/#CP_Settings_TextSize) sets how big the letters are. The default is 16, which is small in an 800 by 600 window:

<!-- no-compile -->
```C
CP_Settings_TextSize(48.0f);
CP_Font_DrawText("Hello, world!", 100.0f, 100.0f);
```

The two numbers after the text are where it goes. By default, `(100, 100)` is the **left end of the line the letters sit on**, not the top left corner of the text.

> **Try it:** change `100.0f, 100.0f` to `100.0f, 0.0f`. Where did the text go? Then try 10, 30 and 48 for the second number. Which part of the text can you still see when it is at 0? Can you explain it from "the line the letters sit on"?

## Step 3: Place it by its middle

Positioning by the left end is awkward for text in the middle of the window, because you would have to know how wide the text is. Ask the computer to place it by its middle instead. [CP_Settings_TextAlignment](Settings/#CP_Settings_TextAlignment) chooses which part of the text sits at `(x, y)`: left, center or right, and top, middle or bottom.

<!-- no-compile -->
```C
CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

float centerX = CP_System_GetWindowWidth() / 2.0f;
float centerY = CP_System_GetWindowHeight() / 2.0f;
CP_Font_DrawText("Hello, world!", centerX, centerY);
```

> **Try it:** the text is now centered however long it is. Change "Hello, world!" to something much longer and see that it still is. Then try `CP_TEXT_ALIGN_H_LEFT` and `CP_TEXT_ALIGN_H_RIGHT` with the same `centerX`, and `CP_TEXT_ALIGN_V_TOP` and `CP_TEXT_ALIGN_V_BOTTOM` with the same `centerY`. Where does the text go in each?

Like the fill color, the alignment stays until you change it.

## Step 4: Put a number in the text

A score is a number, and `CP_Font_DrawText` wants text. Try giving it the number:

<!-- no-compile -->
```C
int score = 5;
CP_Font_DrawText(score, 100.0f, 100.0f);
```

> **Try it:** put those two lines in `game_update` and build. Read the messages. One of them mentions `const char *` and `int`: the function asked for text and was handed a number. Take the lines back out.

A number has to be turned into text first. **`snprintf`** does that. It works like a form with blanks: you write the text you want, with a code where each number goes, and it fills in the blanks and puts the result in a `char` array that you can draw.

<!-- no-compile -->
```C
#include <stdio.h>                              // at the top of the file

int score = 5;

char text[50] = { 0 };                          // room for the text: 50 characters
snprintf(text, sizeof(text), "Score: %d", score);
CP_Font_DrawText(text, 100.0f, 100.0f);
```

* `text` is the `char` array that gets the result.
* `sizeof(text)` tells `snprintf` how big `text` is, so it never writes past the end.
* `"Score: %d"` is the form. `%d` is the blank, and `score` is what fills it in. The result is `Score: 5`.

The code after the `%` says what kind of number goes in the blank:

| Code | For | Example | Shows |
|---|---|---|---|
| `%d` | a whole number (`int`) | `"%d", 42` | `42` |
| `%u` | a whole number that can't be negative (`unsigned`), like the frame count | `"%u", CP_System_GetFrameCount()` | `1503` |
| `%f` | a decimal number (`float`) | `"%f", 1.5f` | `1.500000` |
| `%.1f` | a decimal number with 1 digit after the point | `"%.1f", 1.5f` | `1.5` |
| `%s` | other text | `"%s", name` | `Alex` |
| `%%` | a `%` sign | `"100%%"` | `100%` |

A form can have several blanks, filled from left to right: `snprintf(text, sizeof(text), "%d of %d", score, total)`.

> **Try it:** make the score go up with every click: `if (CP_Input_MouseTriggered(CP_MOUSE_LEFT)) { score++; }`. Then show the time since the program started with `CP_System_GetSeconds()` and `%.1f`. How does `%f` look next to `%.1f`, and `%.0f`?

Now try three things that go wrong:

> **Try it:** use `%d` for the seconds, which is a decimal number. The compiler stops with warning C4477 and says the format wants an `int` and was given a `double`. A `%d` can only hold a whole number. Fix it by changing it to `%f`.
>
> Make the buffer small: `char text[8]`. The text is cut off, and nothing breaks. That is what `sizeof(text)` is for. Make it big again.
>
> Draw `text` *before* the `snprintf` in the same frame. What do you see, and why? (The array is empty until `snprintf` fills it.)

## Step 5: Put it together

Three numbers, each against a different edge. The left one is placed by its left end, the middle one by its middle, and the right one by its right end. Right-aligned text matters most for numbers, because they get wider as they grow:

```C
#include "cprocessing.h"
#include <stdio.h>

int clicks;

void game_init(void)
{
	CP_System_SetWindowSize(800, 600);
	clicks = 0;
}

void game_update(void)
{
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		clicks++;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(30, 30, 50, 255));
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));

	float centerX = CP_System_GetWindowWidth() / 2.0f;
	float centerY = CP_System_GetWindowHeight() / 2.0f;
	float rightX = CP_System_GetWindowWidth() - 20.0f;
	char text[50] = { 0 };

	CP_Settings_TextSize(28.0f);

	// Top left: placed by its left end
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Clicks: %d", clicks);
	CP_Font_DrawText(text, 20.0f, 20.0f);

	// Top middle: placed by its middle
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Time: %.1f", CP_System_GetSeconds());
	CP_Font_DrawText(text, centerX, 20.0f);

	// Top right: placed by its right end, 20 pixels from the edge
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_RIGHT, CP_TEXT_ALIGN_V_TOP);
	snprintf(text, sizeof(text), "Frame: %u", CP_System_GetFrameCount());
	CP_Font_DrawText(text, rightX, 20.0f);

	// A message in the middle of the window
	CP_Settings_TextSize(48.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("Click anywhere", centerX, centerY);
}

void game_exit(void)
{
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
```

The same `text` array is used three times: each `snprintf` writes over what was there before.

> **Try it:** change the top right text to `CP_TEXT_ALIGN_H_LEFT` and leave `rightX` where it is. Let the frame count grow for a while. What happens to the text as the number gets longer? Put it back to `CP_TEXT_ALIGN_H_RIGHT`: now it grows to the left and stays in the window.

## Make it yours

Pick two or more.

* Count down instead of up: start at 30 and show how many seconds are left, with `%.0f`.
* Show the mouse position, `CP_Input_GetMouseX` and `GetMouseY`, next to the mouse as it moves.
* Show a title large and a smaller line under it. Use [CP_Font_GetTextSize](Font/#CP_Font_GetTextSize) to find the height of a line so the second line lands just below the first.
* Make text fade out by lowering the alpha of its fill color (the fourth number), as [Project 1](Project-01-Splash-Screen) does for a picture.
* Wrap a long sentence inside a box with [CP_Font_DrawTextBox](Font/#CP_Font_DrawTextBox).
* Load a font of your own and use it for the title ([CP_Font_Load](Font/#CP_Font_Load)).
* Draw a button: a rectangle with text on it, centered. The text is drawn *after* the rectangle, or the rectangle covers it.

## Check yourself

* Why is the text hard to see when you haven't set a fill color?
* Where is the text, compared with `(x, y)`, when the alignment is `CP_TEXT_ALIGN_H_RIGHT` and `CP_TEXT_ALIGN_V_TOP`?
* What does `snprintf(text, sizeof(text), "%d + %d", 2, 3)` put in `text`?
* Why is the array's size given to `snprintf`?
* Which code prints an `int`? A `float` with one decimal? The frame count?

## Common mistakes

* **The text is almost invisible.** Text uses the fill color, which starts out light grey. Set it with `CP_Settings_Fill` before drawing.
* **Giving `CP_Font_DrawText` a number.** It wants text. Build the text with `snprintf` first.
* **The wrong code for the number.** `%d` is for `int`, `%u` for `unsigned`, `%f` for decimals. The compiler warns about a mismatch, so read the warning.
* **A buffer too small for the text.** The end of the text is cut off. Make the array bigger, and always pass `sizeof` the array to `snprintf`.
* **The text is above the window.** By default `y` is the line the letters sit on, so `y` of 0 puts almost the whole text above the window.
* **Forgetting `#include <stdio.h>`.** `snprintf` needs it.
* **An older tutorial uses `sprintf_s` and `_countof`.** Those only exist in Visual Studio's C library. `snprintf` and `sizeof` work everywhere: see [Updating older code](Updating-older-code).

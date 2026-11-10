# CProcessing template (Visual Studio)

A starting point for a CProcessing game on Windows, with Visual Studio.

## What's here

| File | What it is |
|---|---|
| `main.c` | Your game. It starts as a small complete program: a circle that follows the mouse. |
| `Assets/` | Images, sounds and fonts. Each build copies it next to the game. |
| `MyGame.sln` | The Visual Studio solution. Open this. |
| `MyGame.vcxproj` | The project: how `main.c` is built and linked with CProcessing. |
| `CProcessing/` | The CProcessing library, already built: headers in `inc`, libraries in `lib`. |

## What you need

Visual Studio 2026 with the **Desktop development with C++** workload.

## Build and run

1. Open `MyGame.sln`.
2. Press **F5** (or **Debug > Start Debugging**). Visual Studio builds the
   game and runs it. A window opens with an orange circle that follows the
   mouse.
3. Edit `main.c` and press **F5** again.

The toolbar picks the configuration:

- **Debug** or **Release.** Debug is easier to step through in the
  debugger. It uses `CProcessingd.dll`, the debug build of CProcessing.
- **x64** or **x86.** x64 is the usual choice.

The game is built into a folder such as `x64\Debug`, next to the CProcessing
DLL and a copy of the `Assets` folder.

When the game ends, CProcessing lists any images, sounds and fonts that
were loaded and never freed. Run from Visual Studio, the list appears in the
**Output** window; in Debug, a message box shows it too.

## Seeing `printf` output

The game opens without a console window. Call `CP_System_ShowConsole(TRUE);`
(it's in `main.c`, commented out) to open one.

## Renaming the game

To rename the program from `MyGame`, rename the project in Solution
Explorer (right-click it, **Rename**). The folder can have any name.

## Learning more

- [CProcessing documentation](https://github.com/DigiPen-Faculty/CProcessing/wiki),
  starting with "Your first program"
- `CProcessing-Offline-Documentation.zip` on the
  [Releases page](https://github.com/DigiPen-Faculty/CProcessing/releases),
  to read it without an internet connection
- The `Examples` folder: small complete programs to read, run and copy from

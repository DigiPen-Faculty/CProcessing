# CProcessing starter project (CMake)

A starting point for a CProcessing game on Windows, Linux or macOS, built
with CMake. On Windows with Visual Studio, most students use the Visual
Studio template instead (`CProcessingTemplate.zip` on the
[Releases page](https://github.com/DigiPen-Faculty/CProcessing/releases)).

## What's here

| File | What it is |
|---|---|
| `main.c` | Your game. |
| `Assets/` | Images, sounds and fonts. Each build copies it next to the game. |
| `CMakeLists.txt` | The build. Change `MyGame` in `project(MyGame C)` to rename the game. |
| `CProcessing/` | The CProcessing library, already built. Only in the Linux and macOS release packages. |

## What you need

- **Linux:** a C compiler and CMake, for example
  `sudo apt install build-essential cmake` on Debian or Ubuntu.
- **macOS:** the Xcode Command Line Tools (`xcode-select --install`) and
  CMake (`brew install cmake`).
- **Windows:** Visual Studio 2026 with the "Desktop development with C++"
  workload, which includes CMake.

Without the `CProcessing/` folder, the first build downloads CProcessing and
builds it from source. That needs an internet connection and, on Linux, the
extra packages listed in
[BUILDING.md](https://github.com/DigiPen-Faculty/CProcessing/blob/main/BUILDING.md).

## Build and run

From this folder:

```sh
cmake -B build
cmake --build build
cd build/bin
./MyGame          # MyGame.exe on Windows
```

After editing `main.c`, run `cmake --build build` again. Always start the
game from `build/bin`, because CProcessing loads assets relative to the
current folder.

## Seeing `printf` output

- **Linux and macOS:** run the game from a terminal, as above.
- **Windows:** call `CP_System_ShowConsole(TRUE);` (it's in `main.c`,
  commented out) to open a console window.

## Visual Studio Code

Open this folder in VS Code and install the recommended C/C++ and CMake
Tools extensions when asked. Pick a compiler ("kit") when CMake Tools asks.
Then use **Build** and **Debug** in the status bar. The debugger starts the
game in `build/bin`, so assets load.

## macOS: "can't be opened"

macOS support is new. Please report anything that doesn't work.

If macOS refuses to open `libCProcessing.dylib` from a downloaded package
(for example "cannot be opened because the developer cannot be verified"),
remove the download quarantine from this folder and build again:

```sh
xattr -dr com.apple.quarantine .
```

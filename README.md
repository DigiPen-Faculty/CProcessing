# CProcessing
 
## What is it?
* It's a framework that provides simple implementations for many of the common things used in 2D games all within the C programming language.
* Graphics, Audio, Input, Fonts, Colors, Math (Vectors and Matrices), Random
* It is a DLL that can be incorporated into projects across many platforms and accessed from many different programming languages.
* It runs on Windows, Linux and macOS. macOS support is a preview.
* It was patterned after Processing (Java) and P5.js (JavaScript)
 
## What can you do with it?
* It's currently used in GAM100 in Redmond, Malaysia and Singapore as a way for students to very quickly build prototype games and even do their first team game.
* R&D uses it for a whole curriculum series aimed at High School students.
* It could be used in other classes such as AI, Math, and Physics where course projects have students demonstrating cool enemy behaviors or dynamic body collisions and resolutions, but they wouldn't need to write a complete engine or know how to do graphics etc.

## What's next?
* Grab a copy of the [Latest Release](https://github.com/DigiPen-Faculty/CProcessing/releases):
  `CProcessingTemplate.zip` for Visual Studio on Windows, or the CMake starter project for Linux (`CProcessingTemplate-linux-x64.zip`) or macOS (`CProcessingTemplate-macos.zip`).
  The macOS package is a preview: since 3.0.1, drawing, the demos and the automated tests work on an Apple silicon Mac (macOS 26), but sound, gamepads and Intel Macs haven't been confirmed yet. Please [report problems](https://github.com/DigiPen-Faculty/CProcessing/issues).
* Read through the [Documentation](https://github.com/DigiPen-Faculty/CProcessing/wiki).
* See what changed in each release in the [Changelog](CHANGELOG.md).
* Have fun building awesome stuff!

## Building from source
* **Windows:** open `Processing_Sample/Processing_Sample.sln` in Visual Studio.
* **Windows, Linux and macOS:** use CMake (`cmake -B build && cmake --build build`).
* See [BUILDING.md](BUILDING.md) for details and platform notes, and
  [DEPENDENCIES.md](DEPENDENCIES.md) for the third-party libraries and how they're pinned.

## License
The library is offered under the [MIT License](https://github.com/DigiPen-Faculty/CProcessing/blob/main/LICENSE).

Default font, built into the library:
* Exo 2 by Natanael Gama - licensed under the [SIL Open Font License 1.1](Processing_Sample/Assets/Exo2_license.txt)

## Acknowledgment
* [NanoVG](https://github.com/memononen/nanovg) by Mikko Mononen
* [SoLoud](https://solhsa.com/soloud/) by Jari Komppa
* [GLFW](https://www.glfw.org/) by Marcus Geelnard and Camilla Löwy
* [miniaudio](https://miniaud.io/) by David Reid

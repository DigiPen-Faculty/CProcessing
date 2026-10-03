# NanoVG in CProcessing

This folder is [NanoVG](https://github.com/memononen/nanovg), the vector
graphics library CProcessing draws with. It holds upstream's source at a
known commit, plus a small set of CProcessing patches. This file records
both, so that anyone updating NanoVG, or wondering whether an upstream fix
applies, can see exactly what is here.

## Which upstream version

| | |
|---|---|
| Upstream | https://github.com/memononen/nanovg |
| Commit | **`ce3bf745eb2d2dbc14a50bf2446783f691ac4353`** (2026-02-19) |
| Files taken | `src/` (`nanovg.c`, `nanovg.h`, `nanovg_gl.h`, `nanovg_gl_utils.h`, `fontstash.h`, `stb_image.h`, `stb_truetype.h`), `LICENSE.txt`, `README.md` |
| Not taken | `example/`, `obsolete/`, `premake4.lua`, `.github/`: not used by CProcessing |

Upstream has no releases or tags, so a commit is the only way to pin it. Its
README says it "is not actively maintained", and it changes rarely: seven
source changes from 2021 to 2026.

`stb_image.h` and `stb_truetype.h` are the copies upstream NanoVG bundles.

### Why this commit, and what it changed (CProcessing 3.0)

Until 3.0, this folder held upstream commit `30a943c` (April 2018), with
CProcessing's patches applied on top. 3.0 moved it to `ce3bf74`, the latest
commit. These upstream fixes since 2018 matter to CProcessing:

- Text drawn with a mirroring transform, such as `CP_Settings_Scale(-1, 1)`,
  vanished (`621e0b8`).
- Text whose size changes every frame lost glyphs (`8d1b1e7`), and leaked
  glyph atlas textures (`4e42b6c`). Both had already been fixed here in
  October 2026 by applying upstream's code.
- On 32-bit builds, large text (size 600 or so) ran out of fontstash's
  scratch memory: an assert in Debug builds, nothing drawn in Release. The
  larger buffer (`c7f7078`) and newer stb_truetype fix it. Exo 2's @ and
  MS Gothic's 龍 (16 strokes) now draw exactly the same on x86 as on x64,
  up to size 2000.
- Glyph positions are rounded down rather than toward zero, so text at
  negative coordinates isn't shifted by a pixel (`426aa3f`).
- A 1x1 texture is bound when drawing without an image, since some OpenGL
  drivers reject sampling an unbound texture (`35dbc98`).
- `nvgBeginFrame` takes the window size as floats (`6b6e3a5`).

One upstream change is visible to students: **the text size is the font's
em size** (`69e1a47`), as in Processing and CSS. Before, it was the font's
full height from ascender to descender. With the default font, Exo 2, text
is 20% larger at the same `CP_Settings_TextSize` than before 3.0. Line
spacing in text boxes follows the font's ascender, descender and line gap
(`528dc4e`).

## CProcessing's patches

Every change to upstream's files is marked with a `// CProcessing:` comment.
`tools/nanovg-diff.sh` in the repository root prints the complete
difference from the upstream commit above, and nothing should appear there
that isn't listed here.

| Patch | Files | Used by | Why it can't live outside NanoVG |
|---|---|---|---|
| **Tint:** `nvgTintColor`, applied by `nvg__applyTint` in `nvgFill`, `nvgStroke` and text drawing | `nanovg.c`, `nanovg.h` | `CP_Settings_Tint`, `CP_Settings_NoTint` | It has to reach every paint NanoVG builds, including image patterns and text. |
| **Image filter per draw:** `nvgTextureFilter` (nearest or linear) | `nanovg.c`, `nanovg.h`, `nanovg_gl.h` | `CP_Settings_ImageFilterMode` | Upstream only has a per-image flag, set when the image is created (`NVG_IMAGE_NEAREST`). CProcessing, like Processing, makes it a drawing setting. |
| **Image wrap per draw:** `nvgTextureWrap` (clamp to border, clamp to edge, repeat, mirror) | `nanovg.c`, `nanovg.h`, `nanovg_gl.h` | `CP_Settings_ImageWrapMode` | Upstream only has repeat X/Y flags set at creation, and no mirror or clamp-to-border. |
| **Blend modes:** blend equations (`NVGblendEquation`), the `NVG_BLEND_*` composite operations, `glBlendEquation` in the GL back end | `nanovg.c`, `nanovg.h`, `nanovg_gl.h` | `CP_Settings_BlendMode` | Subtract, min and max need a blend equation, which upstream's GL back end never sets. |
| **Settings last across frames:** `nvgBeginFrame` doesn't reset the drawing state | `nanovg.c` | Processing's model: fill, stroke, blend mode and so on carry over to the next frame | Upstream resets the state at the start of every frame. |
| **Freeing fonts:** `nvgFreeFont`, and `fons__remFont` in fontstash. A freed font's data and glyphs are released, but it keeps its slot as an empty font, because a font's handle is its index: moving later fonts down would break every handle to them. | `nanovg.c`, `nanovg.h`, `fontstash.h` | `CP_Font_Free` | Fontstash has no way to remove a font. |

Things that used to be patches here and now live in CProcessing's own code:

- Reading an image's pixels (`CP_Image_GetPixelData`) uses upstream's
  `nvglImageHandleGL3` and OpenGL directly.
- Points (`CP_Graphics_DrawPoint`) build their own path.
- Casts and similar edits that silenced compiler warnings. Both build
  systems now compile NanoVG without warnings, as they do all vendored code
  (see `CMakeLists.txt` and `CProcessing.vcxproj`).

The OpenGL back end, `nanovg_gl.h`, is header-only. It is compiled once, in
`Source/nanovg_gl3.c`; the rest of CProcessing includes it for the
declarations only.

## Updating to a newer upstream commit

1. Merge each patched file three ways, with the current pin as the base,
   CProcessing's file as one side and the new upstream file as the other:
   `git merge-file ours.c base.c theirs.c`. Copy the files that have no
   patches over directly.
2. Resolve conflicts, keep every patch marked with `// CProcessing:`, and
   remove any patch that upstream now covers.
3. Update the commit in this file, in `tools/nanovg-diff.sh` and in
   `DEPENDENCIES.md`, then run `tools/nanovg-diff.sh` and check that
   everything it shows is listed above.
4. Build with both build systems and run both test tiers. Tier 2 covers each
   patch: tint, image filter and wrap, blend modes, fonts, and settings that
   last across frames and screenshots. It also covers the upstream fixes
   above, including the text size.

//---------------------------------------------------------
// file:	nanovg_gl3.c
// brief:	Compiles NanoVG's OpenGL 3 back end once for the library.
//
// nanovg_gl.h is header-only: it declares its functions, and defines them
// when NANOVG_GL3_IMPLEMENTATION is set. Everything else in CProcessing
// includes it for the declarations only (see Internal_System.h). Like
// nanovg.c, this file compiles vendored code as-is, so both build systems
// turn its warnings off. See nanovg/CPROCESSING.md.
//---------------------------------------------------------

#include "glad.h"

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"

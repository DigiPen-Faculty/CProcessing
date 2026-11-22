//---------------------------------------------------------
// file:	stb_image_write.c
// brief:	Compiles stb_image_write once for the library, for CP_Image_Save.
//
// stb_image_write.h is header-only: it defines its functions where
// STB_IMAGE_WRITE_IMPLEMENTATION is set. STB_IMAGE_WRITE_STATIC keeps them
// private to this file, so a game that compiles its own copy of
// stb_image_write doesn't clash with CProcessing's; the one function below
// is what the rest of the library uses. Like nanovg.c, this file compiles
// vendored code as-is, so both build systems turn its warnings off. See
// DEPENDENCIES.md.
//---------------------------------------------------------

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Writes width * height RGBA pixels, row by row from the top left, to a PNG
// file. Returns 0 if the file couldn't be written. (Internal_Image.h)
int CP_WritePNG(const char* filepath, int width, int height, const void* rgbaPixels)
{
	return stbi_write_png(filepath, width, height, 4, rgbaPixels, width * 4);
}

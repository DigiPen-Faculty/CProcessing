// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

// Assertions against data tier2_capture.c already captured. See
// Scn_ImageLoadAndDraw / Scn_ImageSubImage there for how assets/quadrants.png
// (a 4x4 fixture: TL=red, TR=green, BL=blue, BR=yellow) was used.
#include <string.h>
#include "unity.h"
#include "tier2_capture.h"

static void assertColorWithin(unsigned char tolerance, CP_Color expected, CP_Color actual)
{
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.r, actual.r);
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.g, actual.g);
	TEST_ASSERT_UINT8_WITHIN(tolerance, expected.b, actual.b);
}

// See Scn_SharedAssets: loads of the same file share it until the last
// one is freed
void test_tier2_shared_image_survives_freeing_one_load(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.sharedImageSame);
	TEST_ASSERT_TRUE(tier2_scalars.sharedImageFirstCleared);
	TEST_ASSERT_TRUE(tier2_scalars.sharedImageReloadSame);
	TEST_ASSERT_TRUE(tier2_scalars.emptyPathLoadIsNull);
	const char* message = tier2_scalars.messageEmptyPath;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, tier2_CountText(message, "CProcessing: CP_Image_Load was given no file name"), message);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, tier2_CountText(message, "CProcessing: "), message);
	// The image was drawn over the whole canvas after the first free: red
	// top left quadrant, yellow bottom right
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_SHARED_ASSETS, 50, 50));
	assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_SHARED_ASSETS, 150, 150));
}

void test_tier2_shared_font_and_sound_survive_freeing_one_load(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.sharedFontSame);
	TEST_ASSERT_TRUE(tier2_scalars.sharedFontReloadSame);
	if (!tier2_scalars.soundAvailable)
	{
		TEST_IGNORE_MESSAGE("no audio device: sounds don't load");
	}
	TEST_ASSERT_TRUE(tier2_scalars.soundAndStreamDiffer);
	TEST_ASSERT_TRUE(tier2_scalars.sharedSoundReloadSame);
}

// See LeaveAssetsLoadedForTheExitReport: the harness leaves two loads of an
// image and an image made from data unfreed, and CProcessing lists them
void test_tier2_assets_still_loaded_are_reported_at_exit(void)
{
	const char* report = tier2_scalars.exitReport;
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(report, "still loaded when the program ended"), report);
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(report, "assets/quadrants16.png: 2 loads not freed"), report);
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(report, "1 made with CP_Image_Create, CP_Image_CreateFromData or CP_Image_Screenshot"), report);
}

void test_tier2_image_load_reports_correct_dimensions(void)
{
	TEST_ASSERT_EQUAL_INT(4, tier2_scalars.quadImageWidth);
	TEST_ASSERT_EQUAL_INT(4, tier2_scalars.quadImageHeight);
}

void test_tier2_image_draw_places_quadrants_correctly(void)
{
	// Drawn as an 80x80 square centered at (100,100): each 40x40 quadrant's center.
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_IMAGE_LOAD_AND_DRAW, 80, 80));     // TL red
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_LOAD_AND_DRAW, 120, 80));    // TR green
	assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_LOAD_AND_DRAW, 80, 120));    // BL blue
	assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_LOAD_AND_DRAW, 120, 120)); // BR yellow
}

void test_tier2_image_draw_rotated_with_translate_and_rotate(void)
{
	// 90 degrees clockwise: red goes top left to top right, green top right to
	// bottom right, yellow bottom right to bottom left, blue bottom left to top left
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_IMAGE_DRAW_ROTATED, 120, 80));
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_DRAW_ROTATED, 120, 120));
	assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_DRAW_ROTATED, 80, 120));
	assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_DRAW_ROTATED, 80, 80));
}

void test_tier2_image_createfromdata_roundtrips_exactly(void)
{
	CP_Color expected[4] = {
		CP_Color_Create(10, 20, 30, 255), CP_Color_Create(40, 50, 60, 255),
		CP_Color_Create(70, 80, 90, 255), CP_Color_Create(100, 110, 120, 255)
	};
	for (int i = 0; i < 4; ++i)
	{
		assertColorWithin(0, expected[i], tier2_scalars.createFromDataReadback[i]);
	}
}

void test_tier2_image_updatepixeldata_roundtrips_exactly(void)
{
	CP_Color expected[4] = {
		CP_Color_Create(200, 0, 0, 255), CP_Color_Create(0, 200, 0, 255),
		CP_Color_Create(0, 0, 200, 255), CP_Color_Create(200, 200, 0, 255)
	};
	for (int i = 0; i < 4; ++i)
	{
		assertColorWithin(0, expected[i], tier2_scalars.updatePixelDataReadback[i]);
	}
}

void test_tier2_image_drawsubimage_selects_correct_region(void)
{
	// u:[0.5,1.0] v:[0.0,0.5] selects only the top-right (green) quadrant.
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_SUBIMAGE, 100, 100));
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_SUBIMAGE, 75, 75));
	assertColorWithin(2, CP_Color_Create(255, 255, 255, 255), tier2_SamplePixel(SCN_IMAGE_SUBIMAGE, 20, 20));
}

void test_tier2_image_16_bit_png_loads_and_draws(void)
{
	// See Scn_Image16Bit: drawn like quadrants.png, 80x80 centered
	TEST_ASSERT_EQUAL_INT(4, tier2_scalars.image16BitWidth);
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_IMAGE_16_BIT, 80, 80));
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_16_BIT, 120, 80));
	assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_16_BIT, 80, 120));
	assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_16_BIT, 120, 120));
}

void test_tier2_image_filter_mode_nearest_and_linear(void)
{
	// See Scn_ImageFilterModes. Nearest: four solid colors meeting in the
	// middle of the left half.
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 45, 95));
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 55, 95));
	assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 45, 105));
	assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 55, 105));
	// Linear: the middle of the right half averages all four
	assertColorWithin(10, CP_Color_Create(128, 128, 64, 255), tier2_SamplePixel(SCN_IMAGE_FILTER_MODES, 150, 100));
}

void test_tier2_image_wrap_modes(void)
{
	// See Scn_ImageWrapModes. Each source pixel is 20x20 on screen; the
	// samples are in the top row, 2.5 and 3.5 source pixels across, past
	// the image's right edge (top row: red, green).
	const CP_Color red = CP_Color_Create(255, 0, 0, 255);
	const CP_Color green = CP_Color_Create(0, 255, 0, 255);
	const CP_Color white = CP_Color_Create(255, 255, 255, 255);
	struct
	{
		int x, y;
		CP_Color past, farther;
	} tiles[4] = {
		{ 10,  10,  white, white }, // clamp: nothing past the edge
		{ 110, 10,  green, green }, // clamp to edge: the edge pixel repeats
		{ 10,  110, red,   green }, // repeat: the image again
		{ 110, 110, green, red   }, // mirror: the image reversed
	};
	for (int i = 0; i < 4; ++i)
	{
		assertColorWithin(2, red, tier2_SamplePixel(SCN_IMAGE_WRAP_MODES, tiles[i].x + 10, tiles[i].y + 10));
		assertColorWithin(2, tiles[i].past, tier2_SamplePixel(SCN_IMAGE_WRAP_MODES, tiles[i].x + 50, tiles[i].y + 10));
		assertColorWithin(2, tiles[i].farther, tier2_SamplePixel(SCN_IMAGE_WRAP_MODES, tiles[i].x + 70, tiles[i].y + 10));
	}
}

// ---- Image fills: see Scn_ImageFillTiles and the scenarios after it ----

// The color of the four color image's square at (x, y), for tiles of 40x40
// from (tileX, tileY): red, green / blue, yellow, repeating
static CP_Color TileColor(int x, int y, int tileX, int tileY)
{
	const CP_Color colors[4] = {
		CP_Color_Create(255, 0, 0, 255), CP_Color_Create(0, 255, 0, 255),
		CP_Color_Create(0, 0, 255, 255), CP_Color_Create(255, 255, 0, 255)
	};
	// floor division, so squares left of or above the tiles' corner count too
	int column = (x - tileX + 4000) / 20;
	int row = (y - tileY + 4000) / 20;
	return colors[(row % 2) * 2 + column % 2];
}

static const CP_Color canvasWhite = { 255, 255, 255, 255 };

void test_tier2_image_fill_repeats_the_tiles_whatever_the_wrap_mode(void)
{
	// The middle of every square of the 200x100 rect: five tiles across
	for (int y = 10; y < 100; y += 20)
	{
		for (int x = 10; x < 200; x += 20)
		{
			assertColorWithin(2, TileColor(x, y, 0, 0), tier2_SamplePixel(SCN_IMAGE_FILL_TILES, x, y));
		}
	}
}

void test_tier2_image_fill_tiles_start_at_their_corner(void)
{
	for (int x = 10; x < 200; x += 20)
	{
		// Moved a square right: the first square is the green from the tile
		// to the left
		assertColorWithin(2, TileColor(x, 110, 20, 100), tier2_SamplePixel(SCN_IMAGE_FILL_TILES, x, 110));
		assertColorWithin(2, TileColor(x, 130, 20, 100), tier2_SamplePixel(SCN_IMAGE_FILL_TILES, x, 130));
		// Moved whole tiles: the same as from 0
		assertColorWithin(2, TileColor(x, 10, 0, 0), tier2_SamplePixel(SCN_IMAGE_FILL_TILES, x, 160));
		assertColorWithin(2, TileColor(x, 30, 0, 0), tier2_SamplePixel(SCN_IMAGE_FILL_TILES, x, 180));
	}
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILL_TILES, 10, 110));
}

void test_tier2_image_fill_fills_only_the_shape(void)
{
	// The circle: tiles inside, nothing in the corners around it
	assertColorWithin(2, TileColor(50, 50, 0, 0), tier2_SamplePixel(SCN_IMAGE_FILL_SHAPES, 50, 50));
	assertColorWithin(2, TileColor(30, 70, 0, 0), tier2_SamplePixel(SCN_IMAGE_FILL_SHAPES, 30, 70));
	assertColorWithin(2, canvasWhite, tier2_SamplePixel(SCN_IMAGE_FILL_SHAPES, 14, 14));
	assertColorWithin(2, canvasWhite, tier2_SamplePixel(SCN_IMAGE_FILL_SHAPES, 86, 86));
	// The square with a hole: tiles around it, nothing in it
	assertColorWithin(2, TileColor(120, 20, 0, 0), tier2_SamplePixel(SCN_IMAGE_FILL_SHAPES, 120, 20));
	assertColorWithin(2, TileColor(180, 80, 0, 0), tier2_SamplePixel(SCN_IMAGE_FILL_SHAPES, 180, 80));
	assertColorWithin(2, canvasWhite, tier2_SamplePixel(SCN_IMAGE_FILL_SHAPES, 150, 50));
}

void test_tier2_image_fill_with_no_image_or_no_size_fills_nothing(void)
{
	// A NULL image and a tile of no size: no tiles, and nothing else either
	// (not the old fill, not black)
	const int samples[][2] = {
		{ 25, 125 },
		{ 75, 125 },
		{ 25, 175 },
		{ 75, 175 }
	};
	for (int i = 0; i < 4; ++i)
	{
		assertColorWithin(2, canvasWhite, tier2_SamplePixel(SCN_IMAGE_FILL_SHAPES, samples[i][0], samples[i][1]));
	}
	// An image freed after it was set, the frame after: the same
	for (int y = 10; y < 200; y += 20)
	{
		for (int x = 10; x < 200; x += 20)
		{
			assertColorWithin(2, canvasWhite, tier2_SamplePixel(SCN_IMAGE_FILL_FREED_NEXT_FRAME, x, y));
		}
	}
}

void test_tier2_image_fill_alpha(void)
{
	// Half of each color over white: red is (255, 127, 127), blue (127, 127, 255)
	assertColorWithin(3, CP_Color_Create(255, 127, 127, 255), tier2_SamplePixel(SCN_IMAGE_FILL_TRANSFORMED, 10, 10));
	assertColorWithin(3, CP_Color_Create(127, 127, 255, 255), tier2_SamplePixel(SCN_IMAGE_FILL_TRANSFORMED, 10, 30));
	assertColorWithin(3, CP_Color_Create(255, 255, 127, 255), tier2_SamplePixel(SCN_IMAGE_FILL_TRANSFORMED, 70, 190));
}

void test_tier2_image_fill_follows_translate_and_scale(void)
{
	// 40x40 on screen from (100, 0). Placed in window coordinates instead,
	// they would be 20x20 from (0, 0), so the squares would alternate
	// every 10 pixels.
	for (int y = 10; y < 200; y += 20)
	{
		for (int x = 110; x < 200; x += 20)
		{
			assertColorWithin(2, TileColor(x, y, 100, 0), tier2_SamplePixel(SCN_IMAGE_FILL_TRANSFORMED, x, y));
		}
	}
}

void test_tier2_image_fill_is_replaced_by_fill_and_kept_by_save_restore(void)
{
	// Restore undid the FillImage: green
	assertColorWithin(2, CP_Color_Create(0, 200, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILL_SETTINGS, 30, 10));
	assertColorWithin(2, CP_Color_Create(0, 200, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILL_SETTINGS, 70, 70));
	// Restore undid the Fill: the tiles
	for (int y = 10; y < 100; y += 20)
	{
		for (int x = 110; x < 200; x += 20)
		{
			assertColorWithin(2, TileColor(x, y, 0, 0), tier2_SamplePixel(SCN_IMAGE_FILL_SETTINGS, x, y));
		}
	}
}

void test_tier2_image_fill_text_keeps_the_last_fill_color(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.imageFillGetFillKept);
	// The middle of a 120 high I
	assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_FILL_SETTINGS, 50, 150));
}

void test_tier2_image_fill_leaves_the_wrap_mode_for_images(void)
{
	// The sub-image's own top left square (red), then past its right edge
	// the edge pixels stretched out: green beside the top row, yellow beside
	// the bottom. Repeating would show the image again there: red at both.
	assertColorWithin(2, CP_Color_Create(255, 0, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILL_SETTINGS, 120, 120));
	assertColorWithin(2, CP_Color_Create(0, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILL_SETTINGS, 160, 120));
	assertColorWithin(2, CP_Color_Create(255, 255, 0, 255), tier2_SamplePixel(SCN_IMAGE_FILL_SETTINGS, 160, 160));
}

void test_tier2_image_many_images_load_draw_and_free(void)
{
	// See Scn_ImageMany: 40 images, more than the image list starts with.
	TEST_ASSERT_EQUAL_INT(40, tier2_scalars.manyImagesCreated);
	TEST_ASSERT_EQUAL_INT(4, tier2_scalars.manyImagesFileWidth);
	TEST_ASSERT_EQUAL_INT(40, tier2_scalars.manyImagesFreed);
	// The first and the last were drawn: 10x10 squares at (0,0) and (140,80)
	assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_MANY, 5, 5));
	assertColorWithin(2, CP_Color_Create(0, 0, 255, 255), tier2_SamplePixel(SCN_IMAGE_MANY, 145, 85));
}

// ---- CP_Image_Save: see Scn_ImageSave ----

void test_tier2_image_save_writes_a_png_that_loads_back_the_same(void)
{
	TEST_ASSERT_TRUE(tier2_scalars.saveReturnedTrue);
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.saveFileIsPNG, "the saved file doesn't start with PNG's signature");
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.saveRoundTrip, "an image made from data didn't load back the same");
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.saveCapitalPNG, "a name ending in .PNG wasn't saved");
}

void test_tier2_image_save_keeps_drawn_images_and_screenshots_the_right_way_up(void)
{
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.saveDrawnRoundTrip, "an image drawn into didn't load back the same");
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.saveDrawnTopRowFirst, "an image drawn into was saved upside down");
	TEST_ASSERT_TRUE_MESSAGE(tier2_scalars.saveScreenshotRoundTrip, "a screenshot didn't load back the same");
}

void test_tier2_image_save_refuses_what_it_cannot_save(void)
{
	// No image, no name, an empty name, a .jpg name, a folder that isn't there
	TEST_ASSERT_EQUAL_INT(5, tier2_scalars.saveRefusals);
	TEST_ASSERT_TRUE(tier2_scalars.saveRefusedLeftNoFile);

	// Each says why, once: no name and an empty name share a message
	const char* messages = tier2_scalars.messageSaveRefusals;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, tier2_CountText(messages, "CProcessing: CP_Image_Save was given no image (NULL)"), messages);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, tier2_CountText(messages, "CProcessing: CP_Image_Save needs a file name"), messages);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, tier2_CountText(messages, "must end in .png: tier2_saved.jpg was not saved"), messages);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, tier2_CountText(messages, "CProcessing: couldn't save tier2_no_such_folder/saved.png"), messages);
	TEST_ASSERT_EQUAL_INT_MESSAGE(4, tier2_CountText(messages, "CProcessing: "), messages);
	TEST_ASSERT_EQUAL_INT_MESSAGE(4, tier2_CountText(messages, "\n"), messages);
}

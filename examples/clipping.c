//---------------------------------------------------------
// file:	clipping.c
//
// Example: clipping, so drawing stays inside a box.
//
// Shows how to:
// - scroll a long list inside a panel, with CP_Settings_Clip cutting off
//   the rows above and below it
// - draw a minimap: the whole world, smaller, inside a box in the corner
// - refill an ability icon from the bottom up as its cooldown runs out
// - reveal a title with a wipe from left to right
//
// CP_Settings_Clip(x, y, w, h) cuts off everything drawn outside that
// rectangle (shapes, outlines, text and images) until CP_Settings_NoClip.
// Its numbers follow CP_Settings_RectMode, like CP_Graphics_DrawRect. It's
// placed when you call it, so drawing moved by a later Translate moves under
// it while it stays put. Save and Restore keep it, a new clip replaces the
// old one, and it starts over every frame, like the transform.
//
// Clipping only changes what you see: a row scrolled out of the panel can
// still be clicked, so the list checks that the mouse is in the panel.
//
// Scroll the list with the mouse wheel or by dragging it, and click a row to
// pick it. The arrow keys move the player on the minimap, and 1, 2 and 3 use
// the abilities.
//
// Try this:
// - Give a chat log or a dialog box a scrolling list of its own.
// - Make the minimap round: draw a thick ring over its square edges.
// - Wipe in the other direction, or from the middle out.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"

#define WINDOW_WIDTH  800.0f
#define WINDOW_HEIGHT 600.0f

// The inventory panel, and the height of one row of its list
#define PANEL_LEFT   40.0f
#define PANEL_TOP    90.0f
#define PANEL_WIDTH  300.0f
#define PANEL_HEIGHT 440.0f
#define ROW_HEIGHT   44.0f

// The minimap, and the world it shows
#define MAP_LEFT   450.0f
#define MAP_TOP    90.0f
#define MAP_WIDTH  310.0f
#define MAP_HEIGHT 190.0f
#define MAP_SCALE  0.25f
#define WORLD_SIZE 1200.0f
#define TREE_COUNT 70

#define ABILITY_COUNT 3

const char* items[] = {
	"Rusty sword",
	"Wooden shield",
	"Health potion",
	"Mana potion",
	"Rope",
	"Torch",
	"Map of the caves",
	"Silver key",
	"Bag of gold",
	"Bread",
	"Lucky coin",
	"Iron helmet",
	"Leather boots",
	"Fire scroll",
	"Ice scroll",
	"Old compass",
	"Lantern",
	"Fishing rod",
	"Gemstone",
	"Dragon scale",
};
#define ITEM_COUNT ((int)(sizeof(items) / sizeof(items[0])))

float scroll = 0.0f; // how far the list has moved up, in pixels
int selected = 0;    // the row that was clicked

CP_Vector player;
CP_Vector trees[TREE_COUNT];

float cooldownLeft[ABILITY_COUNT] = { 0.0f, 0.0f, 0.0f }; // seconds
const float cooldownTime[ABILITY_COUNT] = { 1.5f, 4.0f, 8.0f };

float wipeTime = 0.0f;

//---------------------------------------------------------
// The inventory: 20 rows in a panel with room for 10

CP_BOOL MouseInPanel(void)
{
	const float x = CP_Input_GetMouseX();
	const float y = CP_Input_GetMouseY();
	return x >= PANEL_LEFT && x < PANEL_LEFT + PANEL_WIDTH && y >= PANEL_TOP && y < PANEL_TOP + PANEL_HEIGHT;
}

// The row under the mouse, or -1. The clip doesn't stop clicks, so a row
// scrolled out of sight would still be found without the MouseInPanel check.
int RowUnderMouse(void)
{
	if (!MouseInPanel())
	{
		return -1;
	}
	const int row = (int)((CP_Input_GetMouseY() - PANEL_TOP + scroll) / ROW_HEIGHT);
	return row < ITEM_COUNT ? row : -1;
}

void UpdateInventory(void)
{
	if (MouseInPanel())
	{
		// The wheel turned away from you is positive: back up the list
		scroll -= CP_Input_MouseWheel() * ROW_HEIGHT;
		if (CP_Input_MouseDragged(CP_MOUSE_LEFT))
		{
			scroll -= CP_Input_GetMouseDeltaY();
		}
		if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
		{
			selected = RowUnderMouse();
		}
	}
	scroll = CP_Math_ClampFloat(scroll, 0.0f, ITEM_COUNT * ROW_HEIGHT - PANEL_HEIGHT);
}

void DrawInventory(void)
{
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_TextSize(24.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);

	CP_Settings_Fill(CP_Color_Create(240, 240, 250, 255));
	CP_Font_DrawText("Inventory", PANEL_LEFT, PANEL_TOP - 24.0f);

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(30, 32, 48, 255));
	CP_Graphics_DrawRect(PANEL_LEFT, PANEL_TOP, PANEL_WIDTH, PANEL_HEIGHT);

	// From here on, nothing shows outside the panel. The list is drawn as if
	// it were all there, 20 rows tall, and moved up by the scroll: the clip
	// stays where it was set, so the rows slide under it.
	CP_Settings_Clip(PANEL_LEFT, PANEL_TOP, PANEL_WIDTH, PANEL_HEIGHT);
	CP_Settings_Translate(0.0f, -scroll);

	const int hovered = RowUnderMouse();
	for (int i = 0; i < ITEM_COUNT; ++i)
	{
		const float top = PANEL_TOP + i * ROW_HEIGHT;
		if (i == selected)
		{
			CP_Settings_Fill(CP_Color_Create(70, 110, 200, 255));
		}
		else if (i == hovered)
		{
			CP_Settings_Fill(CP_Color_Create(55, 60, 90, 255));
		}
		else
		{
			CP_Settings_Fill(i % 2 == 0 ? CP_Color_Create(38, 40, 60, 255) : CP_Color_Create(30, 32, 48, 255));
		}
		CP_Graphics_DrawRect(PANEL_LEFT, top, PANEL_WIDTH, ROW_HEIGHT);

		CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
		CP_Font_DrawText(items[i], PANEL_LEFT + 16.0f, top + ROW_HEIGHT / 2.0f);
	}

	CP_Settings_ResetMatrix();
	CP_Settings_NoClip();

	// The scroll bar, beside the panel: how much of the list shows, and where
	const float listHeight = ITEM_COUNT * ROW_HEIGHT;
	const float barHeight = PANEL_HEIGHT * PANEL_HEIGHT / listHeight;
	const float barTop = PANEL_TOP + (PANEL_HEIGHT - barHeight) * scroll / (listHeight - PANEL_HEIGHT);
	CP_Settings_Fill(CP_Color_Create(30, 32, 48, 255));
	CP_Graphics_DrawRoundedRect(PANEL_LEFT + PANEL_WIDTH + 8.0f, PANEL_TOP, 10.0f, PANEL_HEIGHT, 5.0f);
	CP_Settings_Fill(CP_Color_Create(120, 130, 170, 255));
	CP_Graphics_DrawRoundedRect(PANEL_LEFT + PANEL_WIDTH + 8.0f, barTop, 10.0f, barHeight, 5.0f);
}

//---------------------------------------------------------
// The minimap: the world drawn a quarter of its size, around the player,
// and cut off at the edges of the map

void DrawWorld(void)
{
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(70, 130, 70, 255));
	CP_Graphics_DrawRect(0.0f, 0.0f, WORLD_SIZE, WORLD_SIZE);

	CP_Settings_Fill(CP_Color_Create(60, 120, 200, 255));
	CP_Graphics_DrawEllipse(800.0f, 350.0f, 420.0f, 260.0f);

	CP_Settings_Stroke(CP_Color_Create(170, 140, 90, 255));
	CP_Settings_StrokeWeight(24.0f);
	CP_Graphics_DrawLine(0.0f, 900.0f, WORLD_SIZE, 650.0f);

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(30, 80, 40, 255));
	for (int i = 0; i < TREE_COUNT; ++i)
	{
		CP_Graphics_DrawCircle(trees[i].x, trees[i].y, 60.0f);
	}

	CP_Settings_Fill(CP_Color_Create(255, 220, 60, 255));
	CP_Graphics_DrawCircle(player.x, player.y, 60.0f);
}

void UpdatePlayer(void)
{
	CP_Vector move = CP_Vector_Zero();
	move.x = (float)CP_Input_KeyDown(CP_KEY_RIGHT) - (float)CP_Input_KeyDown(CP_KEY_LEFT);
	move.y = (float)CP_Input_KeyDown(CP_KEY_DOWN) - (float)CP_Input_KeyDown(CP_KEY_UP);
	move = CP_Vector_Scale(CP_Vector_Normalize(move), 400.0f * CP_System_GetDt());
	player = CP_Vector_Add(player, move);
	player.x = CP_Math_ClampFloat(player.x, 0.0f, WORLD_SIZE);
	player.y = CP_Math_ClampFloat(player.y, 0.0f, WORLD_SIZE);
}

void DrawMinimap(void)
{
	CP_Settings_TextSize(24.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_Fill(CP_Color_Create(240, 240, 250, 255));
	CP_Font_DrawText("Map", MAP_LEFT, MAP_TOP - 24.0f);

	// Past the edges of the world, the map is dark
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(10, 10, 16, 255));
	CP_Graphics_DrawRect(MAP_LEFT, MAP_TOP, MAP_WIDTH, MAP_HEIGHT);

	// Save, so Restore takes away the clip and the transforms together. The
	// clip comes first, while the coordinates are still the window's.
	CP_Settings_Save();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(MAP_LEFT, MAP_TOP, MAP_WIDTH, MAP_HEIGHT);

	// The middle of the map, a quarter of the size, with the player there
	CP_Settings_Translate(MAP_LEFT + MAP_WIDTH / 2.0f, MAP_TOP + MAP_HEIGHT / 2.0f);
	CP_Settings_Scale(MAP_SCALE, MAP_SCALE);
	CP_Settings_Translate(-player.x, -player.y);

	DrawWorld();
	CP_Settings_Restore();

	// The frame, drawn after the clip is gone so all of its outline shows
	CP_Settings_NoFill();
	CP_Settings_Stroke(CP_Color_Create(240, 240, 250, 255));
	CP_Settings_StrokeWeight(3.0f);
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(MAP_LEFT, MAP_TOP, MAP_WIDTH, MAP_HEIGHT);
}

//---------------------------------------------------------
// Ability icons. Each is drawn twice: gray, and then in color through a clip
// that grows from the bottom as the cooldown runs out.

void DrawAbilityIcon(int ability, float left, float top, float size, CP_Color color, CP_Color symbol)
{
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(color);
	CP_Graphics_DrawRoundedRect(left, top, size, size, 14.0f);

	const float x = left + size / 2.0f;
	const float y = top + size / 2.0f;
	CP_Settings_Fill(symbol);
	if (ability == 0)
	{
		CP_Graphics_DrawCircle(x, y, size * 0.45f);
	}
	else if (ability == 1)
	{
		CP_Graphics_DrawTriangle(x, y - 22.0f, x + 22.0f, y + 18.0f, x - 22.0f, y + 18.0f);
	}
	else
	{
		CP_Graphics_DrawQuad(x, y - 24.0f, x + 20.0f, y, x, y + 24.0f, x - 20.0f, y);
	}
}

void UpdateAbilities(void)
{
	for (int i = 0; i < ABILITY_COUNT; ++i)
	{
		cooldownLeft[i] -= CP_System_GetDt();
		if (cooldownLeft[i] < 0.0f)
		{
			cooldownLeft[i] = 0.0f;
		}
		if (CP_Input_KeyTriggered((CP_KEY)(CP_KEY_1 + i)) && cooldownLeft[i] == 0.0f)
		{
			cooldownLeft[i] = cooldownTime[i];
		}
	}
}

void DrawAbilities(float left, float top)
{
	const float size = 80.0f;
	const CP_Color colors[ABILITY_COUNT] = {
		CP_Color_Create(220, 80, 60, 255),
		CP_Color_Create(70, 170, 230, 255),
		CP_Color_Create(170, 100, 220, 255),
	};
	const char* keys[ABILITY_COUNT] = { "1", "2", "3" };

	for (int i = 0; i < ABILITY_COUNT; ++i)
	{
		const float x = left + i * (size + 35.0f);
		const float ready = 1.0f - cooldownLeft[i] / cooldownTime[i]; // 0 just used, 1 ready

		DrawAbilityIcon(i, x, top, size, CP_Color_Create(70, 70, 80, 255), CP_Color_Create(120, 120, 130, 255));

		// Only the bottom part, as high as the icon is ready, gets the color
		CP_Settings_RectMode(CP_POSITION_CORNER);
		CP_Settings_Clip(x, top + size * (1.0f - ready), size, size * ready);
		DrawAbilityIcon(i, x, top, size, colors[i], CP_Color_Create(255, 255, 255, 255));
		CP_Settings_NoClip();

		CP_Settings_Fill(CP_Color_Create(240, 240, 250, 255));
		CP_Settings_TextSize(22.0f);
		CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_TOP);
		CP_Font_DrawText(keys[i], x + size / 2.0f, top + size + 8.0f);
	}
}

//---------------------------------------------------------
// A title revealed by a wipe: the clip starts with no width and grows to the
// title's full width, then waits, and starts again

void DrawWipe(float left, float top, float width, float height)
{
	const float t = CP_Math_ClampFloat(wipeTime / 1.2f, 0.0f, 1.0f);
	const float eased = t * t * (3.0f - 2.0f * t); // slow at the start and the end

	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Clip(left, top, width * eased, height);

	CP_Settings_NoStroke();
	CP_Settings_FillLinearGradient(left, 0.0f, CP_Color_Create(250, 170, 60, 255), left + width, 0.0f, CP_Color_Create(230, 70, 120, 255));
	CP_Graphics_DrawRoundedRect(left, top, width, height, 12.0f);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	CP_Settings_TextSize(40.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Font_DrawText("LEVEL CLEAR!", left + width / 2.0f, top + height / 2.0f);

	CP_Settings_NoClip();
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize((int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Clipping");

	player = CP_Vector_Set(WORLD_SIZE / 2.0f, WORLD_SIZE / 2.0f);
	CP_Random_Seed(7);
	for (int i = 0; i < TREE_COUNT; ++i)
	{
		trees[i] = CP_Vector_Set(CP_Random_RangeFloat(0.0f, WORLD_SIZE), CP_Random_RangeFloat(0.0f, WORLD_SIZE));
	}
}

void game_update(void)
{
	UpdateInventory();
	UpdatePlayer();
	UpdateAbilities();
	wipeTime += CP_System_GetDt();
	if (wipeTime > 3.0f)
	{
		wipeTime = 0.0f;
	}

	CP_Graphics_ClearBackground(CP_Color_Create(18, 18, 28, 255));
	DrawInventory();
	DrawMinimap();
	DrawAbilities(MAP_LEFT + 5.0f, 330.0f);
	DrawWipe(MAP_LEFT, 460.0f, MAP_WIDTH, 70.0f);

	CP_Settings_Fill(CP_Color_Create(150, 150, 170, 255));
	CP_Settings_TextSize(18.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BOTTOM);
	CP_Font_DrawText("Wheel or drag: scroll   Click: pick   Arrows: move   1 2 3: abilities", WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 16.0f);
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

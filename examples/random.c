//---------------------------------------------------------
// file:	random.c
//
// Example: random numbers for games: dice, loot, shuffled cards, chances,
// spread and worlds you can make again.
//
// Shows how to:
// - roll dice with CP_Random_RangeInt, which can give both of its ends, and
//   flip a coin with CP_Random_GetBool
// - see why two dice give 7 more often than 2 or 12
// - pick from a list, and never the same thing twice in a row
// - shuffle cards so every order is as likely (the Fisher-Yates shuffle)
// - give something a chance, like a 25% critical hit, with CP_Random_GetFloat
// - make some loot rarer than others, with weights
// - spread shots evenly with CP_Random_RangeFloat, or mostly near the middle
//   with CP_Random_Gaussian (a bell curve)
// - make the same world every time from a seed, with CP_Random_Seed
//
// Random numbers in a computer come from a formula: each one is made from
// the one before. A seed is where the formula starts, so the same seed gives
// the same numbers, in the same order. CProcessing picks a seed from the
// clock when it starts, so every run is different until you choose one.
//
// Press 1 to 6 to do what the box with that number shows. Hold Shift with 1,
// 4 or 5 to do it 100 times and watch the counts take shape. Left and Right
// change the seed in box 6.
//
// Try this:
// - Roll three dice. Which totals come up most now?
// - Shuffle a playlist, and shuffle again when it ends, without the last
//   song coming first.
// - Give the chest a "pity timer": a legendary after 20 chests without one.
// - Make the seed from a word the player types, like a level code.
//
// See also: noise.c, for random that changes smoothly; tiled_fill.c, for
// pebbles placed from a seed.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

//---------------------------------------------------------
// What the boxes share

#define COLUMNS    3
#define BOX_WIDTH  400.0f
#define BOX_HEIGHT 320.0f
#define MARGIN     18.0f

CP_Color textColor;
CP_Color dimColor;
CP_Color barColor;

// The box itself, its title and two lines of code at the bottom
void DrawBox(float left, float top, const char* title, const char* code1, const char* code2)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(40, 44, 60, 255));
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Graphics_DrawRect(left, top, BOX_WIDTH, BOX_HEIGHT);

	CP_Settings_Fill(CP_Color_Create(230, 230, 240, 255));
	CP_Settings_TextSize(20.0f);
	CP_Font_DrawText(title, left + 12.0f, top + 26.0f);

	CP_Settings_Fill(CP_Color_Create(150, 200, 255, 255));
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(code1, left + 12.0f, top + BOX_HEIGHT - 32.0f);
	CP_Font_DrawText(code2, left + 12.0f, top + BOX_HEIGHT - 12.0f);
}

// How many times to do it: 100 with Shift held, otherwise once
int TimesToDo(void)
{
	return CP_Input_KeyDown(CP_KEY_LEFT_SHIFT) || CP_Input_KeyDown(CP_KEY_RIGHT_SHIFT) ? 100 : 1;
}

// Bars for counts: one per count, as tall as its share of the biggest, with
// a label under each
void DrawBars(float left, float bottom, float width, float height, const int counts[], int count, const char* labels[])
{
	int biggest = 1;
	for (int i = 0; i < count; ++i)
	{
		if (counts[i] > biggest)
		{
			biggest = counts[i];
		}
	}
	const float barWidth = width / count;
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_TextSize(14.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_BASELINE);
	for (int i = 0; i < count; ++i)
	{
		const float x = left + i * barWidth;
		const float barHeight = height * counts[i] / biggest;
		CP_Settings_Fill(barColor);
		CP_Graphics_DrawRect(x + 2.0f, bottom - barHeight, barWidth - 4.0f, barHeight);
		CP_Settings_Fill(dimColor);
		CP_Font_DrawText(labels[i], x + barWidth / 2.0f, bottom + 16.0f);
	}
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
}

//---------------------------------------------------------
// 1. Dice and coins

int dice[2] = { 3, 4 };
bool heads = true;
int totals[11]; // how many times each total came up: totals[0] is 2, totals[10] is 12
const char* totalLabels[11] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12" };

void RollDice(void)
{
	for (int roll = TimesToDo(); roll > 0; --roll)
	{
		// RangeInt gives any whole number from the first to the second, both included
		dice[0] = CP_Random_RangeInt(1, 6);
		dice[1] = CP_Random_RangeInt(1, 6);
		totals[dice[0] + dice[1] - 2]++;
		heads = CP_Random_GetBool();
	}
}

// A die with its pips: 3 by 3 places, and which of them each face uses
void DrawDie(float x, float y, float size, int value)
{
	static const char* faces[6] = { "....o....", "o.......o", "o...o...o", "o.o...o.o", "o.o.o.o.o", "o.oo.oo.o" };
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CENTER);
	CP_Settings_Fill(CP_Color_Create(245, 245, 240, 255));
	CP_Graphics_DrawRect(x, y, size, size);
	CP_Settings_Fill(CP_Color_Create(30, 30, 40, 255));
	const float spacing = size * 0.28f;
	for (int place = 0; place < 9; ++place)
	{
		if (faces[value - 1][place] == 'o')
		{
			CP_Graphics_DrawCircle(x + (place % 3 - 1) * spacing, y + (place / 3 - 1) * spacing, size * 0.18f);
		}
	}
}

void DrawDiceBox(float left, float top)
{
	DrawBox(left, top, "1. Dice and a coin", "die = CP_Random_RangeInt(1, 6);", "heads = CP_Random_GetBool();");
	DrawDie(left + 50.0f, top + 80.0f, 56.0f, dice[0]);
	DrawDie(left + 120.0f, top + 80.0f, 56.0f, dice[1]);

	CP_Settings_Fill(heads ? CP_Color_Create(240, 200, 80, 255) : CP_Color_Create(190, 190, 200, 255));
	CP_Graphics_DrawCircle(left + 340.0f, top + 80.0f, 56.0f);
	CP_Settings_Fill(textColor);
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText(heads ? "Heads" : "Tails", left + 318.0f, top + 130.0f);

	CP_Settings_Fill(dimColor);
	CP_Font_DrawText("Totals of two dice:", left + 12.0f, top + 140.0f);
	DrawBars(left + 12.0f, top + 250.0f, BOX_WIDTH - 24.0f, 100.0f, totals, 11, totalLabels);
}

//---------------------------------------------------------
// 2. Pick from a list

#define LOOT_COUNT 8
const char* loot[LOOT_COUNT] = { "Sword", "Shield", "Bow", "Potion", "Bomb", "Key", "Map", "Boots" };
#define HISTORY_COUNT 8
int history[HISTORY_COUNT]; // the last picks, the newest first; -1 for none yet

void PickLoot(void)
{
	// 0 to LOOT_COUNT - 1: the first and the last place in the array. Again
	// if it's the same as last time, so the same never comes twice in a row.
	int pick;
	do
	{
		pick = CP_Random_RangeInt(0, LOOT_COUNT - 1);
	} while (pick == history[0]);

	for (int i = HISTORY_COUNT - 1; i > 0; --i)
	{
		history[i] = history[i - 1];
	}
	history[0] = pick;
}

void DrawPickBox(float left, float top)
{
	DrawBox(left, top, "2. Pick from a list", "loot[CP_Random_RangeInt(0, LOOT_COUNT - 1)]", "Same as last time? Pick again.");
	CP_Settings_Fill(textColor);
	CP_Settings_TextSize(40.0f);
	CP_Font_DrawText(history[0] >= 0 ? loot[history[0]] : "Press 2", left + 24.0f, top + 100.0f);

	CP_Settings_Fill(dimColor);
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText("Before that:", left + 24.0f, top + 140.0f);
	for (int i = 1; i < HISTORY_COUNT && history[i] >= 0; ++i)
	{
		CP_Font_DrawText(loot[history[i]], left + 24.0f + ((i - 1) % 4) * 90.0f, top + 168.0f + ((i - 1) / 4) * 24.0f);
	}
}

//---------------------------------------------------------
// 3. Shuffle

#define CARD_COUNT 10
int cards[CARD_COUNT];

// Fisher-Yates: from the last card down, swap each card with one at or
// before it, chosen at random. Every order is as likely, and no card is lost
// or doubled. (Swapping with any card, not just one at or before it, makes
// some orders more likely than others.)
void Shuffle(int values[], int count)
{
	for (int last = count - 1; last > 0; --last)
	{
		const int other = CP_Random_RangeInt(0, last);
		const int swap = values[last];
		values[last] = values[other];
		values[other] = swap;
	}
}

void DrawShuffleBox(float left, float top)
{
	DrawBox(left, top, "3. Shuffle", "Swap each card, from the last, with", "CP_Random_RangeInt(0, last)");
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_TextSize(26.0f);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	for (int i = 0; i < CARD_COUNT; ++i)
	{
		const float x = left + 24.0f + (i % 5) * 72.0f;
		const float y = top + 50.0f + (i / 5) * 100.0f;
		CP_Settings_NoStroke();
		CP_Settings_Fill(CP_Color_Create(245, 245, 240, 255));
		CP_Graphics_DrawRect(x, y, 60.0f, 84.0f);
		CP_Settings_Fill(cards[i] % 2 ? CP_Color_Create(200, 50, 60, 255) : CP_Color_Create(30, 30, 40, 255));
		char text[4];
		snprintf(text, sizeof(text), "%d", cards[i]);
		CP_Font_DrawText(text, x + 30.0f, y + 42.0f);
	}
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);
}

//---------------------------------------------------------
// 4. Chances and weights

#define RARITY_COUNT 4
const char* rarities[RARITY_COUNT] = { "Common", "Rare", "Epic", "Legendary" };
const float weights[RARITY_COUNT] = { 60.0f, 30.0f, 9.0f, 1.0f }; // they needn't add up to 100
int rarityCounts[RARITY_COUNT];
int lastRarity = -1;
int chests = 0;
int criticalHits = 0;
bool lastWasCritical = false;

// A number from 0 to the total of the weights, then take each weight off in
// turn: the one that takes it below 0 wins. A weight of 30 wins three times
// as often as a weight of 10.
int PickWeighted(const float weightList[], int count)
{
	float total = 0.0f;
	for (int i = 0; i < count; ++i)
	{
		total += weightList[i];
	}
	float roll = CP_Random_RangeFloat(0.0f, total);
	for (int i = 0; i < count; ++i)
	{
		roll -= weightList[i];
		if (roll < 0.0f)
		{
			return i;
		}
	}
	return count - 1; // only if roll was exactly total
}

void OpenChests(void)
{
	for (int chest = TimesToDo(); chest > 0; --chest)
	{
		lastRarity = PickWeighted(weights, RARITY_COUNT);
		rarityCounts[lastRarity]++;
		chests++;

		// GetFloat is from 0 up to (not including) 1, so it's below 0.25 a
		// quarter of the time
		lastWasCritical = CP_Random_GetFloat() < 0.25f;
		if (lastWasCritical)
		{
			criticalHits++;
		}
	}
}

void DrawChanceBox(float left, float top)
{
	DrawBox(left, top, "4. Chances and weights", "critical = CP_Random_GetFloat() < 0.25f;", "PickWeighted(weights, RARITY_COUNT)");
	char text[80];
	CP_Settings_TextSize(18.0f);
	CP_Settings_Fill(textColor);
	snprintf(text, sizeof(text), "Chest: %s", lastRarity >= 0 ? rarities[lastRarity] : "press 4");
	CP_Font_DrawText(text, left + 24.0f, top + 62.0f);
	snprintf(text, sizeof(text), "25%% critical hit: %s", chests == 0 ? "-" : (lastWasCritical ? "yes!" : "no"));
	CP_Font_DrawText(text, left + 210.0f, top + 62.0f);

	CP_Settings_Fill(dimColor);
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText("Weight", left + 130.0f, top + 98.0f);
	CP_Font_DrawText("Chests", left + 210.0f, top + 98.0f);
	CP_Font_DrawText("Share", left + 300.0f, top + 98.0f);
	for (int i = 0; i < RARITY_COUNT; ++i)
	{
		const float y = top + 124.0f + i * 22.0f;
		const float share = chests > 0 ? 100.0f * rarityCounts[i] / chests : 0.0f;
		CP_Font_DrawText(rarities[i], left + 24.0f, y);
		snprintf(text, sizeof(text), "%.0f", weights[i]);
		CP_Font_DrawText(text, left + 130.0f, y);
		snprintf(text, sizeof(text), "%d", rarityCounts[i]);
		CP_Font_DrawText(text, left + 210.0f, y);
		snprintf(text, sizeof(text), "%.1f%%", share);
		CP_Font_DrawText(text, left + 300.0f, y);
	}
	const float criticalShare = chests > 0 ? 100.0f * criticalHits / chests : 0.0f;
	snprintf(text, sizeof(text), "Critical hits: %d of %d (%.1f%%)", criticalHits, chests, criticalShare);
	CP_Font_DrawText(text, left + 24.0f, top + 222.0f);
}

//---------------------------------------------------------
// 5. Spread: even, or a bell curve

#define SHOT_COUNT 300
CP_Vector evenShots[SHOT_COUNT]; // from the middle of the target
CP_Vector bellShots[SHOT_COUNT];
int shotsFired = 0; // the next goes at shotsFired % SHOT_COUNT

void Fire(void)
{
	for (int shot = TimesToDo(); shot > 0; --shot)
	{
		const int i = shotsFired % SHOT_COUNT;
		// Anywhere in a square, every place as likely
		evenShots[i] = CP_Vector_Set(CP_Random_RangeFloat(-60.0f, 60.0f), CP_Random_RangeFloat(-60.0f, 60.0f));
		// Gaussian is 0 on average; about 2 in 3 are between -1 and 1, and
		// hardly any beyond -3 or 3. Times 20: most land within 20 pixels.
		bellShots[i] = CP_Vector_Set(CP_Random_Gaussian() * 20.0f, CP_Random_Gaussian() * 20.0f);
		shotsFired++;
	}
}

void DrawTarget(float x, float y, const CP_Vector shots[])
{
	CP_Settings_NoFill();
	CP_Settings_StrokeWeight(2.0f);
	CP_Settings_Stroke(CP_Color_Create(90, 95, 120, 255));
	for (int ring = 1; ring <= 3; ++ring)
	{
		CP_Graphics_DrawCircle(x, y, ring * 40.0f);
	}
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 120, 90, 200));
	const int count = shotsFired < SHOT_COUNT ? shotsFired : SHOT_COUNT;
	for (int i = 0; i < count; ++i)
	{
		CP_Graphics_DrawCircle(x + shots[i].x, y + shots[i].y, 5.0f);
	}
}

void DrawSpreadBox(float left, float top)
{
	DrawBox(left, top, "5. Spread: even, or a bell curve", "x = CP_Random_RangeFloat(-60, 60);", "x = CP_Random_Gaussian() * 20;");
	DrawTarget(left + 105.0f, top + 140.0f, evenShots);
	DrawTarget(left + 295.0f, top + 140.0f, bellShots);
	CP_Settings_Fill(dimColor);
	CP_Settings_TextSize(16.0f);
	CP_Font_DrawText("RangeFloat", left + 68.0f, top + 240.0f);
	CP_Font_DrawText("Gaussian", left + 264.0f, top + 240.0f);
}

//---------------------------------------------------------
// 6. A world from a seed

#define STAR_COUNT 40
#define TREE_COUNT 7
int seed = 1;
CP_Vector stars[STAR_COUNT]; // from the box's top left
float starSizes[STAR_COUNT];
float treeX[TREE_COUNT];
float treeHeights[TREE_COUNT];
float moonX;

// Everything random in the world comes after CP_Random_Seed, so a seed always
// makes the same world. Seeding changes every random number after it,
// though, and the dice would roll the same in every run too. So take a
// random number first, and seed with it afterwards: the rest of the game
// stays as unpredictable as before.
void MakeWorld(void)
{
	const unsigned int carryOn = CP_Random_GetInt();
	CP_Random_Seed(seed);

	for (int i = 0; i < STAR_COUNT; ++i)
	{
		stars[i] = CP_Vector_Set(CP_Random_RangeFloat(20.0f, BOX_WIDTH - 20.0f), CP_Random_RangeFloat(50.0f, 170.0f));
		starSizes[i] = CP_Random_RangeFloat(1.5f, 4.0f);
	}
	for (int i = 0; i < TREE_COUNT; ++i)
	{
		treeX[i] = CP_Random_RangeFloat(20.0f, BOX_WIDTH - 20.0f);
		treeHeights[i] = CP_Random_RangeFloat(30.0f, 70.0f);
	}
	moonX = CP_Random_RangeFloat(40.0f, BOX_WIDTH - 40.0f);

	CP_Random_Seed((int)carryOn);
}

void DrawSeedBox(float left, float top)
{
	DrawBox(left, top, "6. A world from a seed", "CP_Random_Seed(seed);", "...then make everything random");
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(250, 245, 210, 255));
	CP_Graphics_DrawCircle(left + moonX, top + 70.0f, 34.0f);
	CP_Settings_Fill(CP_Color_Create(220, 220, 255, 255));
	for (int i = 0; i < STAR_COUNT; ++i)
	{
		CP_Graphics_DrawCircle(left + stars[i].x, top + stars[i].y, starSizes[i]);
	}
	const float ground = top + 250.0f;
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_Fill(CP_Color_Create(50, 90, 60, 255));
	CP_Graphics_DrawRect(left, ground, BOX_WIDTH, 20.0f);
	CP_Settings_Fill(CP_Color_Create(70, 150, 90, 255));
	for (int i = 0; i < TREE_COUNT; ++i)
	{
		CP_Graphics_DrawTriangle(left + treeX[i], ground - treeHeights[i], left + treeX[i] - 16.0f, ground, left + treeX[i] + 16.0f, ground);
	}

	char text[40];
	snprintf(text, sizeof(text), "Seed: %d", seed);
	CP_Settings_Fill(textColor);
	CP_Settings_TextSize(18.0f);
	CP_Font_DrawText(text, left + BOX_WIDTH - 150.0f, top + 26.0f);
}

//---------------------------------------------------------

void game_init(void)
{
	CP_System_SetWindowSize((int)(COLUMNS * (BOX_WIDTH + MARGIN) + MARGIN), 720);
	CP_System_SetWindowTitle("Random numbers");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);

	textColor = CP_Color_Create(240, 240, 245, 255);
	dimColor = CP_Color_Create(170, 175, 195, 255);
	barColor = CP_Color_Create(90, 170, 240, 255);

	for (int i = 0; i < HISTORY_COUNT; ++i)
	{
		history[i] = -1;
	}
	for (int i = 0; i < CARD_COUNT; ++i)
	{
		cards[i] = i + 1;
	}
	MakeWorld();
}

void game_update(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_1))
	{
		RollDice();
	}
	if (CP_Input_KeyTriggered(CP_KEY_2))
	{
		PickLoot();
	}
	if (CP_Input_KeyTriggered(CP_KEY_3))
	{
		Shuffle(cards, CARD_COUNT);
	}
	if (CP_Input_KeyTriggered(CP_KEY_4))
	{
		OpenChests();
	}
	if (CP_Input_KeyTriggered(CP_KEY_5))
	{
		Fire();
	}
	if (CP_Input_KeyTriggered(CP_KEY_6))
	{
		seed = CP_Random_RangeInt(1, 99999); // a new seed, at random
		MakeWorld();
	}
	if (CP_Input_KeyTriggered(CP_KEY_LEFT) && seed > 1)
	{
		seed--;
		MakeWorld();
	}
	if (CP_Input_KeyTriggered(CP_KEY_RIGHT))
	{
		seed++;
		MakeWorld();
	}

	CP_Graphics_ClearBackground(CP_Color_Create(24, 26, 36, 255));

	const float column[COLUMNS] = { MARGIN, MARGIN + (BOX_WIDTH + MARGIN), MARGIN + 2.0f * (BOX_WIDTH + MARGIN) };
	const float topRow = MARGIN;
	const float bottomRow = MARGIN * 2.0f + BOX_HEIGHT;

	DrawDiceBox(column[0], topRow);
	DrawPickBox(column[1], topRow);
	DrawShuffleBox(column[2], topRow);
	DrawChanceBox(column[0], bottomRow);
	DrawSpreadBox(column[1], bottomRow);
	DrawSeedBox(column[2], bottomRow);

	CP_Settings_Fill(CP_Color_Create(170, 170, 190, 255));
	CP_Settings_TextSize(20.0f);
	CP_Font_DrawText("1-6: do it once    Shift with 1, 4 or 5: 100 times    Left/Right: the seed", MARGIN, 705.0f);
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

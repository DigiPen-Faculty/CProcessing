//---------------------------------------------------------
// file:	score.c
//
// brief:	The score, shared by the game and the menu
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "score.h"

// static: only the functions in this file can see the variable
static int score = 0;

int GetScore(void)
{
	return score;
}

void SetScore(int newScore)
{
	score = newScore;
}

void IncrementScore(void)
{
	score++;
}

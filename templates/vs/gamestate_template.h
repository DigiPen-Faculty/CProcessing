//---------------------------------------------------------
// file:	gamestate_template.h
// author:	[YOUR NAME]
//
// brief:	[WHAT THIS GAME STATE DOES]
//
// A blank game state: an init, an update and an exit function. To make a
// new state, for example a menu:
//   1. Copy this file and gamestate_template.c, and rename both copies
//      (gamestate_menu.h and gamestate_menu.c).
//   2. In both, change every "template" to "menu".
//   3. Add the two files to your project.
//   4. Start it with CP_Engine_SetNextGameState(gamestate_menu_init,
//      gamestate_menu_update, gamestate_menu_exit), after
//      #include "gamestate_menu.h".
// Project 6, "A game in many files", in the documentation explains it step
// by step.
//---------------------------------------------------------

#pragma once

void gamestate_template_init(void);
void gamestate_template_update(void);
void gamestate_template_exit(void);

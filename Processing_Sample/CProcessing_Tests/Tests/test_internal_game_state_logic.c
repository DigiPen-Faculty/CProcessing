// Tier 1 coverage for Source/Internal_GameStateLogic.h: the rules of the
// game state stack. The hooks write what happened into a log, so each test
// checks the order in which inits and exits run, and which states run and
// are seen.
#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "Internal_GameStateLogic.h"

// Three states, told apart by their function pointers
// (each does something different, so an optimizing linker can't fold them
// into one function with one address)
static volatile int gCalls[9];
static void MenuInit(void) { gCalls[0]++; }
static void MenuUpdate(void) { gCalls[1]++; }
static void MenuExit(void) { gCalls[2]++; }
static void GameInit(void) { gCalls[3]++; }
static void GameUpdate(void) { gCalls[4]++; }
static void GameExit(void) { gCalls[5]++; }
static void PauseInit(void) { gCalls[6]++; }
static void PauseUpdate(void) { gCalls[7]++; }
static void PauseExit(void) { gCalls[8]++; }

static const CP_GSL_State kMenu = { MenuInit, MenuUpdate, MenuExit, CP_BELOW_FROZEN };
static const CP_GSL_State kGame = { GameInit, GameUpdate, GameExit, CP_BELOW_FROZEN };
static const CP_GSL_State kPause = { PauseInit, PauseUpdate, PauseExit, CP_BELOW_FROZEN };

static char gLog[512];

static const char* Name(const CP_GSL_State* state)
{
    if (state->update == MenuUpdate) return "menu";
    if (state->update == GameUpdate) return "game";
    if (state->update == PauseUpdate) return "pause";
    return "?";
}

static CP_GSL_Stack gStack;

static void Log(const char* what, int index)
{
    const size_t used = strlen(gLog);
    snprintf(gLog + used, sizeof(gLog) - used, "%s%s.%s@%d", used ? " " : "", Name(&gStack.states[index]), what, index);
}

static void HookEnter(void* user, int index, CP_GSL_RequestKind why) { (void)user; (void)why; Log("init", index); }
static void HookLeave(void* user, int index, CP_GSL_RequestKind why) { (void)user; (void)why; Log("exit", index); }
static void HookUncover(void* user, int index) { (void)user; Log("top", index); }

static const CP_GSL_Hooks kHooks = { HookEnter, HookLeave, HookUncover, NULL };

static void Reset(void)
{
    memset(&gStack, 0, sizeof(gStack));
    gLog[0] = '\0';
}

// Apply everything requested, as the engine does at the start of a frame
static void NextFrame(void)
{
    CP_GSL_Request requests[CP_MAX_GAME_STATE_REQUESTS];
    const int count = CP_GSL_TakeRequests(&gStack, requests);
    for (int i = 0; i < count; ++i)
    {
        CP_GSL_Apply(&gStack, &requests[i], &kHooks);
    }
}

static void StartIn(CP_GSL_State state)
{
    Reset();
    CP_GSL_Ask(&gStack, CP_GSL_SET, state);
    NextFrame();
    gLog[0] = '\0';
}

void test_gamestate_first_set_runs_init(void)
{
    Reset();
    CP_GSL_Ask(&gStack, CP_GSL_SET, kMenu);
    TEST_ASSERT_EQUAL_INT(0, gStack.depth);     // nothing happens until the next frame
    NextFrame();
    TEST_ASSERT_EQUAL_INT(1, gStack.depth);
    TEST_ASSERT_EQUAL_STRING("menu.init@0", gLog);
}

void test_gamestate_set_exits_then_inits(void)
{
    StartIn(kMenu);
    CP_GSL_Ask(&gStack, CP_GSL_SET, kGame);
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("menu.exit@0 game.init@0", gLog);
    TEST_ASSERT_EQUAL_INT(1, gStack.depth);
}

void test_gamestate_set_to_the_same_state_does_nothing(void)
{
    StartIn(kGame);
    TEST_ASSERT_EQUAL_INT(CP_GSL_DONE, CP_GSL_Ask(&gStack, CP_GSL_SET, kGame));
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("", gLog);
}

void test_gamestate_forced_set_restarts_the_same_state(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_SET_FORCED, kGame);
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("game.exit@0 game.init@0", gLog);
}

void test_gamestate_push_runs_only_the_new_init(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("pause.init@1", gLog);     // no game.exit
    TEST_ASSERT_EQUAL_INT(2, gStack.depth);
}

void test_gamestate_pop_runs_only_the_top_exit(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    NextFrame();
    gLog[0] = '\0';
    CP_GSL_Ask(&gStack, CP_GSL_POP, kPause);
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("pause.exit@1 game.top@0", gLog);   // no game.init
    TEST_ASSERT_EQUAL_INT(1, gStack.depth);
}

void test_gamestate_pop_with_one_state_is_ignored(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_POP, kGame);
    CP_GSL_Request request = gStack.queue[0];
    TEST_ASSERT_EQUAL_INT(CP_GSL_POP_LAST, CP_GSL_Apply(&gStack, &request, &kHooks));
    TEST_ASSERT_EQUAL_INT(1, gStack.depth);
    TEST_ASSERT_EQUAL_STRING("", gLog);
}

void test_gamestate_set_from_a_pause_exits_every_state_from_the_top(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    NextFrame();
    gLog[0] = '\0';
    CP_GSL_Ask(&gStack, CP_GSL_SET, kMenu);
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("pause.exit@1 game.top@0 game.exit@0 menu.init@0", gLog);
    TEST_ASSERT_EQUAL_INT(1, gStack.depth);
}

void test_gamestate_set_with_a_stack_of_the_same_state_still_changes(void)
{
    // the game with a pause on top is not "only the game"
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    NextFrame();
    gLog[0] = '\0';
    CP_GSL_Ask(&gStack, CP_GSL_SET, kGame);
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("pause.exit@1 game.top@0 game.exit@0 game.init@0", gLog);
}

void test_gamestate_leave_all_exits_from_the_top(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kMenu);
    NextFrame();
    gLog[0] = '\0';
    CP_GSL_LeaveAll(&gStack, &kHooks, CP_GSL_SET_FORCED);
    TEST_ASSERT_EQUAL_STRING("menu.exit@2 pause.top@1 pause.exit@1 game.top@0 game.exit@0", gLog);
    TEST_ASSERT_EQUAL_INT(0, gStack.depth);
}

void test_gamestate_pushes_and_pops_apply_in_order(void)
{
    // a pop then a push in one frame swaps the top state
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    NextFrame();
    gLog[0] = '\0';
    CP_GSL_Ask(&gStack, CP_GSL_POP, kPause);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kMenu);
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("pause.exit@1 game.top@0 menu.init@1", gLog);
    TEST_ASSERT_EQUAL_INT(2, gStack.depth);
}

void test_gamestate_a_set_drops_what_was_asked_before_it(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    CP_GSL_Ask(&gStack, CP_GSL_SET, kMenu);
    TEST_ASSERT_EQUAL_INT(1, gStack.queued);
    // ...but a push after a set still happens, on top of the new state
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("game.exit@0 menu.init@0 pause.init@1", gLog);
}

void test_gamestate_the_last_set_wins(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_SET, kMenu);
    CP_GSL_Ask(&gStack, CP_GSL_SET, kGame);     // back to the state it's in: nothing to do
    NextFrame();
    TEST_ASSERT_EQUAL_STRING("", gLog);
}

void test_gamestate_requests_without_update_are_ignored(void)
{
    Reset();
    CP_GSL_State noUpdate = { MenuInit, NULL, MenuExit, CP_BELOW_FROZEN };
    TEST_ASSERT_EQUAL_INT(CP_GSL_NO_UPDATE, CP_GSL_Ask(&gStack, CP_GSL_SET, noUpdate));
    TEST_ASSERT_EQUAL_INT(CP_GSL_NO_UPDATE, CP_GSL_Ask(&gStack, CP_GSL_PUSH, noUpdate));
    TEST_ASSERT_EQUAL_INT(0, gStack.queued);
}

void test_gamestate_requests_made_while_applying_wait_for_the_next_frame(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    CP_GSL_Request requests[CP_MAX_GAME_STATE_REQUESTS];
    const int count = CP_GSL_TakeRequests(&gStack, requests);
    TEST_ASSERT_EQUAL_INT(1, count);
    // as if pause's init asked for something
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kMenu);
    CP_GSL_Apply(&gStack, &requests[0], &kHooks);
    TEST_ASSERT_EQUAL_INT(2, gStack.depth);
    TEST_ASSERT_EQUAL_INT(1, gStack.queued);
}

void test_gamestate_stack_and_queue_limits(void)
{
    StartIn(kGame);
    for (int i = 1; i < CP_MAX_GAME_STATES; ++i)
    {
        CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
        NextFrame();
    }
    TEST_ASSERT_EQUAL_INT(CP_MAX_GAME_STATES, gStack.depth);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    CP_GSL_Request request = gStack.queue[0];
    TEST_ASSERT_EQUAL_INT(CP_GSL_STACK_FULL, CP_GSL_Apply(&gStack, &request, &kHooks));

    Reset();
    for (int i = 0; i < CP_MAX_GAME_STATE_REQUESTS; ++i)
    {
        TEST_ASSERT_EQUAL_INT(CP_GSL_DONE, CP_GSL_Ask(&gStack, CP_GSL_POP, kGame));
    }
    TEST_ASSERT_EQUAL_INT(CP_GSL_QUEUE_FULL, CP_GSL_Ask(&gStack, CP_GSL_POP, kGame));
}

void test_gamestate_which_states_run(void)
{
    StartIn(kGame);
    CP_GSL_State dialog = kPause;
    dialog.below = CP_BELOW_RUNNING;
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, dialog);
    NextFrame();
    TEST_ASSERT_TRUE(CP_GSL_IsRunning(&gStack, 0));     // the game runs under a RUNNING dialog
    TEST_ASSERT_TRUE(CP_GSL_IsRunning(&gStack, 1));

    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kMenu);         // a frozen state on top stops both
    NextFrame();
    TEST_ASSERT_FALSE(CP_GSL_IsRunning(&gStack, 0));
    TEST_ASSERT_FALSE(CP_GSL_IsRunning(&gStack, 1));
    TEST_ASSERT_TRUE(CP_GSL_IsRunning(&gStack, 2));
    TEST_ASSERT_FALSE(CP_GSL_IsRunning(&gStack, 3));     // past the top
}

void test_gamestate_which_states_are_seen(void)
{
    StartIn(kGame);
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    NextFrame();
    TEST_ASSERT_EQUAL_INT(0, CP_GSL_LowestVisible(gStack.states, gStack.depth));    // frozen: still seen

    CP_GSL_State options = kMenu;
    options.below = CP_BELOW_HIDDEN;
    CP_GSL_Ask(&gStack, CP_GSL_PUSH, options);
    NextFrame();
    TEST_ASSERT_EQUAL_INT(2, CP_GSL_LowestVisible(gStack.states, gStack.depth));    // hides both below

    CP_GSL_Ask(&gStack, CP_GSL_PUSH, kPause);
    NextFrame();
    TEST_ASSERT_EQUAL_INT(2, CP_GSL_LowestVisible(gStack.states, gStack.depth));    // still hidden under it
}

void test_gamestate_a_bottom_state_is_never_hidden_or_running_below(void)
{
    // the mode describes the states under a pushed one; a set state has none
    Reset();
    CP_GSL_State hidden = kGame;
    hidden.below = CP_BELOW_HIDDEN;
    CP_GSL_Ask(&gStack, CP_GSL_SET, hidden);
    NextFrame();
    TEST_ASSERT_EQUAL_INT(CP_BELOW_FROZEN, gStack.states[0].below);
}

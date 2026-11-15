//------------------------------------------------------------------------------
// file:	Internal_GameStateLogic.h
// brief:	The rules of the game state stack, used by CP_System.c.
//
//			Game states form a stack, like cards on a table.
//			CP_Engine_SetNextGameState clears the table and puts down a new
//			card; CP_Engine_PushGameState puts a card on top (the ones under it
//			stay, frozen by default); CP_Engine_PopGameState takes the card
//			that asks off, with any on top of it. Requests are queued and
//			applied at the start of the next frame.
//
//			This file is only the bookkeeping: which state is where, what is
//			queued, and in what order init and exit run. It calls back into
//			the engine (CP_GSL_Hooks) to actually run a state's init or exit,
//			so it has no globals, no window and no OpenGL, and the Tier 1 test
//			project tests it directly.
//
// Internal header: not part of the public API.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2026 DigiPen Institute of Technology
//------------------------------------------------------------------------------

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "cprocessing_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CP_MAX_GAME_STATES			16	// how deep the stack can go
#define CP_MAX_GAME_STATE_REQUESTS	16	// requests waiting for the next frame

typedef struct CP_GSL_State
{
	FunctionPtr init;
	FunctionPtr update;
	FunctionPtr exit;
	CP_STATE passThrough;	// what the states under this one can do
} CP_GSL_State;

typedef enum CP_GSL_RequestKind
{
	CP_GSL_PUSH,
	CP_GSL_POP,
	CP_GSL_SET,			// CP_Engine_SetNextGameState: ignored if it is already the only state
	CP_GSL_SET_FORCED	// CP_Engine_SetNextGameStateForced: always
} CP_GSL_RequestKind;

typedef struct CP_GSL_Request
{
	CP_GSL_RequestKind kind;
	CP_GSL_State state;		// unused for a pop
	int from;				// a pop: the state that asked (it leaves with those above it); -1 for the top
} CP_GSL_Request;

typedef enum CP_GSL_Result
{
	CP_GSL_DONE,
	CP_GSL_SAME_STATE,		// a set for the state that is already the only one: nothing to do
	CP_GSL_NO_UPDATE,		// a push or set without an update function: ignored
	CP_GSL_POP_LAST,		// a pop that would leave no state: ignored (the game needs a state)
	CP_GSL_GONE,			// a pop from a state that has already left: nothing to do
	CP_GSL_STACK_FULL,		// a push past CP_MAX_GAME_STATES: ignored
	CP_GSL_QUEUE_FULL		// more than CP_MAX_GAME_STATE_REQUESTS in one frame: ignored
} CP_GSL_Result;

typedef struct CP_GSL_Stack
{
	CP_GSL_State states[CP_MAX_GAME_STATES];	// [0] is the bottom
	int depth;
	CP_GSL_Request queue[CP_MAX_GAME_STATE_REQUESTS];
	int queued;
} CP_GSL_Stack;

// How the rules run a state. index is the state's place in the stack.
//   enter: the state was just put at index (index is the new top); run its init
//   leave: the state at index (the top) is about to be taken off; run its exit
//   uncover: the state at index is the top again after the one above it left
typedef struct CP_GSL_Hooks
{
	void (*enter)(void* user, int index, CP_GSL_RequestKind why);
	void (*leave)(void* user, int index, CP_GSL_RequestKind why);
	void (*uncover)(void* user, int index);
	void* user;
} CP_GSL_Hooks;

static inline bool CP_GSL_SameState(const CP_GSL_State* a, const CP_GSL_State* b)
{
	return a->init == b->init && a->update == b->update && a->exit == b->exit;
}

//------------------------------------------------------------------------------
// Requests: made at any time, applied at the start of the next frame.

static inline CP_GSL_Result CP_GSL_Ask(CP_GSL_Stack* stack, CP_GSL_RequestKind kind, CP_GSL_State state)
{
	if (kind != CP_GSL_POP && state.update == NULL)
	{
		return CP_GSL_NO_UPDATE;
	}
	if (kind == CP_GSL_SET || kind == CP_GSL_SET_FORCED)
	{
		// A set ends every state, so whatever was asked before it in this
		// frame doesn't matter: the last set wins, as it always has
		stack->queued = 0;
	}
	if (stack->queued >= CP_MAX_GAME_STATE_REQUESTS)
	{
		return CP_GSL_QUEUE_FULL;
	}
	CP_GSL_Request* request = &stack->queue[stack->queued++];
	request->kind = kind;
	request->state = state;
	request->from = -1;
	if (kind == CP_GSL_POP)
	{
		request->state.init = request->state.update = request->state.exit = NULL;
		request->state.passThrough = CP_STATE_FROZEN;
	}
	return CP_GSL_DONE;
}

// A pop asked for by the state at index `from` (-1: the top): that state and
// every state above it leave
static inline CP_GSL_Result CP_GSL_AskPop(CP_GSL_Stack* stack, int from)
{
	const CP_GSL_State none = { NULL, NULL, NULL, CP_STATE_FROZEN };
	const CP_GSL_Result result = CP_GSL_Ask(stack, CP_GSL_POP, none);
	if (result == CP_GSL_DONE)
	{
		stack->queue[stack->queued - 1].from = from;
	}
	return result;
}

// Moves the waiting requests into `out` (room for CP_MAX_GAME_STATE_REQUESTS)
// and empties the queue, so requests made while these are applied (by an
// init or an exit) wait for the next frame. Returns how many there were.
static inline int CP_GSL_TakeRequests(CP_GSL_Stack* stack, CP_GSL_Request* out)
{
	const int count = stack->queued;
	for (int i = 0; i < count; ++i)
	{
		out[i] = stack->queue[i];
	}
	stack->queued = 0;
	return count;
}

//------------------------------------------------------------------------------
// Applying one request

static inline void CP_GSL_LeaveTop(CP_GSL_Stack* stack, const CP_GSL_Hooks* hooks, CP_GSL_RequestKind why)
{
	const int top = stack->depth - 1;
	if (hooks && hooks->leave)
	{
		hooks->leave(hooks->user, top, why);
	}
	stack->depth--;
}

// Ends every state, from the top down. Each state below the top is the top
// again (uncovered) just before its own exit, so it exits as itself.
static inline void CP_GSL_LeaveAll(CP_GSL_Stack* stack, const CP_GSL_Hooks* hooks, CP_GSL_RequestKind why)
{
	while (stack->depth > 0)
	{
		CP_GSL_LeaveTop(stack, hooks, why);
		if (stack->depth > 0 && hooks && hooks->uncover)
		{
			hooks->uncover(hooks->user, stack->depth - 1);
		}
	}
}

static inline CP_GSL_Result CP_GSL_Apply(CP_GSL_Stack* stack, const CP_GSL_Request* request, const CP_GSL_Hooks* hooks)
{
	switch (request->kind)
	{
	case CP_GSL_PUSH:
		if (stack->depth >= CP_MAX_GAME_STATES)
		{
			return CP_GSL_STACK_FULL;
		}
		stack->states[stack->depth++] = request->state;
		if (hooks && hooks->enter)
		{
			hooks->enter(hooks->user, stack->depth - 1, CP_GSL_PUSH);
		}
		return CP_GSL_DONE;

	case CP_GSL_POP:
	{
		const int from = request->from < 0 ? stack->depth - 1 : request->from;
		if (from >= stack->depth)
		{
			return CP_GSL_GONE;
		}
		if (from <= 0)
		{
			return CP_GSL_POP_LAST;
		}
		// From the top down to the one that asked. Each state below the top
		// is the top again (uncovered) just before its own exit, and the one
		// left on top at the end carries on.
		while (stack->depth > from)
		{
			CP_GSL_LeaveTop(stack, hooks, CP_GSL_POP);
			if (hooks && hooks->uncover)
			{
				hooks->uncover(hooks->user, stack->depth - 1);
			}
		}
		return CP_GSL_DONE;
	}

	case CP_GSL_SET:
	case CP_GSL_SET_FORCED:
	default:
		// a set (not forced) to the state that is already the only one does nothing
		if (request->kind == CP_GSL_SET && stack->depth == 1 && CP_GSL_SameState(&stack->states[0], &request->state))
		{
			return CP_GSL_SAME_STATE;
		}
		CP_GSL_LeaveAll(stack, hooks, request->kind);
		stack->states[0] = request->state;
		stack->states[0].passThrough = CP_STATE_FROZEN;
		stack->depth = 1;
		if (hooks && hooks->enter)
		{
			hooks->enter(hooks->user, 0, request->kind);
		}
		return CP_GSL_DONE;
	}
}

//------------------------------------------------------------------------------
// Which states run and which are seen
//
// Each state's pass-through says what the states under it can do: be
// updated (CP_STATE_UPDATING) and be seen (CP_STATE_VISIBLE). A state under
// several can do only what all of them let through. The top state does
// everything, and is the only one with input.

// Is the state at index updated this frame? The top always is; one below it
// only if every state above it lets CP_STATE_UPDATING through.
static inline bool CP_GSL_IsRunning(const CP_GSL_Stack* stack, int index)
{
	if (index < 0 || index >= stack->depth)
	{
		return false;
	}
	for (int above = index + 1; above < stack->depth; ++above)
	{
		if ((stack->states[above].passThrough & CP_STATE_UPDATING) == 0)
		{
			return false;
		}
	}
	return true;
}

// The lowest state that can be seen: everything from here to the top is
// drawn. A state that doesn't let CP_STATE_VISIBLE through hides everything
// under it.
static inline int CP_GSL_LowestVisible(const CP_GSL_State* states, int depth)
{
	for (int i = depth - 1; i > 0; --i)
	{
		if ((states[i].passThrough & CP_STATE_VISIBLE) == 0)
		{
			return i;
		}
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------
// file:	particles.c
//
// Example: particles: fireworks, a campfire, trails and connecting lines.
//
// A particle is a small thing that lives for a moment: a spark, a puff of
// smoke, a bit of a trail. Each one is simple, it moves, changes color and
// shrinks, and hundreds together make fire, explosions and magic.
//
// Shows how to:
// - keep particles in an array of structs: each struct holds one particle's
//   position, speed, age and looks
// - add and remove them without malloc: a fixed array and a count (a pool).
//   A particle that's done swaps places with the last one, and the count
//   goes down.
// - make a particle change over its life: its age divided by its lifetime
//   goes from 0 to 1, for CP_Color_Lerp and CP_Math_LerpFloat
// - spawn a number every second, the same at any frame rate
// - burst fireworks at the top of their climb, and on a click
// - make a campfire from flames, smoke and sparks
// - draw a trail into an image that fades (CP_Image_BeginDrawing), and
//   compare it with a trail made of particles
// - join dots that come close with lines (the closer, the brighter)
// - make particles glow where they overlap with CP_BLEND_ADD
//
// Keys: 1 to 4 choose a scene. G switches the glow (adding light) on and
// off. Each scene shows what the mouse does in it.
//
// Try this:
// - Make the fireworks burst into a ring: the same speed for every spark.
// - Make the campfire burn lower when you press Down, and go out.
// - Make the sparks bounce once off the bottom of the window.
// - Give each particle a spin, and draw it as a small square.
//
// See also: random.c, for the random directions and colors; easing.c, to
// change a particle's size along a curve; state_stack.c, for a game with
// states to put these in.
//
// To run it, copy this file over the template's main.c.
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

//---------------------------------------------------------
// The particles: an array of structs, and a count

typedef struct Particle
{
	CP_Vector position;
	CP_Vector velocity; // pixels per second
	float age;          // seconds since it was added
	float lifetime;     // seconds it lasts
	float startSize;    // its size when it starts, and when it ends
	float endSize;
	CP_Color startColor; // its color when it starts, and when it ends
	CP_Color endColor;
	float gravity; // pixels per second, per second, down; less than 0 rises
	float drag;    // how much of its speed it loses in a second, 0 to 1
} Particle;

#define MAX_PARTICLES 4000
Particle particles[MAX_PARTICLES];
int particleCount = 0; // particles[0] to particles[particleCount - 1] are alive

CP_BOOL glow = TRUE;

// Adds a particle and returns it, for you to set more of it, or NULL if the
// array is full (then skip it: nobody misses one spark). It starts with a
// size of 6 that shrinks to 0, white that fades away, and no gravity or drag.
Particle* AddParticle(CP_Vector position, CP_Vector velocity, float lifetime)
{
	if (particleCount >= MAX_PARTICLES)
	{
		return NULL;
	}
	Particle* particle = &particles[particleCount];
	particleCount++;

	particle->position = position;
	particle->velocity = velocity;
	particle->age = 0.0f;
	particle->lifetime = lifetime;
	particle->startSize = 6.0f;
	particle->endSize = 0.0f;
	particle->startColor = CP_Color_Create(255, 255, 255, 255);
	particle->endColor = CP_Color_Create(255, 255, 255, 0);
	particle->gravity = 0.0f;
	particle->drag = 0.0f;
	return particle;
}

void UpdateParticles(float dt)
{
	int i = 0;
	while (i < particleCount)
	{
		Particle* particle = &particles[i];
		particle->age += dt;
		if (particle->age >= particle->lifetime)
		{
			// Done: the last particle takes its place, and the count goes
			// down. Don't move on (no ++i): particles[i] is a new one now.
			particles[i] = particles[particleCount - 1];
			particleCount--;
			continue;
		}

		particle->velocity.y += particle->gravity * dt;
		float keep = 1.0f - particle->drag * dt;
		if (keep < 0.0f)
		{
			keep = 0.0f;
		}
		particle->velocity = CP_Vector_Scale(particle->velocity, keep);
		particle->position = CP_Vector_Add(particle->position, CP_Vector_Scale(particle->velocity, dt));
		++i;
	}
}

void DrawParticles(void)
{
	CP_Settings_NoStroke();
	for (int i = 0; i < particleCount; ++i)
	{
		const Particle* particle = &particles[i];
		const float life = particle->age / particle->lifetime; // 0 when it starts, 1 when it ends
		const float size = CP_Math_LerpFloat(particle->startSize, particle->endSize, life);
		CP_Color color = CP_Color_Lerp(particle->startColor, particle->endColor, life);
		if (glow)
		{
			// A faint, bigger circle around it: with CP_BLEND_ADD these add
			// up to a glow where particles crowd together
			CP_Color halo = color;
			halo.a = (unsigned char)(color.a / 6);
			CP_Settings_Fill(halo);
			CP_Graphics_DrawCircle(particle->position.x, particle->position.y, size * 3.0f);
		}
		CP_Settings_Fill(color);
		CP_Graphics_DrawCircle(particle->position.x, particle->position.y, size);
	}
}

// How many to add this frame, for perSecond a second at any frame rate.
// At 60 frames a second and 30 a second, that's one every other frame:
// timer adds up the parts until they make a whole one.
int SpawnCount(float* timer, float perSecond)
{
	*timer += perSecond * CP_System_GetDt();
	const int count = (int)*timer;
	*timer -= count;
	return count;
}

// A direction at random, as long as a speed between the two
CP_Vector RandomVelocity(float minSpeed, float maxSpeed)
{
	return CP_Vector_Scale(CP_Vector_FromAngle(CP_Random_RangeFloat(0.0f, 360.0f)), CP_Random_RangeFloat(minSpeed, maxSpeed));
}

CP_Color RandomBrightColor(void)
{
	return CP_Color_FromColorHSL(CP_ColorHSL_Create(CP_Random_RangeInt(0, 359), 90, 62, 255));
}

CP_Color Transparent(CP_Color color)
{
	color.a = 0;
	return color;
}

//---------------------------------------------------------
// 1. Fireworks

#define ROCKET_COUNT 8
typedef struct Rocket
{
	CP_Vector position;
	CP_Vector velocity;
	CP_Color color;
	float sparkTimer; // for SpawnCount
	CP_BOOL flying;
} Rocket;

Rocket rockets[ROCKET_COUNT];
float launchTimer = 0.0f;

// Sparks in every direction. The speeds are random too, so some stay near
// the middle and the burst is a ball, not a ring.
void Burst(CP_Vector position, CP_Color color, int count)
{
	for (int i = 0; i < count; ++i)
	{
		Particle* spark = AddParticle(position, RandomVelocity(30.0f, 330.0f), CP_Random_RangeFloat(1.0f, 1.8f));
		if (spark == NULL)
		{
			return;
		}
		spark->startSize = 5.0f;
		spark->endSize = 1.0f;
		spark->startColor = color;
		spark->endColor = Transparent(color);
		spark->gravity = 110.0f;
		spark->drag = 1.2f;
	}
}

void LaunchRocket(void)
{
	for (int i = 0; i < ROCKET_COUNT; ++i)
	{
		Rocket* rocket = &rockets[i];
		if (!rocket->flying)
		{
			rocket->position = CP_Vector_Set(CP_Random_RangeFloat(200.0f, WINDOW_WIDTH - 200.0f), (float)WINDOW_HEIGHT);
			rocket->velocity = CP_Vector_Set(CP_Random_RangeFloat(-60.0f, 60.0f), CP_Random_RangeFloat(-640.0f, -520.0f));
			rocket->color = RandomBrightColor();
			rocket->sparkTimer = 0.0f;
			rocket->flying = TRUE;
			return;
		}
	}
}

void UpdateFireworks(void)
{
	const float dt = CP_System_GetDt();
	for (int i = SpawnCount(&launchTimer, 0.8f); i > 0; --i)
	{
		LaunchRocket();
	}
	if (CP_Input_MouseTriggered(CP_MOUSE_LEFT))
	{
		Burst(CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY()), RandomBrightColor(), 150);
	}

	for (int i = 0; i < ROCKET_COUNT; ++i)
	{
		Rocket* rocket = &rockets[i];
		if (!rocket->flying)
		{
			continue;
		}
		rocket->velocity.y += 300.0f * dt;
		rocket->position = CP_Vector_Add(rocket->position, CP_Vector_Scale(rocket->velocity, dt));

		// A trail of small sparks that fall behind it
		for (int s = SpawnCount(&rocket->sparkTimer, 60.0f); s > 0; --s)
		{
			Particle* spark = AddParticle(rocket->position, RandomVelocity(0.0f, 30.0f), 0.5f);
			if (spark != NULL)
			{
				spark->startSize = 3.0f;
				spark->startColor = CP_Color_Create(255, 220, 160, 255);
				spark->endColor = CP_Color_Create(255, 120, 40, 0);
				spark->gravity = 80.0f;
			}
		}

		// At the top of its climb, it stops going up: burst
		if (rocket->velocity.y >= 0.0f)
		{
			Burst(rocket->position, rocket->color, 120);
			rocket->flying = FALSE;
		}
	}
}

void DrawFireworks(void)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 240, 210, 255));
	for (int i = 0; i < ROCKET_COUNT; ++i)
	{
		if (rockets[i].flying)
		{
			CP_Graphics_DrawCircle(rockets[i].position.x, rockets[i].position.y, 5.0f);
		}
	}
}

//---------------------------------------------------------
// 2. A campfire

CP_Vector fire;
float flameTimer = 0.0f;
float smokeTimer = 0.0f;
float fireSparkTimer = 0.0f;

void UpdateCampfire(void)
{
	// Drag it: the smoke that's already out stays where it was let go, since
	// each particle keeps its own position
	if (CP_Input_MouseDown(CP_MOUSE_LEFT))
	{
		fire = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
	}

	// Smoke: big, slow and faint
	for (int i = SpawnCount(&smokeTimer, 14.0f); i > 0; --i)
	{
		const CP_Vector start = CP_Vector_Add(fire, CP_Vector_Set(CP_Random_RangeFloat(-15.0f, 15.0f), -70.0f));
		Particle* smoke = AddParticle(start, CP_Vector_Set(CP_Random_RangeFloat(-15.0f, 15.0f), -45.0f), CP_Random_RangeFloat(2.5f, 3.5f));
		if (smoke != NULL)
		{
			smoke->startSize = 20.0f;
			smoke->endSize = 80.0f;
			smoke->startColor = CP_Color_Create(60, 60, 70, 60);
			smoke->endColor = CP_Color_Create(40, 40, 50, 0);
			smoke->gravity = -8.0f;
		}
	}

	// Flames: big, yellow and fast at the bottom, smaller, red and fading
	// as they rise
	for (int i = SpawnCount(&flameTimer, 140.0f); i > 0; --i)
	{
		const CP_Vector start = CP_Vector_Add(fire, CP_Vector_Set(CP_Random_RangeFloat(-26.0f, 26.0f), 0.0f));
		Particle* flame = AddParticle(start, CP_Vector_Set(CP_Random_RangeFloat(-20.0f, 20.0f), CP_Random_RangeFloat(-190.0f, -90.0f)), CP_Random_RangeFloat(0.6f, 1.2f));
		if (flame != NULL)
		{
			flame->startSize = 28.0f;
			flame->endSize = 4.0f;
			flame->startColor = CP_Color_Create(255, 190, 70, 110);
			flame->endColor = CP_Color_Create(210, 40, 20, 0);
			flame->gravity = -70.0f; // rises faster as it goes, like hot air
			flame->drag = 0.6f;
		}
	}

	// A few sparks that fly up, then fall
	for (int i = SpawnCount(&fireSparkTimer, 8.0f); i > 0; --i)
	{
		Particle* spark = AddParticle(fire, CP_Vector_Set(CP_Random_RangeFloat(-70.0f, 70.0f), CP_Random_RangeFloat(-320.0f, -160.0f)), CP_Random_RangeFloat(1.0f, 1.6f));
		if (spark != NULL)
		{
			spark->startSize = 4.0f;
			spark->endSize = 1.0f;
			spark->startColor = CP_Color_Create(255, 230, 150, 255);
			spark->endColor = CP_Color_Create(255, 110, 40, 0);
			spark->gravity = 140.0f;
		}
	}
}

void DrawCampfireLogs(void)
{
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(90, 55, 35, 255));
	CP_Graphics_DrawEllipse(fire.x, fire.y + 12.0f, 120.0f, 26.0f);
}

//---------------------------------------------------------
// 3. Trails: drawn into an image, or made of particles

#define ORBIT_COUNT 3
CP_Image trail;     // the window's size: drawn into, every frame
int fadeChoice = 1; // Up and Down choose how fast the trail fades
const int fadeAmounts[3] = { 2, 4, 12 };
float cometTimer = 0.0f;
CP_Color orbitColors[ORBIT_COUNT];

void ClearTrail(void)
{
	CP_Image_BeginDrawing(trail);
	CP_Graphics_ClearBackground(CP_Color_Create(0, 0, 0, 0));
	CP_Image_EndDrawing();
}

CP_Vector OrbitPosition(int i)
{
	const float seconds = CP_System_GetSeconds();
	const float radius = 120.0f + 90.0f * i;
	const float angle = seconds * (90.0f - 25.0f * i) + i * 120.0f;
	return CP_Vector_Add(CP_Vector_Set(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f), CP_Vector_Scale(CP_Vector_FromAngle(angle), radius));
}

void UpdateTrails(void)
{
	if (CP_Input_KeyTriggered(CP_KEY_UP) && fadeChoice < 2)
	{
		fadeChoice++;
	}
	if (CP_Input_KeyTriggered(CP_KEY_DOWN) && fadeChoice > 0)
	{
		fadeChoice--;
	}

	// The image trail: each frame, the whole image fades a little, and gets
	// a dot where each ball is now. Everything in it fades together, and it
	// costs the same however long the trail is.
	//
	// The fade takes a nearly clear white rectangle away (CP_BLEND_SUBTRACT):
	// its alpha of 6 takes 6 off the red, green, blue and alpha of every
	// pixel, so the trail gets darker and clearer together, all the way to
	// nothing. (A black one would take away only alpha: the colors would
	// stay bright until they vanished.)
	CP_Image_BeginDrawing(trail);
	CP_Settings_NoStroke();
	CP_Settings_RectMode(CP_POSITION_CORNER);
	CP_Settings_BlendMode(CP_BLEND_SUBTRACT);
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, fadeAmounts[fadeChoice]));
	CP_Graphics_DrawRect(0.0f, 0.0f, (float)CP_Image_GetWidth(trail), (float)CP_Image_GetHeight(trail));
	CP_Settings_BlendMode(CP_BLEND_ALPHA);
	for (int i = 0; i < ORBIT_COUNT; ++i)
	{
		const CP_Vector position = OrbitPosition(i);
		CP_Settings_Fill(orbitColors[i]);
		CP_Graphics_DrawCircle(position.x, position.y, 16.0f);
	}
	CP_Image_EndDrawing();

	// The particle trail: the comet at the mouse leaves particles, each with
	// a little speed of its own, so the trail spreads and shimmers
	const CP_Vector mouse = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
	for (int i = SpawnCount(&cometTimer, 120.0f); i > 0; --i)
	{
		Particle* bit = AddParticle(mouse, RandomVelocity(5.0f, 40.0f), CP_Random_RangeFloat(0.4f, 0.8f));
		if (bit != NULL)
		{
			bit->startSize = 14.0f;
			bit->endSize = 0.0f;
			bit->startColor = CP_Color_Create(150, 240, 255, 220);
			bit->endColor = CP_Color_Create(60, 90, 255, 0);
		}
	}
}

void DrawTrails(void)
{
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_Image_Draw(trail, 0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 255);
	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
	for (int i = 0; i < ORBIT_COUNT; ++i)
	{
		const CP_Vector position = OrbitPosition(i);
		CP_Graphics_DrawCircle(position.x, position.y, 16.0f);
	}
}

//---------------------------------------------------------
// 4. Connecting lines

#define DOT_COUNT 120
typedef struct Dot
{
	CP_Vector position;
	CP_Vector velocity;
	CP_Color color; // half bright: two together, added, make a full color
} Dot;

Dot dots[DOT_COUNT];
const float linkDistance = 110.0f;  // dots closer than this get a line
const float mouseDistance = 200.0f; // and so does the mouse

void ScatterDots(void)
{
	const CP_Color colors[6] = {
		{ 127, 0,   0,   255 },
		{ 127, 127, 0,   255 },
		{ 0,   127, 0,   255 },
		{ 0,   127, 127, 255 },
		{ 0,   0,   127, 255 },
		{ 127, 0,   127, 255 }
	};
	for (int i = 0; i < DOT_COUNT; ++i)
	{
		dots[i].position = CP_Vector_Set(CP_Random_RangeFloat(0.0f, (float)WINDOW_WIDTH), CP_Random_RangeFloat(0.0f, (float)WINDOW_HEIGHT));
		dots[i].velocity = RandomVelocity(30.0f, 110.0f);
		dots[i].color = colors[CP_Random_RangeInt(0, 5)];
	}
}

void UpdateDots(void)
{
	const float dt = CP_System_GetDt();
	for (int i = 0; i < DOT_COUNT; ++i)
	{
		Dot* dot = &dots[i];
		dot->position = CP_Vector_Add(dot->position, CP_Vector_Scale(dot->velocity, dt));
		// Off an edge: turn back toward the window
		if ((dot->position.x < 0.0f && dot->velocity.x < 0.0f) || (dot->position.x > WINDOW_WIDTH && dot->velocity.x > 0.0f))
		{
			dot->velocity.x = -dot->velocity.x;
		}
		if ((dot->position.y < 0.0f && dot->velocity.y < 0.0f) || (dot->position.y > WINDOW_HEIGHT && dot->velocity.y > 0.0f))
		{
			dot->velocity.y = -dot->velocity.y;
		}
	}
}

// A line that fades out as the two ends get to the distance
void DrawLink(CP_Vector a, CP_Vector b, CP_Color color, float distance)
{
	const float apart = CP_Vector_Distance(a, b);
	if (apart < distance)
	{
		color.a = (unsigned char)(255.0f * (1.0f - apart / distance));
		CP_Settings_Stroke(color);
		CP_Graphics_DrawLine(a.x, a.y, b.x, b.y);
	}
}

void DrawDots(void)
{
	const CP_Vector mouse = CP_Vector_Set(CP_Input_GetMouseX(), CP_Input_GetMouseY());
	CP_Settings_StrokeWeight(3.0f);
	CP_Settings_LineCapMode(CP_LINE_CAP_ROUND);
	for (int i = 0; i < DOT_COUNT; ++i)
	{
		DrawLink(dots[i].position, mouse, CP_Color_Create(127, 127, 127, 255), mouseDistance);

		// Each pair once: j starts after i
		for (int j = i + 1; j < DOT_COUNT; ++j)
		{
			const CP_Color mixed = CP_Color_Create(dots[i].color.r + dots[j].color.r, dots[i].color.g + dots[j].color.g,
				dots[i].color.b + dots[j].color.b, 255);
			DrawLink(dots[i].position, dots[j].position, mixed, linkDistance);
		}
	}
	CP_Settings_LineCapMode(CP_LINE_CAP_BUTT);

	CP_Settings_NoStroke();
	for (int i = 0; i < DOT_COUNT; ++i)
	{
		CP_Settings_Fill(CP_Color_Create(dots[i].color.r * 2, dots[i].color.g * 2, dots[i].color.b * 2, 255));
		CP_Graphics_DrawCircle(dots[i].position.x, dots[i].position.y, 5.0f);
	}
}

//---------------------------------------------------------

int scene = 1;
const char* sceneHelp[5] = {
	"",
	"1. Fireworks: click for a burst",
	"2. A campfire: drag it with the mouse",
	"3. Trails: the balls draw into an image (Up/Down: how fast it fades); the mouse leaves particles",
	"4. Connecting lines: dots that come close, and the mouse"
};

void ChooseScene(int newScene)
{
	scene = newScene;
	particleCount = 0; // every particle goes at once
	for (int i = 0; i < ROCKET_COUNT; ++i)
	{
		rockets[i].flying = FALSE;
	}
	fire = CP_Vector_Set(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 120.0f);
	ClearTrail();
	ScatterDots();
}

void game_init(void)
{
	CP_System_SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	CP_System_SetWindowTitle("Particles");
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_LEFT, CP_TEXT_ALIGN_V_BASELINE);

	trail = CP_Image_Create(WINDOW_WIDTH, WINDOW_HEIGHT);
	orbitColors[0] = CP_Color_Create(255, 120, 90, 255);
	orbitColors[1] = CP_Color_Create(120, 230, 120, 255);
	orbitColors[2] = CP_Color_Create(200, 130, 255, 255);
	ChooseScene(1);
}

void game_update(void)
{
	for (int i = 1; i <= 4; ++i)
	{
		if (CP_Input_KeyTriggered((CP_KEY)(CP_KEY_0 + i)))
		{
			ChooseScene(i);
		}
	}
	if (CP_Input_KeyTriggered(CP_KEY_G))
	{
		glow = !glow;
	}

	switch (scene)
	{
	case 1: UpdateFireworks(); break;
	case 2: UpdateCampfire(); break;
	case 3: UpdateTrails(); break;
	case 4: UpdateDots(); break;
	}
	UpdateParticles(CP_System_GetDt());

	CP_Graphics_ClearBackground(CP_Color_Create(12, 12, 24, 255));
	if (scene == 2)
	{
		DrawCampfireLogs();
	}
	if (scene == 3)
	{
		DrawTrails();
	}

	// With CP_BLEND_ADD, each color adds its light to what's under it, so
	// where particles overlap, it gets brighter, toward white
	CP_Settings_BlendMode(glow ? CP_BLEND_ADD : CP_BLEND_ALPHA);
	if (scene == 1)
	{
		DrawFireworks();
	}
	if (scene == 4)
	{
		DrawDots();
	}
	DrawParticles();
	CP_Settings_BlendMode(CP_BLEND_ALPHA);

	char text[80];
	CP_Settings_Fill(CP_Color_Create(220, 220, 235, 255));
	CP_Settings_TextSize(22.0f);
	CP_Font_DrawText(sceneHelp[scene], 16.0f, 32.0f);
	CP_Settings_TextSize(18.0f);
	snprintf(text, sizeof(text), "Particles: %d of %d", particleCount, MAX_PARTICLES);
	CP_Font_DrawText(text, 16.0f, 58.0f);
	snprintf(text, sizeof(text), "1-4: scenes    G: glow (%s)", glow ? "on" : "off");
	CP_Font_DrawText(text, 16.0f, WINDOW_HEIGHT - 16.0f);
}

void game_exit(void)
{
	CP_Image_Free(&trail);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}

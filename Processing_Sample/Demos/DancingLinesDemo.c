//---------------------------------------------------------
// file:	DancingLinesDemo.c
// 
// CProcessing features in this demo:
// CP_Settings_BlendMode	- switch between AlPHA and ADD for effects
// CP_Settings_LineCapMode	- line end options, draw lines with rounded ends
// Robust point/wall collision checks and resolutions
//---------------------------------------------------------

#include "cprocessing.h"
#include "DancingLinesDemo.h"
#include "DemoManager.h"
#include <math.h>
#include <stdio.h>

typedef struct _Particle
{
	CP_Vector pos;
	CP_Vector vel;
	CP_Color* color;
} Particle;

const float EPSILON = 0.0000001f;

float particleSize = 3.0f;

CP_Color randomColors[] = {
	{ 127, 0,   0,   255 },
	{ 127, 127, 0,   255 },
	{ 0,   127, 0,   255 },
	{ 0,   127, 127, 255 },
	{ 0,   0,   127, 255 },
	{ 127, 0,   127, 255 } };

void ParticleCreate(Particle* part) {
	part->pos.x = CP_Random_RangeFloat(0, (float)CP_System_GetWindowWidth());
	part->pos.y = CP_Random_RangeFloat(0, (float)CP_System_GetWindowHeight());
	part->vel.x = CP_Random_RangeFloat(-150, 150);
	part->vel.y = CP_Random_RangeFloat(-150, 150);
	part->color = &randomColors[CP_Random_RangeInt(0, 5)];
}

void ParticleDisplay(Particle* part)
{
	CP_Graphics_DrawCircle(part->pos.x, part->pos.y, particleSize);
}

void ParticleUpdate(Particle* part)
{
	// move particle based on velocity and correct for wall collisions
	float time = CP_System_GetDt();
	float timeX = time;
	float timeY = time;

	while (time > EPSILON)
	{
		CP_BOOL collisionX = FALSE;
		CP_BOOL collisionY = FALSE;

		float newPosX = part->pos.x + part->vel.x * time;
		float newPosY = part->pos.y + part->vel.y * time;
		float newTime = time;

		// check wall collisions X and Y
		if (newPosX <= 0)
		{
			timeX = part->pos.x / (part->pos.x - newPosX) * time;
			collisionX = TRUE;
		}
		else if (newPosX >= CP_System_GetWindowWidth())
		{
			timeX = (CP_System_GetWindowWidth() - part->pos.x) / (newPosX - part->pos.x) * time;
			collisionX = TRUE;
		}

		if (newPosY <= 0)
		{
			timeY = part->pos.y / (part->pos.y - newPosY) * time;
			collisionY = TRUE;
		}
		else if (newPosY >= CP_System_GetWindowHeight())
		{
			timeY = (CP_System_GetWindowHeight() - part->pos.y) / (newPosY - part->pos.y) * time;
			collisionY = TRUE;
		}

		// resolve collisions
		if ((collisionX == TRUE) || (collisionY == TRUE))
		{
			// take the nearest time
			if (timeX < timeY)
			{
				newTime = timeX;
			}
			else
			{
				newTime = timeY;
			}

			// move the particle
			part->pos.x += part->vel.x * newTime;
			part->pos.y += part->vel.y * newTime;

			// flip velocity vectors to reflect off walls
			if ((collisionX == TRUE) && (collisionY == FALSE))
			{
				part->vel.x *= -1;
			}
			else if ((collisionX == FALSE) && (collisionY == TRUE))
			{
				part->vel.y *= -1;
			}
			else
			{	// they must both be colliding for this condition to occur
				if (timeX < timeY)
				{
					part->vel.x *= -1;
				}
				else if (timeX > timeY)
				{
					part->vel.y *= -1;
				}
				else
				{	// they must be colliding at the same time (ie. a corner)
					part->vel.x *= -1;
					part->vel.y *= -1;
				}
			}

			// decrease time and iterate
			time -= newTime;
		}
		else
		{
			// no collision
			part->pos.x = newPosX;
			part->pos.y = newPosY;
			time = 0;
		}
	}
}

Particle particles[160];
int numParticles = 160;

float lineProximityDistance = 100.0f;
float mouseProximityDistance = 200.0f;

void DancingLinesDemoInit(void)
{
	CP_Settings_Save();
	CP_Settings_LineCapMode(CP_LINE_CAP_ROUND);

	for (int i = 0; i < numParticles; ++i) {
		ParticleCreate(&particles[i]);
	}
}

void DancingLinesDemoUpdate(void)
{
	CP_Settings_BlendMode(CP_BLEND_ALPHA);
	CP_Graphics_ClearBackground(GetCommonBackgoundColor());

	CP_Settings_NoStroke();
	CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));

	for (int i = 0; i < numParticles; ++i)
	{
		ParticleUpdate(&particles[i]);
		ParticleDisplay(&particles[i]);
	}

	CP_Settings_BlendMode(CP_BLEND_ADD);
	CP_Settings_StrokeWeight(4);
	CP_Color lineColor;

	for (int i = 0; i < numParticles; ++i)
	{
		// draw white lines from the particles to the mouse position
		float distXMouse = (float)fabsf(particles[i].pos.x - (float)CP_Input_GetMouseX());
		float distYMouse = (float)fabsf(particles[i].pos.y - (float)CP_Input_GetMouseY());
		if (distXMouse < mouseProximityDistance && distYMouse < mouseProximityDistance)
		{
			lineColor.r = 127;
			lineColor.g = 127;
			lineColor.b = 127;
			lineColor.a = (unsigned char)(255.0f * min(1.0f, (mouseProximityDistance - max(distXMouse, distYMouse)) / (mouseProximityDistance * 0.3f)));
			CP_Settings_Stroke(lineColor);
			CP_Graphics_DrawLine(particles[i].pos.x, particles[i].pos.y, CP_Input_GetMouseX(), CP_Input_GetMouseY());
		}

		// draw lines between particles based on the additive color of both particles
		for (int j = i + 1; j < numParticles; ++j)
		{
			float distX = (float)fabsf(particles[i].pos.x - particles[j].pos.x);
			float distY = (float)fabsf(particles[i].pos.y - particles[j].pos.y);
			if (distX < lineProximityDistance && distY < lineProximityDistance)
			{
				lineColor.r = particles[i].color->r + particles[j].color->r;
				lineColor.g = particles[i].color->g + particles[j].color->g;
				lineColor.b = particles[i].color->b + particles[j].color->b;
				lineColor.a = (unsigned char)(255.0f * min(1.0f, (lineProximityDistance - max(distX, distY)) / (lineProximityDistance * 0.3f)));
				CP_Settings_Stroke(lineColor);
				CP_Graphics_DrawLine(particles[i].pos.x, particles[i].pos.y, particles[j].pos.x, particles[j].pos.y);
			}
		}
	}
}

void DancingLinesDemoExit(void)
{
	CP_Settings_Restore();
}

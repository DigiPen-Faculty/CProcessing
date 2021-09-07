//---------------------------------------------------------
// file:	main.c
// author:	Justin Chambers
// brief:	Implementation for dancing lines test gamestate
//
// Copyright © 2019 DigiPen, All rights reserved.
//---------------------------------------------------------

#include "cprocessing.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "DancingLines.h"

//-------------------------------------------
// DANCING LINES
// Including Particle "class"
//
typedef struct _Particle
{
  Vec2 pos;
  Vec2 vel;
  PColor *color;
} Particle;

const float EPSILON = 0.0000001f;

float particleSize = 3.0f;

PColor randomColors[] = {
  { 128, 0,   0,   255 },
  { 128, 128, 0,   255 },
  { 0,   128, 0,   255 },
  { 0,   128, 128, 255 },
  { 0,   0,   128, 255 },
  { 128, 0,   128, 255 } };

void ParticleCreate(Particle *part) {
  part->pos.x = randomRangeFloat(0, (float)canvasWidth);
  part->pos.y = randomRangeFloat(0, (float)canvasHeight);
  part->vel.x = randomRangeFloat(-150, 150);
  part->vel.y = randomRangeFloat(-150, 150);
  part->color = &randomColors[randomRangeInt(0, 5)];
}

void ParticleDisplay(Particle *part)
{
  ellipse(part->pos.x, part->pos.y, particleSize, particleSize);
}

void ParticleUpdate(Particle *part)
{
  // move particle based on velocity and correct for wall collisions
  float time = dt();
  float timeX = time;
  float timeY = time;

  while (time > EPSILON)
  {
    bool collisionX = false;
    bool collisionY = false;

    float newPosX = part->pos.x + part->vel.x * time;
    float newPosY = part->pos.y + part->vel.y * time;
    float newTime = time;

    // check wall collisions X and Y
    if (newPosX <= 0)
    {
      timeX = part->pos.x / (part->pos.x - newPosX) * time;
      collisionX = true;
    }
    else if (newPosX >= canvasWidth)
    {
      timeX = (canvasWidth - part->pos.x) / (newPosX - part->pos.x) * time;
      collisionX = true;
    }

    if (newPosY <= 0)
    {
      timeY = part->pos.y / (part->pos.y - newPosY) * time;
      collisionY = true;
    }
    else if (newPosY >= canvasHeight)
    {
      timeY = (canvasHeight - part->pos.y) / (newPosY - part->pos.y) * time;
      collisionY = true;
    }

    // resolve collisions
    if ((collisionX == true) || (collisionY == true))
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
      if ((collisionX == true) && (collisionY == false))
      {
        part->vel.x *= -1;
      }
      else if ((collisionX == false) && (collisionY == true))
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

Particle particles[240];
int numParticles = 240;

float lineProximityDistance = 100.0f;
float mouseProximityDistance = 200.0f;

int recommendedWidth = 1600;
int recommendedHeight = 900;

bool drawColors = true;
bool drawFPS = true;

void DancingLinesInit()
{
  size(recommendedWidth, recommendedHeight);
  //setFrameRate(60.0f);

  for (int i = 0; i < numParticles; ++i) {
    ParticleCreate(&particles[i]);
  }
}

void DancingLinesUpdate()
{
  blendMode(CP_BLEND_ALPHA);
  background(color(51, 51, 51, 255));

  //background(color((int)(mouseX / canvasWidth * 255.0f), 0, 0, 255));
  //CP_Graphics_Background(CP_CreateColor((int)(CP_Input_GetMouseX() / CP_Graphics_GetCanvasWidth() * 255.0f), 0, 0, 255));

  noStroke();
  fill(color(0, 0, 0, 255));

  for (int i = 0; i < numParticles; ++i)
  {
    ParticleUpdate(&particles[i]);
    ParticleDisplay(&particles[i]);
  }

  blendMode(CP_BLEND_ADD);
  strokeWeight(3);
  PColor lineColor;

  for (int i = 0; i < numParticles; ++i)
  {
    // draw white lines from the particles to the mouse position
    float distXMouse = (float)fabsf(particles[i].pos.x - (float)CP_Input_GetMouseX());
    float distYMouse = (float)fabsf(particles[i].pos.y - (float)getmouseY());
    if (distXMouse < mouseProximityDistance && distYMouse < mouseProximityDistance)
    {
      lineColor.r = 128;
      lineColor.g = 128;
      lineColor.b = 128;
      lineColor.a = (int)(255.0f * min(1.0f, (mouseProximityDistance - max(distXMouse, distYMouse)) / (mouseProximityDistance * 0.3f)));
      stroke(lineColor);
      line(particles[i].pos.x, particles[i].pos.y, mouseX, mouseY);
    }

    // draw lines between particles based on the additive color of both particles
    if (!drawColors)
    {
      continue;
    }
    for (int j = i + 1; j < numParticles; ++j)
    {
      float distX = (float)fabsf(particles[i].pos.x - particles[j].pos.x);
      float distY = (float)fabsf(particles[i].pos.y - particles[j].pos.y);
      if (distX < lineProximityDistance && distY < lineProximityDistance)
      {
        lineColor.r = particles[i].color->r + particles[j].color->r;
        lineColor.g = particles[i].color->g + particles[j].color->g;
        lineColor.b = particles[i].color->b + particles[j].color->b;
        lineColor.a = (int)(255.0f * min(1.0f, (lineProximityDistance - max(distX, distY)) / (lineProximityDistance * 0.3f)));
        stroke(lineColor);
        line(particles[i].pos.x, particles[i].pos.y, particles[j].pos.x, particles[j].pos.y);
      }
    }
  }

  if (keyPressed(KEY_SPACE))
  {
    drawColors = !drawColors;
  }

  // Profiling info and frameRate testing
  if (drawFPS)
  {
    textSize(20);
    blendMode(CP_BLEND_ALPHA);
    fill(color(0, 0, 0, 128));
    noStroke();
    rect(0, 0, 150, 30);
    fill(color(255, 255, 255, 255));
    char buffer[100];
    sprintf_s(buffer, 100, "FPS: %f", frameRate);
    text(buffer, 20, 20);
  }
}
//
// end DANCING LINES
//----------------------------------------------
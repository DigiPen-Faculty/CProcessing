//---------------------------------------------------------
// file:	GridWaveDemo.c
// 
// CProcessing features in this demo:
// CP_Vector				- Lots of math going on here
// CP_Color_Lerp			- Smooth color changes based on line length
//---------------------------------------------------------

#include "cprocessing.h"
#include "GridWaveDemo.h"
#include "DemoManager.h"
#include "stdlib.h"

int grid_stride = 20;
int grid_width = 0;
int grid_height = 0;
int memory_size = 0;
float dampen_constant = 0.96f;
float spring_constant = 0.1f;

CP_Vector* grid_positions = NULL;
CP_Vector* grid_velocities = NULL;
CP_Vector* grid_offsets = NULL;
CP_Vector* grid_offsets_cache = NULL;

CP_Color grid_color_base = { 50, 50, 50, 255 };
CP_Color grid_color_high = { 30, 180, 255, 255 };
CP_Color grid_color_low = { 255, 180, 30, 255 };

void DrawGrid(void)
{
	for (int j = 0; j < grid_height; ++j)
	{
		for (int i = 0; i < grid_width; ++i)
		{
			CP_Vector p1 = CP_Vector_Add(grid_positions[j * grid_width + i], grid_offsets[j * grid_width + i]);
			if (i < grid_width - 1)
			{
				CP_Vector p2 = CP_Vector_Add(grid_positions[j * grid_width + i + 1], grid_offsets[j * grid_width + i + 1]);
				float delta = CP_Vector_Length(CP_Vector_Subtract(p2, p1));
				if (delta >= grid_stride)
				{
					CP_Settings_Stroke(CP_Color_Lerp(grid_color_base, grid_color_high, (delta - grid_stride) / 2.0f));
				}
				else
				{
					CP_Settings_Stroke(CP_Color_Lerp(grid_color_base, grid_color_low, (grid_stride - delta) / 3.0f));
				}
				CP_Settings_StrokeWeight(2.0f);
				CP_Graphics_DrawLine(p1.x, p1.y, p2.x, p2.y);
			}
			if (j < grid_height - 1)
			{
				CP_Vector p2 = CP_Vector_Add(grid_positions[(j + 1) * grid_width + i], grid_offsets[(j + 1) * grid_width + i]);
				float delta = CP_Vector_Length(CP_Vector_Subtract(p2, p1));
				if (delta >= grid_stride)
				{
					CP_Settings_Stroke(CP_Color_Lerp(grid_color_base, grid_color_high, (delta - grid_stride) / 2.0f));
				}
				else
				{
					CP_Settings_Stroke(CP_Color_Lerp(grid_color_base, grid_color_low, (grid_stride - delta) / 3.0f));
				}
				CP_Settings_StrokeWeight(2.0f);
				CP_Graphics_DrawLine(p1.x, p1.y, p2.x, p2.y);
			}
		}
	}
}

void UpdateGrid(void)
{
	memcpy_s(grid_offsets_cache, memory_size, grid_offsets, memory_size);

	for (int j = 1; j < grid_height - 1; ++j)
	{
		for (int i = 1; i < grid_width - 1; ++i)
		{
			int index = j * grid_width + i;
			CP_Vector center_point = grid_offsets_cache[index];
			CP_Vector spring_value = CP_Vector_Zero();
			spring_value = CP_Vector_Add(spring_value, CP_Vector_Subtract(grid_offsets_cache[index - grid_width], center_point));
			spring_value = CP_Vector_Add(spring_value, CP_Vector_Subtract(grid_offsets_cache[index + grid_width], center_point));
			spring_value = CP_Vector_Add(spring_value, CP_Vector_Subtract(grid_offsets_cache[index + 1], center_point));
			spring_value = CP_Vector_Add(spring_value, CP_Vector_Subtract(grid_offsets_cache[index - 1], center_point));
			grid_velocities[index] = CP_Vector_Add(grid_velocities[index], CP_Vector_Scale(spring_value, spring_constant));
			grid_offsets[index] = CP_Vector_Add(grid_offsets[index], grid_velocities[index]);
			grid_velocities[index] = CP_Vector_Scale(grid_velocities[index], dampen_constant);
		}
	}
}

void GridWaveDemoInit(void)
{
	CP_Settings_Save();
	grid_width = CP_System_GetWindowWidth() / grid_stride;
	grid_height = CP_System_GetWindowHeight() / grid_stride;
	memory_size = sizeof(CP_Vector) * grid_width * grid_height;

	// setup grid positions
	grid_positions = malloc(sizeof(CP_Vector) * grid_width * grid_height);
	for (int j = 0; j < grid_height; ++j)
	{
		for (int i = 0; i < grid_width; ++i)
		{
			grid_positions[j * grid_width + i].x = i * grid_stride + (grid_stride / 2.0f);
			grid_positions[j * grid_width + i].y = j * grid_stride + (grid_stride / 2.0f);
		}
	}

	// setup grid velocities
	grid_velocities = calloc(sizeof(CP_Vector), grid_width * grid_height);

	// setup grid offsets
	grid_offsets = calloc(sizeof(CP_Vector), grid_width * grid_height);
	grid_offsets_cache = calloc(sizeof(CP_Vector), grid_width * grid_height);
}

void GridWaveDemoUpdate(void)
{
	CP_Graphics_ClearBackground(GetCommonBackgoundColor());

	// User Input
	if (CP_Input_MouseDown(MOUSE_BUTTON_1))
	{
		int mouse_grid_x = ((int)CP_Input_GetMouseX()) / grid_stride;
		int mouse_grid_y = ((int)CP_Input_GetMouseY()) / grid_stride;

		if (mouse_grid_x < 2)
		{
			mouse_grid_x = 2;
		}
		if (mouse_grid_x > grid_width - 3)
		{
			mouse_grid_x = grid_width - 3;
		}
		if (mouse_grid_y < 2)
		{
			mouse_grid_y = 2;
		}
		if (mouse_grid_y > grid_height - 3)
		{
			mouse_grid_y = grid_height - 3;
		}

		int i = mouse_grid_x;
		int j = mouse_grid_y;

		float grid_stride_f = (float)grid_stride * 0.5f;
		grid_velocities[(j - 1) * grid_width + i].y -= grid_stride_f;
		grid_velocities[(j + 1) * grid_width + i].y += grid_stride_f;
		grid_velocities[j * grid_width + i + 1].x += grid_stride_f;
		grid_velocities[j * grid_width + i - 1].x -= grid_stride_f;

		float grid_stride_corner_f = (float)grid_stride * 0.25f;
		grid_velocities[(j - 1) * grid_width + i - 1].x -= grid_stride_corner_f;
		grid_velocities[(j - 1) * grid_width + i - 1].y -= grid_stride_corner_f;
		grid_velocities[(j + 1) * grid_width + i - 1].x -= grid_stride_corner_f;
		grid_velocities[(j + 1) * grid_width + i - 1].y += grid_stride_corner_f;
		grid_velocities[(j - 1) * grid_width + i + 1].x += grid_stride_corner_f;
		grid_velocities[(j - 1) * grid_width + i + 1].y -= grid_stride_corner_f;
		grid_velocities[(j + 1) * grid_width + i + 1].x += grid_stride_corner_f;
		grid_velocities[(j + 1) * grid_width + i + 1].y += grid_stride_corner_f;
	}

	UpdateGrid();
	DrawGrid();
}

void GridWaveDemoExit(void)
{
	free(grid_positions);
	free(grid_velocities);
	free(grid_offsets);
	free(grid_offsets_cache);
	CP_Settings_Restore();
}

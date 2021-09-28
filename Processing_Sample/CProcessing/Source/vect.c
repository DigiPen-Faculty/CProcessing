//------------------------------------------------------------------------------
// File:	vect.c
// Author:	Daniel Hamilton
// Brief:	Vector data structure
//
// GitHub Repository:
// https://github.com/DigiPen-Faculty/CProcessing
//
//---------------------------------------------------------
// MIT License
//
// Copyright (C) 2021 DigiPen Institute of Technology
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//---------------------------------------------------------

#include "vect.h"

#define VECT_CAPACITY 256

typedef struct vect_t {
	int size;
	int capacity;
	size_t data_size;
	void* data;
} vect_t;

void vect_err(const char* error)
{
	fprintf(stderr, "%s\n", error);
	exit(1);
}

vect_t* vect_init(size_t data_size, unsigned int capacity)
{
	vect_t* v = malloc(sizeof(vect_t));
	if (v == NULL)
		vect_err("vect_init: allocation of vect_t failed.");
	else
	{
		if (capacity == 0)
			capacity = VECT_CAPACITY;
		v->data = malloc(data_size * capacity);
		if (v->data == NULL)
			vect_err("vect_init: allocation of vect_t data failed.");
		v->data_size = data_size;
		v->size = 0;
		v->capacity = capacity;
	}
	return v;
}

void vect_free(void* v)
{
	free(((vect_t*)v)->data);
	free(((vect_t*)v));
}

void vect_resize(vect_t* v)
{
	if (v != NULL && v->data != NULL)
	{
		v->capacity *= 2; /* double capacity */
		v->data = realloc(v->data, v->data_size * v->capacity);
		if (v->data == NULL)
			vect_err("vect_resize: resize failed.");
	}
}

unsigned int vect_chk_bounds(vect_t* v, unsigned int pos) {
	return ((int)pos < v->size);
}

unsigned int vect_at(vect_t* v, unsigned int pos)
{
	if (!vect_chk_bounds((vect_t*)v, pos))
		vect_err("vect_at: out of bounds.");
	return pos;
}

unsigned int vect_set(vect_t* v, unsigned int pos)
{
	if (!vect_chk_bounds((vect_t*)v, pos))
		vect_err("vect_set: out of bounds.");
	return pos;
}

unsigned int vect_push(vect_t* v)
{
	if (v->size == v->capacity)
		vect_resize(v);
	return v->size++;
}

unsigned int vect_pop(vect_t* v)
{
	if (v->size == 0)
		vect_err("vect_pop: underflow.");
	return --v->size;
}

unsigned int vect_rem(vect_t* v, unsigned int pos)
{
	if (!vect_chk_bounds((vect_t*)v, pos))
		vect_err("vect_rem: out of bounds.");
	char* dest = (char*)v->data + (v->data_size * pos);
	char* src = dest + v->data_size;
	size_t num_bytes = v->data_size * (v->size - pos - 1);
	memmove(dest, src, num_bytes);
	v->size--;
	return pos;
}

unsigned int vect_ins(vect_t* v, unsigned int pos)
{
	if (!vect_chk_bounds((vect_t*)v, pos))
		vect_err("vect_rem: out of bounds.");
	char* src = (char*)v->data + (v->data_size * pos);
	char* dest = src + v->data_size;
	size_t num_bytes = v->data_size * (v->size - pos);
	memmove(dest, src, num_bytes);
	v->size++;
	return pos;
}

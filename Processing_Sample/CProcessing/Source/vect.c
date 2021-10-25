//------------------------------------------------------------------------------
// file:	vect.c
// author:	Daniel Hamilton
// brief:	Vector data structure
//
// Copyright © 2019 DigiPen, All rights reserved.
//------------------------------------------------------------------------------

#include "vect.h"

#define VECT_CAPACITY 256

typedef struct vect_t {
    size_t size;
    size_t capacity;
    size_t data_size;
    void *data;
} vect_t;

void vect_err(const char *error)
{
    fprintf(stderr, "%s\n", error);
    exit(1);
}

vect_t* vect_init(size_t data_size, size_t capacity)
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

void vect_free(void *v)
{
    free(((vect_t *)v)->data);
    free(((vect_t *)v));
}

void vect_resize(vect_t *v)
{
    if (v != NULL && v->data != NULL)
    {
        v->capacity *= 2; /* double capacity */
        v->data = realloc(v->data, v->data_size * v->capacity);
        if (v->data == NULL)
            vect_err("vect_resize: resize failed.");
    }
}

size_t vect_chk_bounds(vect_t *v, size_t pos) {
    return ((int)pos < v->size);
}

size_t vect_at(vect_t *v, size_t pos)
{
    if (!vect_chk_bounds((vect_t*)v, pos))
        vect_err("vect_at: out of bounds.");
    return pos;
}

size_t vect_set(vect_t *v, size_t pos)
{
    if (!vect_chk_bounds((vect_t*)v, pos))
        vect_err("vect_set: out of bounds.");
    return pos;
}

size_t vect_push(vect_t *v)
{
    if (v->size == v->capacity)
        vect_resize(v);
    return v->size++;	
}

size_t vect_pop(vect_t *v)
{
    if (v->size == 0)
        vect_err("vect_pop: underflow.");
    return --v->size;
}

size_t vect_rem(vect_t *v, size_t pos)
{
    if (!vect_chk_bounds((vect_t*)v, pos))
        vect_err("vect_rem: out of bounds.");
    char *dest = (char *) v->data + (v->data_size * pos);
    char *src = dest + v->data_size;
    size_t num_bytes = v->data_size * (v->size - pos - 1);
    memmove(dest, src, num_bytes);
    v->size--;	
    return pos;
}

size_t vect_ins(vect_t *v, size_t pos)
{
    if (!vect_chk_bounds((vect_t*)v, pos))
        vect_err("vect_rem: out of bounds.");
    char *src = (char *) v->data + (v->data_size * pos);
    char *dest = src + v->data_size;
    size_t num_bytes = v->data_size * (v->size - pos);
    memmove(dest, src, num_bytes);
    v->size++;
    return pos;
}
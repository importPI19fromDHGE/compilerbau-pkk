//
// Created by max on 20.03.22.
//

#include "dynamic_array.h"

#include <malloc.h>

void initArray(Array *a, size_t initialSize) {
    a->array = malloc(initialSize * sizeof(nameentry_t));
    a->used = 0;
    a->size = initialSize;
}

void insertArray(Array *a, nameentry_t *element) {
    // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
    // Therefore a->used can go up to a->size
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(nameentry_t));
    }
    a->array[a->used++] = *element;
}

void freeArray(Array *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

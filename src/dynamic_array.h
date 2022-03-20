//
// Created by max on 20.03.22.
// Source: https://stackoverflow.com/questions/3536153/c-dynamically-growing-array#3536261
//

#ifndef COMPILERBAU_PKK_DYNAMIC_ARRAY_H
#define COMPILERBAU_PKK_DYNAMIC_ARRAY_H

#include <stddef.h>
#include "turtle-types.h"

typedef struct {
    nameentry_t *array;
    size_t used;
    size_t size;
} Array;

void initArray(Array *a, size_t initialSize);
void insertArray(Array *a, nameentry_t *element);
void freeArray(Array *a);

#endif //COMPILERBAU_PKK_DYNAMIC_ARRAY_H

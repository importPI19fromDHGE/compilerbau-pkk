//
// Created by max on 20.03.22.
// Source: https://stackoverflow.com/questions/3536153/c-dynamically-growing-array#3536261
//

#ifndef COMPILERBAU_PKK_DYNAMIC_ARRAY_H
#define COMPILERBAU_PKK_DYNAMIC_ARRAY_H

#include <stddef.h>
#include "turtle-types.h"

/// Totally under-engineered dynamic array.
/// Sadly, it only works for the token stream.
typedef struct {
    token_t *array; ///< Pointer to the array on the heap.
    size_t used; ///< number of used elements.
    size_t size; ///< number of total allocated elements.
} Array;

/// Initialize an Array Pointer, allocate memory.
void initArray(Array *a, ///< [in,out] Pointer to the Array.
               size_t initialSize ///< [in] Initial size of the Array.
               );
/// Inserts an element into an initialized Array, allocating more space if needed.
void insertArray(Array *a, ///< [in,out] Pointer of the Array to insert to.
                 token_t element ///< [in] The Token element to insert.
                 );
/// Frees the Array.
void freeArray(Array *a /**< The Array to free.*/ );

#endif //COMPILERBAU_PKK_DYNAMIC_ARRAY_H

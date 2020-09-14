#pragma once

#include <stddef.h>

#define CONTAINER_ARRAY

void container_array_delete(void* array);

#define container_array_reserve(array, len) \
    container_array_reserve_((void**) &array, sizeof(*array), len)
void container_array_reserve_(void** array, size_t elmlen, size_t len);

#define container_array_resize(array, len) \
    container_array_resize_((void**) &array, sizeof(*array), len)
void container_array_resize_(void** array, size_t elmlen, size_t len);

#define container_array_insert(array, index) \
    container_array_insert_((void**) &array, sizeof(*array), index)
void container_array_insert_(void** array, size_t elmlen, size_t index);

#define container_array_remove(array, index) \
    container_array_remove_((void**) &array, sizeof(*array), index)
void container_array_remove_(void** array, size_t elmlen, size_t index);

#define container_array_duplicate(array) \
    (typeof(array)) container_array_duplicate_( \
        (const void*) array, sizeof(*array))
void* container_array_duplicate_(const void* array, size_t elmlen);

#define container_array_get_length(array) container_array_get_length_(array)
size_t container_array_get_length_(const void* array);

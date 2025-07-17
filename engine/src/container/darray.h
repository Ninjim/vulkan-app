#pragma once

#include "defines.h"

// Enum to keep track of what position in the darray to we store the information about it, the last value is again to indicate how many values there are
enum{
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH
};

// Capacity and length are not the same! Capacity determines the entire size
// of the darray and is what is technically allocated on the heap as space
// for the darray, however the length is the actualy amount of entries used
// in the darray, the actualy data inside

KAPI void* _darray_create(u64 length, u64 stride);
KAPI void _darray_destroy(void* array);

KAPI u64 _darray_field_get(void* array, u64 field);
KAPI void _darray_field_set(void* array, u64 field, u64 value);

KAPI void* _darray_resize(void* array);

// We use pointers here so we can use memcopy
KAPI void* _darray_push(void* array, const void* value_ptr);
KAPI void _darray_pop(void* array, void* dest);

KAPI void* _darray_pop_at(void* array, u64 index, void* dest);
KAPI void* _darray_insert_at(void* array, u64 index, void* value_ptr);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRA_RESIZE_FACTOR 2

// We export the underscore functions even tho they aren't made to be used
// outside the engine, because we need to have them exported so we can use
// the macros!

// We also use macros to incorporate template-like functionalities
#define darray_create(type) _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define darray_reserve(type, capacity) _darray_create(capacity, sizeof(type))

#define darray_destroy(array) _darray_destroy(array)

#define darray_push(array, value) {  \
    typeof(value) temp = value; \
    array = _darray_push(array,&temp); \
}

#define darray_pop(array, value_ptr)   \
    _darray_pop(array, value_ptr);


#define darray_insert_at(array, index, value) {  \
    typeof(value) temp = value; \
    array = _darray_insert_at(array, index, &temp); \
}

#define darray_pop_at(array, index, value_ptr) _darray_pop_at(array, index, value_ptr)

#define darray_clear(array) _darray_field_set(array, DARRAY_LENGTH, 0)

#define darray_capacity(array) _darray_field_get(array, DARRAY_CAPACITY)

#define darray_length(array) _darray_field_get(array, DARRAY_LENGTH)

#define darray_stride(array) _darray_field_get(array, DARRAY_STRIDE)

#define darray_length_set(array, value) _darray_field_set(__ARRAY_OPERATORS, DARRAY_LENGTH, value)

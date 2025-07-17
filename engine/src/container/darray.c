#include "darray.h"

#include "core/logging.h"
#include <stdlib.h>
#include <string.h>

// We only return the array part of the darray without the info stored,
// I think we use a u64 pointer first just so we can write to the
// first three values?
 void* _darray_create(u64 length, u64 stride){
u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
u64 array_size = length * stride;
u64* new_array = malloc(header_size+array_size);
memset(new_array,0,header_size + array_size);
new_array[DARRAY_CAPACITY] = length;
new_array[DARRAY_LENGTH] = 0;
new_array[DARRAY_STRIDE] = stride;

return (void*)(new_array + DARRAY_FIELD_LENGTH);
}

 void _darray_destroy(void* array){
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH; // Gets the header part
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    free(header);
}

 u64 _darray_field_get(void* array, u64 field){
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

 void _darray_field_set(void* array, u64 field, u64 value){
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

 void* _darray_resize(void* array){
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    void* temp = _darray_create((darray_capacity(array) * DARRA_RESIZE_FACTOR), stride);
    memcpy(temp,array,length*stride);

    _darray_field_set(temp, DARRAY_LENGTH, length);
    _darray_destroy(array);
    return temp;
 }


void* _darray_push(void* array, const void* value_ptr){
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if(length >= darray_capacity(array)){
        array = _darray_resize(array);
    }

    u64 addr = (u64)array; // Idk why we aren't just making addr a pointer but oh well! Maybe because we can't do pointer arithmetic on a void pointer and we obviously can't cast it to a type in this case?
    addr += (length*stride);
    memcpy((void*)addr,value_ptr,stride);
    _darray_field_set(array, DARRAY_LENGTH, length+1);
    return array;
}


// DOESN'T POP IN THE TRADITIONAL SENSE? It doesn't actually REMOVE the data, but it
// changes the length part of the header so that
// the darray doesn't actually consider the removed bit part
// of itself? Wow...
 void _darray_pop(void* array, void* dest){
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    u64 addr = (u64)array; // Idk why we aren't just making addr a pointer but oh well! Maybe because we can't do pointer arithmetic on a void pointer and we obviously can't cast it to a type in this case?
    addr += ((length-1)*stride);
    memcpy(dest, (void*)addr,stride);
    _darray_field_set(array, DARRAY_LENGTH, length-1);
}

 void* _darray_pop_at(void* array, u64 index, void* dest){
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    // The comment above explains why we check for length and not capacity!
    if(index >= length){
        ERROR("Index outside the bounds of this array! Length: %i, index: %i", length, index);
        return array;
    }

    u64 addr = (u64)array; // Idk why we aren't just making addr a pointer but oh well! Maybe because we can't do pointer arithmetic on a void pointer and we obviously can't cast it to a type in this case?
    
    memcpy(dest,(void*)(addr+(index*stride)),stride);

    if(index != length - 1){
        memcpy(
            (void*)(addr + (index *stride)),
            (void*)(addr+((index+1)*stride)),
            ((length-index) * stride)
        );
    }

    _darray_field_set(array, DARRAY_LENGTH, length-1);
    return array;
 }

KAPI void* _darray_insert_at(void* array, u64 index, void* value_ptr){
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    // We check for length because that is what the darray internally deals with!
    if(index > length){
        ERROR("Index outside the bounds of this array! Length: %i, index: %i", length, index);
        return array;
    }

    // Feel like we should first allocate space before calling out of bound indexes!
    if(length >= darray_capacity(array)){
        array = _darray_resize(array);
    }

    u64 addr = (u64)array; // Idk why we aren't just making addr a pointer but oh well! Maybe because we can't do pointer arithmetic on a void pointer and we obviously can't cast it to a type in this case?
    
    // I don't agree with this! I think it should be length! But let's see
    if(index != length){
        memcpy(
            (void*)(addr + ((index+1)*stride)),
            (void*)(addr + (index*stride)),
            ((length-index) * stride)
        );
    }

    memcpy((void*)(addr + (index * stride)), value_ptr, stride);

    _darray_field_set(array, DARRAY_LENGTH, length+1);
    return array;
}
#ifndef DARRAY_H_
#define DARRAY_H_
#define DADEF static inline

#ifndef DA_DEFAULT_INITIAL_CAPACITY
#define DA_DEFAULT_INITIAL_CAPACITY 64
#endif // DEFAULT_INITIAL_CAPACITY

#ifndef DA_DEFAULT_INCREASE_SIZE
#define DA_DEFAULT_INCREASE_SIZE 64
#endif // DEFAULT_INCREASE_SIZE

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"

typedef struct DArray
{
    void* elements;             // pointer to the array of elements
    uint16_t element_size;      // size of the element in byte
    uint32_t length;            // number of element we holden in the array
    uint32_t capacity;          // number of element can be hold in the array
    uint32_t increase_size;     // number of elements we prepare to hold in the array
} DArray;

DADEF DArray DA_create_array(uint16_t element_size, uint32_t initial_capacity, uint32_t increase_size);
DADEF void DA_append(DArray* da, void* element);
DADEF void DA_insert(DArray* da, void *element, uint32_t index);
DADEF void DA_set_element(DArray* da, void *element, uint32_t index);
DADEF size_t DA_len(DArray* da);
DADEF int64_t DA_get_first_index(DArray* da, void* element);
DADEF void* DA_get_element(DArray* da, uint32_t index);
DADEF void DA_delete(DArray* da, uint32_t index);
DADEF void DA_reset(DArray* da);
DADEF void DA_clear(DArray* da);
DADEF void DA_destroy(DArray* da);

#ifdef DA_IMPLEMENTATION

DADEF DArray DA_create_array(uint16_t element_size, 
                            uint32_t initial_capacity, 
                            uint32_t increase_size){
    if (initial_capacity == 0) {
        initial_capacity = DA_DEFAULT_INITIAL_CAPACITY;
    }
    if (increase_size == 0) {
        increase_size = DA_DEFAULT_INCREASE_SIZE;
    }
    DArray da = {.element_size = element_size,
                .capacity = initial_capacity,
                .increase_size = increase_size};
    da.elements = (void*)malloc(da.capacity * da.element_size);
    return da;
}

DADEF void DA_append(DArray* da, void* element){
    if(da->length + 1 > da->capacity){
        da->elements = (void*)realloc(da->elements, (da->capacity + da->increase_size) * da->element_size);
        da->capacity += da->increase_size;
    }
    memcpy(da->elements + (da->length * da->element_size), element, (size_t)(da->element_size));
    da->length += 1;
}

DADEF void DA_insert(DArray* da, void *element, uint32_t index){
    if(da->length + 1 > da->capacity){
        da->elements = (void*)realloc(da->elements, (size_t)((da->capacity + da->increase_size) * da->element_size));
        da->capacity += da->increase_size;
    }
    
    for(uint32_t i = da->length - 1; i >= index; i--){
        // memcpy(da->elements + (da->length * da->element_size), element, (size_t)(da->element_size));
        memmove(da->elements + ((i + 1) * da->element_size), 
                da->elements + ((i + 0) * da->element_size), 
                (size_t)da->element_size);
    }
    memcpy(da->elements + (index * da->element_size), 
            element, (size_t)(da->element_size));
    da->length += 1;
}

DADEF void DA_set_element(DArray* da, void *element, uint32_t index){
    memcpy(da->elements + (index * da->element_size),
            element, (size_t)da->element_size);
}

DADEF size_t DA_len(DArray* da){
    return (size_t)da->length;
}

DADEF int64_t DA_get_first_index(DArray* da, void* element){
    for(uint32_t i = 0; i < da->length; i++)
    {
        if(memcmp(da->elements + (i * da->element_size), 
                element, da->element_size) == 0){
            return (int64_t)i;
        }
    }
    return -1;
}

DADEF void* DA_get_element(DArray* da, uint32_t index){
    if(index >= da->length){ 
        return NULL;
    }
    return (da->elements + (index * da->element_size));
}

DADEF void DA_delete(DArray* da, uint32_t index){
    if(index >= da->length) return;
    uint32_t NumberOfElementsBehind = da->length - 1 - index;
    (void)NumberOfElementsBehind;
    for(uint32_t i = index; i < da->length; i++){
        memmove(da->elements + ((i + 0) * da->element_size), 
                da->elements + ((i + 1) * da->element_size), 
                (size_t)da->element_size);
    }
    da->length -= 1;
    if(da->length < (da->capacity - da->increase_size)){
        da->elements = (void*)realloc(da->elements, (size_t)((da->capacity - da->increase_size) * da->element_size));
        da->capacity -= da->increase_size;
    }
}

DADEF void DA_reset(DArray* da){
    da->length = 0;
}

DADEF void DA_clear(DArray* da){
    memset(da->elements, 0, (size_t)(da->capacity * da->element_size));
    da->capacity = da->increase_size;
    da->elements = (void*)realloc(da->elements, da->capacity);
    da->length = 0;
}

DADEF void DA_destroy(DArray* da){
    free(da->elements);
    da->increase_size = 0;
    da->capacity = 0;
    da->length = 0;
}

#endif // DA_IMPLEMENTATION
#endif // DARRAY_H_

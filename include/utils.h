#ifndef utils_h
#define utils_h


typedef struct {
    void* data; // Pointer to the dynamic array data
    size_t element_size; // Size of each element in the array
    size_t capacity; // Total capacity of the array
    size_t size; // Current number of elements in the array
} dynamic_array_t;


#define da_append(xs, x) do { \
    if ((xs)->size >= (xs)->capacity) { \
        (xs)->capacity = (xs)->capacity ? (xs)->capacity * 2 : 256; \
        (xs)->data = realloc((xs)->data, (xs)->capacity * (xs)->element_size); \
    } \
    memcpy((char*)(xs)->data + ((xs)->size * (xs)->element_size), &(x), (xs)->element_size); \
    (xs)->size++; \
} while (0)











#endif
#ifndef DA_H
#define DA_H

#ifndef DA_REALLOC
#include <stdlib.h>
#define DA_REALLOC realloc
#endif

#ifndef DA_FREE
#include <stdlib.h>
#define DA_FREE free
#endif

#ifndef DA_ASSERT
#include <assert.h>
#define DA_ASSERT assert
#endif

/**
 * Dynamic array implementation
 * The dynamic array struct should be defined by the user as follows:
 * typedef struct {
 *     Type *data;
 *     size_t length;
 *     size_t capacity;
 *     ... // any additional fields
 * } DynamicArray;
 */

#define DA_INIT_CAPACITY 16

#define da_reserve(da, expected_capacity)                                              \
    do {                                                                               \
        if ((size_t)(expected_capacity) > (da)->capacity) {                            \
            if ((da)->capacity == 0) {                                                 \
                if ((size_t)(expected_capacity) > DA_INIT_CAPACITY)                    \
                    (da)->capacity = (size_t)(expected_capacity);                      \
                else                                                                   \
                    (da)->capacity = DA_INIT_CAPACITY;                                 \
            } else {                                                                   \
                if ((size_t)(expected_capacity) > (da)->capacity * 2)                  \
                    (da)->capacity = (size_t)(expected_capacity);                      \
                else                                                                   \
                    (da)->capacity *= 2;                                               \
            }                                                                          \
            (da)->data = DA_REALLOC((da)->data, (da)->capacity * sizeof(*(da)->data)); \
            DA_ASSERT((da)->data != NULL);                                             \
        }                                                                              \
    } while (0)

#define da_append(da, item)                  \
    do {                                     \
        da_reserve((da), (da)->length + 1);  \
        (da)->data[(da)->length++] = (item); \
    } while (0)

#define da_remove_unordered(da, idx)                      \
    do {                                                  \
        (da)->data[(idx)] = (da)->data[(da)->length - 1]; \
        (da)->length--;                                   \
    } while (0)

#define da_shrink(da, capacity)                                                        \
    do {                                                                               \
        if ((da)->capacity > (capacity)) {                                             \
            (da)->capacity = (capacity);                                               \
            (da)->data = DA_REALLOC((da)->data, (da)->capacity * sizeof(*(da)->data)); \
            DA_ASSERT((da)->data != NULL);                                             \
        }                                                                              \
    } while (0)

#define da_shrink_to_fit(da) da_shrink((da), (da)->length)

#define da_free(da)          \
    do {                     \
        DA_FREE((da)->data); \
        (da)->data = NULL;   \
        (da)->length = 0;    \
        (da)->capacity = 0;  \
    } while (0)

#endif // DA_H
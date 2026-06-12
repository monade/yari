#ifndef YR_DA_H
#define YR_DA_H

#ifndef YR_DA_REALLOC
#include <stdlib.h>
#define YR_DA_REALLOC realloc
#endif

#ifndef YR_DA_FREE
#include <stdlib.h>
#define YR_DA_FREE free
#endif

#ifndef YR_DA_ASSERT
#include <assert.h>
#define YR_DA_ASSERT assert
#endif

#define YR_ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))

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

#define YR_DA_INIT_CAPACITY 16

#define yr_da_reserve(da, expected_capacity)                                              \
    do {                                                                                  \
        if ((size_t)(expected_capacity) > (da)->capacity) {                               \
            if ((da)->capacity == 0) {                                                    \
                if ((size_t)(expected_capacity) > YR_DA_INIT_CAPACITY)                    \
                    (da)->capacity = (size_t)(expected_capacity);                         \
                else                                                                      \
                    (da)->capacity = YR_DA_INIT_CAPACITY;                                 \
            } else {                                                                      \
                if ((size_t)(expected_capacity) > (da)->capacity * 2)                     \
                    (da)->capacity = (size_t)(expected_capacity);                         \
                else                                                                      \
                    (da)->capacity *= 2;                                                  \
            }                                                                             \
            (da)->data = YR_DA_REALLOC((da)->data, (da)->capacity * sizeof(*(da)->data)); \
            YR_DA_ASSERT((da)->data != NULL);                                             \
        }                                                                                 \
    } while (0)

#define yr_da_append(da, item)                 \
    do {                                       \
        yr_da_reserve((da), (da)->length + 1); \
        (da)->data[(da)->length++] = (item);   \
    } while (0)

#define yr_da_remove_unordered(da, idx)                   \
    do {                                                  \
        (da)->data[(idx)] = (da)->data[(da)->length - 1]; \
        (da)->length--;                                   \
    } while (0)

#define yr_da_shrink(da, capacity)                                                        \
    do {                                                                                  \
        YR_DA_ASSERT((capacity) <= (da)->length);                                         \
        if ((da)->capacity > (capacity)) {                                                \
            (da)->capacity = (capacity);                                                  \
            (da)->data = YR_DA_REALLOC((da)->data, (da)->capacity * sizeof(*(da)->data)); \
            YR_DA_ASSERT((da)->data != NULL);                                             \
        }                                                                                 \
    } while (0)

#define yr_da_shrink_to_fit(da) yr_da_shrink((da), (da)->length)

#define yr_da_free(da)          \
    do {                        \
        YR_DA_FREE((da)->data); \
        (da)->data = NULL;      \
        (da)->length = 0;       \
        (da)->capacity = 0;     \
    } while (0)

#ifdef YARI_NO_PREFIX
#define da_reserve yr_da_reserve
#define da_append yr_da_append
#define da_remove_unordered yr_da_remove_unordered
#define da_shrink yr_da_shrink
#define da_shrink_to_fit yr_da_shrink_to_fit
#define da_free yr_da_free
#endif
#endif // YR_DA_H

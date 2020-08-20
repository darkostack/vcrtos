#ifndef CORE_CODE_UTILS_H
#define CORE_CODE_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

#define ARRAY_END(array) (&array[ARRAY_LENGTH(array)])

#define ALIGNED_VAR_SIZE(size, align_type) (((size) + (sizeof(align_type) - 1)) / sizeof(align_type))

#define DEFINE_ALIGNED_VAR(name, size, align_type) \
    align_type name[(((size) + (sizeof(align_type) - 1)) / sizeof(align_type))]

#if defined __GNUC__
#define container_of(PTR, TYPE, MEMBER) \
    (__extension__ ({ \
        __extension__ const __typeof__ (((TYPE *) 0)->MEMBER) *__m____ = (PTR); \
        ((TYPE *) ((char *) __m____ - offsetof(TYPE, MEMBER))); \
    }))
#else
#define container_of(PTR, TYPE, MEMBER) \
        ((TYPE *) ((char *) (PTR) - offsetof(TYPE, MEMBER)))
#endif

#define SUCCESS_OR_EXIT(status) \
    do                          \
    {                           \
        if ((status) != 0)      \
        {                       \
            goto exit;          \
        }                       \
    } while (false)

#define VERIFY_OR_EXIT(condition, ...) \
    do                                 \
    {                                  \
        if (!(condition))              \
        {                              \
            __VA_ARGS__;               \
            goto exit;                 \
        }                              \
    } while (false)

#define EXIT_NOW(...) \
    do                \
    {                 \
        __VA_ARGS__;  \
        goto exit;    \
    } while (false)

#define IGNORE_RETURN_VALUE(statement) \
    do                                 \
    {                                  \
        if (statement)                 \
        {                              \
        }                              \
    } while (false)

#ifdef __cplusplus
}
#endif

#endif /* CORE_CODE_UTILS_H */

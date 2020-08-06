#ifndef CORE_CODE_UTILS_HPP
#define CORE_CODE_UTILS_HPP

#include <stdbool.h>

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

#define ARRAY_END(array) (&array[ARRAY_LENGTH(array)])

#define ALIGNED_VAR_SIZE(size, align_type) (((size) + (sizeof(align_type) - 1)) / sizeof(align_type))

#define DEFINE_ALIGNED_VAR(name, size, align_type) \
    align_type name[(((size) + (sizeof(align_type) - 1)) / sizeof(align_type))]

#define CONTAINER_OF(ptr, type, member) ({                    \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);  \
        (type *)( (char *)__mptr - offsetof(type,member) );})

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

#endif /* CORE_CODE_UTILS_HPP */

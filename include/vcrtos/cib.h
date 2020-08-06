#ifndef VCRTOS_CIB_H
#define VCRTOS_CIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cib
{
    unsigned int read_count;
    unsigned int write_count;
    unsigned int mask;
} cib_t;

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_CIB_H */

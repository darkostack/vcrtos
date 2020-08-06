#ifndef VCRTOS_INSTANCE_H
#define VCRTOS_INSTANCE_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct instance instance_t;

instance_t *instance_init(void);

bool instance_is_initialized(instance_t *instance);

instance_t *instance_get(void);

const char *instance_get_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_INSTANCE_H */

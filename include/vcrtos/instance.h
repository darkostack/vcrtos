#ifndef VCRTOS_INSTANCE_H
#define VCRTOS_INSTANCE_H

#include <stdbool.h>
#include <stdlib.h>

#include <vcrtos/config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct instance instance_t;

#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

instance_t *instance_init(void *buffer, size_t *size);

#else

instance_t *instance_init_single(void);

instance_t *instance_get(void);

#endif // #if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

bool instance_is_initialized(instance_t *instance);

const char *instance_get_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_INSTANCE_H */

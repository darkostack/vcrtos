#ifndef VCRTOS_MUTEX_H
#define VCRTOS_MUTEX_H

#include <vcrtos/config.h>
#include <vcrtos/list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MUTEX_LOCKED ((list_node_t *)-1)

typedef struct mutex
{
    list_node_t queue;
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    void *instance;
#endif
} mutex_t;

void mutex_lock(mutex_t *mutex);

void mutex_unlock(mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_MUTEX_H */

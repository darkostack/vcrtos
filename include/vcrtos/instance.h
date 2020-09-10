/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

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

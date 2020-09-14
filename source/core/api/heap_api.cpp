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

#include <vcrtos/config.h>
#include <vcrtos/heap.h>

#include "core/instance.hpp"

#include "utils/heap.hpp"

#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

using namespace vc;

void heap_free(void *instance, void *ptr)
{
    Instance &instances = *static_cast<Instance *>(instance);
    utils::Heap heap = instances.get_heap();
    heap.free(ptr);
}

void *heap_calloc(void *instance, size_t count, size_t size)
{
    Instance &instances = *static_cast<Instance *>(instance);
    utils::Heap heap = instances.get_heap();
    return heap.calloc(count, size);
}

#endif // #if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

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

#include "core/instance.hpp"
#include "core/code_utils.h"
#include "core/thread.hpp"
#include "core/new.hpp"

namespace vc {

#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
DEFINE_ALIGNED_VAR(instance_raw, sizeof(Instance), uint64_t);
#endif

Instance::Instance(void)
    : thread_scheduler(*this)
#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    , heap()
#endif
    , initialized(false)
{
#ifdef UNITTEST
    initialized = true;
#endif
}

#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE

Instance &Instance::init(void *buffer, size_t *size)
{
    Instance *instance = NULL;

    VERIFY_OR_EXIT(size != NULL);

    VERIFY_OR_EXIT(sizeof(Instance) <= *size, *size = sizeof(Instance));

    VERIFY_OR_EXIT(buffer != NULL);

    instance = new (buffer) Instance();

    instance->after_init();

exit:
    return *instance;
}

#else /* #if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE */

Instance &Instance::init_single(void)
{
    Instance *instance = &get();

    VERIFY_OR_EXIT(instance->is_initialized() == false);

    instance = new (&instance_raw) Instance();

    instance->after_init();

exit:
    return *instance;
}

Instance &Instance::get(void)
{
    void *instance = &instance_raw;
    return *static_cast<Instance *>(instance);
}

#endif /* #if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE */

void Instance::after_init(void)
{
    initialized = true;
}

} // namespace vc

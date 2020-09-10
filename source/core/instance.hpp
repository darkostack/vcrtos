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

#ifndef CORE_INSTANCE_HPP
#define CORE_INSTANCE_HPP

#include <stdint.h>
#include <stdbool.h>

#include <vcrtos/config.h>
#include <vcrtos/instance.h>

#include "core/thread.hpp"

typedef struct instance
{
} instance_t;

namespace vc {

class Instance : public instance_t
{
public:
#ifdef UNITTEST
    explicit Instance(void);
#endif
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    static Instance &init(void *buffer, size_t *size);
#else
    static Instance &init_single(void);
    static Instance &get(void);
#endif

    bool is_initialized(void) const { return initialized; }

    template <typename Type> inline Type &get(void);

    void after_init(void);

private:
#ifndef UNITTEST
    explicit Instance(void);
#endif

    ThreadScheduler thread_scheduler;

    bool initialized;
};

template <> inline ThreadScheduler &Instance::get(void)
{
    return thread_scheduler;
}

} // namespace vc

#endif /* CORE_INSTANCE_HPP */

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

#include <vcrtos/mutex.h>

#include "core/instance.hpp"
#include "core/mutex.hpp"
#include "core/new.hpp"

using namespace vc;

void mutex_init(void *instances, mutex_t *mutex)
{
    Instance &instance = *static_cast<Instance *>(instances);
    mutex = new (mutex) Mutex(instance);
}

void mutex_lock(mutex_t *mutex)
{
    Mutex &mtx = *static_cast<Mutex *>(mutex);
    mtx.lock();
}

int mutex_try_lock(mutex_t *mutex)
{
    Mutex &mtx = *static_cast<Mutex *>(mutex);
    return mtx.try_lock();
}

void mutex_unlock(mutex_t *mutex)
{
    Mutex &mtx = *static_cast<Mutex *>(mutex);
    mtx.unlock();
}

void mutex_unlock_and_sleeping(mutex_t *mutex)
{
    Mutex &mtx = *static_cast<Mutex *>(mutex);
    mtx.unlock_and_sleeping_current_thread();
}

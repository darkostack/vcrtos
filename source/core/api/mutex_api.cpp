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

void mutex_try_lock(mutex_t *mutex)
{
    Mutex &mtx = *static_cast<Mutex *>(mutex);
    mtx.try_lock();
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

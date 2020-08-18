#include <vcrtos/config.h>
#include <vcrtos/assert.h>
#include <vcrtos/xtimer.h>

#include "core/instance.hpp"
#include "core/xtimer.hpp"
#include "core/new.hpp"

using namespace vc;

void xtimer_init(void *instances, xtimer_t *timer, xtimer_handler_func_t callback, void *arg)
{
    Instance &instance = *static_cast<Instance *>(instances);
    timer = new (timer) XTimer(instance, callback, arg);
}

uint32_t xtimer_now_usec(void *instances)
{
    vcassert(VCRTOS_CONFIG_XTIMER_HZ == VCRTOS_CONFIG_XTIMER_HZ_BASE);
    Instance &instance = *static_cast<Instance *>(instances);
    return instance.get<XTimerScheduler>().now();
}

uint64_t xtimer_now_usec64(void *instances)
{
    vcassert(VCRTOS_CONFIG_XTIMER_HZ == VCRTOS_CONFIG_XTIMER_HZ_BASE);
    Instance &instance = *static_cast<Instance *>(instances);
    return instance.get<XTimerScheduler>().now64();
}

void xtimer_set(xtimer_t *timer, uint32_t offset)
{
    XTimer &xtimer = *static_cast<XTimer *>(timer);
    xtimer.set(offset);
}

void xtimer_set64(xtimer_t *timer, uint64_t offset)
{
    XTimer &xtimer = *static_cast<XTimer *>(timer);
    xtimer.set64(offset, offset >> 32);
}

void xtimer_sleep_usec(void *instances, uint32_t usec)
{
    Instance &instance = *static_cast<Instance *>(instances);
    instance.get<XTimerScheduler>().sleep64(usec, 0);
}

void xtimer_sleep_usec64(void *instances, uint64_t usec)
{
    Instance &instance = *static_cast<Instance *>(instances);
    instance.get<XTimerScheduler>().sleep64(static_cast<uint32_t>(usec), static_cast<uint32_t>(usec >> 32));
}

void xtimer_remove(xtimer_t *timer)
{
    XTimer &xtimer = *static_cast<XTimer *>(timer);
    xtimer.remove();
}

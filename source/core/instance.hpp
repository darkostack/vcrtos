#ifndef CORE_INSTANCE_HPP
#define CORE_INSTANCE_HPP

#include <stdint.h>
#include <stdbool.h>

#include <vcrtos/config.h>
#include <vcrtos/instance.h>

#include "core/thread.hpp"
#include "core/xtimer.hpp"

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

    XTimerScheduler xtimer_scheduler;

    bool initialized;
};

template <> inline ThreadScheduler &Instance::get(void)
{
    return thread_scheduler;
}

template <> inline XTimerScheduler &Instance::get(void)
{
    return xtimer_scheduler;
}

} // namespace vc

#endif /* CORE_INSTANCE_HPP */

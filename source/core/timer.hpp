#ifndef CORE_TIMER_HPP
#define CORE_TIMER_HPP

#include <vcrtos/config.h>
#include <vcrtos/timer.h>
#include <vcrtos/cpu.h>

namespace vc {

class TimerScheduler;
class Instance;

#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t instance_raw[];
#endif

class Timer : public timer_t
{
public:
    explicit Timer(Instance &instances, timer_handler_func_t cb, void *args)
        : next(nullptr)
        , target(0)
        , long_target(0)
        , callback(cb)
        , arg(args)
    {
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
        instance = static_cast<void *>(&instances);
#else
        (void)instances;
#endif
    }

    void set

private:
    template <typename Type> inline Type &get(void) const;

#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance &get_instance(void) const { return *static_cast<Instance *>(instance); }
#else
    Instance &get_instance(void) const { return *reinterpret_cast<Instance *>(&instance_raw); }
#endif
};

} // namespace vc

#endif /* CORE_TIMER_HPP */

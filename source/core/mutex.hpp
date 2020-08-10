#ifndef CORE_MUTEX_HPP
#define CORE_MUTEX_HPP

#include <stddef.h>
#include <stdint.h>

#include <vcrtos/config.h>
#include <vcrtos/mutex.h>

#include "core/list.hpp"

namespace vc {

class Instance;

#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
extern uint64_t instance_raw[];
#endif

class Mutex : public mutex_t
{
public:
    explicit Mutex(Instance &instance)
    {
        init(instance);
    }

    void init(Instance &instances)
    {
#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
        instance = static_cast<void *>(&instances);
#else
        (void)instances;
#endif
        queue.next = NULL;
    }

    int try_lock(void) { return set_lock(0); }

    void lock(void) { set_lock(1); }

    void unlock(void);

    void unlock_and_sleeping_current_thread(void);

private:
    int set_lock(int blocking);

    template <typename Type> inline Type &get(void) const;

#if VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
    Instance &get_instance(void) const { return *static_cast<Instance *>(instance); }
#else
    Instance &get_instance(void) const { return *reinterpret_cast<Instance *>(&instance_raw); }
#endif
};

} // namespace vc

#endif /* CORE_MUTEX_HPP */

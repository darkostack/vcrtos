#include "core/instance.hpp"
#include "core/code_utils.hpp"
#include "core/thread.hpp"
#include "core/new.hpp"

namespace vc {

#if !VCRTOS_CONFIG_MULTIPLE_INSTANCE_ENABLE
DEFINE_ALIGNED_VAR(instance_raw, sizeof(Instance), uint64_t);
#endif

Instance::Instance(void)
    : initialized(false)
    , thread_scheduler()
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

    VERIFY_OR_EXIT(instance->is_initialized == false);

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

#ifndef UNITTEST
#if VCRTOS_CONFIG_THREAD_EVENT_ENABLE
char event_queue_stack[THREAD_EVENT_STACK_SIZE];

extern "C" void *thread_event_handler(void *arg)
{
    Instance *instance = static_cast<Instance *>(arg);

    instance->get<ThreadScheduler>().event_claim();

    instance->get<ThreadScheduler>().event_loop();

    /* should not reach here */

    return NULL;
}
#endif
#endif

void Instance::after_init(void)
{
#ifndef UNITTEST
#if VCRTOS_CONFIG_THREAD_EVENT_ENABLE
    Thread::init(*this, event_queue_stack, ARRAY_LENGTH(event_queue_stack),
                 THREAD_EVENT_PRIORITY, 0, thread_event_handler,
                 static_cast<void *>(this), "event");
#endif
#endif

    initialized = true;
}

} // namespace vc

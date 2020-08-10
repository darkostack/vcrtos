#include <vcrtos/thread.h>

#include "core/instance.hpp"
#include "core/thread.hpp"

using namespace vc;

kernel_pid_t thread_create(void *instance,
                           char *stack,
                           int size,
                           char priority,
                           int flags,
                           thread_handler_func_t func,
                           void *arg,
                           const char *name)
{
    Instance &instances = *static_cast<Instance *>(instance);
    Thread *thread = Thread::init(instances, stack, size, priority, flags, func, arg, name);
    return thread->get_pid();
}

void thread_scheduler_run(void *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    instances.get<ThreadScheduler>().run();
}

void thread_exit(void *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    instances.get<ThreadScheduler>().exit_current_active_thread();
}

int thread_pid_is_valid(kernel_pid_t pid)
{
    return Thread::is_pid_valid(pid);
}

void thread_yield(void *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    instances.get<ThreadScheduler>().yield();
}

thread_t *thread_current(void *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    Thread *thread = instances.get<ThreadScheduler>().get_current_active_thread();
    return static_cast<thread_t *>(thread);
}

kernel_pid_t thread_current_pid(void *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    return instances.get<ThreadScheduler>().get_current_active_pid();
}

thread_t *thread_get_from_scheduler(void *instance, kernel_pid_t pid)
{
    Instance &instances = *static_cast<Instance *>(instance);
    Thread *thread = instances.get<ThreadScheduler>().get_thread_from_scheduler(pid);
    return static_cast<thread_t *>(thread);
}

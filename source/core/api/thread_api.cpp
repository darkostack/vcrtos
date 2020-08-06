#include <vcrtos/thread.h>

#include "core/thread.hpp"

using namespace vc;

kernel_pid_t thread_create(instance_t *instance,
                           char *stack,
                           int stack_size,
                           char priority,
                           int flags,
                           thread_handler_func_t func,
                           void *arg,
                           const char *name)
{
    Thread *thread = Thread::init(*instance, stack, stack_size, priority, flags, func, arg, name);
    return thread->get_pid();
}

void thread_scheduler_run(instance_t *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    instances.get<ThreadScheduler>().run();
}

void thread_exit(instance_t *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    instances.get<ThreadScheduler>().exit_current_active_thread();
}

int thread_pid_is_valid(kernel_pid_t pid)
{
    return Thread::is_pid_valid(pid);
}

void thread_yield(instance_t *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    instances.get<ThreadScheduler>().yield();
}

thread_t *thread_current(instance_t *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    Thread *thread = instances.get<ThreadScheduler>().get_current_active_thread();
    return static_cast<thread_t *>(thread);
}

kernel_pid_t thread_current_pid(instance_t *instance)
{
    Instance &instances = *static_cast<Instance *>(instance);
    return instances.get<ThreadScheduler>().get_current_active_pid();
}

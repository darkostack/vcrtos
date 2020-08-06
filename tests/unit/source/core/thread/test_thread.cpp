#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.hpp"
#include "core/mutex.hpp"

#include "test-helper.h"

using namespace vc;

class TestThread : public testing::Test
{
};

TEST_F(TestThread, single_thread_test)
{
    DEFINE_ALIGNED_VAR(buffer, sizeof(Instance), uint64_t);

    uint32_t size = ARRAY_LENGTH(buffer);

    Instance instance = Instance::init((void *)buffer, (size_t *)&size);

    EXPECT_TRUE(instance.is_initialized());

    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] make sure Thread class size is correct
     * -------------------------------------------------------------------------
     **/

    EXPECT_EQ(sizeof(Thread), sizeof(thread_t));

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create single thread and run the thread scheduler, that
     * thread is expected to be in running state and become current active
     * thread
     * -------------------------------------------------------------------------
     **/

    char stack[128];

    Thread *thread = Thread::init(instance, stack, sizeof(stack), 15,
                                  THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                  NULL, NULL, "idle");

    EXPECT_NE(thread, nullptr);

    EXPECT_EQ(thread->get_pid(), 1);
    EXPECT_EQ(thread->get_priority(), 15);
    EXPECT_EQ(thread->get_name(), "idle");
    EXPECT_EQ(thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 1);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(thread->get_pid()), thread);
    EXPECT_FALSE(instance.get<ThreadScheduler>().is_context_switch_requested());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(thread->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 1);
}

TEST_F(TestThread, multiple_thread_test)
{
    DEFINE_ALIGNED_VAR(buffer, sizeof(Instance), uint64_t);

    uint32_t size = ARRAY_LENGTH(buffer);

    Instance instance = Instance::init((void *)buffer, (size_t *)&size);

    EXPECT_TRUE(instance.is_initialized());

    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create multiple thread ("idle" and "main" thread) and make
     * sure the thread with higher priority will be in running state and the
     * thread with lower priority ("idle" thread) is in pending state
     * -------------------------------------------------------------------------
     **/

    char idle_stack[128];

    Thread *idle_thread = Thread::init(instance, idle_stack, sizeof(idle_stack), 15,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "idle");

    EXPECT_NE(idle_thread, nullptr);

    EXPECT_EQ(idle_thread->get_pid(), 1);
    EXPECT_EQ(idle_thread->get_priority(), 15);
    EXPECT_EQ(idle_thread->get_name(), "idle");
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    char main_stack[128];

    Thread *main_thread = Thread::init(instance, main_stack, sizeof(main_stack), 7,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "main");

    EXPECT_NE(main_thread, nullptr);

    EXPECT_EQ(main_thread->get_pid(), 2);
    EXPECT_EQ(main_thread->get_priority(), 7);
    EXPECT_EQ(main_thread->get_name(), "main");
    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(idle_thread->get_pid()), idle_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(main_thread->get_pid()), main_thread);
    EXPECT_FALSE(instance.get<ThreadScheduler>().is_context_switch_requested());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), main_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), main_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);

    /* Note: at this point "main" thread is in running state and "idle" thread
     * is in pending state */

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] set the higher priority thread ("main" thread) to blocked
     * state and lower priority thread ("idle" thread) should be in in running
     * state
     * -------------------------------------------------------------------------
     **/

    instance.get<ThreadScheduler>().set_thread_status(main_thread, THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), main_thread);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), idle_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), idle_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);

    /* Note: at this point "main" thread is in blocked state and "idle" thread
     * is in blocked state */

    /**
     * -----------------------------------------------------------------------------
     * [TEST CASE] create new thread with higher priority than main and idle thread
     * and yield immediately
     * -----------------------------------------------------------------------------
     **/

    char task1_stack[128];

    Thread *task1_thread = Thread::init(instance, task1_stack, sizeof(task1_stack), 5,
                                        THREAD_FLAGS_CREATE_STACKMARKER,
                                        NULL, NULL, "task1");

    EXPECT_NE(task1_thread, nullptr);

    EXPECT_EQ(task1_thread->get_pid(), 3);
    EXPECT_EQ(task1_thread->get_priority(), 5);
    EXPECT_EQ(task1_thread->get_name(), "task1");
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);

    /* Note: at this point cpu should immediately yield the "task1" by
     * triggering the PendSV interrupt and context switch from Isr is not requested */

    EXPECT_TRUE(test_helper_is_pendsv_interrupt_triggered());
    EXPECT_FALSE(instance.get<ThreadScheduler>().is_context_switch_requested());

    instance.get<ThreadScheduler>().run();

    /* Note: after run the scheduler current active thread is expected to be
     * "task1" */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), task1_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), task1_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 3);

    instance.get<ThreadScheduler>().yield(); /* this function will run higher priority thread */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    /* task1_thread already the highest priority thread currently running, so
     * nothing change */

    EXPECT_EQ(ThreadScheduler::thread_status_to_string(main_thread->get_status()), "bl rx");
    EXPECT_EQ(ThreadScheduler::thread_status_to_string(idle_thread->get_status()), "pending");
    EXPECT_EQ(ThreadScheduler::thread_status_to_string(task1_thread->get_status()), "running");

    EXPECT_EQ(ThreadScheduler::thread_status_to_string(THREAD_STATUS_STOPPED), "stopped");
    EXPECT_EQ(ThreadScheduler::thread_status_to_string(THREAD_STATUS_SLEEPING), "sleeping");
    EXPECT_EQ(ThreadScheduler::thread_status_to_string(THREAD_STATUS_MUTEX_BLOCKED), "bl mutex");
    EXPECT_EQ(ThreadScheduler::thread_status_to_string(THREAD_STATUS_SEND_BLOCKED), "bl send");
    EXPECT_EQ(ThreadScheduler::thread_status_to_string(THREAD_STATUS_REPLY_BLOCKED), "bl reply");
    EXPECT_EQ(ThreadScheduler::thread_status_to_string(static_cast<thread_status_t>(100)), "unknown");

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] mutexes
     * -------------------------------------------------------------------------
     **/

    Mutex mutex = Mutex(instance);

    mutex.lock();

    /* Note: mutex was unlocked, set to locked for the first time,
     * current active thread (task1_thread) will not change it's status until
     * second time `mutex.lock()` is call */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    mutex.lock();

    /* Note: mutex was locked, calling `mutex.lock()` for the second time in current active
     * thread (task1_thread), now it will set current active thread status to mutex_blocked and
     * yield to higher priority thread or remain thread */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex.unlock();

    /* Note: this will unlock `task1_thread` and set to pending status and yield
     * to highest priority task */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    /* Note: set main_thread status to pending */

    instance.get<ThreadScheduler>().set_thread_status(main_thread, THREAD_STATUS_PENDING);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    mutex.lock(); // call this in task1_thread : HEAD

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex.lock(); // call this in main_thread

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    /* Note: at this point both main_thread and task1_thread is in MUTEX LOCKED
     * status. */

    mutex.unlock();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);

    /* Note: task1_thread was in HEAD of the mutex queue, it will unlock this
     * task first. */

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    /* Note: task1_thread have higher priority than idle_thread, so run
     * task1_thread. */

    mutex.unlock();

    /* This will change main_thread status to pending */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    /* Note: task1_thread has higher priority than main_thread, so keep running
     * task1_thread. */

    mutex.unlock();

    /* No thread was waiting this mutex, set to NULL */

    EXPECT_EQ(mutex.queue.next, nullptr);

    /* Note: set main_thread status back to receive blocked */

    instance.get<ThreadScheduler>().set_thread_status(main_thread, THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try to set current active thread to sleep and wakeup
     * -------------------------------------------------------------------------
     **/

    instance.get<ThreadScheduler>().sleeping_current_thread();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);

    /* Note: at this point both main_thread and task1_thread are in blocking
     * status, so the next expected thread to run is idle_thread because
     * idle_thread was in pending status. */

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), idle_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), idle_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 3);

    EXPECT_EQ(instance.get<ThreadScheduler>().wakeup_thread(main_thread->get_pid()), THREAD_STATUS_NOT_FOUND);

    /* Note: because main_thread is not is SLEEPING status, we expect nothing to
     * happen. */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);

    /* Note: now we wakeup task1_thread which was in sleeping status */

    EXPECT_EQ(instance.get<ThreadScheduler>().wakeup_thread(task1_thread->get_pid()), true);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    /* Note: note that both idle_thread and task1_thread is in running status,
     * that's okay, after calling ThreadScheduler::run() it will fixed threads
     * status and run highest priority thread. In this case we expect idle_thread
     * should be in PENDING status and task1_thread in RUNNING status */

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try to set sleep in Isr
     * -------------------------------------------------------------------------
     **/

    test_helper_set_cpu_in_isr(); // artificially set CPU in Isr

    instance.get<ThreadScheduler>().sleeping_current_thread();

    /* Note: we can't sleep in Isr functions so nothing is expected to happen */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), task1_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), task1_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 3);

    test_helper_reset_cpu_in_isr();

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try to context swithing to lower priority thread than current
     * running thread
     * -------------------------------------------------------------------------
     **/

    test_helper_reset_pendsv_trigger(); /* reset the PendSV state */

    EXPECT_FALSE(test_helper_is_pendsv_interrupt_triggered());

    EXPECT_EQ(idle_thread->get_priority(), 15);
    EXPECT_EQ(main_thread->get_priority(), 7);
    EXPECT_EQ(task1_thread->get_priority(), 5); /* highest priority thread */

    instance.get<ThreadScheduler>().context_switch(main_thread->get_priority());

    /* Note: because "main_thread" priority is lower than current running thread
     * and current running thread is still in running status, nothing should
     * happened */

    EXPECT_FALSE(test_helper_is_pendsv_interrupt_triggered());

    EXPECT_FALSE(instance.get<ThreadScheduler>().is_context_switch_requested());

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), task1_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), task1_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 3);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] request context swicth inside Isr (Interrupt Service Routine) and
     * current running thread is not in running status, it expected to see context
     * switch is requested instead of yielding immediately to the next thread
     * ------------------------------------------------------------------------------
     **/

    /* Note: set "task1" at blocked state first and is expected to be the next thread to
     * run is "idle" thread */

    instance.get<ThreadScheduler>().set_thread_status(task1_thread, THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), idle_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), idle_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 3);

    /* Note: at this point idle thread is run as expected, because other
     * higher priority threads is in blocked state */

    test_helper_set_cpu_in_isr(); /* artificially set cpu in Isr */

    EXPECT_TRUE(cpu_is_in_isr());

    instance.get<ThreadScheduler>().context_switch(task1_thread->get_priority());

    EXPECT_FALSE(test_helper_is_pendsv_interrupt_triggered());

    EXPECT_TRUE(instance.get<ThreadScheduler>().is_context_switch_requested());

    /* Note: because it is in Isr at this point context switch is requested
     * instead of immediatelly yield to "task1" */

    /* Note: in real cpu implementation, before exiting the Isr function it will
     * check this flag and trigger the PendSV interrupt if context switch is
     * requested, therefore after exiting Isr function PendSV interrupt will be
     * triggered and run thread scheduler */

    cpu_end_of_isr((instance_t *)&instance);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);

    test_helper_reset_cpu_in_isr(); /* artificially out from Isr */

    EXPECT_FALSE(cpu_is_in_isr());

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);

    /* Note: at this point the current active thread is still "idle" because "task1"
     * is still in receive blocked state even though it was try to context
     * switch to "task1", now try set "task1" to pending state and
     * context switch to "task1" priority */

    instance.get<ThreadScheduler>().set_thread_status(task1_thread, THREAD_STATUS_PENDING);

    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().context_switch(task1_thread->get_priority());

    EXPECT_TRUE(test_helper_is_pendsv_interrupt_triggered());

    EXPECT_FALSE(instance.get<ThreadScheduler>().is_context_switch_requested());

    instance.get<ThreadScheduler>().run();

    test_helper_reset_pendsv_trigger(); /* artificially reset PendSV state */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);

    /* Note: at this point it succesfully switched to "task1" */

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), task1_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), task1_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 3);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] create a thread with highest priority but with THREAD_FLAGS
     * sleeping and not using THREAD_FLAGS create stack marker.
     * ------------------------------------------------------------------------------
     **/

    char task2_stack[128];

    /* Note: intentionally create thread with misaligment stack boundary on
     * 16/32 bit boundary (&task2_stack[1]) will do the job */

    Thread *task2_thread = Thread::init(instance, &task2_stack[1], sizeof(task2_stack), 1,
                                        THREAD_FLAGS_CREATE_SLEEPING,
                                        NULL, NULL, "task2");

    EXPECT_NE(task2_thread, nullptr);

    EXPECT_EQ(task2_thread->get_pid(), 4);
    EXPECT_EQ(task2_thread->get_priority(), 1);
    EXPECT_EQ(task2_thread->get_name(), "task2");
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_SLEEPING);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_SLEEPING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_SLEEPING);

    instance.get<ThreadScheduler>().wakeup_thread(task2_thread->get_pid());

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);
}

TEST_F(TestThread, multiple_instance_multithreads_test)
{
    DEFINE_ALIGNED_VAR(buffer1, sizeof(Instance), uint64_t);
    DEFINE_ALIGNED_VAR(buffer2, sizeof(Instance), uint64_t);

    uint32_t size1 = ARRAY_LENGTH(buffer1);
    uint32_t size2 = ARRAY_LENGTH(buffer2);

    Instance instance1 = Instance::init((void *)buffer1, (size_t *)&size1);
    Instance instance2 = Instance::init((void *)buffer2, (size_t *)&size2);

    EXPECT_TRUE(instance1.is_initialized());
    EXPECT_TRUE(instance2.is_initialized());

    EXPECT_EQ(instance1.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance1.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance1.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    EXPECT_EQ(instance2.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create multiple instances and multiple threads in those
     * instances
     * -------------------------------------------------------------------------
     **/

    /* Note: create "thread1" and "thread2" in instance1 */

    char instance1_stack1[128];
    char instance1_stack2[128];

    Thread *instance1_thread1 = Thread::init(instance1, instance1_stack1, sizeof(instance1_stack1), 15,
                                             THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                             NULL, NULL, "thread1");

    Thread *instance1_thread2 = Thread::init(instance1, instance1_stack2, sizeof(instance1_stack2), 15,
                                             THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                             NULL, NULL, "thread2");

    EXPECT_NE(instance1_thread1, nullptr);
    EXPECT_NE(instance1_thread2, nullptr);

    EXPECT_EQ(instance1_thread1->get_pid(), 1);
    EXPECT_EQ(instance1_thread1->get_priority(), 15);
    EXPECT_EQ(instance1_thread1->get_name(), "thread1");
    EXPECT_EQ(instance1_thread1->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance1_thread2->get_pid(), 2);
    EXPECT_EQ(instance1_thread2->get_priority(), 15);
    EXPECT_EQ(instance1_thread2->get_name(), "thread2");
    EXPECT_EQ(instance1_thread2->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance1.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);
    EXPECT_EQ(instance1.get<ThreadScheduler>().get_thread_from_scheduler(1), instance1_thread1);
    EXPECT_EQ(instance1.get<ThreadScheduler>().get_thread_from_scheduler(2), instance1_thread2);
    EXPECT_FALSE(instance1.get<ThreadScheduler>().is_context_switch_requested());
    EXPECT_EQ(instance1.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance1.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    instance1.get<ThreadScheduler>().run();

    EXPECT_EQ(instance1_thread1->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(instance1_thread2->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance1.get<ThreadScheduler>().get_current_active_thread(), instance1_thread1);
    EXPECT_EQ(instance1.get<ThreadScheduler>().get_current_active_pid(), instance1_thread1->get_pid());
    EXPECT_EQ(instance1.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);

    /* Note: create "thread1" and "thread2" in instance2 */

    char instance2_stack1[128];
    char instance2_stack2[128];

    Thread *instance2_thread1 = Thread::init(instance2, instance2_stack1, sizeof(instance2_stack1), 15,
                                             THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                             NULL, NULL, "thread1");

    Thread *instance2_thread2 = Thread::init(instance2, instance2_stack2, sizeof(instance2_stack2), 15,
                                             THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                             NULL, NULL, "thread2");

    EXPECT_NE(instance2_thread1, nullptr);
    EXPECT_NE(instance2_thread2, nullptr);

    EXPECT_EQ(instance2_thread1->get_pid(), 1);
    EXPECT_EQ(instance2_thread1->get_priority(), 15);
    EXPECT_EQ(instance2_thread1->get_name(), "thread1");
    EXPECT_EQ(instance2_thread1->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance2_thread2->get_pid(), 2);
    EXPECT_EQ(instance2_thread2->get_priority(), 15);
    EXPECT_EQ(instance2_thread2->get_name(), "thread2");
    EXPECT_EQ(instance2_thread2->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance2.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_thread_from_scheduler(1), instance2_thread1);
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_thread_from_scheduler(2), instance2_thread2);
    EXPECT_FALSE(instance2.get<ThreadScheduler>().is_context_switch_requested());
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    instance2.get<ThreadScheduler>().run();

    EXPECT_EQ(instance2_thread1->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(instance2_thread2->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance2.get<ThreadScheduler>().get_current_active_thread(), instance2_thread1);
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_current_active_pid(), instance2_thread1->get_pid());
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] set "thread2" in instance2 to pending state, since "thread1" and
     * "thread2" have the same priority all of those thread will not run
     * unless one of those thread in blocked state and expected to run the thread
     * with the pending state or status
     * ------------------------------------------------------------------------------
     **/

    instance2.get<ThreadScheduler>().set_thread_status(instance2_thread1, THREAD_STATUS_PENDING);

    EXPECT_EQ(instance2_thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(instance2_thread2->get_status(), THREAD_STATUS_PENDING);

    instance2.get<ThreadScheduler>().run();

    EXPECT_EQ(instance2_thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(instance2_thread2->get_status(), THREAD_STATUS_PENDING);

    instance2.get<ThreadScheduler>().run();

    EXPECT_EQ(instance2_thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(instance2_thread2->get_status(), THREAD_STATUS_PENDING);

    instance2.get<ThreadScheduler>().run();

    EXPECT_EQ(instance2_thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(instance2_thread2->get_status(), THREAD_STATUS_PENDING);

    instance2.get<ThreadScheduler>().set_thread_status(instance2_thread1, THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(instance2_thread1->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(instance2_thread2->get_status(), THREAD_STATUS_PENDING);

    instance2.get<ThreadScheduler>().run();

    EXPECT_EQ(instance2_thread1->get_status(), THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(instance2_thread2->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance2.get<ThreadScheduler>().get_current_active_thread(), instance2_thread2);
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_current_active_pid(), instance2_thread2->get_pid());
    EXPECT_EQ(instance2.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);
}

TEST_F(TestThread, thread_flags_test)
{
    DEFINE_ALIGNED_VAR(buffer, sizeof(Instance), uint64_t);

    uint32_t size = ARRAY_LENGTH(buffer);

    Instance instance = Instance::init((void *)buffer, (size_t *)&size);

    EXPECT_TRUE(instance.is_initialized());

    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    char idle_stack[128];

    Thread *idle_thread = Thread::init(instance, idle_stack, sizeof(idle_stack), 15,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "idle");

    EXPECT_NE(idle_thread, nullptr);

    EXPECT_EQ(idle_thread->get_pid(), 1);
    EXPECT_EQ(idle_thread->get_priority(), 15);
    EXPECT_EQ(idle_thread->get_name(), "idle");
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    char main_stack[128];

    Thread *main_thread = Thread::init(instance, main_stack, sizeof(main_stack), 7,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "main");

    EXPECT_NE(main_thread, nullptr);

    EXPECT_EQ(main_thread->get_pid(), 2);
    EXPECT_EQ(main_thread->get_priority(), 7);
    EXPECT_EQ(main_thread->get_name(), "main");
    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(idle_thread->get_pid()), idle_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(main_thread->get_pid()), main_thread);
    EXPECT_FALSE(instance.get<ThreadScheduler>().is_context_switch_requested());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), main_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), main_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] thread flags wait any
     * ------------------------------------------------------------------------------
     **/

    instance.get<ThreadScheduler>().thread_flags_wait_any(0xf);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(main_thread->flags, 0);
    EXPECT_EQ(main_thread->waited_flags, 0xf);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x1);

    /* any flag set to thread flags will immediately change the thread status to
     * pending status */

    EXPECT_EQ(main_thread->flags, 0x1);
    EXPECT_EQ(main_thread->waited_flags, 0xf);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().thread_flags_clear(0xf);

    /* at this point we already clear thread->flags so calling
     * thread_flags_wait_any will make the current thread in FLAG_BLOCKED_ANY
     * status */

    EXPECT_EQ(main_thread->flags, 0);
    EXPECT_EQ(main_thread->waited_flags, 0xf);

    instance.get<ThreadScheduler>().thread_flags_wait_any(0xf);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(main_thread->flags, 0);
    EXPECT_EQ(main_thread->waited_flags, 0xf);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x3);

    /* now we use different flag to unlocked main_thread, any flag will unlocked
     * main_thread */

    EXPECT_EQ(main_thread->flags, 0x3);
    EXPECT_EQ(main_thread->waited_flags, 0xf);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(main_thread->flags, 0x3);
    EXPECT_EQ(main_thread->waited_flags, 0xf);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] thread flags wait all
     * ------------------------------------------------------------------------------
     **/

    instance.get<ThreadScheduler>().thread_flags_wait_all(0xff);

    /* note: wait all will clear the previous main_thread flags */

    EXPECT_EQ(main_thread->flags, 0);
    EXPECT_EQ(main_thread->waited_flags, 0xff);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ALL);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ALL);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x1);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x2);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x4);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x8);

    /* the thread should remain in FLAG_BLOCKED_ALL status until all flags is
     * filled */

    EXPECT_EQ(main_thread->flags, 0xf);
    EXPECT_EQ(main_thread->waited_flags, 0xff);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ALL);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x10);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x20);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x40);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x80);

    /* at this point all flags is filled */

    EXPECT_EQ(main_thread->flags, 0xff);
    EXPECT_EQ(main_thread->waited_flags, 0xff);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().thread_flags_clear(0xf);

    /* clear half of the flags in main_thread->flags */

    EXPECT_EQ(main_thread->flags, 0xf0);
    EXPECT_EQ(main_thread->waited_flags, 0xff);

    instance.get<ThreadScheduler>().thread_flags_wait_all(0xff);

    /* this function will clear all the remain flags in main_thread->flags to
     * wait all flags to be received again and set to FLAG_BLOCKED_ALL status */

    EXPECT_EQ(main_thread->flags, 0);
    EXPECT_EQ(main_thread->waited_flags, 0xff);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ALL);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ALL);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x1);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x2);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x4);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x8);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x10);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x20);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x40);
    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x80);

    EXPECT_EQ(main_thread->flags, 0xff);
    EXPECT_EQ(main_thread->waited_flags, 0xff);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->flags, 0xff);
    EXPECT_EQ(main_thread->waited_flags, 0xff);

    instance.get<ThreadScheduler>().thread_flags_wait_all(0xff); /* this will take no effect */

    /* Note: previous main_thread->flags is 0xff and calling this function again
     * will assume that the flags has been filled and clear the
     * main_thread->flags and will not blocked main_thread. to make
     * thread_flags_wait_all immediately taking effect we must clear the
     * main_thread->flags first. */

    EXPECT_EQ(main_thread->flags, 0x0);
    EXPECT_EQ(main_thread->waited_flags, 0xff);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    /* main_thread->flags is cleared but main_thread is still in running status */

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] thread flags wait one
     * ------------------------------------------------------------------------------
     **/

    EXPECT_EQ(main_thread->flags, 0x0);
    EXPECT_EQ(main_thread->waited_flags, 0xff);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().thread_flags_wait_one(0x3);

    EXPECT_EQ(main_thread->flags, 0x0);
    EXPECT_EQ(main_thread->waited_flags, 0x3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x10);

    /* Note: we set the flag that is not being wait */

    EXPECT_EQ(main_thread->flags, 0x10);
    EXPECT_EQ(main_thread->waited_flags, 0x3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x1);

    EXPECT_EQ(main_thread->flags, 0x11);
    EXPECT_EQ(main_thread->waited_flags, 0x3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->flags, 0x11);
    EXPECT_EQ(main_thread->waited_flags, 0x3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().thread_flags_wait_one(0x3);

    /* previous flags is not being cleared, calling this function will assume
     * the flags has been filled and clear the flags corresponding to waited_one 
     * mask*/

    EXPECT_EQ(main_thread->flags, 0x10);
    EXPECT_EQ(main_thread->waited_flags, 0x3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().thread_flags_wait_one(0x3);

    /* now it will blocked main_thread */

    EXPECT_EQ(main_thread->flags, 0x10);
    EXPECT_EQ(main_thread->waited_flags, 0x3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().thread_flags_set(main_thread, 0x2);

    EXPECT_EQ(main_thread->flags, 0x12);
    EXPECT_EQ(main_thread->waited_flags, 0x3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(main_thread->flags, 0x12);
    EXPECT_EQ(main_thread->waited_flags, 0x3);

    instance.get<ThreadScheduler>().thread_flags_clear(0xff);

    /* clear main_thread (current thread) flags */

    EXPECT_EQ(main_thread->flags, 0);
    EXPECT_EQ(main_thread->waited_flags, 0x3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
}


uint32_t event1_handler_counter = 0;
uint32_t event2_handler_counter = 0;
uint32_t event3_handler_counter = 0;
uint32_t event4_handler_counter = 0;
uint32_t event5_handler_counter = 0;

extern "C" void _event1_handler(event_t *ev)
{
    event1_handler_counter += 1;
}

extern "C" void _event2_handler(event_t *ev)
{
    event2_handler_counter += 1;
}

extern "C" void _event3_handler(event_t *ev)
{
    event3_handler_counter += 1;
}

extern "C" void _event4_handler(event_t *ev)
{
    event4_handler_counter += 1;
}

extern "C" void _event5_handler(event_t *ev)
{
    event5_handler_counter += 1;
}

TEST_F(TestThread, thread_event_test)
{
    DEFINE_ALIGNED_VAR(buffer, sizeof(Instance), uint64_t);

    uint32_t size = ARRAY_LENGTH(buffer);

    Instance instance = Instance::init((void *)buffer, (size_t *)&size);

    EXPECT_TRUE(instance.is_initialized());

    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    char idle_stack[128];

    Thread *idle_thread = Thread::init(instance, idle_stack, sizeof(idle_stack), 15,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "idle");

    EXPECT_NE(idle_thread, nullptr);

    EXPECT_EQ(idle_thread->get_pid(), 1);
    EXPECT_EQ(idle_thread->get_priority(), 15);
    EXPECT_EQ(idle_thread->get_name(), "idle");
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    char main_stack[128];

    Thread *main_thread = Thread::init(instance, main_stack, sizeof(main_stack), 7,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "main");

    EXPECT_NE(main_thread, nullptr);

    EXPECT_EQ(main_thread->get_pid(), 2);
    EXPECT_EQ(main_thread->get_priority(), 7);
    EXPECT_EQ(main_thread->get_name(), "main");
    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(idle_thread->get_pid()), idle_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(main_thread->get_pid()), main_thread);
    EXPECT_FALSE(instance.get<ThreadScheduler>().is_context_switch_requested());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_thread(), main_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_current_active_pid(), main_thread->get_pid());
    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] create thread event
     * ------------------------------------------------------------------------------
     **/

    char event_stack[128];

    Thread *event_thread = Thread::init(instance, event_stack, sizeof(event_stack), 5,
                                        THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                        NULL, NULL, "event");

    EXPECT_NE(event_thread, nullptr);

    EXPECT_EQ(event_thread->get_pid(), 3);
    EXPECT_EQ(event_thread->get_priority(), 5);
    EXPECT_EQ(event_thread->get_name(), "event");
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance.get<ThreadScheduler>().get_numof_threads_in_scheduler(), 3);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(idle_thread->get_pid()), idle_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(main_thread->get_pid()), main_thread);
    EXPECT_EQ(instance.get<ThreadScheduler>().get_thread_from_scheduler(event_thread->get_pid()), event_thread);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_RUNNING);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] put thread event in loop to wait new event
     * ------------------------------------------------------------------------------
     **/

    instance.get<ThreadScheduler>().event_claim();
    instance.get<ThreadScheduler>().event_loop();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    /* Note: now event_thread is already in FLAG_BLOCKED_ANY status to wait any
     * event to be received */

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] post event
     * ------------------------------------------------------------------------------
     **/

    Event event1 = Event(_event1_handler);

    instance.get<ThreadScheduler>().event_post(&event1);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().event_loop();

    EXPECT_EQ(event1_handler_counter, 1);

    /* Note: at this point event is succesfully received and event handler was
     * called to increase the event_handler_counter */

    Event *ev = instance.get<ThreadScheduler>().event_wait();

    /* Note: there is no event in the queue */

    EXPECT_EQ(ev, nullptr);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().event_loop();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().event_post(&event1);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().event_loop();

    EXPECT_EQ(event1_handler_counter, 2);

    EXPECT_NE(event_thread->flags, 0);

    instance.get<ThreadScheduler>().event_loop();

    /* Note: at this point nothing expected to be happen, because
     * event_thread->flags is not clear it before. It will clear the
     * event_thread->flags first, before set event_thread to FLAG_BLOCKED_ANY */

    EXPECT_EQ(event1_handler_counter, 2); /* this should not increase */

    EXPECT_EQ(event_thread->flags, 0);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().event_loop();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().event_post(&event1);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().event_loop();

    EXPECT_EQ(event1_handler_counter, 3);

    EXPECT_NE(event_thread->flags, 0);

    /* Note: try to clear event thread flags first before calling event_loop() */

    instance.get<ThreadScheduler>().thread_flags_clear(0xffff);

    EXPECT_EQ(event_thread->flags, 0);

    instance.get<ThreadScheduler>().event_loop();

    /* Note: calling event_loop() now will effective immediately because
     * THREAD_FLAG_EVENT was cleared */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().event_post(&event1);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_RUNNING);

    ev = instance.get<ThreadScheduler>().event_get();

    EXPECT_NE(ev, nullptr);

    EXPECT_EQ(ev, &event1);

    ev->handler(ev); // call event handler

    EXPECT_EQ(event1_handler_counter, 4);

    instance.get<ThreadScheduler>().event_loop(); /* it will clear the THREAD_FLAG_EVENT first */
    instance.get<ThreadScheduler>().event_loop();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] post multiple event
     * ------------------------------------------------------------------------------
     **/

    Event event2 = Event(_event2_handler);
    Event event3 = Event(_event3_handler);
    Event event4 = Event(_event4_handler);
    Event event5 = Event(_event5_handler);

    instance.get<ThreadScheduler>().event_post(&event1);
    instance.get<ThreadScheduler>().event_post(&event2);
    instance.get<ThreadScheduler>().event_post(&event3);
    instance.get<ThreadScheduler>().event_post(&event4);
    instance.get<ThreadScheduler>().event_post(&event5);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    /* cancel event 3 */

    instance.get<ThreadScheduler>().event_cancel(&event3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_RUNNING);

    ev = instance.get<ThreadScheduler>().event_get(); /* get event1 */
    EXPECT_NE(ev, nullptr);
    EXPECT_EQ(ev, &event1);
    ev->handler(ev);
    EXPECT_EQ(event1_handler_counter, 5);

    ev = instance.get<ThreadScheduler>().event_get(); /* get event2 */
    EXPECT_NE(ev, nullptr);
    EXPECT_EQ(ev, &event2);
    ev->handler(ev);
    EXPECT_EQ(event2_handler_counter, 1);

    ev = instance.get<ThreadScheduler>().event_get(); /* get event4, event3 was canceled */
    EXPECT_NE(ev, nullptr);
    EXPECT_EQ(ev, &event4);
    ev->handler(ev);
    EXPECT_EQ(event3_handler_counter, 0);
    EXPECT_EQ(event4_handler_counter, 1);

    instance.get<ThreadScheduler>().event_loop(); /* this will get event5 */

    EXPECT_EQ(event5_handler_counter, 1);

    ev = instance.get<ThreadScheduler>().event_get(); /* no event available */
    EXPECT_EQ(ev, nullptr);

    instance.get<ThreadScheduler>().thread_flags_clear(0xffff);

    instance.get<ThreadScheduler>().event_loop();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] cancel event and post it again
     * ------------------------------------------------------------------------------
     **/

    instance.get<ThreadScheduler>().event_post(&event3);
    instance.get<ThreadScheduler>().event_post(&event4);
    instance.get<ThreadScheduler>().event_post(&event5);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().event_cancel(&event3);
    instance.get<ThreadScheduler>().event_post(&event3);

    /* Note: after canceled and repost again, event3 will at the last queue */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_PENDING);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_RUNNING);

    instance.get<ThreadScheduler>().event_loop(); /* this will get event4 */
    EXPECT_EQ(event4_handler_counter, 2);

    instance.get<ThreadScheduler>().event_loop(); /* this will get event5 */
    EXPECT_EQ(event5_handler_counter, 2);

    instance.get<ThreadScheduler>().event_loop(); /* this will get event3 */
    EXPECT_EQ(event3_handler_counter, 1);

    ev = instance.get<ThreadScheduler>().event_get(); /* no event left */
    EXPECT_EQ(ev, nullptr);

    instance.get<ThreadScheduler>().event_loop();
    instance.get<ThreadScheduler>().event_loop();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);

    instance.get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(event_thread->get_status(), THREAD_STATUS_FLAG_BLOCKED_ANY);
}

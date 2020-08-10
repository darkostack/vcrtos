#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.hpp"
#include "core/mutex.hpp"

#include "test-helper.h"

using namespace vc;

class TestThreadApi : public testing::Test
{
public:
    void instance_reset(void)
    {
        memset(instance, 0, sizeof(Instance));
        instance->after_init();
    }

protected:
    Instance *instance;

    virtual void SetUp()
    {
        instance = new Instance();
    }

    virtual void TearDown()
    {
        delete instance;
    }
};

class TestMultipleInstancesThreadApi: public testing::Test
{
protected:
    Instance *instance1;
    Instance *instance2;

    virtual void SetUp()
    {
        instance1 = new Instance();
        instance2 = new Instance();
    }

    virtual void TearDown()
    {
        delete instance1;
        delete instance2;
    }
};

TEST_F(TestThreadApi, constructor_test)
{
    EXPECT_TRUE(instance);
}

TEST_F(TestMultipleInstancesThreadApi, constructor_test)
{
    EXPECT_TRUE(instance1);
    EXPECT_TRUE(instance2);
}

TEST_F(TestThreadApi, single_thread_test)
{
    EXPECT_TRUE(instance->is_initialized());

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

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

    thread_t *thread;

    kernel_pid_t thread_pid = thread_create(static_cast<void *>(instance), stack, sizeof(stack), 15,
                                            THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                            NULL, NULL, "idle");

    thread = thread_get_from_scheduler(static_cast<void *>(instance), thread_pid);

    EXPECT_EQ(thread_pid_is_valid(thread->pid), 1);

    EXPECT_NE(thread, nullptr);

    EXPECT_EQ(thread->pid, 1);
    EXPECT_EQ(thread->priority, 15);
    EXPECT_EQ(thread->name, "idle");
    EXPECT_EQ(thread->status, THREAD_STATUS_PENDING);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 1);
    EXPECT_FALSE(instance->get<ThreadScheduler>().is_context_switch_requested());

    EXPECT_EQ(thread_current(static_cast<void *>(instance)), nullptr);
    EXPECT_EQ(thread_current_pid(static_cast<void *>(instance)), KERNEL_PID_UNDEF);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    thread_scheduler_run(static_cast<void *>(instance));

    EXPECT_EQ(thread->status, THREAD_STATUS_RUNNING);

    EXPECT_EQ(thread_current(static_cast<void *>(instance)), thread);
    EXPECT_EQ(thread_current_pid(static_cast<void *>(instance)), thread->pid);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), thread->pid);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 1);
}

TEST_F(TestThreadApi, multiple_thread_test)
{
    instance_reset();

    EXPECT_TRUE(instance->is_initialized());

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create multiple thread ("idle" and "main" thread) and make
     * sure the thread with higher priority will be in running state and the
     * thread with lower priority ("idle" thread) is in pending state
     * -------------------------------------------------------------------------
     **/

    char idle_stack[128];

    thread_t *idle_thread;

    kernel_pid_t idle_pid = thread_create(static_cast<void *>(instance), idle_stack, sizeof(idle_stack), 15,
                                          THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                          NULL, NULL, "idle");

    idle_thread = thread_get_from_scheduler(static_cast<void *>(instance), idle_pid);

    EXPECT_EQ(thread_pid_is_valid(idle_thread->pid), 1);

    EXPECT_NE(idle_thread, nullptr);

    EXPECT_EQ(idle_thread->pid, 1);
    EXPECT_EQ(idle_thread->priority, 15);
    EXPECT_EQ(idle_thread->name, "idle");
    EXPECT_EQ(idle_thread->status, THREAD_STATUS_PENDING);

    char main_stack[128];

    thread_t *main_thread;

    kernel_pid_t main_pid = thread_create(static_cast<void *>(instance), main_stack, sizeof(main_stack), 7,
                                          THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                          NULL, NULL, "main");

    main_thread = thread_get_from_scheduler(static_cast<void *>(instance), main_pid);

    EXPECT_NE(main_thread, nullptr);

    EXPECT_EQ(main_thread->pid, 2);
    EXPECT_EQ(main_thread->priority, 7);
    EXPECT_EQ(main_thread->name, "main");
    EXPECT_EQ(main_thread->status, THREAD_STATUS_PENDING);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);
    EXPECT_FALSE(instance->get<ThreadScheduler>().is_context_switch_requested());
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    thread_scheduler_run(static_cast<void *>(instance));

    EXPECT_EQ(main_thread->status, THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->status, THREAD_STATUS_PENDING);

    EXPECT_EQ(thread_current(static_cast<void *>(instance)), main_thread);
    EXPECT_EQ(thread_current_pid(static_cast<void *>(instance)), main_thread->pid);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);

    /* Note: at this point "main" thread is in running state and "idle" thread
     * is in pending state */

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] set the higher priority thread ("main" thread) to blocked
     * state and lower priority thread ("idle" thread) should be in in running
     * state
     * -------------------------------------------------------------------------
     **/

    instance->get<ThreadScheduler>().set_thread_status(static_cast<Thread *>(main_thread),
                                                       THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(main_thread->status, THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->status, THREAD_STATUS_PENDING);

    EXPECT_EQ(thread_current(static_cast<void *>(instance)), main_thread);

    thread_scheduler_run(static_cast<void *>(instance));

    EXPECT_EQ(main_thread->status, THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->status, THREAD_STATUS_RUNNING);

    EXPECT_EQ(thread_current(static_cast<void *>(instance)), idle_thread);
    EXPECT_EQ(thread_current_pid(static_cast<void *>(instance)), idle_thread->pid);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);

    /* Note: at this point "main" thread is in blocked state and "idle" thread
     * is in blocked state */

    /**
     * -----------------------------------------------------------------------------
     * [TEST CASE] create new thread with higher priority than main and idle thread
     * and yield immediately
     * -----------------------------------------------------------------------------
     **/

    char task1_stack[128];

    thread_t *task1_thread;

    kernel_pid_t task1_pid = thread_create(static_cast<void *>(instance), task1_stack, sizeof(task1_stack), 5,
                                           THREAD_FLAGS_CREATE_STACKMARKER,
                                           NULL, NULL, "task1");

    task1_thread = thread_get_from_scheduler(static_cast<void *>(instance), task1_pid);

    EXPECT_EQ(thread_pid_is_valid(task1_thread->pid), 1);

    EXPECT_NE(task1_thread, nullptr);

    EXPECT_EQ(task1_thread->pid, 3);
    EXPECT_EQ(task1_thread->priority, 5);
    EXPECT_EQ(task1_thread->name, "task1");
    EXPECT_EQ(task1_thread->status, THREAD_STATUS_PENDING);

    /* Note: at this point cpu should immediately yield the "task1" by
     * triggering the PendSV interrupt and context switch from Isr is not requested */

    EXPECT_TRUE(test_helper_is_pendsv_interrupt_triggered());
    EXPECT_FALSE(instance->get<ThreadScheduler>().is_context_switch_requested());

    thread_scheduler_run(static_cast<void *>(instance));

    /* Note: after run the scheduler current active thread is expected to be
     * "task1" */

    EXPECT_EQ(main_thread->status, THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->status, THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->status, THREAD_STATUS_RUNNING);

    EXPECT_EQ(thread_current(static_cast<void *>(instance)), task1_thread);
    EXPECT_EQ(thread_current_pid(static_cast<void *>(instance)), task1_thread->pid);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 3);

    thread_yield(static_cast<void *>(instance)); /* this function will run higher priority thread */

    EXPECT_EQ(main_thread->status, THREAD_STATUS_RECEIVE_BLOCKED);
    EXPECT_EQ(idle_thread->status, THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->status, THREAD_STATUS_RUNNING);

    /* task1_thread already the highest priority thread currently running, so
     * nothing change */
}

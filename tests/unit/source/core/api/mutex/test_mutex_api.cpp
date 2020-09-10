/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.h"
#include "core/mutex.hpp"

#include "test-helper.h"

using namespace vc;

class TestMutexApi : public testing::Test
{
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

TEST_F(TestMutexApi, constructor_test)
{
    EXPECT_TRUE(instance);
}

TEST_F(TestMutexApi, single_instance_mutex_test)
{
    /* prepare threads for test */

    EXPECT_TRUE(instance->is_initialized());

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    char idle_stack[128];

    Thread *idle_thread = Thread::init(*instance, idle_stack, sizeof(idle_stack), 15,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "idle");

    EXPECT_NE(idle_thread, nullptr);

    EXPECT_EQ(idle_thread->get_pid(), 1);
    EXPECT_EQ(idle_thread->get_priority(), 15);
    EXPECT_EQ(idle_thread->get_name(), "idle");
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);

    char main_stack[128];

    Thread *main_thread = Thread::init(*instance, main_stack, sizeof(main_stack), 7,
                                       THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                       NULL, NULL, "main");

    EXPECT_NE(main_thread, nullptr);

    EXPECT_EQ(main_thread->get_pid(), 2);
    EXPECT_EQ(main_thread->get_priority(), 7);
    EXPECT_EQ(main_thread->get_name(), "main");
    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);

    char task1_stack[128];

    Thread *task1_thread = Thread::init(*instance, task1_stack, sizeof(task1_stack), 5,
                                        THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                        NULL, NULL, "task1");

    EXPECT_NE(task1_thread, nullptr);

    EXPECT_EQ(task1_thread->get_pid(), 3);
    EXPECT_EQ(task1_thread->get_priority(), 5);
    EXPECT_EQ(task1_thread->get_name(), "task1");
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);

    char task2_stack[128];

    Thread *task2_thread = Thread::init(*instance, task2_stack, sizeof(task1_stack), 5,
                                        THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                        NULL, NULL, "task2");

    EXPECT_NE(task2_thread, nullptr);

    EXPECT_EQ(task2_thread->get_pid(), 4);
    EXPECT_EQ(task2_thread->get_priority(), 5);
    EXPECT_EQ(task2_thread->get_name(), "task2");
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 4);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_thread_from_scheduler(idle_thread->get_pid()), idle_thread);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_thread_from_scheduler(main_thread->get_pid()), main_thread);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_thread_from_scheduler(task1_thread->get_pid()), task1_thread);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_thread_from_scheduler(task2_thread->get_pid()), task2_thread);
    EXPECT_FALSE(instance->get<ThreadScheduler>().is_context_switch_requested());
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    /* Note: because task1_thread created first compare to task2_thread, therefore
     * task1_thread will running first although they have same priority. */

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), task1_thread);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), task1_thread->get_pid());
    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 4);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] make sure Mutex class size is correct
     * -------------------------------------------------------------------------
     **/

    EXPECT_EQ(sizeof(Mutex), sizeof(mutex_t));

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] single mutex
     * -------------------------------------------------------------------------
     **/

    mutex_t mutex;

    mutex_init(static_cast<void *>(instance), &mutex);

    mutex_lock(&mutex);

    /* Note: mutex was unlocked, set to locked for the first time and still
     * running current thread */

    mutex_try_lock(&mutex);
    mutex_try_lock(&mutex);
    mutex_try_lock(&mutex);
    mutex_try_lock(&mutex);

    /* Note: mutex already in locked, so nothing happen when calling try_lock() at this point */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    mutex_lock(&mutex); /* this will blocked task1_thread */

    /* Note: mutex was locked, set current thread status to mutex blocked */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    mutex_lock(&mutex); /* this will blocked task2_thread */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex_unlock(&mutex); /* this will unlocked task1_thread */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex_unlock(&mutex); /* this will unlocked task2_thread */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    /* Note: task1_thread and task2_thread have the same priority so it will keep
     * running task1_thread. */

    mutex_unlock(&mutex); /* no body wait for this mutex, set to null */

    EXPECT_EQ(mutex.queue.next, nullptr);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] multiple mutexes
     * -------------------------------------------------------------------------
     **/

    mutex_t mutex1;
    mutex_t mutex2;
    mutex_t mutex3;

    mutex_init(static_cast<void *>(instance), &mutex1);
    mutex_init(static_cast<void *>(instance), &mutex2);
    mutex_init(static_cast<void *>(instance), &mutex3);

    mutex_lock(&mutex1);
    mutex_lock(&mutex2);
    mutex_lock(&mutex3);

    /* Note: all mutexes was unlocked, now set to locked */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    mutex_lock(&mutex1); /* call this mutex in task1_thread */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    mutex_lock(&mutex2); /* call this mutex in task2_thread */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex_lock(&mutex3); /* call this mutex in main_thread */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex_unlock(&mutex2); /* this will set task2_thread to pending status */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    mutex_unlock(&mutex1); /* this will set task1_thread to pending status but keep running task2_thread */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    mutex_unlock(&mutex3); /* this will set main_thread to pending status */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    mutex_lock(&mutex3); /* this will set task2_thread to mutex blocked status */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex_unlock(&mutex3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] unlock mutex in Isr
     * -------------------------------------------------------------------------
     **/

    mutex_lock(&mutex3); /* this will set task1_thread to mutex blocked status */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    test_helper_set_cpu_in_isr(); // artificially set CPU in Isr

    mutex_unlock(&mutex3);

    test_helper_reset_cpu_in_isr();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] unlock and sleeping current thread
     * -------------------------------------------------------------------------
     **/

    mutex_lock(&mutex3); /* this will set task2_thread to mutex blocked status */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    mutex_unlock_and_sleeping(&mutex3); /* this will unlock task2_thread and set task1_thread to sleeping */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_PENDING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_RUNNING);

    mutex_unlock_and_sleeping(&mutex3);

    /* Note: No thread was blocked by mutex3, nothing to unlocked but keep set
     * the currentThread to sleeping */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_SLEEPING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_SLEEPING);

    mutex_unlock_and_sleeping(&mutex3);

    /* Note: No thread was blocked by mutex3, nothing to unlocked but keep set
     * the currentThread to sleeping */

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_SLEEPING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_SLEEPING);

    mutex_unlock(&mutex3);
    mutex_unlock(&mutex3);
    mutex_unlock(&mutex3);
    mutex_unlock(&mutex3);
    mutex_unlock(&mutex3);
    mutex_unlock(&mutex3);

    EXPECT_EQ(main_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(idle_thread->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(task1_thread->get_status(), THREAD_STATUS_SLEEPING);
    EXPECT_EQ(task2_thread->get_status(), THREAD_STATUS_SLEEPING);

    /* Note: mutex3 was unlocked, nothing happen */
}

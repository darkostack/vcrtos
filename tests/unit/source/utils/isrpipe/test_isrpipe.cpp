#include "gtest/gtest.h"

#include "core/instance.hpp"

#include "utils/isrpipe.hpp"

using namespace vc;
using namespace utils;

class TestUtilsTsrb : public testing::Test
{
protected:
    char buffer[8];

    Tsrb *tsrb;

    virtual void SetUp()
    {
        tsrb = new Tsrb(buffer, sizeof(buffer));
    }

    virtual void TearDown()
    {
        delete tsrb;
    }
};

class TestUtilsUartIsrpipe : public testing::Test
{
protected:
    Instance *instance;
    UartIsrpipe *uart_isrpipe;

    virtual void SetUp()
    {
        instance = new Instance();
        uart_isrpipe = new UartIsrpipe(*instance);
    }

    virtual void TearDown()
    {
        delete instance;
        delete uart_isrpipe;
    }
};

TEST_F(TestUtilsTsrb, constructor_test)
{
    EXPECT_TRUE(tsrb);
}

TEST_F(TestUtilsUartIsrpipe, constructor_test)
{
    EXPECT_TRUE(instance);
    EXPECT_TRUE(uart_isrpipe);
}

TEST_F(TestUtilsTsrb, functions_test)
{
    EXPECT_EQ(tsrb->avail(), 0);
    EXPECT_TRUE(tsrb->is_empty());
    EXPECT_FALSE(tsrb->is_full());
    EXPECT_EQ(tsrb->free(), 8);

    tsrb->add_one(0x1);
    tsrb->add_one(0x2);
    tsrb->add_one(0x3);
    tsrb->add_one(0x4);

    EXPECT_EQ(tsrb->avail(), 4);
    EXPECT_FALSE(tsrb->is_empty());
    EXPECT_FALSE(tsrb->is_full());
    EXPECT_EQ(tsrb->free(), 4);

    tsrb->push(0x5);
    tsrb->push(0x6);
    tsrb->push(0x7);
    tsrb->push(0x8);

    EXPECT_EQ(tsrb->avail(), 8);
    EXPECT_FALSE(tsrb->is_empty());
    EXPECT_TRUE(tsrb->is_full());
    EXPECT_EQ(tsrb->free(), 0);

    EXPECT_EQ(tsrb->pop(), 0x1);
    EXPECT_EQ(tsrb->pop(), 0x2);
    EXPECT_EQ(tsrb->pop(), 0x3);
    EXPECT_EQ(tsrb->pop(), 0x4);

    EXPECT_EQ(tsrb->free(), 4);

    tsrb->drop(2);

    EXPECT_EQ(tsrb->free(), 6);
    EXPECT_EQ(tsrb->avail(), 2);

    EXPECT_EQ(tsrb->get_one(), 0x7);
    EXPECT_EQ(tsrb->get_one(), 0x8);

    EXPECT_EQ(tsrb->avail(), 0);
    EXPECT_TRUE(tsrb->is_empty());
    EXPECT_FALSE(tsrb->is_full());
    EXPECT_EQ(tsrb->free(), 8);

    char data[4] = {(char)0xfa, (char)0xfb, (char)0xfc, (char)0xfd};

    tsrb->add(data, sizeof(data));

    EXPECT_EQ(tsrb->avail(), 4);
    EXPECT_FALSE(tsrb->is_empty());
    EXPECT_FALSE(tsrb->is_full());
    EXPECT_EQ(tsrb->free(), 4);

    char result[4];

    tsrb->get(result, sizeof(result));

    EXPECT_EQ(tsrb->avail(), 0);
    EXPECT_TRUE(tsrb->is_empty());
    EXPECT_FALSE(tsrb->is_full());
    EXPECT_EQ(tsrb->free(), 8);

    EXPECT_EQ(result[0], (char)0xfa);
    EXPECT_EQ(result[1], (char)0xfb);
    EXPECT_EQ(result[2], (char)0xfc);
    EXPECT_EQ(result[3], (char)0xfd);
}

TEST_F(TestUtilsUartIsrpipe, functions_test)
{
    EXPECT_TRUE(instance->is_initialized());

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 0);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    char stack1[128];

    Thread *thread1 = Thread::init(*instance, stack1, sizeof(stack1), 15,
                                   THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                   NULL, NULL, "thread1");

    EXPECT_NE(thread1, nullptr);

    EXPECT_EQ(thread1->get_pid(), 1);
    EXPECT_EQ(thread1->get_priority(), 15);
    EXPECT_EQ(thread1->get_name(), "thread1");
    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 1);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_thread_from_scheduler(thread1->get_pid()), thread1);
    EXPECT_FALSE(instance->get<ThreadScheduler>().is_context_switch_requested());
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), nullptr);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), KERNEL_PID_UNDEF);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), thread1);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), thread1->get_pid());
    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 1);

    char stack2[128];

    Thread *thread2 = Thread::init(*instance, stack2, sizeof(stack2), 14,
                                   THREAD_FLAGS_CREATE_WOUT_YIELD | THREAD_FLAGS_CREATE_STACKMARKER,
                                   NULL, NULL, "thread2");

    EXPECT_NE(thread2, nullptr);

    EXPECT_EQ(thread2->get_pid(), 2);
    EXPECT_EQ(thread2->get_priority(), 14);
    EXPECT_EQ(thread2->get_name(), "thread2");
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_PENDING);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_thread(), thread2);
    EXPECT_EQ(instance->get<ThreadScheduler>().get_current_active_pid(), thread2->get_pid());
    EXPECT_EQ(instance->get<ThreadScheduler>().get_numof_threads_in_scheduler(), 2);

    uart_isrpipe->write_one(0x1);
    uart_isrpipe->write_one(0x2);
    uart_isrpipe->write_one(0x3);

    EXPECT_EQ(uart_isrpipe->get_tsrb().avail(), 3);

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_RUNNING);

    char data;

    uart_isrpipe->read(&data, 1);

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(data, 0x1);

    uart_isrpipe->read(&data, 1);

    EXPECT_EQ(data, 0x2);

    EXPECT_EQ(uart_isrpipe->get_tsrb().avail(), 1);

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_RUNNING);

    uart_isrpipe->read(&data, 1);

    EXPECT_EQ(data, 0x3);

    EXPECT_EQ(uart_isrpipe->get_tsrb().avail(), 0);

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_RUNNING);

    uart_isrpipe->read(&data, 1);

    /* Note: mutex was unlocked, set to locked for the first time and still
     * running current thread */

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_RUNNING);

    uart_isrpipe->read(&data, 1);

    /* Note: second time call read, mutex was locked and no data in buffer
     * it will set current thread to mutex blocked status*/

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_MUTEX_BLOCKED);

    uart_isrpipe->write_one(0xa);

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_PENDING);

    /* Note: we get new data in uart_isrpipe so set thread2 to pending */

    instance->get<ThreadScheduler>().run();

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_RUNNING);

    EXPECT_EQ(uart_isrpipe->get_tsrb().avail(), 1); /* one data available to read */

    uart_isrpipe->read(&data, 1);

    EXPECT_EQ(data, 0xa);

    EXPECT_EQ(thread1->get_status(), THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->get_status(), THREAD_STATUS_RUNNING);
}

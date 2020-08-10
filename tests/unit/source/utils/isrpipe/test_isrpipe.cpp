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
}

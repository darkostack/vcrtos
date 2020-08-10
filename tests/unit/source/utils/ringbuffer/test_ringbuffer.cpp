#include "gtest/gtest.h"

#include "utils/ringbuffer.hpp"

using namespace vc;
using namespace utils;

class TestUtilsRingbuffer : public testing::Test
{
protected:
    char buffer[8];

    RingBuffer *obj;

    virtual void SetUp()
    {
        obj = new RingBuffer(buffer, sizeof(buffer));
    }

    virtual void TearDown()
    {
        delete obj;
    }
};

TEST_F(TestUtilsRingbuffer, constructor_test)
{
    EXPECT_TRUE(obj);
}

TEST_F(TestUtilsRingbuffer, functions_test)
{
    EXPECT_EQ(obj->avail(), 0);

    obj->add_one(0x1);
    obj->add_one(0x2);

    EXPECT_EQ(obj->avail(), 2);

    EXPECT_NE(obj->get_one(), -1); /* this get 0x1 */
    EXPECT_EQ(obj->get_one(), 0x2);

    EXPECT_TRUE(obj->is_empty());
    EXPECT_FALSE(obj->is_full());

    EXPECT_EQ(obj->peek_one(), -1); /* nothing in the buffer */

    obj->add_one(0x1);
    obj->add_one(0x2);
    obj->add_one(0x3);
    obj->add_one(0x4);
    obj->add_one(0x5);
    obj->add_one(0x6);
    obj->add_one(0x7);
    obj->add_one(0x8);

    EXPECT_EQ(obj->avail(), 8);

    EXPECT_FALSE(obj->is_empty());
    EXPECT_TRUE(obj->is_full());

    EXPECT_EQ(obj->add_one(0x9), 0x1); /* this will remove the head (0x1) and add (0x9) to the tail */

    EXPECT_TRUE(obj->is_full());

    EXPECT_EQ(obj->peek_one(), 0x2); /* 0x2 is the head */
    EXPECT_EQ(obj->peek_one(), 0x2); /* 0x2 is the head */
    EXPECT_EQ(obj->peek_one(), 0x2); /* 0x2 is the head */
    EXPECT_EQ(obj->peek_one(), 0x2); /* 0x2 is the head */

    EXPECT_EQ(obj->avail(), 8); /* peek will not reduce the buffer size */

    char peeked[4];

    obj->peek(peeked, sizeof(peeked));

    EXPECT_EQ(obj->avail(), 8);

    EXPECT_EQ(peeked[0], 0x2);
    EXPECT_EQ(peeked[1], 0x3);
    EXPECT_EQ(peeked[2], 0x4);
    EXPECT_EQ(peeked[3], 0x5);

    obj->remove(2); /* 0x2, 0x3 was removed */

    EXPECT_EQ(obj->avail(), 6);

    char gotten[4];

    obj->get(gotten, sizeof(gotten));

    EXPECT_EQ(obj->avail(), 2);

    EXPECT_EQ(gotten[0], 0x4);
    EXPECT_EQ(gotten[1], 0x5);
    EXPECT_EQ(gotten[2], 0x6);
    EXPECT_EQ(gotten[3], 0x7);

    EXPECT_EQ(obj->free(), 6); /* we have 6 byte of free space */

    char added[2] = {static_cast<char>(0xff), static_cast<char>(0xfd)};

    obj->add(added, sizeof(added));

    EXPECT_EQ(obj->avail(), 4);
    EXPECT_EQ(obj->free(), 4);

    EXPECT_EQ(obj->get_one(), 0x8);
    EXPECT_EQ(obj->get_one(), 0x9);
    EXPECT_EQ(obj->get_one(), 0xff);
    EXPECT_EQ(obj->get_one(), 0xfd);

    EXPECT_EQ(obj->free(), 8);
    EXPECT_EQ(obj->avail(), 0);
}

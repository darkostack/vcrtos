#include "gtest/gtest.h"

#include "utils/ringbuffer.hpp"

using namespace vc;
using namespace utils;

class TestUtilsRingbuffer : public testing::Test
{
protected:
    char buffer[8];

    RingBuffer *rb;

    virtual void SetUp()
    {
        rb = new RingBuffer(buffer, sizeof(buffer));
    }

    virtual void TearDown()
    {
        delete rb;
    }
};

TEST_F(TestUtilsRingbuffer, constructor_test)
{
    EXPECT_TRUE(rb);
}

TEST_F(TestUtilsRingbuffer, functions_test)
{
    EXPECT_EQ(rb->avail(), 0);

    rb->add_one(0x1);
    rb->add_one(0x2);

    EXPECT_EQ(rb->avail(), 2);

    EXPECT_NE(rb->get_one(), -1); /* this get 0x1 */
    EXPECT_EQ(rb->get_one(), 0x2);

    EXPECT_TRUE(rb->is_empty());
    EXPECT_FALSE(rb->is_full());

    EXPECT_EQ(rb->peek_one(), -1); /* nothing in the buffer */

    rb->add_one(0x1);
    rb->add_one(0x2);
    rb->add_one(0x3);
    rb->add_one(0x4);
    rb->add_one(0x5);
    rb->add_one(0x6);
    rb->add_one(0x7);
    rb->add_one(0x8);

    EXPECT_EQ(rb->avail(), 8);

    EXPECT_FALSE(rb->is_empty());
    EXPECT_TRUE(rb->is_full());

    EXPECT_EQ(rb->add_one(0x9), 0x1); /* this will remove the head (0x1) and add (0x9) to the tail */

    EXPECT_TRUE(rb->is_full());

    EXPECT_EQ(rb->peek_one(), 0x2); /* 0x2 is the head */
    EXPECT_EQ(rb->peek_one(), 0x2); /* 0x2 is the head */
    EXPECT_EQ(rb->peek_one(), 0x2); /* 0x2 is the head */
    EXPECT_EQ(rb->peek_one(), 0x2); /* 0x2 is the head */

    EXPECT_EQ(rb->avail(), 8); /* peek will not reduce the buffer size */

    char peeked[4];

    rb->peek(peeked, sizeof(peeked));

    EXPECT_EQ(rb->avail(), 8);

    EXPECT_EQ(peeked[0], 0x2);
    EXPECT_EQ(peeked[1], 0x3);
    EXPECT_EQ(peeked[2], 0x4);
    EXPECT_EQ(peeked[3], 0x5);

    rb->remove(2); /* 0x2, 0x3 was removed */

    EXPECT_EQ(rb->avail(), 6);

    char gotten[4];

    rb->get(gotten, sizeof(gotten));

    EXPECT_EQ(rb->avail(), 2);

    EXPECT_EQ(gotten[0], 0x4);
    EXPECT_EQ(gotten[1], 0x5);
    EXPECT_EQ(gotten[2], 0x6);
    EXPECT_EQ(gotten[3], 0x7);

    EXPECT_EQ(rb->free(), 6); /* we have 6 byte of free space */

    char added[2] = {static_cast<char>(0xff), static_cast<char>(0xfd)};

    rb->add(added, sizeof(added));

    EXPECT_EQ(rb->avail(), 4);
    EXPECT_EQ(rb->free(), 4);

    EXPECT_EQ(rb->get_one(), 0x8);
    EXPECT_EQ(rb->get_one(), 0x9);
    EXPECT_EQ(rb->get_one(), 0xff);
    EXPECT_EQ(rb->get_one(), 0xfd);

    EXPECT_EQ(rb->free(), 8);
    EXPECT_EQ(rb->avail(), 0);
}

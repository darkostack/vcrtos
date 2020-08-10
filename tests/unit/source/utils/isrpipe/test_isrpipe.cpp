#include "gtest/gtest.h"

#include "utils/isrpipe.hpp"

using namespace vc;
using namespace utils;

class TestUtilsTsrb : public testing::Test
{
protected:
    char buffer[8];

    Tsrb *obj;

    virtual void SetUp()
    {
        obj = new Tsrb(buffer, sizeof(buffer));
    }

    virtual void TearDown()
    {
        delete obj;
    }
};

TEST_F(TestUtilsTsrb, constructor_test)
{
    EXPECT_TRUE(obj);
}

TEST_F(TestUtilsTsrb, functions_test)
{
    EXPECT_EQ(obj->avail(), 0);
    EXPECT_TRUE(obj->is_empty());
    EXPECT_FALSE(obj->is_full());
    EXPECT_EQ(obj->free(), 8);

    obj->add_one(0x1);
    obj->add_one(0x2);
    obj->add_one(0x3);
    obj->add_one(0x4);

    EXPECT_EQ(obj->avail(), 4);
    EXPECT_FALSE(obj->is_empty());
    EXPECT_FALSE(obj->is_full());
    EXPECT_EQ(obj->free(), 4);

    obj->push(0x5);
    obj->push(0x6);
    obj->push(0x7);
    obj->push(0x8);

    EXPECT_EQ(obj->avail(), 8);
    EXPECT_FALSE(obj->is_empty());
    EXPECT_TRUE(obj->is_full());
    EXPECT_EQ(obj->free(), 0);

    EXPECT_EQ(obj->pop(), 0x1);
    EXPECT_EQ(obj->pop(), 0x2);
    EXPECT_EQ(obj->pop(), 0x3);
    EXPECT_EQ(obj->pop(), 0x4);

    EXPECT_EQ(obj->free(), 4);

    obj->drop(2);

    EXPECT_EQ(obj->free(), 6);
    EXPECT_EQ(obj->avail(), 2);

    EXPECT_EQ(obj->get_one(), 0x7);
    EXPECT_EQ(obj->get_one(), 0x8);

    EXPECT_EQ(obj->avail(), 0);
    EXPECT_TRUE(obj->is_empty());
    EXPECT_FALSE(obj->is_full());
    EXPECT_EQ(obj->free(), 8);

    char data[4] = {(char)0xfa, (char)0xfb, (char)0xfc, (char)0xfd};

    obj->add(data, sizeof(data));

    EXPECT_EQ(obj->avail(), 4);
    EXPECT_FALSE(obj->is_empty());
    EXPECT_FALSE(obj->is_full());
    EXPECT_EQ(obj->free(), 4);

    char result[4];

    obj->get(result, sizeof(result));

    EXPECT_EQ(obj->avail(), 0);
    EXPECT_TRUE(obj->is_empty());
    EXPECT_FALSE(obj->is_full());
    EXPECT_EQ(obj->free(), 8);

    EXPECT_EQ(result[0], (char)0xfa);
    EXPECT_EQ(result[1], (char)0xfb);
    EXPECT_EQ(result[2], (char)0xfc);
    EXPECT_EQ(result[3], (char)0xfd);
}

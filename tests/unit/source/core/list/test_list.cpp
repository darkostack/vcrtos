#include "gtest/gtest.h"

#include "core/list.hpp"

using namespace vc;

class TestList : public testing::Test
{
protected:
    List *obj;

    virtual void SetUp()
    {
        obj = new List;
    }

    virtual void TearDown()
    {
        delete obj;
    }
};

TEST_F(TestList, constructor_test)
{
    EXPECT_TRUE(obj);

    EXPECT_EQ(sizeof(List), sizeof(list_node_t));
}

TEST_F(TestList, functions_test)
{
    List node1;
    List node2;
    List node3;

    obj->add(&node1);
    obj->add(&node2);
    obj->add(&node3);

    /* obj->3->2->1->null */

    EXPECT_EQ(obj->next, &node3);
    EXPECT_EQ(obj->next->next, &node2);
    EXPECT_EQ(obj->next->next->next, &node1);
    EXPECT_EQ(obj->next->next->next->next, nullptr);

    EXPECT_EQ(obj->remove_head(), &node3);

    /* obj->2->1->null */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, nullptr);

    obj->add(&node3);

    /* obj->3->2->1->null */

    EXPECT_EQ(obj->next, &node3);
    EXPECT_EQ(obj->next->next, &node2);
    EXPECT_EQ(obj->next->next->next, &node1);
    EXPECT_EQ(obj->next->next->next->next, nullptr);

    EXPECT_EQ(List::remove(obj, &node2), &node2);

    /* obj->3->1->null */

    EXPECT_EQ(obj->next, &node3);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, nullptr);

    EXPECT_EQ(List::remove(obj, &node3), &node3);

    /* obj->1->null */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, nullptr);

    EXPECT_EQ(List::remove(obj, &node1), &node1);

    /* obj->null */

    EXPECT_EQ(obj->next, nullptr);
}

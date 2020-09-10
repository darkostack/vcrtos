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

#include "core/clist.hpp"

using namespace vc;

class TestClist : public testing::Test
{
protected:
    Clist *obj;

    virtual void SetUp()
    {
        obj = new Clist;
    }

    virtual void TearDown()
    {
        delete obj;
    }
};

TEST_F(TestClist, constructor_test)
{
    EXPECT_TRUE(obj);

    EXPECT_EQ(sizeof(Clist), sizeof(list_node_t));
}

TEST_F(TestClist, right_push_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->right_push(&node1);

    /* obj->1->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node1);

    obj->right_push(&node2);

    /* obj->2->1->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, &node2);

    obj->right_push(&node3);

    /* obj->3->1->2->3 */

    EXPECT_EQ(obj->next, &node3);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node3);

    obj->right_push(&node4);

    /* obj->4->1->2->3->4 */

    EXPECT_EQ(obj->next, &node4);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next->next, &node4);
}

TEST_F(TestClist, left_push_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->left_push(&node1);

    /* obj->1->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node1);

    obj->left_push(&node2);

    /* obj->1->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node2);
    EXPECT_EQ(obj->next->next->next, &node1);

    obj->left_push(&node3);

    /* obj->1->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node3);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);

    obj->left_push(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);
}

TEST_F(TestClist, left_pop_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->left_push(&node1);
    obj->left_push(&node2);
    obj->left_push(&node3);
    obj->left_push(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->left_pop(), &node4);

    /* obj->1->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node3);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);

    EXPECT_EQ(obj->left_pop(), &node3);
    EXPECT_EQ(obj->left_pop(), &node2);

    /* obj->1->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node1);

    EXPECT_EQ(obj->left_pop(), &node1);

    /* obj->null */

    EXPECT_EQ(obj->next, nullptr);
}

TEST_F(TestClist, left_pop_right_push_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->left_push(&node1);
    obj->left_push(&node2);
    obj->left_push(&node3);
    obj->left_push(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    obj->left_pop_right_push();

    /* obj->4->3->2->1->4 */

    EXPECT_EQ(obj->next, &node4);
    EXPECT_EQ(obj->next->next, &node3);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);
    EXPECT_EQ(obj->next->next->next->next->next, &node4);

    obj->left_pop_right_push();

    /* obj->3->2->1->4->3 */

    EXPECT_EQ(obj->next, &node3);
    EXPECT_EQ(obj->next->next, &node2);
    EXPECT_EQ(obj->next->next->next, &node1);
    EXPECT_EQ(obj->next->next->next->next, &node4);
    EXPECT_EQ(obj->next->next->next->next->next, &node3);

    /* obj->2->1->4->3->2 */

    obj->left_pop_right_push();

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, &node4);
    EXPECT_EQ(obj->next->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next->next, &node2);

    obj->left_pop_right_push();

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);
}

TEST_F(TestClist, count_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->left_push(&node1);
    obj->left_push(&node2);
    obj->left_push(&node3);
    obj->left_push(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->count(), 4);
}

TEST_F(TestClist, remove_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->left_push(&node1);
    obj->left_push(&node2);
    obj->left_push(&node3);
    obj->left_push(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->remove(&node3), &node3);

    /* obj->1->4->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);

    EXPECT_EQ(obj->remove(&node1), &node1);

    /* obj->2->4->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node2);

    EXPECT_EQ(obj->remove(&node4), &node4);

    /* obj->2->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node2);

    EXPECT_EQ(obj->remove(&node2), &node2);

    /* obj->null */

    EXPECT_EQ(obj->next, nullptr);
}

TEST_F(TestClist, find_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->left_push(&node1);
    obj->left_push(&node2);
    obj->left_push(&node3);
    obj->left_push(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->find(&node1), &node1);
    EXPECT_EQ(obj->find(&node3), &node3);
    EXPECT_EQ(obj->find(&node2), &node2);
    EXPECT_EQ(obj->find(&node4), &node4);

    EXPECT_EQ(obj->remove(&node3), &node3);

    EXPECT_EQ(obj->find(&node3), nullptr);

    /* obj->1->4->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);
}

TEST_F(TestClist, find_before_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->left_push(&node1);
    obj->left_push(&node2);
    obj->left_push(&node3);
    obj->left_push(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->find_before(&node3), &node4);
    EXPECT_EQ(obj->find_before(&node4), &node1);
    EXPECT_EQ(obj->find_before(&node1), &node2);
    EXPECT_EQ(obj->find_before(&node2), &node3);
}

TEST_F(TestClist, right_pop_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->left_push(&node1);
    obj->left_push(&node2);
    obj->left_push(&node3);
    obj->left_push(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->right_pop(), &node1);

    /* obj->2->4->3->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);

    EXPECT_EQ(obj->right_pop(), &node2);

    /* obj->3->4->3 */

    EXPECT_EQ(obj->next, &node3);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);

    EXPECT_EQ(obj->right_pop(), &node3);

    /* obj->4->4 */

    EXPECT_EQ(obj->next, &node4);
    EXPECT_EQ(obj->next->next, &node4);

    EXPECT_EQ(obj->right_pop(), &node4);

    /* obj->null */

    EXPECT_EQ(obj->next, nullptr);
}

TEST_F(TestClist, left_right_peek_test)
{
    Clist node1;
    Clist node2;
    Clist node3;
    Clist node4;

    obj->left_push(&node1);
    obj->left_push(&node2);
    obj->left_push(&node3);
    obj->left_push(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->left_peek(), &node4);
    EXPECT_EQ(obj->right_peek(), &node1);

    /* obj->1->3->2->1 */

    EXPECT_EQ(obj->left_pop(), &node4);

    EXPECT_EQ(obj->left_peek(), &node3);
    EXPECT_EQ(obj->right_peek(), &node1);

    /* obj->2->3->2 */

    EXPECT_EQ(obj->right_pop(), &node1);

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node3);
    EXPECT_EQ(obj->next->next->next, &node2);

    EXPECT_EQ(obj->left_peek(), &node3);
    EXPECT_EQ(obj->right_peek(), &node2);

    EXPECT_EQ(obj->left_pop(), &node3);

    /* obj->2->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node2);

    EXPECT_EQ(obj->left_peek(), &node2);
    EXPECT_EQ(obj->right_peek(), &node2);

    EXPECT_EQ(obj->right_pop(), &node2);

    /* obj->null */

    EXPECT_EQ(obj->next, nullptr);

    EXPECT_EQ(obj->left_peek(), nullptr);
    EXPECT_EQ(obj->right_peek(), nullptr);
}

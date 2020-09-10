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

#include <vcrtos/instance.h>

#include "core/instance.hpp"
#include "core/code_utils.h"

using namespace vc;

class TestInstanceApi : public testing::Test
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

TEST_F(TestInstanceApi, constructor_test)
{
    EXPECT_TRUE(instance1);
    EXPECT_TRUE(instance2);
}

TEST_F(TestInstanceApi, init_test)
{
    EXPECT_TRUE(instance_is_initialized((instance_t *)instance1));
    EXPECT_TRUE(instance_is_initialized((instance_t *)instance2));
}

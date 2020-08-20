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

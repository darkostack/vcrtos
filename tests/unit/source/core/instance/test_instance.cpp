#include "gtest/gtest.h"

#include "core/instance.hpp"
#include "core/code_utils.h"

using namespace vc;

class TestInstance : public testing::Test
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

TEST_F(TestInstance, constructor_test)
{
    EXPECT_TRUE(instance1);
    EXPECT_TRUE(instance2);
}

TEST_F(TestInstance, init_test)
{
    EXPECT_TRUE(instance1->is_initialized());
    EXPECT_TRUE(instance2->is_initialized());
}

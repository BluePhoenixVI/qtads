
#include <gtest/gtest.h>

namespace LifecycleTests
{
// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions)
{
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}

TEST(HelloTest, BasicAssertionsBonus)
{
    // Expect two strings not to be equal.
    EXPECT_STREQ("hello", "hello");
    // Expect equality.
    EXPECT_NE(7 * 6, 49);
}

} // namespace LifecycleTests
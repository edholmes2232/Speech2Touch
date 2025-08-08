#include <gtest/gtest.h>

class TestSuite : public ::testing::Test
{
  protected:
  void SetUp() override
  {
    // Code to set up the test environment, if needed
  }

  void TearDown() override
  {
    // Code to clean up after tests, if needed
  }
};

// Example test case
TEST_F(TestSuite, BasicTest)
{
  EXPECT_EQ(1, 1);
}

// Add your own tests below

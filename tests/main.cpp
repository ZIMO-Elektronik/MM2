#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mm2/mm2.hpp>

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  return RUN_ALL_TESTS();
}
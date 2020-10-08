//
// Created by skushneryuk on 08/10/2020.
//

#include "gtest/gtest.h"

TEST(indexTest, testPrimes){
    EXPECT_EQ(5, 5);
}

TEST(libTest, rightGenerating){
    EXPECT_EQ(10, 10);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

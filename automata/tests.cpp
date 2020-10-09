//
// Created by skushneryuk on 08/10/2020.
//

#include "gtest/gtest.h"
#include "../automata_lib/automata.h"

TEST(Additional, TransitionTest){
    EXPECT_EQ(5, 5);
}

TEST(BuildingTest, Sizes){
    EXPECT_EQ(5, 5);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

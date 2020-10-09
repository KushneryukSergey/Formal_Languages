//
// Created by skushneryuk on 08/10/2020.
//

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "automata.h"
#include <iostream>

TEST(Additional, StateTest){
    State test0("name", true, true);
    State test1("abracadabra", true, false);
    State test2("5", false, true);
    State test3("", false, false);
    EXPECT_TRUE(test0.get_is_start());
    EXPECT_TRUE(test0.get_is_accept());
    EXPECT_EQ(test0.get_name(), "name");

    EXPECT_TRUE(test1.get_is_start());
    EXPECT_FALSE(test1.get_is_accept());
    EXPECT_EQ(test1.get_name(), "abracadabra");

    EXPECT_FALSE(test2.get_is_start());
    EXPECT_TRUE(test2.get_is_accept());
    EXPECT_EQ(test2.get_name(), "5");

    EXPECT_FALSE(test3.get_is_start());
    EXPECT_FALSE(test3.get_is_accept());
    EXPECT_EQ(test3.get_name(), "");

    test3.make_accept();
    EXPECT_TRUE(test3.get_is_accept());

    test0 += test2;
    test1 += test3;

    EXPECT_TRUE(test0.get_is_start());
    EXPECT_TRUE(test0.get_is_accept());
    EXPECT_EQ(test0.get_name(), "name+5");

    EXPECT_TRUE(test1.get_is_start());
    EXPECT_TRUE(test1.get_is_accept());
    EXPECT_EQ(test1.get_name(), "abracadabra+");
}


TEST(Additional, _MinStateTest){
    vector<size_t> v0 = {1, 2, 3};
    vector<size_t> v1 = {1, 2, 4};
    _MinState test0(v0);
    _MinState test1(v1);

    EXPECT_TRUE(test0 < test1);
}


TEST(Additional, _DetStateTest){
    _DetState test0(32+16+1, true, true);
    _DetState test1(16+2, false, false);
    _DetState test2(64+8+1, true, true);

    EXPECT_FALSE(test1.get_is_start());
    EXPECT_TRUE(test2.get_is_accept());
    EXPECT_EQ((test0|test1).get_mask(), 32+16+2+1);

    EXPECT_TRUE(test0 < test2);
    EXPECT_TRUE(test1 < test2);
    EXPECT_TRUE((test1|test0) < test2);
}

TEST(Additional, TransitionTest){
    Transition test0("a", 5);
    Transition test1("b", 8);
    Transition test2("a", 1);
    EXPECT_FALSE(test0 < test2);
    EXPECT_FALSE(test1 < test2);
    EXPECT_EQ(test0.get_finish(), 5);
    EXPECT_EQ(test2.get_expr(), "a");
}

TEST(Automata, MinDFASizes){
    vector<State> st = {State("0", true, true),
                        State("1", false, true),
                        State("2", false, false),
                        State("3", false, false)}; // задача 1а), семинар 3
    vector<set<Transition>> tr {{Transition("a", 1), Transition("b", 0)},
                                {Transition("a", 2), Transition("b", 0)},
                                {Transition("a", 1), Transition("b", 3)},
                                {Transition("a", 2), Transition("b", 3)},
                                };
    Automaton test(st, tr);
    EXPECT_EQ(test.get_transition_number(), 8);
    EXPECT_EQ(test.get_state_number(), 4);

    test.make_one_letter();
    EXPECT_EQ(test.get_transition_number(), 8);
    EXPECT_EQ(test.get_state_number(), 4);

    test.determinize();
    EXPECT_EQ(test.get_transition_number(), 8);
    EXPECT_EQ(test.get_state_number(), 4);

    // Проверим заодно, может ли использование дважды функии что-то изменить
    test.complete();
    EXPECT_EQ(test.get_transition_number(), 8);
    EXPECT_EQ(test.get_state_number(), 4);

    test.complete();
    EXPECT_EQ(test.get_transition_number(), 8);
    EXPECT_EQ(test.get_state_number(), 4);

    test.minimize(false);
    EXPECT_EQ(test.get_transition_number(), 8);
    EXPECT_EQ(test.get_state_number(), 4);

    test.minimize(false);
    EXPECT_EQ(test.get_transition_number(), 8);
    EXPECT_EQ(test.get_state_number(), 4);
}

TEST(Automata, ArbitaryNFASizes){ // (a*b*c)*
    vector<State> st = {State("0", true, true),
                        State("1", false, false),
                        State("2", false, false)}; // задача 1а), семинар 3
    vector<set<Transition>> tr {{Transition("a", 0), Transition("", 1)},
                                {Transition("b", 1), Transition("", 2)},
                                {Transition("c", 2), Transition("", 0)}};
    Automaton test(st, tr);
    EXPECT_EQ(test.get_transition_number(), 6);
    EXPECT_EQ(test.get_state_number(), 3);
    test.make_one_letter();
    EXPECT_EQ(test.get_transition_number(), 9);
    EXPECT_EQ(test.get_state_number(), 3);

    test.make_one_letter();
    EXPECT_EQ(test.get_transition_number(), 9);
    EXPECT_EQ(test.get_state_number(), 3);

    test.determinize();
    EXPECT_EQ(test.get_transition_number(), 9);
    EXPECT_EQ(test.get_state_number(), 3);

    test.complete();
    EXPECT_EQ(test.get_transition_number(), 9);
    EXPECT_EQ(test.get_state_number(), 3);

    test.minimize(false);
    EXPECT_EQ(test.get_transition_number(), 3);
    EXPECT_EQ(test.get_state_number(), 1);
}

TEST(Automata, HomeworkNFASizes){ //
    vector<State> st = {State("0", true, true),
                        State("1", false, false),
                        State("2", false, false),
                        State("3", false, false)}; //2 задача 4 домашнего задания
    vector<set<Transition>> tr {{Transition("a", 1)},
                                {Transition("b", 2), Transition("", 0), Transition("ab", 3)},
                                {Transition("a", 3),Transition("ba", 2)},
                                {Transition("", 1)}};
    Automaton test(st, tr);
    EXPECT_EQ(test.get_transition_number(), 7);
    EXPECT_EQ(test.get_state_number(), 4);

    test.make_one_letter();
    EXPECT_EQ(test.get_transition_number(), 11);
    EXPECT_EQ(test.get_state_number(), 6);

    test.determinize();
    EXPECT_EQ(test.get_transition_number(), 16);
    EXPECT_EQ(test.get_state_number(), 9);

    test.determinize();
    EXPECT_EQ(test.get_transition_number(), 16);
    EXPECT_EQ(test.get_state_number(), 9);

    test.complete();
    EXPECT_EQ(test.get_transition_number(), 20);
    EXPECT_EQ(test.get_state_number(), 10);

    test.minimize(false);
    EXPECT_EQ(test.get_transition_number(), 12);
    EXPECT_EQ(test.get_state_number(), 6);
}

TEST(Automata, StartStateNumberLimit){ //
    vector<State> st = {State("0", true, true),
                        State("1", true, false),
                        State("2", false, false),
                        State("3", false, false)};
    vector<set<Transition>> tr {{Transition("a", 1)},
                                {Transition("b", 2), Transition("", 0), Transition("ab", 3)},
                                {Transition("a", 3),Transition("ba", 2)},
                                {Transition("", 1)}};

    EXPECT_THROW(Automaton(st, tr), too_many_start_states_exception);
}

TEST(Automata, StateNumberLimit){ //
    vector<State> st;
    st.emplace_back("0", true, false);
    for(size_t i = 0; i < 60; ++i){
        st.emplace_back(std::to_string(i + 1), false, false);
    }
    vector<set<Transition>> tr {{Transition("a", 1)},
                                {Transition("b", 2), Transition("", 0), Transition("ab", 3)},
                                {Transition("a", 3),Transition("ba", 2)},
                                {Transition("", 1)}};

    Automaton test(st, tr);
    EXPECT_THROW(test.determinize(), too_many_states_exception);
    EXPECT_THROW(test.minimize(false), too_many_states_exception);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

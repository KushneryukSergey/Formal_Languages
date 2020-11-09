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


TEST(Workshop, ExampleTests){
    string expr1 = "ab +c .aba.* .bac. +.+ *";
    string expr2 = "acb. .bab.c .*.a b.ba.+.+* a.";

    Automaton automaton1(expr1);
    automaton1.make_one_letter();
    automaton1.remove_useless();

    Automaton automaton2(expr2);
    automaton1.make_one_letter();
    automaton1.remove_useless();
    string word = "a";
    EXPECT_EQ(automaton1.solve_workshop_problem_for_automaton(word), 2);
    EXPECT_EQ(automaton2.solve_workshop_problem_for_automaton(word), 2);
}

TEST(Workshop, InfTests) {
    string expr1 = "ab+*c.d.ae*.+*";
    string word1 = "a";
    string expr2 = "abc..*abc..*abc..*..";
    string word2 = "abc";
    string expr3 = "aba.*baa..*+.ba.*.";
    string word3 = "aba";
    string expr4 = "ab.abab...*.";
    string word4 = "ab";

    Automaton automaton1(expr1);
    Automaton automaton2(expr2);
    Automaton automaton3(expr3);
    Automaton automaton4(expr4);
    EXPECT_EQ(automaton1.solve_workshop_problem_for_automaton(word1), -1);
    EXPECT_EQ(automaton2.solve_workshop_problem_for_automaton(word2), -1);
    EXPECT_EQ(automaton3.solve_workshop_problem_for_automaton(word3), -1);
    EXPECT_EQ(automaton4.solve_workshop_problem_for_automaton(word4), -1);
}


TEST(Workshop, ErrorHandling) {
    string expr1 = "*ab c*.+"; // "accidental" iteration in the beginning
    string expr2 = "a b"; // "forgot" about operation in the end
    string expr3 = "a bc+ +."; // too many operation
    string expr4 = "abc +.+"; // too many operation again

    EXPECT_THROW(Automaton automaton1(expr1), incorrect_polish_expr_exception);
    EXPECT_THROW(Automaton automaton1(expr2), incorrect_polish_expr_exception);
    EXPECT_THROW(Automaton automaton1(expr3), incorrect_polish_expr_exception);
    EXPECT_THROW(Automaton automaton1(expr4), incorrect_polish_expr_exception);
}




int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

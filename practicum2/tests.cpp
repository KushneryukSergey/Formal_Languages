//
// Created by skushneryuk on 08/10/2020.
//

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "earley.h"
#include <iostream>

TEST(EarleyTesting, Palindrome){
    vector<ContextFreeGrammarRule> rules = {
            ContextFreeGrammarRule{'S', "aSa"},
            ContextFreeGrammarRule{'S', "bSb"},
            ContextFreeGrammarRule{'S', "cSc"},
            ContextFreeGrammarRule{'S', "dSd"},
            ContextFreeGrammarRule{'S', "a"},
            ContextFreeGrammarRule{'S', "b"},
            ContextFreeGrammarRule{'S', "c"},
            ContextFreeGrammarRule{'S', "d"},
            ContextFreeGrammarRule{'S', ""}
    };
    EarleySolver solver('S', rules);
    
    EXPECT_TRUE(solver.check_word("abcdcba"));
    EXPECT_TRUE(solver.check_word("abcddcba"));
    EXPECT_TRUE(solver.check_word("babacdcddcdcabab"));
    EXPECT_TRUE(solver.check_word("a"));
    EXPECT_TRUE(solver.check_word("bb"));
    
    EXPECT_FALSE(solver.check_word("ba"));
    EXPECT_FALSE(solver.check_word("abcdcb"));
    EXPECT_FALSE(solver.check_word("bbcddcba"));
    EXPECT_FALSE(solver.check_word("abc"));
    EXPECT_FALSE(solver.check_word("abcd"));
    
    EXPECT_THROW(solver.check_word("abcdS"), non_terminals_in_word_exception);
    EXPECT_THROW(solver.check_word("cdeedc"), non_alphabet_symbol_exception);
}


TEST(EarleyTesting, Wa_equals_Wb) {
    vector<ContextFreeGrammarRule> rules = {
            ContextFreeGrammarRule{'S', "aSbS"},
            ContextFreeGrammarRule{'S', "bSaS"},
            ContextFreeGrammarRule{'S', ""}
    };
    EarleySolver solver('S', rules);
    
    EXPECT_TRUE(solver.check_word("aaaabbbb"));
    EXPECT_TRUE(solver.check_word("aabbaabb"));
    EXPECT_TRUE(solver.check_word("aaaababababbbb"));
    EXPECT_TRUE(solver.check_word("ab"));
    EXPECT_TRUE(solver.check_word(""));
    
    EXPECT_FALSE(solver.check_word("baa"));
    EXPECT_FALSE(solver.check_word("aabbb"));
    EXPECT_FALSE(solver.check_word("aaaababababbb"));
    EXPECT_FALSE(solver.check_word("a"));
    EXPECT_FALSE(solver.check_word("b"));
    
    EXPECT_THROW(solver.check_word("aabbS"), non_terminals_in_word_exception);
    EXPECT_THROW(solver.check_word("aabbcc"), non_alphabet_symbol_exception);
}

TEST(EarleyTesting, Wb_equals_2Wa) {
    vector<ContextFreeGrammarRule> rules = {
            ContextFreeGrammarRule{'S', "aSbSbS"},
            ContextFreeGrammarRule{'S', "bSaSbS"},
            ContextFreeGrammarRule{'S', "bSbSaS"},
            ContextFreeGrammarRule{'S', ""}
    };
    EarleySolver solver('S', rules);
    
    EXPECT_TRUE(solver.check_word("abb"));
    EXPECT_TRUE(solver.check_word("bbabab"));
    EXPECT_TRUE(solver.check_word("bbbaaabbb"));
    EXPECT_TRUE(solver.check_word(""));
    EXPECT_TRUE(solver.check_word("bba"));
    EXPECT_TRUE(solver.check_word("bab"));
    
    EXPECT_FALSE(solver.check_word("baba"));
    EXPECT_FALSE(solver.check_word("bbbbbb"));
    EXPECT_FALSE(solver.check_word("bbaaabbb"));
    EXPECT_FALSE(solver.check_word("aab"));
    EXPECT_FALSE(solver.check_word("baa"));
    
    EXPECT_THROW(solver.check_word("abbS"), non_terminals_in_word_exception);
    EXPECT_THROW(solver.check_word("abbcccc"), non_alphabet_symbol_exception);
}

TEST(EarleyTesting, BBS) { // Balanced bracket sequence
    vector<ContextFreeGrammarRule> rules = {
            ContextFreeGrammarRule{'S', "(S)S"},
            ContextFreeGrammarRule{'S', "[S]S"},
            ContextFreeGrammarRule{'S', "<S>S"},
            ContextFreeGrammarRule{'S', ""}
    };
    EarleySolver solver('S', rules);
    
    EXPECT_TRUE(solver.check_word("()"));
    EXPECT_TRUE(solver.check_word("<[][()]>"));
    EXPECT_TRUE(solver.check_word("<()()>[]"));
    EXPECT_TRUE(solver.check_word("<>"));
    EXPECT_TRUE(solver.check_word("[]"));
    
    EXPECT_FALSE(solver.check_word("([)]"));
    EXPECT_FALSE(solver.check_word("((((]]]]"));
    EXPECT_FALSE(solver.check_word("<(>)[(<)>]"));
    EXPECT_FALSE(solver.check_word("())("));
    EXPECT_FALSE(solver.check_word("]<><><><>["));
    
    EXPECT_THROW(solver.check_word("(S)(S)"), non_terminals_in_word_exception);
    EXPECT_THROW(solver.check_word("(.)(.)"), non_alphabet_symbol_exception);
}


TEST(EarleyTesting, SubmodulesCorrectness) {
    vector<ContextFreeGrammarRule> rules = {
            ContextFreeGrammarRule{'S', "aSbS"},
            ContextFreeGrammarRule{'S', "bSaS"},
            ContextFreeGrammarRule{'S', ""}
    };
    EarleySolver solver('S', rules);
    
    solver._layers.resize(4);
    solver._layers[2] = {EarleySituation{2, 1, 1},  // S->aSbS is third after sorting
                         EarleySituation{3, 3, 1},  // S->bSaS is forth after sorting
                         EarleySituation{2, 2, 1}};
    
    solver._scan(2, 'b');  // can read b from the third situation (S->aS.bS, 1)
    EXPECT_EQ(solver._next.size(), 1);
    EXPECT_EQ(solver._next.front(), (EarleySituation{2, 3, 1}));
    
    swap(solver._next, solver._current);
    
    solver._predict(3);  // there is (S->aSb.S, 2) situation,
                         // so all existing rules should be added
    EXPECT_EQ(solver._next.size(), 3);
    EXPECT_EQ(solver._next.front(), (EarleySituation{1, 0, 3}));
    
    // adding some situations to check whether all possible completes
    // are made (by S -> ., 2) situation
    solver._current.push(EarleySituation{1, 0, 2});  // should be made
    solver._current.push(EarleySituation{3, 4, 2});  // should be made
    solver._current.push(EarleySituation{1, 0, 1});  // cannot be made
                                                                                     // (no elements in _layer[1])
    solver._current.push(EarleySituation{2, 2, 2});  // cannot be made
                                                                                     // (last symbol is terminal)
    
    
    solver._complete(3);  // check if all situations added
    EXPECT_EQ(solver._next.size(), 5);
    EXPECT_EQ(solver._next.back(), (EarleySituation{3, 4, 1}));
    
    // check if _layer[3] size is correct (new situations not included because
    // they should be "predicted" and "completed" as mush as possible before it
    EXPECT_EQ(solver._layers[3].size(), 6);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

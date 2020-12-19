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


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

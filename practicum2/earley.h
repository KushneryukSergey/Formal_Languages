//
// Created by skushneryuk on 18/12/2020.
//

#ifndef EARLEY_H
#define EARLEY_H

#include <algorithm>
#include <set>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <exception>
#include <gtest/gtest.h>

using std::string;
using std::vector;
using std::set;
using std::map;
using std::queue;
using std::lower_bound;

const char spec_symbol = '$';
// Not the best way to add special symbol for for S'->S rule, but in current
// situation it's the best way
//
// Ideally, there should be numbers for every non-terminal and alphabet element
// to avoid such situations (vector of numbers instead of string expression
// and so on), it can be changed without any problems. I hope...

class incorrect_word : public std::exception {};

class non_terminals_in_word_exception : public incorrect_word {
    [[nodiscard]] const char* what() const noexcept override;
};

class non_alphabet_symbol_exception : public incorrect_word {
    [[nodiscard]] const char* what() const noexcept override;
};

struct ContextFreeGrammarRule {
    char non_terminal;
    string expression;
    
    bool operator<(const ContextFreeGrammarRule&) const;
};


struct EarleySituation {
    size_t rule_number;
    size_t point_pos;
    size_t prev_pos;
    
    bool operator<(const EarleySituation&) const;
    bool operator==(const EarleySituation&) const;
    
    EarleySituation next();
};


class EarleySolver {
public:
    char _start;
    vector <ContextFreeGrammarRule> _rules;
    set<char> _non_terminals;
    set<char> _alphabet;
    vector <set<EarleySituation>> _layers;
    map<char, int> _first_rule;
    queue <EarleySituation> _current, _next;
    void _predict(const size_t&);
    void _complete(const size_t&);
    void _scan(const size_t&, char);
    
    bool _is_complete_possible(const EarleySituation&);
    bool _is_non_terminal(const char&);
    
    char _next_symbol(const EarleySituation&);
    void _add_situation(const size_t&, const EarleySituation&);
    
    // Testing module
    FRIEND_TEST(EarleyTesting, SubmodulesCorrectness);
    
public:
    EarleySolver(const char&, const vector<ContextFreeGrammarRule>&);
    bool check_word(const string&);
};


#endif //EARLEY_H

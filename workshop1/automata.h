//
// Created by skushneryuk on 09/10/2020.
//

#ifndef AUTOMATA_AUTOMATA_H
#define AUTOMATA_AUTOMATA_H

#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>
#include <set>
#include <queue>
#include <stack>
#include <exception>


using std::vector;
using std::string;
using std::pair;
using std::set;
using std::queue;
using std::stack;


class too_many_states_exception : public std::exception {
    [[nodiscard]] const char *what() const noexcept override;
};

class too_many_start_states_exception : public std::exception {
    [[nodiscard]] const char *what() const noexcept override;
};

class incorrect_polish_expr_exception : public std::exception {
    [[nodiscard]] const char *what() const noexcept override;
};


class State {
public:
    string name;
    bool is_start;
    bool is_accept;

public:
    State() = delete;
    State(string name, const bool &is_start, const bool &is_accept) :
            name(std::move(name)),
            is_start(is_start),
            is_accept(is_accept) {
    }
    [[nodiscard]] const string &get_name() const;
    [[nodiscard]] const bool &get_is_start() const;
    [[nodiscard]] const bool &get_is_accept() const;
    void make_accept();
    void operator+=(const State &);
};


class Transition {
    string expr;
    size_t finish;
public:
    Transition() = delete;
    Transition(string expr, const size_t &finish) : expr(std::move(expr)), finish(finish) {};
    [[nodiscard]] const string &get_expr() const;
    [[nodiscard]] const size_t &get_finish() const;
    bool operator<(const Transition &) const;
};


class Automaton {
    vector<State> states;
    vector<set<Transition>> transitions;
    set<string> alphabet;
    //set<size_t> finish;
    size_t state_number = 0;
    size_t transition_number = 0;
    size_t start_state = UINT32_MAX;
    bool is_one_letter = false;
    static const size_t MAX_AUTOMATA_SIZE = 62;
    Automaton() = delete;

public:
    Automaton(const vector<State> &, const vector<set<Transition>> &);
    Automaton(const string &);
    friend std::ostream &operator<<(std::ostream &stream, const Automaton &automaton);
    void output_alphabet(std::ostream &) const;
    void output_states(std::ostream &) const;
    void output_transitions(std::ostream &) const;
    void make_one_letter();
    void remove_useless();
    int solve_workshop_problem_for_automaton(const string &word);
    [[nodiscard]] size_t get_state_number() const;
    [[nodiscard]] size_t get_transition_number() const;

private:
    void _add_transition(const size_t &, const size_t &, const string &);
    void _delete_transition(const size_t &, const Transition &);
    void _add_state(const string &, const bool &, const bool &);
    void _make_leq_one_letter();
    void _remove_epsilon_transitions();
    void _remove_unreachable();
    void _push_epsilon_transitions_in_state(const size_t &, const vector<set<Transition>> &);
    void _recalc_state_number();
    void _recalc_transition_number();
    void _find_reachable_state_by_word(const size_t &from, const string &word, set<size_t> &save);
};

class CycleFinder {
public:
    enum COLOR {
        WHITE, GREY, BLACK
    };

    CycleFinder(const vector<set<size_t>> &graph, vector<COLOR> &color) : graph(graph), color(color) {};
    bool dfs_cycle(const size_t &);

private:
    const vector<set<size_t>> &graph;
    vector<COLOR> &color;
};


class TopSorter {
private:
    const vector<set<size_t>> &graph;
    vector<bool> used;
    void _dfs_ts(const size_t &start);

public:
    explicit TopSorter(const vector<set<size_t>> &);
    vector<size_t> sorted;
    void topological_sort();
};

#endif //AUTOMATA_AUTOMATA_H

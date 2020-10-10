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
#include <map>
#include <queue>
#include <exception>


using std::vector;
using std::string;
using std::pair;
using std::set;
using std::queue;
using std::map;


class too_many_states_exception: std::exception{
    [[nodiscard]] const char* what() const noexcept override;
};

class too_many_start_states_exception: std::exception{
    [[nodiscard]] const char* what() const noexcept override;
};


class State{
public:
    string name;
    bool is_start;
    bool is_accept;

public:
    State() = delete;
    State(string  name, const bool& is_start, const bool& is_accept):
        name(std::move(name)),
        is_start(is_start),
        is_accept(is_accept) {
    }

    [[nodiscard]] const string& get_name() const;
    [[nodiscard]] const bool& get_is_start() const;
    [[nodiscard]] const bool& get_is_accept() const;
    void make_accept();

    void operator+=(const State&);
};


class _MinState{
    string achievable;
public:
    _MinState() = delete;
    explicit _MinState(const vector<size_t>&);

    bool operator< (const _MinState&) const;
};


class _DetState{
    unsigned long long mask;
    bool is_start;
    bool is_accept;

public:
    _DetState();
    _DetState(const unsigned long long&, const bool&, const bool&);

    [[nodiscard]] const unsigned long long& get_mask() const;
    [[nodiscard]] const bool& get_is_start() const;
    [[nodiscard]] const bool& get_is_accept() const;

    bool operator<(const _DetState&) const;

    _DetState operator|(const _DetState&) const;
    _DetState& operator|=(const _DetState&);
};


class Transition{
    string expr;
    size_t finish;
public:
    Transition() = delete;
    Transition(string expr, const size_t& finish): expr(std::move(expr)), finish(finish) {};
    [[nodiscard]] const string& get_expr() const;
    [[nodiscard]] const size_t& get_finish() const;

    bool operator<(const Transition &) const;
};


class Automaton{
    vector<State> states;
    vector<set<Transition>> transitions;
    set<string> alphabet;
    size_t state_number;
    size_t transition_number;
    size_t start_state = UINT32_MAX;
    bool is_one_letter = false;
    bool is_DFA = false;
    bool is_complete = false;
    bool is_minimum = false;

    static const size_t MAX_AUTOMATA_SIZE = 60;

public:
    Automaton() = delete;
    Automaton(const vector<State>&, const vector<set<Transition>>&);

    friend std::ostream& operator<<(std::ostream & stream, const Automaton& automaton);

    void determinize();
    void output_alphabet(std::ostream&) const ;
    void output_states(std::ostream&) const ;
    void output_transitions(std::ostream&) const ;
    void minimize(bool print_log=false, std::ostream& stream=std::cout);
    void complete();
    void tex_graph_print(std::ostream & stream) const ;
    void tex_transition_table_print(std::ostream & stream) const ;
    void make_one_letter();
    size_t get_state_number();
    size_t get_transition_number();

private:
    void _add_transition(const size_t&, const size_t&, const string&);
    void _delete_transition(const size_t&, const Transition&);
    void _add_state(const string&, const bool&, const bool&);
    bool _is_exist_transition_by_letter(const int &start, const string &expr);

    void _make_leq_one_letter();
    void _remove_epsilon_transitions();
    void _push_epsilon_transitions_in_state(const size_t&, const vector<set<Transition>>&);
    void _classify();
    void _recalc_state_number();
    void _recalc_transition_number();

    string _build_name_by_mask(const unsigned long long& mask, const vector<State>& old_states, const string& separator = "");
};

#endif //AUTOMATA_AUTOMATA_H

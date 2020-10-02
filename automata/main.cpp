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
    [[nodiscard]] const char* what() const noexcept override {
        return "Too many states in the automaton to calc DFA!\n";
    }
};

class too_many_start_states_exception: std::exception{
    [[nodiscard]] const char* what() const noexcept override {
        return "Too many start states in the automaton!\n";
    }
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

    [[nodiscard]] string get_name() const;
    [[nodiscard]] bool get_is_start() const;
    [[nodiscard]] bool get_is_accept() const;
};

string State::get_name() const {
    return name;
}

bool State::get_is_start() const {
    return is_start;
}

bool State::get_is_accept() const {
    return is_accept;
}

class _IntermediateState{
    unsigned long long mask;
    bool is_start;
    bool is_accept;

public:
    _IntermediateState(): mask(0), is_start(false), is_accept(false) {}
    _IntermediateState(const unsigned long long& mask, const bool& is_start, const bool& is_accept):
        mask(mask), is_start(is_start), is_accept(is_accept){
    }

    [[nodiscard]] unsigned long long get_mask() const;
    [[nodiscard]] bool get_is_start() const;
    [[nodiscard]] bool get_is_accept() const;

    bool operator<(const _IntermediateState&) const;

    _IntermediateState operator|(const _IntermediateState&) const;
    _IntermediateState& operator|=(const _IntermediateState&);
};

//оператор, чтобы можно было запихнуть все это в set
bool _IntermediateState::operator<(const _IntermediateState& other) const {
    return mask < other.mask;
}

_IntermediateState _IntermediateState::operator|(const _IntermediateState& other) const {
    return _IntermediateState(mask|other.mask, false, is_accept|other.is_accept);
}

_IntermediateState& _IntermediateState::operator|=(const _IntermediateState& other) {
    mask |= other.mask;
    is_start = (other.mask == 0) && is_start;
    is_accept |= other.is_accept;
    return *this;
}

unsigned long long _IntermediateState::get_mask() const {
    return mask;
}

bool _IntermediateState::get_is_start() const {
    return is_start;
}

bool _IntermediateState::get_is_accept() const {
    return is_accept;
}

class Transition{
public:
    string expr;
    int finish;

    Transition() = delete;
    Transition(string expr, const int& finish): expr(std::move(expr)), finish(finish) {};

    bool operator<(const Transition &) const;
};

bool Transition::operator<(const Transition& other) const {
    if(expr == other.expr){
        return finish < other.finish;
    } else {
        return expr < other.expr;
    }
}

class Automaton{
    vector<State> state;
    vector<set<Transition>> transition;
    set<string> alphabet;
    int start_state = -1;

    static const size_t MAX_AUTOMATA_SIZE = 60;

public:
    Automaton() = delete;
    Automaton(const vector<State>&, const vector<set<Transition>>&);
    void determinize();
    friend std::ostream& operator<<(std::ostream & stream, const Automaton& automaton);

private:
    void _make_leq_one_letter();
    void _remove_epsilon_transitions();
    void _push_epsilon_transitions_in_state(const int &curr_state, set<Transition> &new_transitions);
    void _classify();
    string _build_name_by_mask(const unsigned long long&);
};

std::ostream& operator<<(std::ostream & stream, const Automaton& automaton) {
    std::cout << "AUTOMATON" << std::endl;
    std::cout << "Alphabet: " << std::endl;
    std::cout << "| ";
    for (const auto& letter: automaton.alphabet){ //позже выделю отдельно функции вывода алфавита, состояний и переходов
        std::cout << letter << " | ";
    }
    std::cout << std::endl;
    std::cout << "States: " << std::endl;
    for(size_t i = 0; i < automaton.state.size(); ++i){
        auto st = automaton.state[i];
        std::cout << i << ' ' << st.name << (st.is_start ? " start": "") << (st.is_accept ? " accept ": "") << std::endl;
    }
    std::cout << std::endl << "Transitions: " << std::endl;
    for(size_t i = 0; i < automaton.transition.size(); ++i){
        for(const auto& tr: automaton.transition[i]){
            std::cout << i << " -> " << tr.finish << ' ' << (tr.expr.empty() ? "#eps#" : tr.expr) << std::endl;
        }
    }

    return stream;
}

Automaton::Automaton(const vector<State>& state, const vector<set<Transition>>& transition):
        state(state),
        transition(transition) {
        for(const auto& curr_state_trans: transition){
            for(const auto& curr_trans: curr_state_trans){
                for(size_t i = 0; i < curr_trans.expr.size(); ++i){
                    alphabet.insert(curr_trans.expr.substr(i, 1));
                }
            }
        }
        for(size_t i = 0; i < state.size(); ++i){
            if(state[i].get_is_start()){
                if(start_state == -1){
                    start_state = i;
                } else {
                    throw too_many_start_states_exception();
                }
            }
        }
    }

void Automaton::_make_leq_one_letter() {
    auto original_state_number = state.size();
    for(size_t curr_state = 0; curr_state < original_state_number; ++curr_state){
        queue<Transition> long_trans;
        for(const auto& trans: transition[curr_state]){
            if(trans.expr.size() > 1){
                long_trans.push(trans);
            }
        }
        while(!long_trans.empty()){
            Transition tr = long_trans.front();
            long_trans.pop();
            transition[curr_state].erase(tr);
            size_t last = curr_state;
            for(size_t i = 0; i < tr.expr.size(); ++i){
                size_t new_state = state.size();
                if(i + 1 == tr.expr.size()){
                    new_state = tr.finish;
                } else {
                    state.emplace_back(std::to_string(new_state), false, false);
                    transition.emplace_back(set<Transition>());
                }
                transition[last].insert(Transition(tr.expr.substr(i, 1), new_state));
                last = new_state;
            }
        }
    }
}

void Automaton::_remove_epsilon_transitions() {
    vector<set<Transition>> new_transitions(state.size());
    for(size_t curr_state = 0; curr_state < transition.size(); ++curr_state){
        _push_epsilon_transitions_in_state(curr_state, new_transitions[curr_state]);
    }
    transition = new_transitions;
}

void Automaton::_classify() {
    vector<map<string, _IntermediateState>> old_transition(transition.size());
    map<_IntermediateState, int> renumeration;
    int cnt = 2;
    vector<set<Transition>> new_transition;
    vector<State> new_state;

    vector<_IntermediateState> old_states;
    for(size_t i = 0; i < state.size(); ++i){
        old_states.emplace_back(1u<<i, state[i].get_is_start(), state[i].get_is_accept());
    }

    for(size_t i = 0; i < transition.size(); ++i){
        for(const auto& trans: transition[i]){
            old_transition[i][trans.expr] |= old_states[trans.finish];
        }
    }

    queue<_IntermediateState> pack_states;
    map<_IntermediateState, bool> used;
    pack_states.push(_IntermediateState(1 << start_state, true, state[start_state].get_is_accept()));
    renumeration[_IntermediateState(1<<start_state, true, false)] = 1;

    new_state.emplace_back(_build_name_by_mask(1u<<start_state), true, state[start_state].get_is_accept());
    new_transition.emplace_back();

    while(!pack_states.empty()){
        auto st = pack_states.front();
        pack_states.pop();
        unsigned long long curr_mask;
        if(!used[st]){
            used[st] = true;
            for(const string& letter: alphabet){
                _IntermediateState curr_state_pack; // все достижимые состояния по данному символу
                curr_mask = st.get_mask();
                for(size_t i = 0; 1u<<i <= curr_mask; ++i) {
                    if(1u<<i & curr_mask){
                        curr_state_pack |= old_transition[i][letter];
                    }
                }
                if(curr_state_pack.get_mask()){
                    if(renumeration[curr_state_pack] == 0){
                        renumeration[curr_state_pack] = cnt;
                        ++cnt;
                        new_state.emplace_back(_build_name_by_mask(curr_state_pack.get_mask()),
                                               curr_state_pack.get_is_start(),
                                               curr_state_pack.get_is_accept());
                        new_transition.emplace_back();
                    }
                    new_transition[renumeration[st] - 1].insert(Transition(letter, renumeration[curr_state_pack] - 1));
                    pack_states.push(curr_state_pack);
                }
            }
        }
    }

    transition = new_transition;
    state = new_state;
}

void Automaton::determinize() {
    if(state.size() > MAX_AUTOMATA_SIZE){
        throw too_many_states_exception();
    }
    _make_leq_one_letter();
    std::cout << *this << std::endl;
    _remove_epsilon_transitions();
    std::cout << *this << std::endl;
    _classify();
    std::cout << *this << std::endl;
}

void Automaton::_push_epsilon_transitions_in_state(const int& curr_state, set<Transition>& new_transitions) {
    vector<bool> used(state.size(), false);
    queue<int> reachable;
    reachable.push(curr_state);
    while(!reachable.empty()){
        int s = reachable.front();
        reachable.pop();
        if(used[s]){
            continue;
        }
        used[s] = true;
        new_transitions.insert(transition[s].begin(), transition[s].end());
        for(const auto& trans: transition[s]){
            if(!trans.expr.empty()){
                break;
            }
            reachable.push(trans.finish);
            state[curr_state].is_accept |= state[trans.finish].is_accept;
        }
    }
    auto iter = new_transitions.begin();
    while(iter != new_transitions.end() && iter->expr.empty()){
        auto copy_iter = iter;
        ++iter;
        new_transitions.erase(copy_iter);
    }
}

string Automaton::_build_name_by_mask(const unsigned long long& mask) {
    vector<string> names;
    for(size_t i = 0; 1u<<i <= mask; ++i){
        if(1u<<i & mask){
            names.push_back(state[i].get_name());
        }
    }
    string new_name = "{";
    for(size_t i = 0; i + 1 < names.size(); ++i){
        new_name += names[i] + ", ";
    }
    new_name += names.back() + "}";
    return new_name;
}

Automaton input_automata(){
    int size, trans_number;
    std::cout << "\n Input number of states: \n";
    std::cin >> size;
    if(size > 60){
        throw too_many_states_exception();
    }
    std::cout << "\n Input states in format \"[name] [start or not (0 or 1)] [accept or not (0 or 1)]\"\n"
                 "(example \"А 1 0\" - state A, start and not accept): \n";
    vector<State> st;
    string name;
    bool start, accept;
    for(size_t i = 0; i < size; ++i){
        std::cout << i << ": ";
        std::cin >> name >> start >> accept;
        st.emplace_back(name, start, accept);
    }
    std::cout << "\n Input number of transitions: \n";
    std::cin >> trans_number;
    std::cout << "\n Input transitions in format \"[number of first state] [number of second state] [word]\"\n"
                 "(numbers were written when you were inputting states' information): \n";
    vector<set<Transition>> tr(size);
    int u, v;
    string word;
    for(size_t i = 0; i < trans_number; ++i){
        std::cin >> u >> v >> word;
        tr[u].insert(Transition(word, v));
    }
    Automaton automaton(st, tr);

    return automaton;
}

void determinize_automata(){
    Automaton automaton = input_automata();
    automaton.determinize();
    std::cout << automaton << std::endl;
}

void test1(){
    vector<State> st = {State("0", true, false),
                        State("1", false, false),
                        State("2", false, true)};
    vector<set<Transition>> tr = {{Transition("abab", 1)},
                                  {Transition("aba", 2), Transition("abab", 1)},
                                  {Transition("abaabaaba", 2)}};
    Automaton A(st, tr);
    std::cout << A;
    A.determinize();
    std::cout << A;
}

void test2(){
    vector<State> st = {State("0", true, false),
                        State("1", false, false),
                        State("2", false, true),
                        State("3", false, true)};
    vector<set<Transition>> tr = {{Transition("a", 1), Transition("b", 2)},
                                  {Transition("", 2), Transition("cx", 3)},
                                  {Transition("", 1), Transition("def", 3)},
                                  {}};
    Automaton A(st, tr);
    std::cout << A;
    A.determinize();
    std::cout << A;
}


int main() {
    determinize_automata();
}
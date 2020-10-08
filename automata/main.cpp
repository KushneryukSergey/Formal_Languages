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

    [[nodiscard]] const string& get_name() const;
    [[nodiscard]] const bool& get_is_start() const;
    [[nodiscard]] const bool& get_is_accept() const;
    void make_accept();

    void operator+=(const State&);
};

const string& State::get_name() const {
    return name;
}

const bool& State::get_is_start() const {
    return is_start;
}

const bool& State::get_is_accept() const {
    return is_accept;
}

void State::operator+=(const State& other) {
    name += "+" + other.name;
    is_start |= other.is_start;
    is_accept |= other.is_accept;
}

void State::make_accept() {
    is_accept = true;
}


class _MinState{
    string achievable;
public:
    _MinState() = delete;
    explicit _MinState(const vector<int>&);

    bool operator< (const _MinState&) const;
};

_MinState::_MinState(const vector<int>& ach) {
    for(const int& i: ach){
        achievable += char(i);
    }
}

bool _MinState::operator< (const _MinState& other) const {
    return achievable < other.achievable;
}



class _DetState{
    unsigned long long mask;
    bool is_start;
    bool is_accept;

public:
    _DetState(): mask(0), is_start(false), is_accept(false) {}
    _DetState(const unsigned long long& mask, const bool& is_start, const bool& is_accept):
        mask(mask), is_start(is_start), is_accept(is_accept){
    }

    [[nodiscard]] const unsigned long long& get_mask() const;
    [[nodiscard]] const bool& get_is_start() const;
    [[nodiscard]] const bool& get_is_accept() const;

    bool operator<(const _DetState&) const;

    _DetState operator|(const _DetState&) const;
    _DetState& operator|=(const _DetState&);
};

//оператор, чтобы можно было запихнуть все это в set
bool _DetState::operator<(const _DetState& other) const {
    return mask < other.mask;
}

_DetState _DetState::operator|(const _DetState& other) const {
    return _DetState(mask | other.mask, false, is_accept | other.is_accept);
}

_DetState& _DetState::operator|=(const _DetState& other) {
    mask |= other.mask;
    is_start = (other.mask == 0) && is_start;
    is_accept |= other.is_accept;
    return *this;
}

const unsigned long long& _DetState::get_mask() const {
    return mask;
}

const bool& _DetState::get_is_start() const {
    return is_start;
}

const bool& _DetState::get_is_accept() const {
    return is_accept;
}

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

bool Transition::operator<(const Transition& other) const {
    if(expr == other.expr){
        return finish < other.finish;
    } else {
        return expr < other.expr;
    }
}

const string& Transition::get_expr() const {
    return expr;
}

const size_t& Transition::get_finish() const {
    return finish;
}

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

void Automaton::_recalc_state_number(){
    state_number = states.size();
}

void Automaton::_recalc_transition_number(){
    transition_number = 0;
    for(const auto& current_state_transitions: transitions){
        transition_number += current_state_transitions.size();
    }
}


void Automaton::output_alphabet(std::ostream& stream) const {
    stream << "Alphabet: " << std::endl;
    stream << "| ";
    for (const auto& letter: alphabet){
        stream << letter << " | ";
    }
    stream << '\n';
}

void Automaton::output_states(std::ostream& stream) const {
    stream << "States: " << std::endl;
    for(size_t i = 0; i < states.size(); ++i){
        auto st = states[i];
        stream << i << ' ' << st.name << (st.get_is_start() ? " start": "")
                << (st.get_is_accept() ? " accept ": "") << std::endl;
    }
    stream << '\n';
}

void Automaton::output_transitions(std::ostream& stream) const {
    stream << std::endl << "Transitions: " << std::endl;
    for(size_t i = 0; i < transitions.size(); ++i){
        for(const auto& transition: transitions[i]){
            stream << i << " -> " << transition.get_finish() << ' '
                    << (transition.get_expr().empty() ? "#eps#" : transition.get_expr()) << std::endl;
        }
    }
    stream << '\n';
}


std::ostream& operator<<(std::ostream & stream, const Automaton& automaton) {
    stream << "AUTOMATON\n";

    automaton.output_alphabet(stream);
    automaton.output_states(stream);
    automaton.output_transitions(stream);

    return stream;
}

Automaton::Automaton(const vector<State>& states, const vector<set<Transition>>& transitions):
        states(states),
        transitions(transitions),
        state_number(states.size()),
        transition_number(transitions.size()){
        for(const auto& current_state_transitions: transitions){
            for(const auto& current_transition: current_state_transitions){
                for(size_t i = 0; i < current_transition.get_expr().size(); ++i){
                    alphabet.insert(current_transition.get_expr().substr(i, 1));
                }
            }
        }
        for(size_t i = 0; i < states.size(); ++i){
            if(states[i].get_is_start()){
                if(start_state == UINT32_MAX){
                    start_state = i;
                } else {
                    throw too_many_start_states_exception();
                }
            }
        }
    }

void Automaton::_make_leq_one_letter() {
    auto original_state_number = states.size();
    for(size_t current_state = 0; current_state < original_state_number; ++current_state){
        queue<Transition> long_expr_transitions;
        for(const auto& current_state_transition: transitions[current_state]){
            if(current_state_transition.get_expr().size() > 1){
                long_expr_transitions.push(current_state_transition);
            }
        }
        while(!long_expr_transitions.empty()){
            Transition current_transition = long_expr_transitions.front();
            long_expr_transitions.pop();
            _delete_transition(current_state, current_transition);
            size_t last = current_state;
            for(size_t i = 0; i < current_transition.get_expr().size(); ++i){
                size_t new_state = states.size();
                if(i + 1 == current_transition.get_expr().size()){
                    new_state = current_transition.get_finish();
                } else {
                    _add_state(std::to_string(new_state), false, false);
                }
                _add_transition(last, new_state, current_transition.get_expr().substr(i, 1));
                last = new_state;
            }
        }
    }
}

void Automaton::_remove_epsilon_transitions() {
    vector<set<Transition>> old_transitions(states.size());
    transition_number = 0;
    swap(old_transitions, transitions);
    for(size_t current_state = 0; current_state < transitions.size(); ++current_state){
        _push_epsilon_transitions_in_state(current_state, old_transitions);
    }
    for(const auto& current_state_transitions: transitions){
        transition_number += current_state_transitions.size();
    }
}

void Automaton::_classify() {
    vector<map<string, _DetState>> old_transition_packs_by_letter(transitions.size());
    map<_DetState, int> renumeration;
    vector<set<Transition>> old_transitions;
    vector<State> old_states;

    vector<_DetState> old_states_masks;
    old_states_masks.reserve(states.size());
    for(size_t i = 0; i < states.size(); ++i){
        old_states_masks.emplace_back(1u << i, states[i].get_is_start(), states[i].get_is_accept());
    }

    for(size_t i = 0; i < transitions.size(); ++i){
        for(const auto& current_transition: transitions[i]){
            old_transition_packs_by_letter[i][current_transition.get_expr()] |= old_states_masks[current_transition.get_finish()];
        }
    }

    queue<_DetState> pack_states;
    map<_DetState, bool> used;
    size_t new_state_number = 1;
    _DetState new_start_state(1u << start_state, true, states[start_state].get_is_accept());
    pack_states.push(_DetState(new_start_state));
    renumeration[new_start_state] = new_state_number;
    ++new_state_number;

    swap(old_states, states);
    state_number = 0;
    swap(old_transitions, transitions);
    transition_number = 0;

    _add_state(_build_name_by_mask(1u << start_state, old_states), true, old_states[start_state].get_is_accept());

    while(!pack_states.empty()){
        auto current_state = pack_states.front();
        pack_states.pop();
        unsigned long long current_mask;
        if(!used[current_state]){
            used[current_state] = true;
            for(const string& letter: alphabet){
                _DetState current_state_pack; // все достижимые состояния по данному символу
                current_mask = current_state.get_mask();
                for(size_t i = 0; 1u<<i <= current_mask; ++i) {
                    if(1u<<i & current_mask){
                        current_state_pack |= old_transition_packs_by_letter[i][letter];
                    }
                }
                if(current_state_pack.get_mask()){
                    if(renumeration[current_state_pack] == 0){
                        renumeration[current_state_pack] = new_state_number;
                        ++new_state_number;
                        _add_state(_build_name_by_mask(current_state_pack.get_mask(), old_states),
                                                current_state_pack.get_is_start(),
                                                current_state_pack.get_is_accept());
                    }
                    _add_transition(renumeration[current_state] - 1, renumeration[current_state_pack] - 1, letter);
                    pack_states.push(current_state_pack);
                }
            }
        }
    }
}



void Automaton::determinize() {
    if(states.size() > MAX_AUTOMATA_SIZE){
        throw too_many_states_exception();
    }
    if(is_DFA){
        return;
    }
    make_one_letter();
    _classify();
    is_DFA = true;
}

void Automaton::_push_epsilon_transitions_in_state(const size_t& current_state, const vector<set<Transition>>& old_transitions) {
    vector<bool> used(states.size(), false);
    queue<int> reachable;
    reachable.push(current_state);
    while(!reachable.empty()){
        int s = reachable.front();
        reachable.pop();
        if(used[s]){
            continue;
        }
        used[s] = true;
        transitions[current_state].insert(old_transitions[s].begin(), old_transitions[s].end());
        for(const auto& current_transition: old_transitions[s]){
            if(!current_transition.get_expr().empty()){
                break;
            }
            reachable.push(current_transition.get_finish());
            if(states[current_transition.get_finish()].get_is_accept()){
                states[current_state].make_accept();
            }
        }
    }
    auto iter = transitions[current_state].begin();
    while(iter != transitions[current_state].end() && iter->get_expr().empty()){
        auto copy_iter = iter;
        ++iter;
        transitions[current_state].erase(copy_iter);
    }
}

string Automaton::_build_name_by_mask(const unsigned long long& mask, const vector<State>& old_states, const string& separator) {
    vector<string> names;
    for(size_t i = 0; 1u<<i <= mask; ++i){
        if(1u<<i & mask){
            names.push_back(old_states[i].get_name());
        }
    }
    string new_name;
    for(size_t i = 0; i + 1 < names.size(); ++i){
        new_name += names[i] + separator;
    }
    new_name += names.back();
    return new_name;
}

void Automaton::tex_graph_print(std::ostream & stream) const {
    stream << "LaTeX code for graph of NFA/DFA \n"
              "ATTENTION: if number of state is more than 7, graph will be incorrectly displayed \n\n";
    stream << "\\begin{tikzpicture}[shorten >=1pt,node distance=2cm,on grid,auto]\n"
              "    \\tikzstyle{every state}=[fill={rgb:black,1;white,10}]\n";
    for(size_t i = 0; i < states.size(); ++i){
        auto st = states[i];
        stream << "    \\node[state" << (st.get_is_start() ? ",initial": "") << (st.get_is_accept() ? ",accepting": "") << "] ";
        stream << "(q_" << i << ")" << (i ? "[right of=q_" + std::to_string(i - 1) +" ]" : "") << " {$" <<
        st.get_name() << "$};\n";
    }
    stream << "\n    \\path[->]\n";
    for(size_t i = 0; i < transitions.size(); ++i){
        for(const auto& transition: transitions[i]){
            stream << "    (q_" << i << ") edge [" << (transition.get_finish() == i ? "loop above" : "bend right" ) <<
            "] node {$" + (transition.get_expr().empty() ? "\\varepsilon" : transition.get_expr()) + "$} (" <<
            (transition.get_finish() != i ? "q_" + std::to_string(transition.get_finish()) : "" ) << ")";
            if(i + 1 == transitions.size()){
                stream << ";\n";
            } else {
                stream << "\n";
            }
        }
    }
    stream << "\\end{tikzpicture}\n\n";
}

void Automaton::tex_transition_table_print(std::ostream & stream) const {
    vector<string> letters;
    stream << "LaTeX code for transition table in NFA/DFA \n \n";
    stream << "\\begin{tabular}{ |c|c|c| } \n"
              " \\hline\n";
    stream << "Verticle & ";
    for(const auto& i : alphabet){
        letters.push_back(i);
    }
    for(size_t i = 0; i + 1 < letters.size(); ++i){
        stream << letters[i] << " & ";
    }
    stream << letters.back() << " \\\\ \n \\hline \n";
    int cnt = 0;
    for(size_t j = 0; j < transitions.size(); ++j){
        const auto& st = transitions[j];
        stream << states[j].get_name() << " & ";
        for(size_t i = 0; i + 1 < letters.size(); ++i){
            auto transition = st.lower_bound(Transition(letters[i], -1));
            if(transition->get_expr() == letters[i]){
                stream << states[transition->get_finish()].get_name() << " & ";
            } else {
                stream << " - & ";
            }
        }
        auto transition = st.rbegin();
        if(transition->get_expr() == letters.back()){
            stream << states[transition->get_finish()].get_name() << " \\\\ \n";
        } else {
            stream << " - \\\\ \n";
        }
        ++cnt;
    }
    stream << " \\hline\n \\end{tabular} \n";
}


void Automaton::minimize(bool print_log, std::ostream& stream) {
    if(state_number > MAX_AUTOMATA_SIZE){
        throw too_many_states_exception();
    }
    if(is_minimum){
        return;
    }
    is_minimum = true;
    complete(); // после этого можем быть уверены, что для каждой буквы есть переход + они будут отсортированы по этим буквам

    vector<string> minimizing_log;

    map<_MinState, int> used;
    vector<int> type_mask(alphabet.size());
    vector<int> previous_types, current_types;

    for(const auto& st:states){
        current_types.push_back(st.get_is_accept());
        previous_types.push_back(0);
        minimizing_log.push_back(st.get_name() + " & " + std::to_string(st.get_is_accept()));
    }
    string format = "|c|c|";
    string header = "vert. & type";

    size_t types_number = 1;
    while(current_types != previous_types){
        std::swap(previous_types, current_types);
        for(const auto& letter : alphabet){
            format += "c|";
            header += "& " + letter + " ";
        }
        format += "|c|";
        header += "& type ";
        for(size_t current_state = 0; current_state < states.size(); ++current_state){
            int cnt = 0;
            for(const auto& transition: transitions[current_state]){
                type_mask[cnt] = previous_types[transition.get_finish()];
                minimizing_log[current_state] += "& " + std::to_string(previous_types[transition.get_finish()]) + " ";
                ++cnt;
            }
            _MinState current_state_mask(type_mask);
            if(used[current_state_mask] == 0){
                used[current_state_mask] = types_number;
                ++types_number;
            }
            current_types[current_state] = used[current_state_mask];
            minimizing_log[current_state] += "& " + std::to_string(current_types[current_state]) + " ";
        }
        used.clear();
        types_number = 1;
    }
    vector<set<Transition>> old_transitions;
    vector<State> old_states;

    swap(old_states, states);
    state_number = 0;
    swap(old_transitions, transitions);
    transition_number = 0;

    for(size_t i = 0; i < old_states.size(); ++i){
        if(current_types[i] > state_number){
            _add_state(old_states[i].get_name(), old_states[i].get_is_start(), old_states[i].get_is_accept());
            for(const auto& transition: old_transitions[i]){
                _add_transition(state_number - 1, current_types[transition.get_finish()] - 1, transition.get_expr());
            }
        } else {
            states[current_types[i] - 1] += old_states[i];
        }
    }

    if(!print_log){
        return;
    }
    stream << "LaTeX code for table of building minimum complete DFA \n \n";
    stream << "\\begin{tabular} {" + format + "} \n"
              " \\hline\n";
    stream << header << "\\\\ \n";
    for(const auto& s: minimizing_log){
        stream << s << "\\\\ \n";
    }
    stream << std::endl;
}

void Automaton::complete() {
    if(is_complete){
        return;
    }
    is_complete = true;
    bool complete = true;
    for(const auto& state_transitions: transitions){
        if(state_transitions.size() != alphabet.size()){
            complete = false;
        }
    }
    if(complete){
        return;
    }

    _add_state("drain", false, false);
    for(size_t i = 0; i < state_number; ++i){
        for(const auto& letter: alphabet){
            if(!_is_exist_transition_by_letter(i, letter)){
                _add_transition(i, state_number - 1, letter);
            }
        }
    }
}

bool Automaton::_is_exist_transition_by_letter(const int& start, const string& expr) {
    const auto& transition = transitions[start].lower_bound(Transition(expr, -1));
    if(transition->get_expr() == expr){
        return true;
    }
    return false;
}

void Automaton::_add_transition(const size_t& start, const size_t& finish, const string& expr) {
    size_t change = transitions[start].size();
    transitions[start].emplace(expr, finish);
    change -= transitions[start].size();
    transition_number -= change;
}

void Automaton::_delete_transition(const size_t& start, const Transition& transition) {
    size_t change = transitions[start].size();
    transitions[start].erase(transition);
    change -= transitions[start].size();
    transition_number -= change;
}

void Automaton::_add_state(const string& name, const bool& is_start, const bool& is_accept) {
    states.emplace_back(name, is_start, is_accept);
    transitions.emplace_back();
    ++state_number;
}

void Automaton::make_one_letter() {
    if(is_one_letter){
        return;
    }
    _make_leq_one_letter();
    _remove_epsilon_transitions();
    is_one_letter = true;
}

size_t Automaton::get_state_number() {
    return state_number;
}

size_t Automaton::get_transition_number() {
    return transition_number;
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
                 "If you want word to be empty, print #\n"
                 "(numbers were written when you were inputting states' information): \n";
    vector<set<Transition>> tr(size);
    int u, v;
    string word;
    for(size_t i = 0; i < trans_number; ++i){
        std::cin >> u >> v >> word;
        if (word == "#"){
            word = "";
        }
        tr[u].insert(Transition(word, v));
    }
    Automaton automaton(st, tr);

    return automaton;
}

void operate_automata(){
    Automaton automaton = input_automata();
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);
    automaton.determinize();
    std::cout << automaton << std::endl;
    std::cout << "\n\n\n";
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);
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
    auto automaton = input_automata();
    /*automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);*/
    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout /*<< automaton*/ << "\nStep 1\n\n";
    automaton.make_one_letter();
    /*automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);*/
    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout /*<< automaton*/ << "\nStep 2\n\n";
    automaton.determinize();
    /*automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);*/
    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout /*<< automaton*/ << "\nStep 3\n\n";
    automaton.complete();
    /*automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);*/
    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout /*<< automaton*/ << "\nStep 4\n\n";
    automaton.minimize(true);
    /*automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);*/

    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout << automaton << std::endl;
}
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>
#include <set>
#include <map>
#include <queue>
#include <exception>
#include <array>


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


class _MinState{
    string achievable;
public:
    _MinState() = delete;
    _MinState(const vector<int>& ach) {
        for(const int& i: ach){
            achievable += char(i);
        }
    };
    bool operator< (const _MinState& other) const {
        return achievable < other.achievable;
    }
};



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
    int finish;
public:
    Transition() = delete;
    Transition(string expr, const int& finish): expr(std::move(expr)), finish(finish) {};
    [[nodiscard]] const string& get_expr() const;
    [[nodiscard]] const int& get_finish() const;

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
const int& Transition::get_finish() const {
    return finish;
}

class Automaton{
    vector<State> state;
    vector<set<Transition>> transition;
    set<string> alphabet;
    int start_state = -1;
    bool is_one_letter = false;
    bool is_DFA = false;
    bool is_complete = false;
    bool is_minimum = false;

    static const size_t MAX_AUTOMATA_SIZE = 60;

public:
    Automaton() = delete;
    Automaton(const vector<State>&, const vector<set<Transition>>&);
    void determinize();
    void minimize(bool print_log=false, std::ostream& stream=std::cout);
    void complete();
    friend std::ostream& operator<<(std::ostream & stream, const Automaton& automaton);
    void tex_graph_print(std::ostream & stream);
    void tex_transition_table_print(std::ostream & stream);
    void tex_minimize_transition_table_print(std::ostream & stream);
    void make_one_letter();

private:
    void _make_leq_one_letter();
    void _add_transition(const int&, const int&, const string&);
    bool _is_exist_transition_by_letter(const int &start, const string &expr);
    void _remove_epsilon_transitions();
    void _push_epsilon_transitions_in_state(const int &curr_state, set<Transition> &new_transitions);
    void _classify();
    string _build_name_by_mask(const unsigned long long&);

};

std::ostream& operator<<(std::ostream & stream, const Automaton& automaton) {
    stream << "AUTOMATON" << std::endl;
    stream << "Alphabet: " << std::endl;
    stream << "| ";
    for (const auto& letter: automaton.alphabet){ //позже выделю отдельно функции вывода алфавита, состояний и переходов
        stream << letter << " | ";
    }
    stream << std::endl;
    stream << "States: " << std::endl;
    for(size_t i = 0; i < automaton.state.size(); ++i){
        auto st = automaton.state[i];
        stream << i << ' ' << st.name << (st.is_start ? " start": "") << (st.is_accept ? " accept ": "") << std::endl;
    }
    stream << std::endl << "Transitions: " << std::endl;
    for(size_t i = 0; i < automaton.transition.size(); ++i){
        for(const auto& tr: automaton.transition[i]){
            stream << i << " -> " << tr.get_finish() << ' ' << (tr.get_expr().empty() ? "#eps#" : tr.get_expr()) << std::endl;
        }
    }

    return stream;
}

Automaton::Automaton(const vector<State>& state, const vector<set<Transition>>& transition):
        state(state),
        transition(transition) {
        for(const auto& curr_state_trans: transition){
            for(const auto& curr_trans: curr_state_trans){
                for(size_t i = 0; i < curr_trans.get_expr().size(); ++i){
                    alphabet.insert(curr_trans.get_expr().substr(i, 1));
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
            if(trans.get_expr().size() > 1){
                long_trans.push(trans);
            }
        }
        while(!long_trans.empty()){
            Transition tr = long_trans.front();
            long_trans.pop();
            transition[curr_state].erase(tr);
            size_t last = curr_state;
            for(size_t i = 0; i < tr.get_expr().size(); ++i){
                size_t new_state = state.size();
                if(i + 1 == tr.get_expr().size()){
                    new_state = tr.get_finish();
                } else {
                    state.emplace_back(std::to_string(new_state), false, false);
                    transition.emplace_back(set<Transition>());
                }
                transition[last].insert(Transition(tr.get_expr().substr(i, 1), new_state));
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
    vector<map<string, _DetState>> old_transition(transition.size());
    map<_DetState, int> renumeration;
    int cnt = 2;
    vector<set<Transition>> new_transition;
    vector<State> new_state;

    vector<_DetState> old_states;
    for(size_t i = 0; i < state.size(); ++i){
        old_states.emplace_back(1u<<i, state[i].get_is_start(), state[i].get_is_accept());
    }

    for(size_t i = 0; i < transition.size(); ++i){
        for(const auto& trans: transition[i]){
            old_transition[i][trans.get_expr()] |= old_states[trans.get_finish()];
        }
    }

    queue<_DetState> pack_states;
    map<_DetState, bool> used;
    pack_states.push(_DetState(1 << start_state, true, state[start_state].get_is_accept()));
    renumeration[_DetState(1 << start_state, true, false)] = 1;

    new_state.emplace_back(_build_name_by_mask(1u<<start_state), true, state[start_state].get_is_accept());
    new_transition.emplace_back();

    while(!pack_states.empty()){
        auto st = pack_states.front();
        pack_states.pop();
        unsigned long long curr_mask;
        if(!used[st]){
            used[st] = true;
            for(const string& letter: alphabet){
                _DetState curr_state_pack; // все достижимые состояния по данному символу
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
    if(is_DFA){
        return;
    }
    make_one_letter();
    _classify();
    is_DFA = true;
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
            if(!trans.get_expr().empty()){
                break;
            }
            reachable.push(trans.get_finish());
            state[curr_state].is_accept |= state[trans.get_finish()].is_accept;
        }
    }
    auto iter = new_transitions.begin();
    while(iter != new_transitions.end() && iter->get_expr().empty()){
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
    string new_name;
    for(size_t i = 0; i + 1 < names.size(); ++i){
        //new_name += names[i] + ", ";
        new_name += names[i];
    }
    new_name += names.back();
    return new_name;
}

void Automaton::tex_graph_print(std::ostream & stream) {
    stream << "LaTeX code for graph of NFA/DFA \n"
              "ATTENTION: if number of state is more than 7, graph will be incorrectly displayed \n\n";
    stream << "\\begin{tikzpicture}[shorten >=1pt,node distance=2cm,on grid,auto]\n"
              "    \\tikzstyle{every state}=[fill={rgb:black,1;white,10}]" << std::endl;
    for(size_t i = 0; i < state.size(); ++i){
        auto st = state[i];
        stream << "    \\node[state" << (st.get_is_start() ? ",initial": "") << (st.get_is_accept() ? ",accepting": "") << "] ";
        stream << "(q_" << i << ")" << (i ? "[right of=q_" + std::to_string(i - 1) +" ]" : "") << " {$" <<
        st.get_name() << "$};" << std::endl;
    }
    stream << std::endl << "    \\path[->]" << std::endl;
    for(size_t i = 0; i < transition.size(); ++i){
        for(const auto& tr: transition[i]){
            stream << "    (q_" << i << ") edge [" << (tr.get_finish() == i ? "loop above" : "bend right" ) <<
            "] node {$" + (tr.get_expr().empty() ? "\\varepsilon" : tr.get_expr()) + "$} (" <<
            (tr.get_finish() != i ? "q_" + std::to_string(tr.get_finish()) : "" ) << ")";
            if(i + 1 == transition.size()){
                stream << ";\n";
            } else {
                stream << std::endl;
            }
        }
    }
    stream << "\\end{tikzpicture}\n\n";
}

void Automaton::tex_transition_table_print(std::ostream & stream) {
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
    for(size_t j = 0; j < transition.size(); ++j){
        const auto& st = transition[j];
        stream << state[j].get_name() << " & ";
        for(size_t i = 0; i + 1 < letters.size(); ++i){
            auto tr = st.lower_bound(Transition(letters[i], -1));
            if(tr->get_expr() == letters[i]){
                stream << state[tr->get_finish()].get_name() << " & ";
            } else {
                stream << " - & ";
            }
        }
        auto tr = st.rbegin();
        if(tr->get_expr() == letters.back()){
            stream << state[tr->get_finish()].get_name() << " \\\\ \n";
        } else {
            stream << " - \\\\ \n";
        }
        ++cnt;
    }
    stream << " \\hline\n \\end{tabular} \n";
}
/*
void Automaton::tex_minimize_transition_table_print(std::ostream & stream) {
    vector<string> letters;
    stream << "LaTeX code for transition table in NFA/DFA \n \n";
    stream << "\\begin{tabular}{ |c|c|c|c| } \n"
              " \\hline\n";
    stream << "Verticle & TYPE & ";
    for(const auto& i : alphabet){
        letters.push_back(i);
    }
    for(size_t i = 0; i + 1 < letters.size(); ++i){
        stream << letters[i] << " & ";
    }
    stream << letters.back() << " \\\\ \n \\hline \n";
    int cnt = 0;
    for(size_t j = 0; j < transition.size(); ++j){
        const auto& st = transition[j];
        stream << state[j].get_name() << " & " << state[j].get_is_accept() << " & ";
        for(size_t i = 0; i + 1 < letters.size(); ++i){
            auto tr = st.lower_bound(Transition(letters[i], -1));
            if(tr->get_expr() == letters[i]){
                stream << state[tr->get_finish()].get_is_accept() << " & ";
            } else {
                stream << "- & ";
            }
        }
        auto tr = st.rbegin();
        if(tr->get_expr() == letters.back()){
            stream << state[tr->get_finish()].get_is_accept() << " \\\\ \n";
        } else {
            stream << "- \\\\ \n";
        }
        ++cnt;
    }
    stream << " \\hline\n \\end{tabular} \n";
}
*/
void Automaton::minimize(bool print_log, std::ostream& stream) {
    if(state.size() > MAX_AUTOMATA_SIZE){
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
    vector<int> prev_types, curr_types;

    for(const auto& st:state){
        curr_types.push_back(st.get_is_accept());
        prev_types.push_back(0);
        minimizing_log.push_back(st.get_name() + " & " + std::to_string(st.get_is_accept()));
    }
    string format = "|c|c|";
    string header = "vert. & type";

    size_t type_cnt = 1;
    while(curr_types != prev_types){
        std::swap(prev_types, curr_types);
        for(const auto& letter : alphabet){
            format += "c|";
            header += "& " + letter + " ";
        }
        format += "|c|";
        header += "& type ";
        for(size_t state_number = 0; state_number < state.size(); ++state_number){
            int cnt = 0;
            for(const auto& tr: transition[state_number]){
                type_mask[cnt] = prev_types[tr.get_finish()];
                minimizing_log[state_number] += "& " + std::to_string(prev_types[tr.get_finish()]) + " ";
                ++cnt;
            }
            _MinState curr_state_mask(type_mask);
            if(used[curr_state_mask] == 0){
                used[curr_state_mask] = type_cnt;
                ++type_cnt;
            }
            curr_types[state_number] = used[curr_state_mask];
            minimizing_log[state_number] += "& " + std::to_string(curr_types[state_number]) + " ";
        }
        for(auto i : curr_types){
            std::cout << i << ' ';
        }
        std::cout << std::endl << std::endl;
        used.clear();
        type_cnt = 1;
    }
    vector<set<Transition>> new_transition;
    vector<State> new_state;
    for(size_t i = 0; i < state.size(); ++i){
        if(curr_types[i] > new_state.size()){
            new_state.push_back(state[i]);
            new_transition.emplace_back();
            for(const auto& tr: transition[i]){
                new_transition.back().emplace(tr.get_expr(), curr_types[tr.get_finish()] - 1);
            }
        } else {
            new_state[curr_types[i] - 1] += state[i];
        }
    }
    state = new_state;
    transition = new_transition;
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
    for(const auto& state_transitions: transition){
        if(state_transitions.size() != alphabet.size()){
            complete = false;
        }
    }
    if(complete){
        return;
    }

    int new_state_number = state.size();
    state.emplace_back("drain", 0, 0);
    transition.emplace_back();
    for(size_t i = 0; i < state.size(); ++i){
        for(const auto& letter: alphabet){
            if(!_is_exist_transition_by_letter(i, letter)){
                _add_transition(i, new_state_number, letter);
            }
        }
    }
}

bool Automaton::_is_exist_transition_by_letter(const int& start, const string& expr) {
    const auto& trans = transition[start].lower_bound(Transition(expr, -1));
    if(trans->get_expr() == expr){
        return true;
    }
    return false;
}

void Automaton::_add_transition(const int& start, const int& finish, const string& expr) {
    transition[start].emplace(expr, finish);
}

void Automaton::make_one_letter() {
    if(is_one_letter){
        return;
    }
    _make_leq_one_letter();
    _remove_epsilon_transitions();
    is_one_letter = true;
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
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);

    std::cout << automaton << "\nStep 1\n\n";
    automaton.make_one_letter();
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);

    std::cout << automaton << "\nStep 2\n\n";
    automaton.determinize();
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);

    std::cout << automaton << "\nStep 3\n\n";
    automaton.complete();
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);

    std::cout << automaton << "\nStep 4\n\n";
    automaton.minimize(true);
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);
    std::cout << automaton << std::endl;
}
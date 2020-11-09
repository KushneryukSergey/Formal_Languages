#include "automata.h"

//--------------------------
// Exceptions' what methods
//--------------------------
[[nodiscard]] const char* too_many_states_exception::what() const noexcept {
    return "Too many states in the automaton to calc DFA!\n";
}

[[nodiscard]] const char* too_many_start_states_exception::what() const noexcept {
    return "Too many start states in the automaton!\n";
}

[[nodiscard]] const char *incorrect_polish_expr_exception::what() const noexcept {
    return "Regular is not correct expression in polish notation!\n";
}

//-------
// State
//-------
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

//------------
// Transition
//------------
bool Transition::operator<(const Transition& other) const {
    if (expr == other.expr) {
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


//-----------
// Automaton
//-----------
void Automaton::_recalc_state_number() {
    state_number = states.size();
}

void Automaton::_recalc_transition_number() {
    transition_number = 0;
    for (const auto& current_state_transitions: transitions) {
        transition_number += current_state_transitions.size();
    }
}


void Automaton::output_alphabet(std::ostream& stream) const {
    stream << "Alphabet: " << std::endl;
    stream << "| ";
    for (const auto& letter: alphabet) {
        stream << letter << " | ";
    }
    stream << '\n';
}

void Automaton::output_states(std::ostream& stream) const {
    stream << "States: " << std::endl;
    for (size_t i = 0; i < states.size(); ++i) {
        auto st = states[i];
        stream << i << ' ' << st.name << (st.get_is_start() ? " start": "")
                << (st.get_is_accept() ? " accept ": "") << std::endl;
    }
    stream << '\n';
}

void Automaton::output_transitions(std::ostream& stream) const {
    stream << std::endl << "Transitions: " << std::endl;
    for (size_t i = 0; i < transitions.size(); ++i) {
        for (const auto& transition: transitions[i]) {
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
        transitions(transitions) {
        for (const auto& current_state_transitions: transitions) {
            for (const auto& current_transition: current_state_transitions) {
                for (size_t i = 0; i < current_transition.get_expr().size(); ++i) {
                    alphabet.insert(current_transition.get_expr().substr(i, 1));
                }
            }
        }
        for (size_t i = 0; i < states.size(); ++i) {
            if (states[i].get_is_start()) {
                if (start_state == UINT32_MAX) {
                    start_state = i;
                } else {
                    throw too_many_start_states_exception();
                }
            }
        }
        _recalc_transition_number();
        _recalc_state_number();
    }

typedef struct {
    set<size_t> in;
    set<pair<size_t, string>> out;
} AutoBlock;

Automaton::Automaton(const string& polish_expr) {
    stack<AutoBlock> blocks;
    for(size_t i = 0; i < polish_expr.size(); ++i){
        if(isspace(polish_expr[i])){
            continue;
        } else if(polish_expr[i] == '.'){
            if (blocks.size() < 2) {
                throw incorrect_polish_expr_exception();
            }
            AutoBlock right = blocks.top(); blocks.pop();
            AutoBlock left = blocks.top(); blocks.pop();
            if (right.in.size() != 1 || left.out.size() != 1) {
                size_t inter_state_number = state_number;
                _add_state(std::to_string(inter_state_number), false, false);
                for (auto state: right.in) {
                    _add_transition(inter_state_number, state, "");
                }
                for (const auto &state_n_letter: left.out) {
                    _add_transition(state_n_letter.first, inter_state_number, state_n_letter.second);
                }
                left.out = right.out;
                blocks.push(left);
            } else {
                auto left_out_it = left.out.begin();
                auto right_in_it = right.in.begin();
                _add_transition(left_out_it->first, *right_in_it, left_out_it->second);
                left.out = right.out;
                blocks.push(left);
            }
        } else if (polish_expr[i] == '+'){
            if (blocks.size() < 2) {
                throw incorrect_polish_expr_exception();
            }
            AutoBlock first = blocks.top(); blocks.pop();
            AutoBlock second = blocks.top(); blocks.pop();
            first.in.insert(second.in.begin(), second.in.end());
            first.out.insert(second.out.begin(), second.out.end());
            blocks.push(first);
        } else if (polish_expr[i] == '*') {
            if (blocks.empty()) {
                throw incorrect_polish_expr_exception();
            }
            AutoBlock& block = blocks.top();
            if (block.in.size() != 1 || block.out.size() != 1) {
                size_t inter_state_number = state_number;
                _add_state(std::to_string(inter_state_number), false, false);
                for (auto state: block.in) {
                    _add_transition(inter_state_number, state, "");
                }
                for (const auto &state_n_letter: block.out) {
                    _add_transition(state_n_letter.first, inter_state_number, state_n_letter.second);
                }
                block.in.erase(block.in.begin(), block.in.end());
                block.in.insert(inter_state_number);
                block.out.erase(block.out.begin(), block.out.end());
                block.out.insert(std::make_pair(inter_state_number, ""));
            } else {
                auto state_n_letter = *(block.out.begin());
                _add_transition(state_n_letter.first, *(block.in.begin()), state_n_letter.second);
                block.out.erase(block.out.begin(), block.out.end());
                for(auto state : block.in){
                    block.out.insert(std::make_pair(state, ""));
                }
            }
        } else {
            alphabet.insert(polish_expr.substr(i, 1));
            size_t new_state_number = state_number;
            _add_state(std::to_string(new_state_number), false, false);
            AutoBlock block;
            block.in.insert(new_state_number);
            block.out.insert(std::make_pair(new_state_number, polish_expr.substr(i, 1)));
            blocks.push(block);
        }
    }
    if(blocks.size() != 1){
        throw incorrect_polish_expr_exception();
    }
    AutoBlock &block = blocks.top();
    size_t final_state_number = state_number;
    _add_state(std::to_string(final_state_number), false, true);
    for (const auto &state_n_letter: block.out) {
        _add_transition(state_n_letter.first, final_state_number, state_n_letter.second);
    }

    size_t initial_state_number = state_number;
    start_state = initial_state_number;
    _add_state(std::to_string(initial_state_number), true, false);
    for (const auto &state: block.in) {
        _add_transition(initial_state_number, state, "");
    }
}

void Automaton::_make_leq_one_letter() {
    auto original_state_number = states.size();
    for (size_t current_state = 0; current_state < original_state_number; ++current_state) {
        queue<Transition> long_expr_transitions;
        for (const auto& current_state_transition: transitions[current_state]) {
            if (current_state_transition.get_expr().size() > 1) {
                long_expr_transitions.push(current_state_transition);
            }
        }
        while (!long_expr_transitions.empty()) {
            Transition current_transition = long_expr_transitions.front();
            long_expr_transitions.pop();
            _delete_transition(current_state, current_transition);
            size_t last = current_state;
            for (size_t i = 0; i < current_transition.get_expr().size(); ++i) {
                size_t new_state = states.size();
                if (i + 1 == current_transition.get_expr().size()) {
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
    for (size_t current_state = 0; current_state < transitions.size(); ++current_state) {
        _push_epsilon_transitions_in_state(current_state, old_transitions);
    }
    for (const auto& current_state_transitions: transitions) {
        transition_number += current_state_transitions.size();
    }
}

void Automaton::_push_epsilon_transitions_in_state(const size_t& current_state, const vector<set<Transition>>& old_transitions) {
    vector<bool> used(states.size(), false);
    queue<int> reachable;
    reachable.push(current_state);
    while (!reachable.empty()) {
        int s = reachable.front();
        reachable.pop();
        if (used[s]) {
            continue;
        }
        used[s] = true;
        transitions[current_state].insert(old_transitions[s].begin(), old_transitions[s].end());
        for (const auto& current_transition: old_transitions[s]) {
            if (!current_transition.get_expr().empty()) {
                break;
            }
            reachable.push(current_transition.get_finish());
            if (states[current_transition.get_finish()].get_is_accept()) {
                states[current_state].make_accept();
            }
        }
    }
    auto iter = transitions[current_state].begin();
    while (iter != transitions[current_state].end() && iter->get_expr().empty()) {
        auto copy_iter = iter;
        ++iter;
        transitions[current_state].erase(copy_iter);
    }
}

void Automaton::_add_transition(const size_t& start, const size_t& finish, const string& expr) {
    size_t change = transitions[start].size();
    transitions[start].emplace(expr, finish);
    change -= transitions[start].size();
    transition_number += change;
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
    if (is_one_letter) {
        return;
    }
    _make_leq_one_letter();
    _remove_epsilon_transitions();
    is_one_letter = true;
}

void Automaton::_remove_unreachable(){
    vector<bool> used(state_number, false);
    queue<size_t> reachable;
    reachable.push(start_state);
    while(!reachable.empty()) {
        size_t s = reachable.front();
        reachable.pop();
        if (used[s]) {
            continue;
        }
        used[s] = true;
        for (const auto& transition: transitions[s]) {
            if(!used[transition.get_finish()]){
                reachable.push(transition.get_finish());
            }
        }
    }
    for(size_t i = 0; i < state_number; ++i){
        if(!used[i]){
            transitions[i].erase(transitions[i].begin(), transitions[i].end());
        }
    }
}

void Automaton::remove_useless() {
    _remove_unreachable();
}

size_t Automaton::get_state_number() const {
    return state_number;
}

size_t Automaton::get_transition_number() const {
    return transition_number;
}

void Automaton::_find_reachable_state_by_word(const size_t& from, const string& word, set<size_t>& save){
    queue<pair<size_t, size_t>> reached_states; // reached state and index of letter
    set<pair<size_t, size_t>> used;
    reached_states.push(std::make_pair(from, 0));
    while(!reached_states.empty()){
        pair<size_t, size_t> state = reached_states.front(); reached_states.pop();

        if(used.find(state) == used.end()){
            used.insert(state);
            if (state.second == word.size()) {
                save.insert(state.first);
                continue;
            }
            for(const auto& transition : transitions[state.first]){
                if(transition.get_expr()[0] == word[state.second]){
                    if(used.find(std::make_pair(transition.get_finish(), state.second + 1)) == used.end()){
                        reached_states.push(std::make_pair(transition.get_finish(), state.second + 1));
                    }
                }
            }
        }

    }
}

int find_longest_path_in_directed_graph(const vector<set<size_t>>& graph){
    size_t vertex_number = graph.size();
    vector<bool> used(vertex_number, false);

    // сheck if there is a cycle
    vector<CycleFinder::COLOR> color(graph.size(), CycleFinder::WHITE);
    CycleFinder finder(graph, color);
    for(size_t start = 0; start < vertex_number; ++start){
        if(finder.dfs_cycle(start)){
            return -1; // if exists, longest path is infinite
        }
    }

    TopSorter sorter(graph);
    sorter.topological_sort();
    vector<int> answer(graph.size(), 0);
    for(size_t i = 0; i < graph.size(); ++i){
        for(size_t vertex : graph[sorter.sorted[i]]){
            answer[vertex] = std::max(answer[vertex], answer[sorter.sorted[i]] + 1);
        }
    }

    int ans = 0;
    for(int depth : answer){
        ans = std::max(ans, depth);
    }
    return ans;
}

int Automaton::solve_workshop_problem_for_automaton(const string& word) {
    make_one_letter();
    remove_useless();
    vector<set<size_t>> graph(state_number);
    for(size_t state = 0; state < state_number; ++state){
        _find_reachable_state_by_word(state, word, graph[state]);
    }
    return find_longest_path_in_directed_graph(graph);
}


//---------------
//  CycleFinder
//---------------
bool CycleFinder::dfs_cycle(const size_t &start) {
    if (color[start] == BLACK)
        return false;
    color[start] = GREY;
    int curr_ans;
    for (int vertex : graph[start]) {
        if (color[vertex] == GREY) {
            return true;
        }
        if (color[vertex] == WHITE) {
            if (dfs_cycle(vertex)) {
                return true;
            }
        }
    }
    color[start] = BLACK;
    return false;
}

//---------------
//   TopSorter
//---------------
TopSorter::TopSorter(const vector<set<size_t>> &graph) : graph(graph) {
    used.resize(graph.size(), false);
}

void TopSorter::_dfs_ts(const size_t &start) {
    used[start] = true;
    for (size_t to : graph[start]) {
        if (!used[to])
            _dfs_ts(to);
    }
    sorted.push_back(start);
}

void TopSorter::topological_sort() {
    for (int vertex = 0; vertex < graph.size(); ++vertex)
        if (!used[vertex])
            _dfs_ts(vertex);
    reverse(sorted.begin(), sorted.end());
}
#include "automata.h"

[[nodiscard]] const char* too_many_states_exception::what() const noexcept {
    return "Too many states in the automaton to calc DFA!\n";
}

[[nodiscard]] const char* too_many_start_states_exception::what() const noexcept {
    return "Too many start states in the automaton!\n";
}



// State

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



//_MinState

_MinState::_MinState(const vector<size_t>& ach) {
    for (const size_t& i: ach) {
        achievable += char(i);
    }
}

bool _MinState::operator< (const _MinState& other) const {
    return achievable < other.achievable;
}



//_DetState


_DetState::_DetState(): mask(0), is_start(false), is_accept(false) {}

_DetState::_DetState(const unsigned long long& mask, const bool& is_start, const bool& is_accept):
        mask(mask), is_start(is_start), is_accept(is_accept) {
}

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



//Transition

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



//Automaton

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

void Automaton::_classify() {
    vector<map<string, _DetState>> old_transition_packs_by_letter(transitions.size());
    map<_DetState, int> renumeration;
    vector<set<Transition>> old_transitions;
    vector<State> old_states;

    vector<_DetState> old_states_masks;
    old_states_masks.reserve(states.size());
    for (size_t i = 0; i < states.size(); ++i) {
        old_states_masks.emplace_back(1u << i, states[i].get_is_start(), states[i].get_is_accept());
    }

    for (size_t i = 0; i < transitions.size(); ++i) {
        for (const auto& current_transition: transitions[i]) {
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

    while (!pack_states.empty()) {
        auto current_state = pack_states.front();
        pack_states.pop();
        unsigned long long current_mask;
        if (!used[current_state]) {
            used[current_state] = true;
            for (const string& letter: alphabet) {
                _DetState current_state_pack; // все достижимые состояния по данному символу
                current_mask = current_state.get_mask();
                for (size_t i = 0; 1u<<i <= current_mask; ++i) {
                    if (1u<<i & current_mask) {
                        current_state_pack |= old_transition_packs_by_letter[i][letter];
                    }
                }
                if (current_state_pack.get_mask()) {
                    if (renumeration[current_state_pack] == 0) {
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
    if (states.size() > MAX_AUTOMATA_SIZE) {
        throw too_many_states_exception();
    }
    if (is_DFA) {
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

string Automaton::_build_name_by_mask(const unsigned long long& mask, const vector<State>& old_states, const string& separator) {
    vector<string> names;
    for (size_t i = 0; 1u<<i <= mask; ++i) {
        if (1u<<i & mask) {
            names.push_back(old_states[i].get_name());
        }
    }
    string new_name;
    for (size_t i = 0; i + 1 < names.size(); ++i) {
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
    for (size_t i = 0; i < states.size(); ++i) {
        auto st = states[i];
        stream << "    \\node[state" << (st.get_is_start() ? ",initial": "") << (st.get_is_accept() ? ",accepting": "") << "] ";
        stream << "(q_" << i << ")" << (i ? "[right of=q_" + std::to_string(i - 1) +" ]" : "") << " {$" <<
        st.get_name() << "$};\n";
    }
    stream << "\n    \\path[->]\n";
    for (size_t i = 0; i < transitions.size(); ++i) {
        for (const auto& transition: transitions[i]) {
            stream << "    (q_" << i << ") edge [" << (transition.get_finish() == i ? "loop above" : "bend right" ) <<
            "] node {$" + (transition.get_expr().empty() ? "\\varepsilon" : transition.get_expr()) + "$} (" <<
            (transition.get_finish() != i ? "q_" + std::to_string(transition.get_finish()) : "" ) << ")";
            if (i + 1 == transitions.size()) {
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
    for (const auto& i : alphabet) {
        letters.push_back(i);
    }
    for (size_t i = 0; i + 1 < letters.size(); ++i) {
        stream << letters[i] << " & ";
    }
    stream << letters.back() << " \\\\ \n \\hline \n";
    int cnt = 0;
    for (size_t j = 0; j < transitions.size(); ++j) {
        const auto& st = transitions[j];
        stream << states[j].get_name() << " & ";
        for (size_t i = 0; i + 1 < letters.size(); ++i) {
            auto transition = st.lower_bound(Transition(letters[i], 0));
            if (transition->get_expr() == letters[i]) {
                stream << states[transition->get_finish()].get_name() << " & ";
            } else {
                stream << " - & ";
            }
        }
        auto transition = st.rbegin();
        if (transition->get_expr() == letters.back()) {
            stream << states[transition->get_finish()].get_name() << " \\\\ \n";
        } else {
            stream << " - \\\\ \n";
        }
        ++cnt;
    }
    stream << " \\hline\n \\end{tabular} \n";
}


void Automaton::minimize(bool print_log, std::ostream& stream) {
    if (state_number > MAX_AUTOMATA_SIZE) {
        throw too_many_states_exception();
    }
    if (is_minimum) {
        return;
    }
    is_minimum = true;
    complete(); // после этого можем быть уверены, что для каждой буквы есть переход + они будут отсортированы по этим буквам

    vector<string> minimizing_log;

    map<_MinState, int> used;
    vector<size_t> type_mask(alphabet.size());
    vector<size_t> previous_types, current_types;

    for (const auto& st:states) {
        current_types.push_back(st.get_is_accept());
        previous_types.push_back(0);
        minimizing_log.push_back(st.get_name() + " & " + std::to_string(st.get_is_accept()));
    }
    string format = "|c|c|";
    string header = "vert. & type";

    size_t types_number = 1;
    while (current_types != previous_types) {
        std::swap(previous_types, current_types);
        for (const auto& letter : alphabet) {
            format += "c|";
            header += "& " + letter + " ";
        }
        format += "|c|";
        header += "& type ";
        for (size_t current_state = 0; current_state < states.size(); ++current_state) {
            int cnt = 0;
            for (const auto& transition: transitions[current_state]) {
                type_mask[cnt++] = previous_types[transition.get_finish()];
                minimizing_log[current_state] += "& " + std::to_string(previous_types[transition.get_finish()]) + " ";
            }
            _MinState current_state_mask(type_mask);
            if (used[current_state_mask] == 0) {
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

    for (size_t i = 0; i < old_states.size(); ++i) {
        if (current_types[i] > state_number) {
            _add_state(old_states[i].get_name(), old_states[i].get_is_start(), old_states[i].get_is_accept());
            for (const auto& transition: old_transitions[i]) {
                _add_transition(state_number - 1, current_types[transition.get_finish()] - 1, transition.get_expr());
            }
        } else {
            states[current_types[i] - 1] += old_states[i];
        }
    }

    if (!print_log) {
        return;
    }
    stream << "LaTeX code for table of building minimum complete DFA \n \n";
    stream << "\\begin{tabular} {" + format + "} \n"
              " \\hline\n";
    stream << header << "\\\\ \n";
    for (const auto& s: minimizing_log) {
        stream << s << "\\\\ \n";
    }
    stream << std::endl;
}

void Automaton::complete() {
    if (is_complete) {
        return;
    }
    is_complete = true;
    bool complete = true;
    for (const auto& state_transitions: transitions) {
        if (state_transitions.size() != alphabet.size()) {
            complete = false;
        }
    }
    if (complete) {
        return;
    }

    _add_state("drain", false, false);
    for (size_t i = 0; i < state_number; ++i) {
        for (const auto& letter: alphabet) {
            if (!_is_exist_transition_by_letter(i, letter)) {
                _add_transition(i, state_number - 1, letter);
            }
        }
    }
}

bool Automaton::_is_exist_transition_by_letter(const int& start, const string& expr) {
    const auto& transition = transitions[start].lower_bound(Transition(expr, 0));
    return transition->get_expr() == expr;
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
    if (is_one_letter) {
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
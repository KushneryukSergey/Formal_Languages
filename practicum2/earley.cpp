#include "earley.h"


[[nodiscard]] const char* non_terminals_in_word_exception::what() const noexcept {
    return "Non-terminal is detected in word\n";
}

[[nodiscard]] const char* non_alphabet_symbol_exception::what() const noexcept {
    return "Word contains symbols which are not presented in alphabet\n";
}

bool ContextFreeGrammarRule::operator<(const ContextFreeGrammarRule& other) const {
    if (non_terminal == other.non_terminal) {
        return expression < other.expression;
    }
    return non_terminal < other.non_terminal;
}

bool EarleySituation::operator<(const EarleySituation& other) const {
    if (rule_number == other.rule_number) {
        if (point_pos == other.point_pos) {
            return prev_pos < other.prev_pos;
        }
        return point_pos < other.point_pos;
    }
    return rule_number < other.rule_number;
}

EarleySituation EarleySituation::next() {
    return EarleySituation{rule_number, point_pos + 1, prev_pos};
}


EarleySolver::EarleySolver(const char& start,
                           const vector<ContextFreeGrammarRule>& rules):
        _start(start), _rules(rules) {
    for(const auto& rule: rules) {
        _non_terminals.insert(rule.non_terminal);
    }
    for (const auto& rule: rules) {
        for (auto alpha: rule.expression) {
            if (!_is_non_terminal(alpha))
                _alphabet.insert(alpha);
        }
    }
    ContextFreeGrammarRule new_rule{spec_symbol, string(1, _start)};
    _rules.push_back(new_rule);
    sort(_rules.begin(), _rules.end());
    for (auto non_terminal: _non_terminals) {
        _first_rule[non_terminal] = -1;
    }
    for (int i = 0; i < _rules.size(); ++i) {
        if (_first_rule[_rules[i].non_terminal] == -1) {
            _first_rule[_rules[i].non_terminal] = i;
        }
    }
}


void EarleySolver::_scan(const size_t& pos, char symbol) {
    for (auto situation: _d[pos]) {
        if (_is_complete_possible(situation))
            continue;
        if (_next_symbol(situation) == symbol) {
            _add_situation(pos + 1, situation.next());
        }
    }
}

void EarleySolver::_predict(const size_t& pos) {
    size_t todo = _current.size();
    for (size_t i = 0; i < todo; ++i) {
        auto& situation = _current.front();
        if (_is_complete_possible(situation)) {
            _current.push(situation);
        } else if (_is_non_terminal(_next_symbol(situation))) {
            char symbol = _next_symbol(situation);
            int it = _first_rule[symbol];
            if (it != -1) {
                while (it != _rules.size() && _rules[it].non_terminal == symbol) {
                    EarleySituation new_situation{(size_t)(it++), 0, pos};
                    _add_situation(pos, new_situation);
                }
            }
        }
        _current.pop();
    }
}


void EarleySolver::_complete(const size_t& pos) {
    while (!_current.empty()) {
        auto& situation = _current.front();
        if (!_is_complete_possible(situation)) {
            _current.pop();
            continue;
        }
        
        char symbol = _rules[situation.rule_number].non_terminal;
        for (auto prev_situation: _d[situation.prev_pos]) {
            if (_next_symbol(prev_situation) == symbol) {
                _add_situation(pos, prev_situation.next());
            }
        }
        _current.pop();
    }
}

bool EarleySolver::check_word(const string& word) {
    for (auto symbol: word) {
        if (_is_non_terminal(symbol))
            throw non_terminals_in_word_exception();
        if (_alphabet.find(symbol) == _alphabet.end())
            throw non_alphabet_symbol_exception();
    }
    while(!_current.empty())
        _current.pop();
    while (!_next.empty())
        _next.pop();
    _d.clear();
    
    EarleySituation start_state{(size_t)_first_rule[spec_symbol],
                                0,
                                0};
    _current.push(start_state);
    _d.resize(word.size() + 1);
    _d[0].insert(start_state);
    
    while (!_current.empty()) {
        _predict(0);
        _complete(0);
        swap(_current, _next);
    }
    
    for (int i = 1; i < _d.size(); ++i) {
        _scan(i - 1, word[i - 1]);
        swap(_current, _next);
        while (!_current.empty()) {
            _predict(i);
            _complete(i);
            swap(_current, _next);
        }
    }
    if (_d[word.size()].find(start_state.next()) != _d[word.size()].end()) {
        return true;
    }
    return false;
}

char EarleySolver::_next_symbol(const EarleySituation& state) {
    return _rules[state.rule_number].expression[state.point_pos];
}

bool EarleySolver::_is_complete_possible(const EarleySituation& state) {
    return _rules[state.rule_number].expression.size() == state.point_pos;
}

bool EarleySolver::_is_non_terminal(const char& symbol) {
    return _non_terminals.find(symbol) != _non_terminals.end();
}

void EarleySolver::_add_situation(const size_t& pos, const EarleySituation& situation) {
    if (_d[pos].find(situation) == _d[pos].end()) {
        _next.push(situation);
        _d[pos].insert(situation);
    }
}

#include <iostream>
#include "earley.h"
#include <vector>

using std::cout;
using std::cin;

void input_rules(vector<ContextFreeGrammarRule>& rules) {
    rules.clear();
    size_t N;
    cout << "Input number of rules: ";
    cin >> N;
    cout << "Input rules of context-free grammar as follows:\n"
            "\"<non-terminal> <expression>\", where\n"
            " * Non-terminal is one symbol \n"
            " * Expression contains non-terminals and letters \n"
            "For empty expression use # (hashtag) symbol\n"
            "E.g. S #; T TaTb\n";
    char c;
    string expr;
    for (int i = 0; i < N; ++i) {
        cin >> c >> expr;
        rules.push_back(ContextFreeGrammarRule{c, expr == "#" ? "" : expr});
    }
}

int main() {
    vector<ContextFreeGrammarRule> rules;
    input_rules(rules);
    char S;
    cout << "Input start non-terminal symbol: ";
    cin >> S;
    EarleySolver solver(S, rules);
    
    string word;
    size_t N;
    cout << "Input number of words to check: ";
    cin >> N;
    cout << "Input words: \n";
    
    for (size_t i = 0; i < N; ++i) {
        cin >> word;
        word = word == "#" ? "" : word;
        try {
            bool answer = solver.check_word(word);
            cout << word << " : " << (answer ? "YES\n" : "NO\n");
        } catch (std::exception& exc) {
            cout << "Exception: " << exc.what();
        }
    }
    return 0;
}

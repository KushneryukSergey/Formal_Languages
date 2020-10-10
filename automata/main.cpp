//
// Created by skushneryuk on 09/10/2020.
//

#include "automata.h"

Automaton input_automata(){
    size_t size, trans_number;
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
    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout << automaton << "\nStep 1\n\n";
    automaton.make_one_letter();
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);
    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout << automaton << "\nStep 2\n\n";
    automaton.determinize();
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);
    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout << automaton << "\nStep 3\n\n";
    automaton.complete();
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);
    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout << automaton << "\nStep 4\n\n";
    automaton.minimize(true);
    automaton.tex_transition_table_print(std::cout);
    automaton.tex_graph_print(std::cout);

    std::cout << automaton.get_state_number() << ' ' << automaton.get_transition_number() << "\n";

    std::cout << automaton << std::endl;
}
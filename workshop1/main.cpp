//
// Created by skushneryuk on 09/10/2020.
//

#include "automata.h"

int main() {
    string expr, word;
    std::cin >> expr >> word;
    try{
        Automaton automaton(expr);
        int answer = automaton.solve_workshop_problem_for_automaton(word);
        if(answer == -1){
            std::cout << "INF" << std::endl;
        } else {
            std::cout << answer << std::endl;
        }
    } catch (std::exception& ex){
        std::cout << "ERROR";
        return 0;
    }
}
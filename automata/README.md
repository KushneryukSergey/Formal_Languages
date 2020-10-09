# Automata
This code works under the assumption that we interested in NFA and DFA with number of states less then 60. I do not intent to do code for other cases

Tests coverage is not so big because I didn't came up to idea how to mock std::stream ((

![sad puppy image](extra/puppy.jpg)

## How to run:
1. Clone this repo
2. Run this command in terminal in **automata** directory
```
mkdir build && cd build
cmake .. && make
```
If you want to check tests coverage use `make testing` in **build** and check **coverage report** folder

All the information, how to input info about state, transitions etc. will be written by program

Enjoy


## Supported features
 * Input and determinize automata
 * Printing minimization log
 * Printing <img src="https://latex.codecogs.com/png.latex?\text { \LaTeX{} }" /> code for tklz library to make automaton graph and table of transitions automaton

> #### See future updates!


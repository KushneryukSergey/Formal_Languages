# Workshop 1
I had following problem:

> Suppose we have regular expression in polish notation defining language L 
> and some word x. Find the biggest k such that x^k is a substring of some
> word from language L

This code uses lightweight and reformatted version of automata library (without
determinization, minimization and tex-printings), that will be updated soon

## How to run:
1. Clone this repo
2. Run this command in terminal in **automata** directory
```shell
mkdir build && cd build
cmake .. && make
```
If you want to check tests coverage use `make testing` after previous commands
in **build** and check **coverage report** folder

You should input regular expression in polish notation and some word after space

Enjoy!

> #### See future updates!


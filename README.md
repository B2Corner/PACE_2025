# Intro

This is the code for a PACE 2025 Challenge submission, Dominating Set - Heuristic track.

# Installation and usage

The solver is written in C++ and does not have any external dependencies. All the code is in the file ```main.cpp```. To install,

1) Clone this repository
2) Compile with any C++ compiler (C++11 support is required):
```c++ ./main.cpp -o solver```
3) The solver can now be run by invoking the binary:
```./solver```
   I/O is performed via standard input / output.
   

# Solver description

Broadly speaking, the solver performs a local search procedure. At all times, it maintains a feasible solution that is returned upon receiveing SIGTERM. Initially, the soluton contains all vertices in the graph but those that dominate a subset of what some other vertex dominates (matching sets should be handled carefully).

The main search procedure goes as follows:
1) Vertices are considered in random order; if it is possible to remove the current vertex so that the solution remains feasible, the vertex is removed. As a result, the solution is now locally optimal (that is, no vertex can be removed without making the solution invalid).
2) After that, some predefined amount of times a random vertex is selected. If the vertex is not in the answer, it is added there.

Throughout the procedure, the best found solution is maintained. Moreover, if no improvement (or at least a solution of the same size as the best one) is found for 3 iterations straight, the state is reset to the best solution.

The amount of newly introduced vertices starts at 1% of the total number of vertices and decreases dynamically (that is, if there were relatively few improvements in the last 100 iterations, it's time to decrease the amount of vertices added).

The main non-trivial part of the solution is making the above go as fast as possible, since more iterations = more chance to find a good solution. To accomplish this, several counters are maintained for each vertex, which allow to quickly understand whether a vertex can be removed right now, as well as get an exhaustive list of potentially removable vertices after new vertices have been added in step 2 above. Also, apparently it can be beneficial to sometimes not shuffle the candidates for removal before step 1, because on step 2 a certain amount of randomness is already introduced.
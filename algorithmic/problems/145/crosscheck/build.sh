#!/bin/bash
# Builds the new checker (../checker.cpp), the original checker and the cross-check tools.
set -e
cd "$(dirname "$0")"
INC=../../../judge/include
g++ -O2 -pipe -std=gnu++17 -I $INC -o chk_new ../checker.cpp            # the checker in this branch (same flags as gojudge.js)
g++ -O2 -pipe -std=gnu++17 -I $INC -o chk_old checker_official.cpp     # the original checker (can run for a very long time)
g++ -O2 -std=gnu++17 -o dpcount dpcount.cpp          # ./dpcount grid [NR NC]   exact loop count by plug DP (any size)
g++ -O2 -std=gnu++17 -o hybridcount hybridcount.cpp  # ./hybridcount grid [budget]  DFS stage + DP fallback with CPU times
g++ -O2 -std=gnu++17 -o naive naive.cpp              # ./naive grid NR NC       exhaustive 2^E enumeration (small grids)
g++ -O2 -std=gnu++17 -o count count.cpp              # ./count grid LIMIT       independent brute-force DFS (stores solutions)
g++ -O2 -std=gnu++17 -o count_ns count_ns.cpp        # same without storing solutions (LIMIT up to 1e6+)
echo built

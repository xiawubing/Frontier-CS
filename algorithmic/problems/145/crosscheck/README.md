# Cross-check kit for the problem 145 checker

Companion to the pull request that replaces the solution enumeration in `../checker.cpp`
(brute-force DFS with a node budget, then a plug DP).  Everything needed to re-run the
validation is here; nothing in this directory is used by the judge.

## Build

```
./build.sh          # needs g++; testlib.h is taken from ../../../judge/include
```

| binary | what it is |
|---|---|
| `chk_new` | `../checker.cpp` from this branch, compiled with the judge's flags |
| `chk_old` | `checker_official.cpp`, the original checker |
| `dpcount` | `./dpcount grid [NR NC]` – exact number of valid loops by plug DP, any grid size |
| `hybridcount` | `./hybridcount grid [budget]` – DFS stage then DP fallback, prints CPU time of each stage |
| `naive` | `./naive grid NR NC` – exhaustive 2^E enumeration, the ground truth for small grids |
| `count`, `count_ns` | `./count grid LIMIT` – an independent brute-force DFS (edge order, path-endpoint tracking); `_ns` does not store solutions |

Grid files are 12 text lines (or NR lines), `' '` = no clue, `0`–`3` = clue, exactly like a contestant output.

## Checks

| script | what it compares | scale | result file |
|---|---|---|---|
| `./verify_trials.sh` | old vs new checker: verdict line and exit code on every real output in `samples/` (old checker capped at 10 s CPU like the judge) | 30 outputs | `results/trial_verdicts.txt` |
| `python3 duipai_small.py 3000 1` | `naive` (2^E) vs `dpcount` vs `dpcount` on the transposed grid | 3000 random small grids, 17 sizes up to 3×3 / 6×1 | `results/duipai_small.txt` |
| `python3 duipai_12.py 400 7` (and seed 8) | `count` (exact, LIMIT 1e5) vs `dpcount` vs `dpcount` on the grid rotated 90° | 800 random 12×12 grids on the official template: projections of random loops, half of them perturbed | `results/duipai_12.txt`, `duipai_12b.txt` |
| `python3 duipai_loose.py 100 21` (and seed 22) | same, on loosely constrained grids (few `0` clues, the style contestants submit), `count_ns` LIMIT 1e6 | 200 grids | `results/duipai_loose.txt`, `duipai_loose_b.txt` |
| `python3 duipai_hybrid.py template 400 7` … | `hybridcount` (what the checker does) vs `dpcount` on min(count, 6) | 1400 grids | `results/duipai_hybrid.txt` |

## Results

* `verify_trials.sh`: the old checker finishes within 10 s CPU on 10 of the 30 real outputs
  (9 scored, 1 format error); on all 10 the new checker's verdict line and exit code are identical.
  On the other 20 the old checker is killed (it was still running after 600 s CPU on several of them),
  which the judge scores as 0; 19 of those 20 outputs have at least one valid solution (true scores
  1.8 – 60.7), the remaining one (`raw_N7ACqQz.out`) really has none.  The new checker answers all 30
  in ≤ 0.15 s CPU.
* min(count, 6) of the new checker agrees with the independent DFS (`count`, run to completion, up to 50 min)
  on all 30 real outputs.  For `raw_FvvShgn.out` the DFS in its default edge order does not find 6 solutions
  in 50 minutes although the grid has 133,179,472 of them (a pathological search order); running it on the
  vertically flipped and on the transposed grid finds 6 within a second (`results/dfs_*_FvvShgn.txt`).
* Random grids: 0 mismatches in every check above – 3000 small grids against exhaustive enumeration,
  1000 template grids (864 compared as exact counts, 89 of them with 1–6 solutions, i.e. inside the scoring
  buckets), 1400 grids hybrid-vs-DP, and the DP count is invariant under all 8 grid symmetries
  (each symmetry gives the DP a different frontier order).
* Cost of the new checker on the real outputs: ≤ 0.15 s CPU, ≤ 14 MB (judge limits: 10 s, 256 MB).

## Samples

`samples/raw_*.out` are 30 real contestant outputs (as captured from the judge, input `0`), `1.in`/`1.ans` are
the problem's test data.  `raw_b8VoXCz.out` is a format error, `raw_N7ACqQz.out` has no valid solution,
`raw_GKtmkcG.out`/`raw_4rWMtgo.out`/… have exactly one, the rest have six or more.

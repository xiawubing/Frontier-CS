import random, sys, time
from p145lib import *
n = int(sys.argv[1]); seed = int(sys.argv[2]) if len(sys.argv) > 2 else 1
rng = random.Random(seed)
mism = 0; sizes = {}
t0 = time.time()
for it in range(n):
    g, NR, NC = random_small_grid(rng)
    path = 'rs_%d.txt' % (seed)
    write_grid(path, g)
    a = run_naive(path, NR, NC)
    b, _, _ = run_dp(path, NR, NC)
    # symmetry: transposed grid must give same DP count
    write_grid(path + '.t', transp(g)); c, _, _ = run_dp(path + '.t', NC, NR)
    sizes[(NR,NC)] = sizes.get((NR,NC), 0) + 1
    if a != b or a != c:
        mism += 1
        print('MISMATCH', NR, NC, 'naive=%d dp=%d dp_transposed=%d' % (a, b, c), repr(g), flush=True)
print('small-grid duipai: %d grids, %d mismatches, %.0fs' % (n, mism, time.time() - t0))
print('sizes:', sorted(sizes.items()))

import random, sys, time, collections
from p145lib import *
n = int(sys.argv[1]); seed = int(sys.argv[2]) if len(sys.argv) > 2 else 7
LIM = 1000000; TO = 15
rng = random.Random(seed)
stats = collections.Counter(); mism = 0; rows = []
t0 = time.time()
for it in range(n):
    g, tag = random_loose_grid(rng)
    path = 'rl_%d.txt' % seed
    write_grid(path, g)
    exact, c6, tdp = run_dp(path)
    # symmetry self-check
    write_grid(path + '.t', rot90(g)); ex2, _, _ = run_dp(path + '.t')
    dfs = run_dfs_ns(path, LIM, TO)
    if dfs is None:
        dfs6 = run_dfs(path, 6, TO)
        kind = 'dfs6' if dfs6 is not None else 'dfs-timeout'
        ok = (dfs6 is None) or (dfs6 == c6)
        cmp = 'capped6 dfs=%s dp=%d' % (dfs6, c6)
    elif dfs >= LIM:
        kind = 'dfs>=LIM'; ok = (exact is None or exact >= LIM); cmp = 'dfs>=%d dp=%s' % (LIM, exact)
    else:
        kind = 'exact'; ok = (exact == dfs); cmp = 'exact dfs=%d dp=%s' % (dfs, exact)
    ok = ok and (exact == ex2)
    stats[kind] += 1
    ones = sum(l.count('1') for l in g); zeros = sum(l.count('0') for l in g)
    bucket = 'cnt=%s' % (exact if exact is not None and exact <= 6 else '>6')
    stats[bucket] += 1
    if not ok:
        mism += 1
        print('MISMATCH', it, tag, cmp, 'rot90dp=%s' % ex2, g, flush=True)
    rows.append((it, tag, kind, cmp, 'zeros=%d ones=%d' % (zeros, ones), 'dp %.2fs' % tdp))
    if it % 25 == 0: print('progress', it, dict(stats), 'mism', mism, '%.0fs' % (time.time() - t0), flush=True)
print('12x12 duipai: %d grids, %d mismatches, %.0fs' % (n, mism, time.time() - t0))
print('stats:', dict(stats))
for r in rows: print(*r)

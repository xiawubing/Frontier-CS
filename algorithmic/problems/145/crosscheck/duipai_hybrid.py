import random, sys, re, subprocess
from p145lib import *
kind, n, seed = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
rng = random.Random(seed); mism = 0; maxcpu = 0.0; fallback = 0
for it in range(n):
    g, tag = (random_template_grid(rng) if kind == 'template' else random_loose_grid(rng))
    path = 'rh_%s_%d.txt' % (kind, seed); write_grid(path, g)
    out = subprocess.run(['./hybridcount', path, '3000000'], capture_output=True, text=True).stdout
    ans = int(re.search(r'answer=(\d+)', out).group(1)); cpu = float(re.search(r'total cpu=([\d.]+)s', out).group(1)); maxcpu = max(maxcpu, cpu)
    if 'aborted=1' in out: fallback += 1
    exact, c6, _ = run_dp(path)
    if ans != c6: mism += 1; print('MISMATCH', it, tag, 'hybrid=%d dp=%d' % (ans, c6), g, flush=True)
print('hybrid duipai %s seed %d: %d grids, %d mismatches, dp-fallback used %d times, max total cpu %.3fs' % (kind, seed, n, mism, fallback, maxcpu))

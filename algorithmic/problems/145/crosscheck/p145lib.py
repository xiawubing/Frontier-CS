import random, subprocess, re, os
TEMPLATE = ["?   ?   ??? ","?? ??  ?   ?","? ? ?  ?   ?","? ? ?  ???? ","? ? ?  ?    ","?   ?  ?    ",
            "            ","?  ?   ?????","? ?      ?  ","??   ? ? ?  ","? ?  ? ? ?  ","?  ? ??? ?  "]
HERE = os.path.dirname(os.path.abspath(__file__))

def read_grid(path, NR=12, NC=12):
    ls = [l.rstrip('\n') for l in open(path)][:NR]
    while len(ls) < NR: ls.append('')
    return [l.ljust(NC)[:NC] for l in ls]

def write_grid(path, g):
    open(path, 'w').write('\n'.join(g) + '\n')

# ---- dihedral transforms (count of loops is invariant under all of them)
def flipV(g): return g[::-1]
def flipH(g): return [l[::-1] for l in g]
def transp(g): return [''.join(g[r][c] for r in range(len(g))) for c in range(len(g[0]))]
def rot90(g): return flipH(transp(g))
TRANSFORMS = {'flipV': flipV, 'flipH': flipH, 'transp': transp, 'rot90': rot90,
              'rot180': lambda g: flipV(flipH(g)), 'rot270': lambda g: transp(flipH(g)),
              'antitr': lambda g: flipV(transp(flipH(g)))}

# ---- random single loop on NR x NC cells via boundary of a grown cell region; returns per-cell edge counts or None
def random_loop_counts(NR, NC, rng, tries=60):
    for _ in range(tries):
        k = rng.randint(1, max(1, NR*NC*rng.choice([1,1,2,3,4,6,8])//8))
        region = {(rng.randrange(NR), rng.randrange(NC))}
        while len(region) < k:
            r, c = rng.choice(sorted(region))
            dr, dc = rng.choice([(0,1),(1,0),(0,-1),(-1,0)])
            nr, nc = r+dr, c+dc
            if 0 <= nr < NR and 0 <= nc < NC: region.add((nr, nc))
        # boundary edges
        H = [[0]*NC for _ in range(NR+1)]; V = [[0]*(NC+1) for _ in range(NR)]
        for i in range(NR+1):
            for j in range(NC):
                a = (i-1, j) in region; b = (i, j) in region
                if a != b: H[i][j] = 1
        for i in range(NR):
            for j in range(NC+1):
                a = (i, j-1) in region; b = (i, j) in region
                if a != b: V[i][j] = 1
        deg = [[0]*(NC+1) for _ in range(NR+1)]
        adj = {}
        ne = 0
        for i in range(NR+1):
            for j in range(NC):
                if H[i][j]:
                    deg[i][j] += 1; deg[i][j+1] += 1; ne += 1
                    adj.setdefault((i,j), []).append((i,j+1)); adj.setdefault((i,j+1), []).append((i,j))
        for i in range(NR):
            for j in range(NC+1):
                if V[i][j]:
                    deg[i][j] += 1; deg[i+1][j] += 1; ne += 1
                    adj.setdefault((i,j), []).append((i+1,j)); adj.setdefault((i+1,j), []).append((i,j))
        if any(d not in (0,2) for row in deg for d in row): continue
        # connectivity
        start = next(iter(adj)); seen = {start}; st = [start]
        while st:
            u = st.pop()
            for w in adj[u]:
                if w not in seen: seen.add(w); st.append(w)
        if len(seen) != ne: continue
        cnt = [[0]*NC for _ in range(NR)]
        for i in range(NR+1):
            for j in range(NC):
                if H[i][j]:
                    if i < NR: cnt[i][j] += 1
                    if i > 0: cnt[i-1][j] += 1
        for i in range(NR):
            for j in range(NC+1):
                if V[i][j]:
                    if j < NC: cnt[i][j] += 1
                    if j > 0: cnt[i][j-1] += 1
        return cnt
    return None

def random_template_grid(rng):
    """random 12x12 grid on the official template; returns (grid, tag)"""
    mode = rng.random()
    cnt = random_loop_counts(12, 12, rng) if mode < 0.85 else None
    g = []
    for r in range(12):
        row = ''
        for c in range(12):
            if TEMPLATE[r][c] == '?':
                if cnt is not None: row += (str(cnt[r][c]) if cnt[r][c] <= 3 else rng.choice('0123'))
                else: row += rng.choice('0111222233')
            else: row += ' '
        g.append(row)
    tag = 'loop' if cnt is not None else 'rand'
    if cnt is not None and rng.random() < 0.5:
        m = rng.randint(1, 3)
        cells = [(r,c) for r in range(12) for c in range(12) if TEMPLATE[r][c] == '?']
        for (r,c) in rng.sample(cells, m):
            g[r] = g[r][:c] + rng.choice('0123') + g[r][c+1:]
        tag = 'loop+perturb%d' % m
    return g, tag

def random_small_grid(rng):
    NR, NC = rng.choice([(1,1),(1,2),(1,3),(1,4),(1,5),(1,6),(2,1),(2,2),(2,3),(2,4),(3,1),(3,2),(3,3),(4,1),(4,2),(5,1),(6,1)])
    p = rng.uniform(0.2, 1.0)
    cnt = random_loop_counts(NR, NC, rng) if rng.random() < 0.7 else None
    g = []
    for r in range(NR):
        row = ''
        for c in range(NC):
            if rng.random() < p:
                if cnt is not None and rng.random() < 0.85: row += str(cnt[r][c])
                else: row += rng.choice('0123')
            else: row += ' '
        g.append(row)
    return g, NR, NC

def run_dp(path, NR=12, NC=12):
    out = subprocess.run([os.path.join(HERE,'dpcount'), path, str(NR), str(NC)], capture_output=True, text=True).stdout
    m = re.search(r'solutions: (\S+)', out); c6 = re.search(r'capped6=(\d+)', out); t = re.search(r'time=([\d.]+)s', out)
    ex = m.group(1)
    exact = None if ex.startswith('>=') else int(ex)
    return exact, int(c6.group(1)), float(t.group(1))

def run_naive(path, NR, NC):
    return int(subprocess.run([os.path.join(HERE,'naive'), path, str(NR), str(NC)], capture_output=True, text=True).stdout.strip())

def run_dfs(path, limit, timeout):
    """count.cpp (12x12 only). returns count (int) or None on timeout"""
    try:
        out = subprocess.run([os.path.join(HERE,'count'), path, str(limit)], capture_output=True, text=True, timeout=timeout).stdout
    except subprocess.TimeoutExpired:
        return None
    m = re.search(r'solutions\(capped \d+\): (\d+)', out)
    return int(m.group(1))

def random_tree_loop_counts(NR, NC, rng, target):
    """boundary loop of a random induced-tree polyomino (no 2x2 blocks, no diagonal pinches) -> few interior zeros"""
    for _ in range(200):
        region = {(rng.randrange(NR), rng.randrange(NC))}
        stuck = 0
        while len(region) < target and stuck < 400:
            r, c = rng.choice(sorted(region))
            dr, dc = rng.choice([(0,1),(1,0),(0,-1),(-1,0)])
            nr, nc = r+dr, c+dc
            if not (0 <= nr < NR and 0 <= nc < NC) or (nr,nc) in region: stuck += 1; continue
            nb = sum(((nr+a, nc+b) in region) for a,b in [(0,1),(1,0),(0,-1),(-1,0)])
            diag_pinch = any(((nr+a, nc+b) in region) and not ((nr+a, nc) in region) and not ((nr, nc+b) in region) for a in (-1,1) for b in (-1,1))
            if nb != 1 or diag_pinch: stuck += 1; continue
            region.add((nr,nc)); stuck = 0
        cnt = [[0]*NC for _ in range(NR)]
        for (r,c) in region:
            for a,b in [(0,1),(1,0),(0,-1),(-1,0)]:
                if (r+a, c+b) not in region:
                    cnt[r][c] += 1
                    if 0 <= r+a < NR and 0 <= c+b < NC: cnt[r+a][c+b] += 1
        return cnt
    return None

def random_loose_grid(rng):
    cnt = random_tree_loop_counts(12, 12, rng, rng.randint(40, 120))
    g = []
    for r in range(12):
        row = ''
        for c in range(12):
            if TEMPLATE[r][c] == '?':
                v = cnt[r][c]
                if v == 4 or (v == 0 and rng.random() < 0.7): v = rng.choice([1,2,2,3])   # push towards the agents' zero-free style
                row += str(v)
            else: row += ' '
        g.append(row)
    tag = 'tree'
    if rng.random() < 0.4:
        m = rng.randint(1, 2)
        cells = [(r,c) for r in range(12) for c in range(12) if TEMPLATE[r][c] == '?']
        for (r,c) in rng.sample(cells, m):
            g[r] = g[r][:c] + rng.choice('123') + g[r][c+1:]
        tag = 'tree+perturb%d' % m
    return g, tag

def run_dfs_ns(path, limit, timeout):
    try:
        out = subprocess.run([os.path.join(HERE,'count_ns'), path, str(limit)], capture_output=True, text=True, timeout=timeout).stdout
    except subprocess.TimeoutExpired:
        return None
    m = re.search(r'solutions\(capped \d+\): (\d+)', out)
    return int(m.group(1))

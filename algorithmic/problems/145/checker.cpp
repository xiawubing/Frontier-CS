// checker.cpp
// C++ testlib special judge for the Slitherlink-like problem (Number Loop Construction).
// Reproduces the behaviour (parsing, scoring, verdict text) of the original checker, but counts the
// solutions with an algorithm that always finishes well inside the judge's 10 s checker CPU limit:
//   1. a brute-force DFS over the edges with a fixed node budget (every loop it finds is a genuine solution,
//      so reaching the 6-solution cap or exhausting the search is an exact answer), then, only if the
//      budget ran out,
//   2. a plug DP (broken-profile DP with bracket-encoded connectivity) that counts all single closed loops
//      consistent with the clues.
// The original enumeration (full assignment of all 312 edges + O(E) propagation scans) needed far more than
// 10 s CPU on loosely constrained outputs (no '0' clues); the judge then killed it and scored the output 0.
// Requires testlib.h in include path.

#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

const int N = 12;
const vector<string> M = {
    "?   ?   ??? ",
    "?? ??  ?   ?",
    "? ? ?  ?   ?",
    "? ? ?  ???? ",
    "? ? ?  ?    ",
    "?   ?  ?    ",
    "            ",
    "?  ?   ?????",
    "? ?      ?  ",
    "??   ? ? ?  ",
    "? ?  ? ? ?  ",
    "?  ? ??? ?  "
};

string valid_char;
vector<string> grid;

int clue_cell_count(){
    int total = 0;
    for(const string &row : M) {
        for(char c : row) {
            if(c == '?') total++;
        }
    }
    return total;
}

int count_ones(){
    int total = 0;
    for(const string &row : grid) {
        for(char c : row) {
            if(c == '1') total++;
        }
    }
    return total;
}

double score_percent(int solution_count){
    if(solution_count <= 0) return 0.0;

    const double u = (double)count_ones() / (double)clue_cell_count();
    if(solution_count >= 6) return 1.0 + 9.0 * u;
    if(solution_count == 5) return 10.0 + 10.0 * u;
    if(solution_count == 4) return 20.0 + 10.0 * u;
    if(solution_count == 3) return 30.0 + 10.0 * u;
    if(solution_count == 2) return 40.0 + 10.0 * u;
    return 50.0 + 50.0 * u;
}

// helper printing to stderr (like original)
void eprint(const string &s){ cerr << s << "\n"; }
void myAssert(bool cond, const string &msg){
    if(!cond) {
        // In SPJ we usually call quitf for WA, but here follow original: print to stderr and exit
        quitf(_pe, "Assertion failed: %s", msg.c_str());
    }
}

string clean_line(string line){
    const string allowed = " 0123";
    while(!line.empty() && allowed.find(line.front())==string::npos) line.erase(line.begin());
    while(!line.empty() && allowed.find(line.back())==string::npos) line.pop_back();
    return line;
}

vector<string> readSol(InStream &in){
    vector<string> res;
    for(int i=0;i<N;i++){
        string line = in.readLine();
        line = clean_line(line);
        myAssert((int)line.size()==12, "The size of the result should be 12 * 12");
        for(int j=0;j<12;j++){
            char c = line[j];
            myAssert(valid_char.find(c)!=string::npos, string("Invalid char ") + c);
            if(c != ' ')
                myAssert(M[i][j] == '?', "Position (" + to_string(i) + "," + to_string(j) + ") should be empty");
            else
                myAssert(M[i][j] == ' ', "Position (" + to_string(i) + "," + to_string(j) + ") should be non-empty");
        }
        res.push_back(line);
    }
    return res;
}


// ===================== solution counting: brute-force DFS with node budget =====================
// Brute-force DFS (12x12, edge order, path-endpoint tracking)
// with a deterministic node budget.  Finds genuine loops; if it reaches `limit` the answer ">= limit" is exact;
// if it finishes without abort the count is exact; if aborted, the caller must fall back to the DP.
namespace dfs12 {
const int N = 12;
static std::string grid[N];
static int H[N+1][N], V[N][N+1];
static int cellSel[N][N], cellUn[N][N];
static int degv[N+1][N+1], unv[N+1][N+1];
static int mate[(N+1)*(N+1)], plen[(N+1)*(N+1)];
static int totalSel; static bool closed; static int cyclelen;
static long long found; static int LIMIT;
static long long nodes, budget; static bool aborted;
struct E { bool h; int i, j; };
static std::vector<E> order_;
static inline int vid(int i, int j){ return i*(N+1)+j; }
static bool cellOK(int r, int c){ if(r<0||r>=N||c<0||c>=N) return true; if(grid[r][c]==' ') return true; int d=grid[r][c]-'0'; if(cellSel[r][c]>d) return false; if(cellSel[r][c]+cellUn[r][c]<d) return false; return true; }
static bool vertOK(int i, int j){ if(degv[i][j]>2) return false; if(unv[i][j]==0 && degv[i][j]!=0 && degv[i][j]!=2) return false; if(degv[i][j]==1 && unv[i][j]==0) return false; return true; }
static bool allCluesSat(){ for(int r=0;r<N;r++)for(int c=0;c<N;c++) if(grid[r][c]!=' '){ if(cellSel[r][c]!=grid[r][c]-'0') return false; } return true; }
static void dfs(int idx){
    if(aborted) return;
    if(++nodes > budget){ aborted = true; return; }
    if(found>=LIMIT) return;
    if(idx==(int)order_.size()){ if(closed && allCluesSat()){ found++; } return; }
    E e=order_[idx]; int lo=0, hi=1; if(closed) hi=0;
    for(int val=lo; val<=hi; val++){
        int sTot=totalSel; bool sClosed=closed; int sCyc=cyclelen;
        int u,v,r1,c1,r2,c2;
        if(e.h){ u=vid(e.i,e.j); v=vid(e.i,e.j+1); r1=e.i-1;c1=e.j; r2=e.i;c2=e.j; H[e.i][e.j]=val; }
        else   { u=vid(e.i,e.j); v=vid(e.i+1,e.j); r1=e.i;c1=e.j-1; r2=e.i;c2=e.j; V[e.i][e.j]=val; }
        int ui=u/(N+1), uj=u%(N+1), vi=v/(N+1), vj=v%(N+1);
        bool ok=true; int cells[2][2]={{r1,c1},{r2,c2}};
        for(int k=0;k<2;k++){int r=cells[k][0],c=cells[k][1]; if(r<0||r>=N||c<0||c>=N)continue; cellUn[r][c]--; if(val)cellSel[r][c]++;}
        unv[ui][uj]--; unv[vi][vj]--; if(val){degv[ui][uj]++;degv[vi][vj]++;}
        int savedMateA=-1,savedMateB=-1,ma=-1,mb=-1,savedPlenA=0,savedPlenB=0;
        if(val){ totalSel++;
            if(degv[ui][uj]>2||degv[vi][vj]>2) ok=false;
            else{ ma=mate[u]; mb=mate[v];
                if(ma==v){ int len=plen[u]+1; if(closed) ok=false; else { closed=true; cyclelen=len; if(cyclelen!=totalSel) ok=false; } }
                else { savedMateA=mate[ma]; savedMateB=mate[mb]; savedPlenA=plen[ma]; savedPlenB=plen[mb]; int len=plen[u]+plen[v]+1; mate[ma]=mb; mate[mb]=ma; plen[ma]=len; plen[mb]=len; } } }
        if(ok){ for(int k=0;k<2&&ok;k++){int r=cells[k][0],c=cells[k][1]; if(!cellOK(r,c))ok=false;} if(ok&&!vertOK(ui,uj))ok=false; if(ok&&!vertOK(vi,vj))ok=false; }
        if(ok) dfs(idx+1);
        if(val){ if(ma!=-1){ if(ma==v){} else { mate[ma]=savedMateA; mate[mb]=savedMateB; plen[ma]=savedPlenA; plen[mb]=savedPlenB; } } degv[ui][uj]--; degv[vi][vj]--; }
        unv[ui][uj]++; unv[vi][vj]++;
        for(int k=0;k<2;k++){int r=cells[k][0],c=cells[k][1]; if(r<0||r>=N||c<0||c>=N)continue; cellUn[r][c]++; if(val)cellSel[r][c]--;}
        totalSel=sTot; closed=sClosed; cyclelen=sCyc;
        if(e.h) H[e.i][e.j]=-1; else V[e.i][e.j]=-1;
        if(found>=LIMIT || aborted) return;
    }
}
// returns found (== limit means ">= limit"); sets aborted_ if the node budget ran out (count then unusable)
inline long long count_capped(const std::vector<std::string>& g, int limit, long long budget_, bool& aborted_, long long* nodes_used = nullptr){
    for(int i=0;i<N;i++){ std::string l = i < (int)g.size() ? g[i] : ""; while((int)l.size()<N) l.push_back(' '); grid[i]=l.substr(0,N); }
    LIMIT=limit; budget=budget_; nodes=0; aborted=false; found=0; totalSel=0; closed=false; cyclelen=0;
    memset(H,-1,sizeof(H)); memset(V,-1,sizeof(V));
    for(int r=0;r<N;r++)for(int c=0;c<N;c++){cellSel[r][c]=0;cellUn[r][c]=4;}
    for(int i=0;i<=N;i++)for(int j=0;j<=N;j++){ degv[i][j]=0; int t=0; if(i)t++; if(i<N)t++; if(j)t++; if(j<N)t++; unv[i][j]=t; mate[vid(i,j)]=vid(i,j); plen[vid(i,j)]=0; }
    order_.clear();
    for(int r=0;r<N;r++){ for(int c=0;c<N;c++){ order_.push_back({true,r,c}); order_.push_back({false,r,c}); } order_.push_back({false,r,N}); }
    for(int c=0;c<N;c++) order_.push_back({true,N,c});
    dfs(0);
    aborted_ = aborted; if(nodes_used) *nodes_used = nodes;
    return found;
}
} // namespace dfs12

// ===================== solution counting: plug DP (bracket representation) =====================
// Plug DP (bracket representation) that counts single closed loops
// on the (NR+1)x(NC+1) vertex grid whose per-cell edge counts match every clue.
// grid[r][c]: ' ' = no clue, '0'..'3' = clue.  Count saturates at CAP.

namespace slither {

typedef unsigned long long u64;
const u64 CAP = 1000000000000000000ULL; // 1e18

inline u64 satadd(u64 a, u64 b){ u64 s = a + b; return s > CAP ? CAP : s; }

struct HashTable {
    std::vector<u64> key, val; std::vector<int> nxt;
    std::vector<int> head; std::vector<int> used; int mask;
    explicit HashTable(int bits = 20): head(1 << bits, -1), mask((1 << bits) - 1) {}
    void clear(){ for(int h : used) head[h] = -1; used.clear(); key.clear(); val.clear(); nxt.clear(); }
    static inline u64 mix(u64 x){ x ^= x >> 33; x *= 0xff51afd7ed558ccdULL; x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL; x ^= x >> 33; return x; }
    void add(u64 k, u64 v){
        int h = (int)(mix(k) & mask);
        for(int e = head[h]; e != -1; e = nxt[e]) if(key[e] == k){ val[e] = satadd(val[e], v); return; }
        if(head[h] == -1) used.push_back(h);
        key.push_back(k); val.push_back(v); nxt.push_back(head[h]); head[h] = (int)key.size() - 1;
    }
    size_t size() const { return key.size(); }
};

// Frontier at vertex (i,j): positions 0..j-1 = down plugs of columns 0..j-1 (edges row i -> i+1),
// position j = horizontal plug (edge (i,j-1)->(i,j)), positions j+1..NC+1 = up plugs of columns j..NC
// (edges row i-1 -> i).  Plug value: 0 none, 1 '(', 2 ')'.
// H-bit c = H[i-1][c] (top edge of the cell above the frontier), kept only for clue cells.
inline u64 count_loops(const std::vector<std::string>& grid, int NR, int NC, size_t* maxStates = nullptr){
    const int P   = NC + 2;      // plug positions
    const int HB0 = 2 * P;       // first h-bit
    const int CL  = HB0 + NC;    // closed-flag bit
    if(CL >= 64) quitf(_fail, "checker internal error: grid too wide for 64-bit DP state");
    auto clue = [&](int r, int c)->int { if(r < 0 || r >= NR || c < 0 || c >= NC) return -1; char ch = grid[r][c]; return ch == ' ' ? -1 : ch - '0'; };
    auto getp = [&](u64 s, int p)->int { return (int)((s >> (2 * p)) & 3ULL); };
    auto setp = [&](u64 s, int p, int v)->u64 { return (s & ~(3ULL << (2 * p))) | ((u64)v << (2 * p)); };
    auto geth = [&](u64 s, int c)->int { return (int)((s >> (HB0 + c)) & 1ULL); };
    auto seth = [&](u64 s, int c, int v)->u64 { return (s & ~(1ULL << (HB0 + c))) | ((u64)v << (HB0 + c)); };
    const u64 CLOSED = 1ULL << CL;
    const u64 PLUGMASK = (1ULL << (2 * P)) - 1;

    HashTable *cur = new HashTable(20), *nxt = new HashTable(20);
    cur->add(0, 1);
    size_t mx = 0;
    for(int i = 0; i <= NR; i++){
        for(int j = 0; j <= NC; j++){
            nxt->clear();
            const bool canD = (i < NR), canR = (j < NC);
            const int clueAbove = (i >= 1 && j <= NC - 1) ? clue(i - 1, j) : -1;   // cell (i-1,j): closes now
            const int clueLeft  = (i <= NR - 1 && j >= 1) ? clue(i, j - 1) : -1;   // cell (i,j-1): partial
            const int clueHere  = (i <= NR - 1 && j <= NC - 1) ? clue(i, j) : -1;  // cell (i,j): partial
            for(size_t e = 0; e < cur->key.size(); e++){
                const u64 s = cur->key[e]; const u64 cnt = cur->val[e];
                const int L = getp(s, j), U = getp(s, j + 1);
                const bool closed = (s & CLOSED) != 0;
                const int upRight  = (j + 2 <= P - 1) ? getp(s, j + 2) : 0;  // up plug of column j+1 = V[i-1][j+1]
                const int leftDown = (j >= 1) ? getp(s, j - 1) : 0;          // down plug of column j-1 = V[i][j-1]
                const int topAbove = (j <= NC - 1) ? geth(s, j) : 0;         // H[i-1][j]
                auto emit = [&](int D, int R, u64 base, bool setClosed){
                    if(clueAbove >= 0){
                        int c = topAbove + (U ? 1 : 0) + (upRight ? 1 : 0) + (R ? 1 : 0);
                        if(c != clueAbove) return;
                    }
                    if(clueLeft >= 0){
                        int c = (L ? 1 : 0) + (leftDown ? 1 : 0) + (D ? 1 : 0);
                        if(c > clueLeft || c + 1 < clueLeft) return;
                    }
                    if(clueHere >= 0){
                        int c = (R ? 1 : 0) + (D ? 1 : 0);
                        if(c > clueHere || c + 2 < clueHere) return;
                    }
                    u64 t = setp(setp(base, j, D), j + 1, R);
                    if(j <= NC - 1) t = seth(t, j, (clueHere >= 0 && R) ? 1 : 0);
                    if(setClosed) t |= CLOSED;
                    if(j == NC){ // row shift: new p[0]=0, new p[k+1]=old p[k]; old p[NC+1] (=R=0) dropped
                        u64 plugs = ((t & PLUGMASK) << 2) & PLUGMASK;
                        t = (t & ~PLUGMASK) | plugs;
                    }
                    nxt->add(t, cnt);
                };
                if(closed){ if(L || U) continue; emit(0, 0, s, false); continue; }
                if(L == 0 && U == 0){
                    emit(0, 0, s, false);
                    if(canD && canR) emit(1, 2, s, false);          // new path: D='(' , R=')'
                } else if(L != 0 && U == 0){
                    if(canD) emit(L, 0, s, false);
                    if(canR) emit(0, L, s, false);
                } else if(L == 0 && U != 0){
                    if(canD) emit(U, 0, s, false);
                    if(canR) emit(0, U, s, false);
                } else {
                    if(L == 1 && U == 2){                             // matched pair -> loop closes
                        u64 others = (s & PLUGMASK) & ~(3ULL << (2 * j)) & ~(3ULL << (2 * (j + 1)));
                        if(others != 0) continue;                     // would leave other paths -> >1 loop
                        emit(0, 0, s, true);
                    } else if(L == 1 && U == 1){                      // relabel U's match ')' -> '('
                        int depth = 1, q = -1;
                        for(int p = j + 2; p < P; p++){ int v = getp(s, p); if(v == 1) depth++; else if(v == 2 && --depth == 0){ q = p; break; } }
                        if(q < 0) quitf(_fail, "checker internal error: bracket mismatch in plug DP");
                        emit(0, 0, setp(s, q, 1), false);
                    } else if(L == 2 && U == 2){                      // relabel L's match '(' -> ')'
                        int depth = 1, q = -1;
                        for(int p = j - 1; p >= 0; p--){ int v = getp(s, p); if(v == 2) depth++; else if(v == 1 && --depth == 0){ q = p; break; } }
                        if(q < 0) quitf(_fail, "checker internal error: bracket mismatch in plug DP");
                        emit(0, 0, setp(s, q, 2), false);
                    } else {                                          // L=')', U='(' : endpoints already right
                        emit(0, 0, s, false);
                    }
                }
            }
            std::swap(cur, nxt);
            mx = std::max(mx, cur->size());
        }
    }
    u64 total = 0;
    for(size_t e = 0; e < cur->key.size(); e++) if(cur->key[e] & CLOSED) total = satadd(total, cur->val[e]);
    if(maxStates) *maxStates = mx;
    delete cur; delete nxt;
    return total;
}

} // namespace slither

int main(int argc, char **argv){
    registerTestlibCmd(argc, argv);

    int w = inf.readInt();
    if(w == 0) valid_char = " 0123";
    else valid_char = " 123";

    grid = readSol(ouf);

    // Stage 1: brute-force DFS with a fixed node budget.  Every loop it finds is genuine, so reaching 6 is an
    // exact answer; finishing the search without abort is exact too.
    // Stage 2: plug DP (exact, saturated count) whenever the DFS ran out of budget.
    int cnt;
    const long long DFS_BUDGET = 3000000LL; // nodes, not time: the verdict does not depend on machine speed
    bool aborted = false;
    long long f = dfs12::count_capped(grid, 6, DFS_BUDGET, aborted);
    if(!aborted) cnt = (int)f;
    else {
        slither::u64 total = slither::count_loops(grid, N, N);
        cnt = (int)std::min<slither::u64>(total, 6);
    }

    if(cnt == 0){
        quitp(0.0, "There is no valid solution");
    }

    const int ones = count_ones();
    const int clues = clue_cell_count();
    const double score = score_percent(cnt);
    const string count_label = cnt >= 6 ? "six or more" : to_string(cnt);
    quitp(
        score / 100.0,
        "Ratio: %.10f RatioUnbounded: %.10f Score: %.6f, solutions: %s, ones: %d/%d",
        score / 100.0,
        score / 100.0,
        score,
        count_label.c_str(),
        ones,
        clues
    );
    return 0;
}

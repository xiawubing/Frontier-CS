// slither_dp.h -- plug DP (bracket representation) that counts single closed loops
// on the (NR+1)x(NC+1) vertex grid whose per-cell edge counts match every clue.
// grid[r][c]: ' ' = no clue, '0'..'3' = clue.  Count saturates at CAP.
#pragma once
#include <bits/stdc++.h>

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
    if(CL >= 64) throw std::runtime_error("grid too wide for 64-bit state");
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
                        if(q < 0) throw std::runtime_error("bracket mismatch");
                        emit(0, 0, setp(s, q, 1), false);
                    } else if(L == 2 && U == 2){                      // relabel L's match '(' -> ')'
                        int depth = 1, q = -1;
                        for(int p = j - 1; p >= 0; p--){ int v = getp(s, p); if(v == 2) depth++; else if(v == 1 && --depth == 0){ q = p; break; } }
                        if(q < 0) throw std::runtime_error("bracket mismatch");
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

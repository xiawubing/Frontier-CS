// slither_dfs12.h -- the brute-force DFS from count.cpp (12x12, edge-order, path-endpoint tracking),
// with a deterministic node budget.  Finds genuine loops; if it reaches `limit` the answer ">= limit" is exact;
// if it finishes without abort the count is exact; if aborted, the caller must fall back to the DP.
#pragma once
#include <bits/stdc++.h>
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

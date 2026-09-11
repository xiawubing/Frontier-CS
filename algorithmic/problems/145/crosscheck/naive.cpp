// naive: ./naive <grid-file> NR NC  -- enumerate all 2^E edge subsets, count single loops matching clues
#include <bits/stdc++.h>
using namespace std;
int NR,NC; vector<string> g;
int main(int argc,char**argv){
    NR=atoi(argv[2]); NC=atoi(argv[3]);
    ifstream fin(argv[1]);
    for(int i=0;i<NR;i++){ string l; if(!getline(fin,l)) l=""; while((int)l.size()<NC) l.push_back(' '); g.push_back(l.substr(0,NC)); }
    // edges: H[i][j] i in 0..NR, j in 0..NC-1 ; V[i][j] i in 0..NR-1, j in 0..NC
    vector<array<int,4>> E; // (u,v,cellA,cellB) cell index or -1
    auto vid=[&](int i,int j){return i*(NC+1)+j;};
    auto cid=[&](int r,int c){ if(r<0||r>=NR||c<0||c>=NC) return -1; return r*NC+c; };
    for(int i=0;i<=NR;i++)for(int j=0;j<NC;j++) E.push_back({vid(i,j),vid(i,j+1),cid(i-1,j),cid(i,j)});
    for(int i=0;i<NR;i++)for(int j=0;j<=NC;j++) E.push_back({vid(i,j),vid(i+1,j),cid(i,j-1),cid(i,j)});
    int m=E.size(); if(m>26){fprintf(stderr,"too many edges %d\n",m); return 1;}
    int NV=(NR+1)*(NC+1), NCELL=NR*NC;
    vector<int> clue(NCELL,-1); for(int r=0;r<NR;r++)for(int c=0;c<NC;c++) if(g[r][c]!=' ') clue[r*NC+c]=g[r][c]-'0';
    long long cnt=0;
    vector<int> deg(NV), cc(NCELL); vector<vector<int>> adj(NV);
    for(long long mask=1; mask<(1LL<<m); mask++){
        fill(deg.begin(),deg.end(),0); fill(cc.begin(),cc.end(),0);
        bool ok=true;
        for(int e=0;e<m&&ok;e++) if(mask>>e&1){ deg[E[e][0]]++; deg[E[e][1]]++; if(E[e][2]>=0) cc[E[e][2]]++; if(E[e][3]>=0) cc[E[e][3]]++; }
        for(int v=0;v<NV&&ok;v++) if(deg[v]!=0&&deg[v]!=2) ok=false;
        for(int c=0;c<NCELL&&ok;c++) if(clue[c]>=0&&cc[c]!=clue[c]) ok=false;
        if(!ok) continue;
        // connectivity: single cycle
        for(auto&a:adj)a.clear();
        int start=-1, ne=0;
        for(int e=0;e<m;e++) if(mask>>e&1){ adj[E[e][0]].push_back(E[e][1]); adj[E[e][1]].push_back(E[e][0]); start=E[e][0]; ne++; }
        vector<char> vis(NV,0); vector<int> st{start}; vis[start]=1; int nv=0;
        while(!st.empty()){int u=st.back();st.pop_back();nv++; for(int w:adj[u]) if(!vis[w]){vis[w]=1;st.push_back(w);} }
        if(nv==ne) cnt++; // a 2-regular connected graph: #vertices == #edges
    }
    printf("%lld\n",cnt);
}

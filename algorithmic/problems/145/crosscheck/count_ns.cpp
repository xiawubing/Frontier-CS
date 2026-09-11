// Independent Slitherlink single-loop counter (cap at LIMIT solutions)
#include <bits/stdc++.h>
using namespace std;
const int N=12;
string grid[N];
int H[N+1][N], V[N][N+1];           // -1 unassigned, 0/1
int cellSel[N][N], cellUn[N][N];
int degv[N+1][N+1], unv[N+1][N+1];
int mate[(N+1)*(N+1)], plen[(N+1)*(N+1)];
int totalSel=0; bool closed=false; int cyclelen=0;
long long found=0; int LIMIT=6;
vector<string> sols;
struct E{bool h;int i,j;};
vector<E> order_;

inline int vid(int i,int j){return i*(N+1)+j;}

bool cellOK(int r,int c){
    if(r<0||r>=N||c<0||c>=N) return true;
    if(grid[r][c]==' ') return true;
    int d=grid[r][c]-'0';
    if(cellSel[r][c]>d) return false;
    if(cellSel[r][c]+cellUn[r][c]<d) return false;
    return true;
}
bool vertOK(int i,int j){
    if(degv[i][j]>2) return false;
    if(unv[i][j]==0 && degv[i][j]!=0 && degv[i][j]!=2) return false;
    if(degv[i][j]==1 && unv[i][j]==0) return false;
    return true;
}

struct Undo{int type;int a,b,c;};
// we use explicit save/restore of small state via recursion params instead

bool assign(bool h,int i,int j,int val, /*undo info*/ vector<int>&log);

string serialize(){
    string s; s.reserve(400);
    for(int i=0;i<=N;i++) for(int j=0;j<N;j++) s.push_back('0'+H[i][j]);
    s.push_back('#');
    for(int i=0;i<N;i++) for(int j=0;j<=N;j++) s.push_back('0'+V[i][j]);
    return s;
}
bool allCluesSat(){
    for(int r=0;r<N;r++)for(int c=0;c<N;c++) if(grid[r][c]!=' '){
        if(cellSel[r][c]!=grid[r][c]-'0') return false;
    }
    return true;
}

void dfs(int idx);

int main(int argc,char**argv){
    // read grid from file argv[1]
    ifstream fin(argv[1]);
    for(int i=0;i<N;i++){ string l; getline(fin,l); while((int)l.size()<N) l.push_back(' '); grid[i]=l.substr(0,N); }
    if(argc>2) LIMIT=atoi(argv[2]);
    memset(H,-1,sizeof(H)); memset(V,-1,sizeof(V));
    for(int r=0;r<N;r++)for(int c=0;c<N;c++){cellSel[r][c]=0;cellUn[r][c]=4;}
    for(int i=0;i<=N;i++)for(int j=0;j<=N;j++){
        degv[i][j]=0; int t=0; if(i)t++; if(i<N)t++; if(j)t++; if(j<N)t++; unv[i][j]=t;
        mate[vid(i,j)]=vid(i,j); plen[vid(i,j)]=0;
    }
    // broken-profile order
    for(int r=0;r<N;r++){
        for(int c=0;c<N;c++){ order_.push_back({true,r,c}); order_.push_back({false,r,c}); }
        order_.push_back({false,r,N});
    }
    for(int c=0;c<N;c++) order_.push_back({true,N,c});
    dfs(0);
    printf("solutions(capped %d): %lld\n", LIMIT, found);
    for(size_t k=0;k<sols.size()&&k<3;k++) printf("sol %zu key head %s\n",k,sols[k].substr(0,40).c_str());
    return 0;
}

void dfs(int idx){
    if(found>=LIMIT) return;
    if(idx==(int)order_.size()){
        if(closed && allCluesSat()){ found++; }
        return;
    }
    E e=order_[idx];
    int lo=0, hi=1;
    if(closed) hi=0;   // after the loop closed, everything else must be 0
    for(int val=lo; val<=hi; val++){
        // save state
        int sTot=totalSel; bool sClosed=closed; int sCyc=cyclelen;
        int u,v,r1,c1,r2,c2;
        if(e.h){ u=vid(e.i,e.j); v=vid(e.i,e.j+1); r1=e.i-1;c1=e.j; r2=e.i;c2=e.j; H[e.i][e.j]=val; }
        else   { u=vid(e.i,e.j); v=vid(e.i+1,e.j); r1=e.i;c1=e.j-1; r2=e.i;c2=e.j; V[e.i][e.j]=val; }
        int ui=u/(N+1), uj=u%(N+1), vi=v/(N+1), vj=v%(N+1);
        // update cells
        bool ok=true;
        int cells[2][2]={{r1,c1},{r2,c2}};
        for(int k=0;k<2;k++){int r=cells[k][0],c=cells[k][1]; if(r<0||r>=N||c<0||c>=N)continue; cellUn[r][c]--; if(val)cellSel[r][c]++;}
        unv[ui][uj]--; unv[vi][vj]--; if(val){degv[ui][uj]++;degv[vi][vj]++;}
        int savedMateA=-1,savedMateB=-1,ma=-1,mb=-1,savedPlenA=0,savedPlenB=0;
        if(val){
            totalSel++;
            if(degv[ui][uj]>2||degv[vi][vj]>2) ok=false;
            else{
                ma=mate[u]; mb=mate[v];
                if(ma==v){ // closes a cycle
                    int len=plen[u]+1;
                    if(closed) ok=false;
                    else { closed=true; cyclelen=len; if(cyclelen!=totalSel) ok=false; }
                } else {
                    savedMateA=mate[ma]; savedMateB=mate[mb]; savedPlenA=plen[ma]; savedPlenB=plen[mb];
                    int len=plen[u]+plen[v]+1;
                    mate[ma]=mb; mate[mb]=ma; plen[ma]=len; plen[mb]=len;
                }
            }
        }
        if(ok){
            for(int k=0;k<2&&ok;k++){int r=cells[k][0],c=cells[k][1]; if(!cellOK(r,c))ok=false;}
            if(ok&&!vertOK(ui,uj))ok=false;
            if(ok&&!vertOK(vi,vj))ok=false;
        }
        if(ok) dfs(idx+1);
        // rollback
        if(val){
            if(ma!=-1){
                if(ma==v){ /* cycle case: nothing to restore in mate */ }
                else { mate[ma]=savedMateA; mate[mb]=savedMateB; plen[ma]=savedPlenA; plen[mb]=savedPlenB; }
            }
            degv[ui][uj]--; degv[vi][vj]--;
        }
        unv[ui][uj]++; unv[vi][vj]++;
        for(int k=0;k<2;k++){int r=cells[k][0],c=cells[k][1]; if(r<0||r>=N||c<0||c>=N)continue; cellUn[r][c]++; if(val)cellSel[r][c]--;}
        totalSel=sTot; closed=sClosed; cyclelen=sCyc;
        if(e.h) H[e.i][e.j]=-1; else V[e.i][e.j]=-1;
        if(found>=LIMIT) return;
    }
}

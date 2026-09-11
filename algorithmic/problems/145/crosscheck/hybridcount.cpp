#include "slither_dfs12.h"
#include "slither_dp.h"
#include <sys/resource.h>
using namespace std;
static double cpu(){ rusage r; getrusage(RUSAGE_SELF,&r); return r.ru_utime.tv_sec + r.ru_utime.tv_usec/1e6 + r.ru_stime.tv_sec + r.ru_stime.tv_usec/1e6; }
int main(int argc,char**argv){
    long long budget = argc>2 ? atoll(argv[2]) : 3000000LL;
    ifstream fin(argv[1]); vector<string> g; for(int i=0;i<12;i++){ string l; if(!getline(fin,l)) l=""; while((int)l.size()<12) l.push_back(' '); g.push_back(l.substr(0,12)); }
    double t0=cpu(); bool ab; long long nodes; long long f = dfs12::count_capped(g, 6, budget, ab, &nodes); double t1=cpu();
    printf("dfs: found=%lld aborted=%d nodes=%lld cpu=%.3fs", f, (int)ab, nodes, t1-t0);
    if(ab){ size_t mx; slither::u64 r = slither::count_loops(g,12,12,&mx); double t2=cpu(); printf(" | dp: exact=%llu capped6=%llu maxStates=%zu cpu=%.3fs | total cpu=%.3fs answer=%llu\n", r, (unsigned long long)min<slither::u64>(r,6), mx, t2-t1, t2-t0, (unsigned long long)min<slither::u64>(r,6)); }
    else printf(" | dfs exact, no dp | total cpu=%.3fs answer=%lld\n", t1-t0, f);
}

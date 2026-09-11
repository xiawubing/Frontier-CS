// dpcount: ./dpcount <grid-file> [NR NC]   (default 12 12; lines padded with spaces)
#include "slither_dp.h"
using namespace std;
int main(int argc, char** argv){
    int NR = 12, NC = 12;
    if(argc > 3){ NR = atoi(argv[2]); NC = atoi(argv[3]); }
    ifstream fin(argv[1]);
    vector<string> g;
    for(int i = 0; i < NR; i++){ string l; if(!getline(fin, l)) l = ""; while((int)l.size() < NC) l.push_back(' '); g.push_back(l.substr(0, NC)); }
    for(auto& l : g) for(char& c : l) if(c != ' ' && (c < '0' || c > '4')) { fprintf(stderr, "bad char\n"); return 1; }
    size_t mx = 0;
    auto t0 = chrono::steady_clock::now();
    slither::u64 r = slither::count_loops(g, NR, NC, &mx);
    double sec = chrono::duration<double>(chrono::steady_clock::now() - t0).count();
    if(r >= slither::CAP) printf("solutions: >=1e18 (saturated)");
    else printf("solutions: %llu", r);
    printf("  capped6=%llu  maxStates=%zu  time=%.3fs\n", (unsigned long long)min<slither::u64>(r, 6), mx, sec);
    return 0;
}

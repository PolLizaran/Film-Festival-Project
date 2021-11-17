#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <assert.h>

using namespace std;

struct Data {
  int d; // day
  int s; // cinema

  Data(int d = -1, int s = -1) : d(d), s(s) { }
};

vector<string>   I2P, I2S;
map<string, int> P2I, S2I;

vector<vector<int>> g;
vector<Data> sol;
int          obj;


void read_instance(const char* file) {

  ifstream in(file);

  int P;
  in >> P;
  I2P.resize(P);
  for (int k = 0; k < P; ++k) {
    in >> I2P[k];
    P2I[ I2P[k] ] = k;
  }

  g.resize(P);
  int L;
  in >> L;
  for (int k = 0; k < L; ++k) {
    string sp, sq;
    in >> sp >> sq;
    assert(P2I.count(sp));
    assert(P2I.count(sq));
    int p = P2I[sp];
    int q = P2I[sq];
    g[p].push_back(q);
    g[q].push_back(p);
  }

  int S;
  in >> S;
  I2S.resize(S);
  for (int k = 0; k < S; ++k) {
    in >> I2S[k];
    S2I[ I2S[k] ] = k;
  }
}


bool read_solution(const char* file) {
  ifstream in(file);
  double time;
  in >> time >> obj;

  string sp, ss;
  int d;
  int cnt = 0;
  while (in >> sp >> d >> ss) {
    auto jp = P2I.find(sp);
    if (jp == P2I.end()) {
      cout << "Error: didn't recognize " << sp << endl;
      return false;
    }
    int p = jp->second;

    auto js = S2I.find(ss);
    if (js == S2I.end()) {
      cout << "Error: didn't recognize " << ss << endl;
      return false;
    }
    int s = js->second;

    if (d <= 0) {
      cout << "Error: wrong day identifier " << d << endl;
      return false;
    }

    if (p >= sol.size()) sol.resize(p+1);
    if (sol[p].d != -1) {
      cout << "Error: repeated film " << sp << endl;
      return false;
    }
    sol[p] = Data(d, s);
    ++cnt;
  }

  if (cnt < I2P.size()) {
    cout << "Error: read too few films"  << endl;
    return false;
  }

  return true;
}


bool solution_ok() {

  for (int p = 0; p < sol.size(); ++p) {
    for (int q : g[p]) {
      if (p < q and sol[p].d == sol[q].d) {
          cout << "Error: films " << I2P[p] << " and " << I2P[q]
               << " are projected on the same day but should not" << endl;
          return false;
      }
    }
  }

  for (int p = 0; p < sol.size(); ++p) {
    for (int q = p + 1; q < sol.size(); ++q) {
      if (sol[p].d == sol[q].d and sol[p].s == sol[q].s) {
        cout << "Error: films " << I2P[p] << " and " << I2P[q] << " are projected on the same day in the same cinema" << endl;
        return false;
      }
    }
  }

  int max_days = 1;
  for (int p = 0; p < sol.size(); ++p)
    if (max_days < sol[p].d)
      max_days = sol[p].d;

  if (max_days != obj) {
    cout << "Error: length of festival is "
         << max_days << " but should be " << obj << endl;
    return false;
  }

  return true;
}


int main(int argc, char** argv) {

  // Write help message.
  if (argc != 3) {
    cout << "Makes a sanity check of a solution" << endl;
    cout << "Usage: " << argv[0] << " INPUT_FILE OUTPUT_FILE" << endl;
    exit(0);
  }

  read_instance(argv[1]);
  bool ok = read_solution(argv[2]);
  if (not ok) {
    cout << "FAILED" << endl;
    exit(1);
  }
  if (solution_ok()) {cout << "OK"     << endl; exit(0); }
  else               {cout << "FAILED" << endl; exit(1); }
}

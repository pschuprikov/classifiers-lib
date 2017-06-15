/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu)
 * All rights reserved.
 */
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <set>
#include <utility>

using namespace std;

// Max src_ip prefix length + dst_ip prefix length
const int MAX_D = 70;
// Max #rectangles to search
const int MAX_N = 1<<16;
// Max #hops
const int MAX_M = 30;
// Actions
const int DROP = 0;
const int FWD = 1;
// Label of pattern *
const int ROOT = 0;

const double eps = 1e-7;


int N, M, N_PATH;
int cap[MAX_M];


// data used in searching rectangles
// current rules
struct Rule {
  long x, y;
  int a, c; // a --- action; c --- count
} r[2][MAX_N], rule[MAX_N];
int now_itr, next_itr;
int len[2];
int nextIdx[MAX_D][MAX_N];

// mark searched rectangle
set<pair<long, long> >* mk = NULL;
// best rectangle(selx, sely)
long best_x[MAX_M], best_y[MAX_M];
// best #overlapping, #overlapping and utility
int best_overlap[MAX_M], best_internal[MAX_M];
double best_utility[MAX_M];
 
int success;
int countPart = 0;

inline int cmp(double x) {
  return x < -eps ? -1 : x > eps;
}

inline long get_father(long x) {
  return x % 2 ? x / 2 : x / 2 - 1;
}

inline int get_depth(long x, long y) {
  int d = 0;
  while (x) {
    x = get_father(x);
    ++d;
  }
  while (y) {
    y = get_father(y);
    ++d;
  }

  return d;
}

long maxvalue(long x, long y) {
  return x < y ? y : x;
}

//returns if x1 is a prefix of x2
inline long is_father(long x1, long x2) {
  return x1 == get_father(x2);
}

//return if x1 is the ancestor of x2
inline long is_ancestor(long x1, long x2) {
  while (x2 > x1)
    x2 = get_father(x2);
  return x1 == x2;
}

inline long get_common_ancestor(long x1, long x2) {
  while (x1 != x2) {
    if (x1 > x2)
      x1 = get_father(x1);
    else
      x2 = get_father(x2);
  }
  return x1;
}

// return if x1 overlaps with x2
bool overlap(long x1, long x2) {
  return is_ancestor(x1, x2) || is_ancestor(x2, x1);
}

// returns if (x1,y1) intersects with (x2,y2)
bool intersect(long x1, long y1, long x2, long y2) {
  return overlap(x1, x2) && overlap(y1, y2);
}

// returns if (x1,y1) is totally contained in (x2,y2)
bool inside(long x1, long y1, long x2, long y2) {
  return is_ancestor(x2, x1) && is_ancestor(y2, y1);
}

void merge(long x1, long y1, long &x2, long &y2) {
  if (x2 < 0 || y2 < 0) {
    x2 = x1;
    y2 = y1;
  }
  else if (x1 < 0 || y1 < 0) {
    return;
  }
  else {
    x2 = get_common_ancestor(x1, x2);
    y2 = get_common_ancestor(y1, y2);
  }
}

// evaluate a rectangle (x, y)
// (x_sub, y_sub) is the sub-rectangle of (x,y), which contains the same set of rules
// internal is the number of rules inside (x,y)
// overlap is number of rules overlapping (x,y)
// retu is the ratio of internal_rules / overlapped_rules
void evaluate(int d, long x, long y, long &x_sub, long &y_sub, int &internal, int &overlap, double &retu) {
  overlap = 0;
  long x_inter, y_inter;

  // count internal, and compute sub
  internal = 0;
  x_sub = y_sub = -1;
  for (int i = 0; i < len[now_itr]; i = nextIdx[d - 1][i]) {
    if (inside(r[now_itr][i].x, r[now_itr][i].y, x, y)) {
      internal += r[now_itr][i].c;
      merge(r[now_itr][i].x, r[now_itr][i].y, x_sub, y_sub);
    }
  }


  if (x_sub < 0 || y_sub < 0)
    return;

  // count overlap
  overlap = 0;
  x = x_sub;
  y = y_sub;
  int prev = -1;
  for (int i = 0; i < len[now_itr]; i = nextIdx[d - 1][i])  {
    if (intersect(r[now_itr][i].x, r[now_itr][i].y, x, y)) {
      if (prev >= 0) 
	nextIdx[d][prev] = i;
      prev = i;
     
      overlap += r[now_itr][i].c;
    }
    else if (!i)
      prev = i;
  }
  nextIdx[d][prev] = len[now_itr];
  
  retu = (internal - 1) * 1.0 / overlap;
}

//rewrite the policy: remove rules in (x,y), add (x,y) fwd, split into halvs
void update(long x, long y) {
  len[next_itr] = 1;
  r[next_itr][0].x = x;
  r[next_itr][0].y = y;
  r[next_itr][0].a = FWD;
  r[next_itr][0].c = 1;
  int l = 0;
  cerr << "update...\n";
  for (int i = 0; i < len[now_itr]; ++i) {
    cerr << "...checking x = " << r[now_itr][i].x << ", y = " << r[now_itr][i].y << "\n";
    if (inside(r[now_itr][i].x, r[now_itr][i].y, x, y)) {
      cerr << "......inside!\n";
      continue;
    }
    else if (is_father(r[now_itr][i].x, x) && is_ancestor(y, r[now_itr][i].y)) {
      r[next_itr][len[next_itr]] = r[now_itr][i];
      if (x % 2)
        r[next_itr][len[next_itr]].x = x + 1;
      else
        r[next_itr][len[next_itr]].x = x - 1;
      ++len[next_itr];
    }
    else if (is_father(r[now_itr][i].y, y) && is_ancestor(x, r[now_itr][i].x)) {
      r[next_itr][len[next_itr]] = r[now_itr][i];
      if (y % 2)
        r[next_itr][len[next_itr]].y = y + 1;
      else
        r[next_itr][len[next_itr]].y = y - 1;
      ++len[next_itr];
    }
    else {
      r[next_itr][len[next_itr]] = r[now_itr][i];
      ++len[next_itr];
    }
  }

  now_itr = next_itr;
  next_itr = 1 - next_itr;
}

void makeit() {
  success = 1;
}


// mark a new rectangle (x, y)
bool insert(long x, long y) {
  pair<long, long> p(x, y);
  set< pair<long, long> > :: iterator it;

  it = mk->find(p);
  if (it == mk->end()) {
    mk->insert(p);
    return 1;
  }
  return 0;
}

// omit searching the rectangle (x, y)
void no_expand(long x, long y) {
  insert(x, y);
}

// count the total number of rules
int count_rule_num(int now) {
  int total = 0;
  for (int i = 0; i < len[now]; ++i)
    total += r[now][i].c;
  return total;
}

// recursively search for rectangles
void search_rectangle(int d, long x_star, long y_star,int kth, int capacity) {
  if (!insert(x_star, y_star)) 
    return;

  int overlap, internal;
  double utility;
  long x = x_star, y = y_star;

  cerr << "searching in x=" << x_star << " y=" << y_star << '\n';
  if (x_star == ROOT && y_star == ROOT) {
    overlap = count_rule_num(now_itr);
    internal = overlap;
    utility = 1.0;
  }
  if (x_star == ROOT && y_star == ROOT && overlap <= capacity) {
    best_x[kth] = x;
    best_y[kth] = y;
    best_overlap[kth] = overlap;
    best_internal[kth] = internal;
    best_utility[kth] = (internal - 1) * 1.0 / overlap;
    cerr << "...root: utility = " << best_utility[kth] << "\n";

    return;
  }
  else if (x_star != ROOT || y_star != ROOT) {
    evaluate(d, x_star, y_star, x, y, internal, overlap, utility);
    cerr << "...not root: utility = " << utility 
         << "; overlap = " << overlap << "\n";
    if (overlap > 0 && overlap <= capacity) {
      no_expand(x_star * 2 + 1, y_star);
      no_expand(x_star * 2 + 2, y_star);
      no_expand(x_star, y_star * 2 + 1);
      no_expand(x_star, y_star * 2 + 2);
      
      if (utility > best_utility[kth]) {
        best_x[kth] = x;
        best_y[kth] = y;
        best_overlap[kth] = overlap;
        best_internal[kth] = internal;
        best_utility[kth] = utility;
      }
      return;
    }
    else if (internal <= 1)
      return;
  }

  if (x != x_star || y != y_star) {
    no_expand(x_star * 2 + 1, y_star);
    no_expand(x_star * 2 + 2, y_star);
    no_expand(x_star, y_star * 2 + 1);
    no_expand(x_star, y_star * 2 + 2);
  }

  search_rectangle(d + 1, x * 2 + 1, y, kth, capacity);
  search_rectangle(d + 1, x * 2 + 2, y, kth, capacity);
  search_rectangle(d + 1, x, y * 2 + 1, kth, capacity);
  search_rectangle(d + 1, x, y * 2 + 2, kth, capacity);
}

// output the selected rectangle
void output_rectangle(long xx, long yy, ofstream &ofile) {
  long kx, ky;

  for (int i = 0; i < len[now_itr]; ++i)
    if (intersect(xx, yy, r[now_itr][i].x, r[now_itr][i].y)) {
	kx = maxvalue(xx, r[now_itr][i].x);
	ky = maxvalue(yy, r[now_itr][i].y);
	ofile << kx << " " << ky << " " << r[now_itr][i].a << endl;
    }
}

bool check_feasibility(int k, int n) {
  int l = 0;
  for (int i = 0; i <= k; ++i) 
    l += cap[i];
  return (l >= n);
}


void DFS(int kth) {
  if (kth < 0)
    return;

  long x_star, y_star, x, y;

  cerr << "Rectangles for " << kth << endl;
  while (cap[kth] > 1) {
    //Intialize the map
    if (mk)
      delete mk;
    mk = new set< pair<long, long> >();
    //initialize the linked list
    for (int i = 0; i < len[now_itr]; ++i)
      nextIdx[0][i] = i + 1;
    //start searching rectangles
    best_utility[kth] = -1;
    search_rectangle(0, ROOT, ROOT, kth, cap[kth]);
    cerr << "best_utility: " << best_utility[kth] << "\n";
    if (cmp(best_utility[kth]) <= 0)
      break;

    cout << best_overlap[kth] << "\n";
    if (best_x[kth] == ROOT && best_y[kth] == ROOT) {
      makeit();
      return;
    }

    cerr << "(" << best_x[kth] << ", " << best_y[kth] << ")" << endl;
    cap[kth] -= best_overlap[kth];
    update(best_x[kth], best_y[kth]);

    cerr << "number of rules after: " << count_rule_num(now_itr) << "\n";
    if (!check_feasibility(kth, count_rule_num(now_itr)))
      return;
  }

  DFS(kth - 1);
}

// place rules along the path 
bool place_rules(int path) {
  now_itr = 0;
  next_itr = 1;
  len[now_itr] = N;
  for (int i = 0; i < N; ++i) {
    cerr << "rule: " << rule[i].x << " " << rule[i].y << " " << rule[i].c << "\n";
    r[now_itr][i] = rule[i];
  }
  success = 0;

  if (len[now_itr] <= 1)
    return (cap[0] >= len[now_itr]);

  cerr << "===== rectangles on path " << path << " =====\n";
  DFS(M - 1);
  cerr << "=================================\n";

  return success;
}

// read input
void read_in(ifstream &f_alloc, ifstream &f_policy) {
  f_alloc>> M;


  for (int i = 0; i < M; ++i)
    f_alloc >> cap[i];
  
  char s[100];
  f_policy >> s;
  f_policy >> N;
  

  int sb, se, db, de;
  for (int i = 0; i < N; ++i) {
    f_policy >> rule[i].x >> rule[i].y 
             >> sb >> se >> db >> de >> rule[i].c >> rule[i].a;
  }

  now_itr = 0; 
  next_itr = 1;
  len[now_itr] = len[next_itr] = 0;

  len[now_itr] = N;
  for (int i = 0; i < N; ++i)
    r[now_itr][i] = rule[i];
}

int main(int argc, char* argv[]) {
  ifstream f_alloc;
  ifstream f_policy;
  int numProc;
  int kthProc;


  f_alloc.open(argv[1]);
  f_policy.open(argv[2]);

  if (!f_alloc.is_open() || !f_policy.is_open()) {
    cerr << "could not open !!!" << endl;
    return 1;
  }
  
  int npt, npp;
  f_alloc >> npt;
  f_policy >> npp;
  if (npt != npp) {
    cerr << "#PATH in topo = " << npt 
         << ", while #PATH in policy = " << npp << endl;
    return 1;
  }

  N_PATH = npt;

  read_in(f_alloc, f_policy);

  if (place_rules(0) == 1) {
    cout << "OK\n";
  } else {
    cout << "FAIL\n";
  }


  f_alloc.close();
  f_policy.close();
}

#include <bits/stdc++.h>

#include "bstar_tree.h"

using namespace std;
using ll = long long;

constexpr std::size_t SCALE{10000};

int main() {

  b_star_tree<ll, ll, 145> btree{};
  std::map<ll, ll> mp{};

  vector<ll *> ans{};

  for (std::size_t i = 0; i < SCALE; i++) {
    printf("\nINSERT TIMESTAMP %zu\n", i);
    btree.insert(123, (ll *)123);
    // search
    ans = btree.find(123);
    if (ans.size() != (i + 1) || (!ans.empty() && (ll)ans[0] != 123)) {
      fprintf(stderr, "INSERTED/FIND COMPARE FAILED.\n");
      exit(-1); // NOLINT
    }
  }

  for (std::size_t i = 0; i < SCALE; i++) {
    printf("\nERASE TIMESTAMP %zu\n", i);
    btree.erase(123);
    ans = btree.find(123);
    if (ans.size() != (SCALE - i - 1) || (!ans.empty() && (ll)ans[0] != 123)) {
      fprintf(stderr, "REMAINED/FIND COMPARE FAILED.\n");
      exit(-1); // NOLINT
    }
  }

  puts("FINISHED.\n");

  return 0;
}

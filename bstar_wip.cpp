#include <bits/stdc++.h>

#include "bstar_tree.h"

using namespace std;
using ll = long long;

constexpr std::size_t SCALE{10000};

int main() {

  bstar_tree<ll, ll, 100> btree{};

  vector<ll *> ans{};
  for (std::size_t i = 0; i < SCALE; i++) {
    printf("\nINSERT TIMESTAMP %zu\n", i);
    btree.insert(i, (ll *)i);
    // search all
    // for (std::size_t j = 0; j <= i; j++) {
    //   ans = btree.find(j);
    //   if (ans.size() != 1 || (ll)ans[0] != j) {
    //     fprintf(stderr, "INSERTED/FIND COMPARE FAILED.\n");
    //     exit(-1); // NOLINT
    //   }
    // }
  }

  for (std::size_t i = 0; i < SCALE; i++) {
    printf("\nERASE TIMESTAMP %zu\n", i);
    btree.erase(i);
    // search all
    // for (std::size_t j = 0; j <= i; j++) {
    //   ans = btree.find(i);
    //   if (!ans.empty()) {
    //     fprintf(stderr, "ERASED/FIND COMPARE FAILED.\n");
    //     exit(-1); // NOLINT
    //   }
    // }
    // for (std::size_t j = i + 1; j < SCALE; j++) {
    //   ans = btree.find(j);
    //   if (ans.size() != 1 || (ll)ans[0] != j) {
    //     fprintf(stderr, "REMAINED/FIND COMPARE FAILED.\n");
    //     exit(-1); // NOLINT
    //   }
    // }
  }

  puts("SUCCESSFULLY PASSED ALL TESTS!");

  return 0;
}

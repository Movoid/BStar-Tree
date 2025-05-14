#include <bits/stdc++.h>

// #include "bstar_tree.h"

// #include "b_star_tree_.h"
#include "b_star_tree_refactored.h"

using namespace std;
using ll = long long;

constexpr std::size_t SCALE{10000};
constexpr std::size_t FLOOR{7};

using b_star = b_star_tree<ll, ll, FLOOR>;

double time_diff(const timespec &beg, const timespec &end) {
  return static_cast<double>(end.tv_sec - beg.tv_sec) +
         static_cast<double>(end.tv_nsec - beg.tv_nsec) / 1'000'000'000.0;
}

void check_bstar(ll *keys, const b_star &tree, std::size_t beg, std::size_t end) {
  std::vector<ll *> ans{};
  for (std::size_t i = beg; i < end; i++) {
    ans = tree.find(keys[i]);
    if (ans.empty()) {
      fprintf(stderr, "CHECK FAILED IN [%zu, %zu) !\n", beg, end);
      _exit(-1);
    }
  }
}

void random_test() {

  puts("\n[RANDOM_TEST]");

  b_star tree{};

  std::random_device rd{};
  std::mt19937 gen{2};
  std::uniform_int_distribution<> dis{1, SCALE};

  ll *keys{new ll[SCALE]{}};
  for (std::size_t i = 0; i < SCALE; i++) {
    keys[i] = dis(gen);
  }

  puts("INSERT TEST");
  for (std::size_t i = 0; i < SCALE; i++) {
    tree.insert(keys[i], (ll *)i);
    // check_bstar(keys, tree, 0, i + 1);
  }

  puts("ERASE TEST");
  for (std::size_t i = 0; i < SCALE; i++) {
    tree.erase(keys[i]);
    if (i == 1960) check_bstar(keys, tree, i + 1, SCALE);
  }

  delete[] keys;

  puts("[RANDOM_TEST] PASSED !");
}

int main() {

  random_test();
}
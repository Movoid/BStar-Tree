#include <bits/stdc++.h>

// #include "bstar_tree.h"

// #include "bepsilon_tree_.h"
#include "bepsilon_tree_refactored.h"

using namespace std;
using ll = long long;

constexpr std::size_t SCALE{100000};
constexpr std::size_t FLOOR{135};

void dup_test() {

  b_star_tree<ll, ll, FLOOR> btree{}, tmp{};

  vector<ll *> ans{};

  for (std::size_t i = 0; i < SCALE; i++) {
    printf("\nINSERT TIMESTAMP %zu\n", i);
    tmp.insert(123, (ll *)123);
    // search
    ans = tmp.find(123);
    if (ans.size() != (i + 1) || (!ans.empty() && (ll)ans[0] != 123)) {
      fprintf(stderr, "INSERTED/FIND COMPARE FAILED.\n");
      exit(-1); // NOLINT
    }
  }

  btree = std::move(tmp);

  for (std::size_t i = 0; i < SCALE; i++) {
    printf("\nERASE TIMESTAMP %zu\n", i);
    btree.erase(123);
    ans = btree.find(123);
    if (ans.size() != (SCALE - i - 1) || (!ans.empty() && (ll)ans[0] != 123)) {
      fprintf(stderr, "REMAINED/FIND COMPARE FAILED.\n");
      exit(-1); // NOLINT
    }
  }

  puts("DUP_TEST FINISHED.\n");
}

void nodup_test() {

  b_star_tree<ll, ll, FLOOR> btree{};

  vector<ll *> ans{};
  for (std::size_t i = 0; i < SCALE; i++) {
    printf("\nINSERT TIMESTAMP %zu\n", i);
    btree.insert(i, (ll *)i);
    // search all
    for (std::size_t j = 0; j <= i; j++) {
      ans = btree.find(j);
      if (ans.size() != 1 || (ll)ans[0] != j) {
        fprintf(stderr, "INSERTED/FIND COMPARE FAILED.\n");
        exit(-1); // NOLINT
      }
    }
  }

  for (std::size_t i = 0; i < SCALE; i++) {
    printf("\nERASE TIMESTAMP %zu\n", i);
    btree.erase(i);
    // search all
    for (std::size_t j = 0; j <= i; j++) {
      ans = btree.find(i);
      if (!ans.empty()) {
        fprintf(stderr, "ERASED/FIND COMPARE FAILED.\n");
        exit(-1); // NOLINT
      }
    }
    for (std::size_t j = i + 1; j < SCALE; j++) {
      ans = btree.find(j);
      if (ans.size() != 1 || (ll)ans[0] != j) {
        fprintf(stderr, "REMAINED/FIND COMPARE FAILED.\n");
        exit(-1); // NOLINT
      }
    }
  }

  puts("NODUP_TEST FINISHED.\n");
}

void fast_test() {

  b_star_tree<ll, ll, FLOOR> btree{};

  vector<ll *> ans{};
  printf("\nINSERT FAST TEST\n");
  sleep(1);
  for (std::size_t i = 0; i < SCALE; i++) {
    btree.insert(i, (ll *)i);
  }
  // search all
  for (std::size_t j = 0; j < SCALE; j++) {
    ans = btree.find(j);
    if (ans.size() != 1 || (ll)ans[0] != j) {
      fprintf(stderr, "INSERTED/FIND COMPARE FAILED.\n");
      exit(-1); // NOLINT
    }
  }

  printf("\nERASE FAST TEST\n");
  sleep(1);
  for (std::size_t i = 0; i < SCALE; i++) {
    btree.erase(i);
  }

  // search all
  for (std::size_t j = 0; j < SCALE; j++) {
    ans = btree.find(j);
    if (!ans.empty()) {
      fprintf(stderr, "ERASED/FIND COMPARE FAILED.\n");
      exit(-1); // NOLINT
    }
  }

  puts("FAST_TEST FINISHED.\n");
}

void bstar_benchmark() {

  b_star_tree<ll, ll, FLOOR> t{};
  timespec beg1{}, end1{}, beg2{}, end2{}, beg3{}, end3{};

  std::cout << "B-star insert" << std::endl;
  clock_gettime(CLOCK_MONOTONIC, &beg1);
  for (std::size_t i = 0; i < SCALE; i++) {
    t.insert(i, (ll *)i);
  }
  clock_gettime(CLOCK_MONOTONIC, &end1);

  std::cout << "B-star erase" << std::endl;
  clock_gettime(CLOCK_MONOTONIC, &beg2);
  for (std::size_t i = 0; i < SCALE; i++) {
    t.erase(i);
  }
  clock_gettime(CLOCK_MONOTONIC, &end2);

  std::cout << "B-star find" << std::endl;
  std::vector<ll *> ans{};
  clock_gettime(CLOCK_MONOTONIC, &beg3);
  for (std::size_t i = 0; i < SCALE; i++) {
    ans = t.find(i);
  }
  clock_gettime(CLOCK_MONOTONIC, &end3);

  auto timespec_diff_sec = [](const timespec &start,
                              const timespec &end) -> double {
    return static_cast<double>(end.tv_sec - start.tv_sec) +
           static_cast<double>(end.tv_nsec - start.tv_nsec) / 1'000'000'000.0;
  };

  double insert_time = timespec_diff_sec(beg1, end1);
  double erase_time = timespec_diff_sec(beg2, end2);
  double find_time = timespec_diff_sec(beg3, end3);

  std::cout << "Insert time: " << insert_time << " s\n";
  std::cout << "Erase time:  " << erase_time << " s\n";
  std::cout << "Find time:  " << find_time << " s\n";
}

void stdmap_benchmark() {

  std::map<ll, ll *> t{};
  timespec beg1{}, end1{}, beg2{}, end2{}, beg3{}, end3{};

  std::cout << "std::map insert" << std::endl;
  clock_gettime(CLOCK_MONOTONIC, &beg1);
  for (std::size_t i = 0; i < SCALE; i++) {
    t.emplace(i, (ll *)i);
  }
  clock_gettime(CLOCK_MONOTONIC, &end1);

  std::cout << "std::map erase" << std::endl;
  clock_gettime(CLOCK_MONOTONIC, &beg2);
  for (std::size_t i = 0; i < SCALE; i++) {
    t.erase(i);
  }
  clock_gettime(CLOCK_MONOTONIC, &end2);

  std::cout << "std::map find" << std::endl;
  volatile ll sum{};
  clock_gettime(CLOCK_MONOTONIC, &beg3);
  for (std::size_t i = 0; i < SCALE; i++) {
    auto ans = t.find(i);
    if (ans != t.end()) sum += (ll)ans->second;
  }
  clock_gettime(CLOCK_MONOTONIC, &end3);

  auto timespec_diff_sec = [](const timespec &start,
                              const timespec &end) -> double {
    return static_cast<double>(end.tv_sec - start.tv_sec) +
           static_cast<double>(end.tv_nsec - start.tv_nsec) / 1'000'000'000.0;
  };

  double insert_time = timespec_diff_sec(beg1, end1);
  double erase_time = timespec_diff_sec(beg2, end2);
  double find_time = timespec_diff_sec(beg3, end3);

  std::cout << "Insert time: " << insert_time << " s\n";
  std::cout << "Erase time:  " << erase_time << " s\n";
  std::cout << "Find time:  " << find_time << " s\n";
}

void nodup_rangequery_test() {

  b_star_tree<ll, ll, FLOOR> btree{};

  vector<ll *> ans{};

  for (int i = 0; i < SCALE; i++) {
    printf("\nINSERT TIMESTAMP %zu\n", i);
    btree.insert(i, (ll *)123);
    // search
    ans = btree.find_range(std::max(0, i - 100), i + 1);
    if (ans.size() != (i + 1 - std::max(0, i - 100))) {
      fprintf(stderr, "INSERTED/FIND COMPARE FAILED.\n");
      exit(-1); // NOLINT
    }
    if (!ans.empty()) {
      for (std::size_t i = 0; i < ans.size(); i++) {
        if ((ll)ans[i] != 123) {
          fprintf(stderr, "INSERTED/FIND COMPARE FAILED.\n");
          exit(-1); // NOLINT
        }
      }
    }
  }

  for (int i = 0; i < SCALE; i++) {
    printf("\nERASE TIMESTAMP %zu\n", i);
    btree.erase(i);
    ans = btree.find_range(i + 1, SCALE);
    if (ans.size() != (SCALE - i - 1)) {
      fprintf(stderr, "INSERTED/FIND COMPARE FAILED.\n");
      exit(-1); // NOLINT
    }
    if (!ans.empty()) {
      for (std::size_t i = 0; i < ans.size(); i++) {
        if ((ll)ans[i] != 123) {
          fprintf(stderr, "INSERTED/FIND COMPARE FAILED.\n");
          exit(-1); // NOLINT
        }
      }
    }
  }

  puts("NODUP_RANGEQUERY_TEST FINISHED.\n");
}

int main() {

  fast_test();

  // bstar_benchmark();
  // stdmap_benchmark();

  nodup_test();
  dup_test();
  nodup_rangequery_test();
  return 0;
}

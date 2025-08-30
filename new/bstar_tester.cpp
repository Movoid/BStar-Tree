#include <bits/stdc++.h>

#include "b_star_tree_refactored.h"

using namespace std;
using ll = long long;

constexpr std::size_t SCALE{10000000};
constexpr std::size_t FLOOR{145};

using b_star = b_star_tree<ll, ll, FLOOR>;

double time_diff(const timespec &beg, const timespec &end) {
  return static_cast<double>(end.tv_sec - beg.tv_sec) +
         static_cast<double>(end.tv_nsec - beg.tv_nsec) / 1'000'000'000.0;
}

ll *gen_data() {
  // <- gen
  std::mt19937 gen{1337};
  std::vector<ll> key_vec(SCALE, 0);
  std::iota(key_vec.begin(), key_vec.end(), 1);
  std::shuffle(key_vec.begin(), key_vec.end(), gen);
  ll *keys = new ll[SCALE]{};
  for (std::size_t i = 0; i < SCALE; ++i) {
    keys[i] = key_vec[i];
  }
  // gen ->
  return keys;
}

void random_test() {

  puts("\n[RANDOM_TEST]");

  ll *keys{gen_data()};

  b_star tree{};

  puts("INSERT TEST");
  for (std::size_t i = 0; i < SCALE; i++) {
    tree.insert(keys[i], (ll *)i);
  }

  puts("ERASE TEST");
  for (std::size_t i = 0; i < SCALE; i++) {
    tree.erase(keys[i]);
  }

  delete[] keys;

  puts("[RANDOM_TEST] PASSED !");
}

void bstar_benchmark() {

  ll *keys{gen_data()};

  b_star_tree<ll, ll, FLOOR> t{};
  timespec beg1{}, end1{}, beg2{}, end2{}, beg3{}, end3{};

  std::cout << "B-star insert" << std::endl;
  clock_gettime(CLOCK_MONOTONIC, &beg1);
  for (std::size_t i = 0; i < SCALE; i++) {
    t.insert(keys[i], (ll *)i);
  }
  clock_gettime(CLOCK_MONOTONIC, &end1);

  std::cout << "B-star find" << std::endl;
  volatile ll sum{};
  ll *tmp{};
  clock_gettime(CLOCK_MONOTONIC, &beg3);
  for (std::size_t i = 0; i < SCALE; i++) {
    tmp = t.find_single(keys[i]);
    // // debug no shuffle
    // if ((ll)tmp != keys[i] - 1) {
    //   fprintf(stderr, "fail\n");
    //   _exit(-1);
    // }
    sum += (ll)tmp;
  }
  clock_gettime(CLOCK_MONOTONIC, &end3);
  printf("test output %lld\n", sum);

  std::cout << "B-star erase" << std::endl;
  clock_gettime(CLOCK_MONOTONIC, &beg2);
  for (std::size_t i = 0; i < SCALE; i++) {
    t.erase(keys[i]);
  }
  clock_gettime(CLOCK_MONOTONIC, &end2);

  auto timespec_diff_sec = [](const timespec &start, const timespec &end) -> double {
    return static_cast<double>(end.tv_sec - start.tv_sec) +
           static_cast<double>(end.tv_nsec - start.tv_nsec) / 1'000'000'000.0;
  };

  double insert_time = timespec_diff_sec(beg1, end1);
  double erase_time = timespec_diff_sec(beg2, end2);
  double find_time = timespec_diff_sec(beg3, end3);

  std::cout << "Insert time: " << insert_time << " s\n";
  std::cout << "Erase time:  " << erase_time << " s\n";
  std::cout << "Find time:  " << find_time << " s\n";

  delete[] keys;
}

void stdmap_benchmark() {

  ll *keys{gen_data()};

  std::map<ll, ll *> t{};
  timespec beg1{}, end1{}, beg2{}, end2{}, beg3{}, end3{};

  std::cout << "std::map insert" << std::endl;
  clock_gettime(CLOCK_MONOTONIC, &beg1);
  for (std::size_t i = 0; i < SCALE; i++) {
    t.emplace(keys[i], (ll *)i);
  }
  clock_gettime(CLOCK_MONOTONIC, &end1);

  std::cout << "std::map find" << std::endl;
  volatile ll sum{};
  clock_gettime(CLOCK_MONOTONIC, &beg3);
  for (std::size_t i = 0; i < SCALE; i++) {
    auto ans = t.find(keys[i]);
    if (ans != t.end()) sum += (ll)ans->second;
  }
  clock_gettime(CLOCK_MONOTONIC, &end3);
  printf("test output %lld\n", sum);

  std::cout << "std::map erase" << std::endl;
  clock_gettime(CLOCK_MONOTONIC, &beg2);
  for (std::size_t i = 0; i < SCALE; i++) {
    t.erase(keys[i]);
  }
  clock_gettime(CLOCK_MONOTONIC, &end2);

  auto timespec_diff_sec = [](const timespec &start, const timespec &end) -> double {
    return static_cast<double>(end.tv_sec - start.tv_sec) +
           static_cast<double>(end.tv_nsec - start.tv_nsec) / 1'000'000'000.0;
  };

  double insert_time = timespec_diff_sec(beg1, end1);
  double erase_time = timespec_diff_sec(beg2, end2);
  double find_time = timespec_diff_sec(beg3, end3);

  std::cout << "Insert time: " << insert_time << " s\n";
  std::cout << "Erase time:  " << erase_time << " s\n";
  std::cout << "Find time:  " << find_time << " s\n";

  delete[] keys;
}

int main() {

  random_test();

  // stdmap_benchmark();
  bstar_benchmark();
}
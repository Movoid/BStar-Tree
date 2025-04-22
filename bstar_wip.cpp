#include <bits/stdc++.h>

// each node has max of `M` branches,
// for index node, `M - 1` keys,
// for leaf node, `M` keys.

/*================================================*\

  B+* tree.
  No support for duplicate mainkey.

\*================================================*/

// `M` >= 5
template <typename KeyType, typename ValType, std::size_t M,
          typename _Requires =
              std::void_t<std::enable_if_t<M >= 5>, decltype(std::declval<KeyType>() < std::declval<KeyType>())>>
class bstar_tree {
public:
  struct bstar_node {
    KeyType key[M - 1]{};
    union {
      struct {
        ValType *data_ptr[M - 1]{};
        bstar_node *next{};
      } leaf;
      struct {
        bstar_node *key_ptr[M]{};
      } idx;
    };
    std::size_t key_cnt{};
    bool is_leaf{};

    std::size_t find_idx_ptr_index_(const KeyType &k) {
      std::size_t l{0}, r{key_cnt};
      std::size_t mid{};
      // key[-1, l) <= val, key[r, key_cnt + 1) > val.
      while (r > l) {
        mid = (r - l) / 2 + l;
        if (key[mid] < k || (!(key[mid] < k) && !(k < key[mid]))) {
          l = mid + 1;
        } else {
          r = mid;
        }
      }
      return r;
    }

    std::size_t find_data_ptr_index_(const KeyType &k) {
      std::size_t l{0}, r{key_cnt};
      std::size_t mid{};
      // key[-1, l) < val, key[r, key_cnt + 1) >= val.
      while (r > l) {
        mid = (r - l) / 2 + l;
        if (key[mid] < k) {
          l = mid + 1;
        } else {
          r = mid;
        }
      }
      return r;
    }
  };

  bstar_node *root{};
  static constexpr std::size_t MIN_KEYS = (2 * (M - 1) + 2) / 3; // ceil(2.0 / 3.0 * (M - 1))
  static constexpr std::size_t MAX_KEYS = M - 2;

private:
  // ceil((a + b) / 2)
  static constexpr std::size_t ceil_half(std::size_t a, std::size_t b) noexcept {
    return (a >> 1) + (b >> 1) + ((a & 1) + (b & 1) + 1) / 2;
  }

  static inline bool overflow_(const bstar_node *n) noexcept {
    return n->key_cnt >= MAX_KEYS;
  }
  static inline bool will_overflow_(const bstar_node *a, const bstar_node *b) noexcept {
    return ceil_half(a->key_cnt, b->key_cnt) >= MAX_KEYS;
  }

  static inline bool underflow_(const bstar_node *n) noexcept {
    return n->key_cnt < MIN_KEYS;
  }
  static inline bool will_underflow_(const bstar_node *a, const bstar_node *b) noexcept {
    return ceil_half(a->key_cnt, b->key_cnt) < MIN_KEYS;
  }

  // standard balance, parent != nullptr
  KeyType redistribute_keys_(bstar_node *node1, bstar_node *node2, std::size_t need1, std::size_t need2,
                             bstar_node *parent, std::size_t idx1) {

    std::size_t total{node1->key_cnt + node2->key_cnt};
    if (node1->key_cnt > need1) {
      if (node1->is_leaf) {
        std::size_t key_move{node1->key_cnt - need1};
        std::size_t ptr_move{key_move};
        // adjust keys
        std::memmove(node2->key + key_move, node2->key, node2->key_cnt * sizeof(KeyType));
        // move keys
        std::memcpy(node2->key, node1->key + need1, key_move * sizeof(KeyType));
        // adjust ptrs
        std::memmove(node2->leaf.data_ptr + ptr_move, node2->leaf.data_ptr, node2->key_cnt * sizeof(ValType *));
        // move ptrs
        std::memcpy(node2->leaf.data_ptr, node1->leaf.data_ptr + need1, ptr_move * sizeof(ValType *));
        node1->key_cnt = need1;
        node2->key_cnt = need2;
        return node2->key[0];
      } else { // special branch
        std::size_t key_move{node1->key_cnt - need1};
        std::size_t ptr_move{key_move};
        KeyType new_delim{node1->key[need1]};
        // adjust
        std::memmove(node2->key + key_move, node2->key, node2->key_cnt * sizeof(KeyType));
        // move
        std::memcpy(node2->key, node1->key + need1 + 1, (key_move - 1) * sizeof(KeyType));
        // if parent is nullptr? node1 is root?
        // this function will not check this. should be guaranteed valid.
        node2->key[key_move - 1] = parent->key[idx1];
        // adjust
        std::memmove(node2->idx.key_ptr + ptr_move, node2->idx.key_ptr, (node2->key_cnt + 1) * sizeof(bstar_node *));
        // move
        std::memcpy(node2->idx.key_ptr, node1->idx.key_ptr + need1 + 1, ptr_move * sizeof(bstar_node *));
        node1->key_cnt = need1;
        node2->key_cnt = need2;
        return new_delim;
      }
    } else if (node1->key_cnt < need1) {
      if (node1->is_leaf) {
        std::size_t key_move{need1 - node1->key_cnt};
        std::size_t ptr_move{key_move};
        // move
        std::memcpy(node1->key + node1->key_cnt, node2->key, (key_move) * sizeof(KeyType));
        // adjust
        std::memmove(node2->key, node2->key + key_move, (node2->key_cnt - key_move) * sizeof(KeyType));
        // move
        std::memcpy(node1->leaf.data_ptr + node1->key_cnt, node2->leaf.data_ptr, (ptr_move) * sizeof(ValType *));
        // adjust
        std::memmove(node2->leaf.data_ptr, node2->leaf.data_ptr + ptr_move,
                     (node2->key_cnt - ptr_move) * sizeof(ValType *));
        node1->key_cnt = need1;
        node2->key_cnt = need2;
        return node2->key[0];
      } else {
        std::size_t key_move{need1 - node1->key_cnt};
        std::size_t ptr_move{key_move};
        KeyType new_delim{node2->key[node2->key_cnt - need2 - 1]};
        // move
        node1->key[node1->key_cnt] = parent->key[idx1];
        std::memcpy(node1->key + node1->key_cnt + 1, node2->key, (key_move - 1) * sizeof(KeyType));
        // adjust
        std::memmove(node2->key, node2->key + key_move, (node2->key_cnt - key_move) * sizeof(KeyType)); // BUG
        // move
        std::memcpy(node1->idx.key_ptr + node1->key_cnt + 1, node2->idx.key_ptr,
                    ptr_move * sizeof(bstar_node *)); // ?
        // adjust
        std::memmove(node2->idx.key_ptr, node2->idx.key_ptr + ptr_move,
                     (node2->key_cnt + 1 - ptr_move) * sizeof(bstar_node *));
        node1->key_cnt = need1;
        node2->key_cnt = need2;
        return new_delim;
      }
    }

    // nothing should be changed.
    return parent->key[idx1];
  }

  void insert_key_in_parent(bstar_node *node1, bstar_node *node2, bstar_node *parent, std::size_t idx1,
                            const KeyType &new_key) {
    std::memmove(parent->key + idx1 + 1, parent->key + idx1, (parent->key_cnt - idx1) * sizeof(KeyType));
    std::memmove(parent->idx.key_ptr + idx1 + 2, parent->idx.key_ptr + idx1 + 1,
                 (parent->key_cnt - idx1) * sizeof(bstar_node *));
    parent->key[idx1] = new_key;
    parent->idx.key_ptr[idx1] = node1;
    parent->idx.key_ptr[idx1 + 1] = node2;
    parent->key_cnt++;
  }

  void modify_key_in_parent_(bstar_node *node1, bstar_node *node2, bstar_node *parent, std::size_t idx1,
                             const KeyType &new_key) {
    parent->key[idx1] = new_key;
    parent->idx.key_ptr[idx1] = node1;
    parent->idx.key_ptr[idx1 + 1] = node2;
  }

  void try_reduce_root_() {
    if (root->key_cnt == 0 && !root->is_leaf) {
      bstar_node *new_root = root->idx.key_ptr[0];
      delete root;
      root = new_root;
    }
  }

  void do_1_2_split_root_() {

    bstar_node *new_root{new bstar_node{.key_cnt = 0, .is_leaf = false}};
    bstar_node *node2{new bstar_node{.key_cnt = 0, .is_leaf = root->is_leaf}};
    if (root->is_leaf) {
      std::size_t need1{(root->key_cnt + 1) / 2};
      std::size_t need2{root->key_cnt / 2};
      std::memcpy(node2->key, root->key + need1, need2 * sizeof(KeyType));
      std::memcpy(node2->leaf.data_ptr, root->leaf.data_ptr + need1, need2 * sizeof(ValType *));
      new_root->key[new_root->key_cnt++] = node2->key[0];
      root->leaf.next = node2;
      root->key_cnt = need1;
      node2->key_cnt = need2;
    } else {
      std::size_t need1{(root->key_cnt) / 2};
      std::size_t need2{(root->key_cnt - 1) / 2};
      std::memcpy(node2->key, root->key + need1 + 1, need2 * sizeof(KeyType));
      std::memcpy(node2->idx.key_ptr, root->idx.key_ptr + need1 + 1, (need2 + 1) * sizeof(bstar_node *));
      new_root->key[new_root->key_cnt++] = root->key[need1];
      root->key_cnt = need1;
      node2->key_cnt = need2;
    }
    new_root->idx.key_ptr[0] = root;
    new_root->idx.key_ptr[1] = node2;
    root = new_root;
  }

  void do_2_3_split_(bstar_node *node1, bstar_node *node2, bstar_node *parent, std::size_t idx1, std::size_t idx2) {
    // first fill node3
    bstar_node *node3{new bstar_node{.key_cnt = 0, .is_leaf = node1->is_leaf}};

    if (!node1->is_leaf) {
      insert_key_in_parent(node2, node3, parent, idx2, node2->key[node2->key_cnt - 1]);
      node3->idx.key_ptr[0] = node2->idx.key_ptr[node2->key_cnt];
      node2->key_cnt--;
    }

    std::size_t total{node1->key_cnt + node2->key_cnt};
    std::size_t need1{(total + 2) / 3};
    std::size_t need2{(total + 1) / 3};
    std::size_t need3{total / 3};
    std::size_t keep2 = node2->key_cnt - need3;
    // experiment

    KeyType new_key1 = redistribute_keys_(node2, node3, keep2, need3, parent, idx2);

    if (node1->is_leaf) {
      insert_key_in_parent(node2, node3, parent, idx2, new_key1);
    } else {
      modify_key_in_parent_(node2, node3, parent, idx2, new_key1);
    }

    //

    KeyType new_key2 = redistribute_keys_(node1, node2, need1, need2, parent, idx1);
    modify_key_in_parent_(node1, node2, parent, idx1, new_key2);

    if (node1->is_leaf) {
      node3->leaf.next = node2->leaf.next;
      node2->leaf.next = node3;
    }
  }

  // todo

  void do_2_1_merge_(bstar_node *node1, bstar_node *node2, bstar_node *parent, std::size_t idx1, std::size_t idx2) {
  }

  void do_3_2_merge_(bstar_node *node1, bstar_node *node2, bstar_node *node3, bstar_node *parent, std::size_t idx1,
                     std::size_t idx2, std::size_t idx3) {
  }

  void do_equal_split_(bstar_node *node1, bstar_node *node2, bstar_node *parent, std::size_t idx1) {

    std::size_t total{node1->key_cnt + node2->key_cnt};
    std::size_t need1{total / 2};
    std::size_t need2{total - need1};

    KeyType new_key = redistribute_keys_(node1, node2, need1, need2, parent, idx1); // BUG
    modify_key_in_parent_(node1, node2, parent, idx1, new_key);
  }

  void try_fix_overflow(bstar_node *node1, bstar_node *parent, std::size_t idx1) {
    if (!parent) {
      do_1_2_split_root_();
      return;
    }
    std::size_t idx2{idx1 + 1};
    bstar_node *node2{};
    if (idx1 + 1 <= parent->key_cnt) { // ptr index can be key_cnt
      idx2 = idx1 + 1;
      node2 = parent->idx.key_ptr[idx2];
    }
    if (idx1 > 0) {
      idx2 = idx1 - 1;
      node2 = parent->idx.key_ptr[idx2];
      std::swap(idx1, idx2);
      std::swap(node1, node2);
    }
    if (!will_overflow_(node1, node2) && !will_underflow_(node1, node2)) {
      do_equal_split_(node1, node2, parent, idx1);
      return;
    }
    do_2_3_split_(node1, node2, parent, idx1, idx2);
  }

  // todo
  void try_fix_underflow(bstar_node *node1, bstar_node *parent, std::size_t idx1) {
  }

public:
  bstar_tree() {
    root = new bstar_node{.key_cnt = 0, .is_leaf = true};
  }
  ~bstar_tree() {
    std::vector<bstar_node *> decon{};
    decon.emplace_back(root);
    while (!decon.empty()) {
      bstar_node *cur{decon.back()};
      decon.pop_back();
      if (!cur->is_leaf) {
        for (std::size_t i = 0; i <= cur->key_cnt; i++) {
          if (cur->idx.key_ptr[i]) {
            decon.emplace_back(cur->idx.key_ptr[i]);
          }
        }
      }
      delete cur;
    }
  }

  bool insert(const KeyType &k, ValType *v) {
    bstar_node *cur{root}, *prev{};
    std::size_t cur_from{};
    while (true) {
      bool is_split{};
      if (overflow_(cur)) {
        try_fix_overflow(cur, prev, cur_from);
        if (!prev) prev = root;
        is_split = true;
      }
      if (is_split) {
        cur_from = prev->find_idx_ptr_index_(k);
        cur = prev->idx.key_ptr[cur_from];
        continue;
      } else {
        if (cur->is_leaf) break;
        cur_from = cur->find_idx_ptr_index_(k);
        prev = cur;
        cur = cur->idx.key_ptr[cur_from];
      }
    }
    std::size_t check{cur->find_data_ptr_index_(k)};
    std::size_t idx{cur->find_idx_ptr_index_(k)};
    if (idx != check) {
      return false;
    }
    if (idx != cur->key_cnt) {
      std::memmove(cur->leaf.data_ptr + idx + 1, cur->leaf.data_ptr + idx, (cur->key_cnt - idx) * sizeof(ValType *));
      std::memmove(cur->key + idx + 1, cur->key + idx, (cur->key_cnt - idx) * sizeof(KeyType));
    }
    cur->leaf.data_ptr[idx] = v;
    cur->key[idx] = k;
    cur->key_cnt++;
    return true;
  }

  std::vector<ValType *> find(const KeyType &k) const {
    bstar_node *cur{root};
    while (cur && !cur->is_leaf) {
      cur = cur->idx.key_ptr[cur->find_data_ptr_index_(k)];
    }
    std::vector<ValType *> values{};
    while (cur) {
      std::size_t beg{cur->find_data_ptr_index_(k)}, end{cur->find_idx_ptr_index_(k)};
      if (end == 0) break;
      for (std::size_t i = beg; i < end; i++) {
        values.emplace_back(cur->leaf.data_ptr[i]);
      }
      cur = cur->leaf.next;
    }
    return values;
  }
};

using namespace std;
using ll = long long;

#define SCALE 100000000

void bstar_benchmark() {
  printf("bstar benchmark.\n");
  struct timespec beg{}, end{};

  bstar_tree<ll, ll, 150> t{};
  for (std::size_t i = 0; i < SCALE; i++) {
    t.insert(i, (ll *)i);
  }

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &beg);
  vector<ll *> ans{};
  for (std::size_t i = 0; i < SCALE; i++) {
    ans = t.find(i);
    if (ans.size() != 1 || (ll)ans[0] != i) {
      fprintf(stderr, "[ERROR] FIND FAILED.");
    }
  }
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);

  printf("START FROM %ld.%09ld\n", beg.tv_sec, beg.tv_nsec);
  printf("END IN %ld.%09ld\n", end.tv_sec, end.tv_nsec);
  printf("SUCCESS!\n");
}

void stdmap_benchmark() {
  printf("stdmap benchmark.\n");
  struct timespec beg{}, end{};
  map<ll, ll *> m{};
  auto hint = m.end();
  for (std::size_t i = 0; i < SCALE; i++) {
    // m.emplace(i, (ll *)i);
    hint = m.emplace_hint(hint, i, (ll *)i);
  }

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &beg);
  for (std::size_t i = 0; i < SCALE; i++) {
    auto ans = m.find(i);
    if ((ll)ans->second != i) {
      fprintf(stderr, "[ERROR] FIND FAILED");
    }
  }
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);

  printf("START FROM %ld.%09ld\n", beg.tv_sec, beg.tv_nsec);
  printf("END IN %ld.%09ld\n", end.tv_sec, end.tv_nsec);
  printf("SUCCESS!\n");
}

int main() {

  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  stdmap_benchmark();
  bstar_benchmark();

  return 0;
}

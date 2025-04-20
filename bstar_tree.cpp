#include <bits/stdc++.h>

// each node has max of `M` branches,
// for index node, `M - 1` keys,
// for leaf node, `M` keys.

// `M` >= 4
template <typename KeyType, typename ValType, std::size_t M>
class bstar_tree {
private:
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
      bstar_node *ptrs[M]{};
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

  // will update node key_cnt.
  // must fix the father delim key!
  void balance_nodes_(bstar_node *node1, std::size_t need1, bstar_node *node2, std::size_t need2) {
    std::size_t move_cnt{};
    if (node1->key_cnt >= need1) {
      move_cnt = node1->key_cnt - need1;
      std::memmove(node2->key + move_cnt, node2->key, node2->key_cnt * sizeof(KeyType));
      std::memmove(node2->key, node1->key + need1, move_cnt * sizeof(KeyType));
      if (node1->is_leaf) {
        std::memmove(node2->leaf.data_ptr + move_cnt, node2->leaf.data_ptr, move_cnt * sizeof(ValType *));
        std::memmove(node2->leaf.data_ptr, node1->leaf.data_ptr + need1, move_cnt * sizeof(ValType *));
      } else {
        std::memmove(node2->idx.key_ptr + move_cnt + 1, node2->idx.key_ptr + 1, move_cnt * sizeof(bstar_node *));
        std::memmove(node2->idx.key_ptr + 1, node1->idx.key_ptr + need1 + 1, move_cnt * sizeof(bstar_node *));
      }
      node1->key_cnt = need1;
      node2->key_cnt = need2;

    } else if (node1->key_cnt < need1) {
      move_cnt = need1 - node1->key_cnt;
      std::memmove(node1->key + node1->key_cnt, node2->key, move_cnt * sizeof(KeyType));
      std::memmove(node2->key, node2->key + move_cnt, (node2->key_cnt - move_cnt) * sizeof(KeyType));
      if (node1->is_leaf) {
        std::memmove(node1->leaf.data_ptr + node1->key_cnt, node2->leaf.data_ptr, move_cnt * sizeof(ValType *));
        std::memmove(node2->leaf.data_ptr, node2->leaf.data_ptr + move_cnt, move_cnt * sizeof(ValType *));
      } else {
        std::memmove(node1->idx.key_ptr + node1->key_cnt + 1, node2->idx.key_ptr + 1, move_cnt * sizeof(bstar_node *));
        std::memmove(node2->idx.key_ptr + 1, node2->idx.key_ptr + move_cnt + 1, move_cnt * sizeof(bstar_node *));
      }
      node1->key_cnt = need1;
      node2->key_cnt = need2;
    }
  }

  void try_reduce_root_() {
    if (root->key_cnt == 0 && !root->is_leaf) {
      bstar_node *new_root = root->idx.key_ptr[0];
      delete root;
      root = new_root;
    }
  }

  void do_2_1_merge_root_() {
    if (root->key_cnt == 1) {
      bstar_node *node1{root->idx.key_ptr[0]}, *node2{root->idx.key_ptr[1]};
      std::size_t total{node1->key_cnt + node2->key_cnt};
      balance_nodes_(node1, total, node2, 0);
      delete node2;
      root->key_cnt = 0;
      try_reduce_root_();
    }
  }

  void do_1_2_split_(bstar_node *node1, bstar_node *parent, std::size_t idx1) {
    std::size_t need1{(node1->key_cnt + 1) / 2};
    std::size_t need2{node1->key_cnt / 2};
    bstar_node *node2{new bstar_node{.key_cnt = 0, .is_leaf = node1->is_leaf}};
    balance_nodes_(node1, need1, node2, need2);
    if (node1->is_leaf) {
      node2->leaf.next = node1->leaf.next;
      node1->leaf.next = node2;
    }
    if (!parent) {
      bstar_node *new_root{new bstar_node{.key_cnt = 1, .is_leaf = false}};
      new_root->key[0] = node2->key[0];
      new_root->idx.key_ptr[0] = node1;
      new_root->idx.key_ptr[1] = node2;
      root = new_root;
    } else {
      // parent->idx.key_ptr[idx1 + 1] = node2;
      std::memmove(parent->key + idx1 + 1, parent->key + idx1, (parent->key_cnt - idx1) * sizeof(KeyType));
      std::memmove(parent->idx.key_ptr + idx1 + 2, parent->idx.key_ptr + idx1 + 1,
                   (parent->key_cnt - idx1) * sizeof(bstar_node));
      parent->key[idx1] = node2->key[0];
      parent->idx.key_ptr[idx1 + 1] = node2;
      parent->key_cnt++;
    }
  }

  void do_2_3_split_(bstar_node *node1, bstar_node *node2, bstar_node *parent, std::size_t idx1, std::size_t idx2) {
    std::size_t total{node1->key_cnt + node2->key_cnt};
    std::size_t need1{(total + 2) / 3};
    std::size_t need2{(total + 1) / 3};
    std::size_t need3{total / 3};
    bstar_node *node3{new bstar_node{.key_cnt = 0, .is_leaf = node1->is_leaf}};
    // first fill node3
    balance_nodes_(node2, need2, node3, need3);
    balance_nodes_(node1, need1, node2, need2);

    if (node1->is_leaf) {
      node3->leaf.next = node2->leaf.next;
      node2->leaf.next = node3;
    }

    // parent must be a index node.
    // add 1 new key.
    std::memmove(parent->key + idx2 + 1, parent->key + idx2, (parent->key_cnt - idx2) * sizeof(KeyType));
    std::memmove(parent->idx.key_ptr + idx2 + 2, parent->idx.key_ptr + idx2 + 1,
                 (parent->key_cnt - idx2) * sizeof(bstar_node *));
    parent->key[idx1] = node2->key[0];
    parent->idx.key_ptr[idx1 + 1] = node2;
    parent->key[idx2] = node3->key[0];
    parent->idx.key_ptr[idx2 + 1] = node3;
    parent->key_cnt++;
  }

  // todo
  void do_3_2_merge_(bstar_node *node1, bstar_node *node2, bstar_node *parent, std::size_t idx1, std::size_t idx2) {
  }

  void do_equal_split_(bstar_node *node1, bstar_node *node2, bstar_node *parent, std::size_t idx1) {
    std::size_t total{node1->key_cnt + node2->key_cnt};
    std::size_t need1{total / 2};
    std::size_t need2{total - need1};
    balance_nodes_(node1, need1, node2, need2);
    parent->key[idx1] = node2->key[0];
  }

  void try_fix_overflow(bstar_node *node1, bstar_node *parent, std::size_t idx1) {
    if (!parent) {
      do_1_2_split_(root, NULL, 0);
      return;
    }
    std::size_t idx2{idx1 + 1};
    bstar_node *node2{};
    if (idx1 + 1 <= parent->key_cnt) { // ptr index can be key_cnt
      idx2 = idx1 + 1;
      node2 = parent->idx.key_ptr[idx2];
      if (!node2) {
        do_1_2_split_(node1, parent, idx1);
        return;
      }
    }
    if (idx1 > 0) {
      idx2 = idx1 - 1;
      node2 = parent->idx.key_ptr[idx2];
      if (!node2) {
        do_1_2_split_(node1, parent, idx1);
        return;
      }
      std::swap(idx1, idx2);
      std::swap(node1, node2);
    }
    if (!will_overflow_(node1, node2) && !will_underflow_(node1, node2)) {
      do_equal_split_(node1, node2, parent, idx1);
      return;
    }
    do_2_3_split_(node1, node2, parent, idx1, idx2);
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
    std::size_t idx{cur->find_idx_ptr_index_(k)};
    if (idx != cur->key_cnt) {
      std::memmove(cur->leaf.data_ptr + idx + 1, cur->leaf.data_ptr + idx, (cur->key_cnt - idx) * sizeof(ValType *));
      std::memmove(cur->key + idx + 1, cur->key + idx, (cur->key_cnt - idx) * sizeof(KeyType));
    }
    cur->leaf.data_ptr[idx] = v;
    cur->key[idx] = k;
    cur->key_cnt++;
    return true;
  }

  std::vector<ValType *> find(const KeyType &k) {
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

  void DEBUG_traverse(bstar_node *cur) {
    using namespace std;
    if (cur->is_leaf) {
      printf("reach leaf.\n");
      for (int i = 0; i < cur->key_cnt; i++) {
        printf("\t[key] %d, [val] %d \n", cur->key[i], *cur->leaf.data_ptr[i]);
      }
      printf("end.\n");
    } else {
      for (int i = 0; i < cur->key_cnt + 1; i++) {
        if (cur->idx.key_ptr[i] != NULL) {

          DEBUG_traverse(cur->idx.key_ptr[i]);
        }
      }
    }
  }
  bstar_node *get_root() {
    return root;
  }

  // todo
  std::vector<ValType *> erase(const KeyType &k) {
  }
};

int main() {
  using namespace std;

  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  bstar_tree<int, int, 5> tree{};

  int arr[100]{};
  for (int i = 0; i < 100; i++) {
    arr[i] = 5;

    tree.insert(3, &arr[i]);
  }

  // tree.DEBUG_traverse(tree.get_root());

  std::vector<int *> data{tree.find(3)};
  cout << data.size() << endl;

  for (int i = 0; i < data.size(); i++) {
    std::cout << *data[i] << std::endl;
  }

  return 0;
}
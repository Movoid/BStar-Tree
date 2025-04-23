#include <bits/stdc++.h>

// each node has max of `M` branches,
// for index node, `M - 1` keys,
// for leaf node, `M` keys.

/*================================================*\

  B+* tree.
  No support for duplicate mainkey.

\*================================================*/

// `M` >= 4
template <typename KeyType, typename ValType, std::size_t M,
          typename _Requires = std::void_t<std::enable_if_t<M >= 4>,
                                           decltype(std::declval<KeyType>() <
                                                    std::declval<KeyType>())>>
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
  static constexpr std::size_t KEY_SLOTS = M - 1;
  static constexpr std::size_t MAX_KEYS = KEY_SLOTS - 1;
  static constexpr std::size_t MIN_KEYS = (2 * (MAX_KEYS) + 2) / 3; // ceil

private:
  // ceil((a + b) / 2)
  static constexpr std::size_t ceil_half(std::size_t a,
                                         std::size_t b) noexcept {
    return (a >> 1) + (b >> 1) + ((a & 1) + (b & 1) + 1) / 2;
  }

  static inline bool overflow_(const bstar_node *n) noexcept {
    return n->key_cnt > MAX_KEYS;
  }
  static inline bool average_overflow_(const bstar_node *a,
                                       const bstar_node *b) noexcept {
    return ceil_half(a->key_cnt, b->key_cnt) > MAX_KEYS;
  }

  static inline bool underflow_(const bstar_node *n) noexcept {
    return n->key_cnt < MIN_KEYS;
  }
  static inline bool average_underflow_(const bstar_node *a,
                                        const bstar_node *b) noexcept {
    return ceil_half(a->key_cnt, b->key_cnt) < MIN_KEYS;
  }

  void DEBUG_print_node(bstar_node *node) {
    printf("%s Node %p:\n", (node->is_leaf ? "LEAF" : "INDEX"), node);
    printf("\t");
    if (!node) {
      printf("<NULL>");
    }
    for (int i = 0; i < node->key_cnt; i++) {
      printf("%d ", node->key[i]);
    }
    puts("");
  }

  void DEBUG_print_parent(bstar_node *node, std::size_t idx1,
                          std::size_t idx2) {
    printf("%s Parent Node %p:\n", (node->is_leaf ? "LEAF" : "INDEX"), node);
    printf("\t");
    if (!node) {
      printf("<NULL>");
    }
    for (int i = 0; i < node->key_cnt; i++) {
      if (i == idx1 || i == idx2) {
        printf("[%d] ", node->key[i]);
      } else {
        printf("%d ", node->key[i]);
      }
    }
    puts("");
  }

  // standard balance, parent != nullptr
  KeyType redistribute_keys_(bstar_node *node1, bstar_node *node2,
                             std::size_t need1, std::size_t need2,
                             bstar_node *parent, std::size_t idx1) {

    std::size_t total{node1->key_cnt + node2->key_cnt};
    if (node1->key_cnt > need1) {
      if (node1->is_leaf) {
        std::size_t key_move{node1->key_cnt - need1};
        std::size_t ptr_move{key_move};
        // adjust keys
        std::memmove(node2->key + key_move, node2->key,
                     node2->key_cnt * sizeof(KeyType));
        // move keys
        std::memcpy(node2->key, node1->key + need1, key_move * sizeof(KeyType));
        // adjust ptrs
        std::memmove(node2->leaf.data_ptr + ptr_move, node2->leaf.data_ptr,
                     node2->key_cnt * sizeof(ValType *));
        // move ptrs
        std::memcpy(node2->leaf.data_ptr, node1->leaf.data_ptr + need1,
                    ptr_move * sizeof(ValType *));
        node1->key_cnt = need1;
        node2->key_cnt = need2;
        return node2->key[0];
      } else { // special branch
        std::size_t key_move{node1->key_cnt - need1};
        std::size_t ptr_move{key_move};
        KeyType new_delim{node1->key[need1]};
        // adjust
        std::memmove(node2->key + key_move, node2->key,
                     node2->key_cnt * sizeof(KeyType));
        // move
        std::memcpy(node2->key, node1->key + need1 + 1,
                    (key_move - 1) * sizeof(KeyType));
        // if parent is nullptr? node1 is root?
        // this function will not check this. should be guaranteed valid.
        node2->key[key_move - 1] = parent->key[idx1];
        // adjust
        std::memmove(node2->idx.key_ptr + ptr_move, node2->idx.key_ptr,
                     (node2->key_cnt + 1) * sizeof(bstar_node *));
        // move
        std::memcpy(node2->idx.key_ptr, node1->idx.key_ptr + need1 + 1,
                    ptr_move * sizeof(bstar_node *));
        node1->key_cnt = need1;
        node2->key_cnt = need2;
        return new_delim;
      }
    } else if (node1->key_cnt < need1) {
      if (node1->is_leaf) {
        std::size_t key_move{need1 - node1->key_cnt};
        std::size_t ptr_move{key_move};
        // move
        std::memcpy(node1->key + node1->key_cnt, node2->key,
                    (key_move) * sizeof(KeyType));
        // adjust
        std::memmove(node2->key, node2->key + key_move,
                     (node2->key_cnt - key_move) * sizeof(KeyType));
        // move
        std::memcpy(node1->leaf.data_ptr + node1->key_cnt, node2->leaf.data_ptr,
                    (ptr_move) * sizeof(ValType *));
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
        std::memcpy(node1->key + node1->key_cnt + 1, node2->key,
                    (key_move - 1) * sizeof(KeyType));
        // adjust
        std::memmove(node2->key, node2->key + key_move,
                     (node2->key_cnt - key_move) * sizeof(KeyType)); // BUG
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

  void insert_key_in_parent(bstar_node *node1, bstar_node *node2,
                            bstar_node *parent, std::size_t idx1,
                            const KeyType &new_key) {
    std::memmove(parent->key + idx1 + 1, parent->key + idx1,
                 (parent->key_cnt - idx1) * sizeof(KeyType));
    std::memmove(parent->idx.key_ptr + idx1 + 2, parent->idx.key_ptr + idx1 + 1,
                 (parent->key_cnt - idx1) * sizeof(bstar_node *));
    parent->key[idx1] = new_key;
    parent->idx.key_ptr[idx1] = node1;
    parent->idx.key_ptr[idx1 + 1] = node2;
    parent->key_cnt++;
  }

  void modify_key_in_parent_(bstar_node *node1, bstar_node *node2,
                             bstar_node *parent, std::size_t idx1,
                             const KeyType &new_key) {
    parent->key[idx1] = new_key;
    parent->idx.key_ptr[idx1] = node1;
    parent->idx.key_ptr[idx1 + 1] = node2;
  }

  void remove_key_in_parent_(bstar_node *node1, bstar_node *node2,
                             bstar_node *parent, std::size_t idx1) {
    std::memmove(parent->key + idx1, parent->key + idx1 + 1,
                 (parent->key_cnt - idx1) * sizeof(KeyType));
    std::memmove(parent->idx.key_ptr + idx1 + 1, parent->idx.key_ptr + idx1 + 2,
                 (parent->key_cnt - idx1) * sizeof(bstar_node *));
    parent->key_cnt--;
  }

  void do_1_2_split_root_() {
    printf("\n[1-2 split root] Start:\n");
    DEBUG_print_node(root);
    bstar_node *new_root{new bstar_node{.key_cnt = 0, .is_leaf = false}};
    bstar_node *node2{new bstar_node{.key_cnt = 0, .is_leaf = root->is_leaf}};
    if (root->is_leaf) {
      std::size_t need1{(root->key_cnt + 1) / 2};
      std::size_t need2{root->key_cnt / 2};
      std::memcpy(node2->key, root->key + need1, need2 * sizeof(KeyType));
      std::memcpy(node2->leaf.data_ptr, root->leaf.data_ptr + need1,
                  need2 * sizeof(ValType *));
      new_root->key[new_root->key_cnt++] = node2->key[0];
      root->leaf.next = node2;
      root->key_cnt = need1;
      node2->key_cnt = need2;
    } else {
      std::size_t need1{(root->key_cnt) / 2};
      std::size_t need2{(root->key_cnt - 1) / 2};
      std::memcpy(node2->key, root->key + need1 + 1, need2 * sizeof(KeyType));
      std::memcpy(node2->idx.key_ptr, root->idx.key_ptr + need1 + 1,
                  (need2 + 1) * sizeof(bstar_node *));
      new_root->key[new_root->key_cnt++] = root->key[need1];
      root->key_cnt = need1;
      node2->key_cnt = need2;
    }
    new_root->idx.key_ptr[0] = root;
    new_root->idx.key_ptr[1] = node2;
    printf("[1-2 split root] Result:\n");
    DEBUG_print_node(root);
    DEBUG_print_node(node2);
    DEBUG_print_parent(new_root, 0, 1);
    root = new_root;
  }

  void do_2_1_merge_root_() {
    bstar_node *node1{root->idx.key_ptr[0]};
    bstar_node *node2{root->idx.key_ptr[1]};
    if (node1->is_leaf) {
      // key
      std::memmove(root->key, node1->key, node1->key_cnt * sizeof(KeyType));
      std::memmove(root->key + node1->key_cnt, node2->key,
                   node2->key_cnt * sizeof(KeyType));
      // ptr
      std::memmove(root->leaf.data_ptr, node1->leaf.data_ptr,
                   node1->key_cnt * sizeof(ValType *));
      std::memmove(root->leaf.data_ptr + node1->key_cnt, node2->leaf.data_ptr,
                   node2->key_cnt * sizeof(ValType *));
      root->key_cnt = node1->key_cnt + node2->key_cnt;
      root->is_leaf = true;
      root->leaf.next = nullptr;
    } else {
      // key
      root->key[node1->key_cnt] = root->key[0];
      std::memmove(root->key, node1->key, node1->key_cnt * sizeof(KeyType));
      std::memmove(root->key + node1->key_cnt + 1, node2->key,
                   node2->key_cnt * sizeof(KeyType));
      // ptr
      std::memmove(root->idx.key_ptr, node1->idx.key_ptr,
                   (node1->key_cnt + 1) * sizeof(bstar_node *));
      std::memmove(root->idx.key_ptr + node1->key_cnt + 1, node2->idx.key_ptr,
                   (node2->key_cnt + 1) * sizeof(bstar_node *));
      root->key_cnt = node1->key_cnt + node2->key_cnt + 1;
    }
    // delete !!!
    delete node1;
    delete node2;
    return;
  }

  void do_2_3_split_(bstar_node *node1, bstar_node *node2, bstar_node *parent,
                     std::size_t idx1, std::size_t idx2) {
    printf("\n[2-3 split] Start:\n");
    DEBUG_print_node(node1);
    DEBUG_print_node(node2);
    DEBUG_print_parent(parent, idx1, idx2);
    // first fill node3
    bstar_node *node3{new bstar_node{.key_cnt = 0, .is_leaf = node1->is_leaf}};

    if (!node1->is_leaf) {
      insert_key_in_parent(node2, node3, parent, idx2,
                           node2->key[node2->key_cnt - 1]);
      node3->idx.key_ptr[0] = node2->idx.key_ptr[node2->key_cnt];
      node2->key_cnt--;
    }

    std::size_t total{node1->key_cnt + node2->key_cnt};
    std::size_t need1{(total + 2) / 3};
    std::size_t need2{(total + 1) / 3};
    std::size_t need3{total / 3};
    std::size_t keep2 = node2->key_cnt - need3;
    printf("\n<redistribute> Start:\n");
    DEBUG_print_node(node1);
    DEBUG_print_node(node2);
    DEBUG_print_parent(parent, idx1, parent->key_cnt);
    // experiment

    KeyType new_key1 =
        redistribute_keys_(node2, node3, keep2, need3, parent, idx2);

    if (node1->is_leaf) {
      insert_key_in_parent(node2, node3, parent, idx2, new_key1);
    } else {
      modify_key_in_parent_(node2, node3, parent, idx2, new_key1);
    }

    //
    printf("\n<redistribute> Phase1:\n");
    DEBUG_print_node(node2);
    DEBUG_print_node(node3);
    DEBUG_print_parent(parent, idx2, parent->key_cnt);
    KeyType new_key2 =
        redistribute_keys_(node1, node2, need1, need2, parent, idx1);
    modify_key_in_parent_(node1, node2, parent, idx1, new_key2);
    printf("\n<redistribute> Result:\n");
    DEBUG_print_node(node1);
    DEBUG_print_node(node2);
    DEBUG_print_parent(parent, idx1, parent->key_cnt);

    if (node1->is_leaf) {
      node3->leaf.next = node2->leaf.next;
      node2->leaf.next = node3;
    }

    // parent must be a index node.

    printf("[2-3 split] Result:\n");
    DEBUG_print_node(node1);
    DEBUG_print_node(node2);
    DEBUG_print_node(node3);
    DEBUG_print_parent(parent, idx1, idx2);
  }

  void do_3_2_merge_(bstar_node *node1, bstar_node *node2, bstar_node *node3,
                     bstar_node *parent, std::size_t idx1, std::size_t idx2,
                     std::size_t idx3) {
    // todo
    std::size_t total{node1->key_cnt + node2->key_cnt + node3->key_cnt};
    std::size_t need1{(total + 1) / 2};
    std::size_t need2{total / 2};
  }

  void do_equal_split_(bstar_node *node1, bstar_node *node2, bstar_node *parent,
                       std::size_t idx1) {
    printf("\n[equal-split] Start:\n");
    DEBUG_print_node(node1);
    DEBUG_print_node(node2);
    DEBUG_print_parent(parent, idx1, parent->key_cnt);
    std::size_t total{node1->key_cnt + node2->key_cnt};
    std::size_t need1{total / 2};
    std::size_t need2{total - need1};

    printf("\n<redistribute> Start:\n");
    DEBUG_print_node(node1);
    DEBUG_print_node(node2);
    DEBUG_print_parent(parent, idx1, parent->key_cnt);
    KeyType new_key =
        redistribute_keys_(node1, node2, need1, need2, parent, idx1); // BUG
    modify_key_in_parent_(node1, node2, parent, idx1, new_key);

    printf("\n<redistribute> Result:\n");
    DEBUG_print_node(node1);
    DEBUG_print_node(node2);
    DEBUG_print_parent(parent, idx1, parent->key_cnt);

    printf("[equal-split] Result:\n");
    DEBUG_print_node(node1);
    DEBUG_print_node(node2);
    DEBUG_print_parent(parent, idx1, parent->key_cnt);
  }

  void try_fix_overflow(bstar_node *node1, bstar_node *parent,
                        std::size_t idx1) {
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
    if (!average_overflow_(node1, node2) && !average_underflow_(node1, node2)) {
      do_equal_split_(node1, node2, parent, idx1);
      return;
    }
    do_2_3_split_(node1, node2, parent, idx1, idx2);
  }

  // todo
  void try_fix_underflow(bstar_node *node1, bstar_node *parent,
                         std::size_t idx1) {
    if (!parent || (parent == root && parent->key_cnt == 1)) {
      do_2_1_merge_root_();
      return;
    }
  }

public:
  bstar_tree() { root = new bstar_node{.key_cnt = 0, .is_leaf = true}; }
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
        if (!prev)
          prev = root;
        is_split = true;
      }
      if (is_split) {
        cur_from = prev->find_idx_ptr_index_(k);
        cur = prev->idx.key_ptr[cur_from];
        continue;
      } else {
        if (cur->is_leaf)
          break;
        cur_from = cur->find_idx_ptr_index_(k);
        prev = cur;
        cur = cur->idx.key_ptr[cur_from];
      }
    }
    printf("\n[insert] Start:\n");
    DEBUG_print_node(cur);
    if (prev)
      DEBUG_print_parent(prev, cur_from, prev->key_cnt);
    std::size_t check{cur->find_data_ptr_index_(k)};
    std::size_t idx{cur->find_idx_ptr_index_(k)};
    if (idx != check) {
      return false;
    }
    if (idx != cur->key_cnt) {
      std::memmove(cur->leaf.data_ptr + idx + 1, cur->leaf.data_ptr + idx,
                   (cur->key_cnt - idx) * sizeof(ValType *));
      std::memmove(cur->key + idx + 1, cur->key + idx,
                   (cur->key_cnt - idx) * sizeof(KeyType));
    }
    cur->leaf.data_ptr[idx] = v;
    cur->key[idx] = k;
    cur->key_cnt++;
    printf("[insert] Result:\n");
    DEBUG_print_node(cur);
    if (prev)
      DEBUG_print_parent(prev, cur_from, prev->key_cnt);
    return true;
  }

  std::vector<ValType *> find(const KeyType &k) const {
    bstar_node *cur{root};
    while (cur && !cur->is_leaf) {
      cur = cur->idx.key_ptr[cur->find_data_ptr_index_(k)];
    }
    std::vector<ValType *> values{};
    while (cur) {
      std::size_t beg{cur->find_data_ptr_index_(k)},
          end{cur->find_idx_ptr_index_(k)};
      if (end == 0)
        break;
      for (std::size_t i = beg; i < end; i++) {
        values.emplace_back(cur->leaf.data_ptr[i]);
      }
      cur = cur->leaf.next;
    }
    return values;
  }

  [[gnu::noinline]] bstar_node *get_root() { return root; }

  void traverse_full_tree() {
    printf("\n[TRAVERSE FULL]\n");
    bstar_node *cur{root};
    std::vector<ValType> a{};
    while (!cur->is_leaf) {
      cur = cur->idx.key_ptr[0];
    }
    while (cur) {
      for (int i = 0; i < cur->key_cnt; i++) {
        a.emplace_back(cur->key[i]);
        printf("%d ", cur->key[i]);
      }
      cur = cur->leaf.next;
    }
    printf("All keys searched %zu.\n", a.size());
    puts("");
  }

  // todo
  std::vector<ValType *> erase(const KeyType &k) {}
};

void DEBUG_search_full(bstar_tree<int, int, 4> &tree, int ts) {
  std::vector<int *> ans{};
  for (int i = 0; i <= ts; i++) {
    ans = tree.find(i);
    if (ans.size() != 1) {
      printf("\n[ERROR] TREE BROKEN IN TIMESTAMP %d.\n", ts);
      printf("\n[ERROR] KEY %d NOT FOUND.\n", i);
    }
  }
}

int main() {
  using namespace std;

  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

#define SCALE 1000
#define T 1
  random_device rd{};
  mt19937 gen{3};
  uniform_int_distribution<> dis{0, SCALE - 1};

  int t = T;
  int *ptrs[T]{};

  bool pass{true};

  while (t--) {
    ptrs[t] = (int *)malloc(sizeof(int) * SCALE);
    bstar_tree<int, int, 4> tree{};
    auto a = tree.get_root();
    int search_key = dis(gen);

    vector<int> ans;
    vector<int *> ret;
    ans.reserve(1); // 只有一次匹配
    // 插入并在碰到 search_key 时记录下答案
    for (int i = 0; i < SCALE; i++) {
      ptrs[t][i] = dis(gen);
      printf("\nTIMESTAMP %d\n", i);
      tree.insert(i, &ptrs[t][i]);
      if (i == search_key) {
        ans.emplace_back(ptrs[t][i]);
      }
      DEBUG_search_full(tree, i);
    }

    tree.traverse_full_tree();

    // 查找
    printf("Try search: %d.\n", search_key);
    ret = tree.find(search_key);

    // 先校验数量
    if (ret.size() != ans.size()) {
      fprintf(stderr, "SIZE MISMATCH: expected %zu, got %zu\n", ans.size(),
              ret.size());
      pass = false;
    }

    // 按实际元素个数比较
    size_t n = min(ret.size(), ans.size());
    for (size_t i = 0; i < n; i++) {
      if (*ret[i] != ans[i]) {
        fprintf(stderr, "COMPARE FAILED at index %zu: expected %d, got %d\n", i,
                ans[i], *ret[i]);
        pass = false;
      }
    }

    printf("ROUND %d FINISHED.\n", T - t);

    free(ptrs[t]);
  }

  if (pass) {
    printf("SUCCESSFULLY PASSED ALL ROUNDS!\n");
  } else {
    printf("FAILED.\n");
  }

  return 0;
}

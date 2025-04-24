#include <bits/stdc++.h>

/*================================================*\

  B+* tree.
  No support for duplicate mainkey.

  each node has max of `M` branches,
  for index node, `M - 1` keys,
  for leaf node, `M` keys.

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
        bstar_node *sib{};
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
  static constexpr std::size_t MIN_KEYS = (2 * (MAX_KEYS) + 2) / 3 - 1; // ceil

private:
  inline bool overflow_(const bstar_node *n) noexcept {
    return n->key_cnt > MAX_KEYS;
  }
  inline bool average_overflow_(const bstar_node *a,
                                const bstar_node *b) noexcept {
    return std::ceil((a->key_cnt + b->key_cnt + 1) / 2) > MAX_KEYS;
  }

  inline bool underflow_(const bstar_node *n) noexcept {
    return n == root ? false : (n->key_cnt < MIN_KEYS);
  }
  inline bool average_underflow_(const bstar_node *a,
                                 const bstar_node *b) noexcept {
    return std::floor((a->key_cnt + b->key_cnt) / 2) < MIN_KEYS;
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
    bstar_node *new_root{new bstar_node{.key_cnt = 0, .is_leaf = false}};
    bstar_node *node2{new bstar_node{.key_cnt = 0, .is_leaf = root->is_leaf}};
    if (root->is_leaf) {
      std::size_t need1{(root->key_cnt + 1) / 2};
      std::size_t need2{root->key_cnt / 2};
      std::memcpy(node2->key, root->key + need1, need2 * sizeof(KeyType));
      std::memcpy(node2->leaf.data_ptr, root->leaf.data_ptr + need1,
                  need2 * sizeof(ValType *));
      new_root->key[new_root->key_cnt++] = node2->key[0];
      root->leaf.sib = node2;
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
      root->leaf.sib = nullptr;
    } else {
      // key
      root->key[node1->key_cnt] = root->key[0];
      std::memmove(root->key, node1->key, node1->key_cnt * sizeof(KeyType));
      std::memmove(root->key + node1->key_cnt + 1, node2->key,
                   node2->key_cnt * sizeof(KeyType)); // ?
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
    // first fill node3
    bstar_node *node3{new bstar_node{.key_cnt = 0, .is_leaf = node1->is_leaf}};
    std::size_t total{node1->key_cnt + node2->key_cnt};

    if (!node1->is_leaf) {
      insert_key_in_parent(node2, node3, parent, idx2,
                           node2->key[node2->key_cnt - 1]);
      node3->idx.key_ptr[0] = node2->idx.key_ptr[node2->key_cnt];
      node2->key_cnt--;
      total--;
    }

    std::size_t need1{(total + 2) / 3};
    std::size_t need2{(total + 1) / 3};
    std::size_t need3{total / 3};
    // experiment

    // the need sum is node2.key_cnt
    KeyType new_key2{redistribute_keys_(node2, node3, node2->key_cnt - need3,
                                        need3, parent, idx2)};

    KeyType new_key1{
        redistribute_keys_(node1, node2, need1, need2, parent, idx1)};

    if (node1->is_leaf) {
      insert_key_in_parent(node2, node3, parent, idx2, new_key2);
    } else {
      modify_key_in_parent_(node2, node3, parent, idx2, new_key2);
    }

    modify_key_in_parent_(node1, node2, parent, idx1, new_key1);
    if (node1->is_leaf) {
      node3->leaf.sib = node2->leaf.sib;
      node2->leaf.sib = node3;
    }
  }

  void do_3_2_merge_(bstar_node *node1, bstar_node *node2, bstar_node *node3,
                     bstar_node *parent, std::size_t idx1, std::size_t idx2,
                     std::size_t idx3) {
    std::size_t total{node1->key_cnt + node2->key_cnt + node3->key_cnt};
    std::size_t need1{(total + 1) / 2};
    std::size_t need2{total / 2};

    KeyType new_key1{redistribute_keys_(node1, node2, need1,
                                        node1->key_cnt + node2->key_cnt - need1,
                                        parent, idx1)};

    KeyType new_key2{redistribute_keys_(node2, node3, need2, 0, parent, idx2)};

    modify_key_in_parent_(node1, node2, parent, idx1, new_key1);

    remove_key_in_parent_(node2, node3, parent, idx2);

    // patch
    if (node1->is_leaf) {
      parent->key[idx1] = node2->key[0];
    }

    if (node1->is_leaf) {
      // fix next
      node2->leaf.sib = node3->leaf.sib;
    } else {
      node2->key[node2->key_cnt] = new_key2;
      node2->idx.key_ptr[node2->key_cnt + 1] = node3->idx.key_ptr[0];
      node2->key_cnt++;
    }

    delete node3;
  }

  void do_equal_split_2_(bstar_node *node1, bstar_node *node2,
                         bstar_node *parent, std::size_t idx1) {
    std::size_t total{node1->key_cnt + node2->key_cnt};
    std::size_t need1{total / 2};
    std::size_t need2{total - need1};
    KeyType new_key =
        redistribute_keys_(node1, node2, need1, need2, parent, idx1); // BUG
    modify_key_in_parent_(node1, node2, parent, idx1, new_key);
  }

  bool pair_left(bstar_node *&node_l, bstar_node *&node_r, std::size_t &idx_l,
                 std::size_t &idx_r, bstar_node *parent) {
    if (idx_r > 0) {
      idx_l = idx_r - 1;
      node_l = parent->idx.key_ptr[idx_l];
      return true;
    }
    return false;
  }

  bool pair_right(bstar_node *&node_l, bstar_node *&node_r, std::size_t &idx_l,
                  std::size_t &idx_r, bstar_node *parent) {
    if (idx_l + 1 <= parent->key_cnt) {
      idx_r = idx_l + 1;
      node_r = parent->idx.key_ptr[idx_r];
      return true;
    }
    return false;
  }

  void fix_overflow_(bstar_node *node1, bstar_node *parent, std::size_t idx1) {
    std::size_t idx2{};
    bstar_node *node2{};

    if (pair_left(node2, node1, idx2, idx1, parent)) {
      std::swap(node1, node2);
      std::swap(idx1, idx2);
    } else {
      pair_right(node1, node2, idx1, idx2, parent);
    }

    if (!average_overflow_(node1, node2) && !average_underflow_(node1, node2)) {
      do_equal_split_2_(node1, node2, parent, idx1);
      return;
    }
    do_2_3_split_(node1, node2, parent, idx1, idx2);
  }

  void fix_underflow_(bstar_node *node1, bstar_node *parent, std::size_t idx1) {

    std::size_t idx2{};
    bstar_node *node2{};
    if (pair_left(node2, node1, idx2, idx1, parent)) {
      std::swap(node2, node1);
      std::swap(idx2, idx1);
    } else {
      pair_right(node1, node2, idx1, idx2, parent);
    }

    if (parent == root && parent->key_cnt == 1) {
      if (node1->key_cnt + node2->key_cnt < MAX_KEYS) {
        do_2_1_merge_root_();
      }
      return;
    }

    // 必须尝试 3 way 均分
    if (!average_overflow_(node1, node2) && !average_underflow_(node1, node2)) {
      do_equal_split_2_(node1, node2, parent, idx1);
      return;
    }

    std::size_t idx3{};
    bstar_node *node3{};
    if (pair_left(node3, node1, idx3, idx1, parent)) {
      std::swap(node3, node2);
      std::swap(node2, node1);
      std::swap(idx3, idx2);
      std::swap(idx2, idx1);
    } else {
      pair_right(node2, node3, idx2, idx3, parent);
    }
    do_3_2_merge_(node1, node2, node3, parent, idx1, idx2, idx3);
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
    if (overflow_(root)) {
      do_1_2_split_root_();
    }
    bstar_node *cur{root}, *next{};
    std::size_t next_from{};
    while (!cur->is_leaf) {
      next_from = cur->find_idx_ptr_index_(k);
      next = cur->idx.key_ptr[next_from];
      if (overflow_(next)) {
        fix_overflow_(next, cur, next_from);
        next_from = cur->find_idx_ptr_index_(k);
      }
      cur = cur->idx.key_ptr[next_from];
    }
    std::size_t check{cur->find_data_ptr_index_(k)};
    std::size_t idx{cur->find_idx_ptr_index_(k)};
    if (check != cur->key_cnt && cur->key[check] == k) {
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
    return true;
  }

  bool erase(const KeyType &k) {
    bstar_node *cur{root}, *next{};
    std::size_t next_from{};
    while (!cur->is_leaf) {
      next_from = cur->find_idx_ptr_index_(k);
      next = cur->idx.key_ptr[next_from];
      if (underflow_(next)) {
        fix_underflow_(next, cur, next_from);
        if (cur->is_leaf)
          break;
        next_from = cur->find_idx_ptr_index_(k);
      }
      cur = cur->idx.key_ptr[next_from];
    }
    std::size_t check{cur->find_data_ptr_index_(k)};
    if (check == cur->key_cnt || cur->key[check] != k) {
      return false;
    }
    if (check != cur->key_cnt) {
      std::memmove(cur->leaf.data_ptr + check, cur->leaf.data_ptr + check + 1,
                   (cur->key_cnt - (check + 1)) * sizeof(ValType *));
      std::memmove(cur->key + check, cur->key + check + 1,
                   (cur->key_cnt - (check + 1)) * sizeof(KeyType));
    }
    cur->key_cnt--;
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
      cur = cur->leaf.sib;
    }
    return values;
  }
};

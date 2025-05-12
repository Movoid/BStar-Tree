#pragma once
#include <bits/stdc++.h>

/*================================================*\

  B+*-tree,
  A B*-Tree based on B+-Tree,
  Support duplicated keys.

  Each node has up to `M` branches,
  For index and leaf node, up to `M - 1` keys.

\*================================================*/

/*================================================*\

  B epsilon tree,
  based on B+*-tree.

  Preemptive merge in buffer flushing
  is difficult to implement.
  Underflows are allowed in buffere flushing.

\*================================================*/

template <typename NodeType, typename Requires = void>
struct is_a_node : std::false_type {};

template <typename NodeType>
struct is_a_node<
    NodeType,
    std::void_t<decltype(std::declval<typename NodeType::key_type>() <
                         std::declval<typename NodeType::key_type>()),
                decltype(typename NodeType::key_type{}),
                decltype(typename NodeType::key_type{
                    std::declval<const typename NodeType::key_type &>()})>>
    : std::true_type {};

// mixins

template<typename Derived, typename KeyType, typename ValType, std::size_t M,
         typename Requires = std::enable_if_t<is_a_node<Derived>::value>>
struct b_base_node_mixin {

  using key_type = KeyType;
  using val_type = ValType;

  KeyType key[M - 1];
  union {
    struct {
      ValType *data_ptr[M - 1];
      Derived *sib;
    } leaf;
    struct {
      Derived *key_ptr[M];
    } idx;
  };
  std::size_t key_cnt;
  bool is_leaf;

  b_base_node_mixin() {
  }
  ~b_base_node_mixin() {
  }

  std::size_t find_idx_ptr_index_(const KeyType &k) noexcept {
    std::size_t l{0}, r{key_cnt};
    std::size_t mid{};
    // key[-1, l) <= val, key[r, key_cnt + 1) > val.
    while (r > l) {
      mid = (r - l) / 2 + l;
      if (!(k < key[mid])) {
        l = mid + 1;
      } else {
        r = mid;
      }
    }
    return r;
  }

  std::size_t find_data_ptr_index_(const KeyType &k) noexcept {
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

template<typename Derived, typename KeyType, typename ValType, std::size_t M>
struct b_star_node_mixin : b_base_node_mixin<Derived, KeyType, ValType, M> {};

template<typename Derived, typename KeyType, typename ValType, std::size_t M>
struct b_epsilon_node_mixin : b_star_node_mixin<Derived, KeyType, ValType, M> {
  using Base = b_star_node_mixin<Derived, KeyType, ValType, M>;
  struct lazy_entry {
    enum class lazy_type {
      INVALID = 0,
      INSERT = 1,
      REMOVE = 2,
    };
    lazy_type type{};
    KeyType target_key{};
    ValType target_val{};
  } *lazy_buf;
  std::size_t lazy_cnt;

  b_epsilon_node_mixin() {
    lazy_buf = new lazy_entry[BUFSIZ]{};
  }

  ~b_epsilon_node_mixin() {
    delete[] lazy_buf;
  }
};

// nodes

template<typename KeyType, typename ValType, std::size_t M>
struct b_star_node : b_star_node_mixin<b_star_node<KeyType, ValType, M>, KeyType, ValType, M> {};

template<typename KeyType, typename ValType, std::size_t M>
struct b_epsilon_node : b_epsilon_node_mixin<b_epsilon_node<KeyType, ValType, M>, KeyType, ValType, M> {};

// tree mixins

template<typename Derived, typename KeyType, typename ValType, std::size_t M, typename NodeType,
         typename Requires = std::enable_if_t<is_a_node<NodeType>::value && M >= 5>>
class b_base_tree_mixin {
private:
  Derived &derived() {
    return static_cast<Derived &>(*this);
  }
  const Derived &derived() const {
    return static_cast<const Derived &>(*this);
  }

protected:
  NodeType *root{};
  static constexpr std::size_t KEY_SLOTS = M - 1;
  static constexpr std::size_t MAX_KEYS = KEY_SLOTS;

  // this `MIN_KEYS` comes from:
  // `ceil( (MIN + MIN + MIN+2) + 1 ) / 2 <= MAX` .
  // 2-3 split MUST be successful if equal-split-3 is failed.
  static constexpr std::size_t MIN_KEYS = (2 * MAX_KEYS - 5) / 3;

  bool insert_overflow_(const NodeType *n) noexcept {
    return derived().insert_overflow_impl_();
  }

  bool average_2_overflow_(const NodeType *a, const NodeType *b) noexcept {
    return derived().average_2_overflow_impl_();
  }

  bool erase_underflow_(const NodeType *n) noexcept {
    return derived().erase_underflow_impl_();
  }

  bool average_2_underflow_(const NodeType *a, const NodeType *b) noexcept {
    return derived().average_2_underflow_impl_();
  }

  // parent != nullptr
  KeyType redistribute_keys_(NodeType *node1, NodeType *node2, std::size_t need1, std::size_t need2, NodeType *parent,
                             std::size_t idx1) noexcept {

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
        // if parent is nullptr or node1 is root?
        // this function will not check this. should be guaranteed to be valid.
        node2->key[key_move - 1] = parent->key[idx1];
        // adjust
        std::memmove(node2->idx.key_ptr + ptr_move, node2->idx.key_ptr, (node2->key_cnt + 1) * sizeof(NodeType *));
        // move
        std::memcpy(node2->idx.key_ptr, node1->idx.key_ptr + need1 + 1, ptr_move * sizeof(NodeType *));
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
        std::memmove(node2->key, node2->key + key_move,
                     (node2->key_cnt - key_move) * sizeof(KeyType)); // BUG
        // move
        std::memcpy(node1->idx.key_ptr + node1->key_cnt + 1, node2->idx.key_ptr,
                    ptr_move * sizeof(NodeType *)); // ?
        // adjust
        std::memmove(node2->idx.key_ptr, node2->idx.key_ptr + ptr_move,
                     (node2->key_cnt + 1 - ptr_move) * sizeof(NodeType *));
        node1->key_cnt = need1;
        node2->key_cnt = need2;
        return new_delim;
      }
    }

    // nothing should be changed.
    return parent->key[idx1];
  }

  void insert_key_in_parent(NodeType *node1, NodeType *node2, NodeType *parent, std::size_t idx1,
                            const KeyType &new_key) noexcept {
    std::memmove(parent->key + idx1 + 1, parent->key + idx1, (parent->key_cnt - idx1) * sizeof(KeyType));
    std::memmove(parent->idx.key_ptr + idx1 + 2, parent->idx.key_ptr + idx1 + 1,
                 (parent->key_cnt - idx1) * sizeof(NodeType *));
    parent->key[idx1] = new_key;
    parent->idx.key_ptr[idx1] = node1;
    parent->idx.key_ptr[idx1 + 1] = node2;
    parent->key_cnt++;
  }

  void modify_key_in_parent_(NodeType *node1, NodeType *node2, NodeType *parent, std::size_t idx1,
                             const KeyType &new_key) noexcept {
    if (node1->is_leaf) {
      parent->key[idx1] = node2->key[0];
    } else {
      parent->key[idx1] = new_key;
    }
    parent->idx.key_ptr[idx1] = node1;
    parent->idx.key_ptr[idx1 + 1] = node2;
  }

  void remove_key_in_parent_(NodeType *node1, NodeType *node2, NodeType *parent, std::size_t idx1) noexcept {
    std::memmove(parent->key + idx1, parent->key + idx1 + 1, (parent->key_cnt - idx1) * sizeof(KeyType));
    std::memmove(parent->idx.key_ptr + idx1 + 1, parent->idx.key_ptr + idx1 + 2,
                 (parent->key_cnt - idx1) * sizeof(NodeType *));
    parent->key_cnt--;
  }

  void do_1_2_split_root_() noexcept {

    NodeType *new_root{new NodeType{}};
    new_root->key_cnt = 0;
    new_root->is_leaf = false;
    NodeType *node2{new NodeType{}};
    node2->key_cnt = 0;
    node2->is_leaf = root->is_leaf;
    if (root->is_leaf) {
      std::size_t need1{(root->key_cnt + 1) / 2};
      std::size_t need2{root->key_cnt / 2};
      std::memcpy(node2->key, root->key + need1, need2 * sizeof(KeyType));
      std::memcpy(node2->leaf.data_ptr, root->leaf.data_ptr + need1, need2 * sizeof(ValType *));
      new_root->key[new_root->key_cnt++] = node2->key[0];
      root->leaf.sib = node2;
      root->key_cnt = need1;
      node2->key_cnt = need2;
    } else {
      std::size_t need1{(root->key_cnt) / 2};
      std::size_t need2{(root->key_cnt - 1) / 2};
      std::memcpy(node2->key, root->key + need1 + 1, need2 * sizeof(KeyType));
      std::memcpy(node2->idx.key_ptr, root->idx.key_ptr + need1 + 1, (need2 + 1) * sizeof(NodeType *));
      new_root->key[new_root->key_cnt++] = root->key[need1];
      root->key_cnt = need1;
      node2->key_cnt = need2;
    }
    new_root->idx.key_ptr[0] = root;
    new_root->idx.key_ptr[1] = node2;
    root = new_root;
  }

  void do_2_1_merge_root_() noexcept {
    NodeType *node1{root->idx.key_ptr[0]};
    NodeType *node2{root->idx.key_ptr[1]};
    if (node1->is_leaf) {
      // key
      std::memmove(root->key, node1->key, node1->key_cnt * sizeof(KeyType));
      std::memmove(root->key + node1->key_cnt, node2->key, node2->key_cnt * sizeof(KeyType));
      // ptr
      std::memmove(root->leaf.data_ptr, node1->leaf.data_ptr, node1->key_cnt * sizeof(ValType *));
      std::memmove(root->leaf.data_ptr + node1->key_cnt, node2->leaf.data_ptr, node2->key_cnt * sizeof(ValType *));
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
      std::memmove(root->idx.key_ptr, node1->idx.key_ptr, (node1->key_cnt + 1) * sizeof(NodeType *));
      std::memmove(root->idx.key_ptr + node1->key_cnt + 1, node2->idx.key_ptr,
                   (node2->key_cnt + 1) * sizeof(NodeType *));
      root->key_cnt = node1->key_cnt + node2->key_cnt + 1;
    }
    // delete
    delete node1;
    delete node2;
    return;
  }

  void do_2_equal_split_(NodeType *node1, NodeType *node2, NodeType *parent, std::size_t idx1) noexcept {
    std::size_t total{node1->key_cnt + node2->key_cnt};
    std::size_t need1{(total + 1) / 2};
    std::size_t need2{total / 2};

    KeyType new_key{redistribute_keys_(node1, node2, need1, need2, parent, idx1)};
    modify_key_in_parent_(node1, node2, parent, idx1, new_key);
  }

  bool pair_left(NodeType *&node_l, NodeType *&node_r, std::size_t &idx_l, std::size_t &idx_r,
                 NodeType *parent) noexcept {
    if (idx_r > 0) {
      idx_l = idx_r - 1;
      node_l = parent->idx.key_ptr[idx_l];
      return true;
    }
    return false;
  }

  bool pair_right(NodeType *&node_l, NodeType *&node_r, std::size_t &idx_l, std::size_t &idx_r,
                  NodeType *parent) noexcept {
    if (idx_l + 1 <= parent->key_cnt) {
      idx_r = idx_l + 1;
      node_r = parent->idx.key_ptr[idx_r];
      return true;
    }
    return false;
  }

  void fix_overflow_(NodeType *node1, NodeType *parent, std::size_t idx1) noexcept {
    return derived().fix_overflow_impl_();
  }

  void fix_underflow_(NodeType *node1, NodeType *parent, std::size_t idx1) noexcept {
    return derived().fix_underflow_impl_();
  }

  void delete_all_nodes_(bool del_root) {
    if (root) {
      std::vector<NodeType *> decon{};
      if (!root->is_leaf) {
        for (std::size_t i = 0; i <= root->key_cnt; i++) {
          decon.emplace_back(root->idx.key_ptr[i]);
        }
      }
      while (!decon.empty()) {
        NodeType *cur{decon.back()};
        decon.pop_back();
        if (!cur->is_leaf) {
          for (std::size_t i = 0; i <= cur->key_cnt; i++) {
            decon.emplace_back(cur->idx.key_ptr[i]);
          }
        }
        delete cur;
      }
      if (del_root) {
        delete root; // and root
      }
    }
  }

public:
  b_base_tree_mixin() {
    root = new NodeType{};
    root->key_cnt = 0;
    root->is_leaf = true;
  }
  ~b_base_tree_mixin() {
    delete_all_nodes_(true);
  }

  b_base_tree_mixin(const b_base_tree_mixin &obj) = delete;
  b_base_tree_mixin(b_base_tree_mixin &&obj) noexcept {
    delete_all_nodes_(true);
    root = obj.root;
    obj.root = nullptr;
  }
  b_base_tree_mixin &operator=(const b_base_tree_mixin &obj) = delete;
  b_base_tree_mixin &operator=(b_base_tree_mixin &&obj) noexcept {
    delete_all_nodes_(true);
    root = obj.root;
    obj.root = nullptr;
    return *this;
  }
  void clear() {
    delete_all_nodes_(false);
  }

  bool insert(const KeyType &k, ValType *v) noexcept {
    return derived().insert_impl();
  }

  bool erase(const KeyType &k) noexcept {
    return derived().erase_impl();
  }

  std::vector<ValType *> find(const KeyType &k) const {
    NodeType *cur{root};
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
      cur = cur->leaf.sib;
    }
    return values;
  }

  // [low, high)
  std::vector<ValType *> find_range(const KeyType &low, const KeyType &high) const {
    NodeType *cur{root};
    while (cur && !cur->is_leaf) {
      cur = cur->idx.key_ptr[cur->find_data_ptr_index_(low)];
    }
    std::vector<ValType *> values{};
    while (cur) {
      std::size_t beg{cur->find_data_ptr_index_(low)}, end{cur->find_data_ptr_index_(high)};
      if (end == 0) break;
      for (std::size_t i = beg; i < end; i++) {
        values.emplace_back(cur->leaf.data_ptr[i]);
      }
      cur = cur->leaf.sib;
    }
    return values;
  }
};

template<typename Derived, typename KeyType, typename ValType, typename NodeType, std::size_t M>
class b_star_tree_mixin : public b_base_tree_mixin<Derived, KeyType, ValType, M, NodeType> {
private:
protected:
  bool average_3_overflow_(const NodeType *a, const NodeType *b, const NodeType *c) noexcept {
    return ((a->key_cnt + b->key_cnt + c->key_cnt) / 3) >= MAX_KEYS;
  }

public:
};
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

    std::size_t find_idx_ptr_index(const KeyType &k) {
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

    std::size_t find_data_ptr_index(const KeyType &k) {
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

private:
  void split_node_(bstar_node *node, bstar_node *fa) {

    std::cout << "split_node_" << std::endl;

    std::size_t div1{}, div2{};

    // split a node
    std::size_t node_ptr_cnt = node->key_cnt + (node->is_leaf ? 0 : 1);
    std::size_t divlen = node_ptr_cnt / 3;
    div1 = divlen;
    div2 = divlen * 2 + (node_ptr_cnt % 3 / 2);

    bstar_node *node1 = new bstar_node{.key_cnt = div2 - div1, .is_leaf = node->is_leaf};
    bstar_node *node2 = new bstar_node{.key_cnt = node->key_cnt - div2, .is_leaf = node->is_leaf};

    std::memcpy(node1->key, node->key + div1, (div2 - div1) * sizeof(KeyType));
    std::memcpy(node2->key, node->key + div2, (node2->key_cnt) * sizeof(KeyType));

    if (node->is_leaf) {
      std::memcpy(node1->leaf.data_ptr, node->leaf.data_ptr + div1, (node1->key_cnt) * sizeof(ValType *));
      std::memcpy(node2->leaf.data_ptr, node->leaf.data_ptr + div2, (node2->key_cnt) * sizeof(ValType *));
    } else {
      std::memcpy(node1->idx.key_ptr + 1, node->idx.key_ptr + div1 + 1, (node1->key_cnt) * sizeof(bstar_node *));
      std::memcpy(node2->idx.key_ptr + 1, node->idx.key_ptr + div2 + 1, (node2->key_cnt) * sizeof(bstar_node *));
    }

    node->key_cnt = div1;

    // push up keys
    if (!fa) {
      bstar_node *new_root = new bstar_node{.key_cnt = 2, .is_leaf = false};
      new_root->key[0] = node1->key[0];
      new_root->key[1] = node2->key[0];
      new_root->idx.key_ptr[0] = node;
      new_root->idx.key_ptr[1] = node1;
      new_root->idx.key_ptr[2] = node2;
      root = new_root;
    } else {
      // not root
      std::size_t f_idx1, f_idx2;
      f_idx1 = fa->find_idx_ptr_index(node1->key[0]);
      f_idx2 = fa->find_idx_ptr_index(node2->key[0]);
      std::memmove(fa->key + f_idx2 + 2, fa->key + f_idx2, (fa->key_cnt - f_idx2) * sizeof(KeyType));
      std::memmove(fa->idx.key_ptr + f_idx2 + 3, fa->idx.key_ptr + f_idx2 + 1,
                   (fa->key_cnt - f_idx2) * sizeof(KeyType *));
      fa->key[f_idx2 + 1] = node2->key[0];
      fa->idx.key_ptr[f_idx2 + 2] = node2;
      std::memmove(fa->key + f_idx1 + 1, fa->key + f_idx1, (f_idx2 - f_idx1) * sizeof(KeyType));
      std::memmove(fa->idx.key_ptr + 2, fa->idx.key_ptr + f_idx1 + 1, (f_idx2 - f_idx1) * sizeof(bstar_node *));
      fa->key[f_idx1] = node1->key[0];
      fa->idx.key_ptr[f_idx1 + 1] = node1;
    }

    // update nodes link
    if (node->is_leaf) {
      node2->leaf.next = node->leaf.next;
      node1->leaf.next = node2;
      node->leaf.next = node1;
    }
  }

public:
  bstar_tree() {
    root = new bstar_node{.key_cnt = 0, .is_leaf = true};
  }
  ~bstar_tree() {
    // todo
  }

  bool insert(const KeyType &k, ValType *v) {
    bstar_node *cur{root}, *prev{};
    while (true) {
      bool is_split{};
      if ((!cur->is_leaf && cur->key_cnt >= M - 2) || (cur->is_leaf && cur->key_cnt >= M - 1)) {
        split_node_(cur, prev);
        is_split = true;
        if (!prev) prev = root;
      }
      if (cur->is_leaf) break;
      if (is_split) {
        cur = prev->idx.key_ptr[prev->find_idx_ptr_index(k)];
      } else {
        prev = cur;
        cur = cur->idx.key_ptr[cur->find_idx_ptr_index(k)];
      }
    }
    std::size_t idx{cur->find_idx_ptr_index(k)};
    // enable duplicate mainkey support.
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
    while (!cur->is_leaf) {
      cur = cur->idx.key_ptr[cur->find_data_ptr_index(k)];
    }
    std::vector<ValType *> values{};

    while (cur) {
      std::size_t beg{cur->find_data_ptr_index(k)}, end{cur->find_idx_ptr_index(k)};
      if (beg == end) break;
      for (std::size_t i = beg; i < end; i++) {
        values.emplace_back(cur->leaf.data_ptr[i]);
      }
      cur = cur->leaf.next;
    }

    return values;
  }

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

  std::vector<int *> data{tree.find(2)};
  cout << data.size() << endl;

  for (int i = 0; i < data.size(); i++) {
    std::cout << *data[i] << std::endl;
  }

  return 0;
}
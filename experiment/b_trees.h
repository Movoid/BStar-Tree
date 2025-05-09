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

template <typename KeyType, typename ValType, std::size_t M, typename Derived,
          typename Requires = std::enable_if_t<is_a_node<Derived>::value>>
struct b_base_mixin {

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

template <typename KeyType, typename ValType, std::size_t M, typename Derived>
struct b_star_mixin : b_base_mixin<KeyType, ValType, M,
                                   b_star_mixin<KeyType, ValType, M, Derived>> {
};

template <typename KeyType, typename ValType, std::size_t M, typename Derived>
struct b_epsilon_mixin
    : b_star_mixin<KeyType, ValType, M,
                   b_epsilon_mixin<KeyType, ValType, M, Derived>> {};
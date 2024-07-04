#ifndef ART_HPP
#define ART_HPP

#include "childIt.hpp"
#include "innerNode.hpp"
#include "leafNode.hpp"
#include "node.hpp"
#include "node4.hpp"
#include "treeIt.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <numeric>
#include <stack>
#include <vector>

namespace art {

template <typename T> class Art {
public:
  ~Art();

  /**
   * Finds the value associated with the given key.
   *
   * @param key - The key to find.
   * @return the value associated with the key or a default constructed value.
   */
  T get(const char *key) const;

  /**
   * Associates the given key with the given value.
   * If another value is already associated with the given key,
   * since the method consumer is the resource owner.
   *
   * @param key - The key to associate with the value.
   * @param value - The value to be associated with the key.
   * @return a nullptr if no other value is associated with they or the
   * previously associated value.
   */
  T set(const char *key, T value);

  /**
   * Deletes the given key and returns it's associated value.
   * The associated value is returned,
   * since the method consumer is the resource owner.
   * If no value is associated with the given key, nullptr is returned.
   *
   * @param key - The key to delete.
   * @return the values assciated with they key or a nullptr otherwise.
   */
  T del(const char *key);

  /**
   * Forward iterator that traverses the tree in lexicographic order.
   */
  treeIt<T> begin();

  /**
   * Forward iterator that traverses the tree in lexicographic order starting
   * from the provided key.
   */
  treeIt<T> begin(const char *key);

  /**
   * Iterator to the end of the lexicographic order.
   */
  treeIt<T> end();

private:
  Node<T> *root = nullptr;
};

template <typename T> Art<T>::~Art<T>() {
  if (root == nullptr)
    return;

  std::stack<Node<T> *, std::vector<Node<T>>> nodeStack;
  nodeStack.push(root);
  Node<T> *currentNode;
  innerNode<T> *currInnerNode;
  childIt<T> it, itEnd;
  while (!nodeStack.empty()) {
    currentNode = nodeStack.top();
    nodeStack.pop();
    if (!currentNode->isLeaf()) {
      currInnerNode = static_cast<innerNode<T> *>(currentNode);
      for (it = currInnerNode->begin(), itEnd = currInnerNode->end();
           it != itEnd; ++it) {
        nodeStack.push(*currInnerNode->findChild(*it));
      }
    }
    if (currentNode->prefix_ != nullptr)
      delete[] currentNode->prefix_;
  }
  delete currentNode;
}

template <typename T> T Art<T>::get(const char *key) const {

  Node<T> *current = root;
  Node<T> **child;
  int depth = 0, keyLen = std::strlen(key) + 1;
  while (current != nullptr) {
    if (current->prefixLen_ != current->checkPrefix(key, keyLen))
      // Prefix mismatch
      return T{};

    if (current->prefixLen_ == keyLen - depth) {
      // Exact Match
      return current->isLeaf() ? static_cast<LeafNode<T> *>(current)->value
                               : T{};
    }

    child = static_cast<innerNode<T> *>(current)->findChild(
        key[depth + current->prefixLen_]);
    depth += (current->prefixLen_ + 1);
    current = child != nullptr ? *child : nullptr;
  }
  return T{};
}

template <typename T> T Art<T>::set(const char *key, T value) {
  int keyLen = std::strlen(key) + 1, depth = 0, prefixMatchLen;
  if (root == nullptr) {
    root = new LeafNode<T>(value);
    root->prefix_ = new char[keyLen];
    std::copy(key, keyLen + 1, root->prefix_);
    root->prefixLen_ = keyLen;
    return T{};
  }

  Node<T> **currentNode = &root;
  Node<T> **child;
  innerNode<T> currentInner;
  char childPartialKey;
  bool isPrefixMatch;

  while (true) {
    /* number of bytes of the current node's prefix that match the key */
    prefixMatchLen = (**currentNode).checkPrefix(key + depth, keyLen - depth);
    /* true if the current node's prefix matches with a part of the key */
    isPrefixMatch = (std::min<int>((**currentNode).prefixLen_,
                                   keyLen - depth)) == prefixMatchLen;

    if (isPrefixMatch && (**currentNode).prefixLen_ == keyLen - depth) {
      /* exact match:
       * => "replace"
       * => replace value of current node.
       * => return old value to caller to handle.
       *        _                             _
       *        |                             |
       *       (aa)                          (aa)
       *    a /    \ b     +[aaaaa,v3]    a /    \ b
       *     /      \      ==========>     /      \
       * *(aa)->v1  ()->v2             *(aa)->v3  ()->v2
       *
       */

      // CurrentNode must be a leaf
      auto currentLeaf = static_cast<LeafNode<T> *>(*currentNode);
      T oldValue = currentLeaf->value;
      currentLeaf->value = value;
      return oldValue;
    }

    if (!isPrefixMatch) {
      /* prefix mismatch:
       * => new parent node with common prefix and no associated value.
       * => new node with value to insert.
       * => current and new node become children of new parent node.
       *
       *        |                        |
       *      *(aa)                    +(a)->Ø
       *    a /    \ b     +[ab,v3]  a /   \ b
       *     /      \      =======>   /     \
       *  (aa)->v1  ()->v2          *()->Ø +()->v3
       *                          a /   \ b
       *                           /     \
       *                        (aa)->v1 ()->v2
       *                        /|\      /|\
       */
      auto newParent = Node4<T>();
      newParent.prefix_ = new char[prefixMatchLen];
      std::copy((**currentNode)->prefix_,
                (**currentNode)->prefix_ + prefixMatchLen, newParent.prefix_);
      newParent.prefixLen_ = prefixMatchLen;
      newParent.setChild((**currentNode).prefix_[prefixMatchLen], *currentNode);

      auto oldPrefix = (**currentNode).prefix_;
      auto oldPrefixLength = (**currentNode).prefixLen_;
      (**currentNode).prefix = new char[oldPrefixLength - prefixMatchLen - 1];
      (**currentNode).prefixLen_ = oldPrefixLength - prefixMatchLen - 1;

      std::copy(oldPrefix + prefixMatchLen + 1, oldPrefix + oldPrefixLength,
                (**currentNode).prefix_);
      delete oldPrefix;

      auto newNode = LeafNode<T>(value);
      newNode.prefix_ = new char[keyLen - depth - prefixMatchLen - 1];
      std::copy(key + depth + prefixMatchLen + 1, key + keyLen,
                newNode.prefix_);
      newNode.prefixLen_ = keyLen - depth - prefixMatchLen - 1;
      newParent.setChild(key[depth + prefixMatchLen + 1], &newNode);

      *currentNode = newParent;
      return T{};
    }

    currentInner = reinterpret_cast<innerNode<T> **>(currentInner);
    childPartialKey = key[depth + (**currentNode).prefixLen_];
    child = (**currentInner).findChild(childPartialKey);

    if (child == nullptr) {
      /*
       * no child associated with the next partial key.
       * => create new node with value to insert.
       * => new node becomes current node's child.
       *
       *      *(aa)->Ø              *(aa)->Ø
       *    a /        +[aab,v2]  a /    \ b
       *     /         ========>   /      \
       *   (a)->v1               (a)->v1 +()->v2
       */
      if ((**currentNode).isFull())
        *currentInner = (**currentInner).grow();

      LeafNode<T> newNode = LeafNode<T>(value);
      newNode.prefix_ =
          new char[keyLen - depth - (**currentNode).prefixLen_ - 1];
      newNode.prefixLen_ = keyLen - depth - (**currentNode).prefixLen_ - 1;
      std::copy(key + depth + (**currentNode).prefixLen_ + 1, key + keyLen,
                newNode.prefix_);
      (**currentNode).setChild(childPartialKey, newNode);
      return T{};
    }

    /* propagate down and repeat:
     *
     *     *(aa)->Ø                   (aa)->Ø
     *   a /    \ b    +[aaba,v3]  a /    \ b     repeat
     *    /      \     =========>   /      \     ========>  ...
     *  (a)->v1  ()->v2           (a)->v1 *()->v2
     */

    depth += (**currentNode).prefixLen_ + 1;
    currentNode = child;
  }
}

template <class T> T art<T>::del(const char *key) {
  int depth = 0, key_len = std::strlen(key) + 1;

  if (root_ == nullptr) {
    return T{};
  }

  /* pointer to parent, current and child node */
  node<T> **cur = &root_;
  inner_node<T> **par = nullptr;

  /* partial key of current and child node */
  char cur_partial_key = 0;

  while (cur != nullptr) {
    if ((**cur).prefix_len_ !=
        (**cur).check_prefix(key + depth, key_len - depth)) {
      /* prefix mismatch => key doesn't exist */

      return T{};
    }

    if (key_len == depth + (**cur).prefix_len_) {
      /* exact match */
      if (!(**cur).is_leaf()) {
        return T{};
      }
      auto value = static_cast<leaf_node<T> *>(*cur)->value_;
      auto n_siblings = par != nullptr ? (**par).n_children() - 1 : 0;

      if (n_siblings == 0) {
        /*
         * => must be root node
         * => delete root node
         *
         *     |                 |
         *    (aa)->v1          (aa)->v1
         *     | a     -[aaaaa]
         *     |       =======>
         *   *(aa)->v2
         */

        if ((**cur).prefix_ != nullptr) {
          delete[] (**cur).prefix_;
        }
        delete (*cur);
        *cur = nullptr;

      } else if (n_siblings == 1) {
        /* => delete leaf node
         * => replace parent with sibling
         *
         *        |a                         |a
         *        |                          |
         *       (aa)        -"aaaaabaa"     |
         *    a /    \ b     ==========>    /
         *     /      \                    /
         *  (aa)->v1 *()->v2             (aaaaa)->v1
         *  /|\                            /|\
         */

        /* find sibling */
        auto sibling_partial_key = (**par).next_partial_key(0);
        if (sibling_partial_key == cur_partial_key) {
          sibling_partial_key = (**par).next_partial_key(cur_partial_key + 1);
        }
        auto sibling = *(**par).find_child(sibling_partial_key);

        auto old_prefix = sibling->prefix_;
        auto old_prefix_len = sibling->prefix_len_;

        sibling->prefix_ = new char[(**par).prefix_len_ + 1 + old_prefix_len];
        sibling->prefix_len_ = (**par).prefix_len_ + 1 + old_prefix_len;
        std::copy((**par).prefix_, (**par).prefix_ + (**par).prefix_len_,
                  sibling->prefix_);
        sibling->prefix_[(**par).prefix_len_] = sibling_partial_key;
        std::copy(old_prefix, old_prefix + old_prefix_len,
                  sibling->prefix_ + (**par).prefix_len_ + 1);
        if (old_prefix != nullptr) {
          delete[] old_prefix;
        }
        if ((**cur).prefix_ != nullptr) {
          delete[] (**cur).prefix_;
        }
        delete (*cur);
        if ((**par).prefix_ != nullptr) {
          delete[] (**par).prefix_;
        }
        delete (*par);

        /* this looks crazy, but I know what I'm doing */
        *par = static_cast<inner_node<T> *>(sibling);

      } else /* if (n_siblings > 1) */ {
        /* => delete leaf node
         *
         *        |a                         |a
         *        |                          |
         *       (aa)        -"aaaaabaa"    (aa)
         *    a / |  \ b     ==========> a / |
         *     /  |   \                   /  |
         *           *()->v1
         */

        if ((**cur).prefix_ != nullptr) {
          delete[] (**cur).prefix_;
        }
        delete (*cur);
        (**par).del_child(cur_partial_key);
        if ((**par).is_underfull()) {
          *par = (**par).shrink();
        }
      }

      return value;
    }

    /* propagate down and repeat */
    cur_partial_key = key[depth + (**cur).prefix_len_];
    depth += (**cur).prefix_len_ + 1;
    par = reinterpret_cast<inner_node<T> **>(cur);
    cur = (**par).find_child(cur_partial_key);
  }
  return T{};
}

template <class T> tree_it<T> art<T>::begin() {
  return tree_it<T>::min(this->root_);
}

template <class T> tree_it<T> art<T>::begin(const char *key) {
  return tree_it<T>::greater_equal(this->root_, key);
}

template <class T> tree_it<T> art<T>::end() { return tree_it<T>(); }

} // namespace art

#endif // ART_HPP

#ifndef ART_NODE_16_HPP
#define ART_NODE_16_HPP

#include "innerNode.hpp"
#include "node.hpp"
#include <algorithm>
#include <stdexcept>

namespace art {
template <class T> class Node4;
template <class T> class Node48;

template <class T> class Node16 : public innerNode<T> {
  friend class Node4<T>;
  friend class Node48<T>;

public:
  Node<T> **findChild(char partial_key) override;
  void setChild(char partialKey, Node<T> *child) override;
  Node<T> *delChild(char partialKey) override;
  innerNode<T> *grow() override;
  innerNode<T> *shrink() override;
  bool isFull() const override;
  bool isUnderfull() const override;

  char nextPartialKey(char partialKey) const override;

  char prevPartialKey(char partialKey) const override;

  int nChildren() const override;

private:
  uint8_t nChildren_ = 0;
  char keys_[16];
  Node<T> *children_[16];
};

template <typename T> Node<T> **Node16<T>::findChild(char partialKey) {
#if defined(__i386__) || defined(__amd64__)
  int bitfield =
      _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(partial_key),
                                       _mm_loadu_si128((__m128i *)keys_))) &
      ((1 << n_children_) - 1);
  return (bool)bitfield ? &children_[__builtin_ctz(bitfield)] : nullptr;
#else

  // Perform binary search on array to find child;
  int lo, mid, hi;
  lo = 0;
  hi = nChildren_;
  while (lo < hi) {
    mid = (lo + hi) / 2;
    if (partialKey < keys_[mid])
      hi = mid;
    else if (partialKey > keys_[mid])
      lo = mid;
    else
      return children_[mid];
  }
  return nullptr;

#endif
}
// [a, b, c, d, e, f, g, h, x, y, z, 0, 0, 0]
template <typename T>
void Node16<T>::setChild(char partialKey, Node<T> *child) {
  int childI;
  for (int i = this->nChildren_ - 1;; --i) {
    if (i >= 0 && partialKey < this->keys_[i]) {
      this->keys_[i + 1] = this->keys_[i];
      this->children_[i + 1] = this->children_[i];
    } else {
      childI = i + 1;
      break;
    }
  }
  this->keys_[childI] = partialKey;
  this->children_[childI] = child;
  ++nChildren_;
}

template <typename T> Node<T> *Node16<T>::delChild(char partialKey) {
  Node<T> childToDelete = nullptr;
  for (int i = 0; i < nChildren_; ++i) {
    if (childToDelete == nullptr && keys_[i] == partialKey) {
      childToDelete = children_[i];
    }

    // [a, c, d, 0]
    if (childToDelete != nullptr) {
      keys_[i] = i < nChildren_ - 1 ? keys_[i + 1] : 0;
      children_[i] = i < nChildren_ - 1 ? children_[i + 1] : nullptr;
    }
  }

  if (childToDelete != nullptr)
    --nChildren_;

  return childToDelete;
}

template <typename T> innerNode<T> *Node16<T>::grow() {
  auto newNode = new Node48<T>();
  newNode->prefix_ = this->prefix_;
  newNode->prefixLen_ = this->prefixLen_;
  newNode->nChildren_ = this->nChildren_;
  std::copy(this->keys_, this->keys_ + this->nChildren_, newNode->keys_);
  std::copy(this->children_, this->children_ + this->nChildren_,
            newNode->children_);

  delete this;
  return newNode;
}

template <typename T> bool Node16<T>::isFull() const {
  return nChildren_ == 16;
}

template <typename T> bool Node16<T>::isUnderfull() const {
  return nChildren_ == 4;
}

template <typename T> char Node16<T>::nextPartialKey(char partialKey) const {
  for (int i = 0; i < nChildren_; ++i) {
    if (keys_[i] >= partialKey) {
      return keys_[i];
    }
  }
  throw std::out_of_range(
      "There are no successors to the provided partial key");
}

template <typename T> char Node16<T>::prevPartialKey(char partialKey) const {
  for (int i = nChildren_ - 1; i >= 0; --i) {
    if (keys_[i] <= partialKey) {
      return keys_[i];
    }
  }
  throw std::out_of_range(
      "There are no predecessors to the provided partial key");
}

template <typename T> int Node16<T>::nChildren() const { return nChildren_; }
} // namespace art

#endif // !ART_NODE_16_HPP

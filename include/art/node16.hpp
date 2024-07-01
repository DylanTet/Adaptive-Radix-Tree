#ifndef ART_NODE_16_HPP
#define ART_NODE_16_HPP

#include "innerNode.hpp"
#include "node.hpp"

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

template <typename T> innerNode<T> *Node16<T>::grow() {}
} // namespace art

#endif // !ART_NODE_16_HPP

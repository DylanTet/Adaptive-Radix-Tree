#ifndef ART_NODE_4_HPP
#define ART_NODE_4_HPP

#include "innerNode.hpp"
#include "node.hpp"
#include <_types/_uint8_t.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace art {
template <class T> class node0;
template <class T> class Node16;

template <class T> class Node4 : public innerNode<T> {
  friend class node0<T>;
  friend class Node16<T>;

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
  char keys_[4];
  Node<T> *children_[4];
};

template <class T> Node<T> **Node4<T>::findChild(char partialKey) {
  for (int i = 0; i < nChildren_; ++i) {
    if (keys_[i] == partialKey) {
      return &children_[i];
    }
  }
  return nullptr;
}

template <class T> void Node4<T>::setChild(char partialKey, Node<T> *child) {
  /* determine index for child */
  int childIndex;
  for (childIndex = 0;
       childIndex < nChildren_ && partialKey >= keys_[childIndex];
       ++childIndex) {
  }

  std::memmove(keys_ + childIndex + 1, keys_ + childIndex,
               nChildren_ - childIndex);
  std::memmove(children_ + childIndex + 1, children_ + childIndex,
               (nChildren_ - childIndex) * sizeof(void *));

  keys_[childIndex] = partialKey;
  children_[childIndex] = child;
  ++nChildren_;
}

template <class T> Node<T> *Node4<T>::delChild(char partialKey) {
  Node<T> *child_to_delete = nullptr;
  for (int i = 0; i < nChildren_; ++i) {
    if (child_to_delete == nullptr && partialKey == keys_[i]) {
      child_to_delete = children_[i];
    }
    if (child_to_delete != nullptr) {
      /* move existing sibling to the left */
      keys_[i] = i < nChildren_ - 1 ? keys_[i + 1] : 0;
      children_[i] = i < nChildren_ - 1 ? children_[i + 1] : nullptr;
    }
  }
  if (child_to_delete != nullptr) {
    --nChildren_;
  }
  return child_to_delete;
}

template <typename T> innerNode<T> *Node4<T>::grow() {
  auto newNode = new Node16<T>();
  newNode->prefix_ = this->prefix_;
  newNode->prefixLen_ = this->prefixLen_;
  newNode->nChildren_ = this->nChildren_;
  std::copy(this->keys_, this->keys_ + this->nChildren_, newNode->keys_);
  std::copy(this->children_, this->children_ + this->nChildren_,
            newNode->children_);
  delete this;
  return newNode;
}

template <typename T> innerNode<T> *Node4<T>::shrink() {
  throw std::runtime_error("Cant shrink a Node4!");
}

template <typename T> bool Node4<T>::isFull() const { return nChildren_ == 4; }
template <typename T> bool Node4<T>::isUnderfull() const { return false; }

template <typename T> char Node4<T>::nextPartialKey(char partialKey) const {
  for (int i = 0; i < nChildren_; ++i) {
    if (keys_[i] >= partialKey) {
      return keys_[i];
    }
  }
  throw std::out_of_range("provided partial key doesnt have a successor");
}

template <typename T> char Node4<T>::prevPartialKey(char partialKey) const {
  for (int i = nChildren_ - 1; i >= 0; --i) {
    if (keys_[i] <= partialKey) {
      return keys_[i];
    }
  }
  throw std::out_of_range("provided partial key doesnt have a predecessor");
}

template <typename T> int Node4<T>::nChildren() const {
  return this->nChildren_;
}

} // namespace art

#endif // !ART_NODE_4_HPP

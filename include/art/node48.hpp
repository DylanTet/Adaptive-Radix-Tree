#ifndef ART_NODE_48_HPP
#define ART_NODE_48_HPP

#include "innerNode.hpp"
#include "node.hpp"
#include <_types/_uint8_t.h>
#include <algorithm>
#include <stdexcept>

namespace art {

template <typename T> class Node16;
template <typename T> class Node256;

template <typename T> class Node48 : public innerNode<T> {
  friend class Node16<T>;
  friend class Node256<T>;

public:
  Node48();

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
  static const char EMPTY;

  uint8_t nChildren_ = 0;
  char indexes_[256];
  Node<T> *children_[48];
};

template <typename T> Node48<T>::Node48() {
  std::fill(this->indexes_, this->indexes_ + 256, Node48::EMPTY);
  std::fill(this->indexes_, this->indexes_ + 48, Node48::EMPTY);
}

template <typename T> Node<T> **Node48<T>::findChild(char partialKey) {
  uint8_t index = indexes_[128 + partialKey];
  return Node48<T>::EMPTY != index ? &children_[index] : nullptr;
}

template <typename T>
void Node48<T>::setChild(char partialKey, Node<T> *child) {
  for (int i = 0; i < 48; ++i) {
    if (children_[i] == nullptr) {
      indexes_[128 + partialKey] = static_cast<uint8_t>(i);
      children_[i] = child;
      break;
    }
  }
  ++nChildren_;
}

template <typename T> Node<T> *Node48<T>::delChild(char partialKey) {
  Node<T> *childToDelete = nullptr;

  // TODO: DOES THIS ACTUALLY NEED TO BE UNSIGNED??
  char index = indexes_[128 + partialKey];
  if (indexes_[index] != Node48::EMPTY) {
    childToDelete = children_[index];
    indexes_[128 + partialKey] = Node48::EMPTY;
    children_[index] = nullptr;
    --nChildren_;
  }

  return childToDelete;
}

template <typename T> innerNode<T> *Node48<T>::grow() {
  auto newNode = new Node256<T>();
  newNode->prefix_ = this->prefix_;
  newNode->prefixLen_ = this->prefixLen_;
  uint8_t index;
  for (int partialKey = -128; partialKey < 127; ++partialKey) {
    index = indexes_[partialKey + 128];
    if (index != Node48::EMPTY) {
      newNode->setChild(partialKey, children_[index]);
    }
  }
  delete this;
  return newNode;
}

template <typename T> bool Node48<T>::isFull() const {
  return nChildren_ == 48;
}

template <typename T> bool Node48<T>::isUnderfull() const {
  return nChildren_ == 16;
}

template <typename T> const char Node48<T>::EMPTY = 48;

template <typename T> char Node48<T>::nextPartialKey(char partialKey) const {
  while (true) {
    if (indexes_[128 + partialKey] != Node48<T>::EMPTY)
      return partialKey;

    if (partialKey == 127)
      throw std::out_of_range("Provided key doesnt have a successor");

    ++partialKey;
  }
}

template <typename T> char Node48<T>::prevPartialKey(char partialKey) const {
  while (true) {
    if (indexes_[128 + partialKey] != Node48<T>::EMPTY)
      return partialKey;

    if (partialKey == -128)
      throw std::out_of_range("Provided key doesnt have a predecessor");

    --partialKey;
  }
}

template <typename T> int Node48<T>::nChildren() const { return nChildren_; }
} // namespace art

#endif // ART_NODE_48_HPP

#ifndef ART_NODE_256_HPP
#define ART_NODE_256_HPP

#include "innerNode.hpp"
#include "node.hpp"
#include <stdexcept>
#include <vector>

namespace art {

template <typename T> class Node48;

template <typename T> class Node256 : public innerNode<T> {

  friend class Node48<T>;

public:
  Node256();

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
  std::array<Node<T> *, 256> children_;
};

template <typename T> Node256<T>::Node256() { children_.fill(nullptr); }

template <typename T> Node<T> **Node256<T>::findChild(char partialKey) {
  return children_[128 + partialKey] != nullptr ? &children_[128 + partialKey]
                                                : nullptr;
}

template <typename T>
void Node256<T>::setChild(char partialKey, Node<T> *child) {
  children_[128 + partialKey] = child;
  ++nChildren_;
}

template <typename T> Node<T> *Node256<T>::delChild(char partialKey) {
  Node<T> *nodeToDelete = children_[128 + partialKey];
  if (nodeToDelete != nullptr) {
    --nChildren_;
    children_[128 + partialKey] = nullptr;
  }
  return nodeToDelete;
}

template <typename T> innerNode<T> *Node256<T>::grow() {
  throw std::runtime_error("Node of size 256 cant grow bigger");
}

template <typename T> innerNode<T> *Node256<T>::shrink() {
  auto smallerNode = Node48<T>();
  smallerNode->prefix_ = this->prefix_;
  smallerNode->prefixLen_ = this->prefixLen_;
  for (int partialKey = 0; partialKey < 256; ++partialKey) {
    if (children_[128 + partialKey] != nullptr)
      smallerNode->setChild(partialKey, children_[128 + partialKey]);
  }

  delete this;
  return smallerNode;
}

template <typename T> bool Node256<T>::isFull() const {
  return nChildren_ == 256;
}

template <typename T> bool Node256<T>::isUnderfull() const {
  return nChildren_ == 48;
}

template <typename T> char Node256<T>::nextPartialKey(char partialKey) const {
  while (true) {
    if (children_[128 + partialKey] != nullptr)
      return partialKey;

    if (partialKey == 127)
      throw std::out_of_range("Provided key doesnt have a successor");

    partialKey++;
  }
}

template <typename T> char Node256<T>::prevPartialKey(char partialKey) const {
  while (true) {
    if (children_[128 + partialKey] != nullptr)
      return partialKey;

    if (partialKey == -128)
      throw std::out_of_range("Provided key doesnt have a predecessor");

    --partialKey;
  }
}

template <typename T> int Node256<T>::nChildren() const { return nChildren_; }
} // namespace art

#endif // !ART_NODE_256_HPP

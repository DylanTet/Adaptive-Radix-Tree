#ifndef ART_INNER_NODE_HPP
#define ART_INNER_NODE_HPP

#include "childIt.hpp"
#include "leafNode.hpp"
#include "node.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace art {
template <class T> class innerNode<T> : public Node<T> {
public:
  innerNode() = default;
  innerNode(const innerNode<T> &other) = default;
  innerNode(innerNode<T> &&other) noexcept = default;
  innerNode<T> &operator=(const innerNode<T> &other) = default;
  innerNode<T> &operator=(innerNode<T> &&other) = default;
  virtual ~innerNode() = default;

  bool isLeaf() const override;

  /**
   * Finds and returns the child node identified by the given partial key.
   *
   * @param partialKey - The partial key associated with the child.
   * @return Child node identified by the given partial key or
   * a null pointer of no child node is associated with the partial key.
   */
  virtual Node<T> **findChild(char partialKey) = 0;

  /**
   * Adds the given node to the node's children.
   * No bounds checking is done.
   * If a child already exists under the given partial key, the child
   * is overwritten without deleting it.
   *
   * @pre Node should not be full.
   * @param partialKey - The partial key associated with the child.
   * @param child - The child node.
   */
  virtual void setChild(char partialKey, Node<T> *child) = 0;

  /**
   * Deletes the child associated with the given partial key.
   *
   * @param partial_key - The partial key associated with the child.
   */
  virtual Node<T> *delChild(char partialKey) = 0;

  /**
   * Creates and returns a new node with bigger children capacity.
   * The current node gets deleted.
   *
   * @return node with bigger capacity
   */
  virtual innerNode<T> *grow() = 0;

  /**
   * Creates and returns a new node with lesser children capacity.
   * The current node gets deleted.
   *
   * @pre node must be undefull
   * @return node with lesser capacity
   */
  virtual innerNode<T> *shrink() = 0;

  /**
   * Determines if the node is full, i.e. can carry no more child nodes.
   */
  virtual bool isFull() const = 0;

  /**
   * Determines if the node is underfull, i.e. carries less child nodes than
   * intended.
   */
  virtual bool isUnderfull() const = 0;

  virtual int nChildren() const = 0;

  virtual char next_partial_key(char partial_key) const = 0;

  virtual char prev_partial_key(char partial_key) const = 0;

  /**
   * Iterator on the first child node.
   *
   * @return Iterator on the first child node.
   */
  childIt<T> begin();
  std::reverse_iterator<childIt<T>> rbegin();

  /**
   * Iterator on after the last child node.
   *
   * @return Iterator on after the last child node.
   */
  childIt<T> end();
  std::reverse_iterator<childIt<T>> rend();
};

template <class T> bool innerNode<T>::isLeaf() const { return false; }

template <class T> childIt<T> innerNode<T>::begin() { return childIt<T>(this); }

template <class T> std::reverse_iterator<childIt<T>> innerNode<T>::rbegin() {
  return std::reverse_iterator<childIt<T>>(end());
}

template <class T> childIt<T> innerNode<T>::end() {
  return childIt<T>(this, nChildren());
}

template <class T> std::reverse_iterator<childIt<T>> innerNode<T>::rend() {
  return std::reverse_iterator<childIt<T>>(begin());
}

} // namespace art

#endif // ART_INNER_NODE_HPP

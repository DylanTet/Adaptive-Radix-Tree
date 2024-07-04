#ifndef ART_LEAF_NODE_HPP
#define ART_LEAF_NODE_HPP

#include "node.hpp"

namespace art {

template <class T> class LeafNode : public Node<T> {
public:
  explicit LeafNode(T value);
  bool isLeaf() const override;

  T value;
};

template <class T> LeafNode<T>::LeafNode(T value) : value(value){};

template <class T> bool LeafNode<T>::isLeaf() const { return true; }

} // namespace art

#endif // !ART_LEAF_NODE_HPP

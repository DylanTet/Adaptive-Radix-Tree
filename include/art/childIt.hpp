#ifndef ART_CHILDIT_HPP
#define ART_CHILDIT_HPP

#include "node.hpp"
#include <iterator>
#include <stdexcept>

namespace art {
template <class T> class innerNode;

template <class T> class childIt {
public:
  childIt() = default;
  explicit childIt(innerNode<T> *n);
  childIt(innerNode<T> *n, int relativeIndex);
  childIt(const childIt<T> &other) = default;
  childIt(childIt<T> &&other) noexcept = default;
  childIt<T> &operator=(const childIt<T> &other) = default;
  childIt<T> &operator=(childIt<T> &&other) noexcept = default;

  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = const char;
  using difference_type = int;
  using pointer = value_type *;
  using reference = value_type &;

  reference operator*() const;
  pointer operator->() const;
  childIt &operator++();
  childIt operator++(int);
  childIt &operator--();
  childIt operator--(int);
  bool operator==(const childIt &rhs) const;
  bool operator!=(const childIt &rhs) const;
  bool operator<(const childIt &rhs) const;
  bool operator>(const childIt &rhs) const;
  bool operator<=(const childIt &rhs) const;
  bool operator>=(const childIt &rhs) const;

  char getPartialKeys() const;
  Node<T> *getChildNode() const;

private:
  innerNode<T> *node = nullptr;
  char curPartialKey = -128;
  int relativeIndex = 0;
};

template <class T> childIt<T>::childIt(innerNode<T> *n) : childIt<T>(n, 0) {}

template <class T>
childIt<T>::childIt(innerNode<T> *n, int relativeIndex)
    : node(n), curPartialKey(0), relativeIndex(relativeIndex) {
  if (relativeIndex < 0) {
    return;
  }

  if (relativeIndex >= node->nChildren()) {
    return;
  }

  if (relativeIndex == node->nChildren() - 1) {
    curPartialKey = node->prevPartialKey(127);
    return;
  }

  curPartialKey = node->nextPartialKey(-128);
  for (int i = 0; i < relativeIndex; ++i)
    curPartialKey = node->nextPartialKey(curPartialKey + 1);
}

template <class T>
typename childIt<T>::reference childIt<T>::operator*() const {
  if (relativeIndex < 0 || relativeIndex >= node->nChildren()) {
    throw std::out_of_range("Child iterator out of range");
  }

  return &curPartialKey;
}

template <class T> typename childIt<T>::pointer childIt<T>::operator->() const {
  if (relativeIndex < 0 || relativeIndex >= node->n_children()) {
    throw std::out_of_range("child iterator is out of range");
  }

  return &curPartialKey;
}

template <class T> childIt<T> &childIt<T>::operator++() {
  ++relativeIndex;
  if (relativeIndex < 0) {
    return *this;
  } else if (relativeIndex == 0) {
    curPartialKey = node->nextPartialKey(-128);
  } else if (relativeIndex < node->n_children()) {
    curPartialKey = node->nextPartialKey(curPartialKey + 1);
  }
  return *this;
}

template <class T> childIt<T> childIt<T>::operator++(int) {
  auto old = *this;
  operator++();
  return old;
}

template <class T> childIt<T> &childIt<T>::operator--() {
  --relativeIndex;
  if (relativeIndex > node->nChildren() - 1) {
    return *this;
  } else if (relativeIndex == node->nChildren() - 1) {
    curPartialKey = node->prevPartialKey(127);
  } else if (relativeIndex >= 0) {
    curPartialKey = node->prevPartialKey(curPartialKey - 1);
  }
  return *this;
}

template <class T> childIt<T> childIt<T>::operator--(int) {
  auto old = *this;
  operator--();
  return old;
}

template <class T> bool childIt<T>::operator==(const childIt<T> &rhs) const {
  return node == rhs.node && relativeIndex == rhs.relativeIndex;
}

template <class T> bool childIt<T>::operator<(const childIt<T> &rhs) const {
  return node == rhs.node && relativeIndex < rhs.relativeIndex;
}

template <class T> bool childIt<T>::operator!=(const childIt<T> &rhs) const {
  return !((*this) == rhs);
}

template <class T> bool childIt<T>::operator>=(const childIt<T> &rhs) const {
  return !((*this) < rhs);
}

template <class T> bool childIt<T>::operator<=(const childIt<T> &rhs) const {
  return (rhs >= (*this));
}

template <class T> bool childIt<T>::operator>(const childIt<T> &rhs) const {
  return (rhs < (*this));
}

template <class T> char childIt<T>::getPartialKeys() const {
  return curPartialKey;
}

template <class T> Node<T> *childIt<T>::getChildNode() const {
  assert(0 <= relativeIndex && relativeIndex < node->nChildren());
  return *node->findChild(curPartialKey);
}

} // namespace art

#endif // !ART_CHILDIT_HPP

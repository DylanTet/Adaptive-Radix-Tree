#ifndef ART_NODE_HPP
#define ART_NODE_HPP

#include <_types/_uint16_t.h>
#include <algorithm>
namespace art {
template <class T> class Node {
public:
  virtual ~Node() = default;

  Node() = default;
  Node(const Node<T> &other) = default;

  virtual bool isLeaf() const = 0;

  /**
   * Determines the number of matching bytes between the node's prefix and the
   * key.
   *
   * Given a node with prefix: "abbbd", a key "abbbccc",
   * check_prefix returns 4, since byte 4 of the prefix ('d') does not
   * match byte 4 of the key ('c').
   *
   * key:     "abbbccc"
   * prefix:  "abbbd"
   *           ^^^^*
   * index:    01234
   */
  int checkPrefix(const char *key, int keyLen) const;

  char *prefix_ = nullptr;
  uint16_t prefixLen_ = 0;
};

template <class T> int Node<T>::checkPrefix(const char *key, int keyLen) const {
  return std::mismatch(prefix_, prefix_ + prefixLen_, key).second - key;
}
} // namespace art

#endif // !ART_NODE_HPP

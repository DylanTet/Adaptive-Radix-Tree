#ifndef ART_treeIt_HPP
#define ART_treeIt_HPP

#include "childIt.hpp"
#include "node.hpp"
#include <string>
namespace art {

template <typename T> class node;
template <typename T> class inner_node;
template <typename T> class leaf_node;

template <typename T> class treeIt {
public:
  struct step {
    Node<T> *childNode_;
    int depth_;
    char *key_;
    childIt<T> childIt_;
    childIt<T> childItEnd_;

    step();
    step(int depth, childIt<T> c_it, childIt<T> c_it_end);
    step(node<T> *node, int depth, const char *key, childIt<T> c_it,
         childIt<T> c_it_end);
    step(const step &other);
    step(step &&other);
    ~step();

    step &operator=(const step &other);
    step &operator=(step &&other) noexcept;
    step &operator++();
    step operator++(int);
  };

  treeIt();
  explicit treeIt(node<T> *root, std::vector<step> traversal_stack);

  static treeIt<T> min(node<T> *root);
  static treeIt<T> greater_equal(node<T> *root, const char *key);

  using iterator_category = std::forward_iterator_tag;
  using value_type = T;
  using difference_type = int;
  using pointer = value_type *;

  value_type operator*();
  pointer operator->();
  treeIt<T> &operator++();
  treeIt<T> operator++(int);
  bool operator==(const treeIt<T> &rhs) const;
  bool operator!=(const treeIt<T> &rhs) const;

  template <typename OutputIt> void key(OutputIt key) const;
  int get_key_len() const;
  const std::string key() const;

private:
  step &getStep();
  const step &getStep() const;
  node<T> *getNode() const;
  const char *getKey() const;
  int getDepth() const;

  void seek_leaf();

  node<T> *root_;
  std::vector<step> traversalStack_;
};

template <class T>
treeIt<T>::step::step() : step(nullptr, 0, nullptr, {}, {}) {}

template <class T>
treeIt<T>::step::step(int depth, childIt<T> c_it, childIt<T> c_it_end)
    : childNode_(c_it != c_it_end ? c_it.get_child_node() : nullptr),
      depth_(depth), key_(depth ? new char[depth] : nullptr), childIt_(c_it),
      childItEnd_(c_it_end) {}

template <class T>
treeIt<T>::step::step(node<T> *node, int depth, const char *key,
                      childIt<T> c_it, childIt<T> c_it_end)
    : childNode_(node), depth_(depth), key_(depth ? new char[depth] : nullptr),
      childIt_(c_it), childItEnd_(c_it_end) {
  std::copy_n(key, depth, key_);
}

template <class T> typename treeIt<T>::step &treeIt<T>::step::operator++() {
  assert(childIt_ != childItEnd_);
  ++childIt_;
  childNode_ = childIt_ != childItEnd_ ? childIt_.get_child_node() : nullptr;
  key_[depth_ - 1] =
      childIt_ != childItEnd_ ? childIt_.get_partial_key() : '\0';
  return *this;
}

template <class T> typename treeIt<T>::step treeIt<T>::step::operator++(int) {
  auto old = *this;
  operator++();
  return old;
}

template <class T>
treeIt<T>::step::step(const treeIt<T>::step &other)
    : step(other.childNode_, other.depth_, other.key_, other.childIt_,
           other.childItEnd_) {}

template <class T>
treeIt<T>::step::step(treeIt<T>::step &&other)
    : childNode_(other.childNode_), depth_(other.depth_), key_(other.key_),
      childIt_(other.childIt_), childItEnd_(other.childIt_) {
  other.childNode_ = nullptr;
  other.depth_ = 0;
  other.key_ = nullptr;
  other.childIt_ = {};
  other.childItEnd_ = {};
}

template <class T> treeIt<T>::step::~step() { delete[] key_; }

template <class T>
typename treeIt<T>::step &
treeIt<T>::step::operator=(const treeIt<T>::step &other) {
  if (this != &other) {
    node<T> *node = other.childNode_;
    int depth = other.depth_;
    char *key = depth ? new char[depth] : nullptr;
    std::copy_n(other.key_, other.depth_, key);
    childIt<T> c_it = other.childIt_;
    childIt<T> c_it_end = other.childItEnd_;

    childNode_ = node;
    depth_ = depth;
    delete[] key_;
    key_ = key;
    childIt_ = c_it;
    childItEnd_ = c_it_end;
  }
  return *this;
}

template <class T>
typename treeIt<T>::step &
treeIt<T>::step::operator=(treeIt<T>::step &&other) noexcept {
  if (this != &other) {
    childNode_ = other.child_node_;
    other.childNode_ = nullptr;

    depth_ = other.depth_;
    other.depth_ = 0;

    delete[] key_;
    key_ = other.key_;
    other.key_ = nullptr;

    childIt_ = other.childIt_;
    other.childIt_ = {};

    childItEnd_ = other.childItEnd_;
    other.childItEnd_ = {};
  }
  return *this;
}

template <class T> treeIt<T>::treeIt() {}

template <class T>
treeIt<T>::treeIt(node<T> *root, std::vector<step> traversal_stack)
    : root_(root), traversalStack_(traversal_stack) {
  seek_leaf();
}

template <class T> treeIt<T> treeIt<T>::min(node<T> *root) {
  return treeIt<T>::greater_equal(root, "");
}

template <class T>
treeIt<T> treeIt<T>::greater_equal(node<T> *root, const char *key) {
  assert(root != nullptr);

  int key_len = std::strlen(key);
  std::vector<treeIt<T>::step> traversal_stack;

  // sentinel child iterator for root
  traversal_stack.push_back({root, 0, nullptr, {nullptr, -2}, {nullptr, -1}});

  while (true) {
    treeIt<T>::step &cur_step = traversal_stack.back();
    node<T> *cur_node = cur_step.childNode_;
    int cur_depth = cur_step.depth_;

    int prefix_match_len = std::min<int>(
        cur_node->check_prefix(key + cur_depth, key_len - cur_depth),
        key_len - cur_depth);
    // if search key "equals" the prefix
    if (key_len == cur_depth + prefix_match_len) {
      return treeIt<T>(root, traversal_stack);
    }
    // if search key is "greater than" the prefix
    if (prefix_match_len < cur_node->prefix_len_ &&
        key[cur_depth + prefix_match_len] >
            cur_node->prefix_[prefix_match_len]) {
      ++cur_step;
      return treeIt<T>(root, traversal_stack);
    }
    if (cur_node->is_leaf()) {
      continue;
    }
    // seek subtree where search key is "lesser than or equal" the subtree
    // partial key
    inner_node<T> *cur_inner_node = static_cast<inner_node<T> *>(cur_node);
    childIt<T> c_it = cur_inner_node->begin();
    childIt<T> c_it_end = cur_inner_node->end();
    // TODO more efficient with specialized node search method?
    for (; c_it != c_it_end; ++c_it) {
      if (key[cur_depth + cur_node->prefix_len_] <= c_it.get_partial_key()) {
        break;
      }
    }
    int depth = cur_depth + cur_node->prefix_len_ + 1;
    treeIt<T>::step child(depth, c_it, c_it_end);
    /* compute child key: cur_key + cur_node->prefix_ + child_partial_key */
    std::copy_n(cur_step.key_, cur_depth, child.key_);
    std::copy_n(cur_node->prefix_, cur_node->prefix_len_,
                child.key_ + cur_depth);
    child.key_[cur_depth + cur_node->prefix_len_] = c_it.get_partial_key();
    traversal_stack.push_back(child);
  }
}

template <class T> typename treeIt<T>::value_type treeIt<T>::operator*() {
  assert(getNode()->is_leaf());
  return static_cast<leaf_node<T> *>(getNode())->value_;
}

template <class T> typename treeIt<T>::pointer treeIt<T>::operator->() {
  assert(getNode()->is_leaf());
  return &static_cast<leaf_node<T> *>(getNode())->value_;
}

template <class T> treeIt<T> &treeIt<T>::operator++() {
  assert(getNode()->is_leaf());
  ++getStep();
  seek_leaf();
  return *this;
}

template <class T> treeIt<T> treeIt<T>::operator++(int) {
  auto old = *this;
  operator++();
  return old;
}

template <class T> bool treeIt<T>::operator==(const treeIt<T> &rhs) const {
  if (traversalStack_.empty() && rhs.traversalStack_.empty()) {
    /* both are empty */
    return true;
  }
  if (traversalStack_.empty() || rhs.traversalStack_.empty()) {
    /* one is empty */
    return false;
  }
  return getNode() == rhs.getNode();
}

template <class T> bool treeIt<T>::operator!=(const treeIt<T> &rhs) const {
  return !(*this == rhs);
}

template <class T>
template <class OutputIt>
void treeIt<T>::key(OutputIt key) const {
  std::copy_n(getKey(), getDepth(), key);
  std::copy_n(getNode()->prefix_, getNode()->prefix_len_, key + getDepth());
}

template <class T> int treeIt<T>::get_key_len() const {
  return getDepth() + getNode()->prefix_len_;
}

template <class T> const std::string treeIt<T>::key() const {
  std::string str(getDepth() + getNode()->prefix_len_ - 1, 0);
  key(str.begin());
  return str;
}

template <class T> void treeIt<T>::seek_leaf() {
  /* traverse up until a node on the right is found or stack gets empty */
  for (; getStep().childIt_ == getStep().childIt_; ++getStep()) {
    traversalStack_.pop_back();
    if (getStep().childNode_ == root_) { // root guard
      traversalStack_.pop_back();
      assert(traversalStack_.empty());
      return;
    }
  }

  /* find leftmost leaf node */
  while (!getNode()->is_leaf()) {
    inner_node<T> *cur_inner_node = static_cast<inner_node<T> *>(getNode());
    int depth = getDepth() + getNode()->prefix_len_ + 1;
    childIt<T> c_it = cur_inner_node->begin();
    childIt<T> c_it_end = cur_inner_node->end();
    treeIt<T>::step child(depth, c_it, c_it_end);
    /* compute child key: cur_key + cur_node->prefix_ + child_partial_key */
    std::copy_n(getKey(), getDepth(), child.key_);
    std::copy_n(getNode()->prefix_, getNode()->prefix_len_,
                child.key_ + getDepth());
    child.key_[getDepth() + getNode()->prefix_len_] = c_it.get_partial_key();
    traversalStack_.push_back(child);
  }
}

template <class T> node<T> *treeIt<T>::getNode() const {
  return getStep().childNode_;
}

template <class T> int treeIt<T>::getDepth() const { return getStep().depth_; }

template <class T> const char *treeIt<T>::getKey() const {
  return getStep().key_;
}

template <class T> typename treeIt<T>::step &treeIt<T>::getStep() {
  assert(!traversalStack_.empty());
  return traversalStack_.back();
}

template <class T> const typename treeIt<T>::step &treeIt<T>::getStep() const {
  assert(!traversalStack_.empty());
  return traversalStack_.back();
}

} // namespace art

#endif // !ART_treeIt_HPP

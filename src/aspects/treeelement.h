#pragma once

#include <memory>
#include <vector>
#include <set>
#include <algorithm>
#include "abstractraiiguard.h"
#include "common.h"
#include <QtGlobal>

namespace omm
{

template<typename T>
class TreeElement
{
public:
  TreeElement() = default;
  virtual ~TreeElement() = default;
  explicit TreeElement(const TreeElement& other);
  bool is_root() const;
  T& parent() const;
  T& adopt(std::unique_ptr<T> adoptee, const size_t pos);
  T& adopt(std::unique_ptr<T> adoptee);
  std::unique_ptr<T> repudiate(T& repudiatee);
  std::vector<T*> children() const;
  T& child(size_t i) const;
  size_t n_children() const;
  bool is_ancestor_of(const T& subject) const;
  void reset_parent(T& new_parent);
  std::set<T*> all_descendants() const;
  size_t position() const;
  virtual std::unique_ptr<AbstractRAIIGuard> acquire_set_parent_guard() { return nullptr; }

  static void remove_internal_children(std::set<T*>& items);
  static T* lowest_common_ancestor(T *a, T *b);
  static const T* lowest_common_ancestor(const T *a, const T *b);
  static std::vector<T*> sort(const std::set<T*>& items);

protected:
  virtual void on_children_changed(std::set<const void*>) {}

private:
  T* m_parent = nullptr;
  std::vector<std::unique_ptr<T>> m_children;
  T& get() { return static_cast<T&>(*this); }
  const T& get() const { return static_cast<const T&>(*this); }
  friend class ChildrenChangedNotificationBlocker;
  bool m_block_children_change_notifications = false;
public:
  class ChildrenChangedNotificationBlocker
  {
  public:
    ChildrenChangedNotificationBlocker(TreeElement& tree_element);
    ~ChildrenChangedNotificationBlocker();
  private:
    TreeElement& m_tree_element;
  };
};

template<typename T> bool tree_lt(const T* a, const T* b)
{
  static const auto get_descendant = [](const T* root, const T* direction) {
    // return the direct-descendant of root that is a (direct or indirect) ancestor of direction
    assert(root->is_ancestor_of(*direction) && root != direction);
    const T* candidate = direction;
    while (&candidate->parent() != root) {
      assert(!candidate->is_root());
      candidate = &candidate->parent();
    }
    return candidate;
  };

  if  (a == b) {
    return false;
  }

  const auto* lca = TreeElement<T>::lowest_common_ancestor(a, b);
  assert(lca->is_ancestor_of(*a) && lca->is_ancestor_of(*b));

  if (lca == a) {
    return true;
  } else if (lca == b) {
    return false;
  } else {
    const auto* aa = get_descendant(lca, a);
    const auto* ba = get_descendant(lca, b);
    for (const T* c : lca->children()) {
      if (aa == c) {
        return true;
      } else if (ba == c) {
        return false;
      }
    }
  }

  Q_UNREACHABLE();
}

template<typename T> bool tree_gt(const T* a, const T* b)
{
  return a != b && tree_lt(b, a);
}

template<typename T>
std::ostream& print_tree(std::ostream& ostream, const T* item, int indentation = 0)
{
  ostream << std::string(indentation, ' ') << item << "\n";
  if (item != nullptr) {
    const auto children = item->children();
    const auto is_pre_leaf = std::none_of(children.begin(), children.end(), [](auto* c) {
      return c->children().size() > 0;
    });
    if (is_pre_leaf) {
      ::operator<<(ostream, children);
    } else {
      for (auto&& child : item->children()) {
        print_tree(ostream, child, indentation + 2);
      }
    }
  }
  return ostream;
}

/**
 * @brief The TreeTestItem class is a dummy class to be used in TreeElement for testing.
 *  It is not used in the actual application but in the unit tests.
 *  It must, however, be defined here in order to instanciate TreeElement<TreeTestItem>
 *  in treeelement.cpp.
 */
class TreeTestItem : public TreeElement<TreeTestItem>
{
public:
  std::string name;
};

std::ostream& operator<<(std::ostream& ostream, const TreeTestItem* item);
std::ostream& operator<<(std::ostream& ostream, const TreeTestItem& item);

}  // namespace omm

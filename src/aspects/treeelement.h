#pragma once

#include "common.h"
#include <QtGlobal>
#include <algorithm>
#include <memory>
#include <set>
#include <vector>

namespace omm
{
template<typename T> class TreeElement
{
public:
  TreeElement() = default;
  virtual ~TreeElement() = default;
  explicit TreeElement(const TreeElement& other);
  TreeElement(TreeElement&&) = delete;
  TreeElement& operator=(TreeElement&&) = delete;
  TreeElement& operator=(const TreeElement& other) = delete;
  [[nodiscard]] bool is_root() const;
  [[nodiscard]] T& tree_parent() const;
  virtual T& adopt(std::unique_ptr<T> object, std::size_t pos);
  T& adopt(std::unique_ptr<T> object);
  virtual std::unique_ptr<T> repudiate(T& object);
  [[nodiscard]] std::vector<T*> tree_children() const;
  [[nodiscard]] T& tree_child(std::size_t i) const;
  [[nodiscard]] std::size_t n_children() const;
  [[nodiscard]] bool is_ancestor_of(const T& subject) const;
  void reset_parent(T& new_parent);
  [[nodiscard]] std::set<T*> all_descendants() const;
  [[nodiscard]] std::size_t position() const;

  static void remove_internal_children(std::set<T*>& items);
  static T* lowest_common_ancestor(T* a, T* b);
  static const T* lowest_common_ancestor(const T* a, const T* b);
  static std::deque<T*> sort(const std::set<T*>& items);

protected:
  virtual void on_child_added(T& child)
  {
    Q_UNUSED(child);
  }
  virtual void on_child_removed(T& child)
  {
    Q_UNUSED(child);
  }

private:
  T* m_parent = nullptr;
  std::vector<std::unique_ptr<T>> m_children;
  T& get()
  {
    return static_cast<T&>(*this);
  }
  [[nodiscard]] const T& get() const
  {
    return static_cast<const T&>(*this);
  }
};

template<typename T> bool tree_lt(const T* a, const T* b)
{
  static const auto get_descendant = [](const T* root, const T* direction) {
    // return the direct-descendant of root that is a (direct or indirect) ancestor of direction
    assert(root->is_ancestor_of(*direction) && root != direction);
    const T* candidate = direction;
    while (&candidate->tree_parent() != root) {
      assert(!candidate->is_root());
      candidate = &candidate->tree_parent();
    }
    return candidate;
  };

  if (a == b) {
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
    for (const T* c : lca->tree_children()) {
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
void print_tree(QString& buffer, const T* item, int indentation = 0)
{
  const auto sitem = item == nullptr ? QString{"[null]"} : item->to_string();
  buffer += QString(" ").repeated(indentation).toStdString() + sitem + "\n";
  if (item != nullptr) {
    const auto children = item->tree_children();
    const auto is_pre_leaf = std::none_of(children.begin(), children.end(), [](auto* c) {
      return c->tree_children().size() > 0;
    });
    if (is_pre_leaf) {
      buffer += children;
    } else {
      for (auto&& child : item->tree_children()) {
        print_tree(buffer, child, indentation + 2);
      }
    }
  }
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
  QString name;
};

}  // namespace omm

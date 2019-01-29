#pragma once

#include <functional>
#include <cassert>
#include <memory>
#include <algorithm>
#include <vector>
#include <glog/logging.h>
#include "aspects/treeelement.h"
#include "scene/structure.h"
#include "scene/contextes_fwd.h"
#include "scene/list.h"
#include "scene/tree.h"

namespace omm
{

template<typename T, template<typename...> class Wrapper>
class ListContext
{
public:
  using item_type = T;
  ListContext(T& subject, const T* predecessor)
    : subject(subject), predecessor(predecessor) { }

  Wrapper<T> subject;
  T& get_subject() const { return subject; }

  /**
   * @brief the predecessing sibling of `subject`
   * @details may be `nullptr` if `subject` is the first child of `parent`.
   * @details the parent of `predecessor` must be `parent`.
   */
  const T* predecessor;
  virtual bool is_sane() const { return true; }
  static constexpr bool is_tree_context = false;
};

template<typename T, template<typename...> class Wrapper>
class TreeContext : public ListContext<T, Wrapper>
{
  static_assert( std::is_base_of<TreeElement<T>, T>::value, "T must be derived from TreeElement.");
public:
  TreeContext(T& subject, T& parent, const T* predecessor)
    : ListContext<T, Wrapper>(subject, predecessor), parent(parent) { }

  TreeContext(T& subject, const T* predecessor)
    : ListContext<T, Wrapper>(subject, predecessor), parent(subject.parent()) { }

  /**
   * @brief the parent of `subject`
   */
  std::reference_wrapper<T> parent;

  bool is_sane() const override
  {
    return this->predecessor == nullptr || &this->predecessor->parent() == &this->parent.get();
  }

  static constexpr bool is_tree_context = true;
};

template<typename T, template<typename, template<typename...> class > class ContextT>
class MoveContext : public ContextT<T, std::reference_wrapper>
{
public:
  using ContextT<T, std::reference_wrapper>::ContextT;
  /**
   * @brief returns whether the context does produce illogical states
   *  in order to return true, all of the following conditions must be true:
   *    - subject must not be root
   *    - subject and predecessor must be different objects
   *    - subject and parent must be different objects
   */
  virtual bool is_valid() const = 0;

  /**
   * @brief returns whether the context does produce illogical states or same state
   *  in order to return true, all of the following conditions must be true:
   *    - context must be valid (@code is_valid())
   *    - predecessor and predecessor of subject must be different objects
   */
  virtual bool is_strictly_valid(const Structure<T>&) const = 0;

protected:
  bool same_predecessor(const Structure<T>& structure) const
  {
    return this->predecessor == structure.predecessor(this->get_subject());
  }

  bool subject_is_predecessor() const { return this->predecessor == &this->get_subject(); }
};

template<typename T>
class TreeMoveContext : public MoveContext<T, TreeContext>
{
public:
  using MoveContext<T, TreeContext>::MoveContext;
  bool is_valid() const override
  {
    return !is_root() && !moves_into_itself() && !this->subject_is_predecessor();
  }

  bool is_strictly_valid(const Structure<T>& structure) const override
  {
    return is_valid() && !moves_before_itself(structure);
  }

private:
  bool is_root() const { return this->get_subject().is_root(); }
  bool moves_into_itself() const
  {
    // subject cannot become its own parent (illogical)
    return &this->parent.get() == &this->get_subject();
  }

  bool moves_before_itself(const Structure<T>& structure) const
  {
    const bool parent_does_not_change = &this->parent.get()
                                     == structure.predecessor(this->get_subject());

    // the `parent_does_not_change` test is only required if `predecessor == nullptr`.
    return parent_does_not_change && this->same_predecessor(structure);
  }
};

template<typename T>
class ListMoveContext : public MoveContext<T, ListContext>
{
public:
  using MoveContext<T, ListContext>::MoveContext;
  bool is_valid() const override
  {
    return !this->subject_is_predecessor();
  }

  bool is_strictly_valid(const Structure<T>& structure) const override
  {
    return is_valid() && !this->same_predecessor(structure);
  }

private:
  bool is_root() const { return this->get_subject().is_root(); }
};

template<typename ContextT>
void topological_context_sort(std::vector<ContextT>& ts)
{
  // Stupid because I guess it's O(n^2). It's possible to do it in O(n).
  const auto move_if = [](auto begin, auto end, const auto& predicate)
  {
    const auto it = std::find_if(begin, end, predicate);
    if (it != end) {
      std::iter_swap(it, std::prev(end));
      return std::prev(end);
    } else {
      return it;
    }
  };

  const auto begin_ts = ts.begin();
  auto end_ts = ts.end();

  while (begin_ts != end_ts) {
    end_ts = move_if(begin_ts, end_ts, [&begin_ts, &end_ts](const auto& t) {
      return end_ts == std::find_if(begin_ts, end_ts, [&t](const auto& s) {
        return t.predecessor == &s.subject.get();
      });
    });

    while (true) {
      const auto current = move_if(begin_ts, end_ts, [&end_ts](const auto& t) {
        return &end_ts->subject.get() == t.predecessor;
      });
      if (current == end_ts) {
        break;
      } else {
        end_ts = current;
      }
    }
  }
}

namespace
{

template<typename StructureT>
const auto* last_sibling(const StructureT& s)
{
  const auto get_siblings = [&s]() {
    if constexpr (StructureT::is_tree) {
      return s.root().children();
    } else {
      return s.ordered_items();
    }
  };
  const auto siblings = get_siblings();
  return siblings.size() == 0 ? nullptr : siblings.back();
}

}  // namespace

template<typename T, template<typename, template<typename...> class > class ContextT>
using OwningContext = ContextT<T, MaybeOwner>;

template<typename T> class ListOwningContext : public OwningContext<T, ListContext>
{
public:
  using OwningContext<T, ListContext>::OwningContext;
  ListOwningContext(std::unique_ptr<T> item, List<T>& structure)
    : OwningContext<T, ListContext>(*item, last_sibling(structure))
  {
    this->subject.capture(std::move(item));
  }

  ListOwningContext(std::unique_ptr<T> item, List<T>& structure, T* predecessor)
    : OwningContext<T, ListContext>(*item, predecessor)
  {
    this->subject.capture(std::move(item));
  }

};

template<typename T> class TreeOwningContext : public OwningContext<T, TreeContext>
{
public:
  using OwningContext<T, TreeContext>::OwningContext;
  TreeOwningContext(std::unique_ptr<T> item, Tree<T>& structure)
    : OwningContext<T, TreeContext>(*item, structure.root(), last_sibling(structure))
  {
    this->subject.capture(std::move(item));
  };
};

}  // namespace omm

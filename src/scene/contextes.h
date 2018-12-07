#pragma once

#include <functional>
#include <assert.h>
#include <memory>
#include <algorithm>
#include <glog/logging.h>
#include "aspects/treeelement.h"
#include "scene/structure.h"
#include "scene/contextes_fwd.h"

namespace omm
{

template<typename T, template<typename...> class Wrapper>
class ListContext
{
public:
  using item_type = T;
  ListContext(T& subject, const T* predecessor)
    : subject(subject), predecessor(predecessor)
  {
  }

  // ListContext(T& subject)
  //   : ListContext(subject, subject.predecessor())
  // {
  // }

  Wrapper<T> subject;
  T& get_subject() const { return subject; }

  /**
   * @brief the predecessing sibling of `subject`
   * @details may be `nullptr` if `subject` is the first child of `parent`.
   * @details the parent of `predecessor` must be `parent`.
   */
  const T* predecessor;

  /**
   * @brief returns the position at which subject must be inserted into the list of parent's
   *  children such that it comes after `predecessor` or at first if `predecessor` is
   *  nullptr.
   * @return the position.
   */
  size_t get_insert_position() const
  {
    if (this->predecessor == nullptr) {
      return 0;
    } else {
      return this->predecessor->row() + 1;
    }
  }

  static void remove_internal_children(std::vector<T*>& objects)
  {
    // nothing to do. List items don't have children.
  }

  static constexpr bool is_tree_context = false;
};

template<typename T, template<typename...> class Wrapper>
class TreeContext : public ListContext<T, Wrapper>
{
  static_assert( std::is_base_of<TreeElement<T>, T>::value, "T must be derived from TreeElement.");
public:
  TreeContext(T& subject, T& parent, const T* predecessor)
    : ListContext<T, Wrapper>(subject, predecessor), parent(parent)
  {
    assert(this->predecessor == nullptr || &this->predecessor->parent() == &this->parent.get());
  }

  TreeContext(T& subject, const T* predecessor)
    : ListContext<T, Wrapper>(subject, predecessor), parent(subject.parent())
  {
  }

  /**
   * @brief the parent of `subject`
   */
  std::reference_wrapper<T> parent;


  static void remove_internal_children(std::vector<T*>& objects)
  {
    auto has_parent = [&objects](const T* subject) {
      // TODO replace with std::any_of
      for (auto* potential_descendant : objects) {
        if (potential_descendant != subject && potential_descendant->is_descendant_of(*subject))
        {
          return true;
        }
      }
      return false;
    };

    objects.erase(std::remove_if(objects.begin(), objects.end(), has_parent), objects.end());
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

  bool subject_is_predecessor() const
  {
    return this->predecessor == &this->get_subject();
  }
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

}  // namespace omm

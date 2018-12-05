#pragma once

#include <functional>
#include <memory>
#include <glog/logging.h>
#include "maybeowner.h"
#include "aspects/treeelement.h"

namespace omm
{

template<typename T, template<typename...> class Wrapper>
class TreeContext
{
  static_assert( std::is_base_of<TreeElement<T>, T>::value, "T must be derived from TreeElement.");
public:
  TreeContext(T& subject, T& parent, const T* predecessor)
    : subject(subject), parent(parent), predecessor(predecessor)
  {
  }

  TreeContext(T& subject)
    : TreeContext(subject, subject.parent(), subject.predecessor())
  {
  }

  Wrapper<T> subject;
  T& get_subject() const { return subject; }

  /**
   * @brief the parent of `subject`
   */
  std::reference_wrapper<T> parent;

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
    return this->parent.get().TreeElement<T>::get_insert_position(predecessor);
  }
};

template<typename T>
class MoveTreeContext : public TreeContext<T, std::reference_wrapper>
{
public:
  using TreeContext<T, std::reference_wrapper>::TreeContext;
  /**
   * @brief returns whether the context does produce illogical states
   *  in order to return true, all of the following conditions must be true:
   *    - subject must not be root
   *    - subject and predecessor must be different objects
   *    - subject and parent must be different objects
   */
  bool is_valid() const { return !is_root() && !moves_into_itself() && !moves_after_itself(); }

  /**
   * @brief returns whether the context does produce illogical states or same state
   *  in order to return true, all of the following conditions must be true:
   *    - context must be valid (@code is_valid())
   *    - predecessor and predecessor of subject must be different objects
   */
  bool is_strictly_valid() const { return is_valid() && !moves_before_itself(); }


private:
  bool is_root() const { return this->get_subject().is_root(); }

  bool moves_into_itself() const
  {
    // subject cannot become its own parent (illogical)
    return &this->parent.get() == &this->get_subject();
  }

  bool moves_before_itself() const
  {
    const bool parent_does_not_change = &this->parent.get() == &this->get_subject().parent();
    const bool predecessor_did_not_change = this->predecessor == this->get_subject().predecessor();

    // the `parent_does_not_change test` is only required if `predecessor == nullptr`.
    return parent_does_not_change && predecessor_did_not_change;
  }

  bool moves_after_itself() const { return this->predecessor == &this->get_subject(); }
};

template<typename T> using OwningTreeContext = TreeContext<T, MaybeOwner>;

class Object;
using OwningObjectTreeContext = OwningTreeContext<Object>;
using MoveObjectTreeContext = MoveTreeContext<Object>;

}  // namespace omm

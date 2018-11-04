#pragma once

#include <functional>

namespace omm
{

class Object;

/**
 * @brief Describes the context of an object in the object tree
 * @details Subject must not be root. The class does not update itself, it is designed to
 *  describe some context which is not necessarily consistent with the tree. This is useful to
 *  model future or past contexts.
 */
class ObjectTreeContext
{
public:
  /**
   * @brief create an object which represents the `subject`'s current context
   */
  explicit ObjectTreeContext(Object& subject);

  /**
   * @brief create an object representing any context
   */
  explicit ObjectTreeContext(Object& subject, Object& parent, const Object* sibling_before);

  /**
   * @brief the object whose context is described
   */
  std::reference_wrapper<Object> subject;

  /**
   * @brief the parent of `subject`
   */
  std::reference_wrapper<Object> parent;

  /**
   * @brief the predecessing sibling of `subject`
   * @details may be `nullptr` if `subject` is the first child of `parent`.
   * @details the parent of `sibling_before` must be `parent`.
   */
  const Object* sibling_before;

  /**
   * @brief returns the position at which subject must be inserted into the list of parent's
   *  children such that it comes after `sibling_before` or at first if `sibling_before` is
   *  nullptr.
   * @return the position.
   */
  size_t get_insert_position() const;

  /**
   * @brief returns whether the context does produce illogical states
   *  in order to return true, all of the following conditions must be true:
   *    - subject must not be root
   *    - subject and sibling_before must be different objects
   *    - subject and parent must be different objects
   */
  bool is_valid() const;

  /**
   * @brief returns whether the context does produce illogical states or same state
   *  in order to return true, all of the following conditions must be true:
   *    - context must be valid (@code is_valid())
   *    - sibling_before and predecessor of subject must be different objects
   */
  bool is_strictly_valid() const;

private:
  bool is_root() const;
  bool move_into_itself() const;
  bool move_before_itself() const;
  bool move_after_itself() const;
};

}  // namespace omm

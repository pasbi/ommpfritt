#pragma once

#include <functional>
#include <memory>
#include <glog/logging.h>
#include "objects/object.h"

namespace omm
{

class Object;

template<typename T>
class MaybeOwner
{
public:
  MaybeOwner(std::unique_ptr<T>&& own) : m_owned(std::move(own)), m_ref(*m_owned) {}
  MaybeOwner(T& reference) : MaybeOwner(reference.copy()) {}
  operator T&() const { return m_ref; }
  T& reference() const { return *this; }
  bool owns() const { return !!m_owned.get(); }
  auto release() { assert(owns()); return std::move(m_owned); }
  T& capture(std::unique_ptr<T>&& own) { assert(!owns()); m_owned == own; return *this; }

private:
  std::reference_wrapper<T> m_ref;
  std::unique_ptr<T> m_owned;
};

/**
 * @brief Describes the context of an object in the object tree
 * @details Subject must not be root. The class does not update itself, it is designed to
 *  describe some context which is not necessarily consistent with the tree. This is useful to
 *  model future or past contexts.
 */


class AbstractObjectTreeContext
{
public:
  /**
   * @brief create an object representing any context
   */
  AbstractObjectTreeContext(Object& parent, const Object* predecessor);
  AbstractObjectTreeContext(Object& subject);

  /**
   * @brief the parent of `subject`
   */
  std::reference_wrapper<Object> parent;

  /**
   * @brief the predecessing sibling of `subject`
   * @details may be `nullptr` if `subject` is the first child of `parent`.
   * @details the parent of `predecessor` must be `parent`.
   */
  const Object* predecessor;

  /**
   * @brief returns the position at which subject must be inserted into the list of parent's
   *  children such that it comes after `predecessor` or at first if `predecessor` is
   *  nullptr.
   * @return the position.
   */
  size_t get_insert_position() const;
  virtual Object& get_subject() const = 0;
};

template<template<typename...> class Wrapper>
class ObjectTreeContext : public AbstractObjectTreeContext
{
public:
  ObjectTreeContext(Object& subject)
    : AbstractObjectTreeContext(subject), subject(subject)
  {
  }

  ObjectTreeContext(Object& subject, Object& parent, const Object* predecessor)
    : AbstractObjectTreeContext(parent, predecessor), subject(subject)
  {
  }

  Wrapper<Object> subject;
  Object& get_subject() const { return subject; }
};

class MoveObjectTreeContext : public ObjectTreeContext<std::reference_wrapper>
{
public:
  using ObjectTreeContext<std::reference_wrapper>::ObjectTreeContext;
  /**
   * @brief returns whether the context does produce illogical states
   *  in order to return true, all of the following conditions must be true:
   *    - subject must not be root
   *    - subject and predecessor must be different objects
   *    - subject and parent must be different objects
   */
  bool is_valid() const;

  /**
   * @brief returns whether the context does produce illogical states or same state
   *  in order to return true, all of the following conditions must be true:
   *    - context must be valid (@code is_valid())
   *    - predecessor and predecessor of subject must be different objects
   */
  bool is_strictly_valid() const;

private:
  bool is_root() const;
  bool moves_into_itself() const;
  bool moves_before_itself() const;
  bool moves_after_itself() const;
};

class CopyObjectTreeContext : public ObjectTreeContext<MaybeOwner>
{
public:
  using ObjectTreeContext<MaybeOwner>::ObjectTreeContext;
};




}  // namespace omm

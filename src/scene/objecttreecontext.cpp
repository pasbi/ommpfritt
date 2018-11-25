#include "scene/objecttreecontext.h"
#include "objects/object.h"

namespace omm
{

AbstractObjectTreeContext::AbstractObjectTreeContext(Object& parent, const Object* predecessor)
  : parent(parent), predecessor(predecessor)
{
}

AbstractObjectTreeContext::AbstractObjectTreeContext(Object& subject)
  : parent(subject.parent()), predecessor(subject.predecessor())
{
}

size_t AbstractObjectTreeContext::get_insert_position() const
{
  return parent.get().TreeElement::get_insert_position(predecessor);
}

bool MoveObjectTreeContext::is_root() const
{
  return get_subject().is_root();
}

bool MoveObjectTreeContext::moves_into_itself() const
{
  // subject cannot become its own parent (illogical)
  return &parent.get() == &get_subject();
}

bool MoveObjectTreeContext::moves_before_itself() const
{
  const bool parent_does_not_change = &parent.get() == &get_subject().parent();
  const bool predecessor_did_not_change = predecessor == get_subject().predecessor();

  // the `parent_does_not_change test` is only required if `predecessor == nullptr`.
  return parent_does_not_change && predecessor_did_not_change;
}

bool MoveObjectTreeContext::moves_after_itself() const
{
  return predecessor == &get_subject();
}

bool MoveObjectTreeContext::is_valid() const
{
  return !is_root() && !moves_into_itself() && !moves_after_itself();
}

bool MoveObjectTreeContext::is_strictly_valid() const
{
  return is_valid() && !moves_before_itself();
}

}  // namespace omm

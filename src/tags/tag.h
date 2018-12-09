#pragma once

#include <memory>
#include <QIcon>
#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include "aspects/selectable.h"
#include "aspects/copycreatable.h"

namespace omm {

class TagOwner;
class Scene;

class Tag
  : public PropertyOwner<AbstractPropertyOwner::Kind::Tag>
  , public Selectable
  , public CopyCreatable<Tag>
{
public:
  virtual ~Tag() = default;

  virtual bool run() { return true; }
  virtual QIcon icon() const = 0;

  void set_owner(TagOwner* owner);
  TagOwner* owner() const;

private:
  TagOwner* m_owner;

};

void register_tags();

}  // namespace omm

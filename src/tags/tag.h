#pragma once

#include <memory>
#include <QIcon>
#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include "aspects/selectable.h"
#include "aspects/copycreatable.h"

namespace omm {

class Object;
class Scene;

class Tag
  : public PropertyOwner<AbstractPropertyOwner::Kind::Tag>
  , public Selectable
  , public CopyCreatable<Tag>
{
public:
  explicit Tag();
  virtual ~Tag();

  virtual bool run() { return true; }
  virtual QIcon icon() const = 0;

};

void register_tags();

}  // namespace omm

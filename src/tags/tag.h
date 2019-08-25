#pragma once

#include <memory>
#include <QIcon>
#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include <Qt>

namespace omm {

class Object;
class Scene;

class Tag
  : public PropertyOwner<AbstractPropertyOwner::Kind::Tag>
  , public AbstractFactory<std::string, Tag, Object&>
{
public:
  explicit Tag(Object& owner);
  ~Tag();
  virtual QIcon icon() const = 0;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Tag", "Tag");
  virtual std::unique_ptr<Tag> clone() const = 0;
  std::unique_ptr<Tag> clone(Object& owner) const;
  Object* owner;
  virtual void evaluate() = 0;
  Flag flags() const override;
};

void register_tags();
std::ostream& operator<<(std::ostream& ostream, const Tag& tag);

}  // namespace omm

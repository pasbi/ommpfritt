#pragma once

#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include <QIcon>
#include <Qt>
#include <memory>

namespace omm
{

class Object;
class Scene;

class Tag
    : public PropertyOwner<Kind::Tag>
    , public AbstractFactory<QString, true, Tag, Object&>
{
public:
  explicit Tag(Object& owner);
  ~Tag() override;
  Tag(const Tag&) = default;
  Tag(Tag&&) = delete;
  Tag& operator=(const Tag&) = delete;
  Tag& operator=(Tag&&) = delete;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Tag", "Tag");
  std::unique_ptr<Tag> clone(Object& owner) const;
  std::unique_ptr<Tag> clone() const;
  Object* owner;
  virtual void evaluate() = 0;
  virtual void force_evaluate();
  Flag flags() const override;
  QString to_string() const override;
};

}  // namespace omm

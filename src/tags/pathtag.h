#pragma once

#include "tags/tag.h"
#include <Qt>

namespace omm
{

class PathTag : public Tag
{
public:
  explicit PathTag(Object& owner);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("PathTag", "PathTag");
  static constexpr auto PATH_REFERENCE_PROPERTY_KEY = "path";
  static constexpr auto ALIGN_REFERENCE_PROPERTY_KEY = "align";
  static constexpr auto POSITION_PROPERTY_KEY = "t";
  std::unique_ptr<Tag> clone() const override;
  void evaluate() override;
  Flag flags() const override;
};

}  // namespace omm

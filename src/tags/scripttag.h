#pragma once

#include "tags/tag.h"

namespace omm
{

class ScriptTag : public Tag
{
public:
  explicit ScriptTag(Object& owner);
  ~ScriptTag();
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "ScriptTag";
  static constexpr auto CODE_PROPERTY_KEY = "code";
};

}  // namespace omm

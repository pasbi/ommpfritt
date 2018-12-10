#pragma once

#include <memory>
#include "tags/tag.h"

namespace omm {

class StyleTag : public Tag
{
public:
  explicit StyleTag();
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto STYLE_REFERENCE = "style";
  static constexpr auto TYPE = "StyleTag";
};

}  // namespace omm

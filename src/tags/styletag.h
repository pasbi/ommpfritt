#pragma once

#include <memory>
#include "tags/tag.h"

namespace omm {

class StyleTag : public Tag
{
public:
  explicit StyleTag();
  std::string type() const override;
};

}  // namespace omm

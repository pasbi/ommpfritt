#pragma once

#include <memory>
#include "tags/tag.h"

namespace omm {

class StyleTag : public Tag
{
public:
  using Tag::Tag;
  std::string type() const override;
  QIcon icon() const override;

};

}  // namespace omm

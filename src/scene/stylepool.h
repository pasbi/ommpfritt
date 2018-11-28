#pragma once

#include "renderers/style.h"

namespace omm
{

class StylePool
{
public:
  explicit StylePool();
  const Style& default_style() const;
  std::set<Style*> styles() const;

private:
  std::unique_ptr<Style> m_default_style;
  std::vector<std::unique_ptr<Style>> m_styles;
};

}  // namespace omm

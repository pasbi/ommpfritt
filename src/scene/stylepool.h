#pragma once

#include "renderers/style.h"
#include "scene/abstractstylelistobserver.h"

namespace omm
{

class StylePool : public Observed<AbstractStyleListObserver>
{
public:
  explicit StylePool();
  const Style& default_style() const;
  std::vector<Style*> styles() const;

  void insert_style(std::unique_ptr<Style> style);
  std::unique_ptr<Style> remove_style(Style& style);

private:
  std::unique_ptr<Style> m_default_style;
  std::vector<std::unique_ptr<Style>> m_styles;
};

}  // namespace omm

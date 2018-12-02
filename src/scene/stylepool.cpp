#include "scene/stylepool.h"
#include "properties/boolproperty.h"

namespace
{

std::unique_ptr<omm::Style> make_default_style()
{
  auto default_style = std::make_unique<omm::Style>();
  default_style->property<omm::BoolProperty>(omm::Style::PEN_IS_ACTIVE_KEY).set_value(true);
  default_style->property<omm::BoolProperty>(omm::Style::BRUSH_IS_ACTIVE_KEY).set_value(true);
  return default_style;
}

}  // namespace

namespace omm
{

StylePool::StylePool()
  : m_default_style(make_default_style())
{
}

const Style& StylePool::default_style() const
{
  return *m_default_style;
}

std::vector<Style*> StylePool::styles() const
{
  return ::transform<Style*>(m_styles, [](const auto& up) { return up.get(); });
}

void StylePool::insert_style(std::unique_ptr<Style> style)
{
  Observed<AbstractStyleListObserver>::for_each([this](auto* observer){
    observer->beginInsertObject(m_styles.size());
  });
  m_styles.push_back(std::move(style));
  Observed<AbstractStyleListObserver>::for_each([](auto* observer){
    observer->endInsertObject();
  });
}

std::unique_ptr<Style> StylePool::remove_style(Style& style)
{
  return ::extract(m_styles, style);
}

}  // namespace omm

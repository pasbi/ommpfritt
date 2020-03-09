#include "managers/propertymanager/propertymanagertab.h"
#include <memory>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include "properties/typedproperty.h"
#include "propertywidgets/propertywidget.h"
#include "widgets/animationbutton.h"

namespace
{

static constexpr QSize animation_button_size(28, 28);

}  // namespace

namespace omm
{

PropertyManagerTab::PropertyManagerTab()
{
  auto layout = std::make_unique<QVBoxLayout>();
  m_layout = layout.get();
  m_layout->setSpacing(6);
  m_layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout.release());
}

PropertyManagerTab::~PropertyManagerTab()
{
}

void
PropertyManagerTab::add_properties(Scene& scene, const QString& key,
                                   const std::map<AbstractPropertyOwner*, Property*>& property_map)
{
  assert(property_map.size() > 0);
  const auto properties = ::transform<Property*, std::set>(property_map, [](const auto& pair) {
    return pair.second;
  });
  if ((*properties.begin())->is_visible()) {
    auto container_widget = std::make_unique<QWidget>(this);
    auto container_widget_layout = std::make_unique<QHBoxLayout>();
    container_widget_layout->setSpacing(0);
    if (Property::get_value<bool>(properties, std::mem_fn(&Property::is_animatable))) {
      auto animation_button = std::make_unique<AnimationButton>(scene.animator(), property_map);
      animation_button->setFixedSize(animation_button_size);
      container_widget_layout->addWidget(animation_button.release(), 0);
    } else {
      container_widget_layout->addSpacing(animation_button_size.width());
    }

    const auto widget_type = (*properties.begin())->widget_type();
    auto property_widget = AbstractPropertyWidget::make(widget_type, scene, properties);
    container_widget_layout->addWidget(property_widget.release(), 1);

    connect(*properties.begin(), SIGNAL(visibility_changed(bool)),
            container_widget.get(), SLOT(setVisible(bool)));
    container_widget->setToolTip(key);
    m_layout->addLayout(container_widget_layout.release());
  }
}

}  // namespace omm

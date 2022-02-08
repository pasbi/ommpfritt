#include "managers/propertymanager/propertymanagertab.h"
#include "properties/typedproperty.h"
#include "propertywidgets/propertywidget.h"
#include "scene/scene.h"
#include "widgets/animationbutton.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>

namespace
{
constexpr QSize animation_button_size(28, 28);

auto make_header(const QString& text)
{
  const auto make_line = []() {
    auto line = std::make_unique<QFrame>();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
  };

  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(make_line().release(), 1);
  layout->addWidget(std::make_unique<QLabel>(text).release(), 0);
  layout->addWidget(make_line().release(), 1);

  auto widget = std::make_unique<QWidget>();
  widget->setLayout(layout.release());
  return widget;
}

}  // namespace

namespace omm
{
PropertyManagerTab::PropertyManagerTab(const QString& title)
{
  auto layout = std::make_unique<QVBoxLayout>();
  m_layout = layout.get();
  static constexpr int ITEM_SPACING = 6;
  m_layout->setSpacing(ITEM_SPACING);
  m_layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout.release());

  auto header = make_header(title);
  m_header = header.get();
  m_layout->addWidget(header.release());
}

PropertyManagerTab::~PropertyManagerTab() = default;

void PropertyManagerTab::add_properties(
    Scene& scene,
    const QString& key,
    const std::map<AbstractPropertyOwner*, Property*>& property_map)
{
  assert(!property_map.empty());
  const auto properties = util::transform<std::set>(property_map, [](const auto& pair) {
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

    connect(*properties.begin(),
            &Property::visibility_changed,
            container_widget.get(),
            &QWidget::setVisible);
    container_widget->setToolTip(key);
    m_layout->addLayout(container_widget_layout.release());
  }
}

void PropertyManagerTab::set_header_visible(bool visible)
{
  m_header->setVisible(visible);
}

}  // namespace omm

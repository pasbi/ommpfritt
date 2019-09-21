#include "managers/propertymanager/propertymanagertab.h"

#include <memory>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include "properties/typedproperty.h"
#include "propertywidgets/propertywidget.h"
#include "widgets/animationbutton.h"

namespace
{

static constexpr int hspacing = 6;
static constexpr int vspacing = 6;
static constexpr QSize animation_button_size(28, 28);

}  // namespace

namespace omm
{

PropertyManagerTab::PropertyManagerTab()
{
  auto layout = std::make_unique<QFormLayout>();
  m_layout = layout.get();
  m_layout->setHorizontalSpacing(hspacing);
  m_layout->setVerticalSpacing(vspacing);
  setLayout(layout.release());
}

PropertyManagerTab::~PropertyManagerTab()
{
}

void PropertyManagerTab::add_properties(Scene& scene, const std::string& key,
                                        const std::set<Property*>& properties)
{
  assert(properties.size() > 0);

  const auto text = Property::get_value<std::string>(properties, std::mem_fn(&Property::label));
  auto label_widget = new QWidget(this);
  auto label = new QLabel(label_widget);
  auto label_layout = new QHBoxLayout(label_widget);
  label_layout->setSpacing(hspacing);
  if (Property::get_value<bool>(properties, std::mem_fn(&Property::is_animatable))) {
    auto animation_button = std::make_unique<AnimationButton>(scene.animator(), properties, label_widget);
    animation_button->setFixedSize(animation_button_size);
    label_layout->addWidget(animation_button.release());
  } else {
    label_layout->addSpacing(animation_button_size.width() + hspacing);
  }
  label_layout->addWidget(label);
  label_layout->setContentsMargins(0, 0, 0, 0);

  label->setText(QString::fromStdString(text));
  label->setToolTip(QString::fromStdString(key));

  const auto widget_type = (*properties.begin())->widget_type();
  auto property_widget = AbstractPropertyWidget::make(widget_type, scene, properties).release();
  property_widget->setParent(this);

  for (QWidget* w : std::set<QWidget*> { label_widget, property_widget }) {
    connect(*properties.begin(), SIGNAL(visibility_changed(bool)), w, SLOT(setVisible(bool)));
    if (!(*properties.begin())->is_visible()) {
      w->hide();
    }
  }

  label->setBuddy(property_widget);
  m_layout->addRow(label_widget, property_widget);
  m_layout->setContentsMargins(0, 0, 0, 0);
}

}  // namespace omm

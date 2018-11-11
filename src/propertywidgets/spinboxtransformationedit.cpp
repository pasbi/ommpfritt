#include "propertywidgets/spinboxtransformationedit.h"

#include <QDoubleSpinBox>
#include <QLabel>
#include <QGridLayout>

namespace
{

template<typename WidgetT, typename... Args>
WidgetT& add_new(QGridLayout& layout, int row, int column, Args&&... args)
{
  auto widget = std::make_unique<WidgetT>(std::forward<Args>(args)...);
  auto& widget_ref = *widget;
  layout.addWidget(widget.release(), row, column);
  return widget_ref;
}

}  // namespace

namespace omm
{

SpinBoxTransformationEdit::SpinBoxTransformationEdit()
{
  auto layout = std::make_unique<QGridLayout>();

  add_new<QLabel>(*layout, 0, 0, tr("Position:"));
  m_xposition_spinbox = &add_new<QDoubleSpinBox>(*layout, 1, 0);
  m_yposition_spinbox = &add_new<QDoubleSpinBox>(*layout, 2, 0);

  add_new<QLabel>(*layout, 0, 1, tr("Scale:"));
  m_xscale_spinbox = &add_new<QDoubleSpinBox>(*layout, 1, 1);
  m_yscale_spinbox = &add_new<QDoubleSpinBox>(*layout, 2, 1);

  add_new<QLabel>(*layout, 0, 2, tr("Shear:"));
  m_shear_spinbox = &add_new<QDoubleSpinBox>(*layout, 1, 2);

  add_new<QLabel>(*layout, 0, 3, tr("Rotation:"));
  m_rotation_spinbox = &add_new<QDoubleSpinBox>(*layout, 1, 3);

  setLayout(layout.release());

  const auto emit_value_changed = [this]() {
    Q_EMIT value_changed(value());
  };

  for (const auto& spinbox : { m_xposition_spinbox, m_xscale_spinbox, m_shear_spinbox,
                               m_yposition_spinbox, m_yscale_spinbox, m_rotation_spinbox })
  {
    spinbox->setRange(-1000, 1000);
    using T = void(QDoubleSpinBox::*)(double);
    connect(spinbox, static_cast<T>(&QDoubleSpinBox::valueChanged), emit_value_changed);
  }
}

void SpinBoxTransformationEdit::set_value(const ObjectTransformation& value)
{
  const auto parameters = value.parameters();
  m_xposition_spinbox->setValue(parameters.translation_vector(0));
  m_yposition_spinbox->setValue(parameters.translation_vector(1));
  m_xscale_spinbox->setValue(parameters.scale_vector(0));
  m_yscale_spinbox->setValue(parameters.scale_vector(1));
  m_shear_spinbox->setValue(parameters.shear);
  m_rotation_spinbox->setValue(parameters.rotation * 180.0 / M_PI);
}

ObjectTransformation SpinBoxTransformationEdit::value() const
{
  ObjectTransformation::Parameters parameters;
  parameters.translation_vector = { m_xposition_spinbox->value(), m_yposition_spinbox->value() };
  parameters.scale_vector = { m_xscale_spinbox->value(), m_yscale_spinbox->value() };
  parameters.shear = m_shear_spinbox->value();
  parameters.rotation = m_rotation_spinbox->value() * M_PI / 180.0;
  return ObjectTransformation(parameters);
}

void SpinBoxTransformationEdit::set_inconsistent_value()
{

}

}  // namespace omm

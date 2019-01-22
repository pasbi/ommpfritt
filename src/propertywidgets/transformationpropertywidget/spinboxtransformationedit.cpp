#include "propertywidgets/transformationpropertywidget/spinboxtransformationedit.h"

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

SpinBoxTransformationEdit::SpinBoxTransformationEdit(const on_value_changed_t& on_value_changed)
  : MultiValueEdit<ObjectTransformation>(on_value_changed)
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

  for (const auto& spinbox : { m_xposition_spinbox, m_xscale_spinbox, m_shear_spinbox,
                               m_yposition_spinbox, m_yscale_spinbox, m_rotation_spinbox })
  {
    spinbox->setRange(-1000, 1000);
    using T = void(QDoubleSpinBox::*)(double);
    connect(spinbox, static_cast<T>(&QDoubleSpinBox::valueChanged), [this, on_value_changed]() {
      on_value_changed(value());
    });
  }
}

void SpinBoxTransformationEdit::set_value(const ObjectTransformation& value)
{
  m_xposition_spinbox->setValue(value.translation()(0));
  m_yposition_spinbox->setValue(value.translation()(1));
  m_xscale_spinbox->setValue(value.scaling()(0));
  m_yscale_spinbox->setValue(value.scaling()(1));
  m_shear_spinbox->setValue(value.shearing());
  m_rotation_spinbox->setValue(value.rotation() * 180.0 / M_PI);
}

ObjectTransformation SpinBoxTransformationEdit::value() const
{
  ObjectTransformation t;
  t.set_translation({ m_xposition_spinbox->value(), m_yposition_spinbox->value() });
  t.set_scaling({ m_xscale_spinbox->value(), m_yscale_spinbox->value() });
  t.set_shearing(m_shear_spinbox->value());
  t.set_rotation(m_rotation_spinbox->value() * M_PI / 180.0);
  return t;
}

void SpinBoxTransformationEdit::set_inconsistent_value()
{

}

}  // namespace omm

#pragma once

#include "propertywidgets/abstracttransformationedit.h"

class QDoubleSpinBox;

namespace omm
{

class SpinBoxTransformationEdit : public AbstractTransformationEdit
{
public:
  explicit SpinBoxTransformationEdit();

  void set_value(const ObjectTransformation& value) override;
  ObjectTransformation value() const override;

protected:
  void set_inconsistent_value() override;

  QDoubleSpinBox* m_rotation_spinbox;
  QDoubleSpinBox* m_xscale_spinbox;
  QDoubleSpinBox* m_yscale_spinbox;
  QDoubleSpinBox* m_xposition_spinbox;
  QDoubleSpinBox* m_yposition_spinbox;
  QDoubleSpinBox* m_shear_spinbox;
};

}
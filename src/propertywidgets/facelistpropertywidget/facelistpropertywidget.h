#pragma once

#include "properties/facelistproperty.h"
#include "propertywidgets/propertywidget.h"

namespace omm
{

class FaceListWidget;

class FaceListPropertyWidget : public PropertyWidget<FaceListProperty>
{
public:
  explicit FaceListPropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;

private:
  FaceListWidget* m_face_list_widget;
};

}  // namespace omm

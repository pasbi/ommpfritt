#include "propertywidgets/facelistpropertywidget/facelistpropertywidget.h"
#include "properties/typedproperty.h"
#include "propertywidgets/facelistpropertywidget/facelistwidget.h"

#include <QLabel>
#include <QVBoxLayout>

namespace omm
{
FaceListPropertyWidget::FaceListPropertyWidget(Scene& scene, const std::set<Property*>& properties)
    : PropertyWidget(scene, properties)
{
  auto face_list_widget = std::make_unique<FaceListWidget>();
  m_face_list_widget = face_list_widget.get();
  set_widget(std::move(face_list_widget));
  FaceListPropertyWidget::update_edit();
}

void FaceListPropertyWidget::update_edit()
{
  QSignalBlocker blocker(m_face_list_widget);
  m_face_list_widget->set_values(get_properties_values());
}

}  // namespace omm

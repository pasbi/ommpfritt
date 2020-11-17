#include "propertywidgets/propertyconfigwidget.h"
#include "properties/property.h"
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace omm
{
void AbstractPropertyConfigWidget::hideEvent(QHideEvent* event)
{
  QWidget::hideEvent(event);
  Q_EMIT hidden();
}

}  // namespace omm

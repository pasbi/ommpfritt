#include "propertywidgets/propertyconfigwidget.h"
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include "properties/property.h"
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>

namespace omm
{

void AbstractPropertyConfigWidget::hideEvent(QHideEvent *event)
{
  QWidget::hideEvent(event);
  Q_EMIT hidden();
}

}  // namespace omm

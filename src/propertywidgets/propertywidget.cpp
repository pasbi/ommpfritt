#include "propertywidgets/propertywidget.h"

#include <QBoxLayout>
#include <QLabel>

namespace omm
{

AbstractPropertyWidget::~AbstractPropertyWidget()
{
}

void AbstractPropertyWidget::set_default_layout(std::unique_ptr<QWidget> other)
{
  setLayout(new QHBoxLayout());
  layout()->addWidget(make_label_widget().release());
  layout()->addWidget(other.release());
}

std::unique_ptr<QWidget> AbstractPropertyWidget::make_label_widget() const
{
  return std::make_unique<QLabel>(QString::fromStdString(label()));
}

}  // namespace omm

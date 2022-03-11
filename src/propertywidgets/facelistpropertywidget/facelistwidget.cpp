#include "propertywidgets/facelistpropertywidget/facelistwidget.h"

#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>

namespace omm
{

void FaceListWidget::set_value(const value_type& value)
{
  Q_UNUSED(value)
}

void FaceListWidget::set_inconsistent_value()
{
  set_value(FaceList{});
}

FaceListWidget::value_type FaceListWidget::value() const
{
  return FaceList{};
}

FaceListWidget::FaceListWidget(QWidget* parent)
    : QWidget(parent)
{
  setFocusPolicy(Qt::StrongFocus);
  auto layout = std::make_unique<QGridLayout>();

  auto insert = [layout=layout.get()](auto&& widget, const int row, const int col, const int col_span) -> auto& {
    auto& ref = *widget;
    layout->addWidget(widget.release(), row, col, 1, col_span);
    return ref;
  };

  m_lw = &insert(std::make_unique<QListWidget>(), 0, 0, 2);
  m_pb_clear = &insert(std::make_unique<QPushButton>(tr("Clear")), 1, 0, 1);
  m_cb_invert = &insert(std::make_unique<QCheckBox>("Invert"), 1, 1, 1);
  setLayout(layout.release());
}

}  // namespace omm

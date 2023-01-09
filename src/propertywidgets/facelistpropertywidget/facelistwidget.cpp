#include "propertywidgets/facelistpropertywidget/facelistwidget.h"

#include "objects/pathobject.h"
#include "path/pathvector.h"
#include "removeif.h"
#include "path/face.h"
#include "path/edge.h"

#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>


namespace omm
{

void FaceListWidget::set_value(const value_type& value)
{
  m_lw->clear();
  for (const auto& face : value.faces()) {
    m_lw->addItem(face.to_string());
  }
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
  auto layout = std::make_unique<QVBoxLayout>();

  auto insert = [layout=layout.get()](auto&& widget) -> auto& {
    auto& ref = *widget;
    layout->addWidget(widget.release());
    return ref;
  };

  m_lw = &insert(std::make_unique<QListWidget>());
  m_pb_clear = &insert(std::make_unique<QPushButton>(tr("Clear")));
  setLayout(layout.release());
}

}  // namespace omm

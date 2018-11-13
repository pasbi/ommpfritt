#include "propertywidgets/referencelineedit.h"

#include <QDragEnterEvent>
#include <QMimeData>

#include "properties/referenceproperty.h"
#include "managers/objectmanager/objectmimedata.h"
#include "objects/object.h"

namespace omm
{

ReferenceLineEdit::ReferenceLineEdit()
{
  connect(this, &QLineEdit::textChanged, [this]() { setPlaceholderText(""); });
  setReadOnly(true);
}

void ReferenceLineEdit::set_value(const value_type& value)
{
  if (m_value != value) {
    m_value = value;
    if (value == nullptr) {
      setText(tr("< no reference >"));
    } else {
      setText(QString::fromStdString(value->name()));
    }
    Q_EMIT reference_changed(value);
  }
}

void ReferenceLineEdit::set_inconsistent_value()
{
  setPlaceholderText(tr("<multiple values>"));
  clear();
}

void ReferenceLineEdit::dragEnterEvent(QDragEnterEvent* event)
{
  if (can_drop(*event->mimeData())) {
    event->accept();
  } else {
    event->ignore();
  }
}

void ReferenceLineEdit::dropEvent(QDropEvent* event)
{
  const auto& mime_data = *event->mimeData();
  if (!can_drop(mime_data)) {
    event->ignore();
  } else {
    const auto& object_mime_data = *qobject_cast<const ObjectMimeData*>(&mime_data);
    HasProperties* reference = &object_mime_data.objects.front().get();
    set_value(reference);
  }
}

ReferenceLineEdit::value_type ReferenceLineEdit::value() const
{
  return m_value;
}

bool ReferenceLineEdit::can_drop(const QMimeData& mime_data) const
{
  if (mime_data.hasFormat(ObjectMimeData::MIME_TYPE))
  {
    const auto object_mime_data = qobject_cast<const ObjectMimeData*>(&mime_data);
    if (object_mime_data != nullptr) {
      if (object_mime_data->objects.size() == 1) {
        return true;
      }
    }
  }
  return false;
}

void ReferenceLineEdit::mouseDoubleClickEvent(QMouseEvent* event)
{
  set_value(nullptr);
}

}  // namespace omm

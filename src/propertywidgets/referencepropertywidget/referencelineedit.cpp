#include "propertywidgets/referencepropertywidget/referencelineedit.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QLineEdit>

#include "properties/referenceproperty.h"
#include "scene/propertyownermimedata.h"
#include "objects/object.h"
#include "scene/scene.h"
#include "renderers/style.h"

namespace omm
{

std::vector<omm::AbstractPropertyOwner*>
candidates(const Scene& scene, omm::AbstractPropertyOwner::Kind kind)
{
  std::vector<omm::AbstractPropertyOwner*> candidates = { nullptr };
  auto merge = [&candidates](const auto& ts) {
    candidates.insert(candidates.end(), ts.begin(), ts.end());
  };
  if (!!(kind & omm::AbstractPropertyOwner::Kind::Object)) {
    merge(scene.Tree<Object>::items());
  }
  if (!!(kind & omm::AbstractPropertyOwner::Kind::Tag)) {
    merge(scene.tags());
  }
  if (!!(kind & omm::AbstractPropertyOwner::Kind::Style)) {
    merge(scene.List<Style>::items());
  }

  return candidates;
}

ReferenceLineEdit::ReferenceLineEdit(const Scene& scene, AbstractPropertyOwner::Kind allowed_kinds)
  : m_allowed_kinds(allowed_kinds)
  , m_possible_references(candidates(scene, allowed_kinds))
{
  setEditable(false);
  setAcceptDrops(true);
  for (auto candidate : m_possible_references) {
    if (candidate) {
      addItem(QString::fromStdString(candidate->name()));
    } else {
      addItem(tr("< none >"));
    }
  }
  connect( this, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           [this](int index) { set_value(m_possible_references[index]); } );
}

ReferenceLineEdit::~ReferenceLineEdit()
{
}

void ReferenceLineEdit::set_value(const value_type& value)
{
  bool value_has_changed = m_value != value;
  m_value = value;
  const auto it = std::find(m_possible_references.begin(), m_possible_references.end(), value);
  assert(it != m_possible_references.end());
  setCurrentIndex(std::distance(m_possible_references.begin(), it));

  if (value_has_changed) {
    Q_EMIT reference_changed(value);
  }
}

void ReferenceLineEdit::set_inconsistent_value()
{
  setEditText(tr("<multiple values>"));
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
    const auto& property_owner_mime_data = *qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
    AbstractPropertyOwner* reference = property_owner_mime_data.items(m_allowed_kinds).front();
    set_value(reference);
  }
}

ReferenceLineEdit::value_type ReferenceLineEdit::value() const
{
  return m_value;
}

bool ReferenceLineEdit::can_drop(const QMimeData& mime_data) const
{
  if (mime_data.hasFormat(PropertyOwnerMimeData::MIME_TYPE))
  {
    const auto property_owner_mime_data = qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
    if (property_owner_mime_data != nullptr) {
      if (property_owner_mime_data->items(m_allowed_kinds).size() == 1) {
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

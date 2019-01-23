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
collect_candidates(const Scene& scene, omm::AbstractPropertyOwner::Kind kind)
{
  std::vector<omm::AbstractPropertyOwner*> candidates = { nullptr };
  auto merge = [&candidates](const auto& ts) {
    candidates.insert(candidates.end(), ts.begin(), ts.end());
  };
  if (!!(kind & omm::AbstractPropertyOwner::Kind::Object)) {
    merge(scene.object_tree.items());
  }
  if (!!(kind & omm::AbstractPropertyOwner::Kind::Tag)) {
    merge(scene.tags());
  }
  if (!!(kind & omm::AbstractPropertyOwner::Kind::Style)) {
    merge(scene.styles.items());
  }

  return candidates;
}

ReferenceLineEdit::ReferenceLineEdit( Scene& scene, AbstractPropertyOwner::Kind allowed_kinds,
                                      const on_value_changed_t& on_value_changed )
  : MultiValueEdit<AbstractPropertyOwner *>(on_value_changed)
  , m_scene(scene)
  , m_allowed_kinds(allowed_kinds)
{
  setEditable(false);
  setAcceptDrops(true);
  connect( this, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           [this](int index) { set_value(m_possible_references[index]); } );
  m_scene.Observed<AbstractSimpleStructureObserver>::register_observer(*this);
  update_candidates();
}

ReferenceLineEdit::~ReferenceLineEdit()
{
  m_scene.Observed<AbstractSimpleStructureObserver>::unregister_observer(*this);
}

void ReferenceLineEdit::update_candidates()
{
  m_possible_references = collect_candidates(m_scene, m_allowed_kinds);
  QSignalBlocker blocker(this);
  AbstractPropertyOwner* const value_safe = currentIndex() < 0 ? nullptr : value();
  clear();
  for (auto candidate : m_possible_references) {
    if (candidate) {
      addItem(QString::fromStdString(candidate->name()));
    } else {
      addItem(tr("< none >"));
    }
  }
  set_value(value_safe);
}

void ReferenceLineEdit::set_value(const value_type& value)
{
  bool value_has_changed = m_value != value;
  m_value = value;
  const auto it = std::find(m_possible_references.begin(), m_possible_references.end(), value);
  assert(it != m_possible_references.end());
  setCurrentIndex(std::distance(m_possible_references.begin(), it));

  if (value_has_changed && !signalsBlocked()) { on_value_changed(value); }
}

void ReferenceLineEdit::set_inconsistent_value() { setEditText(tr("<multiple values>")); }
ReferenceLineEdit::value_type ReferenceLineEdit::value() const { return m_value; }
void ReferenceLineEdit::mouseDoubleClickEvent(QMouseEvent* event) { set_value(nullptr); }
void ReferenceLineEdit::structure_has_changed() { update_candidates(); }

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


}  // namespace omm

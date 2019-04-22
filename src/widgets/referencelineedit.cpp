#include "widgets/referencelineedit.h"

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

ReferenceLineEdit::ReferenceLineEdit(Scene& scene, const on_value_changed_t& on_value_changed)
  : MultiValueEdit<AbstractPropertyOwner *>(on_value_changed)
  , m_scene(scene)
{
  setEditable(false);
  setAcceptDrops(true);
  connect( this, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           [this](int index) { set_value(m_possible_references[static_cast<std::size_t>(index)]); } );
  connect(&m_scene, SIGNAL(structure_has_changed()), this, SLOT(update_candidates()));
  set_null_label(QObject::tr("< none >", "ReferenceLineEdit").toStdString());
}

void ReferenceLineEdit::set_null_label(const std::string& value)
{
  m_null_label = value;
  update_candidates();
}

void ReferenceLineEdit::update_candidates()
{
  m_possible_references = collect_candidates();
  QSignalBlocker blocker(this);
  AbstractPropertyOwner* const value_safe = currentIndex() < 0 ? nullptr : value();
  clear();
  for (auto candidate : m_possible_references) {
    if (candidate) {
      addItem(QString::fromStdString(candidate->name()));
    } else {
      addItem(QString::fromStdString(m_null_label));
    }
  }
  set_value(value_safe);
}

void ReferenceLineEdit::set_value(const value_type& value)
{
  const bool value_has_changed = m_value != value;
  if (::contains(m_possible_references, value)) {
    m_value = value;
  } else {
    m_value = nullptr;
  }
  assert(::contains(m_possible_references, nullptr));
  const auto it = std::find(m_possible_references.begin(), m_possible_references.end(), m_value);
  assert(it != m_possible_references.end());
  setCurrentIndex(static_cast<int>(std::distance(m_possible_references.begin(), it)));

  if (value_has_changed && !signalsBlocked()) { on_value_changed(m_value); }
}

void ReferenceLineEdit::set_inconsistent_value()
{
  setEditText(QObject::tr("<multiple values>", "ReferenceLineEdit"));
}

ReferenceLineEdit::value_type ReferenceLineEdit::value() const { return m_value; }
void ReferenceLineEdit::mouseDoubleClickEvent(QMouseEvent*) { set_value(nullptr); }

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

void ReferenceLineEdit::set_filter(AbstractPropertyOwner::Kind allowed_kinds)
{
  m_allowed_kinds = allowed_kinds;
  update_candidates();
}

void ReferenceLineEdit::set_filter(AbstractPropertyOwner::Flag required_flags)
{
  m_required_flags = required_flags;
  update_candidates();
}

std::vector<omm::AbstractPropertyOwner*> ReferenceLineEdit::collect_candidates()
{
  std::vector<omm::AbstractPropertyOwner*> candidates = { };
  auto merge = [&candidates](const auto& ts) {
    candidates.insert(candidates.end(), ts.begin(), ts.end());
  };
  if (!!(m_allowed_kinds & omm::AbstractPropertyOwner::Kind::Object)) {
    merge(m_scene.object_tree.items());
  }
  if (!!(m_allowed_kinds & omm::AbstractPropertyOwner::Kind::Tag)) {
    merge(m_scene.tags());
  }
  if (!!(m_allowed_kinds & omm::AbstractPropertyOwner::Kind::Style)) {
    merge(m_scene.styles.items());
  }

  const auto not_has_required_flags = [this](const omm::AbstractPropertyOwner* apo) {
    return m_required_flags & ~apo->flags();
  };

  candidates.erase( std::remove_if(candidates.begin(), candidates.end(), not_has_required_flags),
                    candidates.end() );
  candidates.push_back(nullptr);
  return candidates;
}

}  // namespace omm

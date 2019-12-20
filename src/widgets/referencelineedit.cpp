#include "widgets/referencelineedit.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QLineEdit>
#include <QTimer>

#include "properties/referenceproperty.h"
#include "scene/propertyownermimedata.h"
#include "objects/object.h"
#include "scene/scene.h"
#include "renderers/style.h"
#include "scene/messagebox.h"
#include "scene/stylelist.h"

namespace omm
{

ReferenceLineEdit::ReferenceLineEdit(QWidget* parent) : QComboBox(parent)
{
  setEditable(true);
  setAcceptDrops(true);
  const auto set_value = [this](int index) {
    this->set_value(m_possible_references[static_cast<std::size_t>(index)]);
    QSignalBlocker blocker(this);
    QTimer::singleShot(1, this, SLOT(convert_text_to_placeholder_text()));
  };
  connect(this, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), set_value);
  QTimer::singleShot(1, this, SLOT(convert_text_to_placeholder_text()));
}

void ReferenceLineEdit::set_null_label(const QString& value)
{
  m_null_label = value;
  update_candidates();
}

void ReferenceLineEdit::set_scene(Scene &scene)
{
  set_null_label(QObject::tr("< none >", "ReferenceLineEdit"));

  // scene must not be re-set and must not be cleared.
  // There's nothing fundamentally speaking against this, however, it is not required and slightly
  // simpler to program...
  assert(m_scene == nullptr);
  m_scene = &scene;
  assert(m_scene != nullptr);
  const auto update_candidates_maybe = [this](Object&, AbstractPropertyOwner& object) {
    if (static_cast<bool>(object.flags() & AbstractPropertyOwner::Flag::HasScript)) {
      update_candidates();
    }
  };

  connect(&m_scene->message_box(), &MessageBox::object_removed, this, update_candidates_maybe);
  connect(&m_scene->message_box(), &MessageBox::object_inserted, this, update_candidates_maybe);
  connect(&m_scene->message_box(), &MessageBox::tag_inserted, this, update_candidates_maybe);
  connect(&m_scene->message_box(), &MessageBox::tag_removed, this, update_candidates_maybe);
  connect(&m_scene->message_box(), SIGNAL(scene_reseted()), this, SLOT(update_candidates()));
  connect(&m_scene->message_box(), &MessageBox::property_value_changed, this,
          [this](AbstractPropertyOwner& owner, const QString& key, Property&)
  {
    if (static_cast<bool>(owner.flags() & AbstractPropertyOwner::Flag::HasScript)) {
      if (key == AbstractPropertyOwner::NAME_PROPERTY_KEY) {
        update_candidates();
      }
    }
  });
  update_candidates();
}

void ReferenceLineEdit::update_candidates()
{
  if (m_scene != nullptr) {
    m_possible_references = collect_candidates();
    QSignalBlocker blocker(this);
    AbstractPropertyOwner* const value_safe = currentIndex() < 0 ? nullptr : value();
    clear();
    for (auto candidate : m_possible_references) {
      if (candidate) {
        addItem(candidate->name());
      } else {
        addItem(m_null_label);
      }
    }
    set_value(value_safe);
  }
}

void ReferenceLineEdit::convert_text_to_placeholder_text()
{
  lineEdit()->setPlaceholderText(lineEdit()->text());
  lineEdit()->setText("");
}

void ReferenceLineEdit::set_value(const value_type& value)
{
  assert(m_scene != nullptr);
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

  if (value_has_changed && !signalsBlocked()) {
    Q_EMIT value_changed(m_value);
  }
}

void ReferenceLineEdit::set_inconsistent_value()
{
  setEditText(QObject::tr("<multiple values>", "ReferenceLineEdit"));
}

ReferenceLineEdit::value_type ReferenceLineEdit::value() const { return m_value; }
void ReferenceLineEdit::mouseDoubleClickEvent(QMouseEvent*) { set_value(nullptr); }

void ReferenceLineEdit::dragEnterEvent(QDragEnterEvent* event)
{
  event->setDropAction(Qt::LinkAction);
  if (can_drop(*event)) {
    event->accept();
  } else {
    event->ignore();
  }
}

void ReferenceLineEdit::dropEvent(QDropEvent* event)
{
  event->setDropAction(Qt::LinkAction);
  if (can_drop(*event)) {
    const auto& mime_data = *event->mimeData();
    const auto& property_owner_mime_data = *qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
    AbstractPropertyOwner* reference = property_owner_mime_data.items(m_allowed_kinds).front();
    set_value(reference);
  } else {
    event->ignore();
  }
}

bool ReferenceLineEdit::can_drop(const QDropEvent& event) const
{
  const auto& mime_data = *event.mimeData();
  if (event.dropAction() != Qt::LinkAction) {
    return false;
  } else if (mime_data.hasFormat(PropertyOwnerMimeData::MIME_TYPE)) {
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
    merge(m_scene->object_tree().items());
  }
  if (!!(m_allowed_kinds & omm::AbstractPropertyOwner::Kind::Tag)) {
    merge(m_scene->tags());
  }
  if (!!(m_allowed_kinds & omm::AbstractPropertyOwner::Kind::Style)) {
    merge(m_scene->styles().items());
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

#include "widgets/referencelineedit.h"

#include <QDragEnterEvent>
#include <QLineEdit>
#include <QTimer>

#include "removeif.h"
#include "objects/object.h"
#include "properties/referenceproperty.h"
#include "renderers/style.h"
#include "scene/mailbox.h"
#include "scene/propertyownermimedata.h"
#include "scene/scene.h"
#include "scene/stylelist.h"
#include "tags/tag.h"

namespace omm
{
ReferenceLineEdit::ReferenceLineEdit(QWidget* parent)
    : QComboBox(parent)
    , m_filter(PropertyFilter::accept_anything())
    , m_null_label(QObject::tr("< none >", "ReferenceLineEdit"))
{
  setEditable(true);
  setAcceptDrops(true);
  const auto set_value = [this](int index) {
    this->set_value(m_possible_references[static_cast<std::size_t>(index)]);
    QSignalBlocker blocker(this);
    QTimer::singleShot(1, this, &ReferenceLineEdit::convert_text_to_placeholder_text);
  };
  connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), set_value);

  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  QTimer::singleShot(1, this, &ReferenceLineEdit::convert_text_to_placeholder_text);

  lineEdit()->installEventFilter(this);
  installEventFilter(this);
}

void ReferenceLineEdit::set_null_label(const QString& value)
{
  m_null_label = value;
  update_candidates();
}

void ReferenceLineEdit::set_scene(Scene& scene)
{
  // scene must not be re-set and must not be cleared.
  // There's nothing fundamentally speaking against this, however, it is not required and slightly
  // simpler to program...
  assert(m_scene == nullptr);
  m_scene = &scene;
  assert(m_scene != nullptr);

  connect(&m_scene->mail_box(),
          &MailBox::abstract_property_owner_inserted,
          this,
          &ReferenceLineEdit::update_candidates);
  connect(&m_scene->mail_box(),
          &MailBox::abstract_property_owner_removed,
          this,
          &ReferenceLineEdit::update_candidates);
  connect(&m_scene->mail_box(),
          &MailBox::scene_reseted,
          this,
          &ReferenceLineEdit::update_candidates);
  connect(&m_scene->mail_box(),
          &MailBox::property_value_changed,
          this,
          [this](AbstractPropertyOwner& owner, const QString& key, Property&) {
            using Flag = Flag;
            if (!!(owner.flags() & (Flag::HasScript | Flag::HasPython))) {
              if (key == AbstractPropertyOwner::NAME_PROPERTY_KEY) {
                update_candidates();
              }
            }
          });

  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  QTimer::singleShot(0, this, &ReferenceLineEdit::update_candidates);
  update_candidates();
}

void ReferenceLineEdit::update_candidates()
{
  if (m_scene != nullptr) {
    m_possible_references = collect_candidates();
    QSignalBlocker blocker(this);
    AbstractPropertyOwner* const value_safe = currentIndex() < 0 ? nullptr : value();
    clear();
    for (const auto* candidate : m_possible_references) {
      if (candidate != nullptr) {
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
  const auto pt = lineEdit()->text();
  if (!pt.isEmpty()) {
    lineEdit()->setPlaceholderText(pt);
    lineEdit()->setText("");
  }
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

ReferenceLineEdit::value_type ReferenceLineEdit::value() const
{
  return m_value;
}

void ReferenceLineEdit::set_filter(const PropertyFilter& filter)
{
  if (m_filter != filter) {
    m_filter = filter;
    update_candidates();
  }
}

void ReferenceLineEdit::mouseDoubleClickEvent(QMouseEvent*)
{
  set_value(nullptr);
}

bool ReferenceLineEdit::eventFilter(QObject* o, QEvent* e)
{
  if (o == this || o == lineEdit()) {
    switch (e->type()) {
    case QEvent::Drop:
      return drop(dynamic_cast<QDropEvent&>(*e));
    case QEvent::DragEnter:
      return drag_enter(dynamic_cast<QDragEnterEvent&>(*e));
    case QEvent::DragMove:
      e->accept();
      return true;
    default:;
    }
  }
  return QComboBox::eventFilter(o, e);
}

bool ReferenceLineEdit::drag_enter(QDragEnterEvent& event)
{
  event.setDropAction(Qt::LinkAction);
  if (can_drop(event)) {
    event.accept();
    return true;
  } else {
    event.ignore();
    return false;
  }
}

bool ReferenceLineEdit::drop(QDropEvent& event)
{
  event.setDropAction(Qt::LinkAction);
  if (can_drop(event)) {
    const auto& mime_data = *event.mimeData();
    const auto& property_owner_mime_data = *qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
    const auto items = util::remove_if(property_owner_mime_data.items(), [this](const AbstractPropertyOwner* apo) {
      return !m_filter.accepts(*apo);
    });
    assert(!items.empty());
    set_value(items.front());
    return true;
  } else {
    event.ignore();
    return false;
  }
}

bool ReferenceLineEdit::can_drop(const QDropEvent& event) const
{
  const auto& mime_data = *event.mimeData();
  if (event.dropAction() != Qt::LinkAction) {
    return false;
  } else if (mime_data.hasFormat(PropertyOwnerMimeData::MIME_TYPE)) {
    const auto* property_owner_mime_data = qobject_cast<const PropertyOwnerMimeData*>(&mime_data);
    if (property_owner_mime_data != nullptr) {
      const auto items = property_owner_mime_data->items();
      return items.size() == 1 && m_filter.accepts(*items.front());
    }
  }
  return false;
}

std::vector<omm::AbstractPropertyOwner*> ReferenceLineEdit::collect_candidates()
{
  std::vector<omm::AbstractPropertyOwner*> candidates = {};
  auto merge = [&candidates](const auto& ts) {
    candidates.insert(candidates.end(), ts.begin(), ts.end());
  };
  merge(m_scene->object_tree().items());
  merge(m_scene->tags());
  merge(m_scene->styles().items());
  std::erase_if(candidates, [this](const AbstractPropertyOwner* apo) { return !m_filter.accepts(*apo); });
  candidates.push_back(nullptr);  // No reference is always an option.
  return candidates;
}

}  // namespace omm

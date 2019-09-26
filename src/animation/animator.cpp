#include "animation/animator.h"

#include "logging.h"
#include "serializers/abstractserializer.h"
#include "animation/track.h"
#include "scene/scene.h"
#include "aspects/propertyowner.h"
#include "tags/tag.h"
#include "renderers/style.h"
#include "scene/stylelist.h"
#include <list>
#include <functional>
#include "scene/messagebox.h"
#include "mainwindow/application.h"

namespace
{

template<typename T>
T* predecessor(const std::vector<T*>& candidates, const T& t,
               const std::function<bool(const T&)>& predicate)
{
  T* predecessor = nullptr;
  for (T* candidate : candidates) {
    if (candidate == &t) {
      break;
    } else if (predicate(*candidate)) {
      predecessor = candidate;
    }
  }
  return predecessor;
}

}

namespace omm
{

Animator::Animator(Scene& scene) : scene(scene), accelerator(*this)
{
  m_timer.setInterval(1000.0/30.0);
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(advance()));
  connect(this, SIGNAL(current_changed(int)), this, SLOT(apply()));
  connect(&scene.message_box(), &MessageBox::property_value_changed, this,
               [this](AbstractPropertyOwner& owner, const std::string& key, Property&)
  {
    if (key == AbstractPropertyOwner::NAME_PROPERTY_KEY) {
      const auto& owners = this->accelerator().owners();
      if (std::find(owners.begin(), owners.end(), &owner) != owners.end()) {
         const QModelIndex index = this->index(owner);
         Q_EMIT dataChanged(index, index, { Qt::DisplayRole });
      }
    }
  });

  connect(&scene.message_box(), SIGNAL(abstract_property_owner_inserted(AbstractPropertyOwner&)),
          this, SLOT(invalidate()));
  connect(&scene.message_box(), SIGNAL(abstract_property_owner_removed(AbstractPropertyOwner&)),
          this, SLOT(invalidate()));
  connect(this, SIGNAL(knot_moved(Track&, int, int)), this, SIGNAL(track_changed(Track&)));
  connect(this, SIGNAL(knot_removed(Track&, int)), this, SIGNAL(track_changed(Track&)));
  connect(this, SIGNAL(knot_inserted(Track&, int)), this, SIGNAL(track_changed(Track&)));
  connect(this, SIGNAL(track_inserted(Track&)), this, SIGNAL(track_changed(Track&)));
  connect(this, SIGNAL(track_removed(Track&)), this, SIGNAL(track_changed(Track&)));
}

Animator::~Animator()
{

}

void Animator::serialize(AbstractSerializer &serializer, const Serializable::Pointer &pointer) const
{
  Serializable::serialize(serializer, pointer);

  serializer.set_value(m_start_frame, make_pointer(pointer, START_FRAME_POINTER));
  serializer.set_value(m_end_frame, make_pointer(pointer, END_FRAME_POINTER));
  serializer.set_value(m_current_frame, make_pointer(pointer, CURRENT_FRAME_POINTER));
}

void Animator::deserialize(AbstractDeserializer &deserializer, const Pointer &pointer)
{
  Serializable::deserialize(deserializer, pointer);

  set_start(deserializer.get_int(make_pointer(pointer, START_FRAME_POINTER)));
  set_end(deserializer.get_int(make_pointer(pointer, END_FRAME_POINTER)));
  set_current(deserializer.get_int(make_pointer(pointer, CURRENT_FRAME_POINTER)));
  invalidate();
}

void Animator::set_start(int start)
{
  if (m_start_frame != start) {
    m_start_frame = start;
    Q_EMIT start_changed(start);
  }
}

void Animator::set_end(int end)
{
  if (m_end_frame != end) {
    m_end_frame = end;
    Q_EMIT end_changed(end);
  }
}

void Animator::set_current(int current)
{
  if (m_current_frame != current) {
    m_current_frame = current;
    Q_EMIT current_changed(current);
  }
}

void Animator::toggle_play_pause(bool play)
{
  if (m_is_playing != play) {
    m_is_playing = play;
    if (play) {
      m_timer.start();
    } else {
      m_timer.stop();
    }
    Q_EMIT play_pause_toggled(play);
  }
}

void Animator::advance()
{
  int next = m_current_frame + 1;
  if (next > m_end_frame) {
    if (m_play_mode == PlayMode::Repeat) {
      next = m_start_frame;
    } else if (m_play_mode == PlayMode::Stop) {
      next = m_end_frame;
      m_timer.stop();
    }
  }

  if (next != m_current_frame) {
    set_current(next);
  }
}

void Animator::apply()
{
  for (Property* property : accelerator().properties()) {
    property->track()->apply(m_current_frame);
  }
}

void Animator::invalidate()
{
  beginResetModel();
  accelerator.invalidate();
  endResetModel();
}

Animator::Accelerator Animator::CachedAnimatedPropertiesGetter::compute() const
{
  return Accelerator(m_self.scene);
}

QModelIndex Animator::index(int row, int column, const QModelIndex &parent) const
{
  if (parent.isValid()) {
    AbstractPropertyOwner* owner = this->owner(parent);
    return index(*accelerator().properties(*owner).at(row), column);
  } else {
    return index(*accelerator().owners()[row], column);
  }
}

QModelIndex Animator::parent(const QModelIndex &child) const
{
  switch (index_type(child)) {
  case IndexType::None:
    [[fallthrough]];
  case IndexType::Owner:
    return QModelIndex();
  case IndexType::Property:
    return index(*accelerator().owner(*property(child)));
  default:
    Q_UNREACHABLE();
    return QModelIndex();
  }
}

int Animator::rowCount(const QModelIndex &parent) const
{
  switch (index_type(parent)) {
  case IndexType::None:
    return accelerator().owners().size();
  case IndexType::Owner:
    return accelerator().properties(*owner(parent)).size();
  case IndexType::Property:
    [[ fallthrough ]];
  default:
    return 0;
  }
}

int Animator::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 2;
}

QVariant Animator::data(const QModelIndex &index, int role) const
{
  switch (index_type(index)) {
  case IndexType::Owner:
    switch (index.column()) {
    case 0:
      switch (role) {
      case Qt::DisplayRole:
        return QString::fromStdString(owner(index)->name());
      case Qt::DecorationRole:
        return Application::instance().icon_provider.icon(*owner(index));
      default:
        return QVariant();
      }
    case 1:
      return QVariant();
    default:
      Q_UNREACHABLE();  // there are only two columns
    }
    break;
  case IndexType::Property:
    switch (index.column()) {
    case 0:
      switch (role) {
      case Qt::DisplayRole:
        return QString::fromStdString(property(index)->label());
      default:
        return QVariant();
      }
    case 1:
      return QVariant();
    default:
      Q_UNREACHABLE();  // there are only two columns
    }
  default:
    Q_UNREACHABLE();
  }

  Q_UNREACHABLE();
  return QVariant();
}

Qt::ItemFlags Animator::flags(const QModelIndex& index) const
{
  Q_UNUSED(index);
  return Qt::ItemIsEnabled;
}

QModelIndex Animator::index(Property& property, int column) const
{
  AbstractPropertyOwner& owner = *accelerator().owner(property);
  const auto props = accelerator().properties(owner);
  const int row = std::distance(props.begin(), std::find(props.begin(), props.end(), &property));
  return createIndex(row, column, &property);
}

QModelIndex Animator::index(AbstractPropertyOwner& owner, int column) const
{
  const auto& owners = accelerator().owners();
  const int row = std::distance(owners.begin(), std::find(owners.begin(), owners.end(), &owner));
  return createIndex(row, column, &owner);
}

Animator::IndexType Animator::index_type(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return IndexType::None;
  }

  const QObject* internal_pointer = static_cast<QObject*>(index.internalPointer());
  if (internal_pointer->inherits(Property::staticMetaObject.className())) {
    return IndexType::Property;
  } else if (internal_pointer->inherits(AbstractPropertyOwner::staticMetaObject.className())) {
    return IndexType::Owner;
  } else {
    Q_UNREACHABLE();
    return IndexType::None;
  }
}

Property* Animator::property(const QModelIndex& index) const
{
  assert(index_type(index) == IndexType::Property);
  return static_cast<Property*>(index.internalPointer());
}

AbstractPropertyOwner* Animator::owner(const QModelIndex& index) const
{
  assert(index_type(index) == IndexType::Owner);
  return static_cast<AbstractPropertyOwner*>(index.internalPointer());
}

void Animator::insert_track(AbstractPropertyOwner& owner, std::unique_ptr<Track> track)
{
  Track& track_ref = *track;
  const auto accelerator = this->accelerator();
  if (accelerator.contains(owner)) {
    const QModelIndex parent_index = this->index(owner);
    // the owner already has a track and is in the model. Just add the new track.
    const QModelIndex preprocessor_index = [this, &owner, &property=track->property()]() {
      Property* preprocessor = nullptr;
      for (Property* candidate : owner.properties().values()) {
        if (candidate == &property) {
          break;
        } else if (candidate->track() != nullptr) {
          preprocessor = candidate;
        }
      }

      if (preprocessor == nullptr) {
        return QModelIndex();
      } else {
        return this->index(*preprocessor);
      }
    }();

    assert(!preprocessor_index.isValid() || preprocessor_index.parent() == parent_index);
    const int row = preprocessor_index.row() + 1;
    beginInsertRows(parent_index, row, row);
    track->property().set_track(std::move(track));
    this->accelerator.invalidate();
    endInsertRows();
  } else {
    // the owner is not in the model. Add the owner.
    // it's too complicated to figure out where the owner is added.
    beginResetModel();
    track->property().set_track(std::move(track));
    this->accelerator.invalidate();
    endResetModel();
  }
  Q_EMIT track_inserted(track_ref);
}

std::unique_ptr<Track> Animator::extract_track(AbstractPropertyOwner& owner, Property& property)
{
  const QModelIndex parent = index(owner);
  const auto properties = accelerator().properties();
  const int row = std::distance(properties.begin(),
                                std::find(properties.begin(), properties.end(), &property));
  beginRemoveRows(parent, row, row);
  auto track = property.extract_track();
  this->accelerator.invalidate();
  endRemoveRows();
  Q_EMIT track_removed(*track);
  return track;
}

Track::Knot Animator::remove_knot(Track& track, int frame)
{
  Track::Knot knot = track.remove_knot(frame);
  Q_EMIT knot_removed(track, frame);
  return knot;
}

void Animator::insert_knot(Track& track, int frame, const Track::Knot& knot)
{
  track.insert_knot(frame, knot);
  Q_EMIT knot_inserted(track, frame);
}

void Animator::move_knot(Track& track, int old_frame, int new_frame)
{
  if (old_frame != new_frame) {
    track.move_knot(old_frame, new_frame);
    Q_EMIT knot_moved(track, old_frame, new_frame);
  }
}

Animator::Accelerator::Accelerator(Scene& scene) : m_scene(&scene)
{
  std::list<AbstractPropertyOwner*> owner_order;
  for (AbstractPropertyOwner* owner : animatable_owners()) {
    std::list<Property*> properties;
    for (Property* property : owner->properties().values()) {
      if (property->track() != nullptr) {
        properties.push_back(property);
        m_by_property.insert(std::pair(property, owner));
        m_properties.insert(property);
      }
    }
    if (!properties.empty()) {
      m_by_owner.insert(std::pair(owner, std::vector(properties.begin(), properties.end())));
      owner_order.push_back(owner);
    }
  }
  m_owner_order = std::vector(owner_order.begin(), owner_order.end());
}

Property* Animator::Accelerator::predecessor(AbstractPropertyOwner& owner, Property& property) const
{
  return ::predecessor<Property>(owner.properties().values(), property, [](const Property& p) {
    return p.track() != nullptr;
  });
}

AbstractPropertyOwner* Animator::Accelerator::predecessor(AbstractPropertyOwner& owner) const
{
  return ::predecessor<AbstractPropertyOwner>(m_owner_order, owner, [](const auto& o) {
    const auto properties = o.properties().values();
    return std::any_of(properties.begin(), properties.end(), [](const Property* p) {
      return p->track() != nullptr;
    });
  });
}

std::list<AbstractPropertyOwner*> Animator::Accelerator::animatable_owners() const
{
  std::list<AbstractPropertyOwner*> animatables;
  for (Object* object : m_scene->object_tree().items()) {
    animatables.push_back(object);
    for (Tag* tag : object->tags.items()) {
      animatables.push_back(tag);
    }
  }
  for (Style* style : m_scene->styles().items()) {
    animatables.push_back(style);
  }
  return animatables;
}


}  // namespace omm

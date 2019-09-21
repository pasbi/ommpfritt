#include "animation/animator.h"

#include "logging.h"
#include "serializers/abstractserializer.h"
#include "animation/track.h"
#include "scene/scene.h"
#include "aspects/propertyowner.h"
#include "tags/tag.h"
#include "renderers/style.h"
#include "scene/stylelist.h"

namespace omm
{

Animator::Animator(Scene& scene)
  : scene(scene)
  , animated_properties(*this)
{
  m_timer.setInterval(1000.0/30.0);
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(advance()));
  connect(this, SIGNAL(current_changed(int)), this, SLOT(apply()));
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
  for (auto&& [ owner, properties ] : animated_properties()) {
    Q_UNUSED(owner)
    for (Property* property : properties) {
      Track* track = property->track();
      assert(track != nullptr);
      track->apply(m_current_frame);
    }
  }
}

std::map<AbstractPropertyOwner*, std::vector<Property*>>
Animator::CachedAnimatedPropertiesGetter::compute() const
{
  std::map<AbstractPropertyOwner*, std::vector<Property*>> map;
  const auto collect_animated_properties = [&map](AbstractPropertyOwner* owner) {
    const auto animated_properties = ::filter_if(owner->properties().values(), [](Property* p) {
      return p->track() != nullptr;
    });
    if (!animated_properties.empty()) {
      map[owner] = animated_properties;
    }
  };
  for (Object* object : m_self.scene.object_tree().items()) {
    collect_animated_properties(object);
    for (Tag* tag : object->tags.items()) {
      collect_animated_properties(tag);
    }
  }
  for (Style* style : m_self.scene.styles().items()) {
    collect_animated_properties(style);
  }
  return map;

}

QModelIndex Animator::index(int row, int column, const QModelIndex &parent) const
{
  const auto animated_properties = this->animated_properties();
  if (parent.isValid()) {
    AbstractPropertyOwner* owner = this->owner(parent);
    return createIndex(row, column, animated_properties.at(owner).at(row));
  } else {
    const auto it = std::next(animated_properties.begin() , row);
    return createIndex(row, column, it->first);
  }
}

std::pair<AbstractPropertyOwner*, long> Animator::find_owner(Property& property) const
{
  using pair = std::pair<AbstractPropertyOwner*, long>;
  const auto map = animated_properties();
  for (auto&& [ owner, properties ] : map) {
    const auto it = std::find(properties.begin(), properties.end(), &property);
    if (it != properties.end()) {
      return pair(owner, std::distance(properties.begin(), it));
    }
  }
  return pair(nullptr, -1);
}

QModelIndex Animator::parent(const QModelIndex &child) const
{
  if (child.column() > 0) {
    // by convention, only first column-indices may have parents.
    return QModelIndex();
  }

  switch (index_type(child)) {
  case IndexType::None:
    [[fallthrough]];
  case IndexType::Owner:
    return QModelIndex();
  case IndexType::Property:
  {
    auto&& [ owner, row ] = find_owner(*property(child));
    return index(*owner);
  }
  default:
    Q_UNREACHABLE();
    return QModelIndex();
  }
}

int Animator::rowCount(const QModelIndex &parent) const
{
  const auto animated_properties = this->animated_properties();
  switch (index_type(parent)) {
  case IndexType::None:
    return animated_properties.size();
  case IndexType::Owner:
    LINFO << "Owner: " << owner(parent)->name();
    return animated_properties.at(owner(parent)).size();
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
  switch (role) {
  case Qt::EditRole:
    [[fallthrough]];
  case Qt::DisplayRole:
    switch (index_type(index)) {
    case IndexType::Owner:
      return QString::fromStdString(owner(index)->name());
    case IndexType::Property:
      return QString::fromStdString(property(index)->label());
    default:
      Q_UNREACHABLE();
    }
  }
  return QVariant();
}

QModelIndex Animator::index(Property& property) const
{
  auto&& [ owner, row ] = find_owner(property);
  return index(row, 0, index(*owner));
}

QModelIndex Animator::index(AbstractPropertyOwner& owner) const
{
  const auto map = animated_properties();
  const int row = std::distance(map.begin(), map.find(&owner));
  return createIndex(row, 0, &owner);
  return index(row, 0, QModelIndex());
}

Animator::IndexType Animator::index_type(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return IndexType::None;
  }

  const QObject* internal_pointer = static_cast<QObject*>(index.internalPointer());
  LINFO << "internal pointer of (" << index.row() << ", " << index.column() << "): " << internal_pointer;
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
  const auto animated_properties = this->animated_properties();
  if (animated_properties.find(&owner) != animated_properties.end()) {
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
    this->animated_properties.invalidate();
    endInsertRows();
  } else {
    // the owner is not in the model. Add the owner.
    // it's too complicated to figure out where the owner is added.
    beginResetModel();
    track->property().set_track(std::move(track));
    this->animated_properties.invalidate();
    endResetModel();
  }
}

std::unique_ptr<Track> Animator::extract_track(AbstractPropertyOwner& owner, Property& property)
{
  const auto map = animated_properties();
  const QModelIndex parent = index(owner);
  const auto properties = map.at(&owner);
  const int row =std::distance(properties.begin(),
                               std::find(properties.begin(), properties.end(), &property));
  beginRemoveRows(parent, row, row);
  auto track = property.extract_track();
  this->animated_properties.invalidate();
  endRemoveRows();
  return track;
}

void Animator::remove_key(AbstractPropertyOwner& owner, Track& track, int frame)
{
  track.remove_keyframe(frame);
}

void Animator::set_key(AbstractPropertyOwner& owner, Track& track, int frame, const variant_type& value)
{
  track.record(frame, value);
}


}  // namespace omm

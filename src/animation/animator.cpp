#include "animation/animator.h"
#include "animation/channelproxy.h"
#include "animation/track.h"
#include "animation/knot.h"
#include "aspects/propertyowner.h"
#include "logging.h"
#include "main/application.h"
#include "renderers/style.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "scene/stylelist.h"
#include "serializers/abstractserializer.h"
#include "tags/tag.h"
#include "mainwindow/iconprovider.h"
#include <functional>
#include <list>

namespace
{
const int ANIMATOR_INTERVAL_MS = static_cast<int>(1000.0 / 30.0);
}

namespace omm
{
Animator::Animator(Scene& scene) : scene(scene), accelerator(*this)
{
  m_timer.setInterval(ANIMATOR_INTERVAL_MS);
  connect(&m_timer, &QTimer::timeout, this, qOverload<>(&Animator::advance));
  connect(this, &Animator::current_changed, this, &Animator::apply);
  connect(&scene.mail_box(),
          &MailBox::property_value_changed,
          this,
          [this](AbstractPropertyOwner& owner, const QString& key, Property&) {
            if (key == AbstractPropertyOwner::NAME_PROPERTY_KEY) {
              const auto& owners = this->accelerator().owners();
              if (std::find(owners.begin(), owners.end(), &owner) != owners.end()) {
                const QModelIndex index = this->index(owner);
                Q_EMIT dataChanged(index, index, {Qt::DisplayRole});
              }
            }
          });

  connect(&scene.mail_box(),
          &MailBox::abstract_property_owner_inserted,
          this,
          &Animator::invalidate);
  connect(&scene.mail_box(),
          &MailBox::abstract_property_owner_removed,
          this,
          &Animator::invalidate);
  connect(this, &Animator::knot_moved, this, &Animator::track_changed);
  connect(this, &Animator::knot_removed, this, &Animator::track_changed);
  connect(this, &Animator::knot_inserted, this, &Animator::track_changed);
  connect(this, &Animator::track_inserted, this, &Animator::track_changed);
  connect(this, &Animator::track_removed, this, &Animator::track_changed);

  connect(this, &Animator::track_inserted, this, [this](Track& track) {
    for (std::size_t c = 0; c < track.property().n_channels(); c++) {
      m_channel_proxies.emplace(std::pair{&track, c}, std::make_unique<ChannelProxy>(track, c));
    }
  });
  connect(this, &Animator::track_removed, this, [this](Track& track) {
    for (std::size_t c = 0; c < track.property().n_channels(); c++) {
      m_channel_proxies.erase({&track, c});
    }
  });

  invalidate();
}

Animator::~Animator() = default;

void Animator::serialize(serialization::SerializerWorker& worker) const
{
  worker.sub(START_FRAME_POINTER)->set_value(m_start_frame);
  worker.sub(END_FRAME_POINTER)->set_value(m_end_frame);
  worker.sub(CURRENT_FRAME_POINTER)->set_value(m_current_frame);
  worker.sub(PLAY_MODE_POINTER)->set_value(static_cast<int>(m_play_mode));
}

void Animator::deserialize(serialization::DeserializerWorker& worker)
{
  set_start(worker.sub(START_FRAME_POINTER)->get_int());
  set_end(worker.sub(END_FRAME_POINTER)->get_int());
  m_current_frame = worker.sub(CURRENT_FRAME_POINTER)->get_int();
  m_play_mode = static_cast<PlayMode>(worker.sub(PLAY_MODE_POINTER)->get_int());
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
    Q_EMIT scene.mail_box().scene_appearance_changed();
  }
}

void Animator::set_play_mode(Animator::PlayMode mode)
{
  if (m_play_mode != mode) {
    m_play_mode = mode;
    Q_EMIT play_mode_changed(mode);
  }
}

void Animator::set_play_direction(PlayDirection direction)
{
  if (m_current_play_direction != direction) {
    m_current_play_direction = direction;
    if (direction == PlayDirection::Stopped) {
      m_timer.stop();
    } else {
      m_timer.start();
    }
    Q_EMIT play_direction_changed(direction);
  }
}

void Animator::advance()
{
  advance(m_current_play_direction);
}

void Animator::advance(PlayDirection direction)
{
  bool forward = direction == PlayDirection::Forward;
  int next = m_current_frame + (forward ? 1 : -1);
  if ((forward && next > m_end_frame) || (!forward && next < m_start_frame)) {
    switch (m_play_mode) {
    case PlayMode::Repeat:
      next = forward ? m_start_frame : m_end_frame;
      break;
    case PlayMode::PingPong:
      set_play_direction(forward ? PlayDirection::Backward : PlayDirection::Forward);
      break;
    case PlayMode::Stop:
      next = forward ? m_end_frame : m_start_frame;
      set_play_direction(PlayDirection::Stopped);
      break;
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
  scene.evaluate_tags();
}

void Animator::invalidate()
{
  beginResetModel();
  accelerator.invalidate();
  for (Property* property : accelerator().properties()) {
    for (std::size_t c = 0; c < property->n_channels(); ++c) {
      Track* track = property->track();
      m_channel_proxies.emplace(std::pair{track, c}, std::make_unique<ChannelProxy>(*track, c));
    }
  }
  endResetModel();
}

Animator::Accelerator Animator::CachedAnimatedPropertiesGetter::compute() const
{
  return Accelerator(m_self.scene);
}

QModelIndex Animator::index(int row, int column, const QModelIndex& parent) const
{
  switch (index_type(parent)) {
  case IndexType::None:
    // child of root is owner
    return index(*accelerator().owners().at(row), column);
  case IndexType::Owner:
    // child of owner is property
    return index(*accelerator().properties(*owner(parent)).at(row), column);
  case IndexType::Property:
    // child of property is channel
    return index({property(parent), row}, column);
  case IndexType::Channel:
    [[fallthrough]];
  default:
    Q_UNREACHABLE();  // channel index cannot be parent
    return {};
  }
}

QModelIndex Animator::parent(const QModelIndex& child) const
{
  switch (index_type(child)) {
  case IndexType::None:
    [[fallthrough]];
  case IndexType::Owner:
    // parent of owner is root.
    return {};
  case IndexType::Property:
    // parent of property is owner
    return index(*accelerator().owner(*property(child)));
  case IndexType::Channel:
    // parent of channel is property
    return index(channel(child).track.property());
  default:
    Q_UNREACHABLE();
    return {};
  }
}

int Animator::rowCount(const QModelIndex& parent) const
{
  switch (index_type(parent)) {
  case IndexType::None:
    return static_cast<int>(accelerator().owners().size());
  case IndexType::Owner:
    return static_cast<int>(accelerator().properties(*owner(parent)).size());
  case IndexType::Property:
    return static_cast<int>(n_channels(property(parent)->variant_value()));
  case IndexType::Channel:
    return 0;
  default:
    Q_UNREACHABLE();
    return 0;
  }
}

int Animator::columnCount(const QModelIndex&) const
{
  return COLUMN_COUNT;
}

QVariant Animator::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return {};
  }
  assert(index.column() == 0);
  switch (index_type(index)) {
  case IndexType::Owner:
    switch (role) {
    case Qt::DisplayRole:
      return owner(index)->name();
    case Qt::DecorationRole:
      return IconProvider::icon(*owner(index));
    default:
      return {};
    }
    break;
  case IndexType::Property:
    switch (role) {
    case Qt::DisplayRole:
      return property(index)->label();
    default:
      return {};
    }
  case IndexType::Channel:
    switch (role) {
    case Qt::DisplayRole: {
      const auto& c = channel(index);
      return c.track.property().channel_name(c.channel);
    }
    default:
      return {};
    }
  default:
    Q_UNREACHABLE();
  }

  Q_UNREACHABLE();
  return {};
}

Qt::ItemFlags Animator::flags(const QModelIndex&) const
{
  return Qt::ItemIsEnabled;
}

QModelIndex Animator::index(Property& property, int column) const
{
  AbstractPropertyOwner& owner = *accelerator().owner(property);
  const auto props = accelerator().properties(owner);
  const auto row = static_cast<int>(std::distance(props.begin(), std::find(props.begin(), props.end(), &property)));
  return createIndex(row, column, &property);
}

QModelIndex Animator::index(AbstractPropertyOwner& owner, int column) const
{
  const auto& owners = accelerator().owners();
  const auto row = static_cast<int>(std::distance(owners.begin(), std::find(owners.begin(), owners.end(), &owner)));
  return createIndex(row, column, &owner);
}

QModelIndex Animator::index(const std::pair<Property*, std::size_t>& channel, int column) const
{
  const auto [property, c] = channel;
  return createIndex(static_cast<int>(c), column, m_channel_proxies.at({property->track(), c}).get());
}

Animator::IndexType Animator::index_type(const QModelIndex& index)
{
  if (!index.isValid()) {
    return IndexType::None;
  }

  const auto* internal_pointer = static_cast<const QObject*>(index.internalPointer());
  if (internal_pointer->inherits(Property::staticMetaObject.className())) {
    return IndexType::Property;
  } else if (internal_pointer->inherits(AbstractPropertyOwner::staticMetaObject.className())) {
    return IndexType::Owner;
  } else if (internal_pointer->inherits(ChannelProxy::staticMetaObject.className())) {
    return IndexType::Channel;
  } else {
    Q_UNREACHABLE();
    return IndexType::None;
  }
}

Property* Animator::property(const QModelIndex& index)
{
  assert(index_type(index) == IndexType::Property);
  return static_cast<Property*>(index.internalPointer());
}

ChannelProxy& Animator::channel(const QModelIndex& index)
{
  assert(index_type(index) == IndexType::Channel);
  return *static_cast<ChannelProxy*>(index.internalPointer());
}

AbstractPropertyOwner* Animator::owner(const QModelIndex& index)
{
  assert(index_type(index) == IndexType::Owner);
  return static_cast<AbstractPropertyOwner*>(index.internalPointer());
}

void Animator::insert_track(AbstractPropertyOwner& owner, std::unique_ptr<Track> track)
{
  assert(track);
  Track& track_ref = *track;
  auto& property = track->property();
  const auto accelerator = this->accelerator();
  if (accelerator.contains(owner)) {
    const QModelIndex parent_index = this->index(owner);
    // the owner already has a track and is in the model. Just add the new track.
    const QModelIndex preprocessor_index = [this, &owner, &property = track->property()]() {
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
    property.set_track(std::move(track));
    this->accelerator.invalidate();
    endInsertRows();
  } else {
    // the owner is not in the model. Add the owner.
    // it's too complicated to figure out where the owner is added.
    beginResetModel();
    property.set_track(std::move(track));
    this->accelerator.invalidate();
    endResetModel();
  }
  Q_EMIT track_inserted(track_ref);
}

std::unique_ptr<Track> Animator::extract_track(Property& property)
{
  // complicated model changes may be caused if property is the last track in the relevant owner.
  beginResetModel();
  auto track = property.extract_track();
  if (track) {
    this->accelerator.invalidate();
    endResetModel();
    Q_EMIT track_removed(*track);
  } else {
    endResetModel();
  }
  return track;
}

std::unique_ptr<Knot> Animator::remove_knot(Track& track, int frame)
{
  auto knot = track.remove_knot(frame);
  Q_EMIT knot_removed(track, frame);
  return knot;
}

void Animator::insert_knot(Track& track, int frame, std::unique_ptr<Knot> knot)
{
  track.insert_knot(frame, std::move(knot));
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

const std::vector<Property*>& Animator::Accelerator::properties(AbstractPropertyOwner& owner) const
{
  return m_by_owner.at(&owner);
}

bool Animator::Accelerator::contains(AbstractPropertyOwner& owner) const
{
  return m_by_owner.find(&owner) != m_by_owner.end();
}

AbstractPropertyOwner* Animator::Accelerator::owner(Property& property) const
{
  return m_by_property.at(&property);
}

std::list<AbstractPropertyOwner*> Animator::Accelerator::animatable_owners() const
{
  auto owners = m_scene->property_owners();
  return {owners.begin(), owners.end()};
}

}  // namespace omm

#pragma once

#include "animation/track.h"
#include "cachedgetter.h"
#include <QAbstractItemModel>
#include <QObject>
#include <QTimer>
#include <memory>
#include <set>

namespace omm
{
class AbstractPropertyOwner;
class Track;
class Scene;
class Property;
class ChannelProxy;

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class Animator : public QAbstractItemModel
{
  Q_OBJECT
public:
  enum class PlayMode { Repeat = 0, PingPong = 1, Stop = 2 };
  enum class PlayDirection { Forward, Backward, Stopped };
  explicit Animator(Scene& scene);
  ~Animator() override;
  Animator(Animator&&) = delete;
  Animator(const Animator&) = delete;
  Animator& operator=(Animator&&) = delete;
  Animator& operator=(const Animator&) = delete;
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);

  int start() const
  {
    return m_start_frame;
  }
  int end() const
  {
    return m_end_frame;
  }
  int current() const
  {
    return m_current_frame;
  }
  Animator::PlayDirection play_direction() const
  {
    return m_current_play_direction;
  }
  Animator::PlayMode play_mode() const
  {
    return m_play_mode;
  }

  static constexpr auto START_FRAME_POINTER = "start-frame";
  static constexpr auto END_FRAME_POINTER = "end-frame";
  static constexpr auto CURRENT_FRAME_POINTER = "current-frame";
  static constexpr auto PLAY_MODE_POINTER = "play-mode";
  static constexpr auto COLUMN_COUNT = 1;

  Scene& scene;

public:
  void set_start(int start);
  void set_end(int end);
  void set_current(int current);
  void set_play_direction(omm::Animator::PlayDirection direction);
  void set_play_mode(omm::Animator::PlayMode mode);
  void advance();
  void advance(omm::Animator::PlayDirection direction);
  void apply();

  /**
   * @brief invalidate invalidates the cache and the model indices.
   *  call this if the you have changed the model other than through the members
   * provided by this class.
   */
  void invalidate();

Q_SIGNALS:
  void start_changed(int);
  void end_changed(int);
  void current_changed(int);
  void play_direction_changed(omm::Animator::PlayDirection direction);
  void play_mode_changed(omm::Animator::PlayMode mode);
  void track_changed(omm::Track&);
  void track_inserted(omm::Track&);
  void track_removed(omm::Track&);
  void knot_inserted(omm::Track&, int);
  void knot_removed(omm::Track&, int);
  void knot_moved(omm::Track&, int, int);

  // == ItemModel
public:
  enum class IndexType { Owner = 0, Property = 1, Channel = 2, None };
  static constexpr std::underlying_type_t<Qt::ItemDataRole> type_role = Qt::UserRole + 1;
  static constexpr std::underlying_type_t<Qt::ItemDataRole> channel_role = Qt::UserRole + 2;
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex&) const override;
  QModelIndex index(Property& property, int column = 0) const;
  QModelIndex index(const std::pair<Property*, std::size_t>& channel, int column = 0) const;
  QModelIndex index(AbstractPropertyOwner& owner, int column = 0) const;
  static IndexType index_type(const QModelIndex& index);
  static Property* property(const QModelIndex& index);
  static ChannelProxy& channel(const QModelIndex& index);
  static AbstractPropertyOwner* owner(const QModelIndex& index);

  template<typename F> auto visit_item(const QModelIndex& index, F&& f)
  {
    if (index.isValid()) {
      switch (index_type(index)) {
      case omm::Animator::IndexType::Owner:
        return f(*owner(index));
      case omm::Animator::IndexType::Property:
        return f(*property(index));
      case omm::Animator::IndexType::Channel:
        return f(channel(index));
      case omm::Animator::IndexType::None:
        Q_UNREACHABLE();
      }
    }
    Q_UNREACHABLE();
    return f(*owner(index));  // this does not make sence, only silence compiler warning.
  }

  // == access tracks
public:
  class Accelerator
  {
  public:
    Accelerator() = default;
    explicit Accelerator(Scene& scene);
    [[nodiscard]] const std::vector<AbstractPropertyOwner*>& owners() const
    {
      return m_owner_order;
    }
    const std::vector<Property*>& properties(AbstractPropertyOwner& owner) const;
    bool contains(AbstractPropertyOwner& owner) const;
    [[nodiscard]] const std::set<Property*>& properties() const
    {
      return m_properties;
    }
    AbstractPropertyOwner* owner(Property& property) const;

  private:
    Scene* m_scene{};
    [[nodiscard]] std::list<AbstractPropertyOwner*> animatable_owners() const;
    std::map<AbstractPropertyOwner*, std::vector<Property*>> m_by_owner;
    std::set<Property*> m_properties;
    std::map<Property*, AbstractPropertyOwner*> m_by_property;
    std::vector<AbstractPropertyOwner*> m_owner_order;
  };
  class CachedAnimatedPropertiesGetter : public CachedGetter<Accelerator, Animator>
  {
    using CachedGetter::CachedGetter;
    Accelerator compute() const override;
  } accelerator;

  // == modify tracks
public:
  void insert_track(AbstractPropertyOwner& owner, std::unique_ptr<Track> track);
  std::unique_ptr<Track> extract_track(Property& property);
  std::unique_ptr<Knot> remove_knot(Track& track, int frame);
  void insert_knot(Track& track, int frame, std::unique_ptr<Knot> knot);
  void move_knot(Track& track, int old_frame, int new_frame);

  static constexpr int DEFAULT_START_FRAME = 1;
  static constexpr int DEFAULT_END_FRAME = 100;
  static constexpr int DEFAULT_CURRENT_FRAME = 1;

private:
  int m_start_frame = DEFAULT_START_FRAME;
  int m_end_frame = DEFAULT_END_FRAME;
  int m_current_frame = DEFAULT_CURRENT_FRAME;
  PlayDirection m_current_play_direction = PlayDirection::Stopped;
  QTimer m_timer;
  PlayMode m_play_mode = PlayMode::Repeat;

  // === channel pointers
  std::map<std::pair<Track*, std::size_t>, std::unique_ptr<ChannelProxy>> m_channel_proxies;
};

}  // namespace omm

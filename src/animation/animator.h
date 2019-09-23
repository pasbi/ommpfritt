#pragma once

#include "aspects/serializable.h"
#include <QObject>
#include <QTimer>
#include <set>
#include <memory>
#include "cachedgetter.h"
#include <QAbstractItemModel>
#include "variant.h"
#include "aspects/autoconnectiondeleter.h"

namespace omm
{

class AbstractPropertyOwner;
class Track;
class Scene;
class Property;

class Animator : public QAbstractItemModel, public Serializable, public AutoConnectionDeleter
{
  Q_OBJECT
public:
  enum class PlayMode { Repeat, Stop };
  explicit Animator(Scene& scene);
  ~Animator();
  void serialize(AbstractSerializer&, const Pointer&) const override;
  void deserialize(AbstractDeserializer&, const Pointer&) override;

  int start() const { return m_start_frame; }
  int end() const { return m_end_frame; }
  int current() const { return m_current_frame; }

  static constexpr auto START_FRAME_POINTER = "start-frame";
  static constexpr auto END_FRAME_POINTER = "end-frame";
  static constexpr auto CURRENT_FRAME_POINTER = "current-frame";

  Scene& scene;

public Q_SLOTS:
  void set_start(int start);
  void set_end(int end);
  void set_current(int current);
  void toggle_play_pause(bool play);
  void advance();
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
  void play_pause_toggled(bool);
  void track_changed(Track&);
  void track_inserted(Track&);
  void track_removed(Track&);
  void key_inserted(Track&, int);
  void key_removed(Track&, int);
  void key_moved(Track&, int, int);

  // == ItemModel
public:
  enum class IndexType { Property, Owner, None };
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QModelIndex index(Property& property, int column = 0) const;
  QModelIndex index(AbstractPropertyOwner& owner, int column = 0) const;
  IndexType index_type(const QModelIndex& index) const;
  Property* property(const QModelIndex& index) const;
  AbstractPropertyOwner* owner(const QModelIndex& index) const;

  // == access tracks
public:
  class Accelerator
  {
  public:
    Accelerator() = default;
    explicit Accelerator(Scene& scene);
    const std::vector<AbstractPropertyOwner*>& owners() const { return m_owner_order; }
    const std::vector<Property*>& properties(AbstractPropertyOwner& owner) const
    { return m_by_owner.at(&owner); }
    bool contains(AbstractPropertyOwner& owner) const
    { return m_by_owner.find(&owner) != m_by_owner.end(); }
    Property* predecessor(AbstractPropertyOwner& owner, Property& property) const;
    AbstractPropertyOwner* predecessor(AbstractPropertyOwner& owner) const;
    const std::set<Property*>& properties() const { return m_properties; }
    AbstractPropertyOwner* owner(Property& property) const { return m_by_property.at(&property); }
  private:
    Scene* m_scene;
    std::list<AbstractPropertyOwner*> animatable_owners() const;
    std::map<AbstractPropertyOwner*, std::vector<Property*>> m_by_owner;
    std::set<Property*> m_properties;
    std::map<Property*, AbstractPropertyOwner*> m_by_property;
    std::vector<AbstractPropertyOwner*> m_owner_order;
  };
  class CachedAnimatedPropertiesGetter
    : public CachedGetter<Accelerator, Animator>
  {
    using CachedGetter::CachedGetter;
    Accelerator compute() const override;
  } accelerator;

  // == modify tracks
public:
  void insert_track(AbstractPropertyOwner& owner, std::unique_ptr<Track> track);
  std::unique_ptr<Track> extract_track(AbstractPropertyOwner& owner, Property& property);
  void remove_key(AbstractPropertyOwner& owner, Track& track, int frame);
  void set_key(AbstractPropertyOwner& owner, Track& track, int frame, const variant_type& value);
  void move_key(AbstractPropertyOwner& owner, Track& track, int old_frame, int new_frame);

private:
  int m_start_frame = 1;
  int m_end_frame = 100;
  int m_current_frame = 1;
  bool m_is_playing = false;
  QTimer m_timer;
  PlayMode m_play_mode = PlayMode::Repeat;
};

}  // namespace omm

#pragma once

#include "aspects/serializable.h"
#include <QObject>
#include <QTimer>
#include <set>
#include <memory>
#include "scene/cachedgetter.h"
#include <QAbstractItemModel>
#include "variant.h"

namespace omm
{

class AbstractPropertyOwner;
class Track;
class Scene;
class Property;

class Animator : public QAbstractItemModel, public Serializable
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

Q_SIGNALS:
  void start_changed(int);
  void end_changed(int);
  void current_changed(int);
  void play_pause_toggled(bool);

  // == ItemModel
public:
  enum class IndexType { Property, Owner, None };
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QModelIndex index(Property& property) const;
  QModelIndex index(AbstractPropertyOwner& owner) const;
  IndexType index_type(const QModelIndex& index) const;
  Property* property(const QModelIndex& index) const;
  AbstractPropertyOwner* owner(const QModelIndex& index) const;

  // == access tracks
public:
  class CachedAnimatedPropertiesGetter
    : public CachedGetter<std::map<AbstractPropertyOwner*, std::vector<Property*>>, Animator>
  {
    using CachedGetter::CachedGetter;
    std::map<AbstractPropertyOwner*, std::vector<Property*>> compute() const override;
  } animated_properties;

  // == modify tracks
public:
  void insert_track(AbstractPropertyOwner& owner, std::unique_ptr<Track> track);
  std::unique_ptr<Track> extract_track(AbstractPropertyOwner& owner, Property& property);
  void remove_key(AbstractPropertyOwner& owner, Track& track, int frame);
  void set_key(AbstractPropertyOwner& owner, Track& track, int frame, const variant_type& value);

private:
  int m_start_frame = 1;
  int m_end_frame = 100;
  int m_current_frame = 1;
  bool m_is_playing = false;
  QTimer m_timer;
  PlayMode m_play_mode = PlayMode::Repeat;
  std::pair<AbstractPropertyOwner*, long> find_owner(Property& property) const;
};

}  // namespace omm

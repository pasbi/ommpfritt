#pragma once

#include "aspects/serializable.h"
#include <QObject>
#include <QTimer>
#include <set>
#include <memory>

namespace omm
{

class AbstractPropertyOwner;
class Track;

class Animator : public QObject, public Serializable
{
  Q_OBJECT
public:
  enum class PlayMode { Repeat, Stop };
  explicit Animator();
  ~Animator();
  void serialize(AbstractSerializer&, const Pointer&) const override;
  void deserialize(AbstractDeserializer&, const Pointer&) override;

  int start() const { return m_start_frame; }
  int end() const { return m_end_frame; }
  int current() const { return m_current_frame; }

  static constexpr auto START_FRAME_POINTER = "start-frame";
  static constexpr auto END_FRAME_POINTER = "end-frame";
  static constexpr auto CURRENT_FRAME_POINTER = "current-frame";
  static constexpr auto TRACKS_POINTER = "tracks";

  /**
   * @brief track returns a pointer to the track corresponding to the property represented by the
   *  given key and owner or nullptr if no such track exists.
   *  if there is no such track
   * @param owner
   * @param property_key
   * @return
   */
  Track* track(AbstractPropertyOwner& owner, const std::string& property_key) const;
  Track* create_track(AbstractPropertyOwner& owner, const std::string& property_key);
  std::unique_ptr<Track> extract_track(AbstractPropertyOwner& owner,
                                       const std::string& property_key);

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
  void tracks_changed();

private:
  int m_start_frame = 1;
  int m_end_frame = 100;
  int m_current_frame = 1;
  bool m_is_playing = false;
  QTimer m_timer;
  PlayMode m_play_mode = PlayMode::Repeat;

  std::set<std::unique_ptr<Track>> m_tracks;

};

}  // namespace omm

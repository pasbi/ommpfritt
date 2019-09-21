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
  const auto apply_tracks = [this](const AbstractPropertyOwner* owner) {
    for (Property* property : owner->properties().values()) {
      if (Track* track = property->track(); track != nullptr) {
        track->apply(m_current_frame);
      }
    }
  };
  for (Object* object : scene.object_tree().items()) {
    apply_tracks(object);
    for (Tag* tag : object->tags.items()) {
      return apply_tracks(tag);
    }
  }
  for (Style* style : scene.styles().items()) {
    apply_tracks(style);
  }
}

}  // namespace omm

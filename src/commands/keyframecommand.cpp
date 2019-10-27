#include "commands/keyframecommand.h"
#include "animation/track.h"
#include "properties/property.h"
#include "animation/animator.h"

namespace
{

std::map<omm::Property*, omm::Track::Knot>
collect_knots(const std::set<omm::Property*>& properties, int frame)
{
  std::map<omm::Property*, omm::Track::Knot> map;
  for (auto&& property : properties) {
    if (property->track() != nullptr) {
      // don't add keyframes to non-existing tracks.
      // You must make sure that the track exists beforehand.
      omm::Track::Knot knot = property->track()->knot_at(frame);
      map.insert(std::pair(property, knot));
    }
  }
  return map;
}

std::map<omm::Property*, omm::Track::Knot>
create_knots(const std::set<omm::Property*>& properties, int frame)
{
  std::map<omm::Property*, omm::Track::Knot> map;
  for (auto&& property : properties) {
    if (property->track() != nullptr) {
      Q_UNUSED(frame)
      omm::Track::Knot knot(property->variant_value());
      map.insert(std::pair(property, knot));
    }
  }
  return map;
}

}  // namespace

namespace omm
{

KeyframeCommand::KeyframeCommand(Animator& animator, const QString& label, int frame,
                                 const std::map<Property*, Track::Knot>& values)
  : Command(label), m_animator(animator), m_frame(frame), m_knots(values)
{ }

void KeyframeCommand::insert()
{
  for (auto&& [ property, knot ] : m_knots) {
    Track* track = property->track();
    assert(track != nullptr);
    m_animator.insert_knot(*track, m_frame, knot);
  }
}

void KeyframeCommand::remove()
{
  for (auto&& [ property, value ] : m_knots) {
    Track* track = property->track();
    assert(track != nullptr);
    m_animator.remove_knot(*track, m_frame);
  }
}

InsertKeyframeCommand::
InsertKeyframeCommand(Animator& animator, int frame,
                      const std::set<Property*>& properties)
  : KeyframeCommand(animator, QObject::tr("Create Keyframe"), frame,
                    create_knots(properties, frame))
{

}

RemoveKeyframeCommand::
RemoveKeyframeCommand(Animator& animator, int frame,
                      const std::set<Property*>& values)
  : KeyframeCommand(animator, QObject::tr("Remove Keyframe"),
                    frame, collect_knots(values, frame))
{

}

MoveKeyFrameCommand::MoveKeyFrameCommand(Animator& animator, Property& property,
                                         std::set<int> old_frames, int shift)
  : Command(QObject::tr("Move Keyframes"))
  , m_animator(animator)
  , m_property(property)
  , m_old_frames(old_frames)
  , m_shift(shift)
{
}

void MoveKeyFrameCommand::undo()
{
  // shift may be null if the command is merge from multiple commands which add up to zero.
  if (m_shift != 0) {
    shift_keyframes(true);

    Track* track = m_property.track();
    for (auto&& [frame, knot] : m_removed_values) {
      m_animator.insert_knot(*track, frame, knot);
    }
  }
}

void MoveKeyFrameCommand::redo()
{
  // shift may be null if the command is merge from multiple commands which add up to zero.
  if (m_shift != 0) {
    Track* track = m_property.track();
    for (int frame : m_old_frames) {
      int new_frame = frame + m_shift;
      // do not remove frames that are being shifted itself.
      // the `shift_keyframes`-methods handles such cases.
      if (track->has_keyframe(new_frame) && !::contains(m_old_frames, new_frame)) {
        const Track::Knot knot = m_animator.remove_knot(*track, new_frame);
        m_removed_values.insert(std::pair(new_frame, knot));
      }
    }

    shift_keyframes(false);
  }
}

bool MoveKeyFrameCommand::mergeWith(const QUndoCommand* other)
{
  const MoveKeyFrameCommand& other_mkfc = static_cast<const MoveKeyFrameCommand&>(*other);
  std::set<int> new_frames;
  for (int frame : m_old_frames) {
    new_frames.insert(frame + m_shift);
  }
  if (&m_property == &other_mkfc.m_property && new_frames == other_mkfc.m_old_frames) {
    m_shift = m_shift + other_mkfc.m_shift;
    for (auto&& [frame, knot] : other_mkfc.m_removed_values) {
      m_removed_values.insert(std::pair(frame, knot));
    }
    return true;
  } else {
    return false;
  }
}

void MoveKeyFrameCommand::shift_keyframes(bool invert)
{
  const auto move_knot = [this, invert, &track=*m_property.track()](int frame) {
    if (invert) {
      m_animator.move_knot(*m_property.track(), frame + m_shift, frame);
    } else {
      m_animator.move_knot(*m_property.track(), frame, frame + m_shift);
    }
  };

  if (m_shift < 0 == invert) {
    for (auto it = m_old_frames.rbegin(); it != m_old_frames.rend(); ++it) {
      move_knot(*it);
    }
  } else if (m_shift > 0 == invert) {
    for (auto it = m_old_frames.begin(); it != m_old_frames.end(); ++it) {
      move_knot(*it);
    }
  } else {
    Q_UNREACHABLE();
  }
}

}  // namespace omm

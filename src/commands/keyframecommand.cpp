#include "commands/keyframecommand.h"

#include "animation/animator.h"
#include "animation/knot.h"
#include "animation/track.h"
#include "properties/property.h"
#include <utility>

namespace
{
std::map<omm::Property*, omm::Knot*>
collect_knots(const std::set<omm::Property*>& properties, int frame)
{
  std::map<omm::Property*, omm::Knot*> map;
  for (auto&& property : properties) {
    if (property->track() != nullptr) {
      // don't add keyframes to non-existing tracks.
      // You must make sure that the track exists beforehand.
      omm::Knot& knot = property->track()->knot(frame);
      map.emplace(property, &knot);
    }
  }
  return map;
}

std::map<omm::Property*, std::unique_ptr<omm::Knot>>
create_knots(const std::set<omm::Property*>& properties, int frame)
{
  std::map<omm::Property*, std::unique_ptr<omm::Knot>> map;
  for (auto&& property : properties) {
    if (property->track() != nullptr) {
      Q_UNUSED(frame)
      auto knot = std::make_unique<omm::Knot>(property->variant_value());
      map.emplace(property, std::move(knot));
    }
  }
  return map;
}

std::map<omm::Property*, omm::Knot*>
collect_refs(const std::map<omm::Property*, std::unique_ptr<omm::Knot>>& owns)
{
  std::map<omm::Property*, omm::Knot*> refs;
  for (auto&& [property, own] : owns) {
    refs.emplace(property, own.get());
  }
  return refs;
}

}  // namespace

namespace omm
{
KeyFrameCommand::KeyFrameCommand(Animator& animator,
                                 const QString& label,
                                 int frame,
                                 const std::map<Property*, Knot*>& refs,
                                 std::map<Property*, std::unique_ptr<Knot>>&& owns)
    : Command(label), m_animator(animator), m_frame(frame), m_refs(refs)
{
  for (auto&& [property, knot] : owns) {
    m_owns.emplace(property, std::move(knot));
  }
}

KeyFrameCommand::KeyFrameCommand(Animator& animator,
                                 const QString& label,
                                 int frame,
                                 std::map<Property*, std::unique_ptr<Knot> >&& owns)
    : KeyFrameCommand(animator, label, frame, collect_refs(owns))
{
  for (auto&& [property, knot] : owns) {
    m_owns.emplace(property, std::move(knot));
  }
}

void KeyFrameCommand::insert()
{
  for (auto&& [property, knot] : m_owns) {
    Track* track = property->track();
    assert(track != nullptr);
    m_animator.insert_knot(*track, m_frame, std::move(knot));
  }
  m_owns.clear();
}

void KeyFrameCommand::remove()
{
  for (auto&& [property, value] : m_refs) {
    Track* track = property->track();
    assert(track != nullptr);
    auto knot = m_animator.remove_knot(*track, m_frame);
    m_owns.emplace(property, std::move(knot));
  }
}

InsertKeyFrameCommand::InsertKeyFrameCommand(Animator& animator,
                                             int frame,
                                             const std::set<Property*>& properties)
    : KeyFrameCommand(animator,
                      QObject::tr("Create Keyframe"),
                      frame,
                      create_knots(properties, frame))
{
}

RemoveKeyFrameCommand::RemoveKeyFrameCommand(Animator& animator,
                                             int frame,
                                             const std::set<Property*>& values)
    : KeyFrameCommand(animator,
                      QObject::tr("Remove Keyframe"),
                      frame,
                      collect_knots(values, frame),
                      std::map<Property*, std::unique_ptr<Knot>>())
{
}

MoveKeyFrameCommand::MoveKeyFrameCommand(Animator& animator,
                                         Property& property,
                                         const std::set<int>& old_frames,
                                         int shift)
    : Command(QObject::tr("Move Keyframes")), m_animator(animator), m_property(property),
      m_old_frames(old_frames), m_shift(shift)
{
}

void MoveKeyFrameCommand::undo()
{
  // shift may be null if the command is merge from multiple commands which add up to zero.
  if (m_shift != 0) {
    shift_keyframes(true);

    Track* track = m_property.track();
    for (auto&& [frame, knot] : m_removed_values) {
      m_animator.insert_knot(*track, frame, std::move(knot));
    }
    m_removed_values.clear();
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
      if (track->has_keyframe(new_frame) && !m_old_frames.contains(new_frame)) {
        auto knot = m_animator.remove_knot(*track, new_frame);
        m_removed_values.insert({new_frame, std::move(knot)});
      }
    }

    shift_keyframes(false);
  }
}

bool MoveKeyFrameCommand::mergeWith(const QUndoCommand* other)
{
  const auto& other_mkfc = dynamic_cast<const MoveKeyFrameCommand&>(*other);
  std::set<int> new_frames;
  for (int frame : m_old_frames) {
    new_frames.insert(frame + m_shift);
  }
  if (&m_property == &other_mkfc.m_property && new_frames == other_mkfc.m_old_frames) {
    m_shift = m_shift + other_mkfc.m_shift;
    for (auto&& [frame, knot] : other_mkfc.m_removed_values) {
      m_removed_values.insert({frame, std::move(knot)});
    }
    other_mkfc.m_removed_values.clear();
    return true;
  } else {
    return false;
  }
}

void MoveKeyFrameCommand::shift_keyframes(bool invert)
{
  const auto move_knot = [this, invert, &track = *m_property.track()](int frame) {
    if (invert) {
      m_animator.move_knot(*m_property.track(), frame + m_shift, frame);
    } else {
      m_animator.move_knot(*m_property.track(), frame, frame + m_shift);
    }
  };

  if ((m_shift < 0) == invert) {
    for (auto it = m_old_frames.rbegin(); it != m_old_frames.rend(); ++it) {
      move_knot(*it);
    }
  } else if ((m_shift > 0) == invert) {
    for (int m_old_frame : m_old_frames) {
      move_knot(m_old_frame);
    }
  } else {
    Q_UNREACHABLE();
  }
}

ChangeKeyFrameCommand ::ChangeKeyFrameCommand(int frame,
                                              Property& property,
                                              std::unique_ptr<Knot> new_value)
    : Command(QObject::tr("Change Keyframes")), m_frame(frame), m_property(property),
      m_other_value(std::move(new_value))
{
}

bool ChangeKeyFrameCommand::mergeWith(const QUndoCommand* other)
{
  const auto& other_ckfc = dynamic_cast<const ChangeKeyFrameCommand&>(*other);
  return &other_ckfc.m_property == &m_property;
}

void ChangeKeyFrameCommand::swap()
{
  m_property.track()->knot(m_frame).swap(*m_other_value);
}

}  // namespace omm

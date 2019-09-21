#include "commands/keyframecommand.h"
#include "animation/track.h"
#include "properties/property.h"

namespace
{

std::map<omm::AbstractPropertyOwner*, std::pair<omm::Property*, omm::variant_type>>
collect_values(const std::map<omm::AbstractPropertyOwner*, omm::Property*>& properties)
{
  std::map<omm::AbstractPropertyOwner*, std::pair<omm::Property*, omm::variant_type>> map;
  for (auto&& [owner, property] : properties) {
    if (property->track() != nullptr) {
      // don't add keyframes to non-existing tracks.
      // You must make sure that the track exists beforehand.
      map.insert(std::pair(owner, std::pair(property, property->variant_value())));
    }
  }
  return map;
}

}  // namespace

namespace omm
{

KeyframeCommand::KeyframeCommand(Animator& animator, const std::string& label, int frame,
                                 const std::map<AbstractPropertyOwner*, std::pair<Property*, variant_type>> &values)
  : Command(label), m_animator(animator), m_frame(frame), m_values(values)
{ }

void KeyframeCommand::insert()
{
  for (auto&& [ owner, pair ] : m_values) {
    auto&& [ property, value ] = pair;
    Track* track = property->track();
    assert(track != nullptr);
    track->record(m_frame, value);
  }
}

void KeyframeCommand::remove()
{
  for (auto&& [ owner, pair ] : m_values) {
    Q_UNUSED(owner);
    Property* property = pair.first;
    Track* track = property->track();
    assert(track != nullptr);
    track->remove_keyframe(m_frame);
  }
}

InsertKeyframeCommand::
InsertKeyframeCommand(Animator& animator, int frame,
                      const std::map<AbstractPropertyOwner*, std::pair<Property*, variant_type> >& values)
  : KeyframeCommand(animator, QObject::tr("Create Keyframe").toStdString(), frame, values)
{

}

InsertKeyframeCommand::
InsertKeyframeCommand(Animator& animator, int frame,
                      const std::map<AbstractPropertyOwner*, Property*>& properties)
  : InsertKeyframeCommand(animator, frame, collect_values(properties))
{

}

RemoveKeyframeCommand::
RemoveKeyframeCommand(Animator& animator, int frame,
                      const std::map<AbstractPropertyOwner*, Property*>& properties)
  : KeyframeCommand(animator, QObject::tr("Remove Keyframe").toStdString(), frame, collect_values(properties))
{

}



}  // namespace omm

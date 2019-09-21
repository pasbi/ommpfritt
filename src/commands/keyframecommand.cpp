#include "commands/keyframecommand.h"
#include "animation/track.h"
#include "properties/property.h"

namespace
{

std::map<omm::Property*, omm::variant_type>
collect_values(const std::set<omm::Property*>& properties)
{
  std::map<omm::Property*, omm::variant_type> map;
  for (omm::Property* property : properties) {
    if (property->track() != nullptr) {
      // don't add keyframes to non-existing tracks.
      // You must make sure that the track exists beforehand.
      map.insert(std::pair(property, property->variant_value()));
    }
  }
  return map;
}

}  // namespace

namespace omm
{

KeyframeCommand::KeyframeCommand(const std::string& label, int frame,
                                 const std::map<Property*, variant_type> &values)
  : Command(label), m_frame(frame), m_values(values)
{ }

void KeyframeCommand::insert()
{
  for (auto&& [ property, value ] : m_values) {
    Track* track = property->track();
    assert(track != nullptr);
    track->record(m_frame, value);
  }
}

void KeyframeCommand::remove()
{
  for (auto&& [ property, value ] : m_values) {
    Track* track = property->track();
    assert(track != nullptr);
    track->remove_keyframe(m_frame);
  }
}

RemoveKeyframeCommand
::RemoveKeyframeCommand(int frame, const std::set<Property*> &properties)
  : KeyframeCommand(QObject::tr("Remove Keyframe").toStdString(), frame, collect_values(properties))
{ }

InsertKeyframeCommand
::InsertKeyframeCommand(int frame, const std::map<Property*, variant_type> &values)
  : KeyframeCommand(QObject::tr("Create Keyframe").toStdString(), frame, values)
{ }

InsertKeyframeCommand
::InsertKeyframeCommand(int frame, const std::set<Property*> &properties)
  : InsertKeyframeCommand(frame, collect_values(properties))
{ }



}  // namespace omm

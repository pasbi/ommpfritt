#include "commands/keyframecommand.h"
#include "aspects/propertyowner.h"
#include "animation/animator.h"
#include "animation/track.h"

namespace
{

std::map<omm::AbstractPropertyOwner*, omm::variant_type>
collect_values(omm::Animator& animator,
               const std::set<omm::AbstractPropertyOwner*>& owners, const std::string& property_key)
{
  std::map<omm::AbstractPropertyOwner*, omm::variant_type> map;
  for (omm::AbstractPropertyOwner* owner : owners) {
    if (animator.track(*owner, property_key) != nullptr) {
      // don't add keyframes to non-existing tracks.
      // You must make sure that the track exists beforehand.
      map.insert(std::pair(owner, owner->property(property_key)->variant_value()));
    }
  }
  return map;
}

}  // namespace

namespace omm
{

KeyframeCommand::KeyframeCommand(const std::string& label, Animator &animator, int frame,
                                 const std::map<AbstractPropertyOwner*, variant_type> &values,
                                 const std::string &property_key)
  : Command(label), m_animator(animator), m_frame(frame)
  , m_values(values), m_property_key(property_key)
{
}

void KeyframeCommand::insert()
{
  for (auto&& [ owner, value ] : m_values) {
    m_animator.track(*owner, m_property_key)->record(m_frame, value);
  }
}

void KeyframeCommand::remove()
{
  for (auto&& [ owner, value ] : m_values) {
    m_animator.track(*owner, m_property_key)->remove_keyframe(m_frame);
  }
}

RemoveKeyframeCommand
::RemoveKeyframeCommand(Animator &animator, int frame,
                        const std::set<AbstractPropertyOwner *> &owners,
                        const std::string &property_key)
  : KeyframeCommand(QObject::tr("Remove Keyframe").toStdString(), animator, frame,
                    collect_values(animator, owners, property_key), property_key)
{

}

InsertKeyframeCommand
::InsertKeyframeCommand(Animator &animator, int frame,
                        const std::map<AbstractPropertyOwner *, variant_type> &values,
                        const std::string &property_key)
  : KeyframeCommand(QObject::tr("Create Keyframe").toStdString(), animator, frame,
                    values, property_key)
{

}

InsertKeyframeCommand
::InsertKeyframeCommand(Animator &animator, int frame,
                        const std::set<AbstractPropertyOwner *> &owners,
                        const std::string &property_key)
  : InsertKeyframeCommand(animator, frame,
                          collect_values(animator, owners, property_key), property_key)
{

}



}  // namespace omm

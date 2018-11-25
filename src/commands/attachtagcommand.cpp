#include "commands/attachtagcommand.h"

#include <glog/logging.h>

#include "tags/tag.h"
#include "objects/object.h"
#include "scene/scene.h"

namespace omm
{

AttachTagCommand::AttachTagCommand(Scene& scene, std::unique_ptr<Tag> tag)
  : Command(QObject::tr("Attach Tag").toStdString())
  , m_scene(scene)
  , m_contextes( ::transform<TagContext, std::vector>( scene.selected_objects(),
                                                       [&tag](Object* object) {
    return TagContext(*object, tag->copy());
  }))
{
}

void AttachTagCommand::undo()
{
  for (auto& context : m_contextes) {
    context.detach_tag();
  }
  // important. else, handle or property manager might point to dangling objects
  m_scene.clear_selection();
}

void AttachTagCommand::redo()
{
  for (auto& context : m_contextes) {
    context.attach_tag();
  }
}

AttachTagCommand::TagContext::TagContext(Object& object, std::unique_ptr<Tag> tag)
  : m_owner(&object)
  , m_owned(std::move(tag))
  , m_reference(&*m_owned)
{
}

void AttachTagCommand::TagContext::attach_tag()
{
  if (!m_owned) {
    LOG(FATAL) << "Command cannot give away non-owned object.";
  } else {
    m_owner->add_tag(std::move(m_owned));
  }
}

void AttachTagCommand::TagContext::detach_tag()
{
  if (m_owned) {
    LOG(FATAL) << "Command already owns object. Obtaining ownership again is absurd.";
  } else {
    m_owned = m_owner->remove_tag(*m_reference);
  }
}

}  // namespace omm

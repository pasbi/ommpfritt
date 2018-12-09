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
  , m_contextes( ::transform<TagContext, std::vector>( scene.object_tree.selected_items(),
                                                       [&tag](Object* object) {
    return TagContext(*object, tag->copy());
  }))
{
}

void AttachTagCommand::undo()
{
  for (auto& context : m_contextes) {
    if (context.owned) {
      LOG(FATAL) << "Command already owns object. Obtaining ownership again is absurd.";
    } else {
      auto& tag = *context.reference;
      tag.set_owner(nullptr);
      context.owned = context.tag_owner->tags.remove(tag);
    }
  }
  // important. else, handle or property manager might point to dangling objects
  m_scene.clear_selection();
}

void AttachTagCommand::redo()
{
  for (auto& context : m_contextes) {
    if (!context.owned) {
      LOG(FATAL) << "Command cannot give away non-owned object.";
    } else {
      context.reference->set_owner(context.tag_owner);
      context.tag_owner->tags.insert(std::move(context.owned));
    }
  }
}

AttachTagCommand::TagContext::TagContext(Object& object, std::unique_ptr<Tag> tag)
  : tag_owner(&object)
  , owned(std::move(tag))
  , reference(owned.get())
{
  // TODO use MaybeOwner
}

}  // namespace omm

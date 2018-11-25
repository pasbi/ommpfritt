#pragma once

#include <memory>
#include <vector>
#include "commands/command.h"

namespace omm
{

class Tag;
class Object;

class AttachTagCommand : public Command
{
public:
  AttachTagCommand(Scene& scene, std::unique_ptr<Tag> tag);

private:
  void undo() override;
  void redo() override;

  class TagContext;
  std::vector<TagContext> m_contextes;
  Scene& m_scene;
};


class AttachTagCommand::TagContext
{
public:
  TagContext(Object& object, std::unique_ptr<Tag> tag);

  void attach_tag();
  void detach_tag();

private:
  Object* m_owner;
  std::unique_ptr<Tag> m_owned;
  Tag* m_reference;
};




}  // namespace omm

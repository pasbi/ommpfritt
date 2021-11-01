#pragma once

#include "commands/command.h"

namespace omm
{
class Tag;
class Object;

/**
 * @brief I didn't bother to implement this as a real `ListContext` because moving tags is
 *  more complicated than moving other items since they can be moved across structures.
 *  I.e., from the tag list of one object to the tag list of another.
 */
struct MoveTagContext {
  explicit MoveTagContext(Tag& tag);
  MoveTagContext(Tag& tag, Object& owner, Tag* predecessor);
  Tag* subject;
  Tag* predecessor;
  Object* owner;
  void assert_is_valid() const;
};

class MoveTagsCommand : public Command
{
public:
  MoveTagsCommand(const std::vector<Tag*>& tags, Object& new_owner, Tag* new_predecessor);
  void redo() override;
  void undo() override;

private:
  std::vector<MoveTagContext> m_old_contextes;
  std::vector<MoveTagContext> m_new_contextes;
};

}  // namespace omm

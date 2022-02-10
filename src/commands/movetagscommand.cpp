#include "commands/movetagscommand.h"
#include "objects/object.h"
#include "tags/tag.h"
#include "scene/contextes.h"

namespace
{
auto make_old_contextes(const std::vector<omm::Tag*>& tags)
{
  return util::transform(tags, [](omm::Tag* tag) { return omm::MoveTagContext(*tag); });
}

auto make_new_contextes(const std::vector<omm::Tag*>& tags, omm::Object& owner, omm::Tag* predecessor)
{
  return util::transform(tags, [&predecessor, &owner](omm::Tag* tag) {
    const auto context = omm::MoveTagContext(*tag, owner, predecessor);
    return context;
  });
}

void move(const omm::MoveTagContext& old_context, const omm::MoveTagContext& new_context)
{
  // since other tags might have been moved already, validity of old context is not guaranteed.
  // old_context.assert_is_valid();
  auto tag = old_context.owner->tags.remove(*old_context.subject);
  tag->owner = new_context.owner;
  omm::ListOwningContext<omm::Tag> context(std::move(tag), new_context.predecessor);
  new_context.owner->tags.insert(context);
  new_context.assert_is_valid();
}

template<typename Contextes> void move(const Contextes& old_ctxs, const Contextes& new_ctxs)
{
  assert(old_ctxs.size() == new_ctxs.size());
  for (std::size_t i = 0; i < old_ctxs.size(); ++i) {
    move(old_ctxs[i], new_ctxs[i]);
  }
}

}  // namespace

namespace omm
{
MoveTagContext::MoveTagContext(Tag& tag)
    : MoveTagContext(tag, *tag.owner, tag.owner->tags.predecessor(tag))
{
}

MoveTagContext::MoveTagContext(Tag& tag, Object& owner, Tag* predecessor)
    : subject(&tag), predecessor(predecessor), owner(&owner)
{
}

void MoveTagContext::assert_is_valid() const
{
  (void) subject;  // "use" one member, so static code checker don't think this method can be static.
  assert(subject != nullptr);
  assert(owner != nullptr);
  assert(predecessor == nullptr || predecessor->owner == subject->owner);
  assert(subject->owner == owner);
}

MoveTagsCommand ::MoveTagsCommand(const std::vector<Tag*>& tags,
                                  Object& new_owner,
                                  Tag* new_predecessor)
    : Command(QObject::tr("Move tags")), m_old_contextes(make_old_contextes(tags)),
      m_new_contextes(make_new_contextes(tags, new_owner, new_predecessor))
{
}

void MoveTagsCommand::undo()
{
  move(m_new_contextes, m_old_contextes);
}
void MoveTagsCommand::redo()
{
  move(m_old_contextes, m_new_contextes);
}

}  // namespace omm

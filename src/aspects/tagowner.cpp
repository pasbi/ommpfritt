#include "aspects/tagowner.h"
#include "common.h"

namespace omm
{

Tag& TagOwner::attach_tag(std::unique_ptr<Tag> tag, const Tag* predecessor)
{
  tag->set_owner(this);
  const auto pos = get_insert_position(predecessor);
  return insert(m_tags, std::move(tag), pos);
}

Tag& TagOwner::attach_tag(std::unique_ptr<Tag> tag)
{
  tag->set_owner(this);
  return insert(m_tags, std::move(tag), m_tags.size());
}

Tag& TagOwner::tag(size_t i) const
{
  return *m_tags[i];
}

size_t TagOwner::n_tags() const
{
  return m_tags.size();
}

std::unique_ptr<Tag> TagOwner::detach_tag(const Tag& tag)
{
  auto extracted_tag = extract(m_tags, tag);
  extracted_tag->set_owner(nullptr);
  return extracted_tag;
}

std::vector<Tag*> TagOwner::tags()
{
  return ::transform<Tag*>(m_tags, [](const auto& up) { return up.get(); });
}

size_t TagOwner::get_insert_position(const Tag* tag_before_position) const
{
  if (tag_before_position == nullptr) {
    return 0;
  } else {
    for (size_t i = 0; i < m_tags.size(); ++i) {
      if (tag_before_position == &*m_tags[i]) {
        return i + 1;
      }
    }
    assert(false);
    return 0;
  }
}

}  // namespace omm
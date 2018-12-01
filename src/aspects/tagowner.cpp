#include "aspects/tagowner.h"
#include "common.h"

namespace omm
{

TagOwner::~TagOwner()
{
}

Tag& TagOwner::add_tag(std::unique_ptr<Tag> tag, const Tag* predecessor)
{
  const auto pos = get_insert_position(predecessor);
  return insert(m_tags, std::move(tag), pos);
}

Tag& TagOwner::add_tag(std::unique_ptr<Tag> tag)
{
  const auto n = n_tags();
  const Tag* predecessor = n == 0 ? nullptr : &this->tag(n-1);
  return add_tag(std::move(tag), predecessor);
}

Tag& TagOwner::tag(size_t i) const
{
  return *m_tags[i];
}

size_t TagOwner::n_tags() const
{
  return m_tags.size();
}

std::unique_ptr<Tag> TagOwner::remove_tag(Tag& tag)
{
  return extract(m_tags, tag);
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
#pragma once

#include "scene/list.h"
#include <QObject>

namespace omm
{
class Tag;
class Object;
class TagList
    : public QObject
    , public List<Tag>
{
  Q_OBJECT
public:
  explicit TagList(Object& object);
  TagList(const TagList& other, Object& object);
  void insert(ListOwningContext<Tag>& context) override;
  void remove(ListOwningContext<Tag>& t) override;
  std::unique_ptr<Tag> remove(Tag& tag) override;
  void move(ListMoveContext<Tag>& context) override;
  Scene& scene();

private:
  Object& m_object;
};

}  // namespace omm

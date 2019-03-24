#include "scene/abstractselectionobserver.h"
#include "renderers/style.h"
#include "objects/object.h"
#include "tags/tag.h"
#include "tools/tool.h"

namespace
{

template<typename ItemT> auto cast(const std::set<omm::AbstractPropertyOwner*>& items)
{
  const auto items_t = omm::AbstractPropertyOwner::cast<ItemT>(items);
  assert(items_t.size() == items.size());
  return items_t;
}

}  // namespace

namespace omm
{

void AbstractSelectionObserver::
on_selection_changed(const std::set<AbstractPropertyOwner*>&) {}
void AbstractSelectionObserver::on_object_selection_changed(const std::set<Object*>&) {}
void AbstractSelectionObserver::on_style_selection_changed(const std::set<Style*>&) {}
void AbstractSelectionObserver::on_tag_selection_changed(const std::set<Tag*>&) {}
void AbstractSelectionObserver::on_tool_selection_changed(const std::set<Tool*>&) {}


void AbstractSelectionObserver::
on_selection_changed( const std::set<AbstractPropertyOwner*>& selection,
                      AbstractPropertyOwner::Kind kind )
{
  switch (kind) {
  case AbstractPropertyOwner::Kind::Style:
    on_style_selection_changed(::cast<Style>(selection));
    break;
  case AbstractPropertyOwner::Kind::Object:
    on_object_selection_changed(::cast<Object>(selection));
    break;
  case AbstractPropertyOwner::Kind::Tag:
    on_tag_selection_changed(::cast<Tag>(selection));
    break;
  case AbstractPropertyOwner::Kind::Tool:
    on_tool_selection_changed(::cast<Tool>(selection));
    break;
  default:
    break;
  }
}

}  // namespace omm

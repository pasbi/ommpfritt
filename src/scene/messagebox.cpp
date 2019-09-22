#include "scene/messagebox.h"
#include "objects/object.h"
#include "tags/tag.h"
#include "renderers/style.h"

namespace omm
{

MessageBox::MessageBox()
{
  connect(this, SIGNAL(transformation_changed(Object&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(appearance_changed(Object&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(appearance_changed(Tool&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(appearance_changed(Style&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(scene_reseted()), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(appearance_changed(Tool&)), this, SIGNAL(appearance_changed()));
  connect(this, &MessageBox::transformation_changed, [this](Object& o) {
    if (!o.is_root()) {
      Q_EMIT appearance_changed(o.tree_parent());
    }
  });

  connect(this, &MessageBox::object_inserted, [this](Object& parent, Object& o) {
    Q_UNUSED(o);
    Q_EMIT appearance_changed(parent);
    Q_EMIT abstract_property_owner_inserted(o);
  });
  connect(this, &MessageBox::object_removed, [this](Object& parent, Object& o) {
    Q_UNUSED(o);
    Q_EMIT appearance_changed(parent);
    Q_EMIT abstract_property_owner_removed(o);
  });
  connect(this, &MessageBox::object_moved, [this](Object& old_parent, Object& new_parent, Object& o)
  {
    Q_UNUSED(o);
    Q_EMIT appearance_changed(old_parent);
    Q_EMIT appearance_changed(new_parent);
  });
  connect(this, &MessageBox::tag_removed, [this](Object& owner, Tag& tag) {
    Q_UNUSED(tag);
    Q_EMIT appearance_changed(owner);
    Q_EMIT abstract_property_owner_removed(tag);
  });
  connect(this, &MessageBox::tag_inserted, [this](Object& owner, Tag& tag) {
    Q_UNUSED(tag);
    Q_EMIT appearance_changed(owner);
    Q_EMIT abstract_property_owner_inserted(tag);
  });

  connect(this, &MessageBox::style_removed, [this](Style& style) {
    Q_EMIT abstract_property_owner_removed(style);
  });
  connect(this, &MessageBox::style_inserted, [this](Style& style) {
    Q_EMIT abstract_property_owner_inserted(style);
  });

}

}  // namespace omm

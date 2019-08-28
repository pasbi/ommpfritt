#include "scene/messagebox.h"
#include "objects/object.h"
#include "tags/tag.h"

namespace omm
{

MessageBox::MessageBox()
{
  connect(this, SIGNAL(appearance_changed(Object&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(appearance_changed(Tool&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(appearance_changed(Style&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(transformation_changed(Object&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(scene_reseted()), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(object_inserted(Object&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(object_moved(Object&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(object_removed(Object&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(appearance_changed(Tool&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(tag_inserted(Tag&)), this, SIGNAL(appearance_changed()));
  connect(this, SIGNAL(tag_removed(Tag&)), this, SIGNAL(appearance_changed()));
}

}  // namespace omm

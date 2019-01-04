#include "tools/selecttool.h"
#include <memory>
#include <algorithm>
#include "tools/handles/axishandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"
#include "properties/floatproperty.h"

namespace omm
{

SelectTool::SelectTool(Scene& scene)
  : Tool(scene)
{
  add_property<FloatProperty>(RADIUS_PROPERTY_KEY, 20.0)
    .set_label(QObject::tr("radius").toStdString())
    .set_category(QObject::tr("ellipse").toStdString());
}

std::string SelectTool::type() const
{
  return TYPE;
}

QIcon SelectTool::icon() const
{
  return QIcon();
}

void SelectTool::update_selection(Object* o, std::set<AbstractPropertyOwner*>& old_selection)
{
  if (::contains(old_selection, o)) {
    old_selection.erase(o);
  } else {
    old_selection.insert(o);
  }
}

bool SelectTool::mouse_press(const arma::vec2& pos)
{
  // TODO 1) implement a kd-tree or similar to get the closest object fast
  //      2) I guess it's more intuitive when items can be selected by clicking anywhere inside
  //          its area.
  //      3) improve mouse pointer icon
  //      4) select during mouse move (like drawing)

  const auto items = scene.object_tree.items();
  if (items.size() == 0) {
    return false;
  }

  const auto get_distance = [pos](Object* o) {
    const auto o_pos = o->global_transformation().apply_to_position(arma::vec2{ 0.0, 0.0 });
    return std::pair(arma::norm(pos - o_pos), o);
  };
  const auto distances = ::transform<std::pair<double, Object*>>(items, get_distance);

  const auto compare = [pos](const auto& a, const auto& b) { return a.first < b.first; };
  const auto [distance, object] = *std::min_element(distances.begin(), distances.end(), compare);

  if (distance < property(RADIUS_PROPERTY_KEY).value<double>()) {
    auto selection = scene.selection();
    update_selection(object, selection);
    scene.set_selection(selection);
    return true;
  } else {
    scene.set_selection({});
    return false;
  }
}

}  // namespace omm

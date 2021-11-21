#pragma once

#include "tools/handles/abstractselecthandle.h"
#include "geometry/objecttransformation.h"

namespace omm
{

class Tool;
class Scene;
class Object;

class ObjectSelectHandle : public AbstractSelectHandle
{
public:
  explicit ObjectSelectHandle(Tool& tool, Scene& scene, Object& object);
  [[nodiscard]] bool contains_global(const Vec2f& point) const override;
  void draw(QPainter& painter) const override;

protected:
  [[nodiscard]] ObjectTransformation transformation() const;
  void set_selected(bool selected) override;
  void clear() override;
  [[nodiscard]] bool is_selected() const override;

private:
  Scene& m_scene;
  Object& m_object;
};

}  // namespace omm

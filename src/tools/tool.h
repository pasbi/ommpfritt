#pragma once

#include <QIcon>
#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include "tools/handles/handle.h"

namespace omm {

class Object;
class Scene;
class AbstractRenderer;

class Tool
  : public PropertyOwner<AbstractPropertyOwner::Kind::Tool>
  , public AbstractFactory<std::string, Tool, Scene&>
{
public:
  explicit Tool(Scene& scene);
  virtual ~Tool() = default;
  virtual QIcon icon() const = 0;
  static constexpr auto TYPE = "Tool";
  AbstractPropertyOwner::Kind KIND = AbstractPropertyOwner::Kind::Tool;

  /**
   * @brief returns true if any grabbable was moved
   */
  virtual bool mouse_move(const arma::vec2& delta, const arma::vec2& pos);

  /**
   * @brief returns true if any grabbable was hit
   */
  virtual bool mouse_press(const arma::vec2& pos);

  virtual void mouse_release(const arma::vec2& pos);
  virtual void draw(AbstractRenderer& renderer) const;
  virtual void on_selection_changed();
  virtual void on_scene_changed();
  Scene& scene;

protected:
  std::vector<std::unique_ptr<Handle>> handles;
  double epsilon = 10.0;

private:
  std::set<Object*> m_selection;
};

void register_tools();

}  // namespace omm

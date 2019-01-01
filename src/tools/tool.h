#pragma once

#include <memory>
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
  explicit Tool(Scene& scene, std::vector<std::unique_ptr<Handle>> handles);
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

  virtual void mouse_release();
  virtual void draw(AbstractRenderer& renderer) const;
  ObjectTransformation transformation() const;

  void set_selection(const std::set<Object*>& objects);
  const std::set<Object*> selection() const;
  std::vector<Handle*> handles() const;

  arma::vec2 map_to_tool_local(const arma::vec2& pos) const;

  static constexpr auto ALIGNMENT_PROPERTY_KEY = "alignment";

protected:
  Scene& scene;

private:
  std::set<Object*> m_selection;
  std::vector<std::unique_ptr<Handle>> m_handles;
};

void register_tools();

}  // namespace omm

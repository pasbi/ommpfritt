#pragma once

#include <QIcon>
#include <QMenu>
#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include <Qt>
#include "tools/handles/handle.h"

class QMouseEvent;

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
  static constexpr auto TYPE = QT_TR_NOOP("Tool");
  AbstractPropertyOwner::Kind KIND = AbstractPropertyOwner::Kind::Tool;

  /**
   * @brief returns true if any grabbable was moved
   */
  virtual bool mouse_move( const arma::vec2& delta, const arma::vec2& pos,
                           const QMouseEvent& event);

  /**
   * @brief returns true if any grabbable was hit
   */
  virtual bool mouse_press(const arma::vec2& pos, const QMouseEvent& event);
  virtual void mouse_release(const arma::vec2& pos, const QMouseEvent& event);
  virtual void draw(AbstractRenderer& renderer) const;
  virtual void on_selection_changed();
  virtual void on_scene_changed();
  virtual bool has_transformation() const;
  Scene& scene;
  virtual std::unique_ptr<QMenu> make_context_menu(QWidget* parent);
  virtual ObjectTransformation transformation() const;
  virtual void transform_objects(ObjectTransformation t, const bool tool_space);
  Flag flags() const override;

protected:
  std::vector<std::unique_ptr<Handle>> handles;
  double epsilon = 10.0;
};

void register_tools();

}  // namespace omm

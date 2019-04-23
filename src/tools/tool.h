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
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Tool", "Tool");

  /**
   * @brief returns true if any grabbable was moved
   */
  virtual bool mouse_move( const Vec2f& delta, const Vec2f& pos,
                           const QMouseEvent& event);

  /**
   * @brief returns true if any grabbable was hit
   */
  virtual bool mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force = false);
  virtual void mouse_release(const Vec2f& pos, const QMouseEvent& event);
  virtual bool key_press(const QKeyEvent& event);
  virtual void draw(AbstractRenderer& renderer) const;
  virtual void on_selection_changed();
  virtual void on_scene_changed();
  virtual bool has_transformation() const;
  virtual void cancel();
  Scene& scene;
  virtual std::unique_ptr<QMenu> make_context_menu(QWidget* parent);
  virtual ObjectTransformation transformation() const;
  Flag flags() const override;
  ObjectTransformation viewport_transformation;
  bool integer_transformation() const;

protected:
  std::vector<std::unique_ptr<Handle>> handles;
  double epsilon = 10.0;
};

void register_tools();

}  // namespace omm

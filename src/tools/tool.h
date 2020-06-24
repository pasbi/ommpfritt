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
class Painter;

class Tool
  : public PropertyOwner<Kind::Tool>
  , public AbstractFactory<QString, false, Tool, Scene&>
{
  Q_OBJECT
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
  virtual bool mouse_press(const Vec2f& pos, const QMouseEvent& event);
  virtual void mouse_release(const Vec2f& pos, const QMouseEvent& event);
  virtual bool key_press(const QKeyEvent& event);
  virtual void draw(Painter& painter) const;
  virtual bool has_transformation() const { return false; }
  virtual void cancel();
  virtual void end();
  virtual std::unique_ptr<QMenu> make_context_menu(QWidget* parent);
  virtual ObjectTransformation transformation() const;
  Flag flags() const override { return Flag::None; }
  ObjectTransformation viewport_transformation;
  bool integer_transformation() const;
  QString name() const override;
  virtual SceneMode scene_mode() const = 0;

public Q_SLOTS:
  virtual void reset() {}

protected:
  std::vector<std::unique_ptr<Handle>> handles;
  double epsilon = 10.0;
};

}  // namespace omm

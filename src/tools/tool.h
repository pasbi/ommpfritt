#pragma once

#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include "tools/handles/handle.h"
#include <QIcon>
#include <QMenu>
#include <Qt>

class QMouseEvent;

namespace omm
{
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
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Tool", "Tool");

  /**
   * @brief returns true if any grabbable was moved
   */
  virtual bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event);

  /**
   * @brief returns true if any grabbable was hit
   */
  virtual bool mouse_press(const Vec2f& pos, const QMouseEvent& event);
  virtual void mouse_release(const Vec2f& pos, const QMouseEvent& event);
  virtual bool key_press(const QKeyEvent& event);
  virtual void draw(Painter& renderer) const;
  virtual bool has_transformation() const
  {
    return false;
  }
  virtual void cancel();
  virtual void end();
  virtual void start()
  {
  }
  virtual std::unique_ptr<QMenu> make_context_menu(QWidget* parent);
  virtual ObjectTransformation transformation() const;
  Flag flags() const override
  {
    return Flag::None;
  }
  ObjectTransformation viewport_transformation;
  static bool integer_transformation();
  QString name() const override;
  virtual SceneMode scene_mode() const = 0;
  static QRectF centered_rectangle(const Vec2f& center, double radius);

public Q_SLOTS:
  virtual void reset()
  {
  }

protected:
  std::vector<std::unique_ptr<Handle>> handles;
  static constexpr double epsilon = 10.0;
};

}  // namespace omm

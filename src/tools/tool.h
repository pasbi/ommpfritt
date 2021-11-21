#pragma once

#include "aspects/propertyowner.h"
#include "external/json_fwd.hpp"
#include "geometry/objecttransformation.h"
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
  [[nodiscard]] bool is_active() const;

  /**
   * @brief cancel cancels the current tool application and restores the status before.
   * @return true if something was canceled, false otherwise (i.e., if there was nothing to cancel).
   *  Immediately canceling the tool again should eventually return false.
   */
  virtual bool cancel();
  virtual void end();
  virtual void start();

  virtual std::unique_ptr<QMenu> make_context_menu(QWidget* parent);
  virtual ObjectTransformation transformation() const;
  Flag flags() const override;
  ObjectTransformation viewport_transformation;
  static bool integer_transformation();
  QString name() const override;
  virtual SceneMode scene_mode() const = 0;
  static QRectF centered_rectangle(const Vec2f& center, double radius);
  virtual void reset();
  void clear();
  std::deque<Handle*> handles() const;

protected:
  static constexpr double epsilon = 10.0;
  void push_handle(std::unique_ptr<Handle> handle);
private:
  std::deque<std::unique_ptr<Handle>> m_handles;
};

}  // namespace omm

#pragma once

#include <QObject>

namespace omm
{
/**
 * @brief The Options class stored application wide options that are not persistent, i.e.,
 *  - loading/saving the scene does not change them
 *  - restarting the application resets them
 */
class Options : public QObject
{
  Q_OBJECT

public:
  explicit Options(bool is_cli, bool have_opengl);
  enum class Anchor { TopLeft, BottomLeft, TopRight, BottomRight, Center, None };
  [[nodiscard]] Anchor anchor() const;
  static QPointF anchor_position(const QRectF& grid, const Anchor& anchor);
  [[nodiscard]] QPointF anchor_position(const QRectF& grid) const;

  const bool is_cli;
  const bool have_opengl;

  [[nodiscard]] bool require_gui() const;

public:
  void set_anchor(omm::Options::Anchor anchor);

Q_SIGNALS:
  void anchor_changed(omm::Options::Anchor);

private:
  Anchor m_anchor = Anchor::Center;
};

}  // namespace omm

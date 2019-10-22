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
  enum class Anchor { TopLeft, BottomLeft, TopRight, BottomRight, Center, None };
  Anchor anchor() const;
  static QPointF anchor_position(const QRectF& grid, const Anchor& anchor);
  QPointF anchor_position(const QRectF& grid) const;
public Q_SLOTS:
  void set_anchor(Anchor anchor);
Q_SIGNALS:
  void anchor_changed(Anchor);
private:
  Anchor m_anchor = Anchor::Center;

};

}  // namespace omm

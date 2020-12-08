#pragma once

#include <QIconEngine>

namespace omm
{
class Style;

class StyleIconEngine : public QIconEngine
{
public:
  explicit StyleIconEngine(const Style* style);

  void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;
  [[nodiscard]] QIconEngine* clone() const override;

private:
  const Style* const m_style;
};

}  // namespace omm

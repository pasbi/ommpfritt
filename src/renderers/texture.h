#pragma once

#include <QImage>
#include <QPoint>

namespace omm
{
struct Texture {
  explicit Texture(const QSize& size);
  explicit Texture();
  explicit Texture(const QImage& image, const QPoint& offset);
  const QImage image;
  const QPoint offset;
};

}  // namespace omm

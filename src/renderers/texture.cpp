#include "renderers/texture.h"
#include <QPainter>

namespace
{
QImage uniform_image(const QSize& size)
{
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  if (size.isEmpty()) {
    return image;
  } else {
    QPainter painter(&image);
    painter.fillRect(image.rect(), Qt::blue);
    return image;
  }
}

}  // namespace

namespace omm
{
Texture::Texture(const QSize& size) : image(uniform_image(size)), offset(QPoint(0, 0))
{
}

Texture::Texture() : Texture(QSize(0, 0))
{
}

Texture::Texture(const QImage& image, const QPoint& offset) : image(image), offset(offset)
{
  assert(image.format() == QImage::Format_ARGB32_Premultiplied);
}

}  // namespace omm

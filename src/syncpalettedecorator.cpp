#include "syncpalettedecorator.h"
#include <QApplication>
#include <QEvent>
#include <QAbstractButton>
#include <QTreeWidgetItem>

namespace
{

int img_mean(const QImage& image)
{
  static constexpr double NUM_CHANNELS = 3.0;
  double w = 0.0;
  double sum = 0.0;
  for (int y = 0; y < image.height(); ++y) {
    assert(image.format() == QImage::Format_ARGB32_Premultiplied);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const QRgb* rgba_line = reinterpret_cast<const QRgb*>(image.scanLine(y));
    for (int x = 0; x < image.width(); ++x) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      const auto& rgba = rgba_line[x];
      const double alpha = qAlpha(rgba) / 255.0;
      sum += qRed(rgba) * alpha;
      sum += qGreen(rgba) * alpha;
      sum += qBlue(rgba) * alpha;
      w += NUM_CHANNELS * alpha;
    }
  }
  return static_cast<int>(sum / w);
}

}  // namespace

namespace omm
{

void SyncPaletteButtonDecorator::decorate(QAbstractButton& button)
{
  (new SyncPaletteButtonDecorator{button, button})->update();
}

void SyncPaletteButtonDecorator::update() const
{
  const QColor text_color = QApplication::palette().color(QPalette::Active, QPalette::ButtonText);
  static constexpr int ICON_SIZE = 1024;
  static constexpr int ICON_THRESHOLD = 100;
  QImage img = m_observed.icon().pixmap(QSize(ICON_SIZE, ICON_SIZE)).toImage();
  if (std::abs(img_mean(img) - text_color.value()) > ICON_THRESHOLD) {
    img.invertPixels(QImage::InvertRgb);
    QSignalBlocker blocker(&m_observed);
    m_observed.setIcon(QIcon(QPixmap::fromImage(img)));
  }
}

void SyncPaletteTreeWidgetItemDecorator::decorate(QTreeWidgetItem& item)
{
  (new SyncPaletteTreeWidgetItemDecorator(*item.treeWidget(), item))->update();
}

void SyncPaletteTreeWidgetItemDecorator::update() const
{
  const auto palette = QApplication::palette();
  const auto fg_color = palette.color(QPalette::Active, QPalette::WindowText);
  m_observed.setForeground(0, fg_color);
}


}  // namespace omm

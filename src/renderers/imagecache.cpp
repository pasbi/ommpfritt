#include "renderers/imagecache.h"
#include <QPainter>
#include <QSvgRenderer>
#include <poppler/qt5/poppler-qt5.h>
#include "logging.h"

namespace omm
{

QPicture ImageCache::retrieve(const std::pair<QString, int> &key) const
{
  QPicture picture;
  const QString& filename = key.first;
  QPainter painter(&picture);
  if (filename.endsWith(".svg", Qt::CaseInsensitive)) {
    QSvgRenderer renderer(filename);
    renderer.render(&painter);
  } else if (filename.endsWith(".pdf", Qt::CaseInsensitive)) {
    auto* doc = Poppler::Document::load(filename);
    if (doc) {
      doc->setRenderBackend(Poppler::Document::ArthurBackend);
      int page_num = key.second;
      page_num = std::clamp(page_num, 0, doc->numPages()-1);
      const auto page = doc->page(page_num);
      if (page) {
        const auto success = page->renderToPainter(&painter);
        if (!success) {
          LERROR << "Failed to render pdf.";
        }
        delete page;
      } else {
        LERROR << "Failed to load page";
      }
      delete doc;
    } else {
      LERROR << "Failed to load doc";
    }
  } else {
    QImage image(filename);
    painter.drawImage(QRectF(0, 0, image.width(), image.height()), image);
  }
  return picture;
}

}  // namespace omm

#pragma once

#include <QImage>
#include <QPicture>

namespace omm
{

class ImageCache
{
public:
  ImageCache() = default;
  ImageCache(const ImageCache& other) = delete;
  ImageCache(ImageCache&& other) = delete;
  ImageCache& operator=(const ImageCache& other) = delete;
  void clear();
  const QPicture &load(const QString &filename, int page_num);

private:
  std::map<std::pair<QString, int>, QPicture> m_cache;
};

}  // namespace

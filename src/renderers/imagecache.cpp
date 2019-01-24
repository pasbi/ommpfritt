#include "renderers/imagecache.h"

namespace omm
{

QImage ImageCache::load(const std::string& filename)
{
  if (m_cache.count(filename) == 0) { m_cache[filename].load(QString::fromStdString(filename)); }
  return m_cache.at(filename);
}

void ImageCache::clear()
{
  m_cache.clear();
}

}  // namespace omm

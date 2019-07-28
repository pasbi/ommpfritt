#pragma once

#include <QImage>
#include <QPicture>
#include "cache.h"

namespace omm
{

class ImageCache : public Cache<std::pair<QString, int>, QPicture>
{
protected:
  QPicture retrieve(const std::pair<QString, int>& key) const override;
};

}  // namespace

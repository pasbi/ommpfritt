#include "managers/objectmanager/objectmimedata.h"
#include <glog/logging.h>

namespace omm
{

ObjectMimeData::ObjectMimeData(const ObjectRefs& objects)
  : objects(objects)
{
}

bool ObjectMimeData::hasFormat(const QString& mimeType) const
{
  return mimeType == MIME_TYPE;
}

QStringList ObjectMimeData::formats() const
{
  return { MIME_TYPE };
}

QVariant ObjectMimeData::retrieveData(const QString &mimeType, QVariant::Type type) const
{
  (void) mimeType;
  (void) type;
  LOG(FATAL) << "This function shall not be called.";
  assert(false);
}

}  // namespace omm

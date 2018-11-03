#pragma once

#include <QMimeData>
#include "common.h"

namespace omm
{

class Object;

class ObjectMimeData : public QMimeData
{
  Q_OBJECT

public:
  const ObjectRefs objects;

  static constexpr auto MIME_TYPE = "application/objecttree-drag-drop-object";

  ObjectMimeData(const ObjectRefs& object);
  bool hasFormat(const QString& mimeType) const override;
  QStringList formats() const override;
  QVariant retrieveData(const QString &mimeType, QVariant::Type type) const override;
};

}  // namespace omm

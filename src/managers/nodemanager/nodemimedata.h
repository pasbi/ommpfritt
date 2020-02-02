#pragma once

#include <QMimeData>
#include "managers/nodemanager/nodecompiler.h"
#include "logging.h"

namespace omm
{

class Node;
class InputPort;
class OutputPort;

class NodeMimeData : public QMimeData
{
  Q_OBJECT

public:
  static const std::map<AbstractNodeCompiler::Language, QString> MIME_TYPES;
  NodeMimeData(AbstractNodeCompiler::Language language, const std::set<Node*>& items);
  bool hasFormat(const QString& mimeType) const override;
  QStringList formats() const override { return { MIME_TYPES.at(m_language) }; }
  QVariant retrieveData(const QString &mimeType, QVariant::Type type) const override;
  std::set<Node*> nodes() const { return m_nodes; }
private:
  const AbstractNodeCompiler::Language m_language;
  std::set<Node*> m_nodes;
};


}  // namespace omm

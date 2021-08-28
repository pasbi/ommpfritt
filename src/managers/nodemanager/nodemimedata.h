#pragma once

#include "logging.h"
#include "nodesystem/nodecompiler.h"
#include <QMimeData>

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
  [[nodiscard]] bool hasFormat(const QString& mimeType) const override;
  [[nodiscard]] QStringList formats() const override
  {
    return {MIME_TYPES.at(m_language)};
  }
  [[noreturn]] QVariant retrieveData(const QString&, QMetaType) const override;
  [[nodiscard]] std::set<Node*> nodes() const
  {
    return m_nodes;
  }

private:
  const AbstractNodeCompiler::Language m_language;
  std::set<Node*> m_nodes;
};

}  // namespace omm

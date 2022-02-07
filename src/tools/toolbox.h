#pragma once

#include "common.h"
#include <list>
#include <map>
#include <memory>
#include <QObject>
#include <QString>

namespace omm
{

class Scene;
class Tool;

class ToolBox : public QObject
{
  Q_OBJECT
public:
  ToolBox(Scene& scene);

  [[nodiscard]] Tool& active_tool() const;
  [[nodiscard]] Tool& tool(const QString& key) const;
  void set_active_tool(const QString& key);
  void set_active_tool(Tool& tool);
  void activate_previous_tool();
  void set_scene_mode(SceneMode mode);
  [[nodiscard]] ::transparent_set<Tool* > tools() const;
  void activate_default_tool();

Q_SIGNALS:
  void active_tool_changed(const omm::Tool& active_tool);

private:
  Tool* m_active_tool = nullptr;
  const std::map<QString, std::unique_ptr<Tool>> m_tools;
  static const std::map<SceneMode, QString> m_default_tools;
  Scene& m_scene;
  std::list<Tool*> m_history;
};

}  // namespace omm

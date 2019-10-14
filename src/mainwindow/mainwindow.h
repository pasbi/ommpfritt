#pragma once

#include <memory>
#include <map>
#include <QMainWindow>
#include "keybindings/keybindings.h"

class QSettings;

namespace omm
{

class Application;
class Scene;
class Manager;
class Viewport;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(Application& app);
  void restore_state();
  void save_state();
  void keyPressEvent(QKeyEvent* e) override;
  Viewport& viewport() const;
  void restore_default_layout();

  static std::vector<std::string> object_menu_entries();
  static std::vector<std::string> path_menu_entries();
  static std::vector<std::string> main_menu_entries();

  std::unique_ptr<QMenu> make_about_menu();
  static std::vector<std::string> available_translations();
  void make_unique_manager_name(QDockWidget& widget) const;

  void save_layout();
  void save_layout(QSettings& settings);
  void load_layout();
  void load_layout(QSettings& settings);

  static constexpr auto LOCALE_SETTINGS_KEY = "locale";
  static constexpr auto SKIN_SETTINGS_KEY = "skin";
  static constexpr auto TOOLBAR_SETTINGS_KEY = "mainwindow/toolbars";
  static constexpr auto TOOLBAR_TOOL_SETTINGS_KEY = "tool";
  static constexpr auto TOOLBAR_TOOLS_SETTINGS_KEY = "tools";
  static constexpr auto MANAGER_SETTINGS_KEY = "mainwindow/managers";
  static constexpr auto MANAGER_TYPE_SETTINGS_KEY = "type";
  static constexpr auto MANAGER_NAME_SETTINGS_KEY = "name";
  static constexpr auto GEOMETRY_SETTINGS_KEY = "mainwindow/geometry";
  static constexpr auto WINDOWSTATE_SETTINGS_KEY = "mainwindow/window_state";
  static constexpr auto RECENT_DOCUMENTS_SETTINGS_KEY = "mainwindow/recent_documents";
  static constexpr auto TYPE_NAME_CONTEXT = "typename";
  static constexpr auto LAST_LAYOUT_FILE_NAME = "last_layout_file_name";

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  Application& m_app;
  Viewport* m_viewport;
  std::vector<QDockWidget*> dock_widgets() const;
  void update_recent_files_menu();
  QString get_last_layout_filename() const;


private Q_SLOTS:
  void update_window_title();
};

}  // namespace omm

#pragma once

#include <QMainWindow>
#include <map>
#include <memory>

class QSettings;

namespace omm
{
class Application;
class Scene;
class Manager;
class Viewport;
class ToolBar;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(Application& app);
  void restore_state();
  void save_state();
  void keyPressEvent(QKeyEvent* e) override;
  [[nodiscard]] Viewport& viewport() const;
  void restore_default_layout();

  static std::vector<QString> object_menu_entries();
  static std::vector<QString> path_menu_entries();
  static std::vector<QString> main_menu_entries();

  std::unique_ptr<QMenu> make_about_menu();
  static std::vector<QString> available_translations();
  void assign_unique_objectname(Manager& manager) const;
  void assign_unique_objectname(ToolBar& toolbar) const;

  void save_layout();
  void save_layout(QSettings& settings);
  void load_layout();
  void load_layout(QSettings& settings);

  static constexpr auto LOCALE_SETTINGS_KEY = "locale";
  static constexpr auto SKIN_SETTINGS_KEY = "skin";
  static constexpr auto TOOLBAR_SETTINGS_KEY = "mainwindow/toolbars";
  static constexpr auto TOOLBAR_TYPE_SETTINGS_KEY = "type";
  static constexpr auto TOOLBAR_NAME_SETTINGS_KEY = "name";
  static constexpr auto TOOLBAR_TOOLS_SETTINGS_KEY = "tools";
  static constexpr auto MANAGER_SETTINGS_KEY = "mainwindow/managers";
  static constexpr auto MANAGER_TYPE_SETTINGS_KEY = "type";
  static constexpr auto MANAGER_NAME_SETTINGS_KEY = "name";
  static constexpr auto GEOMETRY_SETTINGS_KEY = "mainwindow/geometry";
  static constexpr auto WINDOWSTATE_SETTINGS_KEY = "mainwindow/window_state";
  static constexpr auto RECENT_SCENES_SETTINGS_KEY = "mainwindow/recent_scenes";
  static constexpr auto TYPE_NAME_CONTEXT = "typename";
  static constexpr auto LAST_LAYOUT_FILE_NAME = "last_layout_file_name";

protected:
  void closeEvent(QCloseEvent* event) override;
  bool eventFilter(QObject* o, QEvent* e) override;

private:
  Application& m_app;
  Viewport* m_viewport;
  [[nodiscard]] std::vector<QDockWidget*> dock_widgets() const;
  void update_recent_scenes_menu();
  [[nodiscard]] static QString get_last_layout_filename();
  void handle_corrupted_config_file(const QSettings& s, const QString& what);
  void save_managers(QSettings& settings);
  void save_toolbars(QSettings& settings);
  void restore_managers(QSettings& settings);
  void restore_toolbars(QSettings& settings);

private:
  void update_window_title();
};

}  // namespace omm

#pragma once

#include "managers/manager.h"
#include <QObject>
#include "scene/scene.h"
#include "python/pythonengine.h"
#include "keybindings/keybindings.h"
#include "keybindings/commandinterface.h"
#include <Qt>
#include "mainwindow/iconprovider.h"
#include <QTimer>
#include <QKeySequence>
#include "preferences/uicolors.h"
#include "mainwindow/options.h"
#include <memory>
#include <map>
#include "preferences/preferences.h"
#include <set>


class QApplication;
class QAbstractButton;

namespace omm
{

class MainWindow;
class Manager;
class ToolBar;
class Options;
class ModeSelector;

class Application : public QObject, public CommandInterface
{
  Q_OBJECT
private:
  std::nullptr_t first_member;
public:
  Application(QCoreApplication& app, std::unique_ptr<Options> options);
  ~Application();
  bool save();
  bool save_as();
  bool save(const QString& filename);
  bool can_close();
  void load();
  void load(const QString& filename, bool force);
  void reset();
  void quit();
  void update_undo_redo_enabled();
  void set_main_window(MainWindow& main_window);
  void evaluate() const;
  QKeySequence default_key_sequence(const QString& name) const;
  static Application& instance();

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Application");
  QString type() const override;

  KeyBindings key_bindings;
  UiColors ui_colors;
  Preferences preferences;
  bool handle_mode(const QString& action_name);
  const std::map<QString, std::unique_ptr<ModeSelector>> mode_selectors;

  PythonEngine python_engine;
  Scene scene;
  MessageBox& message_box();
  MainWindow* main_window() const;
  const IconProvider icon_provider;

  enum class InsertionMode { Default, AsParent, AsChild };
  Object &insert_object(const QString& key, InsertionMode mode);

  static const std::set<int> keyboard_modifiers;
  void register_auto_invert_icon_button(QAbstractButton& button);

  Manager& spawn_manager(const QString& type);
  ToolBar& spawn_toolbar();

  /**
   * @brief get_active_manager returns a reference to an active manager.
   *  Spawns a new manager if no active manager is found.
   *  A manager is active if it is visible and not locked.
   * @param type the type of the manager, e.g., PropertyManager::TYPE
   * @return
   */
  Manager& get_active_manager(const QString& type);

private:
  QCoreApplication& m_app;
  static Application* m_instance;
  MainWindow* m_main_window;

public:
  Options& options() { return *m_options; }
private:
  std::unique_ptr<Options> m_options;

public:
  /**
   * @brief dispatch_key accumulates the key event with already received events.
   *  The CommandInterface to deliver the action is the docked widget which contains the mouse.
   *  If that Manager does not want the action, it is delivered to the Application.
   * @param key the pressed key
   * @param modifiers the modifiers which were down during the key press
   * @return true if the action was accepted by the docked Manager under the mouse or the
   *  Application.
   */
  bool dispatch_key(int key, Qt::KeyboardModifiers modifiers);

  /**
   * @brief dispatch_key same as @code dispatch_key, but the event is delivered to given
   *  CommandInterface @code ci. If that CommandInterface does not want the action, it is forwarded
   *  to the Application.
   * @param key
   * @param modifiers
   * @param ci
   * @return true if the action was accepted by @code ci or Application.
   */
  bool dispatch_key(int key, Qt::KeyboardModifiers modifiers, CommandInterface& ci);

  void register_manager(Manager& manager);
  void unregister_manager(Manager& manager);

  std::set<Manager*> managers(const QString& type) const;

  bool perform_action(const QString& name) override;

private:
  QTimer m_reset_keysequence_timer;
  QKeySequence m_pending_key_sequence;
  std::set<Manager*> m_managers;

  void install_translators();
  std::set<std::unique_ptr<QTranslator>> m_translators;
  const QLocale m_locale;
  static void init(omm::Application* instance);
};

const Preferences& preferences();

}  // namespace omm

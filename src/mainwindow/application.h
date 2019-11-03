#pragma once

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
#include "preferences/preferences.h"

class QApplication;
class QAbstractButton;

namespace omm
{

class MainWindow;
class Manager;
class Options;

class Application : public QObject, public CommandInterface
{
  Q_OBJECT
public:
  Application(QApplication& app);
  ~Application();
  bool save();
  bool save_as();
  bool save(const QString& filename);
  bool can_close();
  void load();
  void load(const QString& filename);
  void reset();
  void quit();
  void update_undo_redo_enabled();
  void set_main_window(MainWindow& main_window);
  QKeySequence default_key_sequence(const QString& name) const;
  static Application& instance();

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Application");
  QString type() const override;

  KeyBindings key_bindings;
  UiColors ui_colors;
  Preferences preferences;

  PythonEngine python_engine;
  Scene scene;
  MessageBox& message_box();
  MainWindow* main_window() const;
  const IconProvider icon_provider;

  enum class InsertionMode { Default, AsParent, AsChild };
  Object &insert_object(const QString& key, InsertionMode mode);

  static const std::set<int> keyboard_modifiers;
  void register_auto_invert_icon_button(QAbstractButton& button);

private:
  QApplication& m_app;
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
  bool perform_action(const QString& name) override;
private:
  QTimer m_reset_keysequence_timer;
  QKeySequence m_pending_key_sequence;
  std::set<Manager*> m_managers;
};

const Preferences& preferences();

}  // namespace omm

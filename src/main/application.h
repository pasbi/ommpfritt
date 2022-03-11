#pragma once

#include "common.h"
#include "keybindings/commandinterface.h"
#include "translator.h"
#include <QCoreApplication>
#include <QKeySequence>
#include <QObject>
#include <QTimer>
#include <Qt>
#include <map>
#include <memory>
#include <set>

class QApplication;
class QAbstractButton;

namespace omm
{
class KeyBindings;
class MainWindow;
class MailBox;
class Manager;
class ModeSelector;
class Object;
class Options;
class Preferences;
class Scene;
class ToolBar;
class UiColors;

class Application
    : public QObject
    , public CommandInterface
{
  Q_OBJECT
private:
  std::nullptr_t first_member;

public:
  Application(QCoreApplication& app, std::unique_ptr<Options> options);
  ~Application() override;

  Application(Application&&) = delete;
  Application(const Application&) = delete;
  Application& operator=(Application&&) = delete;
  Application& operator=(const Application&) = delete;

  bool save();
  bool save_as();
  bool save(const QString& filename);
  bool can_close();
  void open();
  void open(const QString& filename, bool force);
  void reset();
  void quit();
  void update_undo_redo_enabled();
  void set_main_window(MainWindow& main_window);
  void evaluate() const;
  [[nodiscard]] QKeySequence default_key_sequence(const QString& name) const;
  static Application& instance();
  [[nodiscard]] SceneMode scene_mode() const;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Application");
  [[nodiscard]] QString type() const override;

  std::unique_ptr<KeyBindings> key_bindings;
  std::unique_ptr<UiColors> ui_colors;
  std::unique_ptr<Preferences> preferences;
  bool handle_mode(const QString& action_name);
  const std::map<QString, std::unique_ptr<ModeSelector>> mode_selectors;

  std::unique_ptr<Scene> scene;
  [[nodiscard]] MailBox& mail_box() const;
  [[nodiscard]] MainWindow* main_window() const;

  enum class InsertionMode { Default, AsParent, AsChild };

  // NOLINTNEXTLINE(modernize-use-nodiscard)
  Object& insert_object(const QString& key, InsertionMode mode) const;

  static const std::set<int> keyboard_modifiers;

  // NOLINTNEXTLINE(modernize-use-nodiscard)
  Manager& spawn_manager(const QString& type) const;

  // NOLINTNEXTLINE(modernize-use-nodiscard)
  ToolBar& spawn_toolbar() const;

  /**
   * @brief get_active_manager returns a reference to an active manager.
   *  Spawns a new manager if no active manager is found.
   *  A manager is active if it is visible and not locked.
   * @param type the type of the manager, e.g., PropertyManager::TYPE
   * @return
   */
  [[nodiscard]] Manager& get_active_manager(const QString& type) const;

private:
  QCoreApplication& m_app;

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static Application* m_instance;
  MainWindow* m_main_window = nullptr;

public:
  Options& options()
  {
    return *m_options;
  }

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

  [[nodiscard]] std::set<Manager*> managers(const QString& type) const;

  bool perform_action(const QString& name) override;

private:
  QTimer m_reset_keysequence_timer;
  QKeySequence m_pending_key_sequence;
  std::set<Manager*> m_managers;

  Translator m_translator;
  static void init(omm::Application* instance);
};

const Preferences& preferences();

}  // namespace omm

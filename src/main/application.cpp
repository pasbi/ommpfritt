#include "application.h"

#include <cassert>
#include <QAbstractButton>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "tags/nodestag.h"
#include "tags/scripttag.h"

#include "commands/addcommand.h"
#include "commands/movecommand.h"
#include "keybindings/modeselector.h"
#include "keybindings/keybindings.h"
#include "logging.h"
#include "main/options.h"
#include "mainwindow/exportdialog.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/pathactions.h"
#include "mainwindow/toolbar/toolbar.h"
#include "mainwindow/viewport/viewport.h"
#include "managers/manager.h"
#include "objects/object.h"
#include "preferences/preferencedialog.h"
#include "preferences/uicolors.h"
#include "python/pythonengine.h"
#include "registers.h"
#include "scene/history/historymodel.h"
#include "scene/scene.h"
#include "scene/stylelist.h"
#include "tags/tag.h"
#include "tools/toolbox.h"
#include "widgets/pointdialog.h"
#include "objects/path.h"

namespace
{
constexpr auto FILE_ENDING = ".omm";

QKeySequence push_back(const QKeySequence& s, int t)
{
  switch (s.count()) {
  case 0:
    return QKeySequence(t);
  case 1:
    return QKeySequence(s[0], t);
  case 2:
    return QKeySequence(s[0], s[1], t);
  case 3:
    return QKeySequence(s[0], s[1], s[2], t);
  case 4:
    return QKeySequence(s[1], s[2], s[3], t);
  default:
    Q_UNREACHABLE();
    return QKeySequence();
  }
}

auto load_locale()
{
  const auto locale = QSettings().value(omm::MainWindow::LOCALE_SETTINGS_KEY).toLocale();
  return locale;
}

QString scene_directory_hint(const QString& scene_filename)
{
  if (QFileInfo::exists(scene_filename)) {
    return QFileInfo(scene_filename).dir().path();
  } else {
    return QDir::homePath();
  }
}

auto init_mode_selectors()
{
  std::map<QString, std::unique_ptr<omm::ModeSelector>> map;
  const auto insert = [&map](const QString& name,
                             const QString& cycle_action,
                             const std::vector<QString>& activation_actions) {
    map.insert({name,
                std::make_unique<omm::ModeSelector>(omm::Application::instance(),
                                                    name,
                                                    cycle_action,
                                                    activation_actions)});
  };

  insert("scene_mode", "scene_mode.cycle", {"scene_mode.object", "scene_mode.vertex"});
  return map;
}

}  // namespace

namespace omm
{
const std::set<int> Application::keyboard_modifiers{Qt::Key_Shift,
                                                    Qt::Key_Control,
                                                    Qt::Key_Alt,
                                                    Qt::Key_Meta};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,-warnings-as-errors)
Application* Application::m_instance = nullptr;

Application::Application(QCoreApplication& app, std::unique_ptr<Options> options)
    : first_member((init(this), nullptr))
    , key_bindings(std::make_unique<KeyBindings>())
    , ui_colors(std::make_unique<UiColors>())
    , preferences(std::make_unique<Preferences>())
    , mode_selectors(init_mode_selectors())
    , python_engine(std::make_unique<PythonEngine>())
    , scene(std::make_unique<Scene>(*python_engine))
    , m_app(app)
    , m_options(std::move(options))
    , m_translator(load_locale())
{
  static constexpr int RESET_KEYSEQUENCE_INTERVAL_MS = 1000;
  scene->set_selection({});
  if (!this->options().is_cli) {
    ui_colors->apply();
    m_reset_keysequence_timer.setSingleShot(true);
    m_reset_keysequence_timer.setInterval(RESET_KEYSEQUENCE_INTERVAL_MS);
    connect(&m_reset_keysequence_timer, &QTimer::timeout, this, [this]() {
      m_pending_key_sequence = QKeySequence();
    });
  }

  scene->polish();
}

void Application::init(omm::Application* instance)
{
  register_everything();
  QCoreApplication::setOrganizationName(QObject::tr("omm"));
  QCoreApplication::setApplicationName(::QApplication::translate("QObject", "ommpfritt"));
  if (Application::m_instance == nullptr) {
    Application::m_instance = instance;
  } else {
    LFATAL("Resetting application instance.");
  }
}

Application::~Application() = default;

Application& Application::instance()
{
  assert(m_instance != nullptr);
  return *m_instance;
}

SceneMode Application::scene_mode() const
{
  return scene->current_mode();
}

void Application::set_main_window(MainWindow& main_window)
{
  m_main_window = &main_window;
}

void Application::evaluate() const
{
  for (Tag* tag : scene->tags()) {
    tag->evaluate();
  }
}

void Application::quit()
{
  if (can_close()) {
    LINFO << "Quit application.";
    QCoreApplication::quit();
  } else {
    LINFO << "Aborted quit.";
  }
}

void Application::update_undo_redo_enabled()
{
}

bool Application::can_close()
{
  if (scene->has_pending_changes()) {
    const auto decision
        = QMessageBox::question(m_main_window,
                                tr("Question."),
                                tr("Some pending changes will be lost if you don't save."),
                                QMessageBox::Close | QMessageBox::Cancel | QMessageBox::Save,
                                QMessageBox::Save);
    switch (decision) {
    case QMessageBox::Close:
      return true;
    case QMessageBox::Cancel:
      return false;
    case QMessageBox::Save:
      return save();
    default:
      LERROR << "Unexpected response code: " << decision;
      return false;
    }
  } else {
    return true;
  }
}

bool Application::save(const QString& filename)
{
  if (!scene->save_as(filename)) {
    LWARNING << "Error saving scene as '" << filename << "'.";
    QMessageBox::critical(m_main_window,
                          tr("Error."),
                          tr("The scene could not be saved at '%1'.").arg(filename),
                          QMessageBox::Ok,
                          QMessageBox::Ok);
    return false;
  } else {
    return true;
  }
}

bool Application::save()
{
  const QString filename = scene->filename();
  if (filename.isEmpty()) {
    return save_as();
  } else {
    return save(filename);
  }
}

bool Application::save_as()
{
  QFileDialog dialog(m_main_window);
  dialog.setWindowTitle(tr("Save scene as ..."));
  dialog.setDirectoryUrl(scene_directory_hint(scene->filename()));
  dialog.setDefaultSuffix(FILE_ENDING);
  if (dialog.exec() == QDialog::Accepted) {
    const auto files = dialog.selectedFiles();
    assert(files.size() == 1);
    return scene->save_as(files.front());
  } else {
    return false;
  }
}

void Application::reset()
{
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  if (!can_close()) {
    return;
  }

  LINFO << "reset scene.";
  scene->set_selection({});
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  QTimer::singleShot(0, scene.get(), &Scene::reset);
}

void Application::open(const QString& filename, bool force)
{
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  if (force || can_close()) {
    QTimer::singleShot(0, this, [this, filename]() {
      if (!scene->load_from(filename)) {
        QMessageBox::critical(m_main_window,
                              tr("Error."),
                              tr("Opening scene from '%1' failed.").arg(filename),
                              QMessageBox::Ok);
      }
    });
  }
}

void Application::open()
{
  if (can_close()) {
    const QString filename = QFileDialog::getOpenFileName(m_main_window,
                                                          tr("Open scene ..."),
                                                          scene_directory_hint(scene->filename()));
    if (filename.isEmpty()) {
      return;
    }

    open(filename, true);
  }
}

bool Application::perform_action(const QString& action_name)
{
#ifndef NDEBUG
  for (auto&& action_name : path_actions::available_actions()) {
    assert(key_bindings->find_action(Application::TYPE, action_name) != nullptr);
  }
#endif

  if (action_name == "undo") {
    scene->history().undo();
  } else if (action_name == "redo") {
    scene->history().redo();
  } else if (action_name == "new") {
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    reset();
  } else if (action_name == "save") {
    save();
  } else if (action_name == "save as ...") {
    save_as();
  } else if (action_name == "open ...") {
    open();
  } else if (action_name == "export ...") {
    ExportDialog(*scene, main_window()).exec();
  } else if (action_name == "evaluate") {
    evaluate();
  } else if (action_name == "restore default layout") {
    main_window()->restore_default_layout();
  } else if (action_name == "save layout ...") {
    main_window()->save_layout();
  } else if (action_name == "load layout ...") {
    main_window()->load_layout();
  } else if (action_name == "new toolbar") {
    spawn_toolbar();
  } else if (action_name == "previous tool") {
    scene->tool_box().set_previous_tool();
  } else if (action_name == "new style") {
    using command_type = AddCommand<List<Style>>;
    auto style = scene->default_style().clone();
    assert(style->scene() == scene.get());
    scene->submit<command_type>(scene->styles(), std::move(style));
  } else if (action_name == "reset viewport") {
    main_window()->viewport().reset();
  } else if (action_name == "show point dialog") {
    if (const auto paths = scene->item_selection<Path>(); !paths.empty()) {
      PointDialog(paths, main_window()).exec();
    }
  } else if (action_name == "preferences") {
    PreferenceDialog().exec();
  } else if (omm::path_actions::perform_action(action_name, *this)) {
    // action was handled by perform_action.
  } else if (handle_mode(action_name)) {
    // action was handled as mode
  } else {
    for (const auto& key : Object::keys()) {
      if (key == action_name) {
        const auto modifiers = QApplication::keyboardModifiers();
        if ((modifiers & Qt::ControlModifier) != 0u) {
          insert_object(key, InsertionMode::AsChild);
        } else if ((modifiers & Qt::ShiftModifier) != 0u) {
          insert_object(key, InsertionMode::AsParent);
        } else {
          insert_object(key, InsertionMode::Default);
        }
        return true;
      }
    }
    for (const auto& key : Manager::keys()) {
      if (key == action_name) {
        spawn_manager(key);
        return true;
      }
    }
    for (const auto& key : Tool::keys()) {
      if (key == action_name) {
        scene->tool_box().set_active_tool(key);
        return true;
      }
    }
    for (const auto& key : Tag::keys()) {
      if (key == action_name) {
        const auto object_selection = scene->item_selection<Object>();
        if (!object_selection.empty()) {
          [[maybe_unused]] auto macro = scene->history().start_macro(tr("Add Tag"));
          for (auto&& object : object_selection) {
            using AddTagCommand = omm::AddCommand<omm::List<omm::Tag>>;
            scene->submit<AddTagCommand>(object->tags, Tag::make(key, *object));
          }
        }
        return true;
      }
    }
    return false;
  }
  return true;
}

bool Application::dispatch_key(int key, Qt::KeyboardModifiers modifiers, CommandInterface& ci)
{
  const auto dispatch_sequence = [this](CommandInterface& ci) {
    const auto action_name = key_bindings->find_action(ci.type(), m_pending_key_sequence);
    LINFO << "Dispatching " << m_pending_key_sequence << " to " << &ci;
    if (!action_name.isEmpty() && ci.perform_action(action_name)) {
      m_pending_key_sequence = QKeySequence();
      return true;
    } else {
      return false;
    }
  };

  m_pending_key_sequence = push_back(m_pending_key_sequence, key | static_cast<int>(modifiers));
  m_reset_keysequence_timer.start();

  if (dispatch_sequence(ci)) {
    LINFO << "Dispatched key sequence to proposed command interface.";
    return true;
  } else if (&ci != &Application::instance() && dispatch_sequence(Application::instance())) {
    LINFO << "Dispatched key sequence to application.";
    return true;
  } else {
    LINFO << "Pending key sequence: " << m_pending_key_sequence << ".";
    return false;
  }
}

bool Application::dispatch_key(int key, Qt::KeyboardModifiers modifiers)
{
  const QPoint mouse_position = QCursor::pos();
  for (Manager* manager : m_managers) {
    const QRect manager_rect(manager->mapToGlobal(manager->rect().topLeft()),
                             manager->mapToGlobal(manager->rect().bottomRight()));
    if (manager_rect.contains(mouse_position)) {
      return dispatch_key(key, modifiers, *manager);
    }
  }

  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  return dispatch_key(key, modifiers, Application::instance());
}

QString Application::type() const
{
  return TYPE;
}

bool Application::handle_mode(const QString& action_name)
{
  for (auto&& [name, mode_selector] : mode_selectors) {
    if (action_name.startsWith(name + ".")) {
      return mode_selector->handle(action_name);
    }
  }
  return false;
}

MailBox& Application::mail_box() const
{
  return scene->mail_box();
}

MainWindow* Application::main_window() const
{
  return m_main_window;
}

Object& Application::insert_object(const QString& key, InsertionMode mode)
{
  const auto translated_key = QApplication::translate("any-context", key.toUtf8().constData());
  auto macro = scene->history().start_macro(tr("Create %1").arg(translated_key));
  using add_command_type = AddCommand<ObjectTree>;
  auto object = Object::make(key, scene.get());
  object->set_object_tree(scene->object_tree());
  auto& ref = *object;

  Object* parent = nullptr;
  Object* predecessor = nullptr;
  std::vector<Object*> children;
  switch (mode) {
  case InsertionMode::AsChild:
    if (const auto selection = scene->item_selection<Object>(); selection.size() == 1) {
      parent = *selection.begin();
    }
    break;
  case InsertionMode::AsParent: {
    auto selection = scene->item_selection<Object>();
    Object::remove_internal_children(selection);
    children = Object::sort(selection);
    parent = children.empty() ? &scene->object_tree().root() : &children.back()->tree_parent();
    if (!children.empty()) {
      if (std::size_t pos = children.back()->position(); pos > 0) {
        predecessor = parent->tree_children()[pos - 1];
      }
    }
    break;
  }
  default:
    break;
  }

  scene->submit<add_command_type>(scene->object_tree(), std::move(object));
  ref.set_global_transformation(ObjectTransformation(), Space::Scene);
  using move_command_t = MoveCommand<ObjectTree>;
  using move_context_t = move_command_t::context_type;
  if (!children.empty()) {
    if (parent == nullptr) {
      // it's illogical to be parent of no children.
      // This assertion is enforced by the conditions before.
      LFATAL("Unexpected Condition.");
    }
    const auto move_contextes = ::transform<move_context_t>(children, [&ref](auto* c) {
      return move_context_t(*c, ref, nullptr);
    });
    scene->submit<move_command_t>(scene->object_tree(), move_contextes);
    move_context_t move_context(ref, *parent, predecessor);
    if (move_context.is_strictly_valid(scene->object_tree())) {
      // the move will fail if the object is already at the correct position.
      scene->submit<move_command_t>(scene->object_tree(), std::vector{move_context});
    }
  } else if (parent != nullptr) {
    const move_context_t move_context(ref, *parent, nullptr);
    scene->submit<move_command_t>(scene->object_tree(), std::vector{move_context});
  }

  ref.post_create_hook();
  return ref;
}

Manager& Application::spawn_manager(const QString& type)
{
  auto manager = Manager::make(type, *scene);
  main_window()->assign_unique_objectname(*manager);
  auto& ref = *manager;
  main_window()->addDockWidget(Qt::TopDockWidgetArea, manager.release());
  ref.setFloating(true);
  return ref;
}

ToolBar& Application::spawn_toolbar() const
{
  auto toolbar = std::make_unique<ToolBar>();
  main_window()->assign_unique_objectname(*toolbar);
  auto& ref = *toolbar;
  main_window()->addToolBar(toolbar.release());
  return ref;
}

Manager& Application::get_active_manager(const QString& type)
{
  for (Manager* m : Application::instance().managers(type)) {
    if (m->is_visible() && !m->is_locked()) {
      return *m;
    }
  }
  return spawn_manager(type);
}

void Application::register_manager(Manager& manager)
{
  m_managers.insert(&manager);
}

void Application::unregister_manager(Manager& manager)
{
  m_managers.erase(&manager);
}

std::set<Manager*> Application::managers(const QString& type) const
{
  std::set<Manager*> managers;
  for (Manager* m : m_managers) {
    if (m->type() == type) {
      managers.insert(m);
    }
  }
  return managers;
}

const Preferences& preferences()
{
  return *Application::instance().preferences;
}

}  // namespace omm

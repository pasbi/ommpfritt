#include "application.h"

#include <cassert>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>

#include "mainwindow/mainwindow.h"
#include "managers/manager.h"
#include "scene/scene.h"
#include "commands/addcommand.h"
#include "mainwindow/actions.h"
#include "objects/object.h"
#include "tools/toolbox.h"
#include "tags/tag.h"
#include "mainwindow/viewport/viewport.h"
#include "mainwindow/exportdialog.h"
#include "logging.h"
#include "widgets/pointdialog.h"
#include "commands/movecommand.h"
#include "scene/history/historymodel.h"
#include "scene/stylelist.h"
#include "managers/manager.h"
#include "preferences/preferencedialog.h"

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

}  // namespace

namespace omm
{

const std::set<int> Application::keyboard_modifiers { Qt::Key_Shift, Qt::Key_Control, Qt::Key_Alt,
                                                      Qt::Key_Meta };
Application* Application::m_instance = nullptr;

Application::Application(QApplication& app)
  : scene(python_engine)
  , m_app(app)
{
  if (m_instance == nullptr) {
    m_instance = this;
  } else {
    LFATAL("Resetting application instance.");
  }
  scene.set_selection({});

  m_app.setPalette(ui_colors.make_palette());;

  m_reset_keysequence_timer.setSingleShot(true);
  m_reset_keysequence_timer.setInterval(1000);
  connect(&m_reset_keysequence_timer, &QTimer::timeout, this, [this]() {
    m_pending_key_sequence = QKeySequence();
  });
}

Application& Application::instance()
{
  assert(m_instance != nullptr);
  return *m_instance;
}

void Application::set_main_window(MainWindow& main_window)
{
  m_main_window = &main_window;
}

void Application::quit()
{
  if (can_close()) {
    LINFO << "Quit application.";
    m_app.quit();
  } else {
    LINFO << "Aborted quit.";
  }
}

void Application::update_undo_redo_enabled()
{
}

bool Application::can_close()
{
  if (scene.history().has_pending_changes()) {
    const auto decision =
      QMessageBox::question( m_main_window,
                             tr("Question."),
                             tr("Some pending changes will be lost if you don't save."
                                 "What do you want me to do?"),
                             QMessageBox::Close | QMessageBox::Cancel | QMessageBox::Save,
                             QMessageBox::Save );
    switch (decision) {
    case QMessageBox::Close: return true;
    case QMessageBox::Cancel: return false;
    case QMessageBox::Save: return save();
    default:
      qCritical() << "Unexpected response code: " << decision;
      return false;
    }
  } else {
    return true;
  }
}

bool Application::save(const std::string& filename)
{
  if (!scene.save_as(filename)) {
    LWARNING << "Error saving scene as '" << filename << "'.";
    QMessageBox::critical( m_main_window,
                           tr("Error."),
                           tr("The scene could not be saved at '%1'.")
                            .arg(QString::fromStdString(filename)),
                           QMessageBox::Ok, QMessageBox::Ok );
    return false;
  } else {
    return true;
  }
}

bool Application::save()
{
  const std::string filename = scene.filename();
  if (filename.empty()) {
    return save_as();
  } else {
    return save(filename);
  }
}

bool Application::save_as()
{
  LINFO << m_main_window;
  QFileDialog dialog(m_main_window);
  dialog.setWindowTitle(tr("Save scene as ..."));
  dialog.setDirectoryUrl(QString::fromStdString(scene.filename()));
  dialog.setDefaultSuffix(FILE_ENDING);
  if (dialog.exec() == QDialog::Accepted) {
    const auto files = dialog.selectedFiles();
    assert(files.size() == 1);
    return scene.save_as(files.front().toStdString());
  } else {
    return false;
  }
}

void Application::reset()
{
  if (!can_close()) { return; }

  LINFO << "reset scene.";
  scene.set_selection({});
  QTimer::singleShot(0, &scene, SLOT(reset()));
}

void Application::load()
{
  if (!can_close()) {
    return;
  }

  const QString filename =
    QFileDialog::getOpenFileName( m_main_window,
                                  tr("Load scene ..."),
                                  QString::fromStdString(scene.filename()) );
  if (filename.isEmpty()) {
    return;
  }

  scene.set_selection({});
  QTimer::singleShot(0, [this, filename]() {
    if (!scene.load_from(filename.toStdString())) {
      QMessageBox::critical( m_main_window,
                             tr("Error."),
                             tr("Loading scene from '%1' failed.").arg(filename),
                             QMessageBox::Ok );
    }
  });
}

bool Application::perform_action(const std::string& action_name)
{
  if (action_name == QT_TR_NOOP("undo")) {
    scene.history().undo();
  } else if (action_name == QT_TR_NOOP("redo")) {
    scene.history().redo();
  } else if (action_name == QT_TR_NOOP("remove selection")) {
    scene.remove(main_window(), scene.selection());
  } else if (action_name == QT_TR_NOOP("new document")) {
    reset();
  } else if (action_name == QT_TR_NOOP("save document")) {
    save();
  } else if (action_name == QT_TR_NOOP("save document as")) {
    save_as();
  } else if (action_name == QT_TR_NOOP("load document")) {
    load();
  } else if (action_name == QT_TR_NOOP("export")) {
    static ExportDialog* export_dialog = nullptr;
    if (export_dialog == nullptr) {
      export_dialog = new omm::ExportDialog(scene, main_window());
    }
    export_dialog->exec();
  } else if (action_name == QT_TR_NOOP("make smooth")) {
    actions::make_smooth(*this);
  } else if (action_name == QT_TR_NOOP("make linear")) {
    actions::make_linear(*this);
  } else if (action_name == QT_TR_NOOP("remove points")) {
    actions::remove_selected_points(*this);
  } else if (action_name == QT_TR_NOOP("subdivide")) {
    actions::subdivide(*this);
  } else if (action_name == QT_TR_NOOP("evaluate")) {
    actions::evaluate(*this);
  } else if (action_name == QT_TR_NOOP("restore default layout")) {
    main_window()->restore_default_layout();
  } else if (action_name == QT_TR_NOOP("save layout ...")) {
    main_window()->save_layout();
  } else if (action_name == QT_TR_NOOP("load layout ...")) {
    main_window()->load_layout();
  } else if (action_name == QT_TR_NOOP("switch between object and point selection")) {
    scene.tool_box().switch_between_object_and_point_selection();
  } else if (action_name == QT_TR_NOOP("previous tool")) {
    scene.tool_box().set_previous_tool();
  } else if (action_name == QT_TR_NOOP("select all")) {
    actions::select_all(*this);
  } else if (action_name == QT_TR_NOOP("deselect all")) {
    actions::deselect_all(*this);
  } else if (action_name == QT_TR_NOOP("invert selection")) {
    actions::invert_selection(*this);
  } else if (action_name == QT_TR_NOOP("new style")) {
      using command_type = AddCommand<List<Style>>;
      auto style = scene.default_style().clone();
      assert(style->scene() == &scene);
      scene.submit<command_type>(scene.styles(), std::move(style));
  } else if (action_name == QT_TR_NOOP("convert objects")) {
    actions::convert_objects(*this);
  } else if (action_name == QT_TR_NOOP("reset viewport")) {
    main_window()->viewport().reset();
  } else if (action_name == QT_TR_NOOP("show point dialog")) {
    const auto paths = Object::cast<Path>(scene.item_selection<Object>());
    if (paths.size() > 0) {
      PointDialog(paths, main_window()).exec();
    }
  } else if (action_name == QT_TR_NOOP("preferences")) {
    PreferenceDialog().exec();
  } else {
    for (const auto& key : Object::keys()) {
      if (key == action_name) {
        const auto modifiers = QApplication::keyboardModifiers();
        if (modifiers & Qt::ControlModifier) {
          insert_object(key, InsertionMode::AsChild);
        } else if (modifiers & Qt::ShiftModifier) {
          insert_object(key, InsertionMode::AsParent);
        } else {
          insert_object(key, InsertionMode::Default);
        }
        return true;
      }
    }
    for (const auto& key : Manager::keys()) {
      if (key == action_name) {
        auto manager = Manager::make(key, scene);
        main_window()->make_unique_manager_name(*manager);
        auto& ref = *manager;
        main_window()->addDockWidget(Qt::TopDockWidgetArea, manager.release());
        ref.setFloating(true);
        return true;
      }
    }
    for (const auto& key : Tool::keys()) {
      if (key == action_name) {
        scene.tool_box().set_active_tool(key);
        return true;
      }
    }
    for (const auto& key : Tag::keys()) {
      if (key == action_name) {
        const auto object_selection = scene.item_selection<Object>();
        if (!object_selection.empty()) {
          auto macro = scene.history().start_macro(tr("Add Tag"));
          for (auto&& object : object_selection) {
            using AddTagCommand = omm::AddCommand<omm::List<omm::Tag>>;
            scene.submit<AddTagCommand>(object->tags, Tag::make(key, *object));
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
    LINFO << m_pending_key_sequence;
    const auto action_name = key_bindings.find_action(ci.type(), m_pending_key_sequence);
    if (!action_name.empty() && ci.perform_action(action_name)) {
      m_pending_key_sequence = QKeySequence();
      return true;
    } else {
      return false;
    }
  };

  m_pending_key_sequence = push_back(m_pending_key_sequence, key | modifiers);
  m_reset_keysequence_timer.start();
  if (dispatch_sequence(ci)) {
    return true;
  } else if (&ci != &Application::instance()) {
    return dispatch_sequence(Application::instance());
  } else {
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
  return dispatch_key(key, modifiers, Application::instance());
}

std::string Application::type() const { return TYPE; }

MessageBox &Application::message_box()
{
  return scene.message_box();
}

MainWindow* Application::main_window() const { return m_main_window; }

Object& Application::insert_object(const std::string &key, InsertionMode mode)
{
  auto macro = scene.history().start_macro(tr("Create %1")
                  .arg(QApplication::translate("any-context", key.c_str())));
  using add_command_type = AddCommand<ObjectTree>;
  auto object = Object::make(key, &scene);
  object->set_object_tree(scene.object_tree());
  auto& ref = *object;

  Object* parent = nullptr;
  Object* predecessor = nullptr;
  std::vector<Object*> children;
  switch (mode) {
  case InsertionMode::AsChild:
    if (const auto selection = scene.item_selection<Object>(); selection.size() == 1) {
      parent = *selection.begin();
    }
    break;
  case InsertionMode::AsParent: {
    auto selection = scene.item_selection<Object>();
    Object::remove_internal_children(selection);
    children = Object::sort(selection);
    parent = children.empty() ? &scene.object_tree().root() : &children.back()->tree_parent();
    if (!children.empty()) {
      if (std::size_t pos = children.back()->position(); pos > 0) {
        predecessor = parent->tree_children()[pos-1];
      }
    }
    break;
  }
  default:
    break;
  }

  scene.submit<add_command_type>(scene.object_tree(), std::move(object));
  ref.set_global_transformation(ObjectTransformation(), Space::Scene);
  using move_command_t = MoveCommand<ObjectTree>;
  using move_context_t = move_command_t::context_type;
  if (!children.empty()) {
    const auto move_contextes = ::transform<move_context_t>(children, [&ref](auto* c) {
      return move_context_t(*c, ref, nullptr);
    });
    scene.submit<move_command_t>(scene.object_tree(), move_contextes);
    move_context_t move_context(ref, *parent, predecessor);
    if (move_context.is_strictly_valid(scene.object_tree())) {
      // the move will fail if the object is already at the correct position.
      scene.submit<move_command_t>(scene.object_tree(), std::vector { move_context });
    }
  } else if (parent != nullptr) {
    const move_context_t move_context(ref, *parent, nullptr);
    scene.submit<move_command_t>(scene.object_tree(), std::vector { move_context });
  }

  ref.post_create_hook();
  return ref;
}

void Application::register_manager(Manager& manager)
{
  m_managers.insert(&manager);
}

void Application::unregister_manager(Manager& manager)
{
  m_managers.erase(&manager);
}

}  // namespace omm

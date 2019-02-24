#include "application.h"

#include <glog/logging.h>
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
#include "keybindings/keybindingsdialog.h"

namespace {
constexpr auto FILE_ENDING = ".omm";
}

namespace omm
{

Application* Application::m_instance = nullptr;

Application::Application(QApplication& app)
  : m_app(app)
  , scene(python_engine)
{
  if (m_instance == nullptr) {
    m_instance = this;
  } else {
    LOG(FATAL) << "Resetting application instance.";
  }
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
    LOG(INFO) << "Quit application.";
    m_app.quit();
  } else {
    LOG(INFO) << "Aborted quit.";
  }
}

void Application::update_undo_redo_enabled()
{
}

bool Application::can_close()
{
  if (scene.has_pending_changes()) {
    const auto decision =
      QMessageBox::question( m_main_window,
                             tr("Question."),
                             tr("Some pending changes will be lost if you don't save."
                                 "What do you want me to do?"),
                             QMessageBox::Close | QMessageBox::Cancel | QMessageBox::Save,
                             QMessageBox::Save );
    switch (decision) {
    case QMessageBox::Close:
      return true;
    case QMessageBox::Cancel:
      return false;
    case QMessageBox::Save:
      return save();
    default:
      LOG(ERROR) << "Unexpected response code: " << decision;
      return false;
    }
  } else {
    return true;
  }
}

bool Application::save(const std::string& filename)
{
  if (!scene.save_as(filename)) {
    LOG(WARNING) << "Error saving scene as '" << filename << "'.";
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
  LOG(INFO) << m_main_window;
  const QString filename =
    QFileDialog::getSaveFileName( m_main_window,
                                  tr("Save scene as ..."),
                                  QString::fromStdString(scene.filename()) );
  if (filename.isEmpty()) {
    return false;
  } else {
    return scene.save_as(filename.toStdString());
  }
}

void Application::reset()
{
  LOG(INFO) << "reset scene.";
  scene.reset();
}

bool Application::load()
{
  const QString filename =
    QFileDialog::getOpenFileName( m_main_window,
                                  tr("Load scene ..."),
                                  QString::fromStdString(scene.filename()) );

  if (filename.isEmpty()) {
    return false;
  } else if (scene.load_from(filename.toStdString())) {
    return true;
  } else {
    QMessageBox::critical( m_main_window,
                           tr("Error."),
                           tr("Loading scene from '%1' failed.").arg(filename),
                           QMessageBox::Ok );
    return false;
  }
}

std::vector<CommandInterface::ActionInfo<Application>> Application::action_infos()
{
  const auto ks = [](auto&&... code) { return QKeySequence(code...); };
  using AI = ActionInfo<Application>;
  std::list infos {
    AI( QT_TRANSLATE_NOOP("any-context", "undo"),             ks("Ctrl+Z"),
        [](Application& app) { app.scene.undo_stack.undo(); } ),
    AI( QT_TRANSLATE_NOOP("any-context", "redo"),             ks("Ctrl+Y"),
        [](Application& app) { app.scene.undo_stack.redo(); } ),
    AI( QT_TRANSLATE_NOOP("any-context", "remove selection"), ks("Ctrl+Del"),
        [](Application& app) { app.scene.remove(app.main_window(), app.scene.selection()); } ),
    AI( QT_TRANSLATE_NOOP("any-context", "new document"),     ks("Ctrl+N"),
        [](Application& app) { app.reset(); } ),
    AI( QT_TRANSLATE_NOOP("any-context", "save document"),    ks("Ctrl+S"),
        [](Application& app) {} ),
    AI( QT_TRANSLATE_NOOP("any-context", "save document as"), ks("Ctrl+Shfit+S"),
        [](Application& app) { app.save_as(); } ),
    AI( QT_TRANSLATE_NOOP("any-context", "load document"),    ks("Ctrl+O"),
        [](Application& app) { app.load(); } ),
    AI( QT_TRANSLATE_NOOP("any-context", "make smooth"),      ks(), &actions::make_smooth),
    AI( QT_TRANSLATE_NOOP("any-context", "make linear"),      ks(), &actions::make_linear),
    AI( QT_TRANSLATE_NOOP("any-context", "remove points"),    ks("Del"),
        &actions::remove_selected_points ),
    AI( QT_TRANSLATE_NOOP("any-context", "subdivide"),        ks(), &actions::subdivide),
    AI( QT_TRANSLATE_NOOP("any-context", "evaluate"),         ks(), &actions::evaluate),
    AI( QT_TRANSLATE_NOOP("any-context", "show keybindings dialog"), ks(),
        [](Application& app) { KeyBindingsDialog(app.key_bindings, app.main_window()).exec(); } ),
    AI( QT_TRANSLATE_NOOP("any-context", "previous tool"),    ks("Space"),
        [](Application& app) { app.scene.tool_box.set_previous_tool(); } ),
    AI( QT_TRANSLATE_NOOP("any-context", "select all"),       ks("A"), &actions::select_all),
    AI( QT_TRANSLATE_NOOP("any-context", "deselect all"),     ks(), &actions::deselect_all),
    AI( QT_TRANSLATE_NOOP("any-context", "invert selection"), ks("I"),
        &actions::invert_selection),
    AI( QT_TRANSLATE_NOOP("any-context", "new style"),        ks(), [](Application& app) {
          using command_type = AddCommand<List<Style>>;
          app.scene.submit<command_type>(app.scene.styles, app.scene.default_style().clone());
        } ),
    AI( QT_TRANSLATE_NOOP("any-context", "convert objects"),  ks("C"),
        &actions::convert_objects ),
  };

  for (const auto& key : Object::keys()) {
    infos.push_back(AI(key, ks(), [key](Application& app) {
      using add_command_type = AddCommand<Tree<Object>>;
      Scene& scene = app.scene;
      scene.submit<add_command_type>(scene.object_tree, Object::make(key, &scene));
    }));
  }
  for (const auto& key : Manager::keys()) {
    infos.push_back(AI(key, ks(), [key](Application& app) {
      Scene& scene = app.scene;
      auto manager = Manager::make(key, scene);
      auto& ref = *manager;
      app.main_window()->addDockWidget(Qt::TopDockWidgetArea, manager.release());
      ref.setFloating(true);
    }));
  }
  for (const auto& key : Tool::keys()) {
    infos.push_back(AI(key, ks(), [key](Application& app) {
      app.scene.tool_box.set_active_tool(key);
    }));
  }
  for (const auto& key : Tag::keys()) {
    infos.push_back(AI(key, ks(), [key](Application& app) {
      Scene& scene = app.scene;
      scene.undo_stack.beginMacro(tr("Add Tag"));
      for (auto&& object : scene.object_selection()) {
        using AddTagCommand = omm::AddCommand<omm::List<omm::Tag>>;
        scene.submit<AddTagCommand>(object->tags, Tag::make(key, *object));
      }
      scene.undo_stack.endMacro();
    }));
  }
  return std::vector(infos.begin(), infos.end());
}

std::string Application::type() const { return TYPE; }
MainWindow* Application::main_window() const { return m_main_window; }

}  // namespace omm

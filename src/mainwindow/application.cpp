#include "application.h"

#include <glog/logging.h>
#include <cassert>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>

#include "keybindings/defaultkeysequenceparser.h"
#include "mainwindow/mainwindow.h"
#include "managers/manager.h"
#include "scene/scene.h"
#include "commands/addcommand.h"
#include "mainwindow/actions.h"
#include "objects/object.h"
#include "tools/toolbox.h"
#include "tags/tag.h"
#include "keybindings/keybindingsdialog.h"
#include "mainwindow/viewport/viewport.h"
#include "mainwindow/exportdialog.h"

namespace {
constexpr auto FILE_ENDING = ".omm";

void show_export_dialog(omm::Application& app)
{
  static std::unique_ptr<QDialog> export_dialog;
  if (!export_dialog) {
     export_dialog = std::make_unique<omm::ExportDialog>(app.scene, app.main_window());
  }
  export_dialog->exec();
}

}

namespace omm
{

Application* Application::m_instance = nullptr;

Application::Application(QApplication& app)
  : scene(python_engine)
  , m_app(app)
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
    case QMessageBox::Close: return true;
    case QMessageBox::Cancel: return false;
    case QMessageBox::Save: return save();
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
  if (!can_close()) { return; }

  LOG(INFO) << "reset scene.";
  scene.reset();
}

bool Application::load()
{
  if (!can_close()) { return false; }

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
  DefaultKeySequenceParser parser("://default_keybindings.cfg");
  const auto ai = [parser](const std::string& name, const std::function<void(Application&)>& f) {
    return ActionInfo(name, parser.get_key_sequence(name), f);
  };
  std::list infos {
    ai( QT_TRANSLATE_NOOP("any-context", "undo"), [](Application& app) {
      app.scene.undo_stack.undo(); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "redo"), [](Application& app) {
      app.scene.undo_stack.redo(); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "remove selection"), [](Application& app) {
      app.scene.remove(app.main_window(), app.scene.selection()); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "new document"), [](Application& app) { app.reset(); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "save document"), [](Application& app) { app.save(); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "save document as"), [](Application& app) {
      app.save_as(); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "load document"), [](Application& app) { app.load(); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "export"), show_export_dialog),
    ai( QT_TRANSLATE_NOOP("any-context", "make smooth"), &actions::make_smooth),
    ai( QT_TRANSLATE_NOOP("any-context", "make linear"), &actions::make_linear),
    ai( QT_TRANSLATE_NOOP("any-context", "remove points"), &actions::remove_selected_points),
    ai( QT_TRANSLATE_NOOP("any-context", "subdivide"), &actions::subdivide),
    ai( QT_TRANSLATE_NOOP("any-context", "evaluate"), &actions::evaluate),
    ai( QT_TRANSLATE_NOOP("any-context", "show keybindings dialog"), [](Application& app) {
      KeyBindingsDialog(app.key_bindings, app.main_window()).exec(); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "restore default layout"), [](Application& app) {
      app.main_window()->restore_default_layout(); }),
    ai( QT_TRANSLATE_NOOP("any-context", "switch between object and point selection"),
        [](Application& app) { app.scene.tool_box.switch_between_object_and_point_selection(); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "previous tool"), [](Application& app) {
      app.scene.tool_box.set_previous_tool(); } ),
    ai( QT_TRANSLATE_NOOP("any-context", "select all"), &actions::select_all),
    ai( QT_TRANSLATE_NOOP("any-context", "deselect all"), &actions::deselect_all),
    ai( QT_TRANSLATE_NOOP("any-context", "invert selection"), &actions::invert_selection),
    ai( QT_TRANSLATE_NOOP("any-context", "new style"), [](Application& app) {
        using command_type = AddCommand<List<Style>>;
        app.scene.submit<command_type>(app.scene.styles, app.scene.default_style().clone());
      } ),
    ai( QT_TRANSLATE_NOOP("any-context", "convert objects"), &actions::convert_objects ),
    ai( QT_TRANSLATE_NOOP("any-context", "reset viewport"), [](Application& app) {
        app.main_window()->viewport().reset();
      })
  };

  for (const auto& key : Object::keys()) {
    infos.push_back(ai(key, [key](Application& app) {
      using add_command_type = AddCommand<Tree<Object>>;
      Scene& scene = app.scene;
      auto object = Object::make(key, &scene);
      scene.submit<add_command_type>(scene.object_tree, std::move(object));
    }));
  }
  for (const auto& key : Manager::keys()) {
    infos.push_back(ai(key, [key](Application& app) {
      Scene& scene = app.scene;
      auto manager = Manager::make(key, scene);
      auto& ref = *manager;
      app.main_window()->addDockWidget(Qt::TopDockWidgetArea, manager.release());
      ref.setFloating(true);
    }));
  }
  for (const auto& key : Tool::keys()) {
    infos.push_back(ai(key, [key](Application& app) {
      app.scene.tool_box.set_active_tool(key);
    }));
  }
  for (const auto& key : Tag::keys()) {
    infos.push_back(ai(key, [key](Application& app) {
      Scene& scene = app.scene;
      scene.undo_stack.beginMacro(tr("Add Tag"));
      for (auto&& object : scene.item_selection<Object>()) {
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

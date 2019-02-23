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
  key_bindings.set_global_command_interface(*this);
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

std::map<std::string, QKeySequence> Application::default_bindings()
{
  const auto ks = [](auto&&... code) { return QKeySequence(code...); };
  std::map<std::string, QKeySequence> map {
    { QT_TRANSLATE_NOOP("any-context", "undo"),                    ks(tr("Ctrl+Z"))       },
    { QT_TRANSLATE_NOOP("any-context", "redo"),                    ks(tr("Ctrl+Y"))       },
    { QT_TRANSLATE_NOOP("any-context", "new document"),            ks(tr("Ctrl+N"))       },
    { QT_TRANSLATE_NOOP("any-context", "save document"),           ks(tr("Ctrl+S"))       },
    { QT_TRANSLATE_NOOP("any-context", "save document as"),        ks(tr("Ctrl+Shift+S")) },
    { QT_TRANSLATE_NOOP("any-context", "load document"),           ks(tr("Ctrl+O"))       },
    { QT_TRANSLATE_NOOP("any-context", "make smooth"),             ks()                   },
    { QT_TRANSLATE_NOOP("any-context", "make linear"),             ks()                   },
    { QT_TRANSLATE_NOOP("any-context", "remove points"),           ks(tr("Del"))          },
    { QT_TRANSLATE_NOOP("any-context", "subdivide"),               ks()                   },
    { QT_TRANSLATE_NOOP("any-context", "evaluate"),                ks()                   },
    { QT_TRANSLATE_NOOP("any-context", "show keybindings dialog"), ks()                   },
    { QT_TRANSLATE_NOOP("any-context", "previous tool"),           ks(tr("Space"))        },
    { QT_TRANSLATE_NOOP("any-context", "select all"),              ks(tr("A"))            },
    { QT_TRANSLATE_NOOP("any-context", "deselect all"),            ks()                   },
    { QT_TRANSLATE_NOOP("any-context", "invert selection"),        ks(tr("I"))            },
    { QT_TRANSLATE_NOOP("any-context", "remove selection"),        ks(tr("Ctrl+Del"))     },
    { QT_TRANSLATE_NOOP("any-context", "new style"),               ks(tr(""))             },
    { QT_TRANSLATE_NOOP("any-context", "convert objects"),         ks(tr("C"))            },
  };

  for (const auto& key : Object::keys()) {
    map.insert(std::pair(key, ks()));
  }
  for (const auto& key : Manager::keys()) {
    map.insert(std::pair(key, ks()));
  }
  for (const auto& key : Tool::keys()) {
    map.insert(std::pair(key, ks()));
  }
  for (const auto& key : Tag::keys()) {
    map.insert(std::pair(key, ks()));
  }

  LOG(INFO) << "DELETE SEQUENCE: " << map["remove selection"].toString().toStdString();

  return map;
}

// TODO bind all actions to `&actions::...`. Then this method can be made static.
void Application::call(const std::string& command)
{
  const auto save = static_cast<bool(Application::*)()>(&Application::save);

  Dispatcher map {
    { "undo",                    std::bind(&QUndoStack::undo, &scene.undo_stack) },
    { "redo",                    std::bind(&QUndoStack::redo, &scene.undo_stack) },
    { "new document",            std::bind(&Application::reset, this) },
    { "save document",           std::bind(save, this) },
    { "save document as",        std::bind(&Application::save_as, this) },
    { "load document",           std::bind(&Application::load, this) },
    { "make smooth",             &actions::make_smooth },
    { "make linear",             &actions::make_linear },
    { "remove points",           &actions::remove_selected_points },
    { "subdivide",               &actions::subdivide },
    { "evaluate",                &actions::evaluate },
    { "show keybindings dialog", &actions::show_keybindings_dialog },
    { "previous tool",           &actions::previous_tool },
    { "select all",              &actions::select_all },
    { "deselect all",            &actions::deselect_all },
    { "invert selection",        &actions::invert_selection },
    { "remove selection",        &actions::remove_selection },
    { "convert objects",         &actions::convert_objects },
    { "new style",               &actions::new_style },
  };

  for (const auto& key : Object::keys()) {
    map.insert(std::pair(key, [this, key](){
      using add_command_type = AddCommand<Tree<Object>>;
      scene.submit<add_command_type>(scene.object_tree, Object::make(key, &scene));
    }));
  }

  for (const auto& key : Manager::keys()) {
    map.insert(std::pair(key, [this, key](){
      auto manager = Manager::make(key, scene);
      auto& ref = *manager;
      m_main_window->addDockWidget(Qt::TopDockWidgetArea, manager.release());
      ref.setFloating(true);
    }));
  }

  for (const auto& key : Tool::keys()) {
    map.insert(std::pair(key, [this, key](){
      scene.tool_box.set_active_tool(key);
    }));
  }

  for (const auto& key : Tag::keys()) {
    map.insert(std::pair(key, [this, key]() {
      scene.undo_stack.beginMacro(tr("Add Tag"));
      for (auto&& object : scene.object_selection()) {
        using AddTagCommand = omm::AddCommand<omm::List<omm::Tag>>;
        scene.submit<AddTagCommand>(object->tags, Tag::make(key, *object));
      }
      scene.undo_stack.endMacro();
    }));
  }

  dispatch(command, map);
}

std::string Application::type() const { return TYPE; }
MainWindow* Application::main_window() const { return m_main_window; }

}  // namespace omm

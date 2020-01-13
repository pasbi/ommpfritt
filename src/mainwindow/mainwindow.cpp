#include "mainwindow/mainwindow.h"

#include <memory>

#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QSettings>
#include <QCloseEvent>
#include <functional>
#include <QMessageBox>
#include <QDirIterator>
#include <QLocale>
#include <QFileDialog>

#include "scene/messagebox.h"
#include "tools/tool.h"
#include "managers/timeline/timeline.h"
#include "mainwindow/viewport/viewport.h"
#include "mainwindow/application.h"
#include "mainwindow/toolbar/toolbar.h"
#include "managers/manager.h"
#include "tags/tag.h"
#include "logging.h"
#include "ui_aboutdialog.h"
#include "scene/history/historymodel.h"
#include "keybindings/commandinterface.h"
#include "mainwindow/application.h"
#include "widgets/colorwidget/colorwidget.h"

namespace
{

QMenu* find_menu(QMenu* menu, const QString& object_name)
{
  if (menu == nullptr) {
    return nullptr;
  } else if (menu->objectName() == object_name) {
    return menu;
  } else {
    for (QAction* action : menu->actions()) {
      QMenu* sub_menu = find_menu(action->menu(), object_name);
      if (sub_menu != nullptr) {
        return sub_menu;
      }
    }
    return nullptr;
  }
}

QMenu* find_menu(QMenuBar* bar, const QString& object_name)
{
  for (auto&& action : bar->actions()) {
    if (auto* menu = find_menu(action->menu(), object_name); menu != nullptr) {
      return menu;
    }
  }
  return nullptr;
}

QString make_unique(const QString& base, const std::set<QString>& blacklist)
{
  std::size_t i = 0;
  QString name = base;
  while (::contains(blacklist, name)) {
    name = base + QString("_%1").arg(i);
    i++;
  }
  return name;
}

QString get_object_name(const QObject* object)
{
  return object->objectName();
}

}  // namespace

namespace omm
{

std::vector<QString> MainWindow::object_menu_entries()
{
  std::list<QString> entries {
    "object/remove selection",
    "object/convert objects",
  };

  for (const QString& key : Object::keys()) {
    entries.push_back("object/create/" + key);
  }

  for (const QString& key : Tag::keys()) {
    entries.push_back("object/attach/" + key);
  }

  return std::vector(entries.begin(), entries.end());
}

std::vector<QString> MainWindow::path_menu_entries()
{
  return std::vector<QString> {
    "path/make smooth",
    "path/make linear",
    "path/subdivide",
    "path/select all",
    "path/deselect all",
    "path/invert selection",
    "path/show point dialog",
  };
}

std::vector<QString> MainWindow::main_menu_entries()
{
  std::list<QString> entries = {
    QT_TRANSLATE_NOOP("menu_name", "file")"/new document",
    "file/save document",
    "file/save document as",
    "file/load document",
    "file/" QT_TRANSLATE_NOOP("menu_name", "load recent document") "/",
    QStringLiteral("file/") + KeyBindings::SEPARATOR,
    "file/export",
    QT_TRANSLATE_NOOP("menu_name", "edit")"/undo",
    "edit/redo",
    "edit/preferences",
    QT_TRANSLATE_NOOP("menu_name", "object")"/new style",
    "object/" QT_TRANSLATE_NOOP("menu_name", "create")"/",
    "object/" QT_TRANSLATE_NOOP("menu_name", "attach")"/",
    QT_TRANSLATE_NOOP("menu_name", "path")"/",
    QT_TRANSLATE_NOOP("menu_name", "tool")"/previous tool",
    QStringLiteral("tool/") + KeyBindings::SEPARATOR,
    QT_TRANSLATE_NOOP("menu_name", "scene")"/evaluate",
    "scene/reset viewport",
    QT_TRANSLATE_NOOP("menu_name", "window")"/" QT_TRANSLATE_NOOP("menu_name", "show")"/",
  };

  const auto merge = [&es=entries](auto&& ls) { es.insert(es.end(), ls.begin(), ls.end()); };

  for (const QString& key : Manager::keys()) {
    entries.push_back("window/show/" + key);
  }
  entries.insert(entries.end(), {
    QStringLiteral("window/") + KeyBindings::SEPARATOR,
    "window/restore default layout",
    "window/new toolbar",
#ifndef NDEBUG  // these functions facialiate the creation of profiles.
                // they should not be used by the user since the file content is not validated.
    "window/save layout ...",
    "window/load layout ...",
#endif  // NDEBUG
  });
  merge(object_menu_entries());
  merge(path_menu_entries());
  for (const QString& key : Tool::keys()) {
    entries.push_back("tool/" + key);
  }

  return std::vector(entries.begin(), entries.end());
}

void MainWindow::update_window_title()
{
  QString filename = m_app.scene.filename();
  static const QString clean_indicator = tr("");
  static const QString dirty_indicator = tr("*");
  QString indicator = m_app.scene.history().has_pending_changes()
                      ? dirty_indicator : clean_indicator;
  if (filename.isEmpty()) {
    filename = tr("unnamed");
    indicator = clean_indicator;  // never show dirty indicator if no filename is set.
  } else {
    filename = QFileInfo(filename).fileName();
  }
  setWindowTitle(QString("%1%2 - omm").arg(filename).arg(indicator));
}

MainWindow::MainWindow(Application& app)
  : m_app(app)
{
  setDockNestingEnabled(true);
  auto viewport = std::make_unique<Viewport>(app.scene);
  m_viewport = viewport.get();
  setCentralWidget(viewport.release());

  QSettings settings;
  if (settings.allKeys().size() == 0) {
    restore_default_layout();
  } else {
    restore_state();
  }

  setMenuBar(std::make_unique<QMenuBar>().release());
  for (auto&& menu : m_app.key_bindings.make_menus(m_app, main_menu_entries())) {
    menuBar()->addMenu(menu.release());
  }
  menuBar()->addMenu(make_about_menu().release());
  update_recent_files_menu();

  connect(&app.message_box(), SIGNAL(filename_changed()), this, SLOT(update_window_title()));
  connect(&app.message_box(), &MessageBox::filename_changed, [&app, this] {
    QSettings settings;
    auto fns = settings.value(RECENT_DOCUMENTS_SETTINGS_KEY, QStringList()).toStringList();
    const auto fn = QDir::cleanPath(QDir::current().absoluteFilePath(app.scene.filename()));
    if (!fn.isEmpty()) {
      fns.removeAll(fn);
      fns.append(QFileInfo(fn).absoluteFilePath());
    }
    settings.setValue(RECENT_DOCUMENTS_SETTINGS_KEY, fns);
    update_recent_files_menu();
  });
}

std::unique_ptr<QMenu> MainWindow::make_about_menu()
{
  auto menu = std::make_unique<QMenu>(tr("About"));

  connect(menu->addAction(tr("About")), &QAction::triggered, [this]() {
    QDialog about_dialog(this);
    ::Ui::AboutDialog ui;
    ui.setupUi(&about_dialog);
    ui.te_gpl30->hide();
    ui.lb_splash->setPixmap(QPixmap::fromImage(QImage(":/icons/omm.png")));
    about_dialog.exec();
  });

  return menu;
}

void MainWindow::restore_state()
{
  QSettings settings;
  restoreGeometry(settings.value(GEOMETRY_SETTINGS_KEY).toByteArray());
  load_layout(settings);
}

std::vector<QDockWidget*> MainWindow::dock_widgets() const
{
  return ::transform<QDockWidget*, std::vector>(findChildren<QDockWidget*>(), ::identity);
}

void MainWindow::restore_default_layout()
{
  QSettings settings(":/layouts/default.layout", QSettings::IniFormat);
  load_layout(settings);
}

void MainWindow::save_state()
{
  QSettings settings;
  settings.setValue(GEOMETRY_SETTINGS_KEY, saveGeometry());
  save_layout(settings);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (Application::instance().can_close()) {
    save_state();
    event->accept();
  } else {
    event->ignore();
  }
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
  if (::contains(Application::keyboard_modifiers, e->key())) {
    QMainWindow::keyPressEvent(e);
  } else if (!Application::instance().dispatch_key(e->key(), e->modifiers())) {
    QMainWindow::keyPressEvent(e);
  }
}

Viewport& MainWindow::viewport() const { return *m_viewport; }

void MainWindow::assign_unique_objectname(Manager &manager) const
{
  const auto blacklist = ::transform<QString, std::set>(dock_widgets(), get_object_name);
  manager.setObjectName(make_unique(manager.type(), blacklist));
}

void MainWindow::assign_unique_objectname(ToolBar& toolbar) const
{
  const auto blacklist = ::transform<QString, std::set>(findChildren<QToolBar*>(), get_object_name);
  toolbar.setObjectName(make_unique(toolbar.type(), blacklist));
}

QString MainWindow::get_last_layout_filename() const
{
  QSettings settings;
  const QString fn = settings.value(LAST_LAYOUT_FILE_NAME, "").toString();
  if (fn.isEmpty()) {
    return QDir::homePath();
  } else {
    return fn;
  }
}

void MainWindow::load_layout()
{
  const QString fn = QFileDialog::getOpenFileName(this, tr("Restore Layout"),
                                                  get_last_layout_filename());
  if (!fn.isEmpty()) {
    QSettings().setValue(LAST_LAYOUT_FILE_NAME, fn);
    QSettings settings(fn, QSettings::IniFormat);
    load_layout(settings);
  }
}

void MainWindow::save_layout()
{
  const QString fn = QFileDialog::getSaveFileName(this, tr("Save Layout"),
                                                  get_last_layout_filename());

  if (!fn.isEmpty()) {
    QSettings settings(fn, QSettings::IniFormat);
    if (settings.isWritable()) {
      QSettings().setValue(LAST_LAYOUT_FILE_NAME, fn);
      save_layout(settings);
    }
  }
}

void MainWindow:: load_layout(QSettings& settings)
{
  for (Manager* manager : findChildren<Manager*>()) {
    delete manager;
  }
  for (QToolBar* toolbar : findChildren<QToolBar*>()) {
    delete toolbar;
  }

  {
    const auto size = settings.beginReadArray(QString::fromStdString(TOOLBAR_SETTINGS_KEY));
    for (std::remove_const_t<decltype(size)> i = 0; i < size; ++i) {
      settings.setArrayIndex(i);
      const QString type = settings.value(TOOLBAR_TYPE_SETTINGS_KEY).toString();
      const QString name = settings.value(TOOLBAR_NAME_SETTINGS_KEY).toString();
      const QString tools = settings.value(TOOLBAR_TOOLS_SETTINGS_KEY).toString();

      if (type != ToolBar::TYPE) {
        LWARNING << "Unexpected type of toolbar: '" << type << "'.";
      }
      auto toolbar = std::make_unique<ToolBar>(tools);
      toolbar->setObjectName(name);
      assert(toolbar);
      addToolBar(toolbar.release());
    }
    settings.endArray();
  }

  restoreState(settings.value(WINDOWSTATE_SETTINGS_KEY).toByteArray());

  {
    const auto size = settings.beginReadArray(QString::fromStdString(MANAGER_SETTINGS_KEY));
    for (std::remove_const_t<decltype(size)> i = 0; i < size; ++i) {
      settings.setArrayIndex(i);
      const QString type = settings.value(MANAGER_TYPE_SETTINGS_KEY).toString();
      const QString name = settings.value(MANAGER_NAME_SETTINGS_KEY).toString();

      auto manager = Manager::make(type, m_app.scene);
      manager->setObjectName(name);
      assert(manager);
      if (!restoreDockWidget(manager.release())) {
        LWARNING << "Failed to restore geometry of manager.";
      }
    }
    settings.endArray();
  }
}

void MainWindow::save_layout(QSettings& settings)
{
  settings.setValue(WINDOWSTATE_SETTINGS_KEY, saveState());

  {
    std::set<QString> names;
    settings.beginWriteArray(MANAGER_SETTINGS_KEY);
    for (Manager* manager : findChildren<Manager*>()) {
      if (const QString name = manager->objectName(); !::contains(names, name)) {
        settings.setArrayIndex(names.size());
        settings.setValue(MANAGER_TYPE_SETTINGS_KEY, manager->type());
        settings.setValue(MANAGER_NAME_SETTINGS_KEY, name);
        names.insert(name);
      }
    }
    settings.endArray();
  }

  {
    std::set<QString> names;
    settings.beginWriteArray(TOOLBAR_SETTINGS_KEY);
    for (ToolBar* toolbar : findChildren<ToolBar*>()) {
      if (const QString name = toolbar->objectName(); !::contains(names, name)) {
        settings.setArrayIndex(names.size());
        settings.setValue(TOOLBAR_TYPE_SETTINGS_KEY, toolbar->type());
        settings.setValue(TOOLBAR_NAME_SETTINGS_KEY, name);
        settings.setValue(TOOLBAR_TOOLS_SETTINGS_KEY, toolbar->tools());
        names.insert(name);
      }
    }
    settings.endArray();
  }
}

void MainWindow::update_recent_files_menu()
{
  auto* recent_document_menu = find_menu(menuBar(), "load recent document");
  recent_document_menu->clear();
  Scene& scene = Application::instance().scene;
  recent_document_menu->setToolTipsVisible(true);
  assert(recent_document_menu != nullptr);
  std::size_t count = 0;
  std::set<QString> filenames;
  for (auto&& fn : QSettings().value(RECENT_DOCUMENTS_SETTINGS_KEY, QStringList()).toStringList()) {
    const QFileInfo file_info(fn);
    const QString abs_file_path = file_info.absoluteFilePath();
    if (file_info.exists()
        && QFileInfo(scene.filename()).absoluteFilePath() != abs_file_path
        && !::contains(filenames, abs_file_path))
    {
      filenames.insert(abs_file_path);
      auto action = std::make_unique<QAction>(file_info.fileName());
      action->setToolTip(fn);
      connect(action.get(), &QAction::triggered, [fn]() {
        Application::instance().load(fn, false);
      });
      recent_document_menu->addAction(action.release());
      count++;
    }
    if (count > 10) {
      break;
    }
  }
}

}  // namespace omm

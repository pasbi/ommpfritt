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
#include "managers/objectmanager/objectmanager.h"
#include "managers/stylemanager/stylemanager.h"
#include "managers/propertymanager/propertymanager.h"
#include "managers/boundingboxmanager/boundingboxmanager.h"
#include "managers/timeline/timeline.h"
#include "mainwindow/viewport/viewport.h"
#include "mainwindow/application.h"
#include "mainwindow/toolbar.h"
#include "managers/manager.h"
#include "tags/tag.h"
#include "logging.h"
#include "mainwindow/resourcemenu.h"
#include "ui_aboutdialog.h"
#include "scene/history/historymodel.h"

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

}  // namespace

namespace omm
{

std::vector<std::string> MainWindow::object_menu_entries()
{
  std::list<std::string> entries {
    "object/" QT_TRANSLATE_NOOP("any-context", "remove selection"),
    "object/" QT_TRANSLATE_NOOP("any-context", "convert objects"),
  };

  for (const std::string& key : Object::keys()) {
    entries.push_back("object/create/" + key);
  }

  for (const std::string& key : Tag::keys()) {
    entries.push_back("object/attach/" + key);
  }

  return std::vector(entries.begin(), entries.end());
}

std::vector<std::string> MainWindow::path_menu_entries()
{
  return std::vector<std::string> {
    "path/" QT_TRANSLATE_NOOP("any-context", "make smooth"),
    "path/" QT_TRANSLATE_NOOP("any-context", "make linear"),
    "path/" QT_TRANSLATE_NOOP("any-context", "subdivide"),
    "path/" QT_TRANSLATE_NOOP("any-context", "select all"),
    "path/" QT_TRANSLATE_NOOP("any-context", "deselect all"),
    "path/" QT_TRANSLATE_NOOP("any-context", "invert selection"),
    "path/" QT_TRANSLATE_NOOP("any-context", "show point dialog"),
  };
}

std::vector<std::string> MainWindow::main_menu_entries()
{
  using namespace std::string_literals;
  std::list<std::string> entries = {
    QT_TRANSLATE_NOOP("menu_name", "file")"/new document",
    "file/save document",
    "file/save document as",
    "file/load document",
    "file/" QT_TRANSLATE_NOOP("menu_name", "load recent document") "/",
    "file/"s + KeyBindings::SEPARATOR,
    "file/export",
    QT_TRANSLATE_NOOP("menu_name", "edit")"/undo",
    "edit/redo",
    QT_TRANSLATE_NOOP("menu_name", "object")"/new style",
    "object/" QT_TRANSLATE_NOOP("menu_name", "create")"/",
    "object/" QT_TRANSLATE_NOOP("menu_name", "attach")"/",
    QT_TRANSLATE_NOOP("menu_name", "path")"/",
    QT_TRANSLATE_NOOP("menu_name", "tool")"/previous tool",
    "tool/"s + KeyBindings::SEPARATOR,
    QT_TRANSLATE_NOOP("menu_name", "scene")"/evaluate",
    "scene/" QT_TRANSLATE_NOOP("any-context", "reset viewport"),
    QT_TRANSLATE_NOOP("menu_name", "window")"/" QT_TRANSLATE_NOOP("menu_name", "show")"/",
  };

  const auto merge = [&es=entries](auto&& ls) { es.insert(es.end(), ls.begin(), ls.end()); };

  for (const std::string& key : Manager::keys()) {
    entries.push_back("window/show/" + key);
  }
  entries.insert(entries.end(), {
    "window/"s + KeyBindings::SEPARATOR,
    "window/show keybindings dialog",
    "window/restore default layout",
#ifndef NDEBUG  // these functions facialiate the creation of profiles.
                // they should not be used by the user since the file content is not validated.
    "window/save layout ...",
    "window/load layout ...",
#endif  // NDEBUG
  });
  merge(object_menu_entries());
  merge(path_menu_entries());
  for (const std::string& key : Tool::keys()) {
    entries.push_back("tool/" + key);
  }

  return std::vector(entries.begin(), entries.end());
}

void MainWindow::update_window_title()
{
  QString filename = QString::fromStdString(m_app.scene.filename());
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

  auto* recent_document_menu = find_menu(menuBar(), "load recent document");
  assert(recent_document_menu != nullptr);
  for (auto&& fn : settings.value(RECENT_DOCUMENTS_SETTINGS_KEY, QStringList()).toStringList()) {
    auto action = std::make_unique<QAction>(QFileInfo(fn).fileName());
    action->setToolTip(fn);
    connect(action.get(), &QAction::triggered, [fn, &app]() {
      if (app.can_close()) {
        app.scene.set_selection({});
        QTimer::singleShot(0, [&app, fn]() {
          app.scene.load_from(fn.toStdString());
        });
      }
    });
    recent_document_menu->addAction(action.release());
  }

  connect(&app.message_box(), SIGNAL(filename_changed()), this, SLOT(update_window_title()));
  connect(&app.message_box(), &MessageBox::filename_changed, [&app] {
    QSettings settings;
    auto fns = settings.value(RECENT_DOCUMENTS_SETTINGS_KEY, QStringList()).toStringList();
    const auto fn = QString::fromStdString(app.scene.filename());
    fns.removeAll(fn);
    fns.append(fn);
    settings.setValue(RECENT_DOCUMENTS_SETTINGS_KEY, fns);
  });
}

std::unique_ptr<QMenu> MainWindow::make_about_menu()
{
  auto menu = std::make_unique<QMenu>(tr("About"));

  connect(menu->addAction(tr("About")), &QAction::triggered, [this]() {
    QDialog about_dialog(this);
    ::Ui::AboutDialog ui;
    ui.setupUi(&about_dialog);
    ui.te_breeze_license->hide();
    ui.te_gpl30->hide();
    ui.lb_splash->setPixmap(QPixmap::fromImage(QImage(":/icons/omm.png")));
    about_dialog.exec();
  });

  menu->addMenu(std::make_unique<LanguageMenu>().release());
  menu->addMenu(std::make_unique<SkinMenu>().release());

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
  if (Application::instance().dispatch_key(e->key())) {
    QMainWindow::keyPressEvent(e);
  }
}

Viewport& MainWindow::viewport() const { return *m_viewport; }

void MainWindow::make_unique_manager_name(QDockWidget& widget) const
{
  const auto names = ::transform<QString, std::set>(dock_widgets(), [](const auto* w) {
    return w->objectName();
  });
  std::size_t i = 0;
  const QString base = widget.objectName();
  QString name = base;
  while (::contains(names, name)) {
    name += QString("_%1").arg(i);
    i++;
  }
  widget.setObjectName(name);
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

  restoreState(settings.value(WINDOWSTATE_SETTINGS_KEY).toByteArray());
  auto size = settings.beginReadArray(QString::fromStdString(MANAGER_SETTINGS_KEY));
  for (decltype(size) i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    const QString type = settings.value(MANAGER_TYPE_SETTINGS_KEY).toString();
    const QString name = settings.value(MANAGER_NAME_SETTINGS_KEY).toString();

    auto manager = Manager::make(type.toStdString(), m_app.scene);
    manager->setObjectName(name);
    assert(manager);
    if (!restoreDockWidget(manager.release())) {
      LWARNING << "Failed to restore geometry of manager.";
    }

  }
  settings.endArray();
}

void MainWindow::save_layout(QSettings& settings)
{
  settings.setValue(WINDOWSTATE_SETTINGS_KEY, saveState());
  int i = 0;
  settings.beginWriteArray(MANAGER_SETTINGS_KEY);
  for (Manager* manager : findChildren<Manager*>()) {
    settings.setArrayIndex(i);
    settings.setValue(MANAGER_TYPE_SETTINGS_KEY, QString::fromStdString(manager->type()));
    settings.setValue(MANAGER_NAME_SETTINGS_KEY, manager->objectName());
    i += 1;
  }
  settings.endArray();
}

}  // namespace omm

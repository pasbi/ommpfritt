#include "mainwindow/mainwindow.h"

#include <memory>

#include <QCloseEvent>
#include <QDirIterator>
#include <QDockWidget>
#include <QFileDialog>
#include <QLocale>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <functional>

#include "common.h"
#include "config.h"
#include "keybindings/commandinterface.h"
#include "keybindings/keybindings.h"
#include "logging.h"
#include "main/application.h"
#include "mainwindow/gpl3.h"
#include "mainwindow/iconprovider.h"
#include "mainwindow/pathactions.h"
#include "mainwindow/toolbar/toolbar.h"
#include "mainwindow/viewport/viewport.h"
#include "managers/manager.h"
#include "managers/timeline/timeline.h"
#include "scene/history/historymodel.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tags/tag.h"
#include "tools/tool.h"
#include "ui_aboutdialog.h"
#include "widgets/colorwidget/colorwidget.h"

namespace
{

void append_qsettings_stringlist(const QString& key, const QString& value)
{
  QSettings settings;
  auto fns = settings.value(key, QStringList()).toStringList();
  fns.removeAll(value);
  fns.append(value);
  settings.setValue(key, fns);
}

QMenu* find_menu(QMenu* menu, const QString& object_name)
{
  if (menu == nullptr) {
    return nullptr;
  } else if (menu->objectName() == object_name) {
    return menu;
  } else {
    const auto actions = menu->actions();
    for (const QAction* action : actions) {
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
  while (blacklist.contains(name)) {
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
  std::list<QString> entries{
      "object/remove selected items",
      "object/convert objects",
  };

  for (const QString& key : Object::keys()) {
    entries.push_back("object/create/" + key);
  }

  for (const QString& key : Tag::keys()) {
    entries.push_back("object/attach/" + key);
  }

  return {entries.begin(), entries.end()};
}

std::vector<QString> MainWindow::path_menu_entries()
{
  return std::vector<QString>{
      "path/make smooth",
      "path/make linear",
      "path/subdivide",
      "path/select all",
      "path/deselect all",
      "path/join points",
      "path/disjoin points",
      "path/invert selection",
      "path/select connected points",
      "path/fill selection",
      "path/extend selection",
      "path/shrink selection",
      "path/show point dialog",
  };
}

std::vector<QString> MainWindow::main_menu_entries()
{
  using namespace std::string_literals;
  std::list<QString> entries = {
      // NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
      QT_TRANSLATE_NOOP("menu_name", "file") "/new",
      "file/open ...",
      "file/" QT_TRANSLATE_NOOP("menu_name", "open recent") "/",
      QString::fromStdString("file/"s + KeyBindings::SEPARATOR),
      "file/save",
      "file/save as ...",
      QString::fromStdString("file/"s + KeyBindings::SEPARATOR),
      "file/export ...",
      QT_TRANSLATE_NOOP("menu_name", "edit") "/undo",
      "edit/redo",
      "edit/preferences",
      QT_TRANSLATE_NOOP("menu_name", "object") "/new style",
      "object/" QT_TRANSLATE_NOOP("menu_name", "create") "/",
      "object/" QT_TRANSLATE_NOOP("menu_name", "attach") "/",
      QT_TRANSLATE_NOOP("menu_name", "path") "/",
      QT_TRANSLATE_NOOP("menu_name", "tool") "/previous tool",
      QString::fromStdString("tool/"s + KeyBindings::SEPARATOR),
      QT_TRANSLATE_NOOP("menu_name", "scene") "/evaluate",
      "scene/reset viewport",
      QT_TRANSLATE_NOOP("menu_name", "window") "/" QT_TRANSLATE_NOOP("menu_name", "show") "/",
      QT_TRANSLATE_NOOP("menu_name", "actions") "/",
  };

  const auto merge = [&es = entries](auto&& ls) { es.insert(es.end(), ls.begin(), ls.end()); };

  for (const QString& key : Manager::keys()) {
    entries.push_back("window/show/" + key);
  }
  entries.insert(entries.end(),
                 {
                     QString::fromStdString("window/"s + KeyBindings::SEPARATOR),
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
  for (const QString& key : path_actions::available_actions()) {
    entries.push_back("actions/" + key);
  }

  return {entries.begin(), entries.end()};
}

void MainWindow::update_window_title()
{
  QString filename = m_app.scene->filename();
  static const QString clean_indicator = tr("");
  static const QString dirty_indicator = tr("*");
  QString indicator = m_app.scene->has_pending_changes() ? dirty_indicator : clean_indicator;
  if (filename.isEmpty()) {
    filename = tr("unnamed");
    indicator = clean_indicator;  // never show dirty indicator if no filename is set.
  } else {
    filename = QFileInfo(filename).fileName();
  }
  setWindowTitle(QString("%1%2 - omm").arg(filename, indicator));
}

MainWindow::MainWindow(Application& app) : m_app(app)
{
  setDockNestingEnabled(true);
  auto viewport = std::make_unique<Viewport>(*app.scene);
  m_viewport = viewport.get();
  setCentralWidget(viewport.release());

  QSettings settings;
  if (settings.allKeys().empty()) {
    restore_default_layout();
  } else {
    restore_state();
  }

  setMenuBar(std::make_unique<QMenuBar>().release());
  for (auto&& menu : m_app.key_bindings->make_menus(m_app, main_menu_entries())) {
    menuBar()->addMenu(menu.release());
  }
  menuBar()->addMenu(make_about_menu().release());
  update_recent_scenes_menu();

  connect(&app.mail_box(), &MailBox::filename_changed, this, &MainWindow::update_window_title);
  connect(&app.mail_box(), &MailBox::filename_changed, this, [&app, this] {
    if (QString fn = app.scene->filename(); !fn.isEmpty()) {
      fn = QDir::cleanPath(QDir::current().absoluteFilePath(fn));
      append_qsettings_stringlist(RECENT_SCENES_SETTINGS_KEY, fn);
      update_recent_scenes_menu();
    }
  });

  installEventFilter(this);
}

std::unique_ptr<QMenu> MainWindow::make_about_menu()
{
  auto menu = std::make_unique<QMenu>(tr("About"));

  connect(menu->addAction(tr("About")), &QAction::triggered, this, [this]() {
    QDialog about_dialog(this);
    ::Ui::AboutDialog ui;
    ui.setupUi(&about_dialog);
    ui.te_gpl30->hide();
    ui.te_gpl30->setText(QString::fromStdString(std::string(GPL3_TEXT)));
    ui.lb_splash->setPixmap(IconProvider::pixmap("omm", IconProvider::Size::Gigantic));
    ui.lb_version->setText(git_describe().data());
    about_dialog.adjustSize();
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
  return util::transform<std::vector>(findChildren<QDockWidget*>());
}

void MainWindow::restore_default_layout()
{
  QSettings settings(":/layouts/default_layout.ini", QSettings::IniFormat);
  load_layout(settings);
}

void MainWindow::save_state()
{
  QSettings settings;
  settings.setValue(GEOMETRY_SETTINGS_KEY, saveGeometry());
  save_layout(settings);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  if (Application::instance().can_close()) {
    save_state();
    event->accept();
  } else {
    event->ignore();
  }
}

bool omm::MainWindow::eventFilter(QObject* o, QEvent* e)
{
  if (o == this && e->type() == QEvent::ShortcutOverride) {
    e->accept();
  }
  return QMainWindow::eventFilter(o, e);
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
  const bool is_modifier = Application::keyboard_modifiers.contains(e->key());
  const bool is_dispatched = !is_modifier && !Application::instance().dispatch_key(e->key(), e->modifiers());
  if (is_modifier || is_dispatched) {
    QMainWindow::keyPressEvent(e);
  }
}

Viewport& MainWindow::viewport() const
{
  return *m_viewport;
}

void MainWindow::assign_unique_objectname(Manager& manager) const
{
  const auto blacklist = util::transform<std::set>(dock_widgets(), get_object_name);
  manager.setObjectName(make_unique(manager.type(), blacklist));
}

void MainWindow::assign_unique_objectname(ToolBar& toolbar) const
{
  const auto blacklist = util::transform<std::set>(findChildren<QToolBar*>(), get_object_name);
  toolbar.setObjectName(make_unique(toolbar.type(), blacklist));
}

QString MainWindow::get_last_layout_filename()
{
  QSettings settings;
  QString fn = settings.value(LAST_LAYOUT_FILE_NAME, "").toString();
  if (fn.isEmpty()) {
    return QDir::homePath();
  } else {
    return fn;
  }
}

void MainWindow::load_layout()
{
  const QString fn
      = QFileDialog::getOpenFileName(this, tr("Restore Layout"), get_last_layout_filename());
  if (!fn.isEmpty()) {
    QSettings().setValue(LAST_LAYOUT_FILE_NAME, fn);
    QSettings settings(fn, QSettings::IniFormat);
    load_layout(settings);
  }
}

void MainWindow::save_layout()
{
  const QString fn
      = QFileDialog::getSaveFileName(this, tr("Save Layout"), get_last_layout_filename());

  if (!fn.isEmpty()) {
    QSettings settings(fn, QSettings::IniFormat);
    if (settings.isWritable()) {
      QSettings().setValue(LAST_LAYOUT_FILE_NAME, fn);
      save_layout(settings);
    }
  }
}

void MainWindow::restore_toolbars(QSettings& settings)
{
  try {
    const auto size = settings.beginReadArray(QString::fromStdString(TOOLBAR_SETTINGS_KEY));
    for (std::remove_const_t<decltype(size)> i = 0; i < size; ++i) {
      settings.setArrayIndex(i);
      const QString type = settings.value(TOOLBAR_TYPE_SETTINGS_KEY).toString();
      const QString name = settings.value(TOOLBAR_NAME_SETTINGS_KEY).toString();
      const QString configuration = settings.value(TOOLBAR_TOOLS_SETTINGS_KEY).toString();

      if (type != ToolBar::TYPE) {
        LWARNING << "Unexpected type of toolbar: '" << type << "'.";
      }
      auto toolbar = std::make_unique<ToolBar>(configuration);
      toolbar->setObjectName(name);
      assert(toolbar);
      addToolBar(toolbar.release());
    }
    settings.endArray();
  } catch (const ToolBarItemModel::BadConfigurationError& e) {
    handle_corrupted_config_file(settings, e.what());
  }
}

void MainWindow::restore_managers(QSettings& settings)
{
  try {
    const auto size = settings.beginReadArray(QString::fromStdString(MANAGER_SETTINGS_KEY));
    for (std::remove_const_t<decltype(size)> i = 0; i < size; ++i) {
      settings.setArrayIndex(i);
      const QString type = settings.value(MANAGER_TYPE_SETTINGS_KEY).toString();
      const QString name = settings.value(MANAGER_NAME_SETTINGS_KEY).toString();

      auto manager = Manager::make(type, *m_app.scene);
      assert(manager);
      manager->setObjectName(name);

      if (restoreDockWidget(manager.get())) {
        if (manager->isFloating()) {
          manager->setFloating(true);
        }
        (void)manager.release();  // ownership is handled by qt
      } else {
        LWARNING << "Failed to restore geometry of manager.";
        // delete the dock widget.
      }
    }
    settings.endArray();
  } catch (const Manager::InvalidKeyError& e) {
    handle_corrupted_config_file(settings, e.what());
  }
}

void MainWindow::load_layout(QSettings& settings)
{
  const auto managers = findChildren<Manager*>();
  for (const Manager* manager : managers) {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete manager;
  }

  const auto toolbars = findChildren<QToolBar*>();
  for (const QToolBar* toolbar : toolbars) {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete toolbar;
  }

  restore_toolbars(settings);
  restoreState(settings.value(WINDOWSTATE_SETTINGS_KEY).toByteArray());
  restore_managers(settings);
}

void MainWindow::save_managers(QSettings& settings)
{
  std::set<QString> names;
  settings.beginWriteArray(MANAGER_SETTINGS_KEY);
  const auto managers = findChildren<Manager*>();
  for (const Manager* manager : managers) {
    if (const QString name = manager->objectName(); !names.contains(name)) {
      settings.setArrayIndex(static_cast<int>(names.size()));
      settings.setValue(MANAGER_TYPE_SETTINGS_KEY, manager->type());
      settings.setValue(MANAGER_NAME_SETTINGS_KEY, name);
      names.insert(name);
    }
  }
  settings.endArray();
}

void MainWindow::save_toolbars(QSettings& settings)
{
  std::set<QString> names;
  settings.beginWriteArray(TOOLBAR_SETTINGS_KEY);
  const auto tools = findChildren<ToolBar*>();
  for (const ToolBar* toolbar : tools) {
    if (const QString name = toolbar->objectName(); !names.contains(name)) {
      settings.setArrayIndex(static_cast<int>(names.size()));
      settings.setValue(TOOLBAR_TYPE_SETTINGS_KEY, toolbar->type());
      settings.setValue(TOOLBAR_NAME_SETTINGS_KEY, name);
      const auto configuration = toolbar->configuration();
      settings.setValue(TOOLBAR_TOOLS_SETTINGS_KEY, configuration);
      names.insert(name);
    }
  }
  settings.endArray();
}

void MainWindow::save_layout(QSettings& settings)
{
  settings.setValue(WINDOWSTATE_SETTINGS_KEY, saveState());
  save_managers(settings);
  save_toolbars(settings);
}

void MainWindow::update_recent_scenes_menu()
{
  auto* recent_scene_menu = find_menu(menuBar(), "open recent");
  recent_scene_menu->clear();
  Scene& scene = *Application::instance().scene;
  recent_scene_menu->setToolTipsVisible(true);
  assert(recent_scene_menu != nullptr);
  std::size_t count = 0;
  std::set<QString> filenames;
  for (auto&& fn : QSettings().value(RECENT_SCENES_SETTINGS_KEY, QStringList()).toStringList()) {
    const QFileInfo file_info(fn);
    const QString abs_file_path = file_info.absoluteFilePath();
    if (file_info.exists() && file_info.isFile()
        && QFileInfo(scene.filename()).absoluteFilePath() != abs_file_path
        && !filenames.contains(abs_file_path)) {
      filenames.insert(abs_file_path);
      auto action = std::make_unique<QAction>(file_info.fileName());
      action->setToolTip(fn);
      connect(action.get(), &QAction::triggered, [fn]() {
        Application::instance().open(fn, false);
      });
      recent_scene_menu->addAction(action.release());
      count++;
    }
    static constexpr std::size_t RECENT_FILES_MENU_MAX_SIZE = 10;
    if (count > RECENT_FILES_MENU_MAX_SIZE) {
      break;
    }
  }
}

void MainWindow::handle_corrupted_config_file(const QSettings& s, const QString& what)
{
  const auto msg = tr("The configuration file %1 is corrupted.\n"
                      "Delete the file and restart the application.\n"
                      "%2")
                       .arg(s.fileName(), what);
  LERROR << msg;
  QMessageBox::critical(this, tr("Corrupted config file"), msg);
  LFATAL("Corrupted config file.");
}

}  // namespace omm

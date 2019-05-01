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

#include "managers/objectmanager/objectmanager.h"
#include "managers/stylemanager/stylemanager.h"
#include "managers/propertymanager/propertymanager.h"
#include "mainwindow/viewport/viewport.h"
#include "mainwindow/application.h"
#include "mainwindow/toolbar.h"
#include "managers/manager.h"
#include "tags/tag.h"
#include "logging.h"

namespace
{

template<typename T, typename F>
auto read_each(QSettings& settings, const std::string& key, const F& f)
{
  auto size = settings.beginReadArray(QString::fromStdString(key));
  std::vector<T> ts;
  assert(size >= 0);
  ts.reserve(static_cast<std::size_t>(size));
  for (decltype(size) i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    ts.push_back(f());
  }
  settings.endArray();
  return ts;
}

template<typename F>
void read_each(QSettings& settings, const std::string& key, const F& f)
{
  auto size = settings.beginReadArray(QString::fromStdString(key));
  for (decltype(size) i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    f();
  }
  settings.endArray();
}

template<typename Ts, typename F>
void write_each(QSettings& settings, const std::string& key, const Ts& ts, const F& f)
{
  settings.beginWriteArray(QString::fromStdString(key));
  int i = 0;
  for (auto&& t : ts) {
    settings.setArrayIndex(i);
    f(t);
    i += 1;
  }
  settings.endArray();
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
    "file/"s + KeyBindings::SEPARATOR,
    "file/export",
    QT_TRANSLATE_NOOP("menu_name", "edit")"/undo",
    "edit/redo",
    QT_TRANSLATE_NOOP("menu_name", "object")"/new style",
    QT_TRANSLATE_NOOP("menu_name", "object")"/" QT_TRANSLATE_NOOP("menu_name", "create")"/",
    QT_TRANSLATE_NOOP("menu_name", "object")"/" QT_TRANSLATE_NOOP("menu_name", "attach")"/",
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
  QString indicator = m_app.scene.history.has_pending_changes() ? dirty_indicator : clean_indicator;
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

  connect(&app.scene, SIGNAL(filename_changed()), this, SLOT(update_window_title()));
}

std::unique_ptr<QMenu> MainWindow::make_about_menu()
{
  auto menu = std::make_unique<QMenu>(tr("About"));
  connect(menu->addAction(tr("About")), &QAction::triggered, [this]() {
    QMessageBox::information(this, tr("About"), tr("omm."));
  });

  const auto show_restart_hint = [this]() {
    const auto msg = tr("Changing language takes effect after restarting the application.");
    QMessageBox::information(this, tr("information"), msg);
  };

  auto language_menu = std::make_unique<QMenu>(tr("language"));
  QSettings settings;

  auto language_action_group = std::make_unique<QActionGroup>(language_menu.get()).release();
  language_action_group->setExclusive(true);
  bool any_language_selected = false;
  const auto current_locale = settings.value(LOCALE_SETTINGS_KEY);
  for (const std::string& code : available_translations()) {
    if (!code.empty()) {
      const auto locale = QLocale(QString::fromStdString(code));
      const auto language = QLocale::languageToString(locale.language()).toStdString();
      auto* action = language_menu->addAction(tr(language.c_str()));
      language_action_group->addAction(action);
      action->setCheckable(true);
      const bool current_locale_selected = locale == current_locale;
      action->setChecked(current_locale_selected);
      connect(action, &QAction::triggered, [show_restart_hint, locale]() {
        QSettings().setValue(LOCALE_SETTINGS_KEY, locale);
        show_restart_hint();
      });
      assert(!current_locale_selected || !any_language_selected);
      any_language_selected |= current_locale_selected;
    }
  }
  language_menu->addSeparator();
  auto* default_language_action = language_menu->addAction(tr("system default"));
  default_language_action->setCheckable(true);
  default_language_action->setChecked(!any_language_selected);
  connect(default_language_action, &QAction::triggered, [show_restart_hint]() {
    QSettings().remove(LOCALE_SETTINGS_KEY);
    show_restart_hint();
  });
  language_action_group->addAction(default_language_action);
  menu->addMenu(language_menu.release());

  return menu;
}

void MainWindow::restore_state()
{
  QSettings settings;
  restoreGeometry(settings.value(GEOMETRY_SETTINGS_KEY).toByteArray());

  read_each(settings, TOOLBAR_SETTINGS_KEY, [this, &settings]() {
    const auto tools = read_each<std::string>(settings, TOOLBAR_TOOLS_SETTINGS_KEY, [&settings]() {
      return settings.value(TOOLBAR_TOOL_SETTINGS_KEY).toString().toStdString();
    });
    auto tool_bar = std::make_unique<ToolBar>(this, m_app.scene.tool_box, tools);
    addToolBar(Qt::TopToolBarArea, tool_bar.release());
  });

  restoreState(settings.value(WINDOWSTATE_SETTINGS_KEY).toByteArray());

  read_each(settings, MANAGER_SETTINGS_KEY, [this, &settings]() {
    const auto type = settings.value(MANAGER_TYPE_SETTINGS_KEY).toString().toStdString();
    const auto name = settings.value(MANAGER_NAME_SETTINGS_KEY).toString();
    auto manager = Manager::make(type, m_app.scene);
    manager->setObjectName(name);
    assert(manager);
    if (!restoreDockWidget(manager.release())) {
      LWARNING << "Failed to restore geometry of manager.";
    }
  });
}

std::vector<QDockWidget*> MainWindow::dock_widgets() const
{
  return ::transform<QDockWidget*, std::vector>(findChildren<QDockWidget*>(), ::identity);
}

void MainWindow::restore_default_layout()
{
  // TODO restore toolbars
  for (auto* dock : dock_widgets()) {
    removeDockWidget(dock);
    dock->close();
  }

  const auto add_dock = [this](const std::string& type) {
    auto manager = Manager::make(type, m_app.scene);
    make_unique_manager_name(*manager);
    manager->setParent(this);
    this->addDockWidget(Qt::RightDockWidgetArea, manager.release(), Qt::Vertical);
  };

  add_dock(ObjectManager::TYPE);
  add_dock(PropertyManager::TYPE);
  add_dock(StyleManager::TYPE);

  QTimer::singleShot(100, [this]() { save_state(); });
}

void MainWindow::save_state()
{
  QSettings settings;
  settings.setValue(GEOMETRY_SETTINGS_KEY, saveGeometry());
  settings.setValue(WINDOWSTATE_SETTINGS_KEY, saveState());

  const auto save_manager = [&settings](const Manager* manager) {
    LINFO << "save manager " << manager->type() << " "
          << manager->isVisible() << " " << manager->objectName().toStdString();
    settings.setValue(MANAGER_TYPE_SETTINGS_KEY, QString::fromStdString(manager->type()));
    settings.setValue(MANAGER_NAME_SETTINGS_KEY, manager->objectName());
  };
  write_each(settings, MANAGER_SETTINGS_KEY, findChildren<Manager*>(), save_manager);

  const auto save_tool_bar = [&settings](const ToolBar* tool_bar) {
    auto& tools = tool_bar->tools();
    write_each(settings, TOOLBAR_TOOLS_SETTINGS_KEY, tools, [&settings](const auto& t) {
      settings.setValue(TOOLBAR_TOOL_SETTINGS_KEY, QString::fromStdString(t));
    });
  };
  write_each(settings, TOOLBAR_SETTINGS_KEY, findChildren<ToolBar*>(), save_tool_bar);
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
  if (!Application::instance().key_bindings.call(*e, m_app)) {
    QMainWindow::keyPressEvent(e);
  }
}

std::vector<std::string> MainWindow::available_translations()
{
  using namespace std::string_literals;
  std::list<std::string> trs;
  QDirIterator it(LANGUAGE_RESOURCE_DIRECTORY, QDirIterator::Subdirectories);
  static const QString prefix = QString::fromStdString( LANGUAGE_RESOURCE_DIRECTORY + "/"s
                                                      + LANGUAGE_RESOURCE_PREFIX + "_"s);
  static const QString suffix(LANGUAGE_RESOURCE_SUFFIX);
  while (it.hasNext()) {
    const auto filename = it.next();
    if (filename.startsWith(prefix) && filename.endsWith(suffix)) {
      const int code_length = filename.size() - prefix.size() - suffix.size();
      if (code_length < 0) {
        trs.push_back("");
      } else {
        const auto code = filename.mid(prefix.size(), code_length);
        trs.push_back(code.toStdString());
      }
    }
  }

  return std::vector(trs.begin(), trs.end());
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

}  // namespace omm

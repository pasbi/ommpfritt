#include "mainwindow/mainwindow.h"

#include <memory>
#include <glog/logging.h>

#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QSettings>
#include <QCloseEvent>

#include "mainwindow/viewport/viewport.h"
#include "mainwindow/mainmenubar.h"
#include "mainwindow/application.h"
#include "mainwindow/toolbar.h"

#include "managers/propertymanager/propertymanager.h"
#include "managers/objectmanager/objectmanager.h"
#include "scene/scene.h"

namespace
{

template<typename T, typename F>
auto read_each(QSettings& settings, const std::string& key, const F& f)
{
  auto size = settings.beginReadArray(QString::fromStdString(key));
  std::vector<T> ts;
  ts.reserve(size);
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
  size_t i = 0;
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

MainWindow::MainWindow(Application& app)
  : m_app(app)
  , key_bindings(*this)
{
  setMenuBar(std::make_unique<MainMenuBar>(app, *this).release());
  setDockNestingEnabled(true);
  setCentralWidget(std::make_unique<Viewport>(app.scene).release());
  restore_state();
}

MainWindow::~MainWindow()
{
}

void MainWindow::restore_state()
{
  LOG(INFO) << "restore-state";
  QSettings settings;
  restoreGeometry(settings.value("mainwindow/geometry").toByteArray());

  read_each(settings, "mainwindow/toolbars", [this, &settings]() {
    const auto tools = read_each<std::string>(settings, "tools", [&settings]() {
      return settings.value("tool").toString().toStdString();
    });
    auto tool_bar = std::make_unique<ToolBar>(this, m_app.scene.tool_box, tools);
    addToolBar(Qt::TopToolBarArea, tool_bar.release());
  });

  restoreState(settings.value("mainwindow/window_state").toByteArray());

  read_each(settings, "mainwindow/managers", [this, &settings]() {
    const auto type = settings.value("type").toString().toStdString();
    auto manager = Manager::make(type, m_app.scene);
    if (!restoreDockWidget(manager.release())) {
      LOG(WARNING) << "Failed to restore geometry of manager.";
    }
  });
}

void MainWindow::save_state()
{
  QSettings settings;
  settings.setValue("mainwindow/geometry", saveGeometry());
  settings.setValue("mainwindow/window_state", saveState());

  const auto save_manager = [&settings](const Manager* manager) {
    settings.setValue("type", QString::fromStdString(manager->type()));
  };
  write_each(settings, "mainwindow/managers", findChildren<Manager*>(), save_manager);

  const auto save_tool_bar = [&settings](const ToolBar* tool_bar) {
    write_each(settings, "tools", tool_bar->tools(), [&settings](const std::string& tool) {
      settings.setValue("tool", QString::fromStdString(tool));
    });
  };
  write_each(settings, "mainwindow/toolbars", findChildren<ToolBar*>(), save_tool_bar);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  save_state();
  QMainWindow::closeEvent(event);
}

void MainWindow::call(const std::string& command)
{
  LOG(INFO) << "command: " << command;
}

std::string MainWindow::type() const { return "MainWindow"; }

}  // namespace omm
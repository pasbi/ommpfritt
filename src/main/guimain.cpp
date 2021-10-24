#include "main/commandlineparser.h"
#include "main/application.h"
#include <QApplication>
#include "mainwindow/mainwindow.h"
#include "mainwindow/iconprovider.h"
#include "tools/toolbox.h"
#include "tools/selectobjectstool.h"
#include "scene/scene.h"

namespace omm
{

int gui_main(const CommandLineParser& args, Application& app)
{
  QApplication::setWindowIcon(IconProvider::pixmap("omm"));

  auto window = std::make_unique<MainWindow>(app);
  app.set_main_window(*window);
  window->show();

  if (const auto fn = args.scene_filename(); !fn.isEmpty()) {
    app.scene->load_from(fn);
  }

  app.scene->tool_box().set_active_tool(SelectObjectsTool::TYPE);

  return QApplication::exec();
}

}  // namespace omm

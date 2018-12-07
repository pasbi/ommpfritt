#include "application.h"

#include <glog/logging.h>
#include <cassert>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>

#include "mainwindow/mainwindow.h"
#include "commands/addobjectcommand.h"
#include "managers/manager.h"
#include "managers/propertymanager/propertymanager.h"
#include "managers/objectmanager/objectmanager.h"

namespace {
constexpr auto FILE_ENDING = ".omm";
}

namespace omm
{

Application::Application(QApplication& app)
  : m_app(app)
{
}

void Application::set_main_window(MainWindow& main_window)
{
  m_main_window = &main_window;
}

void Application::new_scene()
{

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
  if (m_scene.has_pending_changes()) {
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
  if (!m_scene.save_as(filename)) {
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
  const std::string filename = m_scene.filename();
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
                                  QString::fromStdString(m_scene.filename()) );
  if (filename.isEmpty()) {
    return false;
  } else {
    return m_scene.save_as(filename.toStdString());
  }
}

void Application::reset()
{
  LOG(INFO) << "reset scene.";
  m_scene.reset();
}

bool Application::load()
{
  const QString filename =
    QFileDialog::getOpenFileName( m_main_window,
                                  tr("Load scene ..."),
                                  QString::fromStdString(m_scene.filename()) );

  if (filename.isEmpty()) {
    return false;
  } else if (m_scene.load_from(filename.toStdString())) {
    return true;
  } else {
    QMessageBox::critical( m_main_window,
                           tr("Error."),
                           tr("Loading scene from '%1' failed.").arg(filename),
                           QMessageBox::Ok );
    return false;
  }
}


Scene& Application::scene()
{
  return m_scene;
}

const Scene& Application::scene() const
{
  return m_scene;
}


}  // namespace omm

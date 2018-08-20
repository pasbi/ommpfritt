#include <glog/logging.h>
#include <assert.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>
#include "application.h"
#include "mainwindow.h"

namespace {
constexpr auto FILE_ENDING = ".omm";
}

Application::Application(int argc, char* argv[])
  : Gtk::Application(argc, argv, "org.ommpfritt")
{
}


void Application::on_startup()
{ 
  Gtk::Application::on_startup();
  create_main_menu();
}

void Application::create_main_menu()
{
  add_action("quit", [this]() {
    if (can_close()) {
      LOG(INFO) << "Quit application.";
      quit();
    } else {
      LOG(INFO) << "Aborted quit.";
    }
  });

  add_action("new", [this]() {
    if (can_close()) {
      LOG(INFO) << "Loading new project ...";
      reset();
    } else {
      LOG(INFO) << "User aborted loading new project because the old project has pending changes.";
    }
  });

  add_action("load", [this]() {
    if (can_close()) {
      load();
    } else {
      LOG(INFO) << "User aborted loading project because the old project has pending changes.";
    }
  });

  add_action("save", [this]() { save(); });
  add_action("save_as", [this]() { save_as(); });

  auto m_refBuilder = Gtk::Builder::create();

  Glib::ustring ui_info =
#include "mainmenu.xml"
  ;

  try {
    m_refBuilder->add_from_string(ui_info);
  } catch (const Glib::Error& ex) {
    LOG(FATAL) << "Building menus failed: " << ex.what();
  }

  //Get the menubar and the app menu, and add them to the application:
  auto object = m_refBuilder->get_object("menu-example");
  auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  assert(gmenu);
  set_menubar(gmenu);
}

void Application::on_activate()
{
}

bool Application::can_close()
{
  LOG(INFO) << "Trying to close ...";
  if (m_project.has_pending_changes()) {
    LOG(INFO) << "Paused close because there are pending changes.";
    Gtk::MessageDialog dialog( top_window(),
                               "The current project has pending changes. What shall I do?",
                               false,
                               Gtk::MESSAGE_QUESTION,
                               Gtk::BUTTONS_NONE,
                               true );

    dialog.add_button("Close _without saving", Gtk::RESPONSE_CLOSE);
    dialog.add_button("_Save and close", Gtk::RESPONSE_OK);
    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);

    const auto response = dialog.run();
    LOG(INFO) << "response: " << response;
    switch (response) {
    case Gtk::RESPONSE_CLOSE:
      LOG(INFO) << "User forced close. Ready to close.";
      return true;
    case Gtk::RESPONSE_OK:
      if (save()) {
        LOG(INFO) << "User saved pending changes. Ready to close.";
        return true;
      } else {
        LOG(INFO) << "User aborted save. Abort close.";
        return false;
      }
    case Gtk::RESPONSE_CANCEL:
      LOG(INFO) << "User aborted close.";
      return false;
    default:
      LOG(FATAL) << "Unexpected response code: " << response;
      return false;
    }
  } else {
    LOG(INFO) << "everything is saved. Ready to close.";
    return true;
  }
}

Gtk::Window& Application::top_window()
{
  return *get_windows()[0];
}

bool Application::save_(const std::string& filename)
{
  if (!m_project.save_as(filename)) {
    LOG(WARNING) << "Error saving project as '" << filename << "'.";
    Gtk::MessageDialog dialog( "Failed to save project.",
                               false,
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_OK );
    dialog.set_secondary_text("Failed to save project as '" + filename + "'.\n"
        "Maybe the path does not exist or you may lack write permissions.");
    dialog.set_default_response(Gtk::RESPONSE_OK);
    dialog.run();
    return false;
  } else {
    return true;
  }
}

bool Application::save()
{
  LOG(INFO) << "Trying to save ...";
  const std::string filename = m_project.filename();
  if (filename.empty()) {
    LOG(INFO) << "Project has no filename. Fallback to save_as.";
    return save_as();
  } else {
    LOG(INFO) << "Trying to save project as '" << filename << "'.";
    return save_(filename);
  }
}

namespace {
void add_ending(std::string& s, const std::string& ending)
{
  if (s.length() < ending.length()) {
    s += ending;
  } else if (s.compare(s.length() - ending.length(), ending.length(), ending) != 0) {
    s += ending;
  } else {
    // s already ends with `ending`.
  }
}

}  // namespace

bool Application::save_as()
{
  LOG(INFO) << "Trying to save as ...";
  Gtk::FileChooserDialog dialog("Save as", Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.add_button("_Select", Gtk::RESPONSE_OK);
  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dialog.set_transient_for(top_window());
  dialog.set_filename(m_project.filename());

  const auto response = dialog.run();
  LOG(INFO) << "save as response: " << response;
  switch (response) {
  case Gtk::RESPONSE_OK:
  {
    auto filename = dialog.get_filename();
    add_ending(filename, FILE_ENDING);
    LOG(INFO) << "Trying to save project at user-selected filename: '"
              << dialog.get_filename() << "'.";
    return save_(dialog.get_filename());
  }
  case Gtk::RESPONSE_CANCEL:
    LOG(INFO) << "User aborted file selection. No attempt to save.";
    return false;
  case Gtk::RESPONSE_DELETE_EVENT:
    LOG(INFO) << "File selection aborted otherwise. No attempt to save.";
    return false;
  default:
    LOG(FATAL) << "Unexpected response code: " << response;
    return false;
  }
}


void Application::reset()
{
  LOG(INFO) << "reset project.";
  m_project.reset();
}

bool Application::load()
{
  Gtk::FileChooserDialog dialog("Open", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.add_button("_Open", Gtk::RESPONSE_OK);
  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dialog.set_transient_for(top_window());
  dialog.set_filename(m_project.filename());

  auto ommpfritt_filter = Gtk::FileFilter::create();
  ommpfritt_filter->set_name("ommpfritt files");
  ommpfritt_filter->add_pattern(std::string("*") + FILE_ENDING);
  dialog.add_filter(ommpfritt_filter);

  const auto response = dialog.run();
  switch (response) {
  case Gtk::RESPONSE_OK:
    LOG(INFO) << "User selected file to load: '" << dialog.get_filename() << "'.";
    reset();
    LOG(INFO) << "Try to load project from file.";
    return m_project.load_from(dialog.get_filename());
  case Gtk::RESPONSE_CANCEL:
    LOG(INFO) << "User aborted file selection. No attempt to load.";
    return false;
  case Gtk::RESPONSE_DELETE_EVENT:
    LOG(INFO) << "File selection aborted otherwise. No attempt to load.";
    return false;
  default:
    LOG(FATAL) << "Unexpected response code: " << response;
    return false;
  }

}

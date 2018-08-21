#pragma once

#include <gtkmm/application.h>
#include <gtkmm/builder.h>

#include "project.h"

namespace omm {

class Application : public Gtk::Application
{
public:
  Application(int argc, char* argv[]);
  void on_startup() override;
  void on_activate() override;

  bool save();
  bool save_as();
  bool save_(const std::string& filename);
  bool can_close();
  bool load();
  void reset();
  void update_undo_redo_enabled();
  Gtk::Window& top_window();

private:
  Glib::RefPtr<Gio::Menu> create_main_menu();
  const Glib::RefPtr<Gio::Menu> m_menu;

  Project m_project;
};

}  // namespace omm
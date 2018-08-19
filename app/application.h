#pragma once

#include <gtkmm/application.h>
#include <gtkmm/builder.h>

#include "project.h"

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
  Gtk::Window& top_window();

private:
  void create_main_menu();

  Project m_project;
};
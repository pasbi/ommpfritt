#include "commands/namedcolorscommand.h"
#include <QObject>
#include "mainwindow/application.h"
#include "color/namedcolors.h"

namespace omm
{

NamedColorAddRemoveCommand
::NamedColorAddRemoveCommand(const std::string& label, const std::string& color_name, const Color& value)
  : Command(label), m_color_name(color_name), m_value(value)
{
  assert(value.model() != Color::Model::Named);
}

void NamedColorAddRemoveCommand::add()
{
  Application::instance().scene.named_colors().add(m_color_name, m_value);
}

void NamedColorAddRemoveCommand::remove()
{
  auto& model = Application::instance().scene.named_colors();
  LINFO << m_color_name;
  const Color color = model.color(m_color_name);
  assert(m_value == color);
  model.remove(m_color_name);
}

ChangeNamedColorNameCommand::
ChangeNamedColorNameCommand(const std::string& old_name, const std::string& new_name)
  : Command(QObject::tr("Change name of named color name").toStdString())
  , m_old_name(old_name)
  , m_new_name(new_name)
{
}

void ChangeNamedColorNameCommand::undo()
{
  auto& model = Application::instance().scene.named_colors();
  model.rename(m_old_name, m_new_name);
}

void ChangeNamedColorNameCommand::redo()
{
  auto& model = Application::instance().scene.named_colors();
  model.rename(m_old_name, m_new_name);
}

ChangeNamedColorColorCommand::ChangeNamedColorColorCommand(const std::string& name, const Color& color)
  : Command(QObject::tr("Change Named Color color").toStdString())
  , m_name(name), m_old_color(color), m_new_color(color)
{
  assert(color.model() != Color::Model::Named);
}

void ChangeNamedColorColorCommand::undo()
{
  auto& model = Application::instance().scene.named_colors();
  model.change(m_name, m_old_color);
}

void ChangeNamedColorColorCommand::redo()
{
  auto& model = Application::instance().scene.named_colors();
  model.change(m_name, m_new_color);
}

bool ChangeNamedColorColorCommand::mergeWith(const QUndoCommand* other)
{
  return m_name == static_cast<const ChangeNamedColorColorCommand&>(*other).m_name;
}

RemoveNamedColorCommand::RemoveNamedColorCommand(const std::string& color_name)
  : NamedColorAddRemoveCommand(QObject::tr("Remove Named Color").toStdString(), color_name,
                               Application::instance().scene.named_colors().color(color_name))
{
}

AddNamedColorCommand::AddNamedColorCommand(const std::string& color_name, const Color& value)
  : NamedColorAddRemoveCommand(QObject::tr("Add Named Color").toStdString(), color_name, value)
{

}

}  // namespace omm

#include "commands/movecommand.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/scene.h"

namespace
{

template<typename T> using Context = typename omm::MoveCommand<T>::Context;

template<typename T> Context<T>
make_old_context(const Context<T>& new_context)
{
  // TODO better use general Context ctor here than defining a special Context ctor
  return Context<T>(new_context.subject);
}

}  // namespace

namespace omm
{

template<typename T> MoveCommand<T>
::MoveCommand(Scene& scene, const std::vector<Context>& new_contextes)
  : Command(QObject::tr("reparent").toStdString())
  , m_old_contextes(::transform<Context>(new_contextes, make_old_context<T>))
  , m_new_contextes(new_contextes)
  , m_scene(scene)
{
}

template<typename T> void MoveCommand<T>::MoveCommand::redo()
{
  for (auto& context : m_new_contextes) {
    m_scene.move(context);
  }
}

template<typename T> void MoveCommand<T>::MoveCommand::undo()
{
  for (auto& context : m_old_contextes) {
    m_scene.move(context);
  }
}

template class MoveCommand<Object>;
template class MoveCommand<Style>;

}  // namespace omm

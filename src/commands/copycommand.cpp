#include "commands/copycommand.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/scene.h"

namespace omm
{

template<typename T>
CopyCommand<T>::CopyCommand(Scene& scene, std::vector<Context> contextes)
  : Command(QObject::tr("copy").toStdString())
  , m_contextes(std::move(contextes))
  , m_scene(scene)
{
  const T* predecessor = m_contextes.front().predecessor;
  for (auto& context : m_contextes) {
    context.subject.capture_by_copy();
    context.predecessor = predecessor;
    predecessor = &context.subject.reference();
  }
}

template<typename T> void CopyCommand<T>::redo()
{
  for (auto&& context : m_contextes) {
    assert(context.subject.owns());
    m_scene.insert(context);
  }
}

template<typename T> void CopyCommand<T>::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(!it->subject.owns());
    m_scene.remove(*it);
  }
}

template class CopyCommand<Object>;
template class CopyCommand<Style>;

}  // namespace omm

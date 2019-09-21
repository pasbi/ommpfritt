#include "commands/command.h"
#include "variant.h"
#include <map>
#include <set>

namespace omm
{

class Animator;
class Property;
class KeyframeCommand : public Command
{
protected:
  KeyframeCommand(Animator& animator, const std::string& label, int frame,
                  const std::map<AbstractPropertyOwner*, std::pair<Property*, variant_type> >& values);
  void insert();
  void remove();

private:
  Animator& m_animator;
  const int m_frame;
  const std::map<AbstractPropertyOwner*, std::pair<Property*, variant_type>> m_values;
  const std::string m_property_key;
};

class RemoveKeyframeCommand : public KeyframeCommand
{
public:
  RemoveKeyframeCommand(Animator& animator, int frame,
                        const std::map<AbstractPropertyOwner*, Property*>& properties);
  void undo() override { insert(); }
  void redo() override { remove(); }
};

class InsertKeyframeCommand : public KeyframeCommand
{
public:
  InsertKeyframeCommand(Animator& animator, int frame,
                        const std::map<AbstractPropertyOwner*, std::pair<Property*, variant_type>>& values);
  InsertKeyframeCommand(Animator& animator, int frame,
                        const std::map<AbstractPropertyOwner*, Property*>& properties);
  void undo() override { remove(); }
  void redo() override { insert(); }
};

}  // namespace omm

#include "commands/command.h"
#include "variant.h"
#include <map>
#include <set>

namespace omm
{

class Animator;
class KeyframeCommand : public Command
{
protected:
  KeyframeCommand(const std::string& label, Animator& animator, int frame,
                  const std::map<AbstractPropertyOwner*, variant_type>& values,
                  const std::string& property_key);
  void insert();
  void remove();

private:
  Animator& m_animator;
  const int m_frame;
  const std::map<AbstractPropertyOwner*, variant_type> m_values;
  const std::string m_property_key;
};

class RemoveKeyframeCommand : public KeyframeCommand
{
public:
  RemoveKeyframeCommand(Animator& animator, int frame,
                        const std::set<AbstractPropertyOwner*>& owners,
                        const std::string& property_key);
  void undo() override { insert(); }
  void redo() override { remove(); }
};

class InsertKeyframeCommand : public KeyframeCommand
{
public:
  InsertKeyframeCommand(Animator& animator, int frame,
                        const std::map<AbstractPropertyOwner*, variant_type>& values,
                        const std::string& property_key);
  InsertKeyframeCommand(Animator& animator, int frame,
                        const std::set<AbstractPropertyOwner*>& owners,
                        const std::string& property_key);
  void undo() override { remove(); }
  void redo() override { insert(); }
};

}  // namespace omm

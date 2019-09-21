#include "commands/command.h"
#include "variant.h"
#include <map>
#include <set>

namespace omm
{

class Property;
class KeyframeCommand : public Command
{
protected:
  KeyframeCommand(const std::string& label, int frame,
                  const std::map<Property*, variant_type>& values);
  void insert();
  void remove();

private:
  const int m_frame;
  const std::map<Property*, variant_type> m_values;
  const std::string m_property_key;
};

class RemoveKeyframeCommand : public KeyframeCommand
{
public:
  RemoveKeyframeCommand(int frame, const std::set<Property*>& properties);
  void undo() override { insert(); }
  void redo() override { remove(); }
};

class InsertKeyframeCommand : public KeyframeCommand
{
public:
  InsertKeyframeCommand(int frame, const std::map<Property*, variant_type>& values);
  InsertKeyframeCommand(int frame, const std::set<Property*>& properties);
  void undo() override { remove(); }
  void redo() override { insert(); }
};

}  // namespace omm

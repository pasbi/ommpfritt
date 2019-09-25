#include "commands/command.h"
#include "variant.h"
#include <map>
#include <set>
#include "animation/track.h"

namespace omm
{

class Animator;
class Property;
class KeyframeCommand : public Command
{
protected:
  KeyframeCommand(Animator& animator, const std::string& label, int frame,
                  const std::map<Property*, Track::Knot>& values);
  void insert();
  void remove();

private:
  Animator& m_animator;
  const int m_frame;
  const std::map<Property*, Track::Knot> m_knots;
  const std::string m_property_key;
};

class RemoveKeyframeCommand : public KeyframeCommand
{
public:
  RemoveKeyframeCommand(Animator& animator, int frame,
                        const std::set<Property*>& properties);
  void undo() override { insert(); }
  void redo() override { remove(); }
};

class InsertKeyframeCommand : public KeyframeCommand
{
public:
  InsertKeyframeCommand(Animator& animator, int frame,
                        const std::set<Property*>& properties);
  InsertKeyframeCommand(Animator& animator, int frame,
                        const std::map<Property*, Track::Knot>& values);
  void undo() override { remove(); }
  void redo() override { insert(); }
};

class MoveKeyFrameCommand : public Command
{
public:
  MoveKeyFrameCommand(Animator& animator, Property& property, std::set<int> old_frames, int shift);

  void undo() override;
  void redo() override;

private:
  std::map<int, Track::Knot> m_removed_values;
  Animator& m_animator;
  Property& m_property;
  const std::set<int> m_old_frames;
  const int m_shift;
  void shift_keyframes(bool invert);

};

}  // namespace omm

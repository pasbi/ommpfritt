#pragma once

#include "commands/command.h"
#include "variant.h"
#include <map>
#include <set>

namespace omm
{

class Animator;
class Property;
struct Knot;

class KeyFrameCommand : public Command
{
protected:
  KeyFrameCommand(Animator& animator,
                  const QString& label,
                  int frame,
                  const std::map<Property*, Knot*>& refs,
                  std::map<Property*, std::unique_ptr<Knot>>&& owns = {});
  KeyFrameCommand(Animator& animator,
                  const QString& label,
                  int frame,
                  std::map<Property*, std::unique_ptr<Knot>>&& owns);
  void insert();
  void remove();

private:
  Animator& m_animator;
  const int m_frame;
  const std::map<Property*, Knot*> m_refs;
  std::map<Property*, std::unique_ptr<Knot>> m_owns;
  const QString m_property_key;
};

class RemoveKeyFrameCommand : public KeyFrameCommand
{
public:
  RemoveKeyFrameCommand(Animator& animator, int frame, const std::set<Property*>& values);
  void undo() override
  {
    insert();
  }
  void redo() override
  {
    remove();
  }
};

class InsertKeyFrameCommand : public KeyFrameCommand
{
public:
  InsertKeyFrameCommand(Animator& animator, int frame, const std::set<Property*>& properties);
  void undo() override
  {
    remove();
  }
  void redo() override
  {
    insert();
  }
};

class MoveKeyFrameCommand : public Command
{
public:
  MoveKeyFrameCommand(Animator& animator,
                      Property& property,
                      const std::set<int>& old_frames,
                      int shift);

  void undo() override;
  void redo() override;

  bool mergeWith(const QUndoCommand* other) override;
  int id() const override
  {
    return MOVE_KEYFRAMES_COMMAND_ID;
  }

private:
  mutable std::map<int, std::unique_ptr<Knot>> m_removed_values;
  Animator& m_animator;
  Property& m_property;
  const std::set<int> m_old_frames;
  int m_shift;
  void shift_keyframes(bool invert);
};

class ChangeKeyFrameCommand : public Command
{
public:
  ChangeKeyFrameCommand(int frame, Property& property, std::unique_ptr<Knot> new_value);
  void undo() override
  {
    swap();
  }
  void redo() override
  {
    swap();
  }
  bool mergeWith(const QUndoCommand* other) override;
  [[nodiscard]] int id() const override
  {
    return CHANGE_KEYFRAMES_COMMAND_ID;
  }

private:
  const int m_frame;
  Property& m_property;
  std::unique_ptr<Knot> m_other_value;
  void swap();
};

}  // namespace omm

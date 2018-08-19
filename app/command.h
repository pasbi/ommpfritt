#pragma once
#include <memory>
#include <string>

class Command
{
public:
  explicit Command(const std::string& label);
  virtual ~Command();

  virtual void redo() = 0;
  virtual void undo() = 0;

  /**
   * @brief merges @code other command with @code this command.
   * 
   * @param other the other command
   * @return the unchanged @code other command if merging was not possible
   *  an empty unique_ptr if merge was successfull.
   */
  virtual std::unique_ptr<Command> merge(std::unique_ptr<Command> other);

private:
  const std::string m_label;

};
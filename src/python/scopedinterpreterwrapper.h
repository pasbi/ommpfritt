#pragma once

namespace omm
{

/**
 * @brief The ScopedInterpreterWrapper class
 * The pybind11::scoped_interpreter is a RAII guard for the python interpreter.
 * For some reasons compiler warnings occur if the pybind11::scoped_interpreter is used in a header:
 *  `... declared with greater visibility than the type of its field ...`
 * Apparently this is behavior not unique to pybind11::scoped_interpreter; other pybind11-types
 * exhibit similar behavior.
 *
 * The solution is to introduce another layer of abstraction (as usually).
 * The pybind11::scoped_interpreter is not mentioned within this very header.
 * Rather, a void* pointer to a pybind11::scoped_interpreter-object on the heap is stored.
 * It's lifetime is manually handled using new/delete in ctor/dtor.
 */
class ScopedInterpreterWrapper
{
public:
  ScopedInterpreterWrapper();
  ~ScopedInterpreterWrapper();

  ScopedInterpreterWrapper(const ScopedInterpreterWrapper&) = delete;
  ScopedInterpreterWrapper(ScopedInterpreterWrapper&&) = delete;
  ScopedInterpreterWrapper& operator=(const ScopedInterpreterWrapper&) = delete;

private:
  void* m_scoped_interpreter;
  static ScopedInterpreterWrapper* m_instance;

};


}  // namespace

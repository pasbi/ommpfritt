#pragma once

#include "managers/managertitlebar.h"

namespace omm
{
class PythonConsole;

class PythonConsoleTitleBar : public ManagerTitleBar
{
public:
  explicit PythonConsoleTitleBar(PythonConsole& parent);
};

}  // namespace omm

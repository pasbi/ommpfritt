#pragma once

#include <QGuiApplication>
#include <vector>
#include <array>

namespace ommtest
{

template<std::size_t N>
std::vector<char*> string_array_to_charpp(std::array<std::string, N>& string_array)
{
  std::vector<char*> vs;
  vs.reserve(N);
  for (std::size_t i = 0; i < N; ++i) {
    vs.push_back(string_array.at(i).data());
  }
  return vs;
}

class GuiApplication
{
private:
  std::array<std::string, 3> argv_{"test", "-platform", "offscreen"};
  std::vector<char*> argv = string_array_to_charpp(argv_);
  int argc = argv.size();
  QGuiApplication m_application;
public:
  explicit GuiApplication();
  QGuiApplication& gui_application();
};

}  // namespace ommtest

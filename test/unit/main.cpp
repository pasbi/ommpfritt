#include "main/application.h"
#include "gtest/gtest.h"
#include "registers.h"
#include "testutil.h"
#include "python/pythonengine.h"
#include <QApplication>


int main(int argc, char* argv[])
{
  omm::PythonEngine::instance();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

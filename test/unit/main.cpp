#include "gtest/gtest.h"
#include "mainwindow/application.h"
#include <QApplication>

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#include <gtest/gtest.h>  // for InitGoogleTest, RUN_ALL_TESTS
#include "mainwindow/application.h"
#include <QApplication>
#include "mainwindow/geos.h"

int main(int argc, char* argv[])
{
  omm::Geos geos;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

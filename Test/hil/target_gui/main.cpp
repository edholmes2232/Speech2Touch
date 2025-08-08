#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv); // Create the Qt application
  MainWindow w; // Create an instance of our window
  w.show(); // Show the window
  return a.exec(); // Start the event loop
}
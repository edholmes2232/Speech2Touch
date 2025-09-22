#include "mainwindow.h"
#include "touch_targets.h"

#include <QApplication>
#include <QLoggingCategory>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
  // Parse command line arguments
  po::options_description desc("Allowed options");
  desc.add_options() //
      ("help,h", "produce help message") //
      ("input", po::value<std::string>(), "input device path (e.g. /dev/input/eventX)") //
      ("verbose,v", "enable debug output");

  po::variables_map vm;
  std::string input_device_path;

  try
  {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error parsing command line: " << e.what() << std::endl;
    return 1;
  }

  if (vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }

  // Configure Qt logging based on verbose flag
  if (!vm.count("verbose"))
  {
    // Disable Qt debug output by default
    QLoggingCategory::setFilterRules("*.debug=false\n*.warning=false\n*.info=false");
  }

  if (vm.count("input"))
  {
    input_device_path = vm["input"].as<std::string>();
  }
  else
  {
    std::cerr << "Warning: No --input argument provided. Touch events will not be processed." << std::endl;
    input_device_path = ""; // Empty string will be handled gracefully
  }

  // Create Qt application
  QApplication app(argc, argv);

  // Create main window
  MainWindow window(input_device_path.c_str());

  // Set up a simple callback for button releases (just print to console)
  // This replaces the gtest promise/future mechanism

  window.show();

  std::cout << "Harness GUI started. Close the window to exit." << std::endl;

  // Start the Qt event loop
  return app.exec();
}

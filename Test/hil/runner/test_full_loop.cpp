#include "mainwindow.h" // Your window
#include "touch_targets.h"
#include "tts.h"

#include <QApplication>
#include <QList>
#include <QMetaObject>
#include <QPushButton>
#include <Qt>
#include <algorithm>
#include <atomic>
#include <boost/program_options.hpp>
#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace po = boost::program_options;

namespace
{
// Global device path for use in test fixture
std::string _input_device_path;

MainWindow *_main_window;
QApplication *_qt_app;
std::thread _qt_thread;
};

class HilTest : public ::testing::Test
{
  protected:
  // Max time to wait for Device to press target
  static constexpr int MAX_WAIT_MS = 2000;

  std::promise<TARGET_T> _button_released_promise;
  std::future<TARGET_T> _button_released_future;
  std::unique_ptr<TTS> _tts;

  // void SetUp() override
  static void SetUpTestSuite()
  {

    // Check if input device path is set
    if (_input_device_path.empty())
    {
      GTEST_FAIL() << "Input device path is required. Use --input argument to specify it.";
    }

    // Launch the Qt event loop in a separate thread
    _qt_thread = std::thread(
        [&]()
        {
          int argc = 1;
          char *argv[] = {(char *)"test", nullptr};

          QApplication app(argc, argv);
          MainWindow window(_input_device_path.c_str());

          _qt_app = &app;
          _main_window = &window;
          _main_window->show();

          // Start the Qt event loop
          // This blocks until quit() is called
          QApplication::exec();
        });

    // Wait for Qt to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  void SetUp() override
  {

    // Set up the TTS system
    _tts = std::make_unique<TTS>();
    // Set up the promise and future for button release
    _button_released_future = _button_released_promise.get_future();

    // Set up the main window buttons
    QMetaObject::invokeMethod(
        _main_window,
        [this]() { _main_window->setButtonReleasedPromise(&_button_released_promise); },
        Qt::BlockingQueuedConnection);
  }

  // This runs after each TEST_F
  static void TearDownTestSuite()
  {
    // Shut down the Qt application from it's own thread
    if (_main_window)
    {
      QMetaObject::invokeMethod(_qt_app, "quit", Qt::QueuedConnection);
    }
    // Wait for the thread to finish
    if (_qt_thread.joinable())
    {
      _qt_thread.join();
    }
  }

  bool expectButtonReleased(TARGET_T expected_button, int timeout_ms = 2000)
  {
    // Wait for the button release promise to be fulfilled
    auto status = _button_released_future.wait_for(std::chrono::milliseconds(timeout_ms));
    if (status != std::future_status::ready)
    {
      return false; // Timeout
    }

    // Get the result
    TARGET_T result = _button_released_future.get();
    EXPECT_EQ(result, expected_button) << "Expected " << touch_targets[expected_button].name << " but got "
                                       << touch_targets[result].name;
    return result == expected_button;
  }
};

// Helper to generate all valid targets
std::vector<TARGET_T> get_all_targets()
{
  std::vector<TARGET_T> targets;
  for (int i = 0; i < TARGET_COUNT; ++i)
  {
    targets.push_back(static_cast<TARGET_T>(i));
  }
  return targets;
}

// Custom name generator for parameterized tests
std::string TargetNameGenerator(const ::testing::TestParamInfo<TARGET_T> &info)
{
  const char *name = touch_targets[info.param].name;
  std::string s(name);
  // Replace spaces and special chars for valid C++ identifiers
  std::replace(s.begin(), s.end(), ' ', '_');
  std::replace(s.begin(), s.end(), '-', '_');
  std::replace(s.begin(), s.end(), '.', '_');
  return s;
}

// Parameterized test for all targets
class TargetButtonPressed : public HilTest, public ::testing::WithParamInterface<TARGET_T>
{
};

INSTANTIATE_TEST_SUITE_P(AllTargets, TargetButtonPressed, ::testing::ValuesIn(get_all_targets()), TargetNameGenerator);

TEST_P(TargetButtonPressed, Pressed)
{
  const TARGET_T target = GetParam();
  _tts->say(std::string("Frankie! Make me a ") + touch_targets[target].name + ".");
  ASSERT_TRUE(expectButtonReleased(target, MAX_WAIT_MS)) << "Failed to press target: " << touch_targets[target].name;
}

int main(int argc, char **argv)
{
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "produce help message")(
      "input", po::value<std::string>(), "input device path (e.g. /dev/input/eventX)");

  po::variables_map vm;
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
  if (vm.count("input"))
  {
    _input_device_path = vm["input"].as<std::string>();
  }
  else
  {
    std::cerr << "--input argument is required" << std::endl;
    _input_device_path = {};
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#include "mainwindow.h" // Your window
#include "touch_targets.h"

#include <QApplication>
#include <QList>
#include <QMetaObject>
#include <QPushButton>
#include <Qt>
#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <thread>

class HilTest : public ::testing::Test
{
  protected:
  MainWindow *_main_window;
  QApplication *_qt_app;
  std::thread _qt_thread;
  std::promise<TARGET_T> _button_released_promise;
  std::future<TARGET_T> _button_released_future;

  void SetUp() override
  {

    // Launch the Qt event loop in a separate thread
    _qt_thread = std::thread(
        [&]()
        {
          int argc = 1;
          char *argv[] = {(char *)"test", nullptr};

          QApplication app(argc, argv);
          MainWindow window;

          _qt_app = &app;

          _main_window = &window;
          _main_window->show();

          // Start the Qt event loop
          // This blocks until quit() is called
          QApplication::exec();
        });

    // Wait for Qt to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // Set up the promise and future for button release
    _button_released_future = _button_released_promise.get_future();

    // Set up the main window buttons
    QMetaObject::invokeMethod(
        _main_window,
        [this]() { _main_window->setButtonReleasedPromise(&_button_released_promise); },
        Qt::BlockingQueuedConnection);
  }

  // This runs after each TEST_F
  void TearDown() override
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

TEST_F(HilTest, CaffeLattePressed)
{
  ASSERT_TRUE(expectButtonReleased(TARGET_CAFFE_LATTE, 2000));
}
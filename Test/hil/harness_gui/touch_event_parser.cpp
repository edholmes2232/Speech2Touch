#include "touch_event_parser.h"

#include <QDebug>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

TouchEventParser::TouchEventParser(const char *device_path, TouchCallback callback, QObject *parent)
    : QObject(parent)
    , _touch_fd(-1)
    , _notifier(nullptr)
    , _current_x(0)
    , _current_y(0)
    , _is_touched(false)
    , _was_touched(false)
    , _callback(callback)
{
  _touch_fd = open(device_path, O_RDONLY | O_NONBLOCK);
  if (_touch_fd < 0)
  {
    qWarning() << "FATAL: Failed to open touch device:" << device_path << strerror(errno);
    return;
  }
  if (ioctl(_touch_fd, EVIOCGRAB, 1) < 0)
  {
    qWarning() << "FATAL: EVIOCGRAB failed. Are you running with sudo?";
    close(_touch_fd);
    _touch_fd = -1;
    return;
  }
  if (ioctl(_touch_fd, EVIOCGABS(ABS_X), &_axis_info_x) < 0 || ioctl(_touch_fd, EVIOCGABS(ABS_Y), &_axis_info_y) < 0)
  {
    qWarning() << "FATAL: Could not get axis info from touch device.";
    ioctl(_touch_fd, EVIOCGRAB, 0);
    close(_touch_fd);
    _touch_fd = -1;
    return;
  }
  _notifier = new QSocketNotifier(_touch_fd, QSocketNotifier::Read, this);
  connect(_notifier, &QSocketNotifier::activated, this, &TouchEventParser::readTouchDevice);
}

TouchEventParser::~TouchEventParser()
{
  if (_touch_fd >= 0)
  {
    ioctl(_touch_fd, EVIOCGRAB, 0);
    close(_touch_fd);
  }
}

void TouchEventParser::readTouchDevice()
{
  struct input_event event;
  while (read(_touch_fd, &event, sizeof(struct input_event)) > 0)
  {
    if (event.type == EV_ABS)
    {
      if (event.code == ABS_X)
      {
        _current_x = event.value;
      }
      else if (event.code == ABS_Y)
      {
        _current_y = event.value;
      }
    }
    else if (event.type == EV_KEY && event.code == BTN_TOUCH)
    {
      _is_touched = event.value;
    }
    else if (event.type == EV_SYN && event.code == SYN_REPORT)
    {
      float percent_x =
          static_cast<float>(_current_x - _axis_info_x.minimum) / (_axis_info_x.maximum - _axis_info_x.minimum);
      float percent_y =
          static_cast<float>(_current_y - _axis_info_y.minimum) / (_axis_info_y.maximum - _axis_info_y.minimum);
      if (_callback)
        _callback(percent_x, percent_y, _is_touched);
      _was_touched = _is_touched;
    }
  }
}

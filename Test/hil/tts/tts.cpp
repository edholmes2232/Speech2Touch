#include "tts.h"

#include <cstdlib>
#include <iostream>
extern "C" int system(const char *);

TTS::TTS()
{
  if (system("command -v festival > /dev/null 2>&1") == 0)
  {
    std::cout << "Using Festival TTS engine." << std::endl;
    engine = Engine::Festival;
  }
  else if (system("command -v flite > /dev/null 2>&1") == 0)
  {
    std::cout << "Using Flite TTS engine." << std::endl;
    engine = Engine::Flite;
  }
  else if (system("command -v espeak > /dev/null 2>&1") == 0)
  {
    std::cout << "Using Espeak TTS engine." << std::endl;
    engine = Engine::Espeak;
  }
  else
  {
    throw std::runtime_error("No supported TTS engine (festival, flite, espeak) found on system.");
  }
}

void TTS::say(const std::string &text)
{
  std::string cmd;
  switch (engine)
  {
  case Engine::Festival:
    cmd = "echo \"" + text + "\" | festival --tts";
    break;
  case Engine::Flite:
    cmd = "flite -t \"" + text + "\"";
    break;
  case Engine::Espeak:
    cmd = "espeak \"" + text + "\"";
    break;
  default:
    throw std::runtime_error("No TTS engine available.");
  }
  int ret = system(cmd.c_str());
  if (ret != 0)
  {
    std::cerr << "TTS command failed: " << cmd << std::endl;
  }
}

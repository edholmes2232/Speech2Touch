#pragma once
#include <stdexcept>
#include <string>

class TTS
{
  public:
  TTS();
  void say(const std::string &text);

  private:
  enum class Engine
  {
    None,
    Festival,
    Flite,
    Espeak
  };
  Engine engine = Engine::None;
};
